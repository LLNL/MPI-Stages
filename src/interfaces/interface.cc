#include <sstream>
#include <cstring>
#include <mutex>
#include <memory>

#include "interfaces/interface.h"
#include "debug.h"
#include "daemon.h"
#include "universe.h"
#include "faulthandler.h"

namespace exampi
{

BasicInterface &BasicInterface::get_instance()
{
	// todo get rid of interface singleton, should be part of universe
	static BasicInterface instance;

	return instance;
}

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

	// checkpoint load or initialize for first run
	// STAGES
	recovery_code = universe.checkpoint->load();
	if(recovery_code != MPI_SUCCESS)
	{
		debug("unsuccessful checkpoint load");
		return recovery_code;
	}

	// execute global barrier
	// todo mpi stages move to checkpoint
	if(universe.epoch == 0)
	{
		debug("executing daemon barrier " << universe.rank);

		Daemon &daemon = Daemon::get_instance();
		daemon.barrier();
		// todo if done for udp non-recv reason, then this should live in udp transport
		//      not all transports require a barrier
	}

	// todo Nawrin?
	/* Checkpoint/restart
	 * errHandler handler;
	 * handler.setErrToHandle(SIGUSR2);
	 */

	debug("Finished MPI_Init with code: " << recovery_code);
	return recovery_code;
}

int BasicInterface::MPI_Finalize()
{
	debug("MPI_Finalize");

	// todo mpi stages shut down?
	serialize_handlers.clear();
	deserialize_handlers.clear();

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Request_free(MPI_Request *request)
{
	// sanitize user input
	debug("sanitizing user input");
	CHECK_REQUEST(request);
	CHECK_STAGES_ERROR();

	// TODO garuntees
	// active can be freed, but no longer used in Wait or Test
	// active allowed to complete

	Request_ptr req = reinterpret_cast<Request_ptr>(*request);
	
	Universe &universe = Universe::get_root_universe();
	universe.deallocate_request(req);

	// invalidate user MPI_Request handle
	*request = MPI_REQUEST_NULL;

	return MPI_SUCCESS;
}

void BasicInterface::mark_hidden_persistent(MPI_Request *request)
{
	// mark as hidden persistence, delete on MPI_Wait 
	Request_ptr req = reinterpret_cast<Request_ptr>(*request);
	req->hidden_persistent = true;
}

int BasicInterface::offload_persistent(const void *buf, int count, MPI_Datatype datatype, int rank, int tag, MPI_Comm comm, Operation operation, MPI_Request *request)
{
	// offload into persistent path
	debug("persistent send path");
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

	mark_hidden_persistent(request);

	debug("starting request");
	err = MPI_Start(request);
	if(err != MPI_SUCCESS)
	{
		int terr = MPI_Request_free(request);
	}

	return err;
}

int BasicInterface::offload_persistent_wait(const void *buf, int count, MPI_Datatype datatype, int rank, int tag, MPI_Comm comm, Operation operation)
{
	debug("offloading persistent path with wait");

	MPI_Request request;
	int err = offload_persistent(buf, count, datatype, rank, tag, comm, operation, &request);
	if(err != MPI_SUCCESS) return err;
	// request is garunteed to be cleaned up

	debug("waiting for request");
	err = MPI_Wait(&request, MPI_STATUS_IGNORE);
	// request hidden persistent is freed

	if(err != MPI_SUCCESS)
	{
		MPI_Request_free(&request);
	}

	return err;
}

int BasicInterface::MPI_Send(const void *buf, int count, MPI_Datatype datatype,
                             int dest, int tag, MPI_Comm comm)
{
	return offload_persistent_wait(buf, count, datatype, dest, tag, comm, Operation::Send);
}

int BasicInterface::MPI_Bsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{	
	return offload_persistent_wait(buf, count, datatype, dest, tag, comm, Operation::Bsend);
}

int BasicInterface::MPI_Ssend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{	
	return offload_persistent_wait(buf, count, datatype, dest, tag, comm, Operation::Ssend);
}

int BasicInterface::MPI_Rsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{	
	return offload_persistent_wait(buf, count, datatype, dest, tag, comm, Operation::Rsend);
}

int BasicInterface::MPI_Recv(void *buf, int count, MPI_Datatype datatype,
                             int source, int tag, MPI_Comm comm, MPI_Status *status)
{
	return offload_persistent_wait(buf, count, datatype, source, tag, comm, Operation::Receive);
}

int BasicInterface::MPI_Sendrecv(const void *sendbuf, int sendcount,
                                 MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount,
                                 MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm,
                                 MPI_Status *status)
{
	MPI_Request recvreq;

	int rc = MPI_Irecv(recvbuf, recvcount, recvtype, source, recvtag, comm, &recvreq);
	rc = MPI_Send(sendbuf, sendcount, sendtype, dest, sendtag, comm);

	MPI_Wait(&recvreq, status);

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Isend(const void *buf, int count, MPI_Datatype datatype,
                              int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	return offload_persistent(buf, count, datatype, dest, tag, comm, Operation::Send, request);
}

int BasicInterface::MPI_Ibsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
           MPI_Comm comm, MPI_Request *request)
{
	return offload_persistent(buf, count, datatype, dest, tag, comm, Operation::Bsend, request);
}

int BasicInterface::MPI_Issend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
           MPI_Comm comm, MPI_Request *request)
{
	return offload_persistent(buf, count, datatype, dest, tag, comm, Operation::Ssend, request);
}

