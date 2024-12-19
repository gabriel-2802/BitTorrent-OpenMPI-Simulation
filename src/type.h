#pragma once
#include "auxiliary.h"

extern MPI_Datatype INQUIRY_T;

extern MPI_Datatype FILE_DATA_T;

MPI_Datatype createInquiryType();

MPI_Datatype createFileDataType();