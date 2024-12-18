#pragma once
#include "auxiliray.h"
#include "type.h"

void *download_t_func(void *arg);

void *upload_t_func(void *arg);

void download_fragment(download_args_t *arg, const swarm_t& swarm);

void download_check_file_completion(download_args_t *arg, swarm_t swarm, std::string file);

void upload_inquiry_handler(upload_args_t *arg, int src);

void upload_confirm_inquiry(upload_args_t *arg, const inquiry_t &inquiry, int &ack, std::string &hash);

