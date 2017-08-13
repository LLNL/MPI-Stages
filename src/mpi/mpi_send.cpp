#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"

int MPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
	int rc = exampi::BasicInterface::global->MPI_Send(buf, count, datatype, dest, tag, comm);
	return MPI_SUCCESS;
}
