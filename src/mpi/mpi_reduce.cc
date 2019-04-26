#include "mpi.h"

#include "interfaces/interface.h"


extern "C"
{
#pragma weak MPI_Reduce = PMPI_Reduce

	int PMPI_Reduce(const void *s_buf, void *r_buf, int count, MPI_Datatype type,
	                MPI_Op op, int root, MPI_Comm comm)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Reduce(s_buf, r_buf, count,
		         type, op,
		         root, comm);
	}

}
