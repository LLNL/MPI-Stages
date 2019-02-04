#include "interfaces/interface.h"

#include "debug.h"
#include "daemon.h"
#include "universe.h"

namespace exampi
{

BasicInterface &BasicInterface::get_instance()
{
	static BasicInterface instance;

	return instance;
}

BasicInterface::BasicInterface() : request_pool(256)
{
	;
}

BasicInterface::~BasicInterface()
{
	;
}

int BasicInterface::MPI_Init(int *argc, char ***argv)
{
	debugpp("MPI_Init entered. argc=" << *argc);

	// XXX
	Universe& universe = Universe::get_root_universe();

	// check that exampi-mpiexec was used to launch the application
	if(std::getenv("EXAMPI_MONITORED") == NULL)
	{
		debugpp("Application was not launched with mpiexec.");
		return MPI_ERR_MPIEXEC;
	}

	debugpp("MPI_Init passed EXAMPI_LAUNCHED check.");

	debugpp("Taking rank to be arg " << std::string(std::getenv("EXAMPI_RANK")));
	
	//exampi::rank = std::stoi(std::string(std::getenv("EXAMPI_RANK")));

	universe.rank = std::stoi(std::string(std::getenv("EXAMPI_RANK")));

	//debugpp("Taking epoch config to be " << **argv);
	//exampi::epochConfig = std::string(std::getenv("EXAMPI_EPOCH_FILE"));

	universe.epoch_config = std::string(std::getenv("EXAMPI_EPOCH_FILE"));

	debugpp("Taking epoch to be " << std::string(std::getenv("EXAMPI_EPOCH")));

	//exampi::epoch = std::stoi(std::string(std::getenv("EXAMPI_EPOCH")));
	universe.epoch = std::stoi(std::string(std::getenv("EXAMPI_EPOCH")));

	//exampi::worldSize = std::stoi(std::string(std::getenv("EXAMPI_WORLD_SIZE")));	
	universe.world_size = std::stoi(std::string(std::getenv("EXAMPI_WORLD_SIZE")));	

	// TODO this initializes progress and transport
	recovery_code = exampi::checkpoint->load();

	// execute global barrier
	// this is so that P1 doesn't init and send before P0 is ready to recv
	//if(exampi::epoch == 0)
	if(universe.epoch == 0)
	{
		debugpp("Executing barrier" << exampi::rank);

		Daemon& daemon = Daemon::get_instance();
		daemon.barrier();
	}

	/* Checkpoint/restart
	 * errHandler handler;
	 * handler.setErrToHandle(SIGUSR2);
	 */

	debugpp("Finished MPI_Init with code: " << recovery_code);
	return recovery_code;
}

int BasicInterface::MPI_Finalize()
{
	serialize_handlers.clear();
	deserialize_handlers.clear();

	//exampi::transport->finalize();
	//exampi::progress->finalize();

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Request_free(MPI_Request *request)
{
	// TODO
	return -1;
}

int BasicInterface::MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
	// offload into persistent path
	debugpp("initiating persistent send path");
	int err;
	MPI_Request request;
	err = MPI_Send_init(buf, count, datatype, dest, tag, comm, &request);
	if(err != MPI_SUCCESS) return err;

	debugpp("starting request");
	err = MPI_Start(&request);
	if(err != MPI_SUCCESS) return err;

	debugpp("waiting for request");
	err = MPI_Wait(&request, MPI_STATUS_IGNORE);
	return err;
}

int BasicInterface::MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
	// offload into persistent path
	debugpp("initiating persistent recv path");
	int err;
	MPI_Request request;
	err = MPI_Recv_init(buf, count, datatype, source, tag, comm, &request);
	if(err != MPI_SUCCESS) return err;

	debugpp("starting request");
	err = MPI_Start(&request);
	if(err != MPI_SUCCESS) return err;

	debugpp("waiting for request");
	err = MPI_Wait(&request, MPI_STATUS_IGNORE);
	return err;
}

int BasicInterface::MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	// offload into persistent path
	debugpp("initiating persistent send path");
	int err;
	err = MPI_Send_init(buf, count, datatype, dest, tag, comm, request);
	if(err != MPI_SUCCESS) return err;

	debugpp("starting request");
	err = MPI_Start(request);
	return err;
}

