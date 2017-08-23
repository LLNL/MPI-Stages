#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "basic/interface.h"


extern "C"
{

#pragma weak MPI_Init = PMPI_Init

int PMPI_Init(int *argc, char ***argv) {
	int rc = exampi::global::interface->MPI_Init(argc, argv);
	return MPI_SUCCESS;
}

}
