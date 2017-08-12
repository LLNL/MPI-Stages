#include <ExaMPI.h>
#include <mpi.h>

int MPI_Comm_rank(MPI_Comm c, int *r)
{
  *r = 0;
  return MPI_SUCCESS;
}
