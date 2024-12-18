#pragma once
#include <mpi.h>
#include <pthread.h>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#define TRACKER_RANK 0
#define MAX_FILES 10
#define MAX_FILENAME 15
#define HASH_SIZE 32
#define MAX_CHUNKS 100

#define MAX_USERS 12

#define DIE(assertion, call_description) \
    do { \
        if (assertion) { \
            std::cerr << call_description << "\n"; \
            std::exit(errno); \
        } \
    } while (0)


#define IN_FILE(rank) ("in" + std::to_string(rank) + ".txt")
#define OUT_FILE(rank, name) ("client" + std::to_string(rank) + "_" + name)
#define DOWNLOAD_LIMIT 10

typedef std::pair<int, std::string> f_frag_t; // the index of the fragment and the hash of the fragment

struct download_args_t{
    int rank;
    int *downloads;

    std::vector<std::string> *wantedFiles;
    std::unordered_map<std::string, std::vector<f_frag_t>> *downloadedFrags; // file -> vector<idx, hash>


};

struct swarm_t {
    std::unordered_set<int> seeds; // client with full file
    std::unordered_set<int> peers; // client with fragments
    std::string fname;
    int segNum;

    std::vector<std::string> f_hash; // file fragments hash
};


struct upload_args_t{
    int rank;

};


enum REQUEST_TYPE{
    SWARM_REQUEST, // client requests a file
    FINALISED_FILE_REQUEST, // client finished downloading a file
    FINALISED_CLIENT_REQUEST, // client finished downloading all files
    FINALISED_SEG_REQUEST,  // client finished downloading a segment
};

enum PeerType {
    PEER,
    SEED
};

enum ThreadType {
    DOWNLOAD,
    UPLOAD
};

void send_swarm(const swarm_t &swarm, int dest);

void receive_swarm(swarm_t &swarm, int src);