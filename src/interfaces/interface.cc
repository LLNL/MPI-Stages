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

#include <sstream>
#include <cstring>
#include <mutex>
#include <memory>

#include "interfaces/interface.h"
#include "debug.h"
#include "daemon.h"
#include "universe.h"
#include "faulthandler.h"
#include "checks.h"
#include "exceptions.h"

#include "engines/blockingprogress.h"

namespace exampi
{

BasicInterface::BasicInterface()
{
	;
}

BasicInterface::~BasicInterface()
{
	;
}

int BasicInterface::MPI_Init(int *argc, char ***argv)
{
	debug("MPI_Init entered. argc=" << *argc);

	// check that exampi-mpiexec was used to launch the application
	if(std::getenv("EXAMPI_MONITORED") == NULL)
	{
		debug("Application was not launched with mpiexec.");
		return MPI_ERR_MPIEXEC;
	}
	debug("MPI_Init passed EXAMPI_LAUNCHED check.");

	Universe &universe = Universe::get_root_universe();
	universe.initialize();

	universe.progress = std::make_unique<BlockingProgress>();

	// MPI WORLD GROUP
	debug("generating world group");
	init_group();

	// MPI_COMM_WORLD
	debug("generating world communicator");
	init_communicator();

	debug("generating universe datatypes");
	init_datatype();

	// execute global barrier
	debug("executing daemon barrier " << universe.rank);

	Daemon &daemon = Daemon::get_instance();
	daemon.barrier();
	// todo if done for udp non-recv reason, then this should live in udp transport
	//      not all transports require a barrier
	//}

	// todo Nawrin?
	/* Checkpoint/restart
	 * errHandler handler;
	 * handler.setErrToHandle(SIGUSR2);
	 */

	//debug("Finished MPI_Init with code: " << recovery_code);
	return MPI_SUCCESS;
}

void BasicInterface::init_group()
{
	Universe &universe = Universe::get_root_universe();
	// NOTE this potentially becomes huge! millions++
	std::list<int> rankList;
	for(int idx = 0; idx < universe.world_size; ++idx)
		rankList.push_back(idx);

	universe.world_group = std::make_shared<Group>(rankList);
	universe.groups.push_back(universe.world_group);
}

void BasicInterface::init_communicator()
{
	Universe &universe = Universe::get_root_universe();
	universe.world_comm = std::make_shared<Comm>();

	universe.world_comm->is_intra = true;
	universe.world_comm->local = universe.world_group;
	universe.world_comm->remote = universe.world_group;
	universe.world_comm->set_rank(universe.rank);
	universe.world_comm->set_context(0, 1);

	universe.communicators.push_back(universe.world_comm);
}

void BasicInterface::init_datatype()
{
	Universe &universe = Universe::get_root_universe();
	universe.datatypes =
	{
		{ MPI_BYTE, 			Datatype(MPI_BYTE,           sizeof(unsigned char),  true,  true, true)},
		{ MPI_CHAR, 			Datatype(MPI_CHAR,           sizeof(char),           true,  true, true)},
#if 0
		{ MPI_WCHAR, Datatype(MPI_WCHAR,          sizeof(wchar_t),        true,  true, true)},
#endif
		{ MPI_UNSIGNED_CHAR, 	Datatype(MPI_UNSIGNED_CHAR,  sizeof(unsigned char),  true,  true, true)},
		{ MPI_SHORT,         	Datatype(MPI_SHORT,          sizeof(short),          true,  true, true)},
		{ MPI_UNSIGNED_SHORT,	Datatype(MPI_UNSIGNED_SHORT, sizeof(unsigned short), true,  true, true)},
		{ MPI_INT,           	Datatype(MPI_INT,            sizeof(int),            true,  true, true)},
		{ MPI_UNSIGNED_INT,  	Datatype(MPI_UNSIGNED_INT,   sizeof(unsigned int),   true,  true, true)},
		{ MPI_LONG,          	Datatype(MPI_LONG,           sizeof(long),           true,  true, true)},
		{ MPI_UNSIGNED_LONG, 	Datatype(MPI_UNSIGNED_LONG,  sizeof(unsigned long),  true,  true, true)},
		{ MPI_FLOAT,         	Datatype(MPI_FLOAT,          sizeof(float),          false, true, true)},
		{ MPI_DOUBLE,        	Datatype(MPI_DOUBLE,         sizeof(double),         false, true, true)},
		{ MPI_LONG_LONG_INT, 	Datatype(MPI_LONG_LONG_INT,  sizeof(long long int),  false, true, true)},
		{ MPI_LONG_LONG,     	Datatype(MPI_LONG_LONG,      sizeof(long long),      false, true, true)},
		{ MPI_FLOAT_INT,		Datatype(MPI_FLOAT_INT,		 sizeof(float_int_type), false, false, false)},
		{ MPI_LONG_INT,			Datatype(MPI_LONG_INT,		 sizeof(long_int_type),  false, false, false)},
		{ MPI_DOUBLE_INT,		Datatype(MPI_DOUBLE_INT,	 sizeof(double_int_type),false, false, false)},
		{ MPI_2INT,		    	Datatype(MPI_2INT,	 		 sizeof(int_int_type),   false, false, false)},
#if 0
		{ MPI_LONG_DOUBLE, Datatype(MPI_LONG_DOUBLE,    sizeof(long double),    false, true, true)},
#endif
	};
}

int BasicInterface::MPI_Initialized(int *flag)
{
	Universe &universe = Universe::get_root_universe();

	*flag = static_cast<int>(universe.initialized);

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Finalize()
{
	debug("MPI_Finalize");

	Universe &universe = Universe::get_root_universe();
	universe.finalize();

	debug("Finalize returns");

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Request_free(MPI_Request *request)
{
	// sanitize user input
	debug("sanitizing user input");
	CHECK_REQUEST(request);
	CHECK_STAGES_ERROR();

	Request_ptr req = reinterpret_cast<Request_ptr>(*request);

	// lock request
	std::unique_lock<std::mutex> lock(req->guard);

	// persistent path
	// TODO this needs to be considered more!
	if(!req->complete && req->persistent && req->active)
	{
		debug("marking request as freed, but active/incomplete");

		// marked as freed
		req->freed = true;
		// TODO where do we clean freed requests?
	}

	// complete or inactive request
	else
	{
		debug("deallocating request immediately");

		Universe &universe = Universe::get_root_universe();

		// MR 5/5/19 lock needs to be unlocked before freeing, otherwise we are writing to unallocated memory when freeing!
		lock.unlock();
		universe.deallocate_request(req);
	}

	// invalidate user MPI_Request handle
	debug("invalidating user MPI_Request handle: addr " << request);

	debug("request pointer " << req << " value 0x" << std::hex << ((long *)req)[0]);
	debug("request handle " << *request);
	*request = MPI_REQUEST_NULL;
	debug("request handle " << *request);
	debug("request pointer " << req << " value 0x" << std::hex << ((long *)req)[0]);

	return MPI_SUCCESS;
}

int BasicInterface::offload_persistent(const void *buf, int count,
                                       MPI_Datatype datatype, int rank, int tag, MPI_Comm comm, Operation operation,
                                       MPI_Request *request)
{
	// offload into persistent path
	int err;

	switch (operation)
	{
		case Operation::Send:
		{
			debug("MPI_Send_init");
			err = MPI_Send_init(buf, count, datatype, rank, tag, comm, request);
			break;
		}
		case Operation::Bsend:
		{
			debug("MPI_Bsend_init");
			err = MPI_Bsend_init(buf, count, datatype, rank, tag, comm, request);
			break;
		}
		case Operation::Ssend:
		{
			debug("MPI_Ssend_init");
			err = MPI_Ssend_init(buf, count, datatype, rank, tag, comm, request);
			break;
		}
		case Operation::Rsend:
		{
			debug("MPI_Rsend_init");
			err = MPI_Rsend_init(buf, count, datatype, rank, tag, comm, request);
			break;
		}
		case Operation::Receive:
		{
			debug("MPI_Recv_init");
			err = MPI_Recv_init(buf, count, datatype, rank, tag, comm, request);
			break;
		}
		default:
		{
			throw PersistentOffloadOperationError();
			break;
		}
	}

	if(err != MPI_SUCCESS) return err;

	// disable persistence
	Request_ptr req = reinterpret_cast<Request_ptr>(*request);
	req->persistent = false;

	debug("starting request");
	err = MPI_Start(request);
	if(err != MPI_SUCCESS)
	{
		int terr = MPI_Request_free(request);
	}

	return err;
}

int BasicInterface::offload_persistent_wait(const void *buf, int count,
        MPI_Datatype datatype, int rank, int tag, MPI_Comm comm, Operation operation)
{
	debug("offloading persistent path with wait");

	MPI_Request request;
	int err = offload_persistent(buf, count, datatype, rank, tag, comm, operation,
	                             &request);
	if(err != MPI_SUCCESS) return err;
	// request is garunteed to be cleaned up

	debug("waiting for request");
	err = MPI_Wait(&request, MPI_STATUS_IGNORE);

	if(err != MPI_SUCCESS)
	{
		debug("MPI_Wait completed with error " << err);
		MPI_Request_free(&request);
	}

	return err;
}

int BasicInterface::MPI_Send(const void *buf, int count, MPI_Datatype datatype,
                             int dest, int tag, MPI_Comm comm)
{
	debug("entry MPI_Send with buf " << buf << " count " << count << " dest " <<
	      dest << " tag " << tag);

	return offload_persistent_wait(buf, count, datatype, dest, tag, comm,
	                               Operation::Send);
}

int BasicInterface::MPI_Bsend(const void *buf, int count, MPI_Datatype datatype,
                              int dest, int tag, MPI_Comm comm)
{
	debug("entry MPI_Bsend with buf " << buf << " count " << count << " dest " <<
	      dest << " tag " << tag);

	return offload_persistent_wait(buf, count, datatype, dest, tag, comm,
	                               Operation::Bsend);
}

int BasicInterface::MPI_Ssend(const void *buf, int count, MPI_Datatype datatype,
                              int dest, int tag, MPI_Comm comm)
{
	debug("entry MPI_Ssend with buf " << buf << " count " << count << " dest " <<
	      dest << " tag " << tag);

	return offload_persistent_wait(buf, count, datatype, dest, tag, comm,
	                               Operation::Ssend);
}

int BasicInterface::MPI_Rsend(const void *buf, int count, MPI_Datatype datatype,
                              int dest, int tag, MPI_Comm comm)
{
	debug("entry MPI_Rsend with buf " << buf << " count " << count << " dest " <<
	      dest << " tag " << tag);

	return offload_persistent_wait(buf, count, datatype, dest, tag, comm,
	                               Operation::Rsend);
}

int BasicInterface::MPI_Recv(void *buf, int count, MPI_Datatype datatype,
                             int source, int tag, MPI_Comm comm, MPI_Status *status)
{
	debug("entry MPI_Recv with buf " << buf << " count " << count << " src " <<
	      source << " tag " << tag);

	return offload_persistent_wait(buf, count, datatype, source, tag, comm,
	                               Operation::Receive);
}

int BasicInterface::MPI_Sendrecv(const void *sendbuf, int sendcount,
                                 MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount,
                                 MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm,
                                 MPI_Status *status)
{
	MPI_Request recvreq;

	int rc = MPI_Irecv(recvbuf, recvcount, recvtype, source, recvtag, comm,
	                   &recvreq);
	if(MPI_SUCCESS != rc)
		return rc;

	rc = MPI_Send(sendbuf, sendcount, sendtype, dest, sendtag, comm);
	if(MPI_SUCCESS != rc)
		return rc;

	rc = MPI_Wait(&recvreq, status);

	return rc;
}

int BasicInterface::MPI_Isend(const void *buf, int count, MPI_Datatype datatype,
                              int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	debug("entry MPI_Isend with buf " << buf << " count " << count << " dest " <<
	      dest << " tag " << tag);

	return offload_persistent(buf, count, datatype, dest, tag, comm,
	                          Operation::Send, request);
}

int BasicInterface::MPI_Ibsend(const void *buf, int count,
                               MPI_Datatype datatype, int dest, int tag,
                               MPI_Comm comm, MPI_Request *request)
{
	debug("entry MPI_Ibsend with buf " << buf << " count " << count << " dest " <<
	      dest << " tag " << tag);

	return offload_persistent(buf, count, datatype, dest, tag, comm,
	                          Operation::Bsend, request);
}

int BasicInterface::MPI_Issend(const void *buf, int count,
                               MPI_Datatype datatype, int dest, int tag,
                               MPI_Comm comm, MPI_Request *request)
{
	debug("entry MPI_Issend with buf " << buf << " count " << count << " dest " <<
	      dest << " tag " << tag);

	return offload_persistent(buf, count, datatype, dest, tag, comm,
	                          Operation::Ssend, request);
}

int BasicInterface::MPI_Irsend(const void *buf, int count,
                               MPI_Datatype datatype, int dest, int tag,
                               MPI_Comm comm, MPI_Request *request)
{
	debug("entry MPI_Irsend with buf " << buf << " count " << count << " dest " <<
	      dest << " tag " << tag);

	return offload_persistent(buf, count, datatype, dest, tag, comm,
	                          Operation::Rsend, request);
}

int BasicInterface::MPI_Irecv(void *buf, int count, MPI_Datatype datatype,
                              int source, int tag, MPI_Comm comm, MPI_Request *request)
{
	debug("entry MPI_Irecv with buf " << buf << " count " << count << " src " <<
	      source << " tag " << tag);

	return offload_persistent(buf, count, datatype, source, tag, comm,
	                          Operation::Receive, request);
}

//#############################################################################

int BasicInterface::construct_request(const void *buf, int count,
                                      MPI_Datatype datatype, int source, int dest, int tag, MPI_Comm comm,
                                      MPI_Request *request, Operation operation)
{
	// request generation
	debug("generating request object");

	Universe &universe = Universe::get_root_universe();

	// todo store in a unique_ptr pool?
	// allows ownership to be internal, handle/alias to outside, we need to have C space
	Request_ptr req = universe.allocate_request();
	if(req == nullptr)
	{
		debug("ERROR: universe request allocation is nullptr, ptr " << req);
		return MPI_ERR_INTERN;
	}

	// assign user handle
	*request = reinterpret_cast<MPI_Request>(req);

	// find context of communicator
	std::shared_ptr<Comm> c = universe.communicators.at(comm);
	int context = c->get_context_id_pt2pt();

	// operation descriptor
	req->operation = operation;

	// envelope
	req->envelope.epoch = universe.epoch;
	req->envelope.context = context;
	req->envelope.source = source;
	req->envelope.destination = dest;
	req->envelope.tag = tag;

	// data description
	req->payload.datatype = &universe.datatypes[datatype];
	req->payload.count = count;
	req->payload.buffer = buf;

	// persistent
	req->persistent = true;
	req->active = false;

	debug("request object instantiated");

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Send_init(const void *buf, int count,
                                  MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	// sanitize user input
	CHECK_BUFFER(buf);
	CHECK_COUNT(count);
	CHECK_DATATYPE(datatype);
	CHECK_COMM(comm);
	CHECK_RANK(dest, comm);
	CHECK_TAG(tag);
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	return construct_request(buf, count, datatype, universe.rank, dest, tag, comm,
	                         request, Operation::Send);
}

int BasicInterface::MPI_Bsend_init(const void *buf, int count,
                                   MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	// sanitize user input
	CHECK_BUFFER(buf);
	CHECK_COUNT(count);
	CHECK_DATATYPE(datatype);
	CHECK_COMM(comm);
	CHECK_RANK(dest, comm);
	CHECK_TAG(tag);
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	return construct_request(buf, count, datatype, universe.rank, dest, tag, comm,
	                         request, Operation::Bsend);
}

int BasicInterface::MPI_Rsend_init(const void *buf, int count,
                                   MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	// sanitize user input
	CHECK_BUFFER(buf);
	CHECK_COUNT(count);
	CHECK_DATATYPE(datatype);
	CHECK_COMM(comm);
	CHECK_RANK(dest, comm);
	CHECK_TAG(tag);
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	return construct_request(buf, count, datatype, universe.rank, dest, tag, comm,
	                         request, Operation::Rsend);
}

int BasicInterface::MPI_Ssend_init(const void *buf, int count,
                                   MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	// sanitize user input
	CHECK_BUFFER(buf);
	CHECK_COUNT(count);
	CHECK_DATATYPE(datatype);
	CHECK_COMM(comm);
	CHECK_RANK(dest, comm);
	CHECK_TAG(tag);
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	return construct_request(buf, count, datatype, universe.rank, dest, tag, comm,
	                         request, Operation::Ssend);
}

int BasicInterface::MPI_Recv_init(const void *buf, int count,
                                  MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request)
{
	// sanitize user input
	debug("sanitizing user input");
	CHECK_BUFFER(buf);
	CHECK_COUNT(count);
	CHECK_DATATYPE(datatype);
	CHECK_COMM(comm);
	CHECK_RANK(source, comm);
	CHECK_TAG(tag);
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	return construct_request(buf, count, datatype, source, universe.rank, tag, comm,
	                         request, Operation::Receive);
}

//#############################################################################

int BasicInterface::MPI_Start(MPI_Request *request)
{
	// sanitize user input
	CHECK_REQUEST(request);
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	// hand request to progress engine
	// todo this is dereferencing already anyways, might as well dereference to unique pointer?
	// then hand ownership to progress? but progress doesn't own it
	debug("translating request");
	Request_ptr req = reinterpret_cast<Request_ptr>(*request);

	// check active request
	debug("persistent check");
	if(req->persistent && req->active)
	{
		debug("persistent request is active");
		return MPI_ERR_REQUEST;
	}

	debug("posting request to progress");

	// todo try catch as preprocessors for safe vs unsafe
	universe.progress->post_request(req);

	return MPI_SUCCESS;
}

//int BasicInterface::MPI_Startall()
//{
//}

//#############################################################################

int BasicInterface::finalize_request(MPI_Request *request, Request *req,
                                     MPI_Status *status)
{
	// set status if required
	if(MPI_STATUS_IGNORE != status)
	{
		status->count = req->payload.count;
		status->cancelled = static_cast<int>(req->cancelled);
		status->MPI_SOURCE = req->envelope.source;
		status->MPI_TAG = req->envelope.tag;
		status->MPI_ERROR = req->error;
	}

	// for persistent request
	if(req->persistent)
	{
		req->active = false;

		return MPI_SUCCESS;
	}

	// otherwise deallocate and set to REQUEST_NULL
	else
	{
		debug("finalizing request by freeing");
		return MPI_Request_free(request);
	}
}

int BasicInterface::MPI_Wait(MPI_Request *request, MPI_Status *status)
{
	// sanitize user input
	debug("sanitizing user input");
	CHECK_REQUEST(request);
	CHECK_STATUS(status);
	CHECK_STAGES_ERROR();

	// MPI_REQUEST_NULL check
	debug("checking " << request << " == MPI_REQUEST_NULL " << MPI_REQUEST_NULL);
	if(MPI_REQUEST_NULL == request)
	{
		debug("early exit due to MPI_REQUEST_NULL");
		return MPI_SUCCESS;
	}

	// dereference MPI_Request -> Request
	Request_ptr req = reinterpret_cast<Request_ptr>(*request);
	debug("translated MPI_Request to Request_ptr: " << req);

	// inactive persistent request chec
	debug("checking persistent " << req->persistent << " and inactive " <<
	      req->active);
	if(req->persistent && !req->active)
	{
		debug("persistent and inactive request found");
		return MPI_SUCCESS;
	}

	// note if we want to do some polling execution before blocking
	// poll for counter_max cycles
	//size_t counter = 0; counter_max = 100;
	//while(!req->complete && (counter < counter_max))
	//while(!req->complete) {}
	//	++counter;

	// if request is not complete
	debug("checking if request complete " << req->complete);
	if(!req->complete)
	{
		debug("will wait for completion");

		// wait for completion
		std::unique_lock<std::mutex> lock(req->guard);

		debug("acquired lock on request");

		// register condition variable
		req->condition = &thr_request_condition;

		// wait for completion
		thr_request_condition.wait(lock, [req] () -> bool {return req->complete;});

		debug("finished waiting");

		lock.unlock();
	}
	// request is now definitely completed

	debug("finalizing request");
	return finalize_request(request, req, status);
}

int BasicInterface::MPI_Waitall(int count, MPI_Request array_of_requests[],
                                MPI_Status array_of_statuses[])
{
	// sanitize user input
	CHECK_REQUEST(request);
	CHECK_STATUS(status); // mpi stages error check
	CHECK_STAGES_ERROR();

	if (array_of_statuses != MPI_STATUSES_IGNORE)
	{
		for (int i = 0; i < count; i++)
		{
			if (array_of_requests[i])
			{
				array_of_statuses[i].MPI_ERROR = MPI_Wait(array_of_requests + i,
				                                 array_of_statuses + i);

				if (array_of_statuses[i].MPI_ERROR)
					return array_of_statuses[i].MPI_ERROR;
			}
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			int rc = MPI_Wait(array_of_requests + i, MPI_STATUS_IGNORE);

			if(rc != MPI_SUCCESS)
				return rc;
		}
	}

	return MPI_SUCCESS;
}

//int BasicInterface::MPI_Waitany()
//{
//}

int BasicInterface::MPI_Test(MPI_Request *request, int *flag,
                             MPI_Status *status)
{
	// sanitize user input
	debug("sanitizing user input");
	CHECK_REQUEST(request);
	CHECK_STATUS(status);
	CHECK_STAGES_ERROR();

	// MPI_REQUEST_NULL check
	if(request == MPI_REQUEST_NULL)
	{
		*flag = true;

		return MPI_SUCCESS;
	}

	// dereference MPI_Request -> Request
	Request *req = reinterpret_cast<Request *>(request);

	// inactive persistent request check
	if(req->persistent && !req->active)
	{
		*flag = true;

		return MPI_SUCCESS;
	}

	// if request is not complete
	if(!req->complete)
	{
		*flag = 0;

		return MPI_SUCCESS;
	}
	// if request is complete
	else
	{
		*flag = 1;

		return finalize_request(request, req, status);
	}
}

//int BasicInterface::MPI_Testall()
//{
//}

//int BasicInterface::MPI_Testany()
//{
//}

//#############################################################################

int BasicInterface::MPI_Comm_rank(MPI_Comm comm, int *r)
{
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	// communicator handle -> communicator object
	std::shared_ptr<Comm> c = universe.communicators.at(comm);
	*r = c->get_rank();

	debug("called MPI_Comm_rank: " << *r);

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Comm_size(MPI_Comm comm, int *size)
{
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	// communicator handle -> communicator object
	std::shared_ptr<Comm> c = universe.communicators.at(comm);
	*size = c->get_local_group()->get_process_list().size();

	debug("called MPI_Comm_size: " << *size);

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	// get communicator object
	int rc;
	std::shared_ptr<Comm> c = universe.communicators.at(comm);

	int coll_context;
	int p2p_context;

	// get next context id
	rc = c->get_next_context(&p2p_context, &coll_context);
	if (rc != MPI_SUCCESS)
	{
		return MPIX_TRY_RELOAD;
	}

	// create new communicator object
	std::shared_ptr<Comm> communicator;
	communicator = std::make_shared<Comm>(true, c->get_local_group(),
	                                      c->get_remote_group());

	communicator->set_rank(c->get_rank());
	communicator->set_context(p2p_context, coll_context);

	// register duplicate communicator
	universe.communicators.push_back(communicator);

	// returns the handle of the communicator
	auto it = std::find_if(universe.communicators.begin(),
	                       universe.communicators.end(),
	                       [communicator](const std::shared_ptr<Comm> i) -> bool {return i->get_context_id_pt2pt() == communicator->get_context_id_pt2pt();});

	if (it == universe.communicators.end())
	{
		return MPIX_TRY_RELOAD;
	}
	else
	{
		*newcomm = std::distance(universe.communicators.begin(), it);
	}

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler err)
{

	debug("entered comm error set");

	Universe &universe = Universe::get_root_universe();

	FaultHandler &faulthandler = FaultHandler::get_instance();
	// todo should this be per Comm?
	faulthandler.setErrToHandle(SIGUSR2);

	debug("set error handler");

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Barrier(MPI_Comm comm)
{
	CHECK_STAGES_ERROR();

	int rank, size;
	MPI_Status status;
	int coll_tag = 0;
	int rc;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);
	int msg = 1;
	if (rank == 0)
	{
		rc = MPI_Send(&msg, 1, MPI_INT, (rank + 1) % size, coll_tag, comm);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Recv(&msg, 1, MPI_INT, (rank + size - 1) % size, coll_tag,
		              comm, &status);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Send(&msg, 1, MPI_INT, (rank + 1) % size, coll_tag, comm);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Recv(&msg, 1, MPI_INT, (rank + size - 1) % size, coll_tag,
		              comm, &status);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
	}
	else
	{
		rc = MPI_Recv(&msg, 1, MPI_INT, (rank + size - 1) % size, coll_tag,
		              comm, &status);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Send(&msg, 1, MPI_INT, (rank + 1) % size, coll_tag, comm);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Recv(&msg, 1, MPI_INT, (rank + size - 1) % size, coll_tag,
		              comm, &status);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Send(&msg, 1, MPI_INT, (rank + 1) % size, coll_tag, comm);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
	}
	debug("End of MPI_Barrier");
	return MPI_SUCCESS;
}

int BasicInterface::MPI_Reduce(const void *s_buf, void *r_buf, int count,
                               MPI_Datatype type, MPI_Op op, int root, MPI_Comm comm)
{

	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	int mask, comm_size, peer, peer_rank, peer_rel_rank, rc;
	int rank, rel_rank;

	MPI_Status status;
	Datatype *datatype;
	size_t bufsize;
	void *buf;

	if (count == 0) return MPI_SUCCESS;

	size_t szcount = count;
	datatype = &(universe.datatypes[type]);
	bufsize = datatype->get_extent() * szcount;
	memcpy(r_buf, s_buf, bufsize);

	buf = malloc((size_t) bufsize);

	funcType::const_iterator iter = functions.find(op);

	rank = universe.rank;
	comm_size = universe.world_size;
	if (comm_size == 1) return MPI_SUCCESS;

	rel_rank = (rank - root + comm_size) % comm_size;

	for (mask = 1; mask < comm_size; mask <<= 1)
	{
		peer_rel_rank = rel_rank ^ mask;
		if (peer_rel_rank >= comm_size) continue;
		peer_rank = (peer_rel_rank + root) % comm_size;
		if (peer_rank == root)
		{
			peer = root;
		}
		else
		{
			peer = peer_rank;
		}
		if (rel_rank < peer_rel_rank)
		{
			rc = MPI_Recv(buf, count, type, peer, 0, comm, &status);
			if (rc == MPIX_TRY_RELOAD) return rc;
			(*iter->second)(buf, r_buf, &count, &type);
		}
		else
		{
			rc = MPI_Send(r_buf, count, type, peer, 0, comm);
			if (rc == MPIX_TRY_RELOAD) return rc;
			break;
		}
	}
	free(buf);
	return MPI_SUCCESS;
}

int BasicInterface::MPI_Allreduce(const void *s_buf, void *r_buf, int count,
                                  MPI_Datatype type, MPI_Op op, MPI_Comm comm)
{
	debug("Starting MPI_Allreduce");
	CHECK_STAGES_ERROR();

	int rc = MPI_Reduce(s_buf, r_buf, count, type, op, 0, comm);
	if (rc != MPI_SUCCESS)
	{
		return rc;
	}
	rc = MPI_Bcast(r_buf, count, type, 0, comm);

	return rc;
}

int BasicInterface::MPI_Bcast(void *buf, int count, MPI_Datatype datatype,
                              int root,
                              MPI_Comm comm)
{
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	// todo this is an implementation, and therefore should be in progress engine
	int rc;
	if (universe.rank == root)
	{
		for (int i = 0; i < universe.world_size; i++)
			if (i != root)
			{
				rc = MPI_Send(buf, count, datatype, i, 0, comm);
				if (rc != MPI_SUCCESS)
					return MPIX_TRY_RELOAD;
			}
	}
	else
	{
		MPI_Status st;
		rc = MPI_Recv(buf, count, datatype, root, 0, comm, &st);
		if (rc != MPI_SUCCESS)
			return MPIX_TRY_RELOAD;
	}

	return MPI_SUCCESS;
}

//#############################################################################

int BasicInterface::MPI_Get_count(MPI_Status *status, MPI_Datatype datatype,
                                  int *count)
{
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();

	Datatype type = universe.datatypes[datatype];
	if (type.get_extent())
	{
		*count = status->count / type.get_extent();
		if (status->count >= 0
		        && *count * type.get_extent() != static_cast<size_t>(status->count))
		{
			*count = MPI_UNDEFINED;
		}
	}
	else if (status->count == 0)
	{
		*count = 0;
	}
	else
	{
		*count = MPI_UNDEFINED;
	}
	return MPI_SUCCESS;
}

int BasicInterface::MPI_Abort(MPI_Comm comm, int errorcode)
{
	// early warning for head daemon
	Daemon &daemon = Daemon::get_instance();
	int err = daemon.abort();
	if(err != MPI_SUCCESS)
		return err;

	exit(errorcode);

	return MPI_ERR_ABORT;
}

double BasicInterface::MPI_Wtime()
{
	// todo move this into a macro
	double wtime;

	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = 0;

	clock_gettime(CLOCK_REALTIME, &t);
	wtime = t.tv_sec;
	wtime += t.tv_nsec/1.0e+9;

	return wtime;
}

}
