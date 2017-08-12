#include <stdio.h>
#include "mpi.h"

#define ARRAY_LEN 4 
#define TIMES_AROUND_LOOP 10
#define TAG 0

int main(int argc, char** argv) 
{
  int i;
  int rank, size;
  int smallmessage[ARRAY_LEN];
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  smallmessage[0] = TIMES_AROUND_LOOP;
  for(i = 1; i < ARRAY_LEN; i++)
    smallmessage[i] = 0;

  
  if(rank == 0)
    MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank+1)%size, TAG, MPI_COMM_WORLD); /* inject initial message to ring  */

  do
  {
    MPI_Recv(smallmessage, ARRAY_LEN, MPI_INT, (rank-1+size)%size, TAG, MPI_COMM_WORLD, &status);

    if(rank == 0)
    {  
      /* completed a pass around the ring */
      if(!--smallmessage[0]) break;
    }

    MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank+1)%size, TAG, MPI_COMM_WORLD); /* forward around the logical ring */

  } while(smallmessage[0] > 0);
  
  printf("Hello World!\n");
  MPI_Finalize();
  return 0;
}
