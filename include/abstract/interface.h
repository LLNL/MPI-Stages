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

#ifndef __EXAMPI_ABSTRACT_INTERFACE_H
#define __EXAMPI_ABSTRACT_INTERFACE_H

#include <mpi.h>
#include "stages.h"

namespace exampi
{

class Interface : virtual public Stages
{
public:
	// setup
	virtual int MPI_Init(int *argc, char ***argv) = 0;

	virtual int MPI_Finalize(void) = 0;

	// utility
	virtual int MPI_Abort(MPI_Comm comm, int errorcode) = 0;

	virtual int MPI_Get_count(MPI_Status *status, MPI_Datatype datatype,
	                          int *count) = 0;

	virtual int MPI_Start(MPI_Request *request) =0;

	virtual int MPI_Wait(MPI_Request *request, MPI_Status *status) = 0;
	virtual int MPI_Waitall(int count, MPI_Request array_of_requests[],
	                        MPI_Status array_of_statuses[]) = 0;

	virtual double MPI_Wtime(void) = 0;

	virtual int MPI_Request_free(MPI_Request *request) = 0;

	// communicator
	virtual int MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler err) = 0;

	virtual int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm) = 0;

	virtual int MPI_Comm_rank(MPI_Comm comm, int *r) = 0;

	virtual int MPI_Comm_size(MPI_Comm comm, int *r) = 0;

	// p2p
	virtual int MPI_Send(const void *buf, int count, MPI_Datatype datatype,
	                     int dest, int tag, MPI_Comm comm) = 0;

	virtual int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source,
	                     int tag, MPI_Comm comm, MPI_Status *status) = 0;

	virtual int MPI_Sendrecv(const void *sendbuf, int sendcount,
	                         MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount,
	                         MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm,
	                         MPI_Status *status) = 0;

	virtual int MPI_Isend(const void *buf, int count, MPI_Datatype datatype,
	                      int dest, int tag, MPI_Comm comm, MPI_Request *request) = 0;

	virtual int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int dest,
	                      int tag, MPI_Comm comm, MPI_Request *request) = 0;

	virtual int MPI_Send_init(const void *buf, int count, MPI_Datatype datatype,
	                          int dest, int tag, MPI_Comm comm, MPI_Request *request) = 0;
	virtual int MPI_Recv_init(const void *buf, int count, MPI_Datatype datatype,
	                          int source, int tag, MPI_Comm comm, MPI_Request *request) = 0;

	// collective
	virtual int MPI_Barrier(MPI_Comm comm) = 0;

	virtual int MPI_Allreduce(const void *s_buf, void *r_buf, int count,
	                          MPI_Datatype type, MPI_Op op, MPI_Comm comm) = 0;

	virtual int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root,
	                      MPI_Comm comm) = 0;

	virtual int MPI_Reduce(const void *s_buf, void *r_buf, int count,
	                       MPI_Datatype type, MPI_Op op, int root, MPI_Comm comm) = 0;

	// stages
	virtual int MPIX_Checkpoint_write(void)
	{
		return MPI_SUCCESS;
	}

	virtual int MPIX_Checkpoint_read(void)
	{
		return MPI_SUCCESS;
	}

	virtual int MPIX_Get_fault_epoch(int *epoch)
	{
		return MPI_SUCCESS;
	}

	virtual int MPIX_Deserialize_handles()
	{
		return MPI_SUCCESS;
	}

	virtual int MPIX_Deserialize_handler_register(const MPIX_Deserialize_handler)
	{
		return MPI_SUCCESS;
	}

	virtual int MPIX_Serialize_handles()
	{
		return MPI_SUCCESS;
	}

	virtual int MPIX_Serialize_handler_register(const MPIX_Serialize_handler)
	{
		return MPI_SUCCESS;
	}
};

} // exampi
#endif
