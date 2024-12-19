#include "auxiliary.h"
#include "entity.h"
#include "client.h"
#include "tracker.h"
#include "type.h"

using namespace std;

MPI_Datatype INQUIRY_T;

int main (int argc, char *argv[]) {
    int numtasks, rank;
 
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE) {
        cerr << "MPI nu are suport pentru multi-threading\n";
        exit(-1);
    }

    INQUIRY_T = createInquiryType();

    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    TorrentEntity *entity;

    if (rank == TRACKER_RANK) {
        entity = new Tracker(numtasks, rank);
    } else {
        entity = new Client(numtasks, rank);
    } 

    entity->run();
    delete entity;
    
    MPI_Type_free(&INQUIRY_T);

    MPI_Finalize();
}
