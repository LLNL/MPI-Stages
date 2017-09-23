
/*

  reinit-model2.c
  {AS 9/23/17 V1}

  Assumptions of this pseudo-code example.

   . Uses a form of reinit
   . Expects there to be no MPI_COMM_WORLD (if it is, we ignore it)
   . Relies on the MPIX_New_reinit() -- to start fresh if not a recovery; to bring MPI back to consistent state if it was OK but reinit; to build a new proc.
                                     -- relies on explicit MPI checkpoints from previous epochs; epoch counter user accessible.
				     -- Note, this is similar in name and concept to Ignacio's reinit, but we haven't been precise (intentionally) to make the same
				     
   . Longjmp/setjmp forbidden
   

New things we'd need to add to our prototype for this to work:

    Removal or ability to turn off default MPI_COMM_WORLD


    MPIX_New_reinit(&argc, &argv, &mpi_comm_world); --- a new form of initializing or reinitializing the world; related to Ignacio Reinit; also to what
                                                        we have implicitly in MPI_Init() now in AUMPI
						    --- and MPI_ERRORS_RETURN should be the default on this to avoid race, OR syntax is as follows:
    MPIX_New_reinit2(&argc,&argv,&mpi_comm_world, errhandler) -- make it atomic and allow any errhandler

    MPIX_Get_fault_epoch(mpi_comm_world, &fault_epoch); -- be able to get the MPI-wide constant of what error epoch we're in; may only make sense
                                                           now for MPI_COMM_WORLD substitute; could be for any communicator later; they might differ
							   since not everything connected to everything else.


    MPIX_TRY_REINIT -- a return code that tells you to go back and do a reinit.
    MPIX_SUCCESS_RECOVERY - a code resulting from MPIX_New_reinit/reinit2 -- you quiesced and reloaded MPI checkpoint state
    MPIX_SUCCESS_RESTART  - ditto - you actually restarted this process, then reloaded MPI checkpoint state


    MPIX_Checkpoint_write(i,fault_epoch,mycomm);  -- Creates the checkpoint at the user's discretion; 
                                                 -- There is no checkpoint read; that happens in a reinit.


   Probably the most interesting dangling question is reconnecting the user handles with the MPI objects after a reinit and a reload of MPI state and user state:						 
      MPIX_Comm_serialize_create(mycomm, &mycomm_serialized, &serialized[0]); -- allows you to capture semantics of handlers in your user checkpoint
      MPIX_Comm_deserialize(&serialized[0], &mycomm); 

   Discussion items--->

   1) We have not considered here what happens if we use an errhandler as opposed to MPI_ERRORS_RETURN; that is worth discussing too.

   2) We need to consider the situation where we reinit but don't make future progress; this pseudo-code could infinitely reinit and never increment
   the restart counter at each step... how do we determine if we need to actually abort the problem from an unmodeled situation even though our process
   works, we fail without net progress too many times?

   3) We should look at each individual error code; we should pass back codes when we think it is wiser just to abort the whole job? What else?

   More?

*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdioh.h>

#define CP_FREQUENCY 100  /* checkpoint every 100 times; just an example of how often you might do so */
#define MAX_ITERATIONS 100*100

int Application_Checkpoint_Read();
int Application_Checkpoint_Write();

int main_loop(int restart_iteration);

