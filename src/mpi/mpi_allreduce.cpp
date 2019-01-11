#include "mpi.h"
#include "ExaMPI.h"
#include "basic.h"
#include "interfaces/interface.h"


extern "C"
{
#pragma weak MPI_Allreduce = PMPI_Allreduce

	int PMPI_Allreduce(const void *s_buf, void *r_buf, int count, MPI_Datatype type,
	                   MPI_Op op, MPI_Comm comm)
	{
		int rc = exampi::BasicInterface::get_instance()->MPI_Allreduce(s_buf, r_buf,
		         count, type, op,
		         comm);
		return rc;
	}

}
