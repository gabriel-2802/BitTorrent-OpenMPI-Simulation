#include "auxiliary.h"

using namespace std;

void send_swarm(const swarm_t &swarm, int dest) {
    std::string data = serialize_swarm(swarm);
    int size = data.size();

    MPI_Send(&size, 1, MPI_INT, dest, TAG_DATA_SIZE, MPI_COMM_WORLD);
    MPI_Send(data.data(), size, MPI_CHAR, dest, TAG_DATA, MPI_COMM_WORLD);
}

void receive_swarm(swarm_t &swarm, int src) {
    int size;
    MPI_Recv(&size, 1, MPI_INT, src, TAG_DATA_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Allocate memory for incoming data
    char *data = new char[size];
    if (!data) {
        throw std::bad_alloc();
    }

    MPI_Recv(data, size, MPI_CHAR, src, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    swarm = deserialize_swarm(std::string(data, size));
    

    delete[] data;
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

std::string serialize_swarm(const swarm_t &swarm) {
    // Calculate the total size needed for serialization
    size_t total_size = sizeof(int) + // seeds size
                        sizeof(int) * swarm.seeds.size() +
                        sizeof(int) + // peers size
                        sizeof(int) * swarm.peers.size() +
                        MAX_FILENAME + // filename
                        sizeof(int) + // segment number
                        swarm.f_hash.size() * (HASH_SIZE + 1);

    // Allocate a contiguous buffer
    char *data = (char *)malloc(total_size);
    if (!data) {
        throw std::bad_alloc();
    }

    char *ptr = data;

    // Serialize seeds
    int ss = swarm.seeds.size();
    memcpy(ptr, &ss, sizeof(int));
    ptr += sizeof(int);
    for (int seed : swarm.seeds) {
        memcpy(ptr, &seed, sizeof(int));
        ptr += sizeof(int);
    }

    // Serialize peers
    int ps = swarm.peers.size();
    memcpy(ptr, &ps, sizeof(int));
    ptr += sizeof(int);
    for (int peer : swarm.peers) {
        memcpy(ptr, &peer, sizeof(int));
        ptr += sizeof(int);
    }

    // Serialize filename
    char fname[MAX_FILENAME] = {0};
    strncpy(fname, swarm.fname.c_str(), MAX_FILENAME - 1); // Ensure null-termination
    memcpy(ptr, fname, MAX_FILENAME);
    ptr += MAX_FILENAME;

    // Serialize segment number
    int segNum = swarm.seg_num;
    memcpy(ptr, &segNum, sizeof(int));
    ptr += sizeof(int);

    // Serialize hashes
    for (const std::string &frag : swarm.f_hash) {
        char hash[HASH_SIZE + 1] = {0};
        strncpy(hash, frag.c_str(), HASH_SIZE); // Truncate to HASH_SIZE
        memcpy(ptr, hash, HASH_SIZE + 1);
        ptr += HASH_SIZE + 1;
    }

    // Create a string from the buffer
    std::string res(data, total_size);
    free(data); // Free the allocated memory
    return res;
}


swarm_t deserialize_swarm(const std::string &data) {
    const char *data_ptr = data.data(); // Use pointer to raw data
    swarm_t swarm;

    // Deserialize seeds
    int seeds_size;
    memcpy(&seeds_size, data_ptr, sizeof(int));
    data_ptr += sizeof(int);
    for (int i = 0; i < seeds_size; ++i) {
        int seed;
        memcpy(&seed, data_ptr, sizeof(int));
        data_ptr += sizeof(int);
        swarm.seeds.insert(seed);
    }

    // Deserialize peers
    int peers_size;
    memcpy(&peers_size, data_ptr, sizeof(int));
    data_ptr += sizeof(int);
    for (int i = 0; i < peers_size; ++i) {
        int peer;
        memcpy(&peer, data_ptr, sizeof(int));
        data_ptr += sizeof(int);
        swarm.peers.insert(peer);
    }

    // Deserialize filename
    char fname[MAX_FILENAME] = {0};
    memcpy(fname, data_ptr, MAX_FILENAME);
    data_ptr += MAX_FILENAME;
    swarm.fname = fname;

    // Deserialize segment number
    memcpy(&swarm.seg_num, data_ptr, sizeof(int));
    data_ptr += sizeof(int);

    // Deserialize hashes
    for (int i = 0; i < swarm.seg_num; ++i) {
        char hash[HASH_SIZE + 1] = {0};
        memcpy(hash, data_ptr, HASH_SIZE + 1);
        data_ptr += HASH_SIZE + 1;
        swarm.f_hash.push_back(hash);
    }

    return swarm;
}
