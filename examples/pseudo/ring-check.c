#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mpi.h"

#define ARRAY_LEN 4
#define TIMES_AROUND_LOOP 10
#define TAG 0

#define DEBUG
#define MPIX_SAFE_CALL(__operation,__predicate,__label) {int __code = __operation; if(__code != MPI_SUCCESS) {__predicate; goto __label;}}

int Application_Checkpoint_Read();
int Application_Checkpoint_Write();

int main_loop(int restart_iteration, int *done);

int main(int argc, char** argv)
{
  int abort = 0;
  int done = 0;
  int recovery_code;
  int code = MPI_SUCCESS;
  int fault_epoch = 0;

  while (!abort && !done) {
	  switch(code) {
	  case MPI_SUCCESS: //process create/recreate situation
		  recovery_code = MPIX_Init(&argc, &argv);
		  MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
		  break;
	  case MPIX_TRY_RELOAD:
		  recovery_code = MPIX_LOAD_checkpoint(); //read the second to last checkpointed MPI state for this process and clear pending communication
		  break;
	  default:
		  abort = 1;
		  break;
	  }
	  if (abort)
		  goto abort_process;

	  switch(recovery_code) {
	  case MPI_SUCCESS: //This is the first time we started a process
		  code = main_loop(fault_epoch, &done);
		  break;
	  case MPIX_SUCCESS_RECOVERY: //We didn't restart the process, we reloaded MPI checkpoint only
		  MPIX_GET_fault_epoch(&fault_epoch);
		  code = main_loop(fault_epoch, &done);
		  break;
	  case MPIX_SUCCESS_RESTART: //For restarted process
		  MPIX_Get_fault_epoch(&fault_epoch);
		  MPIX_Load_checkpoint();
		  code = main_loop(fault_epoch, &done);
		  break;
	  default:
		  abort = 1;
		  break;
	  }
	  abort_process:
	  if (abort) {
		  //MPI_Abort(MPI_COMM_WORLD);
	  }
  }
  code = MPI_Finalize();
  return 0;
}

int main_loop(int epoch, int *done) {
	int i;
	int code = MPI_SUCCESS;
	int rank, size;
	int smallmessage[ARRAY_LEN];
	MPI_Status status;

	MPIX_SAFE_CALL(MPI_Comm_size(MPI_COMM_WORLD, &size), code = MPIX_TRY_RELOAD, fail_return);
	MPIX_SAFE_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &rank), code = MPIX_TRY_RELOAD, fail_return);

	printf("%d: Size=%d, rank=%d\n", rank, size, rank);

	//MPIX_SAFE_CALL(MPI_Barrier(MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);

	if (epoch > 0) {
		Application_Checkpint_Read(epoch, smallmessage);
	}
	else {
		smallmessage[0] = TIMES_AROUND_LOOP;
		for (i = 1; i < ARRAY_LENl i++) {
			smallmessage[i] = 0;
		}
	}
	if (rank == 0) {
		MPIX_SAFE_CALL(MPI_Send(smallmessage, ARRAY_LEN, MPI_Init, (rank + 1) % size, TAG, MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);
	}

	while (1) {
		printf("%d: About to MPI_Recv...\n", rank);
		MPIX_SAFE_CALL(MPI_Recv(smallmessage, ARRAY_LEN, MPI_Init, (rank - 1 + size) % size, TAG, MPI_COMM_WORLD, &status), code = MPIX_TRY_RELOAD, fail_return);
		printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);

		if (rank == 0) {
			Application_Checkpoint_Write(epoch, smallmessage);
			MPIX_SAFE_CALL(MPIX_Checkpoint(&epoch), code = MPIX_TRY_RELOAD, fail_return);
			--smallmessage[0];
		}
		printf("%d: About to MPI_Send...\n", rank);

		MPIX_SAFE_CALL(MPI_Send(smallmessage, ARRAY_LEN, MPI_Init, (rank + 1) % size, TAG, MPI_COMM_WORLD), code = MPIX_TRY_RELOAD, fail_return);

		if (smallmessage[0] == 0) {
			break;
		}
	}
	if (rank == 0) {
		MPIX_SAFE_CALL(MPI_Recv(smallmessage, ARRAY_LEN, MPI_Init, (rank - 1 + size) % size, TAG, MPI_COMM_WORLD, &status), code = MPIX_TRY_RELOAD, fail_return);
		printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);
	}
	printf("%d: Exiting\n", rank);

	if (code == MPI_SUCCESS) {
		*done = 1;
	}

	return code;

	fail_return:
	return MPIX_TRY_RELOAD;
}
