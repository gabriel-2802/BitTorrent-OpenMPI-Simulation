#pragma once
#include <unistd.h>

#include "auxiliary.h"
#include "type.h"

// function used by the client to download files
void *downloadThread(void *arg);

// function used by the client to upload files
void *uploadThread(void *arg);

// downloads a single fragment from a peer using the swarm information
void downloadFragment(download_args_t *arg, const swarm_t& swarm);

// checks if the file is complete according to the swarm information
bool checkFileCompletion(download_args_t *arg, swarm_t swarm, std::string file);

// handles an inquiry from a peer regarding a file fragment
void uploadInquiryHandler(upload_args_t *arg, int src);

// checks if the requested fragment is available and sets the ack and hash accordingly
void uploadConfirmation(upload_args_t *arg, const inquiry_t &inquiry, int &ack, std::string &hash);

