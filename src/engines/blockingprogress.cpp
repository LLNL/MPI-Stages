#include "engines/blockingprogress.h"

namespace exampi
{

BlockingProgress::BlockingProgress(std::shared_ptr<Matcher> matcher, std::shared_ptr<Transport> transporter) : 
	shutdown(false),
	matcher(matcher),
	transporter(transporter)
{
	// TODO fetch progress thread count from config
	//for(size_t tidx = 0; tidx < thread_num; ++tidx)
	for(size_t tidx = 0; tidx < 1; ++tidx)
	{
		// launch thread executing progress function
		std::thread thr(&BlockingProgress::progress, this);
		progress_threads.push_back(std::move(thr));
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
}

int BlockingProgress::post_request(Request *request)
{
	// user threads relinquishes control here
	if(request->operation == Operation::Receive)
	{
		// insert into matching mechanism
		int err = matcher->post(request);
		// TODO check err
	}
	else // Send
	{
		std::lock_guard<std::mutex> lock(outbox_guard);
	
		outbox.push();
	}
	
	// NOTE later on there will be others here

	// TODO not meaningful
	return MPI_SUCCESS;
}

void BlockingProgress::progress()
{
	// keep progress threads alive
	while(!this->shutdown)
	{
		// absorb message if any
		// TODO should we use virtual dispatch
		if(std::unique_ptr<ProtocolMessage> message_in = transporter.absorb())
		{
			int err = handle_message(std::move(message_in));
			// TODO handle error
		}
		// match message if any
		else if(matcher.has_work())
		{
			int err = matcher.progress()
			// TODO handle error
		}
		// emit message if any
		else if(outbox.size() > 0)
		{
			int err = emit_message();
			// TODO handle error
		}
		// otherwise go to sleep
		else
		{
			std::this_thread::yield();
		}	
	}
}

int BlockingProgress::handle_message(std::unique_ptr<ProtocolMessage> message)
{
	// act on protocol message
	// continue protocol, match, ...
	// TODO
	// rendevouz protocol?
	// RTA, RAT, TA

	// THIS IS THE BIG SWTICH OR MAP
}

int BlockingProgress::emit_message()
{
	std::unique_lock<std::mutex> lock(outbox_guard);

	// check again that there is work
	if(outbox.size() == 0)
	{
		return MPI_SUCCESS;
	}
	// emit message
	else
	{
		// fetch request
		Request *request = outbox.front();
		outbox.pop();
		// TODO what about rendevouz?

		lock.unlock();

		// request -> protocol message
		//std::unique_ptr<ProtocolMessage> message_out(request->envelope);
		// TODO

		// send protocol message
		if(int err = transporter.send(message_out))
		{
			// TODO handle possible error, signal via request
		}
	}
}

//	// this is done when we are cleaning up?
//	// nullifying match list
////	for (auto &r : matchList)
////	{
////		(r)->unpack();
////		(r)->completionPromise.set_value( { .count = 0, .cancelled = 0,
////		                                    .MPI_SOURCE = (r)->source, .MPI_TAG = (r)->tag, .MPI_ERROR =
////		                                        MPIX_TRY_RELOAD });
////	}
////	matchList.clear();
////	unexpectedList.clear();
////	return 0;
//	

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
