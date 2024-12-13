#pragma once
#include "auxiliray.h"

// abstract class
class TorrentEntity {
public:
    TorrentEntity(int numtasks, int rank) : numtasks(numtasks), rank(rank) {};
    virtual ~TorrentEntity() {};
    virtual void run() {};

protected:
    int numtasks;
    int rank;
};

class Tracker : public TorrentEntity {
public:
    Tracker(int numtasks, int rank);
    ~Tracker();
    void run() override;

};

class Peer : public TorrentEntity {
public:
    Peer(int numtasks, int rank);
    ~Peer();
    void run() override;
private:
    pthread_t download_thread;
    pthread_t upload_thread;
    
    static void *download_thread_func(void *arg);
    static void *upload_thread_func(void *arg);

};
