#include "Peer.h"

using namespace std;

Peer::Peer(int numtasks, int rank) : TorrentEntity(numtasks, rank) {
    readFileFrags();
    downloads = DOWNLOAD_LIMIT;
    busyLevel = 0;

    for (auto &file : wantedFiles) {
        downloadedFrags[file] = {};
    }

}

Peer::~Peer() {

}

void Peer::run() {
    announceToTracker();
    int ack;
    MPI_Bcast(&ack, 1, MPI_INT, TRACKER_RANK, MPI_COMM_WORLD);
    DIE(ack != 1, "Fatal Failure: Tracker did not acknowledge");

    createThreads();
    joinThreads();
    printDownloadedFrags();
}

void Peer::createThreads() {
    void *status;
    int r;

    r = pthread_create(&download_thread, NULL, downloadThreadFunc, buildThreadArg(DOWNLOAD));
    DIE(r, "download thread creation failed");
    
    r = pthread_create(&upload_thread, NULL, uploadThreadFunc, buildThreadArg(UPLOAD));
    DIE(r, "upload thread creation failed");
}

void Peer::joinThreads() {
    void *status;
    int r;

    r = pthread_join(download_thread, &status);
    DIE(r, "download thread join failed");

    r = pthread_join(upload_thread, &status);
    DIE(r, "upload thread join failed");
}

void *Peer::downloadThreadFunc(void *arg) {
	download_args_t *args  = (download_args_t *) arg;
    
}

void *Peer::uploadThreadFunc(void *arg)
{
    int rank = *(int*) arg;

    return NULL;
}

void *Peer::buildThreadArg(ThreadType type) {
    download_args_t *d_args = new download_args_t;
    upload_args_t *u_args = new upload_args_t;

    switch (type) {
        case DOWNLOAD:
            return (void *) d_args;
        case UPLOAD:
            return (void *) u_args;
    }
}

void Peer::readFileFrags() {
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

void Peer::announceToTracker() {
    int numFiles = fileFrags.size();

    MPI_Send(&numFiles, 1, MPI_INT, TRACKER_RANK, 0, MPI_COMM_WORLD);

    char fName[MAX_FILENAME], hash[HASH_SIZE + 1];
    memset(hash, 0, HASH_SIZE + 1);
    memset(fName, 0, MAX_FILENAME);

    for (auto [fileName, frags] : fileFrags) {
        
        memccpy(fName, fileName.c_str(), 1, MAX_FILENAME);
        MPI_Send(fName, MAX_FILENAME, MPI_CHAR, TRACKER_RANK, 0, MPI_COMM_WORLD);

        int numFrags = frags.size();
        MPI_Send(&numFrags, 1, MPI_INT, TRACKER_RANK, 0, MPI_COMM_WORLD);

        for (auto &frag : frags) {
            int idx = frag.first;
            MPI_Send(&idx, 1, MPI_INT, TRACKER_RANK, 0, MPI_COMM_WORLD);

            memcpy(hash, frag.second.c_str(), HASH_SIZE);
            MPI_Send(hash, HASH_SIZE + 1, MPI_CHAR, TRACKER_RANK, 0, MPI_COMM_WORLD);
        }
    }
}

void Peer::printDownloadedFrags() {
    for (auto [fname, fragInfo] : downloadedFrags) {
        ofstream fout(OUT_FILE(rank, fname));

        sort(fragInfo.begin(), fragInfo.end());

        for (auto &frag : fragInfo) {
            fout << frag.second << endl;
        }
    
    }
}

void Peer::debugPrint() {

}

