#pragma once
#include "entity.h"
#include "auxiliary.h"
#include "type.h"


class Tracker : public TorrentEntity {
public:
    Tracker(int numtasks, int rank);
    ~Tracker();
    void run() override;
    void printStringRepresentation() override;

private:
    int active_clients;
    std::unordered_map<std::string, swarm_t> file_swarms; // fileName -> swarm_t
    std::vector<int> upload_per_client;

    // used for the tracker to collect information from the clients at the beginning
    void collectInformation();

    // handles a swarm request from a client
    void handleRequest(int src);

    // handles all types of requests from the clients
    void handleRequest(int src, COMMUNICATION_TAG req);

};
