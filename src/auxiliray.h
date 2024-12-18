#pragma once
#include <mpi.h>
#include <pthread.h>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstring>

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
#define INFINITY 1000000

struct download_args_t{
    int rank;
    int *to_be_downloaded; // number of files downloaded
    int num;
    pthread_mutex_t *lock;

    std::unordered_map<std::string, bool> wanted_files;
    std::unordered_map<std::string, std::vector<std::string>> *partial_files; // file -> vector<idx, hash>

    
};


struct upload_args_t{
    int rank;
    int num;
    pthread_mutex_t *lock;

    std::unordered_map<std::string, std::vector<std::string>> *full_files;
    std::unordered_map<std::string, std::vector<std::string>> *partial_files;

};

struct swarm_t {
    std::unordered_set<int> seeds; // client with full file
    std::unordered_set<int> peers; // client with fragments
    std::string fname;
    int seg_num;

    std::vector<std::string> f_hash; // file fragments hash
};

struct inquiry_t {
    int frag_idx;
    char fileName[MAX_FILENAME];
    char hash[HASH_SIZE + 1];
};


enum COMMUNICATION_TAG{
    TAG_INIT,
    TAG_PROBING,
    TAG_FILE_DONE,
    TAG_CLIENT_DONE,
    TAG_SEG_DONE,
    TAG_KILL_ALL,
    TAG_SWARM,
    TAG_BUSSYNESS,
    TAG_INQUIRY,
    TAG_INQUIRY_ACK,
    TAG_INQUIRY_RESPONSE,
    TAG_DATA,
    TAG_DATA_SIZE
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

void send_inquiry(const inquiry_t &inquiry, int dest);

void receive_inquiry(inquiry_t &inquiry, int src);

std::string serialize_swarm(const swarm_t &swarm);

swarm_t deserialize_swarm(const std::string &data);

MPI_Datatype createInquiryType();
