#include "mpi.h"

#include "interfaces/interface.h"


extern "C"
{
#pragma weak MPI_Allreduce = PMPI_Allreduce

	int PMPI_Allreduce(const void *s_buf, void *r_buf, int count, MPI_Datatype type,
	                   MPI_Op op, MPI_Comm comm)
	{
		exampi::Universe &universe = exampi::Universe::get_root_universe();
		return universe.interface->MPI_Allreduce(s_buf, r_buf, count, type, op, comm);
	}

}
