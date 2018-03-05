#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdioh.h>

#define CP_FREQUENCY 100  /* checkpoint every 100 times; just an example of how often you might do so */
#define MAX_ITERATIONS 100*100
#define ARRAY_LEN 4
#define TIMES_AROUND_LOOP 10
#define TAG 0

int Application_Checkpoint_Read();
int Application_Checkpoint_Write();

int main_loop(int restart_iteration, int *done);

int main(int argc, char **argv) {
	int abort = 0; /* false */
	int done = 0; /* false */

	int recovery_code;
	int code = MPI_SUCCESS;

	int restart_iteration = 0;
	int fault_epoch = 0;
	//MPI_Comm mycomm = MPI_COMM_NULL, mpi_comm_world = MPI_COMM_NULL;

	/* set error codes to return */

	/* this example does not assume that a legacy MPI_COMM_WORLD is used at all or even necessarily created */

	while (!abort && !done) {
		switch (code) {
		case MPI_SUCCESS: /* no way for this pseudo-code to generate this case yet, except if process create/recreate situation, see above */
			recovery_code = MPI_Init(&argc, &argv);
			break;
		case MPIX_TRY_REINIT:
			//recovery_code = MPIX_New_reinit(&argc, &argv, &mpi_comm_world); /* reads the last checkpointed MPI state for this process or starts fresh as needed */
			//MPI_Errhandler_set(mpi_comm_world, MPI_ERRORS_RETURN); /* THIS SHOULD BE DEFAULT FOR MPIX_NEW_REINIT! TO AVOID RACE CONDITION WITH PREVIOUS CALL */
			//MPIX_Get_fault_epoch(mpi_comm_world, &fault_epoch);
			break;
		default:
			abort = 1;
			break;
		}
		if (abort)
			goto abort_process;

		switch (recovery_code) {
		case MPI_SUCCESS: /* This was the first time we started the process */
			code = main_loop(restart_iteration, &done);
			break;
		case MPIX_SUCCESS_RECOVERY: /* We didn't restart the process, we reloaded MPI checkpoint only */
			break;
		case MPIX_SUCCESS_RESTART: /* the process restarted from scratch */
			break;

		default:
			abort = 1;
			break;

		}
		abort_process: if (abort) /* we have an unmodeled situation: */
		{
			//MPI_Abort(mpi_comm_world, code);
		}

	}

	/* this section of code: no fault protection in postamble??? */

	//code = MPI_Comm_free(mycomm);
	//code = MPI_Comm_free(mpi_comm_world); /* TBD---since we create it now, we should destroy it */
	code = MPI_Finalize();

	return 0;
}

int main_loop(int restart_iteration, int *done) /* Doesn't contemplate stopping because of an application error not related to an MPI failure */
{
	int i;
	int code;
	int epoch, return_code;

	int rank, size;
	int smallmessage[ARRAY_LEN];
	MPI_Status status;

	printf("Entering main...\n");

	//int recovery_code = MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	printf("%d: Size=%d, rank=%d\n", rank, size, rank);
	printf("%d: About to populate smallmessage...\n", rank);

	smallmessage[0] = TIMES_AROUND_LOOP;
	for (i = 1; i < ARRAY_LEN; i++)
		smallmessage[i] = 0;

	printf("%d: About to rank-0 send....\n", rank);

	if (rank == 0) {
		// sf:  adding sleep to fix lack of barrier
		usleep(500000);
		MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank + 1) % size, TAG,
				MPI_COMM_WORLD); /* inject initial message to ring  */
	}

	printf("%d: Entering do loop...\n", rank);

	while (1) {
		printf("%d: About to MPI_Recv...\n", rank);

		MPI_Recv(smallmessage, ARRAY_LEN, MPI_INT, (rank - 1 + size) % size,
				TAG, MPI_COMM_WORLD, &status);

		printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);

		if (rank == 0) {
			/*code = Application_Checkpoint_Write(epoch, smallmessage);  needs global blob of objects to
			if (code) {
				 If non-MPI issues may fail the application, we need to generalize this code
				code = MPIX_TRY_REINIT;
				break;
			}*/
			code = MPI_Checkpoint(&epoch); /* arguments ? */
			if (code != MPI_SUCCESS) {
				code = MPIX_TRY_REINIT;
				break;
			}
			--smallmessage[0];
		}

		printf("%d: About to MPI_Send...\n", rank);

		MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank + 1) % size, TAG,
				MPI_COMM_WORLD); /* forward around the logical ring */

		/*code = MPI_Barrier(mycomm);
		if (code != MPI_SUCCESS)
			break;*/

		if (smallmessage[0] == 0) {

			printf("%d: Exiting loop\n", rank);

			break;
		}

	}
	if (rank == 0) {
		MPI_Recv(smallmessage, ARRAY_LEN, MPI_INT, (rank - 1 + size) % size,
				TAG, MPI_COMM_WORLD, &status);

		printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);

	}

	printf("%d: Exiting\n", rank);
	if (code == MPI_SUCCESS)
		*done = 1;

	return code; /* done */
}
