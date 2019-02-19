#include "errors.h"

#include "universe.h"

namespace exampi
{

int check_buffer(void *buf)
{
	if((uintptr_t)buf > (uintptr_t)0)
		return MPI_SUCCESS;
	else
		return MPI_ERR_BUFFER;
}

int check_comm(MPI_Comm comm)
{
	Universe &universe = Universe::get_root_universe();

	// todo return MPI_ERR_COMM
	return MPI_SUCCESS;
}

int check_count(int count)
{
	if(count > 0)
		return MPI_SUCCESS;
	else
		return MPI_ERR_COUNT;
}

int check_tag(int tag)
{
	if((tag >= 0)|| (tag < MPI_TAG_UB))
		return MPI_ERR_TAG;
	else
		return MPI_SUCCESS;
}

int check_datatype(MPI_Datatype datatype)
{
	// todo return MPI_ERR_DATATYPE
	return MPI_SUCCESS;
}

int check_rank(int rank, MPI_Comm comm)
{
	// todo valid for communicator check

	if(rank >= 0)
		return MPI_SUCCESS;
	else
		return MPI_ERR_RANK;
}

int check_request(MPI_Request *request)
{
	if(request == 0)
		return MPI_ERR_REQUEST;
	else
		return MPI_SUCCESS;
}

int check_status(MPI_Status *status)
{
	if(status == MPI_STATUS_IGNORE || (uintptr_t)status > (uintptr_t)0)
		return MPI_SUCCESS;
	else
		return MPI_ERR_STATUS;
}

}
