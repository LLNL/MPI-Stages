#include "mpi.h"
#include "basic.h"
#include "interfaces/interface.h"


extern "C"
{
#pragma weak MPI_Recv = PMPI_Recv

	int PMPI_Recv(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
	              MPI_Comm comm, MPI_Status *status)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Recv(buf, count, datatype,
		         dest, tag,
		         comm, status);
		return rc;
	}

}
