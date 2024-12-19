#include "type.h"

MPI_Datatype createInquiryType() {
    MPI_Datatype INQUIRY_T;

    const int nItems = 3;

    int blockLengths[nItems] = {
        1,                  
        MAX_FILENAME,         
        HASH_SIZE + 1        
    };

    MPI_Datatype types[nItems] = {
        MPI_INT,              
        MPI_CHAR,             
        MPI_CHAR              
    };

    MPI_Aint offsets[nItems];
    offsets[0] = offsetof(inquiry_t, frag_idx);
    offsets[1] = offsetof(inquiry_t, fname);
    offsets[2] = offsetof(inquiry_t, hash);


    MPI_Type_create_struct(nItems, blockLengths, offsets, types, &INQUIRY_T);
    MPI_Type_commit(&INQUIRY_T);

    return INQUIRY_T;
}

MPI_Datatype createFileDataType() {
    const int nitems = 4;

    int block_lengths[nitems] = {
        1,                                  
        MAX_FILES * MAX_FILENAME,          
        MAX_FILES,                        
        MAX_FILES * MAX_CHUNKS * (HASH_SIZE + 1)
    };

    MPI_Aint offsets[nitems];
    offsets[0] = offsetof(file_data_t, num_files);
    offsets[1] = offsetof(file_data_t, file_names);  
    offsets[2] = offsetof(file_data_t, num_frags);    
    offsets[3] = offsetof(file_data_t, hashes);       

    MPI_Datatype types[nitems] = {
        MPI_INT,   
        MPI_CHAR,  
        MPI_INT, 
        MPI_CHAR   
    };

    MPI_Datatype FILE_DATA_T;
    MPI_Type_create_struct(nitems, block_lengths, offsets, types, &FILE_DATA_T);
    MPI_Type_commit(&FILE_DATA_T);

    return FILE_DATA_T;
}