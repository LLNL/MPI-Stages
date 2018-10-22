#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "basic/interface.h"

extern "C"
{
#pragma weak MPIX_Serialize = PMPIX_Serialize

int PMPIX_Serialize(MPI_Comm comm) {
  int rc = exampi::global::interface->MPIX_Serialize(comm);
  return rc;
}

}
