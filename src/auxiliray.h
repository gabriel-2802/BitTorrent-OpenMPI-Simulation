#pragma once
#include <mpi.h>
#include <pthread.h>

#define TRACKER_RANK 0
#define MAX_FILES 10
#define MAX_FILENAME 15
#define HASH_SIZE 32
#define MAX_CHUNKS 100

#define DIE(assertion, call_description) \
    do { \
        if (assertion) { \
            std::cerr << call_description << "\n"; \
            std::exit(errno); \
        } \
    } while (0)
