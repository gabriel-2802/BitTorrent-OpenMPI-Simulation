#pragma once

#include "Entity.h"
#include "auxiliray.h"



class Client : public TorrentEntity {
public:
    Client(int numtasks, int rank);
    ~Client();
    void run() override;
    void debugPrint() override;
private:
    pthread_t download_thread;
    pthread_t upload_thread;

    std::vector<std::string> wantedFiles;
    std::unordered_map<std::string, std::vector<f_frag_t>> fileFrags; //fileName -> vector<file_frags/hash>
    std::unordered_map<std::string, std::vector<f_frag_t>> downloadedFrags; //fileName -> vector<file_frags/hash>

    int downloads;
    int busyLevel; // +1 for every upload

    
    void createThreads();
    void joinThreads();
    static void *downloadThreadFunc(void *arg);
    static void *uploadThreadFunc(void *arg);
    void *buildThreadArg(ThreadType type);

    void readFileFrags();
    void announceToTracker();
    void printDownloadedFrags();



    

};