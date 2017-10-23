#include <stdio.h>
#include <unistd.h>
#include "mpi.h"

#define ARRAY_LEN 4
#define TIMES_AROUND_LOOP 10
#define TAG 0

#define DEBUG

int main(int argc, char** argv)
{
  int i, epoch;
  int process_0_exit = 5;
  int rank, size;
  int smallmessage[ARRAY_LEN];
  MPI_Status status;

#ifdef DEBUG
  printf("Entering main...\n");
#endif

#ifdef DEBUG
  printf("About to init...\n");
#endif
  int recovery-code = MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#ifdef DEBUG
  printf("%d: Size=%d, rank=%d\n", rank, size, rank);
  printf("%d: About to populate smallmessage...\n",rank);
#endif
  // MPI loaded it's previous state; now application needs to load
  if (recovery-code == MPIX_SUCCESS_RECOVERY) {
	  MPI_Epoch(&epoch);
	  Application_Checkpiont_load(&epoch, smallmessage);
	  process_0_exit = 100;
  }
  else {
	  smallmessage[0] = TIMES_AROUND_LOOP;
	  for(i = 1; i < ARRAY_LEN; i++)
		  smallmessage[i] = 0;
  }

#ifdef DEBUG
  printf("%d: About to rank-0 send....\n", rank);
#endif

  if(rank == 0)
  {
    // sf:  adding sleep to fix lack of barrier
    usleep(500000);
    MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank+1)%size, TAG, MPI_COMM_WORLD); /* inject initial message to ring  */
  }

#ifdef DEBUG
  printf("%d: Entering do loop...\n",rank);
#endif

  while (1)
  {
#ifdef DEBUG
    printf("%d: About to MPI_Recv...\n", rank);
#endif

    MPI_Recv(smallmessage, ARRAY_LEN, MPI_INT, (rank-1+size)%size, TAG, MPI_COMM_WORLD, &status);
#ifdef DEBUG
    printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);
#endif

    // Checkpoint after completion of each ring
    // Exit process 0 after completion of 5 ring
    if ((smallmessage[0] == process_0_exit) && rank == 0) {
    	--smallmessage[0];
    	MPI_Checkpoint(&epoch);
    	Application_Checkpoint_save(epoch, smallmessage);
    	exit(123);
    }
    else if(rank == 0) {
      --smallmessage[0];
      MPI_Checkpoint(&epoch);
      Application_Checkpoint_save(epoch, smallmessage);
    }

#ifdef DEBUG
    printf("%d: About to MPI_Send...\n", rank);
#endif

    MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank+1)%size, TAG, MPI_COMM_WORLD); /* forward around the logical ring */

    if (smallmessage[0] == 0) {
#ifdef DEBUG
    	printf("%d: Exiting loop\n", rank);
#endif
    	break;
    }

  }
  if (rank == 0) {
	  MPI_Recv(smallmessage, ARRAY_LEN, MPI_INT, (rank-1+size)%size, TAG, MPI_COMM_WORLD, &status);
	  #ifdef DEBUG
	      printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);
	  #endif
  }

#ifdef DEBUG
  printf("%d: Exiting\n", rank);
#endif

  MPI_Finalize();
  return 0;
}
