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

#ifndef __EXAMPI_INTERFACE_H
#define __EXAMPI_INTERFACE_H

#include <time.h>
#include <cstdlib>

#include "funcType.h"
#include "abstract/interface.h"
#include "daemon.h"
#include "errors.h"
#include "pool.h"
#include "stages.h"
#include "request.h"
#include "universe.h"

namespace exampi
{

class BasicInterface: public Interface
{
private:
	int construct_request(const void *buf, int count, MPI_Datatype datatype,
	                      int source, int dest, int tag, MPI_Comm comm, MPI_Request *request,
	                      Operation operation);
	int finalize_request(MPI_Request *request, Request *req, MPI_Status *status);

	int offload_persistent(const void *, int, MPI_Datatype, int, int, MPI_Comm,
	                       Operation, MPI_Request *);
	int offload_persistent_wait(const void *, int, MPI_Datatype, int, int, MPI_Comm,
	                            Operation);

protected:
	void init_group();
	void init_communicator();
	void init_datatype();

public:
	BasicInterface();
	~BasicInterface();

	int MPI_Init(int *argc, char ***argv);
	//int MPI_Init_thread(int *argc, char ***argv, int required, int *provided);
	int MPI_Initialized(int *flag);
	int MPI_Finalize();

	// blocking
	int MPI_Send(const void *buf, int count, MPI_Datatype datatype,
	             int dest, int tag, MPI_Comm comm);
	int MPI_Bsend(const void *buf, int count, MPI_Datatype datatype, int dest,
	              int tag, MPI_Comm comm);
	int MPI_Ssend(const void *buf, int count, MPI_Datatype datatype, int dest,
	              int tag, MPI_Comm comm);
	int MPI_Rsend(const void *buf, int count, MPI_Datatype datatype, int dest,
	              int tag, MPI_Comm comm);
	int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
	             int source, int tag, MPI_Comm comm, MPI_Status *status);
	int MPI_Sendrecv(const void *sendbuf, int sendcount,
	                 MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount,
	                 MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm,
	                 MPI_Status *status);

	// non-blocking
	int MPI_Isend(const void *buf, int count, MPI_Datatype datatype,
	              int dest, int tag, MPI_Comm comm, MPI_Request *request);
	int MPI_Ibsend(const void *buf, int count, MPI_Datatype datatype, int dest,
	               int tag,
	               MPI_Comm comm, MPI_Request *request);
	int MPI_Issend(const void *buf, int count, MPI_Datatype datatype, int dest,
	               int tag,
	               MPI_Comm comm, MPI_Request *request);
	int MPI_Irsend(const void *buf, int count, MPI_Datatype datatype, int dest,
	               int tag,
	               MPI_Comm comm, MPI_Request *request);
	int MPI_Irecv(void *buf, int count, MPI_Datatype datatype,
	              int source, int tag, MPI_Comm comm, MPI_Request *request);

	// persistent
	int MPI_Send_init(const void *buf, int count, MPI_Datatype datatype, int dest,
	                  int tag, MPI_Comm comm, MPI_Request *request);
	int MPI_Ssend_init(const void *buf, int count, MPI_Datatype datatype, int dest,
	                   int tag, MPI_Comm comm, MPI_Request *request);
	int MPI_Rsend_init(const void *buf, int count, MPI_Datatype datatype, int dest,
	                   int tag, MPI_Comm comm, MPI_Request *request);
	int MPI_Bsend_init(const void *buf, int count, MPI_Datatype datatype, int dest,
	                   int tag, MPI_Comm comm, MPI_Request *request);
	int MPI_Recv_init(const void *buf, int count, MPI_Datatype datatype, int source,
	                  int tag, MPI_Comm comm, MPI_Request *request);


	// ...
	int MPI_Start(MPI_Request *request);

	int MPI_Wait(MPI_Request *request, MPI_Status *status);
	int MPI_Waitall(int count, MPI_Request array_of_requests[],
	                MPI_Status array_of_statuses[]);

	int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status);

	int MPI_Barrier(MPI_Comm comm);
	int MPI_Request_free(MPI_Request *request);

	int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root,
	              MPI_Comm comm);

	int MPI_Comm_rank(MPI_Comm comm, int *r);

	int MPI_Comm_size(MPI_Comm comm, int *r);

	int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm);

	double MPI_Wtime();

	int MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler err);
	int MPI_Reduce(const void *s_buf, void *r_buf, int count,
	               MPI_Datatype type, MPI_Op op, int root, MPI_Comm comm);

	int MPI_Allreduce(const void *s_buf, void *r_buf, int count,
	                  MPI_Datatype type, MPI_Op op, MPI_Comm comm);

	int MPI_Get_count(MPI_Status *status, MPI_Datatype datatype, int *count);
	int MPI_Abort(MPI_Comm comm, int errorcode);

	int save(std::ostream &)
	{
		return MPI_SUCCESS;
	}
	int load(std::istream &)
	{
		return MPI_SUCCESS;
	}
	int cleanup()
	{
		return MPI_SUCCESS;
	}
	int halt()
	{
		return MPI_SUCCESS;
	}
};

} // namespace exampi

#endif //...H
