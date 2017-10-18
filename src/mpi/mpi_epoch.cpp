#include <ExaMPI.h>
#include "basic.h"
#include "basic/interface.h"

extern "C"
{
#pragma weak MPI_Epoch = PMPI_Epoch

int PMPI_Epoch(int *out) {
  exampi::global::interface->MPI_Epoch(out);
  return MPI_SUCCESS;
}

}

