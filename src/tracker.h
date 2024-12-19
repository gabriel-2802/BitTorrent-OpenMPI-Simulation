#pragma once
#include "entity.h"
#include "auxiliary.h"
#include "type.h"


class Tracker : public TorrentEntity {
public:
    Tracker(int numtasks, int rank);
    ~Tracker();
    void run() override;
    void debugPrint() override;

private:
    int active_clients;
    std::unordered_map<std::string, swarm_t> file_swarms; // fileName -> swarm_t
    std::vector<int> upload_per_client;

    void collectInformation();
    void handleRequest(int src);
    void handleRequest(int src, COMMUNICATION_TAG req);

};
