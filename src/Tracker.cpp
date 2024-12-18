#include "Tracker.h"
#include "auxiliray.h"

using namespace std;

Tracker::Tracker(int numtasks, int rank) : TorrentEntity(numtasks, rank) {
    activePeers = numtasks - 1;
    uploadPerClient.resize(numtasks, 0);
}

Tracker::~Tracker() {

}

void Tracker::run() {
    int ack = 1;

    // gets all data from peers
    collectInformation();
    // announce all peers that tracker is ready
    MPI_Bcast(&ack, 1, MPI_INT, TRACKER_RANK, MPI_COMM_WORLD);

    while (true) {
        // wait for messages from clients
        MPI_Status status;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        int source = status.MPI_SOURCE;
        COMMUNICATION_TAG req = (COMMUNICATION_TAG) status.MPI_TAG;

        handleRequest(source, req);

        if (activePeers == 0)
            break;

    }



    // kill uploads threads
    for (int rk = 0; rk < numtasks; ++rk) {
        if (rk == TRACKER_RANK)
            continue;

        MPI_Send(nullptr, 0, MPI_INT, rk, TAG_KILL_ALL, MPI_COMM_WORLD);
    }
}

void Tracker::debugPrint() {
    for (auto [fname, swarm] : fileSwarm) {
        cout << endl;
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

}

void Tracker::collectInformation() {
    
    for (int rk = 0; rk < numtasks; ++rk) {
        if (rk == TRACKER_RANK)
            continue;

        int numFiles;
        MPI_Recv(&numFiles, 1, MPI_INT, rk, TAG_INIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        while (numFiles--) {

            char fname[MAX_FILENAME];
            memset(fname, 0, MAX_FILENAME);
            MPI_Recv(fname, MAX_FILENAME, MPI_CHAR, rk, TAG_INIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int numFrags;
            MPI_Recv(&numFrags, 1, MPI_INT, rk, TAG_INIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (fileSwarm.find(fname) == fileSwarm.end()) {
                swarm_t swarm;
                swarm.fname = fname;
                swarm.seg_num = numFrags;
                fileSwarm[fname] = swarm;
                
            }

            fileSwarm[fname].seeds.insert(rk);
            int numFragsCopy = numFrags;

            while (numFragsCopy--) {
                char hash[HASH_SIZE + 1];
                memset(hash, 0, HASH_SIZE + 1);
                MPI_Recv(hash, HASH_SIZE + 1, MPI_CHAR, rk, TAG_INIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                fileSwarm[fname].f_hash.push_back(hash);
            }


        }
        
    }
}

void Tracker::handleRequest(int src) {
    char fname[MAX_FILENAME];
    memset(fname, 0, MAX_FILENAME);

    MPI_Recv(fname, MAX_FILENAME, MPI_CHAR, src, TAG_PROBING, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    DIE(fileSwarm.find(fname) == fileSwarm.end(), "File not found");
    string file = fname;

   // at this points it is known that src requested file fname => send the swarm of the file
    swarm_t &swarm = fileSwarm[file];
    send_swarm(swarm, src);
}

void Tracker::handleRequest(int src, COMMUNICATION_TAG req) {
    char fname[MAX_FILENAME];
    memset(fname, 0, MAX_FILENAME);

    switch (req) {
        case TAG_PROBING:
            handleRequest(src);
            break;
        case TAG_FILE_DONE:
            MPI_Recv(fname, MAX_FILENAME, MPI_CHAR, src, TAG_FILE_DONE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fileSwarm[fname].seeds.insert(src);
            fileSwarm[fname].peers.erase(src);
            break;
        case TAG_CLIENT_DONE:
            MPI_Recv(nullptr, 0, MPI_INT, src, TAG_CLIENT_DONE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            activePeers--;
            break;
        case TAG_SEG_DONE:
            MPI_Recv(fname, MAX_FILENAME, MPI_CHAR, src, TAG_SEG_DONE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            fileSwarm[fname].seeds.insert(src);
            break;
        case TAG_BUSSYNESS:
            MPI_Recv(nullptr, 0, MPI_INT, src, TAG_BUSSYNESS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(uploadPerClient.data(), numtasks, MPI_INT, src, TAG_BUSSYNESS, MPI_COMM_WORLD);
            break;
        default:
            cerr << "Invalid request type\n";
    }
}
