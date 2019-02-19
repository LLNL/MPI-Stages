#include "mpi.h"

#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Send = PMPI_Send

	int PMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest,
	              int tag, MPI_Comm comm)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Send(buf, count, datatype,
		         dest, tag,
		         comm);
		return rc;
	}

}
