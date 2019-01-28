#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Isend = PMPI_Isend

	int PMPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
	               MPI_Comm comm, MPI_Request *request)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Isend(buf, count, datatype,
		         dest, tag,
		         comm, request);
		return rc;
	}

}
