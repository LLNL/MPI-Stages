#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Finalize = PMPI_Finalize

	int PMPI_Finalize(void)
	{
		int rc = exampi::interface->MPI_Finalize();
		//delete exampi::Interface::global;
		(void)rc;
		return MPI_SUCCESS;
	}

}

