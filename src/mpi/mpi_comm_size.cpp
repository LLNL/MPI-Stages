#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"

#pragma weak MPI_Comm_size = PMPI_Comm_size

int PMPI_Comm_size(MPI_Comm c, int *r)
{
  exampi::BasicInterface::global->MPI_Comm_size(c, r);
  return MPI_SUCCESS;
}
