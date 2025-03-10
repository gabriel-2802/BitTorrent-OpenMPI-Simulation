#include "tracker.h"
#include "auxiliary.h"

using namespace std;

Tracker::Tracker(int numtasks, int rank) : TorrentEntity(numtasks, rank) {
    active_clients = numtasks - 1;
    upload_per_client.resize(numtasks, 0);
}

Tracker::~Tracker() {

}

void Tracker::run() {
    int ack = 1;
    // gets all data from peers
    collectInformation();
    // announce all clients that tracker is ready
    MPI_Bcast(&ack, 1, MPI_INT, TRACKER_RANK, MPI_COMM_WORLD);

    while (true) {
        // wait for messages from clients
        MPI_Status status;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        int source = status.MPI_SOURCE;
        COMMUNICATION_TAG req = (COMMUNICATION_TAG) status.MPI_TAG;

        handleRequest(source, req);

         if (active_clients == 0)
            break;
    }

    // kill uploads threads
    for (int rk = 0; rk < numtasks; ++rk) {
        if (rk == TRACKER_RANK)
            continue;

        MPI_Send(nullptr, 0, MPI_INT, rk, TAG_KILL_ALL, MPI_COMM_WORLD);
    }

    printStringRepresentation();
}

void Tracker::printStringRepresentation() {
    for (auto [fname, swarm] : file_swarms) {
        cout << "File: " << fname << endl;
        cout << "Segments: " << swarm.seg_num << endl;
        cout << "Peers: ";
        for (auto &peer : swarm.peers) {
            cout << peer << " ";
        }
        cout << endl;

        cout << "Seeds: ";
        for (auto &seed : swarm.seeds) {
            cout << seed << " ";
        }
        cout << endl << endl;
    }

    for (int i = 0; i < numtasks; ++i) {
        if (i == TRACKER_RANK)
            continue;
        cout << "Client " << i << " has " << upload_per_client[i] << " uploads\n";
    }

}

void Tracker::collectInformation() {
    // receve all data from clients
    for (int rk = 0; rk < numtasks; ++rk) {
        if (rk == TRACKER_RANK)
            continue;

        file_data_t data;
        MPI_Recv(&data, 1, FILE_DATA_T, rk, TAG_INIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < data.num_files; ++i) {
            std::string fname(data.file_names[i]);

            // create the swarm for the file, if it doesn't exist
            if (file_swarms.find(fname) == file_swarms.end()) {
                swarm_t swarm;
                swarm.fname = fname;
                swarm.seg_num = data.num_frags[i];
                file_swarms[fname] = swarm;

                for (int j = 0; j < data.num_frags[i]; ++j) {
                    string hash(data.hashes[i][j]);
                    file_swarms[fname].f_hash.push_back(hash);
                }
            }

            // Add the client as a seed
            file_swarms[fname].seeds.insert(rk);
        }
    }
}


void Tracker::handleRequest(int src) {
    char *fname = createBuffer(MAX_FILENAME, "");

    MPI_Recv(fname, MAX_FILENAME, MPI_CHAR, src, TAG_PROBING, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    DIE(file_swarms.find(fname) == file_swarms.end(), "File not found");
    string file = fname;

   // at this points it is known that src requested file fname => send the swarm of the file
    swarm_t &swarm = file_swarms[file];
    sendSwarm(swarm, src);
    delete [] fname;
}

void Tracker::handleRequest(int src, COMMUNICATION_TAG req) {
    char *fname = createBuffer(MAX_FILENAME, "");

    switch (req) {
        case TAG_PROBING:
            // client requested the swarm of a file
            handleRequest(src);
            break;
        case TAG_FILE_DONE:
            // client finished downloading a file
            MPI_Recv(fname, MAX_FILENAME, MPI_CHAR, src, TAG_FILE_DONE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            file_swarms[fname].seeds.insert(src);
            file_swarms[fname].peers.erase(src);
            break;
        case TAG_CLIENT_DONE:
            // client finished downloading all files
            MPI_Recv(nullptr, 0, MPI_INT, src, TAG_CLIENT_DONE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            active_clients--;
            break;
        case TAG_SEG_DONE:
            // client finished downloading a segment
            MPI_Recv(fname, MAX_FILENAME, MPI_CHAR, src, TAG_SEG_DONE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            file_swarms[fname].peers.insert(src);
            break;
        case TAG_BUSSYNESS:
            // client requested the number of uploads of all clients
            MPI_Recv(nullptr, 0, MPI_INT, src, TAG_BUSSYNESS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(upload_per_client.data(), numtasks, MPI_INT, src, TAG_BUSSYNESS, MPI_COMM_WORLD);
            break;
        case TAG_UPLOAD_CONFIRM:
            // client confirmed the upload of a fragment
            MPI_Recv(nullptr, 0, MPI_INT, src, TAG_UPLOAD_CONFIRM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            upload_per_client[src]++;
            break;
        default:
            cerr << "Invalid request type\n";
    }
    delete [] fname;
}
