#include "Tracker.h"
#include "auxiliray.h"

using namespace std;

Tracker::Tracker(int numtasks, int rank) : TorrentEntity(numtasks, rank) {
    activePeers = numtasks - 1;
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
        REQUEST_TYPE req;
        MPI_Recv(&req, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    }


    debugPrint();
}

void Tracker::debugPrint() {
    for (auto &swarm : swarms) {
        std::cout << swarm.first << endl;

        for (auto &frag : swarm.second.second) {
            std::cout << frag.first << " " << frag.second << endl;
        }
    }
}
// TODO: Improve ? 
void Tracker::collectInformation() {
    for (int rnk = 0; rnk < numtasks; rnk++) {
        if (rnk == TRACKER_RANK) {
            continue;
        }
        
        int num_files;
        MPI_Recv(&num_files, 1, MPI_INT, rnk, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        char fName[MAX_FILENAME], hash[HASH_SIZE + 1];
        memset(hash, 0, HASH_SIZE + 1);
        memset(fName, 0, MAX_FILENAME);

        while (num_files--) {
            // get file name
            MPI_Recv(fName, MAX_FILENAME, MPI_CHAR, rnk, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // add file to swarms if it doesn't exist
            if (swarms.find(fName) == swarms.end()) {
                swarms[fName] = {rnk, vector<f_frag_t>()};
            }

            int num_frags;
            MPI_Recv(&num_frags, 1, MPI_INT, rnk, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            while (num_frags--) {
                int idx;
                MPI_Recv(&idx, 1, MPI_INT, rnk, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                MPI_Recv(hash, HASH_SIZE + 1, MPI_CHAR, rnk, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                swarms[fName].second.push_back({idx, hash});
            }
        }
    }
}