int BasicInterface::MPI_Irsend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
           MPI_Comm comm, MPI_Request *request)
{
	return offload_persistent(buf, count, datatype, dest, tag, comm, Operation::Rsend, request);
}

int BasicInterface::MPI_Irecv(void *buf, int count, MPI_Datatype datatype,
                              int source, int tag, MPI_Comm comm, MPI_Request *request)
{
	return offload_persistent(buf, count, datatype, source, tag, comm, Operation::Receive, request);
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
	req->payload.datatype = datatype;
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
	Universe &universe = Universe::get_root_universe();

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
	if(status != MPI_STATUS_IGNORE)
	{
		status->count = req->payload.count;
		status->cancelled = static_cast<int>(req->cancelled);
		status->MPI_SOURCE = req->envelope.source;
		status->MPI_TAG = req->envelope.tag;
		status->MPI_ERROR = req->error;
	}

	// for persistent request
	if(req->persistent && !req->hidden_persistent)
	{
		req->active = false;
	}
	// otherwise deallocate and set to REQUEST_NULL
	else
	{
		MPI_Request_free(request);
	}

	debug("returning with error: " << req->error);
	return req->error;
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
	if(request == MPI_REQUEST_NULL)
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
		debug("persitent and inactive request found");
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

//int BasicInterface::MPI_Waitall(int count, MPI_Request array_of_requests[],
//                                MPI_Status array_of_statuses[])
//{
//	// sanitize user input
//	CHECK_REQUEST(request);
//	CHECK_STATUS(status);//	// mpi stages error check
//	CHECK_STAGES_ERROR();
//
//	if (array_of_statuses != MPI_STATUSES_IGNORE)
//	{
//		for (int i = 0; i < count; i++)
//		{
//			if (array_of_requests[i])
//			{
//				array_of_statuses[i].MPI_ERROR = MPI_Wait(array_of_requests + i,
//				                                 array_of_statuses + i);
//				if (array_of_statuses[i].MPI_ERROR)
//					return -1;
//			}
//		}
//	}
//	else
//	{
//		for (int i = 0; i < count; i++)
//		{
//			int rc = MPI_Wait(array_of_requests + i, nullptr);
//			if (rc)
//				return rc;
//		}
//	}
//	return MPI_SUCCESS;
//	return -1;
//}

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

	// fetch root universe
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

	// fetch root universe
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

	int rc;
	std::shared_ptr<Comm> c = universe.communicators.at(comm);

	int coll_context;
	int p2p_context;

	rc = c->get_next_context(&p2p_context, &coll_context);
	if (rc != MPI_SUCCESS)
	{
		return MPIX_TRY_RELOAD;
	}

	std::shared_ptr<Comm> communicator;
	communicator = std::make_shared<Comm>(true, c->get_local_group(),
	                                      c->get_remote_group());

	communicator->set_rank(c->get_rank());
	communicator->set_context(p2p_context, coll_context);

	universe.communicators.push_back(communicator);

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
	// TODO should this be per Comm
	faulthandler.setErrToHandle(SIGUSR2);
	
	debug("set error handler");

	return MPI_SUCCESS;
}

