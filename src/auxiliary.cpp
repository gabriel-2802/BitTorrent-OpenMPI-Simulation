#include "auxiliary.h"

using namespace std;

void sendSwarm(const swarm_t &swarm, int dest) {
    std::string data = serializeSwarm(swarm);
    int size = data.size();

    MPI_Send(&size, 1, MPI_INT, dest, TAG_DATA_SIZE, MPI_COMM_WORLD);
    MPI_Send(data.data(), size, MPI_CHAR, dest, TAG_DATA, MPI_COMM_WORLD);
}

void receiveSwarm(swarm_t &swarm, int src) {
    int size;
    MPI_Recv(&size, 1, MPI_INT, src, TAG_DATA_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    char *data = new char[size];
    DIE(!data, "fatal failre in memory allocation");

    MPI_Recv(data, size, MPI_CHAR, src, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    swarm = deserializeSwarm(std::string(data, size));
    delete[] data;
}

std::string serializeSwarm(const swarm_t &swarm) {
    // total size of the buffer
    size_t total_size = sizeof(int) + // seeds size
                        sizeof(int) * swarm.seeds.size() +
                        sizeof(int) + // peers size
                        sizeof(int) * swarm.peers.size() +
                        MAX_FILENAME + // filename
                        sizeof(int) + // segs nr
                        swarm.f_hash.size() * (HASH_SIZE + 1);


    char *data = new char[total_size];
    DIE(!data, "fatal failre in memory allocation");
    char *ptr = data; // current position in the buffer

    // seeds
    int ss = swarm.seeds.size();
    memcpy(ptr, &ss, sizeof(int));
    ptr += sizeof(int);
    for (int seed : swarm.seeds) {
        memcpy(ptr, &seed, sizeof(int));
        ptr += sizeof(int);
    }

    // peers
    int ps = swarm.peers.size();
    memcpy(ptr, &ps, sizeof(int));
    ptr += sizeof(int);
    for (int peer : swarm.peers) {
        memcpy(ptr, &peer, sizeof(int));
        ptr += sizeof(int);
    }

    // filename
    char fname[MAX_FILENAME] = {0};
    strncpy(fname, swarm.fname.c_str(), MAX_FILENAME - 1); // Ensure null-termination
    memcpy(ptr, fname, MAX_FILENAME);
    ptr += MAX_FILENAME;

    // seg nr
    int segNum = swarm.seg_num;
    memcpy(ptr, &segNum, sizeof(int));
    ptr += sizeof(int);

    // hashes
    for (const std::string &frag : swarm.f_hash) {
        char hash[HASH_SIZE + 1] = {0};
        strncpy(hash, frag.c_str(), HASH_SIZE); // Truncate to HASH_SIZE
        memcpy(ptr, hash, HASH_SIZE + 1);
        ptr += HASH_SIZE + 1;
    }

    // return a string 
    std::string res(data, total_size);
    delete [] data;
    return res;
}


swarm_t deserializeSwarm(const std::string &data) {
    const char *data_ptr = data.data();
    swarm_t swarm;

    // seeds
    int seeds_size;
    memcpy(&seeds_size, data_ptr, sizeof(int));
    data_ptr += sizeof(int);
    for (int i = 0; i < seeds_size; ++i) {
        int seed;
        memcpy(&seed, data_ptr, sizeof(int));
        data_ptr += sizeof(int);
        swarm.seeds.insert(seed);
    }

    // peers
    int peers_size;
    memcpy(&peers_size, data_ptr, sizeof(int));
    data_ptr += sizeof(int);
    for (int i = 0; i < peers_size; ++i) {
        int peer;
        memcpy(&peer, data_ptr, sizeof(int));
        data_ptr += sizeof(int);
        swarm.peers.insert(peer);
    }

    // filename
    char fname[MAX_FILENAME] = {0};
    memcpy(fname, data_ptr, MAX_FILENAME);
    data_ptr += MAX_FILENAME;
    swarm.fname = fname;

    // segment number
    memcpy(&swarm.seg_num, data_ptr, sizeof(int));
    data_ptr += sizeof(int);

    //  hashes
    for (int i = 0; i < swarm.seg_num; ++i) {
        char hash[HASH_SIZE + 1] = {0};
        memcpy(hash, data_ptr, HASH_SIZE + 1);
        data_ptr += HASH_SIZE + 1;
        swarm.f_hash.push_back(hash);
    }

    return swarm;
}

char* createBuffer(size_t size, std::string src) {
    char* p = new char[size];
    DIE(!p, "fatal failure in new");

    memset(p, 0, size);

    if (src != "") {
        memcpy(p, src.c_str(), src.size());
    }

    return p;
}

bool checkDataIntegrity(const std::string &data, const std::string &hash) {
	return data == hash;
}
