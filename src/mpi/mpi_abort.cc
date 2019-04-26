#include "mpi.h"

#include "interfaces/interface.h"


extern "C"
{
#pragma weak MPI_Abort = PMPI_Abort

	int PMPI_Abort(MPI_Comm comm, int errorcode)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Abort(comm, errorcode);
	}

}
