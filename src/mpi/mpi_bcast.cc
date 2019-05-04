#include "mpi.h"

#include "interfaces/interface.h"

extern "C"
{

#pragma weak MPI_Bcast = PMPI_Bcast

	int PMPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root,
	               MPI_Comm comm)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Bcast(buf, count, datatype,
		                                     root, comm);
	}

}
