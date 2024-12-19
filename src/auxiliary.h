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

#include "type.h"

#define DIE(assertion, call_description)            \
    do {                                            \
        if (assertion) {                            \
            std::cerr << call_description << "\n";  \
            std::exit(errno);                       \
        }                                           \
    } while (0)
#define IN_FILE(rank) ("in" + std::to_string(rank) + ".txt")
#define OUT_FILE(rank, name) ("client" + std::to_string(rank) + "_" + name) 


// sends a swarm_T to dest
void sendSwarm(const swarm_t &swarm, int dest);

// receves a swarm_t from src
void receiveSwarm(swarm_t &swarm, int src);

// serializes the data structure into a buffer and sends it whole to dest
std::string serializeSwarm(const swarm_t &swarm);

// deserializes the buffer into a data structure
swarm_t deserializeSwarm(const std::string &data);

// utility function to create a buffer from a string (delete the buffer after use)
char* createBuffer(size_t size, std::string src);

// utility function to checks if the data integrity is preserved
bool checkDataIntegrity(const std::string &data, const std::string &hash);
