#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Send = PMPI_Send

int PMPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
	int rc = exampi::global::interface->MPI_Send(buf, count, datatype, dest, tag, comm);
  (void)rc;
	return rc;
}

}
