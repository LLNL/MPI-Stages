#include <stdio.h>
#include <unistd.h>
#include "mpi.h"

#define NO_BCASTS 100
#define ARRAY_LEN 10

#define DEBUG

int main(int argc, char** argv) 
{
  int i, j;
  int rank, size;
  int smallmessage[ARRAY_LEN];
  MPI_Status status;

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

  if(rank == 0)  /* sequence: 0, 1, 3, 6, 10, 15... i*(i+1)/2 */
  {
     smallmessage[0] = 0;
     for(i = 1; i < ARRAY_LEN; i++)
        smallmessage[i] = i+smallmessage[i-1]; 
  }
  else
  {
     for(i = 0; i < ARRAY_LEN; i++)
        smallmessage[i] = -1;
  }


  if(rank == 0)
  {
    usleep(500000);
    // sf:  adding sleep to fix lack of barrier -- when can we remove this?
  }
#ifdef DEBUG
  printf("%d: Before bcast loop\n", rank);
#endif

  for(i = 0; i < NO_BCASTS; i++)
  {
     MPI_Bcast(smallmessage, ARRAY_LEN, MPI_INT, 0 /* root */, MPI_COMM_WORLD);

     if(! ((i+1)%rank) )
     {
       printf("%d: Rank reporting for broadcast %d is rank %d %s\n", rank, i, rank, (rank == 0)? "Sent data" : "Recvd data");
       for(j = 0; j < ARRAY_LEN; j++)
	 printf("%d: smallmessage[%d] = %d : %s\n", rank, i, smallmessage[j],
		(smallmessage[j] == j*(j+1)/2) ? "Good" : "Bad");
     }
     if(rank != 0)
       for(j = 0; j < ARRAY_LEN; j++)
	 smallmessage[i] = -1;
     
  } 

#ifdef DEBUG
  printf("%d: Exiting loop\n", rank);
#endif
  
  MPI_Finalize();
  return 0;
}

