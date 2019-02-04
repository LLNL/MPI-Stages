#include "errors.h"

namespace exampi
{

int check_buffer(void *buf)
{
	// return MPI_ERR_BUFFER
	return MPI_SUCCESS;
}

int check_comm(MPI_comm comm)
{
	//return MPI_ERR_COMM
	return MPI_SUCCESS;
}

int check_count(int count)
{
	//return MPI_ERR_COUNT
	return MPI_SUCCESS;
}

int check_tag(int tag)
{
	//return MPI_ERR_TAG
	return MPI_SUCCESS;
}

int check_datatype(MPI_Datatype datatype)
{
	//return MPI_ERR_DATATYPE
	return MPI_SUCCESS;
}

int check_rank(int rank)
{
	//return MPI_ERR_RANK
	return MPI_SUCCESS;
}

int check_request(MPI_Request *request)
{
	// return MPI_ERR_REQUEST
	return MPI_SUCCESS;
}

int check_status(MPI_Status *status)
{
	// return MPI_ERR_STATUS
	return MPI_SUCCESS;
}

}
