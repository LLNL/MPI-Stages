#include <ExaMPI.h>
#include <mpi.h>
#include "basic.h"
#include "basic/interface.h"

extern "C"
{
#pragma weak MPIX_Load_checkpoint = PMPIX_Load_checkpoint

int PMPIX_Load_checkpoint() {
  int rc = exampi::global::interface->MPIX_Load_checkpoint();
  return rc;
}

}
