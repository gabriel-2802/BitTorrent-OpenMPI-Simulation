#include "auxiliray.h"
#include "Entity.h"
#include "Client.h"
#include "Tracker.h"

using namespace std;

int main (int argc, char *argv[]) {
    int numtasks, rank;
 
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE) {
        cerr << "MPI nu are suport pentru multi-threading\n";
        exit(-1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    TorrentEntity *entity;

    if (rank == TRACKER_RANK) {
        entity = new Tracker(numtasks, rank);
    } else {
        entity = new Client(numtasks, rank);
    } 

    cout << numtasks << " " << rank << "\n";

    entity->run();
    delete entity;

    MPI_Finalize();
}
