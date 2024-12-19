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
#define DOWNLOAD_LIMIT 10

// OpenMPI data types used for communication
extern MPI_Datatype INQUIRY_T;
extern MPI_Datatype FILE_DATA_T;

// creates the data types used for communication
MPI_Datatype createInquiryType();

// creates the data types used for communication
MPI_Datatype createFileDataType();

// argument structure for the download thread
struct download_args_t{
    int rank;
    int *to_be_downloaded;
    int num; // total number of hosts

    // synchronization between upload and download threads
    pthread_mutex_t *lock;

    // fileName -> wanted_state(false/true)
    std::unordered_map<std::string, bool> wanted_files;
    // fileName -> vector<file_frags/hash>
    std::unordered_map<std::string, std::vector<std::string>> *partial_files; 

    
};

// argument structure for the upload thread
struct upload_args_t{
    int rank;
    int num; // total number of hosts

    // synchronization between upload and download threads
    pthread_mutex_t *lock;

    std::unordered_map<std::string, std::vector<std::string>> *full_files;
    std::unordered_map<std::string, std::vector<std::string>> *partial_files;

};

// data type used for communication between tracker and client
struct swarm_t {
    std::unordered_set<int> seeds; // client ranks with full file
    std::unordered_set<int> peers; // client ranks with fragments
    std::string fname;
    int seg_num;

    std::vector<std::string> f_hash; // hashes of fragments
};

// data type used for communication between clients
struct inquiry_t {
    int frag_idx; // wanted fragment index
    char fname[MAX_FILENAME];
    char hash[HASH_SIZE + 1]; // hash of wanted fragment
};

// data type used for initial communication between client and tracker
struct file_data_t {
    int num_files;
    char file_names[MAX_FILES][MAX_FILENAME];
    int num_frags[MAX_FILES];
    char hashes[MAX_FILES][MAX_CHUNKS][HASH_SIZE + 1];

};

// tags used for communication between entities
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
    TAG_DATA_SIZE,
    TAG_UPLOAD_CONFIRM,
};

// types of threads
enum ThreadType {
    DOWNLOAD,
    UPLOAD
};
