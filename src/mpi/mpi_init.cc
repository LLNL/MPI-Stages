#include "mpi.h"

#include "interfaces/interface.h"

extern "C"
{

	#pragma weak MPI_Init = PMPI_Init

	int PMPI_Init(int *argc, char ***argv)
	{
		//exampi::Universe &universe = exampi::Universe::get_root_universe();
		//return universe.interface->MPI_Init(argc, argv);
		
		return exampi::BasicInterface::get_instance().MPI_Init(argc, argv);
	}

}
