#include "classes.h"
#include <iostream>

using namespace std;

Tracker::Tracker(int numtasks, int rank) : TorrentEntity(numtasks, rank) {

}

Tracker::~Tracker() {

}

void Tracker::run() {
    
}

Peer::Peer(int numtasks, int rank) : TorrentEntity(numtasks, rank) {

}

Peer::~Peer() {

}

void Peer::run() {
    void *status;
    int r;

    r = pthread_create(&download_thread, NULL, download_thread_func, (void *) &rank);
    DIE(r, "Eroare la crearea thread-ului de download");
    
    r = pthread_create(&upload_thread, NULL, upload_thread_func, (void *) &rank);
    DIE(r, "Eroare la crearea thread-ului de upload");

    r = pthread_join(download_thread, &status);
    DIE(r, "Eroare la asteptarea thread-ului de download");

    r = pthread_join(upload_thread, &status);
    DIE(r, "Eroare la asteptarea thread-ului de upload");

}

void *Peer::download_thread_func(void *arg)
{
    int rank = *(int*) arg;

    return NULL;
}

void *Peer::upload_thread_func(void *arg)
{
    int rank = *(int*) arg;

    return NULL;
}


