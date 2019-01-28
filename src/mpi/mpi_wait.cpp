#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Wait = PMPI_Wait

	int PMPI_Wait(MPI_Request *request, MPI_Status *status)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Wait(request, status);
		return rc;
	}

}
