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
#define DOWNLOAD_LIMIT 10


void sendSwarm(const swarm_t &swarm, int dest);

void receiveSwarm(swarm_t &swarm, int src);

std::string serializeSwarm(const swarm_t &swarm);

swarm_t deserializeSwarm(const std::string &data);

char* createBuffer(size_t size, std::string src);

bool checkDataIntegrity(const std::string &data, const std::string &hash);
