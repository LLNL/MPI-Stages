#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#define CP_FREQUENCY 100  /* checkpoint every 100 times; just an example of how often you might do so */
#define MAX_ITERATIONS 100*100

/* CHECK THE setjmp/longjmp usage please */

int Application_Checkpoint_Read();
int Application_Checkpoint_Write();
void sig_handler(int sig_num);

int restart_mpi_val = 0;
jmp_buf restart_mpi_buf;

int main(int argc, char **argv)
{
  int restart_iteration = 0;
  int fault_epoch = 0;
  MPI_Comm mycomm = MPI_COMM_NULL, mpi_comm_world = MPI_COMM_NULL;
  signal(SIGUSR1, sig_handle);

restart_mpi:
  restart_mpi_val = setjmp(restart_mpi_buf);
  
  /* See:  http://man7.org/linux/man-pages/man3/longjmp.3.html
     http://en.cppreference.com/w/cpp/utility/program/setjmp */
         
  fault_epoch = MPI_New_reinit(&argc, &argv, &mpi_comm_world); /* reads the last checkpointed MPI state for this process */
  /* CAN THIS MPI_New_reinit() know if there is a restart, given the longjmp? Is this logically defective*/

  if(!fault_epoch)
  {
    MPI_Comm_dup(mpi_comm_world, &mycomm);
  }

  /* load/reload the user data */
  Application_Checkpoint_Read(-1,rank,fault_epoch /*think here more */,&restart_iteration...,&serialized,&n_serialized); /* needs global blob of objects to  */

  /* revivify the handles (one in this case): */  
  if(fault_epoch > 0)
     MPIX_Comm_deserialize(&serialized[0], &mycomm); /* deserialize and free? */

restart_loop:
  for(i = restart_iteration; i < MAX_ITERATIONS; ++i)
  {
    /* update solution */

    /* communicate */

    /* quiesce */
    MPI_Barrier(mycomm);

    if(!((i+1) % CP_FREQUENCY))
    {
      /* serialize all MPI objects */
      serialized[0] = MPIX_Comm_serialize(mycomm, &mycomm_serialized); /* DEAL WITH DANGLING ALLOCATED MEMORY PLEASE; this is not quite kosher */
      /* ... */

      Application_Checkpoint_Write(i,rank,fault_epoch,...,serialized,1); /* needs global blob of objects to */
      MPIX_Checkpoint_write(i,fault_epoch,mycomm); /* arguments ? */

      MPI_Comm_serialize_free(serialized[0]); /* is this OK? */
    }

  }

}

/* need signal handler here */

void sig_handler(int signum)
{
    switch(signum)
    {
    case SIGUSR1:
      if(restart_mpi_val != 0)
         longjmp(restart_mpi_buf); /* http://www.cplusplus.com/reference/csetjmp/longjmp/ */

      /* this is non-recoverable, we never got to the point of the program where we could restart? ?!!?!? */

      break;
    default:
      break;
    }
   

}
