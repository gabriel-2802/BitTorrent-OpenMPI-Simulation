#include "client.h"

using namespace std;

Client::Client(int numtasks, int rank) : TorrentEntity(numtasks, rank) {
    init();
    to_be_downloaded = wanted_files.size();
    
    // initially all wanted files are empty
    for (auto &file : wanted_files) {
        to_be_downloaded_files[file] = {};
    }

    pthread_mutex_init(&lock, NULL);
}

Client::~Client() {
    pthread_mutex_destroy(&lock);
}

void Client::run() {
    announceTracker();
    // wait for tracker to acknowledge
    int ack;
    MPI_Bcast(&ack, 1, MPI_INT, TRACKER_RANK, MPI_COMM_WORLD);
    DIE(ack != 1, "Fatal Failure: Tracker did not acknowledge");

    createThreads();
    joinThreads();
    showFiles();
}

void Client::createThreads() {
    int r;

    r = pthread_create(&download_thread, NULL, downloadThread, buildThreadArg(DOWNLOAD));
    DIE(r, "download thread creation failed");
    
    r = pthread_create(&upload_thread, NULL, uploadThread, buildThreadArg(UPLOAD));
    DIE(r, "upload thread creation failed");
}

void Client::joinThreads() {
    void *status;
    int r;

    r = pthread_join(download_thread, &status);
    DIE(r, "download thread join failed");

    r = pthread_join(upload_thread, &status);
    DIE(r, "upload thread join failed");
}


void *Client::buildThreadArg(ThreadType type) {
    download_args_t *d_args = new download_args_t;
    upload_args_t *u_args = new upload_args_t;

    // build the argument structure for the threads
    switch (type) {
        case DOWNLOAD:
            d_args->rank = rank;
            d_args->num = numtasks;
            d_args->lock = &lock;
            d_args->to_be_downloaded = &to_be_downloaded;
            d_args->partial_files = &to_be_downloaded_files;
            for (auto &file : wanted_files) {
                d_args->wanted_files[file] = true;
            }
            return (void *) d_args;
        case UPLOAD:
            u_args->rank = rank;
            u_args->num = numtasks;
            u_args->lock = &lock;
            u_args->full_files = &full_files;
            u_args->partial_files = &to_be_downloaded_files;
            return (void *) u_args;
        default:
            throw runtime_error("Invalid thread type");
    }
}

void Client::init() {
    ifstream fin(IN_FILE(rank));
    DIE(!fin, "error opening file");

    // read the files
    int numFiles;
    fin >> numFiles;

    while (numFiles--) {
        string fileName;
        fin >> fileName;

        int numFrags;
        fin >> numFrags;

        while (numFrags--) {
            string frag;
            fin >> frag;

            full_files[fileName].push_back(frag);
        }
    }

    fin >> numFiles;
    while (numFiles--) {
        string fileName;
        fin >> fileName;

        wanted_files.insert(fileName);
    }

    fin.close();
}

void Client::announceTracker() {
    file_data_t data = {};
    data.num_files = full_files.size();

    // populate the file_data_t structure
    int file_index = 0;
    for (auto &[fileName, frags] : full_files) {
        strncpy(data.file_names[file_index], fileName.c_str(), MAX_FILENAME - 1);

        data.num_frags[file_index] = frags.size();

        int frag_index = 0;
        for (const auto &frag : frags) {
            strncpy(data.hashes[file_index][frag_index], frag.c_str(), HASH_SIZE);
            ++frag_index;
        }

        ++file_index;
    }

    // send it to the tracker all at once
    MPI_Send(&data, 1, FILE_DATA_T, TRACKER_RANK, TAG_INIT, MPI_COMM_WORLD);
}


void Client::showFiles() {
    for (auto [fname, fragInfo] : to_be_downloaded_files) {
        std::stringstream buffer;

        for (auto &frag : fragInfo) {
            buffer << frag << std::endl;
        }

        std::ofstream fout(OUT_FILE(rank, fname));
        fout << buffer.str();
        fout.close();
    }
}


void Client::printStringRepresentation() {
    for (auto &[fname, frags] : full_files) {
        cout << "File: " << fname << endl;
        cout << "Frags: ";
        for (auto &frag : frags) {
            cout << frag << " ";
        }
        cout << endl;
    }
}