//#############################################################################

int BasicInterface::MPIX_Serialize_handles()
{
//	CHECK_STAGES_ERROR();
//
//	Universe& universe = Universe::get_root_universe();
//
//	std::stringstream filename;
//	filename << universe.epoch - 1 << "." << universe.rank << ".cp";
//	std::ofstream t(filename.str(), std::ofstream::out | std::ofstream::app);
//
//	if (!serialize_handlers.empty())
//	{
//		for (auto it : serialize_handlers)
//		{
//			MPIX_Handles handles;
//			(*it)(&handles);
//			t.write(reinterpret_cast<char *>(&handles.comm_size), sizeof(int));
//			for (int i = 0; i < handles.comm_size; i++)
//			{
//				Comm *c = universe.communicators.at(handles.comms[i]);
//				int id = c->get_context_id_pt2pt();
//				t.write(reinterpret_cast<char *>(&id), sizeof(int));
//			}
//
//			t.write(reinterpret_cast<char *>(&handles.group_size), sizeof(int));
//			for (int i = 0; i < handles.group_size; i++)
//			{
//				Group *g = universe.groups.at(handles.grps[i]);
//				int id = g->get_group_id();
//				t.write(reinterpret_cast<char *>(&id), sizeof(int));
//			}
//		}
//	}
//	t.close();
//
//	return MPI_SUCCESS;
	return MPI_ERR_DISABLED;
}

int BasicInterface::MPIX_Deserialize_handles()
{
//	CHECK_STAGES_ERROR();
//
//	Universe& universe = Universe::get_root_universe();
//	std::stringstream filename;
//	filename << universe.epoch - 1 << "." << universe.rank << ".cp";
//	std::ifstream t(filename.str(), std::ifstream::in);
//
//	long long int pos;
//	t.read(reinterpret_cast<char *>(&pos), sizeof(long long int));
//	t.seekg(pos);
//
//	int size, id;
//
//	if (!deserialize_handlers.empty())
//	{
//		for (auto iter : deserialize_handlers)
//		{
//			MPIX_Handles handles;
//			t.read(reinterpret_cast<char *>(&size), sizeof(int));
//			handles.comm_size = size;
//			if (handles.comm_size > 0)
//			{
//				handles.comms = (MPI_Comm *)malloc(size * sizeof(MPI_Comm));
//				for (int i = 0; i < size; i++)
//				{
//					t.read(reinterpret_cast<char *>(&id), sizeof(int));
//					auto it = std::find_if(universe.communicators.begin(),
//					                       universe.communicators.end(),
//					                       [id](const Comm *i) -> bool {return i->get_context_id_pt2pt() == id;});
//					if (it == universe.communicators.end())
//					{
//						return MPIX_TRY_RELOAD;
//					}
//					else
//					{
//						handles.comms[i] = std::distance(universe.communicators.begin(), it);
//					}
//				}
//			}
//			t.read(reinterpret_cast<char *>(&size), sizeof(int));
//			handles.group_size = size;
//			if (handles.group_size > 0)
//			{
//				handles.grps = (MPI_Group *)malloc(size * sizeof(MPI_Group));
//				for (int i = 0; i < size; i++)
//				{
//					t.read(reinterpret_cast<char *>(&id), sizeof(int));
//					auto it = std::find_if(universe.groups.begin(),
//					                       universe.groups.end(),
//					                       [id](const Group *i) -> bool {return i->get_group_id() == id;});
//					if (it == universe.groups.end())
//					{
//						return MPIX_TRY_RELOAD;
//					}
//					else
//					{
//						handles.grps[i] = std::distance(universe.groups.begin(), it);
//					}
//				}
//			}
//			(*iter)(handles);
//		}
//	}
//
//	t.close();
//	return MPI_SUCCESS;
	return MPI_ERR_DISABLED;
}

