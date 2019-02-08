#ifndef __EXAMPI_ERRORS_H
#define __EXAMPI_ERRORS_H

#include "mpi.h"

namespace exampi
{

// todo consolidate error definitions
//enum mpi_error
//{
//	MPI_SUCCESS = MPI_SUCCESS,
//	MPI_ERR_COMM,
//	MPI_ERR_COUNT,
//	MPI_ERR_TYPE,
//	MPI_ERR_TAG,
//	MPI_ERR_RANK
//};

#ifdef RUNTIME_ARGUMENT_CHECK

int check_buffer(void *buf);
int check_comm(MPI_Comm comm);
int check_count(int count);
int check_tag(int tag);
int check_datatype(MPI_Datatype datatype);
int check_rank(int rank, MPI_Comm comm);

int check_request(MPI_Request *request);
int check_status(MPI_Status *status);

#define CHECK_BUFFER(buffer)			return check_buffer(buffer);
#define CHECK_COMM(communicator)		return check_comm(communicator);
#define CHECK_COUNT(count)				return check_count(count);
#define CHECK_TAG(tag) 					return check_tag(tag);		
#define CHECK_DATATYPE(datatype)		return check_datatype(datatype);
#define CHECK_RANK(rank, communiator)	return check_rank(rank, communicator);

#define CHECK_REQUEST(request)			return check_request(request);
#define CHECK_STATUS(status) 			return check_status(status);	

#else

#define CHECK_BUFFER(buffer)
#define CHECK_COMM(communicator) 
#define CHECK_COUNT(count) 
#define CHECK_TAG(tag) 
#define CHECK_DATATYPE(datatype) 
#define CHECK_RANK(rank, communicator)

#define CHECK_REQUEST(request) 
#define CHECK_STATUS(status) 

#endif

}

#endif
