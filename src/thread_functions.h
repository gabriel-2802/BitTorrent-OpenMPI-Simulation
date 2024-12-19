#pragma once
#include "auxiliary.h"
#include "type.h"

void *downloadThread(void *arg);

void *uploadThread(void *arg);

void downloadFragment(download_args_t *arg, const swarm_t& swarm);

void downloadCheckFileCompletion(download_args_t *arg, swarm_t swarm, std::string file);

void uploadInquiryHandler(upload_args_t *arg, int src);

void uploadConfirmation(upload_args_t *arg, const inquiry_t &inquiry, int &ack, std::string &hash);

