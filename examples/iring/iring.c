//
// iring.c - ring.c using simple non-blocking + wait variation
//
#include <stdio.h>
#include <unistd.h>
#include "mpi.h"

#define ARRAY_LEN 4 
#define TIMES_AROUND_LOOP 10
#define TAG 0

#define DEBUG

int main(int argc, char** argv) 
{
  int i;
  int rank, size;
  int smallmessage[ARRAY_LEN];
  MPI_Status status;
  MPI_Request sreq, rreq;

#ifdef DEBUG
  printf("Entering main...\n");
#endif

#ifdef DEBUG
  printf("About to init...\n");
#endif
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#ifdef DEBUG
  printf("%d: Size=%d, rank=%d\n", rank, size, rank);
  printf("%d: About to populate smallmessage...\n",rank);
#endif

  smallmessage[0] = TIMES_AROUND_LOOP;
  for(i = 1; i < ARRAY_LEN; i++)
    smallmessage[i] = 0;

#ifdef DEBUG  
  printf("%d: About to rank-0 send....\n", rank);
#endif

  if(rank == 0)
  {
    // sf:  adding sleep to fix lack of barrier
    usleep(500000);
    MPI_Isend(smallmessage, ARRAY_LEN, MPI_INT, (rank+1)%size, TAG, MPI_COMM_WORLD, &sreq); /* inject initial message to ring  */
    MPI_Wait(&sreq, &status);
  }

#ifdef DEBUG
  printf("%d: Entering do loop...\n",rank);
#endif

  do
  {
#ifdef DEBUG
    printf("%d: About to MPI_Recv...\n", rank);
#endif

    MPI_Irecv(smallmessage, ARRAY_LEN, MPI_INT, (rank-1+size)%size, TAG, MPI_COMM_WORLD, &rreq);
    MPI_Wait(&rreq, &status);
#ifdef DEBUG
    printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);
#endif

    // sf:  fixed bug here where 0 would quit as soon as sm[0]==0, meaning it never made it around the ring
    // result of this fix indexes trip count at 0, e.g. TIMES_AROUND_LOOP 10 means 11 trips
    // AS:  Yes, but you added another bug.

    if(rank == 0)
      --smallmessage[0];

#ifdef DEBUG
    printf("%d: About to MPI_Send...\n", rank);
#endif

    if((rank == 0 && smallmessage[0])||(rank > 0)) /* don't leave a dangling message in the network */
    {
      MPI_Isend(smallmessage, ARRAY_LEN, MPI_INT, (rank+1)%size, TAG, MPI_COMM_WORLD, &sreq); /* forward around the logical ring */
      MPI_Wait(&sreq, &status);
    }

  } while(smallmessage[0] > 0);

#ifdef DEBUG
  printf("%d: Exiting loop\n", rank);
#endif
  
  MPI_Finalize();
  return 0;
}
