#ifndef __EXAMPI_ERRORS_H
#define __EXAMPI_ERRORS_H

namespace exampi
{

//enum mpi_error
//{
//	MPI_SUCCESS = 0,
//	MPI_ERR_COMM,
//	MPI_ERR_COUNT,
//	MPI_ERR_TYPE,
//	MPI_ERR_TAG,
//	MPI_ERR_RANK
//};

#ifdef RUNTIME_ARGUMENT_CHECK

// TODO debugpp("santizing buffer"); if...

#define CHECK_BUFFER(buffer)			if(check_buffer(buffer) != 0)		return MPI_ERR_BUF; 
#define CHECK_COMM(communicator)		if(check_comm(communicator) != 0) 	return MPI_ERR_COMM; 
#define CHECK_COUNT(count)				if(check_count(count) != 0) 		return MPI_ERR_COUNT; 
#define CHECK_TAG(tag) 					if(check_tag(tag) != 0) 			return MPI_ERR_TAG;
#define CHECK_DATATYPE(datatype)		if(check_datatype(datatype) != 0) 	return MPI_ERR_TYPE; 
#define CHECK_RANK(rank)				if(check_rank(rank) != 0) 			return MPI_ERR_RANK;

#define CHECK_REQUEST(request) 
#define CHECK_STATUS(status) 

#else

#define CHECK_BUFFER(buffer)
#define CHECK_COMM(communicator) 
#define CHECK_COUNT(count) 
#define CHECK_TAG(tag) 
#define CHECK_DATATYPE(datatype) 
#define CHECK_RANK(rank)

#define CHECK_REQUEST(request) 
#define CHECK_STATUS(status) 

#endif

}

#endif
