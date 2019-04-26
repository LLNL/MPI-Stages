#include "mpi.h"

#include "interfaces/interface.h"


extern "C"
{

#pragma weak MPI_Wait = PMPI_Wait

	int PMPI_Wait(MPI_Request *request, MPI_Status *status)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Wait(request, status);
	}

}
