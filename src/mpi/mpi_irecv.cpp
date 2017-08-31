#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "basic/interface.h"


extern "C"
{

#pragma weak MPI_Irecv = PMPI_Irecv

int PMPI_Irecv(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	int rc = exampi::global::interface->MPI_Irecv(buf, count, datatype, dest, tag, comm, request);
	return MPI_SUCCESS;
}

}
