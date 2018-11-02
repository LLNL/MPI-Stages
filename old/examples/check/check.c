#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mpi.h"

#define ARRAY_LEN 4
#define TIMES_AROUND_LOOP 5
#define TAG 0

#define DEBUG

int Application_Checkpoint_Read(int rank, int smallmessage[]);
void Application_Checkpoint_Write(int rank, int smallmessage[]);

int main(int argc, char** argv)
{
	int i;
	int rank, size;
	int smallmessage[ARRAY_LEN];
	MPI_Status status;
	char fname[10];
	int r = 2;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	printf("%d: Size=%d, rank=%d\n", rank, size, rank);
    sprintf(fname, "check_%d", rank);
	if( access( fname, F_OK ) != -1 ) {

		Application_Checkpoint_Read(rank, smallmessage);
		printf("Recovered time %lf", MPI_Wtime());
				if (rank == 0)
					++smallmessage[0];
				r = 20;
	} else {

		smallmessage[0] = 0;
				for (i = 1; i < ARRAY_LEN; i++) {
					smallmessage[i] = 0;
				}
	}

	if (rank == 0) {
		MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank + 1) % size, TAG, MPI_COMM_WORLD);
	}

	while (1) {
		printf("%d: About to MPI_Recv...\n", rank);
		MPI_Recv(smallmessage, ARRAY_LEN, MPI_INT, (rank - 1 + size) % size, TAG, MPI_COMM_WORLD, &status);
		printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);

		Application_Checkpoint_Write(rank, smallmessage);

		if (rank == 0) {
			++smallmessage[0];
		}
		printf("%d: About to MPI_Send...\n", rank);

		MPI_Send(smallmessage, ARRAY_LEN, MPI_INT, (rank + 1) % size, TAG, MPI_COMM_WORLD);
		if (smallmessage[0] == r && rank == 0) {
			printf("Failure time %lf", MPI_Wtime());
			MPI_Abort(MPI_COMM_WORLD, -1);
		}
		if (smallmessage[0] == TIMES_AROUND_LOOP) {
			break;
		}
	}
	if (rank == 0) {
		MPI_Recv(smallmessage, ARRAY_LEN, MPI_INT, (rank - 1 + size) % size, TAG, MPI_COMM_WORLD, &status);
		printf("%d:  smallmessage[0] is now %d\n", rank, smallmessage[0]);
	}
	printf("%d: Exiting\n", rank);
	MPI_Finalize();
	return 0;
}

int Application_Checkpoint_Read(int rank, int smallmessage[]) {
	FILE *fp;
	char buf[10];
	sprintf(buf, "check_%d", rank);
	if ((fp = fopen(buf, "rb")) == NULL) {
		printf("ERROR: Opening File");
	}
	fread(smallmessage, sizeof(int), ARRAY_LEN, fp);
	fclose(fp);
	return 0;
}

void Application_Checkpoint_Write(int rank, int smallmessage[]) {
	FILE *fp;
	char buf[10];
	sprintf(buf, "check_%d", rank);
	if ((fp = fopen(buf, "wb")) == NULL) {
		printf("ERROR: Opening File\n");
	}

	fwrite(smallmessage, sizeof(int), ARRAY_LEN, fp);
	fclose(fp);
}