int BasicInterface::MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request)
{
	// offload into persistent path
	debugpp("initiating persistent recv path");
	int err;
	err = MPI_Recv_init(buf, count, datatype, source, tag, comm, request);
	if(err != MPI_SUCCESS) return err;
	
	debugpp("starting request");
	err = MPI_Start(request);
	return err;
}

int BasicInterface::MPI_Sendrecv(const void *sendbuf, int sendcount,
                                 MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount,
                                 MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm,
                                 MPI_Status *status)
{
	// sanitize user input
	
//	if (exampi::handler->isErrSet())
//	{
//		return MPIX_TRY_RELOAD;
//	}
//
//	MPI_Request recvreq;
//	int rc = MPI_Irecv(recvbuf, recvcount, recvtype, source, recvtag, comm,
//	                   &recvreq);
//	if (rc != MPI_SUCCESS)
//	{
//		return MPIX_TRY_RELOAD;
//	}
//
//	rc = MPI_Send(sendbuf, sendcount, sendtype, dest, sendtag, comm);
//
//	if (rc != MPI_SUCCESS)
//	{
//		return MPIX_TRY_RELOAD;
//	}
//
//	MPI_Wait(&recvreq, status);
//
//	return MPI_SUCCESS;

	return -1;
}

//#############################################################################

