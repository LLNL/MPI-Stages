// 
// This is a source code component of ExaMPI, a next-generation 
// implementation of the Message Passing Interface.
//
// This is a joint project of University of Tennessee at Chattanooga,
// the Lawrence Livermore National Laboratory, and Auburn University.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE', which is part of this source code package.
// 

#ifndef __EXAMPI_ERRORS_H
#define __EXAMPI_ERRORS_H

#ifdef __cplusplus
extern "C"
{
#endif

enum Errors
{
	// MPI defined
	MPI_SUCCESS = 0,

	// MPI declared
	MPI_ERR_BUFFER,
	MPI_ERR_COUNT,
	MPI_ERR_TYPE,
	MPI_ERR_TAG,
	MPI_ERR_COMM,
	MPI_ERR_RANK,
	MPI_ERR_REQUEST,
	MPI_ERR_ROOT,
	MPI_ERR_GROUP,
	MPI_ERR_OP,
	MPI_ERR_TOPOLOGY,
	MPI_ERR_DIMS,
	MPI_ERR_ARG,
	MPI_ERR_UNKNOWN,
	MPI_ERR_TRUNCATE,
	MPI_ERR_OTHER,
	MPI_ERR_INTERN,
	MPI_ERR_IN_STATUS,
	MPI_ERR_PENDING,
	MPI_ERR_KEYVAL,
	MPI_ERR_NO_MEM,
	MPI_ERR_BASE,
	MPI_ERR_INFO_KEY,
	MPI_ERR_INFO_VALUE,
	MPI_ERR_INFO_NOKEY,
	MPI_ERR_SPAWN,
	MPI_ERR_PORT,
	MPI_ERR_SERVICE,
	MPI_ERR_NAME,
	MPI_ERR_WIN,
	MPI_ERR_SIZE,
	MPI_ERR_DISP,
	MPI_ERR_INFO,
	MPI_ERR_LOCKTYPE,
	MPI_ERR_ASSERT,
	MPI_ERR_RMA_CONFLICT,
	MPI_ERR_RMA_SYNC,
	MPI_ERR_RMA_RANGE,
	MPI_ERR_RMA_ATTACH,
	MPI_ERR_RMA_SHARED,
	MPI_ERR_RMA_FLAVOR,
	MPI_ERR_FILE,
	MPI_ERR_NOT_SAME,
	MPI_ERR_AMODE,
	MPI_ERR_UNSUPPORTED_DATAREP,
	MPI_ERR_UNSUPPORTED_OPERATION,
	MPI_ERR_NO_SUCH_FILE,
	MPI_ERR_FILE_EXISTS,
	MPI_ERR_BAD_FILE,
	MPI_ERR_ACCESS,
	MPI_ERR_NO_SPACE,
	MPI_ERR_QUOTA,
	MPI_ERR_READ_ONLY,
	MPI_ERR_FILE_IN_USE,
	MPI_ERR_DUP_DATAREP,
	MPI_ERR_CONVERSION,
	MPI_ERR_IO,

	// EXAMPI declared
	MPI_ERR_DISABLED,

	// MPI declared
	MPI_ERR_LASTCODE
};

#ifdef __cplusplus
}
#endif

#endif
