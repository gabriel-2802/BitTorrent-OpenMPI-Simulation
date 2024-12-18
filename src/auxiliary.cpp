#include "auxiliray.h"

using namespace std;

void send_swarm(const swarm_t &swarm, int dest) {
    int ss = swarm.seeds.size();
    MPI_Send(&ss, 1, MPI_INT, dest, TAG_SWARM, MPI_COMM_WORLD);

    for (auto &seed : swarm.seeds) {
        MPI_Send(&seed, 1, MPI_INT, dest, TAG_SWARM, MPI_COMM_WORLD);
    }

    int p = swarm.peers.size();
    MPI_Send(&p, 1, MPI_INT, dest, TAG_SWARM, MPI_COMM_WORLD);

    for (auto &peer : swarm.peers) {
        MPI_Send(&peer, 1, MPI_INT, dest, TAG_SWARM, MPI_COMM_WORLD);
    }

    char fname[MAX_FILENAME];
    memset(fname, 0, MAX_FILENAME);
    memcpy(fname, swarm.fname.c_str(), swarm.fname.size());
    MPI_Send(fname, MAX_FILENAME, MPI_CHAR, dest, TAG_SWARM, MPI_COMM_WORLD);

    int segNum = swarm.seg_num;
    MPI_Send(&segNum, 1, MPI_INT, dest, TAG_SWARM, MPI_COMM_WORLD);

    for (auto &frag : swarm.f_hash) {
        char hash[HASH_SIZE + 1];
        memset(hash, 0, HASH_SIZE + 1);
        memcpy(hash, frag.c_str(), frag.size());
        MPI_Send(hash, HASH_SIZE + 1, MPI_CHAR, dest, TAG_SWARM, MPI_COMM_WORLD);
    }
}

void receive_swarm(swarm_t &swarm, int src) {
    int s;
    MPI_Recv(&s, 1, MPI_INT, src, TAG_SWARM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for (int i = 0; i < s; ++i) {
        int seed;
        MPI_Recv(&seed, 1, MPI_INT, src, TAG_SWARM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        swarm.seeds.insert(seed);
    }

    int p;
    MPI_Recv(&p, 1, MPI_INT, src, TAG_SWARM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for (int i = 0; i < p; ++i) {
        int peer;
        MPI_Recv(&peer, 1, MPI_INT, src, TAG_SWARM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        swarm.peers.insert(peer);
    }

    char fname[MAX_FILENAME];
    memset(fname, 0, MAX_FILENAME);
    MPI_Recv(fname, MAX_FILENAME, MPI_CHAR, src, TAG_SWARM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    swarm.fname = fname;

    int segNum;
    MPI_Recv(&segNum, 1, MPI_INT, src, TAG_SWARM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    swarm.seg_num = segNum;

    while (segNum--) {
        char hash[HASH_SIZE + 1];
        memset(hash, 0, HASH_SIZE + 1);
        MPI_Recv(hash, HASH_SIZE + 1, MPI_CHAR, src, TAG_SWARM, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        swarm.f_hash.push_back(hash);
    }
}

void send_inquiry(const inquiry_t &inquiry, int dest) {
    // Send frag_idx (integer)
    MPI_Send(&inquiry.frag_idx, 1, MPI_INT, dest, TAG_INQUIRY, MPI_COMM_WORLD);

    // Send fileName (character array)
    MPI_Send(inquiry.fileName, MAX_FILENAME, MPI_CHAR, dest, TAG_INQUIRY, MPI_COMM_WORLD);

    // Send hash (character array)
    MPI_Send(inquiry.hash, HASH_SIZE + 1, MPI_CHAR, dest, TAG_INQUIRY, MPI_COMM_WORLD);
}

void receive_inquiry(inquiry_t &inquiry, int src) {
    MPI_Status status;

    // Receive frag_idx (integer)
    MPI_Recv(&inquiry.frag_idx, 1, MPI_INT, src, TAG_INQUIRY, MPI_COMM_WORLD, &status);

    // Receive fileName (character array)
    MPI_Recv(inquiry.fileName, MAX_FILENAME, MPI_CHAR, src, TAG_INQUIRY, MPI_COMM_WORLD, &status);

    // Receive hash (character array)
    MPI_Recv(inquiry.hash, HASH_SIZE + 1, MPI_CHAR, src, TAG_INQUIRY, MPI_COMM_WORLD, &status);
}



MPI_Datatype createInquiryType() {
    MPI_Datatype INQUIRY_T;

    const int nItems = 3;

    int blockLengths[nItems] = {
        1,                  
        MAX_FILENAME,         
        HASH_SIZE + 1        
    };

    MPI_Datatype types[nItems] = {
        MPI_INT,              
        MPI_CHAR,             
        MPI_CHAR              
    };

    MPI_Aint offsets[nItems];
    offsets[0] = offsetof(inquiry_t, frag_idx);
    offsets[1] = offsetof(inquiry_t, fileName);
    offsets[2] = offsetof(inquiry_t, hash);


    MPI_Type_create_struct(nItems, blockLengths, offsets, types, &INQUIRY_T);
    MPI_Type_commit(&INQUIRY_T);

    return INQUIRY_T;
}