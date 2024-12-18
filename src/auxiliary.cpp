#include "auxiliray.h"

using namespace std;

void send_swarm(const swarm_t &swarm, int dest) {
    swarm_t s;

    int ss = swarm.seeds.size();
    MPI_Send(&ss, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);

    for (auto &seed : swarm.seeds) {
        MPI_Send(&seed, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    }

    int p = swarm.peers.size();
    MPI_Send(&p, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);

    for (auto &peer : swarm.peers) {
        MPI_Send(&peer, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    }

    char fname[MAX_FILENAME];
    memset(fname, 0, MAX_FILENAME);
    memcpy(fname, swarm.fname.c_str(), swarm.fname.size());
    MPI_Send(fname, MAX_FILENAME, MPI_CHAR, dest, 0, MPI_COMM_WORLD);

    int segNum = swarm.segNum;
    MPI_Send(&segNum, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);

    for (auto &frag : swarm.f_hash) {
        char hash[HASH_SIZE + 1];
        memset(hash, 0, HASH_SIZE + 1);
        memcpy(hash, frag.c_str(), frag.size());
        MPI_Send(hash, HASH_SIZE + 1, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
    }
  
}


void receive_swarm(swarm_t &swarm, int src) {
    int s;
    MPI_Recv(&s, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for (int i = 0; i < s; ++i) {
        int seed;
        MPI_Recv(&seed, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        swarm.seeds.insert(seed);
    }

    int p;
    MPI_Recv(&p, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for (int i = 0; i < p; ++i) {
        int peer;
        MPI_Recv(&peer, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        swarm.peers.insert(peer);
    }

    char fname[MAX_FILENAME];
    memset(fname, 0, MAX_FILENAME);
    MPI_Recv(fname, MAX_FILENAME, MPI_CHAR, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    swarm.fname = fname;

    int segNum;
    MPI_Recv(&segNum, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    swarm.segNum = segNum;

   while (segNum--) {
        char hash[HASH_SIZE + 1];
        memset(hash, 0, HASH_SIZE + 1);
        MPI_Recv(hash, HASH_SIZE + 1, MPI_CHAR, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        swarm.f_hash.push_back(hash);
    }

}


void download_file(void *arg, const swarm_t& swarm) {
    download_args_t *args = (download_args_t *) arg;

    

}