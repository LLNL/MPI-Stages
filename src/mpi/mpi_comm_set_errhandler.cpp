#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Comm_set_errhandler = PMPI_Comm_set_errhandler

	int PMPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler err)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Comm_set_errhandler(comm,
		         err);
		return rc;
	}

}
