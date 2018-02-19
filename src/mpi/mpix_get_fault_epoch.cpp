#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "basic/interface.h"

extern "C"
{
#pragma weak MPIX_Get_fault_epoch = PMPIX_Get_fault_epoch

int PMPIX_Get_fault_epoch(int *out) {
  exampi::global::interface->MPIX_Get_fault_epoch(out);
  return MPI_SUCCESS;
}

}

