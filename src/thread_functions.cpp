#include "thread_functions.h"

using namespace std;

void *download_t_func(void *arg) {
	download_args_t *args  = (download_args_t *) arg;

    while (true) {
        for (auto &[file, wanted] : args->wanted_files) {
            if (!wanted) {
                continue;
            }

            char fName[MAX_FILENAME];
            memset(fName, 0, MAX_FILENAME);
            strcpy(fName, file.c_str());
            MPI_Send(fName, MAX_FILENAME, MPI_CHAR, TRACKER_RANK, TAG_PROBING, MPI_COMM_WORLD);
            
            swarm_t fSwarm;
            receive_swarm(fSwarm, TRACKER_RANK);


            // // download a fragment
            download_fragment(args, fSwarm);
            // // if the file is completed, notify the tracker
            download_check_file_completion(args, fSwarm, file);
        }

        // no more files to download
        if (*(args->to_be_downloaded) == 0)
            break;
    }

    // client ended downloading all its files
    MPI_Send(nullptr, 0, MPI_INT, TRACKER_RANK, TAG_CLIENT_DONE, MPI_COMM_WORLD);

    pthread_exit(NULL);
}

void download_fragment(download_args_t *arg, const swarm_t& swarm) {
    unordered_set<int> all;
    all.insert(swarm.seeds.begin(), swarm.seeds.end());
    all.insert(swarm.peers.begin(), swarm.peers.end());

    // the next fragment to be downloaded
    int wanted_frag = arg->partial_files->find(swarm.fname)->second.size();

    // asks for the level of busyness of all the clients from the tracker
    int busy_lvls[arg->num];
    MPI_Send(nullptr, 0, MPI_INT, TRACKER_RANK, TAG_BUSSYNESS, MPI_COMM_WORLD);
    MPI_Recv(busy_lvls, arg->num, MPI_INT, TRACKER_RANK, TAG_BUSSYNESS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    vector<pair<int, int>> srcs; // busyLevel, src
    for (auto &src : all) {
        srcs.push_back({busy_lvls[src], src});
    }

    sort(srcs.begin(), srcs.end());

    for (auto &[busy, src] : srcs) {
        inquiry_t inquiry;
        inquiry.frag_idx = wanted_frag;
        memset(inquiry.fileName, 0, MAX_FILENAME);
        memcpy(inquiry.fileName, swarm.fname.c_str(), swarm.fname.size());
        memset(inquiry.hash, 0, HASH_SIZE + 1);

        MPI_Send(&inquiry, 1, INQUIRY_T, src, TAG_INQUIRY, MPI_COMM_WORLD);


        int ack;
        MPI_Recv(&ack, 1, MPI_INT, src, TAG_INQUIRY_ACK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        if (ack) {
            char buff[HASH_SIZE + 1];
            memset(buff, 0, HASH_SIZE + 1);
            MPI_Recv(buff, HASH_SIZE + 1, MPI_CHAR, src, TAG_INQUIRY_RESPONSE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            string hash = buff;
            arg->partial_files->find(swarm.fname)->second.push_back(hash);
            break;
        }
    }
}

void download_check_file_completion(download_args_t *arg, swarm_t swarm, string file) {
    if (arg->partial_files->find(file)->second.size() == swarm.seg_num) {
        /*  *(arg->to_be_downloaded)--; is wrong. it was discovered after 2 hours of debugging,
        i despise c/c++ with passion */
        --(*(arg->to_be_downloaded)); 


        char fname[MAX_FILENAME];
        memset(fname, 0, MAX_FILENAME);
        strcpy(fname, file.c_str());

        MPI_Send(fname, MAX_FILENAME, MPI_CHAR, TRACKER_RANK, TAG_FILE_DONE, MPI_COMM_WORLD);
        arg->wanted_files[file] = false;
    }
}

void *upload_t_func(void *arg)
{
    upload_args_t *args = (upload_args_t *) arg;
    int src;

    while (true) {
        MPI_Status status;

        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        COMMUNICATION_TAG tag = (COMMUNICATION_TAG) status.MPI_TAG;

        switch (tag) {
            case TAG_KILL_ALL:
                pthread_exit(NULL);
            case TAG_INQUIRY:
                src = status.MPI_SOURCE;
                upload_inquiry_handler(args, src);
                break;
            default:
                break;
        }

    }
}

void upload_inquiry_handler(upload_args_t *argm, int src) {
    inquiry_t inquiry;
    memset(inquiry.fileName, 0, MAX_FILENAME);

    MPI_Recv(&inquiry, 1, INQUIRY_T, src, TAG_INQUIRY, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // inquiry_t inquiry;
    // receive_inquiry(inquiry, src);

    int ack = 0;
    string hash = "random_hash";
    upload_confirm_inquiry(argm, inquiry, ack, hash);

    

    MPI_Send(&ack, 1, MPI_INT, src, TAG_INQUIRY_ACK, MPI_COMM_WORLD);
    if (ack) {
        char buff[HASH_SIZE + 1];
        memset(buff, 0, HASH_SIZE + 1);
        strcpy(buff, hash.c_str());
        MPI_Send(buff, HASH_SIZE + 1, MPI_CHAR, src, TAG_INQUIRY_RESPONSE, MPI_COMM_WORLD);
    }
}

void upload_confirm_inquiry(upload_args_t *arg, const inquiry_t &inquiry, int &ack, string &hash) {
    string file = string(inquiry.fileName);
    int frag_idx = inquiry.frag_idx;

    if (arg->full_files->find(file) != arg->full_files->end()) {
        ack = 1;
        hash = arg->full_files->find(file)->second[frag_idx];
        return;
    }

    if (arg->partial_files->find(file) != arg->partial_files->end() && arg->partial_files->find(file)->second.size() > frag_idx) {
        ack = 1;
        hash = arg->partial_files->find(file)->second[frag_idx];
        return;
    }
}
