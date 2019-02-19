#include <mpi.h>

#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Comm_size = PMPI_Comm_size

	int PMPI_Comm_size(MPI_Comm c, int *r)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Comm_size(c, r);
		return rc;
	}


}
