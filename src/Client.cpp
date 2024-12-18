#include "Client.h"

using namespace std;

Client::Client(int numtasks, int rank) : TorrentEntity(numtasks, rank) {
    readFileFrags();
    downloads = DOWNLOAD_LIMIT;
    busyLevel = 0;

    for (auto &file : wantedFiles) {
        downloadedFrags[file] = {};
    }

}

Client::~Client() {

}

void Client::run() {
    announceToTracker();
    int ack;
    MPI_Bcast(&ack, 1, MPI_INT, TRACKER_RANK, MPI_COMM_WORLD);
    DIE(ack != 1, "Fatal Failure: Tracker did not acknowledge");

    createThreads();
    joinThreads();
    printDownloadedFrags();
}

void Client::createThreads() {
    void *status;
    int r;

    r = pthread_create(&download_thread, NULL, downloadThreadFunc, buildThreadArg(DOWNLOAD));
    DIE(r, "download thread creation failed");
    
    r = pthread_create(&upload_thread, NULL, uploadThreadFunc, buildThreadArg(UPLOAD));
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

void *Client::downloadThreadFunc(void *arg) {
	download_args_t *args  = (download_args_t *) arg;

    
    for (auto &file : *args->wantedFiles) {
        // send request to tracker of type REQUEST to know the next message will be the file name
        MPI_Send(nullptr, 0, MPI_INT, TRACKER_RANK, SWARM_REQUEST, MPI_COMM_WORLD);

        char fName[MAX_FILENAME];
        memset(fName, 0, MAX_FILENAME);
        strcpy(fName, file.c_str());

        MPI_Send(fName, MAX_FILENAME, MPI_CHAR, TRACKER_RANK, SWARM_REQUEST, MPI_COMM_WORLD);
        
        swarm_t fSwarm;
        receive_swarm(fSwarm, TRACKER_RANK);

       
        
        // consider this complete
        // download frag

        auto it = args->downloadedFrags->find(file);
        auto frags = it->second;
        
        if (frags.size() == fSwarm.segNum) {
            MPI_Send(nullptr, 0, MPI_INT, TRACKER_RANK, FINALISED_FILE_REQUEST, MPI_COMM_WORLD);
            MPI_Send(fName, MAX_FILENAME, MPI_CHAR, TRACKER_RANK, FINALISED_FILE_REQUEST, MPI_COMM_WORLD);
        }
    }

    // client ended downloading all files
    MPI_Send(nullptr, 0, MPI_INT, TRACKER_RANK, FINALISED_CLIENT_REQUEST, MPI_COMM_WORLD);

    pthread_exit(NULL);
}

void *Client::uploadThreadFunc(void *arg)
{
    int rank = *(int*) arg;

    return NULL;
}

void *Client::buildThreadArg(ThreadType type) {
    download_args_t *d_args = new download_args_t;
    upload_args_t *u_args = new upload_args_t;

    switch (type) {
        case DOWNLOAD:
            d_args->rank = rank;
            d_args->downloads = &downloads;
            d_args->wantedFiles = &wantedFiles;
            d_args->downloadedFrags = &downloadedFrags;
            return (void *) d_args;
        case UPLOAD:
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

        int numFrags, idx = 0;
        fin >> numFrags;

        while (numFrags--) {
            string frag;
            fin >> frag;

            fileFrags[fileName].push_back({idx++, frag});
        }
    }

    fin >> numFiles;
    while (numFiles--) {
        string fileName;
        fin >> fileName;

        wantedFiles.push_back(fileName);
    }

    fin.close();
}

void Client::announceToTracker() {
    int numFiles = fileFrags.size();

    MPI_Send(&numFiles, 1, MPI_INT, TRACKER_RANK, 0, MPI_COMM_WORLD);


    for (auto [fileName, frags] : fileFrags) {
        
        char fName[MAX_FILENAME];
        memset(fName, 0, MAX_FILENAME);
        strcpy(fName, fileName.c_str());
        MPI_Send(fName, MAX_FILENAME, MPI_CHAR, TRACKER_RANK, 0, MPI_COMM_WORLD);

        int numFrags = frags.size();
        MPI_Send(&numFrags, 1, MPI_INT, TRACKER_RANK, 0, MPI_COMM_WORLD);

        for (auto &frag : frags) {
            char hash[HASH_SIZE + 1];
            memset(hash, 0, HASH_SIZE + 1);
            strcpy(hash, frag.second.c_str());
            MPI_Send(hash, HASH_SIZE + 1, MPI_CHAR, TRACKER_RANK, 0, MPI_COMM_WORLD);
        }
    }
}

void Client::printDownloadedFrags() {
    for (auto [fname, fragInfo] : downloadedFrags) {
        ofstream fout(OUT_FILE(rank, fname));

        sort(fragInfo.begin(), fragInfo.end());

        for (auto &frag : fragInfo) {
            fout << frag.second << endl;
        }
    
    }
}

void Client::debugPrint() {

}