int BasicInterface::construct_request(const void *buf, int count, MPI_Datatype datatype, int source, int dest, int tag, MPI_Comm comm, MPI_Request *request, Operation operation)
{
	// request generation
	debugpp("generating request object");
	// TODO allocate from Universe::request_pool
	
	Universe& universe = Universe::get_root_universe();

	if(Request_ptr req = universe.allocate_request())
		return MPI_ERR_INTERN;

	*request = reinterpret_cast<MPI_Request>(req);
	
	Comm *c = exampi::communicators.at(comm);
	int context = c->get_context_id_pt2pt();
	size_t szcount = count;

	// operation descriptor
	req->operation = operation;

	// envelope
	req->envelope.epoch = exampi::epoch;
	req->envelope.communicator = comm;
	req->envelope.source = exampi::rank;
	req->envelope.destination = dest;
	req->envelope.tag = tag;
	
	// data description
	// TODO req->datatype = datatype;
	req->payload.datatype = exampi::datatypes[datatype];
	req->payload.count = count;
	req->payload.buffer = buf;

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Bsend_init(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	// sanitize user input
	CHECK_BUFFER(buf);
	CHECK_COUNT(count);
	CHECK_DATATYPE(datatype);
	CHECK_RANK(dest);
	CHECK_TAG(tag);
	CHECK_COMM(comm);
	CHECK_STAGES_ERROR();

	return construct_request(buf, count, datatype, exampi::rank, dest, tag, comm, request, Operation::Bsend);
}

int BasicInterface::MPI_Rsend_init(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	// sanitize user input
	CHECK_BUFFER(buf);
	CHECK_COUNT(count);
	CHECK_DATATYPE(datatype);
	CHECK_RANK(dest);
	CHECK_TAG(tag);
	CHECK_COMM(comm);
	CHECK_STAGES_ERROR();

	return construct_request(buf, count, datatype, exampi::rank, dest, tag, comm, request, Operation::Bsend);
}

int BasicInterface::MPI_Ssend_init(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	// sanitize user input
	CHECK_BUFFER(buf);
	CHECK_COUNT(count);
	CHECK_DATATYPE(datatype);
	CHECK_RANK(dest);
	CHECK_TAG(tag);
	CHECK_COMM(comm);
	CHECK_STAGES_ERROR();

	return construct_request(buf, count, datatype, exampi::rank, dest, tag, comm, request, Operation::Ssend);
}

int BasicInterface::MPI_Send_init(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
	// sanitize user input
	CHECK_BUFFER(buf);
	CHECK_COUNT(count);
	CHECK_DATATYPE(datatype);
	CHECK_RANK(dest);
	CHECK_TAG(tag);
	CHECK_COMM(comm);
	CHECK_STAGES_ERROR();

	return construct_request(buf, count, datatype, exampi::rank, dest, tag, comm, request, Operation::Send);
}

int BasicInterface::MPI_Recv_init(const void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request) 
{
	// sanitize user input
	debugpp("sanitizing user input");
	CHECK_BUFFER(buf);
	CHECK_COUNT(count);
	CHECK_DATATYPE(datatype);
	CHECK_RANK(source);
	CHECK_TAG(tag);
	CHECK_COMM(comm);
	CHECK_STAGES_ERROR();

	return construct_request(buf, count, datatype, source, exampi::rank, tag, comm, request, Operation::Receive);
}

//#############################################################################

int BasicInterface::MPI_Start(MPI_Request *request)
{
	// sanitize user input
	CHECK_REQUEST(request);
	CHECK_STAGES_ERROR();

	// check active status
	// TODO
	//if(request->active)
	//{
	//	return MPI_ERR_REQUEST;
	//}

	// TODO if Bsend, copy over to buffer, we punish those who screw up
	// swap out buffer, so it is free to be reused

	// hand request to progress engine
	Request *req = reinterpret_cast<Request *>(*request);

	Universe& universe = Universe::get_root_universe();
	return universe::progress->post_request(req);
}

//int BasicInterface::MPI_Startall()
//{
//}

//#############################################################################

int BasicInterface::finalize_request(MPI_Request *request, Request *req, MPI_Status *status)
{
	// set status if required
	if(status != MPI_STATUS_IGNORE)
	{
		status->count = req->payload.count;
		status->cancelled = static_cast<int>(req->cancelled);
		status->MPI_SOURCE = req->envelope.source;
		status->MPI_TAG = req->envelope.tag;
		// TODO the request has a MPI_ERROR? It can't always succeed
		status->MPI_ERROR = MPI_SUCCESS; 
	}

	// for persistent request
	if(req->persistent)
	{
		req->active = false;
	}
	// otherwise deallocate and set to REQUEST_NULL
	else
	{
		Universe& universe = Universe::get_root_universe();
		universe.deallocate(req);

		// invalidate user MPI_Request handle
		*request = MPI_REQUEST_NULL;
	}

	return MPI_SUCCESS;
}

int BasicInterface::MPI_Wait(MPI_Request *request, MPI_Status *status)
{
	// sanitize user input
	debugpp("sanitizing user input");
	CHECK_REQUEST(request);
	CHECK_STATUS(status);
	CHECK_STAGES_ERROR();

	// MPI_REQUEST_NULL check
	if(request == MPI_REQUEST_NULL)
	{
		return MPI_SUCCESS;
	}

	// dereference MPI_Request -> Request
	Request *req = reinterpret_cast<Request *>(request);

	// inactive persistent request check
	if(req->persistent && !req->active)
	{
		return MPI_SUCCESS;
	}

	// NOTE if we want to do some polling execution before blocking
	// poll for counter_max cycles
	//size_t counter = 0; counter_max = 100;
	//while(!req->complete && (counter < counter_max))
	//	++counter;

	// if request is not complete
	if(!req->complete)
	{
		// wait for completion 
		std::unique_lock<std::mutex> lock(req->lock);

		// register condition variable
		req->condition = &thr_request_condition;

		// wait for completion
		thr_request_condition.wait(lock, [req] () -> bool {return req->complete;});

		lock.unlock();
	}
	// request is now definitely completed

	return finalize_request(request, req, status);

// TODO also for MPI_Test 
//	if (st.MPI_ERROR == MPIX_TRY_RELOAD)
//	{
//		debugpp("MPIX_TRY_RELOAD FOUND");
//
//		if(status != MPI_STATUS_IGNORE)
//			memmove(status, &st, sizeof(MPI_Status));
//
//		return MPIX_TRY_RELOAD;
//	}
//	else
//	{
//		if(status != MPI_STATUS_IGNORE)
//			memmove(status, &st, sizeof(MPI_Status));
//
//		return 0;
//	}
}

int BasicInterface::MPI_Waitall(int count, MPI_Request array_of_requests[],
                                MPI_Status array_of_statuses[])
{
//	// sanitize user input
//	CHECK_REQUEST(request);
//	CHECK_STATUS(status);
//
//	// mpi stages error check
//	// TODO CHECK_STAGES_ERROR();
//	if (exampi::handler->isErrSet())
//	{
//		return MPIX_TRY_RELOAD;
//	}
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
	return -1;
}

//int BasicInterface::MPI_Waitany()
//{
//}

int BasicInterface::MPI_Test(MPI_Request *request, int *flag, MPI_Status *status)
{
	// sanitize user input
	debugpp("sanitizing user input");
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
	debugpp("entered MPI_Comm_rank");
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}
	Comm *c = exampi::communicators.at(comm);

	*r = (c)->get_rank();
	return 0;
}

