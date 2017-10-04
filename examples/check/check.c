#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char** argv) 
{
  srand(time(NULL));

  int data = 0;
  int epoch;
  int st = MPI_Init(&argc, &argv);

  if(st == MPI_REVERT)
  {
    MPI_Epoch(&epoch);
    printf("Got MPI_REVERT; epoch = %d\n",epoch);
    // "restore" our data to that epoch
    data = epoch;
  }

  while(data < 100)
  {
    printf("data = %d\n", data);
    usleep(10000);
    int r = rand() % 100;
    if(r == 0) // bad luck, partner; your time's run out
      exit(123);
    data++;
    MPI_Checkpoint(NULL);
  }

  printf("All done!\n");
  MPI_Finalize();
  return 0;
}
