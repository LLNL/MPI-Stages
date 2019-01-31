#include "engines/blockingprogress.h"

namespace exampi
{

BlockingProgress::BlockingProgress() : shutdown(false)
{
	// TODO remove this ASAP in here due to inheritance development
	// should be in global mpi state
	// creating MPI_COMM_WORLD and world group
	exampi::groups.push_back(group);
	communicator = new Comm(true, group, group);
	communicator->set_rank(exampi::rank);
	communicator->set_context(0, 1);
	exampi::communicators.push_back(communicator);
	
	// start progress threads
	// TODO fetch progress thread count from config
	//for(size_t tidx = 0; tidx < thread_num; ++tidx)
	for(size_t tidx = 0; tidx < 1; ++tidx)
	{
		// launch thread executing progress function
		std::thread thr(&BlockingProgress::progress, this);
		progress_threads.push_back(thr);
	}
}

BlockingProgress::~BlockingProgress()
{
	// join all threads
	shutdown = true;

	for(auto&& thr : this->progress_threads)
	{	
		thr.join();
	}

	// 
	// ...
}

void BlockingProgress::progress()
{
	// keep progress threads alive
	while(!this->shutdown)
	{
		// TODO remove
		std::this_thread::yield();

		// phase 1
		// matcher.progress()

		// phase 2
		// transport.receive messages
		
		// remove from network
		//exampi::transport->recv
		// receive from transport, ingest into matching
		//matcher->insert(msg from transport, in form of request?);
		
		// phase 3
		// walk protocol queue, find work do, do it
	}
}

// TODO remove ASAP
int BlockingProgress::init()
{
	return 0;
}

// TODO remove ASAP
int BlockingProgress::init(std::istream &t)
{
	return init();
}

// TODO remove ASAP
void BlockingProgress::finalize()
{
	// this should all be in MPI State
	// delete communicators
//	for(auto &&com : exampi::communicators)
//	{
//		delete com;
//	}
//	exampi::communicators.clear();
	
	// delete groups
//	for (auto &&group : exampi::groups)
//	{
//		delete group;
//	}
//	exampi::groups.clear();
}

int BlockingProgress::stop()
{
	// TODO what is this doing?
	// nullifying match list
//	for (auto &r : matchList)
//	{
//		(r)->unpack();
//		(r)->completionPromise.set_value( { .count = 0, .cancelled = 0,
//		                                    .MPI_SOURCE = (r)->source, .MPI_TAG = (r)->tag, .MPI_ERROR =
//		                                        MPIX_TRY_RELOAD });
//	}
//	matchList.clear();
//	unexpectedList.clear();
//	return 0;
	
	// this is done when we are cleaning up?
}

void BlockingProgress::cleanUp()
{
	sigHandler handler;
	handler.setSignalToHandle(SIGUSR1);

	Daemon &daemon = Daemon::get_instance();
	daemon.send_clean_up();

//	// TODO what is this?
//	matchLock.lock();
//	int size = matchList.size();
//	matchLock.unlock();
//	if (size > 0)
//	{
//		exampi::handler->setErrToZero();
//		exampi::BasicInterface::get_instance()->MPI_Send((void *) 0, 0, MPI_INT,
//		        exampi::rank, MPIX_CLEANUP_TAG, MPI_COMM_WORLD);
//		exampi::handler->setErrToOne();
//	}
}

int BlockingProgress::post_request(Request *request)
{
	// user thread relinquishes control here
	if(request->op == Op::Receive)
	{
		// insert into matching mechanism
		matcher->post(request);
	}
	else
	{
		protocol_queue.insert(request);
	}
}

//	// search unexpected message queue
//	debugpp("searching unexpected message queue");
//	unexpectedLock.lock();
//	matchLock.lock();
//	auto res = std::find_if(unexpectedList.begin(), unexpectedList.end(),
//	                        [tag,s, c, e](const std::unique_ptr<Request> &i) -> bool {i->unpack(); return i->tag == tag && i->source == s && i->stage == e && i->comm == c;});
//
//	//
//	if (res == unexpectedList.end())
//	{
//		debugpp("NO match in unexpectedList, push");
//
//		// put request into match list for later matching
//		unexpectedLock.unlock();
//		matchList.push_back(std::move(r));
//		matchLock.unlock();
//	}
//	else
//	{
//		// found in UMQ
//		matchLock.unlock();
//
//		debugpp("Found match in unexpectedList");
//
//		(*res)->unpack();
//		//memcpy(array.ptr, )
//		memcpy(array.getIovec().iov_base, (*res)->temp.iov_base,
//		       array.getIovec().iov_len);
//		(r)->completionPromise.set_value( { .count = (*res)->status.count, .cancelled = 0,
//		                                    .MPI_SOURCE = (*res)->source, .MPI_TAG = (*res)->tag, .MPI_ERROR = MPI_SUCCESS});
//		unexpectedList.erase(res);
//		unexpectedLock.unlock();
//
//	}
//
//	return result;
//}

int BlockingProgress::save(std::ostream &t)
{
//	//save group
//	int group_size = exampi::groups.size();
//	t.write(reinterpret_cast<char *>(&group_size), sizeof(int));
//	for (auto &g : exampi::groups)
//	{
//		int value = g->get_group_id();
//		t.write(reinterpret_cast<char *>(&value), sizeof(int));
//		value = g->get_process_list().size();
//		t.write(reinterpret_cast<char *>(&value), sizeof(int));
//		for (auto p : g->get_process_list())
//		{
//			t.write(reinterpret_cast<char *>(&p), sizeof(int));
//		}
//	}
//
//	//save communicator
//	int comm_size = exampi::communicators.size();
//	t.write(reinterpret_cast<char *>(&comm_size), sizeof(int));
//	for(auto &c : exampi::communicators)
//	{
//		int value = c->get_rank();
//		t.write(reinterpret_cast<char *>(&value), sizeof(int));
//		value = c->get_context_id_pt2pt();
//		t.write(reinterpret_cast<char *>(&value), sizeof(int));
//		value = c->get_context_id_coll();
//		t.write(reinterpret_cast<char *>(&value), sizeof(int));
//		bool intra = c->get_is_intra();
//		t.write(reinterpret_cast<char *>(&intra), sizeof(bool));
//		value = c->get_local_group()->get_group_id();
//		t.write(reinterpret_cast<char *>(&value), sizeof(int));
//		value = c->get_remote_group()->get_group_id();
//		t.write(reinterpret_cast<char *>(&value), sizeof(int));
//	}

	return MPI_SUCCESS;
}

int BlockingProgress::load(std::istream &t)
{
//	alive = true;
//	sendThread = std::thread { sendThreadProc, &alive, &outbox };
//	matchThread = std::thread { matchThreadProc, &alive, &matchList, &unexpectedList,
//	                            &matchLock, &unexpectedLock };
//
//	int comm_size, group_size;
//	int r, p2p, coll, id;
//	bool intra;
//	int num_of_processes;
//	std::list<int> ranks;
//	int rank;
//	exampi::Group *grp;
//	//restore group
//	t.read(reinterpret_cast<char *>(&group_size), sizeof(int));
//	while(group_size)
//	{
//		// todo heap allocation
//		grp = new exampi::Group();
//
//		t.read(reinterpret_cast<char *>(&id), sizeof(int));
//		grp->set_group_id(id);
//		t.read(reinterpret_cast<char *>(&num_of_processes), sizeof(int));
//		for (int i = 0; i < num_of_processes; i++)
//		{
//			t.read(reinterpret_cast<char *>(&rank), sizeof(int));
//			ranks.push_back(rank);
//		}
//		grp->set_process_list(ranks);
//		exampi::groups.push_back(grp);
//		group_size--;
//	}
//	//restore communicator
//	t.read(reinterpret_cast<char *>(&comm_size), sizeof(int));
//
//	while(comm_size)
//	{
//		exampi::Comm *com = new exampi::Comm();
//		t.read(reinterpret_cast<char *>(&r), sizeof(int));
//		com->set_rank(r);
//		t.read(reinterpret_cast<char *>(&p2p), sizeof(int));
//		t.read(reinterpret_cast<char *>(&coll), sizeof(int));
//		com->set_context(p2p, coll);
//		t.read(reinterpret_cast<char *>(&intra), sizeof(bool));
//		com->set_is_intra(intra);
//		t.read(reinterpret_cast<char *>(&id), sizeof(int));
//
//		auto it = std::find_if(exampi::groups.begin(),
//		                       exampi::groups.end(),
//		                       [id](const Group *i) -> bool {return i->get_group_id() == id;});
//		if (it == exampi::groups.end())
//		{
//			return MPIX_TRY_RELOAD;
//		}
//		else
//		{
//			com->set_local_group(*it);
//		}
//		t.read(reinterpret_cast<char *>(&id), sizeof(int));
//		it = std::find_if(exampi::groups.begin(), exampi::groups.end(),
//		                  [id](const Group *i) -> bool {return i->get_group_id() == id;});
//		if (it == exampi::groups.end())
//		{
//			return MPIX_TRY_RELOAD;
//		}
//		else
//		{
//			com->set_remote_group(*it);
//		}
//		exampi::communicators.push_back(com);
//		comm_size--;
//	}
	return MPI_SUCCESS;
}

} // exampi