int BasicInterface::MPI_Comm_size(MPI_Comm comm, int *r)
{
	debugpp("entered MPI_Comm_size");

	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}

	Comm *c = exampi::communicators.at(comm);

	*r = (c)->get_local_group()->get_process_list().size();

	return 0;
}

int BasicInterface::MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}
	int rc;
	Comm *c = exampi::communicators.at(comm);

	int coll_context;
	int p2p_context;

	rc = (c)->get_next_context(&p2p_context, &coll_context);
	if (rc != MPI_SUCCESS)
	{
		return MPIX_TRY_RELOAD;
	}
	Comm *communicator;
	communicator = new Comm(true, (c)->get_local_group(), (c)->get_remote_group());
	communicator->set_rank((c)->get_rank());
	communicator->set_context(p2p_context, coll_context);
	exampi::communicators.push_back(communicator);
	auto it = std::find_if(exampi::communicators.begin(),
	                       exampi::communicators.end(),
	                       [communicator](const Comm *i) -> bool {return i->get_context_id_pt2pt() == communicator->get_context_id_pt2pt();});
	if (it == exampi::communicators.end())
	{
		return MPIX_TRY_RELOAD;
	}
	else
	{
		*newcomm = std::distance(exampi::communicators.begin(), it);
	}
	return MPI_SUCCESS;
}

int BasicInterface::MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler err)
{
	debugpp("entered comm error set");
	// This sets the signal handler for SIGUSR2
	// will call cleanup
	exampi::handler->setErrToHandle(SIGUSR2);
	return MPI_SUCCESS;
}

//#############################################################################

int BasicInterface::MPIX_Serialize_handles()
{
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}

	std::stringstream filename;
	filename << exampi::epoch - 1 << "." << exampi::rank << ".cp";
	std::ofstream t(filename.str(), std::ofstream::out | std::ofstream::app);

	if (!serialize_handlers.empty())
	{
		for (auto it : serialize_handlers)
		{
			MPIX_Handles handles;
			(*it)(&handles);
			t.write(reinterpret_cast<char *>(&handles.comm_size), sizeof(int));
			for (int i = 0; i < handles.comm_size; i++)
			{
				Comm *c = exampi::communicators.at(handles.comms[i]);
				int id = c->get_context_id_pt2pt();
				t.write(reinterpret_cast<char *>(&id), sizeof(int));
			}

			t.write(reinterpret_cast<char *>(&handles.group_size), sizeof(int));
			for (int i = 0; i < handles.group_size; i++)
			{
				Group *g = exampi::groups.at(handles.grps[i]);
				int id = g->get_group_id();
				t.write(reinterpret_cast<char *>(&id), sizeof(int));
			}
		}
	}
	t.close();

	return MPI_SUCCESS;
}

