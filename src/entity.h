#pragma once
#include "auxiliary.h"


// abstract class
class TorrentEntity {
public:
    TorrentEntity(int numtasks, int rank) : numtasks(numtasks), rank(rank) {};
    virtual ~TorrentEntity() {};
    virtual void run() {};
    virtual void debugPrint() {};

protected:
    int numtasks;
    int rank;
};


