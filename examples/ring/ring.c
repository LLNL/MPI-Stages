#include <stdio.h>
#include <unistd.h>
#include "mpi.h"

#define ARRAY_LEN 4 
#define TIMES_AROUND_LOOP 10
#define TAG 0

int main(int argc, char** argv) 
{
  printf("Entering main...\n");
  int i;
  int rank, size;
  int smallmessage[ARRAY_LEN];
  MPI_Status status;

  printf("About to init...\n");
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  printf("Size=%d, rank=%d\n", size, rank);
  printf("About to populate smallmessage...\n");
  smallmessage[0] = TIMES_AROUND_LOOP;
  for(i = 1; i < ARRAY_LEN; i++)
    smallmessage[i] = 0;

  
  printf("About to rank-0 send....\n");
  if(rank == 0)
  {
    // sf:  adding sleep to fix lack of barrier
    usleep(500000);
    MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank+1)%size, TAG, MPI_COMM_WORLD); /* inject initial message to ring  */
  }

  printf("Entering do loop...\n");
  do
  {
    printf("\tAbout to MPI_Recv...\n");
    MPI_Recv(smallmessage, ARRAY_LEN, MPI_INT, (rank-1+size)%size, TAG, MPI_COMM_WORLD, &status);
    printf("\t-> smallmessage[0] is now %d\n", smallmessage[0]);

    // sf:  fixed bug here where 0 would quit as soon as sm[0]==0, meaning it never made it around the ring
    // result of this fix indexes trip count at 0, e.g. TIMES_AROUND_LOOP 10 means 11 trips
    if(rank == 0)
      --smallmessage[0];

    printf("\tAbout to MPI_Send...\n");
    MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank+1)%size, TAG, MPI_COMM_WORLD); /* forward around the logical ring */

  } while(smallmessage[0] > 0);
  
  printf("Hello World!\n");
  MPI_Finalize();
  return 0;
}