int BasicInterface::MPIX_Serialize_handler_register(const MPIX_Serialize_handler
        handler)
{
//	CHECK_STAGES_ERROR();
//
//	Universe& universe = Universe::get_root_universe();
//	if (universe.epoch == 0 && recovery_code == MPI_SUCCESS)
//	{
//		serialize_handlers.push_back(handler);
//	}
//	else if (universe.epoch > 0 && recovery_code == MPIX_SUCCESS_RESTART)
//	{
//		serialize_handlers.push_back(handler);
//	}
//
//	return MPI_SUCCESS;
	return MPI_ERR_DISABLED;
}

int BasicInterface::MPIX_Deserialize_handler_register(const
        MPIX_Deserialize_handler
        handler)
{
//	CHECK_STAGES_ERROR();
//
//	Universe& universe = Universe::get_root_universe();
//	if (universe.epoch == 0 && recovery_code == MPI_SUCCESS)
//	{
//		deserialize_handlers.push_back(handler);
//	}
//	else if (universe.epoch > 0 && recovery_code == MPIX_SUCCESS_RESTART)
//	{
//		deserialize_handlers.push_back(handler);
//	}
//
//	return MPI_SUCCESS;
	return MPI_ERR_DISABLED;
}

int BasicInterface::MPIX_Checkpoint_write()
{
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();
	// STAGES
	universe.checkpoint->save();

	return MPI_SUCCESS;
}

int BasicInterface::MPIX_Checkpoint_read()
{
	// note no check since we are already in a fault
	//CHECK_STAGES_ERROR();

	debug("checkpoint_read");

	// note instead reset
	FaultHandler &faulthandler = FaultHandler::get_instance();
	if(faulthandler.isErrSet())
	{
		debug("resetting fault handler");
		faulthandler.setErrToZero();
	}

	Universe &universe = Universe::get_root_universe();
	debug("surviving process restart " << universe.rank);

	// wait for restarted process
	Daemon &daemon = Daemon::get_instance();

	daemon.wait_commit();
	debug("commit epoch received " << universe.epoch);

	// todo again why is there a barrier, due to transport? then should live in transport
	//      otherwise clarify and put in corrrect place instead of in interface, because
	//      the interface does not require it fundementally.
	debug("entering daemon barrier for restart");
	daemon.barrier();

	return MPI_SUCCESS;
}

int BasicInterface::MPIX_Get_fault_epoch(int *epoch)
{
	CHECK_STAGES_ERROR();

	// fetch universe
	Universe &universe = Universe::get_root_universe();
	*epoch = universe.epoch;
	debug("current epoch " << *epoch);

	return MPI_SUCCESS;
}

//#############################################################################

int BasicInterface::MPI_Barrier(MPI_Comm comm)
{
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();
	int rank, size;
	MPI_Status status;
	int coll_tag = 0;
	int rc;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);

	if (rank == 0)
	{
		rc = MPI_Send((void *) 0, 0, MPI_INT, (rank + 1) % size, coll_tag, comm);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Recv((void *) 0, 0, MPI_INT, (rank + size - 1) % size, coll_tag,
		              comm, &status);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Send((void *) 0, 0, MPI_INT, (rank + 1) % size, coll_tag, comm);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Recv((void *) 0, 0, MPI_INT, (rank + size - 1) % size, coll_tag,
		              comm, &status);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
	}
	else
	{
		rc = MPI_Recv((void *) 0, 0, MPI_INT, (rank + size - 1) % size, coll_tag,
		              comm, &status);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Send((void *) 0, 0, MPI_INT, (rank + 1) % size, coll_tag, comm);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Recv((void *) 0, 0, MPI_INT, (rank + size - 1) % size, coll_tag,
		              comm, &status);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
		rc = MPI_Send((void *) 0, 0, MPI_INT, (rank + 1) % size, coll_tag, comm);
		if (rc == MPIX_TRY_RELOAD)
		{
			return MPIX_TRY_RELOAD;
		}
	}
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
	bufsize = datatype->getExtent() * szcount;
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
	CHECK_STAGES_ERROR();

	Universe &universe = Universe::get_root_universe();
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
	if (type.getExtent())
	{
		*count = status->count / type.getExtent();
		if (status->count >= 0
		        && *count * type.getExtent() != static_cast<size_t>(status->count))
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
