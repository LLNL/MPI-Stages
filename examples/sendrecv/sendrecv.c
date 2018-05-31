#include <stdio.h>
#include <unistd.h>
#include "mpi.h"

int main(int argc, char** argv)  {

  int i, rank, size, extra, m1, m2, maxiter;
  unsigned int mask=1, next_p2, prev_p2;
  //MPI_Comm newcomm;
  MPI_Status status;

  //MPI_Comm_dup(comm, &newcomm);
  MPI_Init(&agrc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  for (next_p2=1; next_p2 < size; next_p2<<=1) ;
  printf("next_p2 %d\n", next_p2);
  prev_p2 = next_p2 >> 1;
  printf("prev_p2 %d\n", prev_p2);
  extra = size % prev_p2;
  printf("extra %d\n", extra);

  if (extra > 0 && rank >= prev_p2)    /* do the initial send */
    MPI_Send((void *)0, 0, MPI_INT, rank-prev_p2, 2001, MPI_COMM_WORLD);
  else if (rank < extra)               /* receive from the extras */
    MPI_Recv((void *)0, 0, MPI_INT, rank+prev_p2, 2001, MPI_COMM_WORLD, &status);

  if (rank < size-extra) {
    maxiter = (size == next_p2)?next_p2:prev_p2;
    for (i=0; maxiter>0; i++, maxiter>>=1) {
      m1 = mask<<i;
      printf("m1 %d\n", m1);
      m2 = mask<<(i+1);
      printf("m2 %d\n", m2);
      if (rank % m2 < m1)
	MPI_Sendrecv((void *)0, 0, MPI_INT, (rank+m1)%(size-extra), 2000,
		     (void *)0, 0, MPI_INT, (rank+m1)%(size-extra), 2000,
			 MPI_COMM_WORLD, &status);
      else
	MPI_Sendrecv((void *)0, 0, MPI_INT, (rank-m1)%(size-extra), 2000,
		     (void *)0, 0, MPI_INT, (rank-m1)%(size-extra), 2000,
			 MPI_COMM_WORLD, &status);
    }
  }

  if (rank < extra)          /* send to the extras */
    MPI_Send((void *)0, 0, MPI_INT, rank+prev_p2, 2001, MPI_COMM_WORLD);
  else if (extra > 0 && rank >= prev_p2) /* receive from the other half */
    MPI_Recv((void *)0, 0, MPI_INT, rank-prev_p2, 2001, MPI_COMM_WORLD, &status);

  //MPI_Comm_free(&newcomm);
  return 0;
}
