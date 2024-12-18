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
    std::unordered_map<std::string, file_info_t> swarms; // fileName -> file_info_t
    int activePeers;

    void collectInformation();

};
