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

    // Total number of blocks (elements)
    const int nitems = 4;

    // Array of block lengths
    int block_lengths[nitems] = {
        1,                                   // num_files
        MAX_FILES * MAX_FILENAME,           // file_names
        MAX_FILES,                          // num_frags
        MAX_FILES * MAX_CHUNKS * (HASH_SIZE + 1) // hashes
    };

    // Array of displacements
    MPI_Aint offsets[nitems];
    offsets[0] = offsetof(file_data_t, num_files);      // num_files
    offsets[1] = offsetof(file_data_t, file_names);     // file_names
    offsets[2] = offsetof(file_data_t, num_frags);      // num_frags
    offsets[3] = offsetof(file_data_t, hashes);         // hashes

    // Array of types
    MPI_Datatype types[nitems] = {
        MPI_INT,     // num_files
        MPI_CHAR,    // file_names
        MPI_INT,     // num_frags
        MPI_CHAR     // hashes
    };

    // Create the MPI struct type
    MPI_Datatype file_data_type;
    MPI_Type_create_struct(nitems, block_lengths, offsets, types, &file_data_type);
    MPI_Type_commit(&file_data_type);

    return file_data_type;
}