#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "basic/interface.h"

extern "C"
{
#pragma weak MPIX_Deserialize = PMPIX_Deserialize

int PMPIX_Deserialize(MPI_Comm *comm) {
  int rc = exampi::global::interface->MPIX_Deserialize(comm);
  return rc;
}

}
