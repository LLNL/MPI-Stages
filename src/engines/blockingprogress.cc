#include "engines/blockingprogress.h"

namespace exampi
{

BlockingProgress::BlockingProgress() :
	BlockingProgress(
		std::shared_ptr<Matcher>(new SimpleMatcher()),
		std::shared_ptr<Transport>(new UDPTransport())
	)
{
	;
}

BlockingProgress::BlockingProgress(std::shared_ptr<Matcher> matcher, std::shared_ptr<Transport> transporter) : 
	shutdown(false),
	maximum_progress_cycles(10),
	matcher(matcher),
	transporter(transporter)
{
	// todo fetch progress thread count from config
	//for(size_t tidx = 0; tidx < thread_num; ++tidx)
	for(size_t tidx = 0; tidx < 1; ++tidx)
	{
		debug("starting progress thread " << tidx);
		// launch thread executing progress function
		std::thread thr(&BlockingProgress::progress, this);
		progress_threads.push_back(std::move(thr));
	}
	debug("progress threads running");
}

BlockingProgress::~BlockingProgress()
{
	// join all threads
	debug("halting all progress threads");
	shutdown = true;

	for(auto&& thr : this->progress_threads)
	{	
		thr.join();
	}

	// this is in progress::stop
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
}

int BlockingProgress::post_request(Request *request)
{
	debug("posting request");

	// user threads relinquishes control here
	if(request->operation == Operation::Receive)
	{
		// insert into matching mechanism
		matcher->post_request(request);

		debug("handed off to matcher");
	}
	else // *sends
	{
		// acquire safety
		std::lock_guard<std::mutex> lock(outbox_guard);
	
		// queue for send
		outbox.push(request);

		debug("pushed into outbox");
	}

	return MPI_SUCCESS;
	
	// note later on there will be others here
	// collectives, rma
	// allreduce, allgather, reduce, broadcast, ...
	// put, get, atomic
}

void BlockingProgress::progress()
{
	// debug_add_thread()

	int cycles = 0;

	// keep progress threads alive
	while(!this->shutdown)
	{
		// note this is actually just a slow poll
		// 		blocking would be woken up by post_request, transporter ordered_recv()
		
		// absorb message if any, this is inflow
		if(ProtocolMessage_uptr msg = transporter->ordered_recv())
		{
			debug("progress thread, receiving message");

			matcher->post_message(std::move(msg));
		}

		// match message if any, this is inflow
		else if(Match match; matcher->progress(match))
		//else if(auto [match, request, msg] = matcher->progress())
		{
			debug("progress thread, matched message -> handling match");

			int err = handle_match(match);
			if(err != MPI_SUCCESS)
			{
				// TODO handle error
				debug("error receiving message");
			}
		}

		// emit message if any, this is outflow
		else if(outbox.size() > 0)
		{
			debug("progress thread, handling request");

			int err = handle_request();
			if(err != MPI_SUCCESS)
			{
				// TODO handle error
				debug("error sending message");
			}

			debug("sent message");
		}

		// otherwise go to sleep
		else
		{
			cycles = 0;	
			// TODO record number of sleep cycles for debugging since last work
			std::this_thread::yield();
		}
		
		// if we are doing work, check work amount
		cycles += 1;
		if(cycles >= maximum_progress_cycles)
		{
			debug("progress thread, going to sleep forcefully");

			cycles = 0;
			std::this_thread::yield();
		}
	}
}

int BlockingProgress::handle_match(Match &match)
{
	// note this is the big switch or dictionary protocol handling

	debug("handling matched request <-> protocol message");

	// TODO put this into a dictionary
	int err = -1;
	switch(match.message->stage)
	{
	case Protocol::EAGER_ACK:
		{
			debug("protocol message: EAGER_ACK");

			// return ACK message
			ProtocolMessage_uptr msg(transporter->allocate_protocol_message());

			msg->envelope = match.request->envelope;
			msg->stage = Protocol::ACK;
			
			err = transporter->reliable_send(std::move(msg));
			if(err != MPI_SUCCESS)
				return err;

			[[fallthrough]];
		}

	case Protocol::EAGER:
		{
			debug("protocol message: EAGER");
			debug("req: " << match.request << " <-> message: " << match.message.get());

			err = match.message->unpack(match.request);

			debug("protocol message unpacked");

			// inform mpi user
			match.request->release();

			debug("request completed and released");
		}	
		break;

	case Protocol::ACK:
		{
			debug("protocol message: ACK");

			match.request->release();

			// protocol message will be automagically deallocated
			debug("request completed and released");
		}
		break;
	}

	return err;
}

int BlockingProgress::handle_request()
{
	std::unique_lock<std::mutex> lock(outbox_guard);

	// emit message if there is a request
	if(outbox.size() > 0)
	{
		debug("fetching request");

		// fetch request
		Request *request = outbox.front();
		outbox.pop();

		lock.unlock();

		// TODO this could be a decider object
		// request, universe -> protocol selection

		// handle_send  -> EAGER 
		// handle_bsend -> EAGER
		// handle_rsend -> EAGER
		// handle_ssend -> EAGER_ACK

		// TODO this is temporary, assume everything is MPI_Send
		return handle_send(request);
	}
	else
	{
		debug("false handle request");

		return MPI_SUCCESS;
	}
}

int BlockingProgress::handle_send(Request *request)
{
	// request -> ProtocolMessage
	debug("allocating protocol message from transport");
	ProtocolMessage_uptr message = transporter->allocate_protocol_message();

	// pack message
	debug("filling protocol message");
	message->stage = Protocol::EAGER;
	message->envelope = request->envelope;

	// pack protocol message
	int err = message->pack(request);
	if(err != MPI_SUCCESS)
		return err;

	debug("packed protocol message, sending");

	// send protocol message
	err = transporter->reliable_send(std::move(message));
	if(err != MPI_SUCCESS)
	{
		// TODO handle error
		debug("error sending message");
	}

	// complete request
	request->release();

	return MPI_SUCCESS;
}

void BlockingProgress::cleanUp()
{
//	sigHandler handler;
//	handler.setSignalToHandle(SIGUSR1);
//
//	Daemon &daemon = Daemon::get_instance();
//	daemon.send_clean_up();
//
//		// todo what is this?
////	matchLock.lock();
////	int size = matchList.size();
////	matchLock.unlock();
////	if (size > 0)
////	{
////		exampi::handler->setErrToZero();
////		exampi::BasicInterface::get_instance()->MPI_Send((void *) 0, 0, MPI_INT,
////		        exampi::rank, MPIX_CLEANUP_TAG, MPI_COMM_WORLD);
////		exampi::handler->setErrToOne();
////	}
}

int BlockingProgress::save(std::ostream &t)
{
//	// save all groups
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
//	// save all communicators
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
//
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
