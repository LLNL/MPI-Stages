#include "mpi.h"
#include "basic.h"
#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Finalize = PMPI_Finalize

	int PMPI_Finalize(void)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Finalize();
		return rc;
	}

}

