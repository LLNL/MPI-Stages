#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"

#pragma weak MPI_Recv = PMPI_Recv

extern "C"
{

int PMPI_Recv(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Status *status) {
	int rc = exampi::BasicInterface::global->MPI_Recv(buf, count, datatype, dest, tag, comm, NULL);
	return MPI_SUCCESS;
}


}
