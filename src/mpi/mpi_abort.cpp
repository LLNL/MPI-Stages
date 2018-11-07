#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "interfaces/interface.h"


extern "C"
{
#pragma weak MPI_Abort = PMPI_Abort

	int PMPI_Abort(MPI_Comm comm, int errorcode)
	{
		int rc = exampi::interface->MPI_Abort(comm, errorcode);
		return rc;
	}

}
