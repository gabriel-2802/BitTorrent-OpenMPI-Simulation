#include "client.h"

using namespace std;

Client::Client(int numtasks, int rank) : TorrentEntity(numtasks, rank) {
    readFileFrags();
    to_be_downloaded = wanted_files.size();
    

    for (auto &file : wanted_files) {
        to_be_downloaded_files[file] = {};
    }

    pthread_mutex_init(&lock, NULL);

}

Client::~Client() {
    pthread_mutex_destroy(&lock);
}

void Client::run() {
    // debugPrint();
    announceTracker();
    int ack;
    MPI_Bcast(&ack, 1, MPI_INT, TRACKER_RANK, MPI_COMM_WORLD);
    DIE(ack != 1, "Fatal Failure: Tracker did not acknowledge");

    createThreads();
    joinThreads();
    printDownloadedFrags();
}

void Client::createThreads() {
    int r;

    r = pthread_create(&download_thread, NULL, download_t_func, buildThreadArg(DOWNLOAD));
    DIE(r, "download thread creation failed");
    
    r = pthread_create(&upload_thread, NULL, upload_t_func, buildThreadArg(UPLOAD));
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
            return NULL;   
    }
}

void Client::readFileFrags() {
    ifstream fin(IN_FILE(rank));
    DIE(!fin, "error opening file");

    
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
    int numFiles = full_files.size();

    MPI_Send(&numFiles, 1, MPI_INT, TRACKER_RANK, TAG_INIT, MPI_COMM_WORLD);


    for (auto [fileName, frags] : full_files) {
        
        char fName[MAX_FILENAME];
        memset(fName, 0, MAX_FILENAME);
        strcpy(fName, fileName.c_str());
        MPI_Send(fName, MAX_FILENAME, MPI_CHAR, TRACKER_RANK, TAG_INIT, MPI_COMM_WORLD);

        int numFrags = frags.size();
        MPI_Send(&numFrags, 1, MPI_INT, TRACKER_RANK, TAG_INIT, MPI_COMM_WORLD);

        for (auto &frag : frags) {
            char hash[HASH_SIZE + 1];
            memset(hash, 0, HASH_SIZE + 1);
            strcpy(hash, frag.c_str());
            MPI_Send(hash, HASH_SIZE + 1, MPI_CHAR, TRACKER_RANK, TAG_INIT, MPI_COMM_WORLD);
        }
    }
}

void Client::printDownloadedFrags() {
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


void Client::debugPrint() {
    for (auto &[fname, frags] : full_files) {
        cout << "File: " << fname << endl;
        cout << "Frags: ";
        for (auto &frag : frags) {
            cout << frag << " ";
        }
        cout << endl;
    }
}

