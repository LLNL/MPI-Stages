#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "basic/interface.h"

extern "C"
{

#pragma weak MPI_Comm_size = PMPI_Comm_size

int PMPI_Comm_size(MPI_Comm c, int *r)
{
  int rc = exampi::global::interface->MPI_Comm_size(c, r);
  return rc;
}


}

