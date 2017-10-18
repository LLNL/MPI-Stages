#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "basic/interface.h"

extern "C" {

#pragma weak MPI_Barrier = PMPI_Barrier
int PMPI_Barrier(MPI_Comm comm) {
	int rc = exampi::global::interface->MPI_Barrier(comm);
	return rc;
}
}
