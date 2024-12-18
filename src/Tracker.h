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
    int activePeers;
    std::unordered_map<std::string, swarm_t> fileSwarm; // fileName -> swarm_t
    std::vector<int> uploadPerClient;

    void collectInformation();
    void handleRequest(int src);
    void handleRequest(int src, COMMUNICATION_TAG req);

};
