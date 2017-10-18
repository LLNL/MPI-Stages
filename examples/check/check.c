#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char** argv) 
{
  srand(time(NULL));

  int data = 0;
  int epoch, i;
  int rank, size;
  int r = 5;
  int st = MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  if(st == MPI_REVERT)
  {
    MPI_Epoch(&epoch);
    printf("Got MPI_REVERT; epoch = %d\n",epoch);
    // "restore" our data to that epoch
    data = epoch + 1;
    r = 0;
  }
  //MPI_Barrier(MPI_COMM_WORLD);
  while(data < 10)
  {
    printf("data = %d\n", data);
    usleep(100000);
    //int r = rand() % 100;
    if(data == r && rank == 0) // bad luck, partner; your time's run out
      exit(123);
    data++;
    MPI_Checkpoint(&i);
  }

  printf("All done!\n");
  MPI_Finalize();
  return 0;
}
