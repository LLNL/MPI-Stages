#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"


extern "C"
{

#pragma weak MPI_Comm_rank = PMPI_Comm_rank

int PMPI_Comm_rank(MPI_Comm c, int *r)
{
  exampi::BasicInterface::global->MPI_Comm_rank(c, r);
  return MPI_SUCCESS;
}

}
