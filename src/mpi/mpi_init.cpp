#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Init = PMPI_Init

	int PMPI_Init(int *argc, char ***argv)
	{
		int rc = exampi::BasicInterface::get_instance().MPI_Init(argc, argv);
		return rc;
	}

}