int BasicInterface::MPIX_Deserialize_handles()
{
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}
	std::stringstream filename;
	filename << exampi::epoch - 1 << "." << exampi::rank << ".cp";
	std::ifstream t(filename.str(), std::ifstream::in);

	long long int pos;
	t.read(reinterpret_cast<char *>(&pos), sizeof(long long int));
	t.seekg(pos);

	int size, id;

	if (!deserialize_handlers.empty())
	{
		for (auto iter : deserialize_handlers)
		{
			MPIX_Handles handles;
			t.read(reinterpret_cast<char *>(&size), sizeof(int));
			handles.comm_size = size;
			if (handles.comm_size > 0)
			{
				handles.comms = (MPI_Comm *)malloc(size * sizeof(MPI_Comm));
				for (int i = 0; i < size; i++)
				{
					t.read(reinterpret_cast<char *>(&id), sizeof(int));
					auto it = std::find_if(exampi::communicators.begin(),
					                       exampi::communicators.end(),
					                       [id](const Comm *i) -> bool {return i->get_context_id_pt2pt() == id;});
					if (it == exampi::communicators.end())
					{
						return MPIX_TRY_RELOAD;
					}
					else
					{
						handles.comms[i] = std::distance(exampi::communicators.begin(), it);
					}
				}
			}
			t.read(reinterpret_cast<char *>(&size), sizeof(int));
			handles.group_size = size;
			if (handles.group_size > 0)
			{
				handles.grps = (MPI_Group *)malloc(size * sizeof(MPI_Group));
				for (int i = 0; i < size; i++)
				{
					t.read(reinterpret_cast<char *>(&id), sizeof(int));
					auto it = std::find_if(exampi::groups.begin(),
					                       exampi::groups.end(),
					                       [id](const Group *i) -> bool {return i->get_group_id() == id;});
					if (it == exampi::groups.end())
					{
						return MPIX_TRY_RELOAD;
					}
					else
					{
						handles.grps[i] = std::distance(exampi::groups.begin(), it);
					}
				}
			}
			(*iter)(handles);
		}
	}

	t.close();
	return MPI_SUCCESS;
}

int BasicInterface::MPIX_Serialize_handler_register(const MPIX_Serialize_handler
        handler)
{
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}
	if (exampi::epoch == 0 && recovery_code == MPI_SUCCESS)
	{
		serialize_handlers.push_back(handler);
	}
	else if (exampi::epoch > 0 && recovery_code == MPIX_SUCCESS_RESTART)
	{
		serialize_handlers.push_back(handler);
	}

	return MPI_SUCCESS;
}

int BasicInterface::MPIX_Deserialize_handler_register(const
        MPIX_Deserialize_handler
        handler)
{
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}
	if (exampi::epoch == 0 && recovery_code == MPI_SUCCESS)
	{
		deserialize_handlers.push_back(handler);
	}
	else if (exampi::epoch > 0 && recovery_code == MPIX_SUCCESS_RESTART)
	{
		deserialize_handlers.push_back(handler);
	}

	return MPI_SUCCESS;
}

int BasicInterface::MPIX_Checkpoint_write()
{
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}

	exampi::checkpoint->save();
	return MPI_SUCCESS;
}

int BasicInterface::MPIX_Checkpoint_read()
{
	if (exampi::handler->isErrSet())
	{
		exampi::handler->setErrToZero();
	}

	debugpp("commit epoch received" << exampi::epoch);

	// wait for restarted process
	Daemon& daemon = Daemon::get_instance();
	daemon.barrier();

	return MPI_SUCCESS;
}

int BasicInterface::MPIX_Get_fault_epoch(int *epoch)
{
	debugpp("entered MPIX_Get_fault_epoch");

	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}

	*epoch = exampi::epoch;
	return MPI_SUCCESS;
}

//#############################################################################

int BasicInterface::MPI_Barrier(MPI_Comm comm)
{
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}
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
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}
	int mask, comm_size, peer, peer_rank, peer_rel_rank, rc;
	int rank, rel_rank;

	MPI_Status status;
	Datatype *datatype;
	size_t bufsize;
	void *buf;

	if (count == 0) return MPI_SUCCESS;

	size_t szcount = count;
	datatype = &(exampi::datatypes[type]);
	bufsize = datatype->getExtent() * szcount;
	memcpy(r_buf, s_buf, bufsize);

	buf = malloc((size_t) bufsize);

	funcType::const_iterator iter = functions.find(op);

	rank = exampi::rank;
	comm_size = exampi::worldSize;
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
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}
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
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}

	// todo this is an implementation, and therefore should be in progress engine
	int rc;
	if (exampi::rank == root)
	{
		for (int i = 0; i < exampi::worldSize; i++)
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
	if (exampi::handler->isErrSet())
	{
		return MPIX_TRY_RELOAD;
	}
	exampi::Datatype type = exampi::datatypes[datatype];
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
	// NOTE is this really all that is required?
	// daemon abort

	exit(-1);
	return errorcode;
}

double BasicInterface::MPI_Wtime()
{
	// TODO move this into a macro
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