int main(int argc, char **argv)
{
  int abort = 0; /* false */
  int done  = 0; /* false */
  
  int recovery_code;
  int code = MPI_SUCCESS;

  int restart_iteration = 0;
  int fault_epoch = 0;
  MPI_Comm mycomm = MPI_COMM_NULL, mpi_comm_world = MPI_COMM_NULL;

  /* set error codes to return */

  /* this example does not assume that a legacy MPI_COMM_WORLD is used at all or even necessarily created */

  while(!abort && !done)
  {
    switch(code)
    {
    case MPI_SUCCESS: /* no way for this pseudo-code to generate this case yet, except if process create/recreate situation, see above */
    case MPIX_TRY_REINIT:
       recovery_code = MPIX_New_reinit(&argc, &argv, &mpi_comm_world); /* reads the last checkpointed MPI state for this process or starts fresh as needed */
       MPI_Errhandler_set(mpi_comm_world, MPI_ERRORS_RETURN); /* THIS SHOULD BE DEFAULT FOR MPIX_NEW_REINIT! TO AVOID RACE CONDITION WITH PREVIOUS CALL */
       MPIX_Get_fault_epoch(mpi_comm_world, &fault_epoch);
       break;
    default;
       abort = 1;
       break;
    }
    if(abort)
      break;

    switch(recovery_code)
    {
    case MPI_SUCCESS:           /* This was the first time we started the process */
    case MPIX_SUCCESS_RECOVERY:	/* We didn't restart the process, we reloaded MPI checkpoint only */	       
    case MPIX_SUCCESS_RESTART:  /* the process restarted from scratch */

      if(!fault_epoch)
      {
	code = MPI_Comm_dup(mpi_comm_world, &mycomm);
	if(return_code) 
	{
	  code = MPIX_TRY_REINIT; /* we could get into an infinite loop of REINITs; how to limit? */
	  break;
	}
      }

      /* load/reload the user data */
      code = Application_Checkpoint_Read(-1,rank,fault_epoch /*think here more */,
					 &restart_iteration...,&serialized,&n_serialized); /* needs global blob of objects to  */

      /* revivify the handles (one in this case): */  
      if(fault_epoch > 0)
      {
	code = MPIX_Comm_deserialize(&serialized[0], &mycomm); /* deserialize and free? */
	if(return_code) 
	{
	  code = MPIX_TRY_REINIT;
	  break;
	}
      }

      code = main_loop(restart_iteration, &done); /* all other application state through some state pointer or global data ptr/object TBD */
      break;

    default:
      abort = 1;
      break;
       
    }
    if(abort) /* we have an unmodeled situation: */
    {
      MPI_Abort(mpi_comm_world, code);
    }

  }

  /* this section of code: no fault protection in postamble??? */

  code = MPI_Comm_free(mycomm);
  code = MPI_Comm_free(mpi_comm_world); /* TBD---since we create it now, we should destroy it */
  code = MPI_Finalize();

  return 0;
}

int main_loop(int restart_iteration, int *done)  /* Doesn't contemplate stopping because of an application error not related to an MPI failure */
{
  int i;
  int code;

  for(i = restart_iteration; i < MAX_ITERATIONS; ++i)
  {
    /* update solution */

    /* communicate */

    if(!((i+1) % CP_FREQUENCY))
    {
      /* quiesce */
      code = MPI_Barrier(mycomm);
      if(code != MPI_SUCCESS) 
	break;

      /* serialize all MPI objects */
      code = MPIX_Comm_serialize_create(mycomm, &mycomm_serialized, &serialized[0]); /* DEAL WITH DANGLING ALLOCATED MEMORY PLEASE; this is not quite kosher */
      if(code != MPI_SUCCESS)
      {
	code = MPIX_TRY_REINIT;
	break;
      }
      /* ... notice, we don't serialize mpi_comm_world; I guess we don't ever use it???  */

      return_code = Application_Checkpoint_Write(i,rank,fault_epoch,...,serialized,1); /* needs global blob of objects to */
      if(return_code) 
      {
	/* If non-MPI issues may fail the application, we need to generalize this code */
	code = MPIX_TRY_REINIT;
	break;
      }
      code = MPIX_Checkpoint_write(i,fault_epoch,mycomm); /* arguments ? */
      if(code != MPI_SUCCESS) 
      {
	code = MPIX_TRY_REINIT;
	break;
      }

      code = MPIX_Comm_serialize_free(serialized[0]); /* is this OK? */
      if(code != MPI_SUCCESS) 
      {
	code = MPIX_TRY_REINIT;
	break;
      }

      /* quiesce */
      code = MPI_Barrier(mycomm);
      if(code != MPI_SUCCESS) 
      {
	code = MPIX_TRY_REINIT;
	break;
      }

    }
  }
  if(code == MPI_SUCCESS)
    *done = 1;

  return code; /* done */
}

