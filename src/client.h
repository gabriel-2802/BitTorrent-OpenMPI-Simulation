#pragma once
#include <sstream>

#include "entity.h"
#include "auxiliary.h"
#include "type.h"
#include "thread_functions.h"



class Client : public TorrentEntity {
public:
    Client(int numtasks, int rank);
    ~Client();
    void run() override;
    void printStringRepresentation() override;
private:
    pthread_t download_thread;
    pthread_t upload_thread;
    int to_be_downloaded; // count of files that must be downloaded

    std::unordered_set<std::string> wanted_files;
    //fileName -> vector<file_frags/hash>
    std::unordered_map<std::string, std::vector<std::string>> full_files;
    //fileName -> vector<file_frags/hash>
    std::unordered_map<std::string, std::vector<std::string>> to_be_downloaded_files;
    
    // synchronization
    pthread_mutex_t lock;

    
    void createThreads();

    void joinThreads();

    void *buildThreadArg(ThreadType type);

    void init();

    void announceTracker();

    // prints all received files
    void showFiles();
};
