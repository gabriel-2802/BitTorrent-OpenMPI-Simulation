#include "thread_functions.h"

using namespace std;

void *downloadThread(void *arg) {
	download_args_t *args  = (download_args_t *) arg;

    while (true) {
        // for all files that the client wants to download
        for (auto &[file, wanted] : args->wanted_files) {
            if (!wanted) {
                continue;
            }

            // establish connection with the tracker
            char *fname = createBuffer(MAX_FILENAME, file);
            MPI_Send(fname, MAX_FILENAME, MPI_CHAR, TRACKER_RANK, TAG_PROBING, MPI_COMM_WORLD);
            
            // receive the swarm information from the tracker
            swarm_t fswarm;
            receiveSwarm(fswarm, TRACKER_RANK);
            bool done = false;

            // get DOWNLOAD_LIMIT fragments, then go to the next file
            // this ensures that the client downloads from multiple sources
            for (int d = 0; d < DOWNLOAD_LIMIT && !done; ++d) {
                downloadFragment(args, fswarm);
                done = checkFileCompletion(args, fswarm, file);
            }

            delete[] fname;
        }

        // no more files to download for this client, exit
        if (*(args->to_be_downloaded) == 0)
            break;
    }

    // client ended downloading all its files => signal the tracker
    MPI_Send(nullptr, 0, MPI_INT, TRACKER_RANK, TAG_CLIENT_DONE, MPI_COMM_WORLD);
    pthread_exit(NULL);
}

void downloadFragment(download_args_t *arg, const swarm_t& swarm) {
    // all possible sources for the fragment => seeds + peers
    unordered_set<int> all;
    all.insert(swarm.seeds.begin(), swarm.seeds.end());
    all.insert(swarm.peers.begin(), swarm.peers.end());

    // the next fragment to be downloaded
    int wanted_frag = arg->partial_files->find(swarm.fname)->second.size();

    // asks for the level of busyness of all the clients from the tracker
    int busy_lvls[arg->num];
    MPI_Send(nullptr, 0, MPI_INT, TRACKER_RANK, TAG_BUSSYNESS, MPI_COMM_WORLD);
    MPI_Recv(busy_lvls, arg->num, MPI_INT, TRACKER_RANK, TAG_BUSSYNESS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    // sort all clients by their busyness level to choose the most suitable one
    vector<pair<int, int>> srcs; // <busyLevel, src>
    for (auto &src : all) {
        srcs.push_back({busy_lvls[src], src});
    }
    sort(srcs.begin(), srcs.end());

    // check if the requested fragment is available
    for (auto &[busy, src] : srcs) {
        inquiry_t inquiry = {};
        inquiry.frag_idx = wanted_frag;
        memcpy(inquiry.fname, swarm.fname.c_str(), swarm.fname.size());

        // if the client is not a peer or a seed, continue
        if (swarm.peers.find(src) == swarm.peers.end() 
                && swarm.seeds.find(src) == swarm.seeds.end())
            continue;
            
        // send the inquiry to the client
        MPI_Send(&inquiry, 1, INQUIRY_T, src, TAG_INQUIRY, MPI_COMM_WORLD);

        // receive the ack from the client
        int ack;
        MPI_Recv(&ack, 1, MPI_INT, src, TAG_INQUIRY_ACK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // if the client has the fragment, `receive it` and end the loop
        if (ack) {
            char *buff = createBuffer(HASH_SIZE + 1, "");
            MPI_Recv(buff, HASH_SIZE + 1, MPI_CHAR, src, TAG_INQUIRY_RESPONSE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // ensure the data integrity
            if (!checkDataIntegrity(string(buff), swarm.f_hash[wanted_frag])) {
                break;
            }
            
            // announce the tracker that this client downloaded a fragment
            MPI_Send(inquiry.fname, MAX_FILENAME, MPI_CHAR, TRACKER_RANK, TAG_SEG_DONE, MPI_COMM_WORLD);

            string hash = buff;

            // ensure an upload thread doesn't read while writing
            pthread_mutex_lock(arg->lock);
            arg->partial_files->find(swarm.fname)->second.push_back(hash);
            pthread_mutex_unlock(arg->lock);

            delete[] buff;
            break;
        }
    }
}

bool checkFileCompletion(download_args_t *arg, swarm_t swarm, string file) {
    // if the number of owned fragments is equal to the total number of fragments
    if ((int)arg->partial_files->find(file)->second.size() == swarm.seg_num) {
        // a file is fully downloaded => decrease the counter
        --(*(arg->to_be_downloaded)); 

        // announce the tracker that this client is now a seed
        char *fname = createBuffer(MAX_FILENAME, file);
        MPI_Send(fname, MAX_FILENAME, MPI_CHAR, TRACKER_RANK, TAG_FILE_DONE, MPI_COMM_WORLD);

        // mark the file as downloaded
        arg->wanted_files[file] = false;
        delete [] fname;
        return true;
    }

    return false;
}

void *uploadThread(void *arg) {
    upload_args_t *args = (upload_args_t *) arg;
    int src;

    while (true) {
        MPI_Status status;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        COMMUNICATION_TAG tag = (COMMUNICATION_TAG) status.MPI_TAG;

        // run until signal to kill is received
        switch (tag) {
            case TAG_KILL_ALL:
                pthread_exit(NULL);
            case TAG_INQUIRY:
                src = status.MPI_SOURCE;
                uploadInquiryHandler(args, src);
                break;
            default:
                break;
        }
    }
}

void uploadInquiryHandler(upload_args_t *argm, int src) {
    inquiry_t inquiry;
    memset(inquiry.fname, 0, MAX_FILENAME);

    // receive the inquiry from the client
    MPI_Recv(&inquiry, 1, INQUIRY_T, src, TAG_INQUIRY, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // check if the client has the requested fragment
    int ack = 0;
    string hash = "random_hash";
    uploadConfirmation(argm, inquiry, ack, hash);

    // send the response ACK to the client
    MPI_Send(&ack, 1, MPI_INT, src, TAG_INQUIRY_ACK, MPI_COMM_WORLD);

    // if the client has the fragment, send it
    if (ack) {
        char *buff = createBuffer(HASH_SIZE + 1, hash);
        
        // send fragment hash to the client
        MPI_Send(buff, HASH_SIZE + 1, MPI_CHAR, src, TAG_INQUIRY_RESPONSE, MPI_COMM_WORLD);

        // send the upload confirmation to the tracker to balance the busyness
        MPI_Send(nullptr, 0, MPI_INT, TRACKER_RANK, TAG_UPLOAD_CONFIRM, MPI_COMM_WORLD);

        delete[] buff;
    }
}

void uploadConfirmation(upload_args_t *arg, const inquiry_t &inquiry, int &ack, string &hash) {
    string file = string(inquiry.fname);
    int frag_idx = inquiry.frag_idx;

    // if the requested file is fully downloaded, the client has the fragment
    if (arg->full_files->find(file) != arg->full_files->end()) {
        ack = 1;
        hash = arg->full_files->find(file)->second[frag_idx];
        return;
    }

    // check if the requested fragment is available in the partial files
    // lock the mutex to ensure no other thread writes while reading
    pthread_mutex_lock(arg->lock);
    if (arg->partial_files->find(file) != arg->partial_files->end() &&
            (int)arg->partial_files->find(file)->second.size() > frag_idx) {
        ack = 1;
        hash = arg->partial_files->find(file)->second[frag_idx];
    }
    pthread_mutex_unlock(arg->lock);
}
