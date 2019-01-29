#ifndef __EXAMPI_INTERFACE_H
#define __EXAMPI_INTERFACE_H

#include <basic.h>
#include <time.h>
#include <errHandler.h>
#include <funcType.h>
#include <cstdlib>

#include "engines/progress.h"
#include "abstract/interface.h"
#include "daemon.h"
#include "config.h"

namespace exampi
{

class BasicInterface: public Interface
{
private:
	BasicInterface() {};
	~BasicInterface() {};

	static BasicInterface *instance;

	std::vector<MPIX_Serialize_handler> serialize_handlers;
	std::vector<MPIX_Deserialize_handler> deserialize_handlers;
	int recovery_code;

public:
	//BasicInterface();
	static BasicInterface &get_instance();

	BasicInterface(const BasicInterface &c) = delete;
	BasicInterface &operator=(const BasicInterface &c) = delete;

	/*
	 * 1st param: config file
	 * 2nd param: rank
	 * 3rd param: epoch file
	 * 4th param: epoch
	 */
	int MPI_Init(int *argc, char ***argv);

	int MPI_Finalize();

	int MPI_Send(const void *buf, int count, MPI_Datatype datatype,
	             int dest, int tag, MPI_Comm comm);

	int MPI_Recv(void *buf, int count, MPI_Datatype datatype,
	             int source, int tag, MPI_Comm comm, MPI_Status *status);

	int MPI_Isend(const void *buf, int count, MPI_Datatype datatype,
	              int dest, int tag, MPI_Comm comm, MPI_Request *request);
	int MPI_Irecv(void *buf, int count, MPI_Datatype datatype,
	              int source, int tag, MPI_Comm comm, MPI_Request *request);


	int MPI_Send_init(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
	int MPI_Recv_init(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request);

	int MPI_Sendrecv(const void *sendbuf, int sendcount,
	                 MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount,
	                 MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm,
	                 MPI_Status *status);

	int MPI_Start(MPI_Request *request);

	int MPI_Wait(MPI_Request *request, MPI_Status *status);

	int MPI_Waitall(int count, MPI_Request array_of_requests[],
	                MPI_Status array_of_statuses[]);

	int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root,
	              MPI_Comm comm);

	int MPI_Comm_rank(MPI_Comm comm, int *r);

	int MPI_Comm_size(MPI_Comm comm, int *r);

	int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm);

	int MPIX_Serialize_handles();
	int MPIX_Deserialize_handles();
	int MPIX_Serialize_handler_register(const MPIX_Serialize_handler
	                                    handler);
	int MPIX_Deserialize_handler_register(const MPIX_Deserialize_handler
	                                      handler);
	int MPIX_Checkpoint_write();
	int MPIX_Checkpoint_read();
	int MPIX_Get_fault_epoch(int *epoch);
	int MPI_Barrier(MPI_Comm comm);


	double MPI_Wtime();

	int MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler err);
	int MPI_Reduce(const void *s_buf, void *r_buf, int count,
	               MPI_Datatype type, MPI_Op op, int root, MPI_Comm comm);

	int MPI_Allreduce(const void *s_buf, void *r_buf, int count,
	                  MPI_Datatype type, MPI_Op op, MPI_Comm comm);

	int MPI_Get_count(MPI_Status *status, MPI_Datatype datatype, int *count);
	int MPI_Abort(MPI_Comm comm, int errorcode);
};

} // namespace exampi

#endif //...H
