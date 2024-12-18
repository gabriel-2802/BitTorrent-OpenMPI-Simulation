#pragma once
#include "Entity.h"
#include "auxiliray.h"


class Tracker : public TorrentEntity {
public:
    Tracker(int numtasks, int rank);
    ~Tracker();
    void run() override;
    void debugPrint() override;

private:
    // std::unordered_map<std::string, file_info_t> swarms; // fileName -> file_info_t
    int activePeers;
    std::unordered_map<std::string, swarm_t> fileSwarm; // fileName -> swarm_t

    void collectInformation();
    void handleRequest(int src);
    void handleRequest(int src, REQUEST_TYPE req);

};
