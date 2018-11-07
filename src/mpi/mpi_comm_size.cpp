#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Comm_size = PMPI_Comm_size

	int PMPI_Comm_size(MPI_Comm c, int *r)
	{
		int rc = exampi::interface->MPI_Comm_size(c, r);
		return rc;
	}


}

