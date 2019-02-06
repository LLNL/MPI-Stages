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
	// user threads relinquishes control here
	if(request->operation == Operation::Receive)
	{
		// insert into matching mechanism
		matcher->post_request(request);
	}
	else // *sends
	{
		// acquire safety
		std::lock_guard<std::mutex> lock(outbox_guard);
	
		// queue for send
		outbox.push(request);
	}

	return MPI_SUCCESS;
	
	// NOTE later on there will be others here
	// collectives, rma
	// allreduce, reduce, scan, exscan, broadcast, ...
}

void BlockingProgress::progress()
{
	int cycles = 0;

	// keep progress threads alive
	while(!this->shutdown)
	{
		// NOTE this is actually just a slow poll
		// 		blocking would be woken up by post_request, transporter absorb()
		
		Match match;
		ProtocolMessage_uptr msg(nullptr);

		// absorb message if any, this is inflow
		if(transporter->peek(msg))
		{
			int err = handle_protocol_message(std::move(msg));
			// TODO handle error
		}

		// match message if any, this is inflow
		else if(matcher->has_work() && matcher->progress(match))
		{
			int err = handle_match(std::move(match));
			// TODO handle error
		}

		// emit message if any, this is outflow
		else if(outbox.size() > 0)
		{
			int err = handle_request();
			// TODO handle error
		}

		// otherwise go to sleep
		else
		{
			cycles = 0;	
			std::this_thread::yield();
		}
		
		// if we are doing work, check work amount
		cycles += 1;
		if(cycles >= maximum_progress_cycles)
		{
			cycles = 0;
			std::this_thread::yield();
		}
	}
}

int BlockingProgress::handle_protocol_message(ProtocolMessage_uptr message)
{
	Match match;

	// look for match
	if(matcher->match(std::move(message), match))
	{
		// TODO would need to fill the ProtocolMessage here
		// not good, we only want to fill once we need to with the request
		// in handle_match

		int err = handle_match(std::move(match));
		// TODO handle error
		return err;
	}
	
	return MPI_SUCCESS;
}

int BlockingProgress::handle_match(Match match)
{
	// act on protocol
	// unpack protocol message, extract protocol
	
	// we might still own a lock on transport here!

	// TODO
	// rendevouz protocol?
	// RTA, RAT, TA

	// THIS IS THE BIG SWTICH OR MAP
}

int BlockingProgress::handle_request()
{
	std::unique_lock<std::mutex> lock(outbox_guard);

	// emit message if there is a request
	if(outbox.size() > 0)
	{
		// fetch request
		Request *request = outbox.front();
		outbox.pop();

		lock.unlock();

		// decide protocol to use
		// handle_send
		// handle_bsend
		// handle_rsend
		// handle_ssend

		return handle_send(request);
	}
}

int BlockingProgress::handle_send(Request *request)
{
	// request -> ProtocolMessage
	ProtocolMessage_uptr message = transporter->allocate_protocol_message();

	// ASSUMPTION TODO whole request fits into a single protocolmessage

	// pack message
	message->stage = ProtocolStage::EAGER;
	message->envelope = request->envelope;
	// TODO pack data
	// eager, all in ProtocolMessage
	// eager_ack, all in ProtocolMessage, requires acknowledgement
	// rendevouz, announce size, request buffer

	// send protocol message
	if(int err = transporter->reliable_send(std::move(message)))
	{
		// TODO handle possible error, signal via request
	}
}


//void BlockingProgress::cleanUp()
//{
//	sigHandler handler;
//	handler.setSignalToHandle(SIGUSR1);
//
//	Daemon &daemon = Daemon::get_instance();
//	daemon.send_clean_up();
//
////	// TODO what is this?
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
//}
//
//int BlockingProgress::save(std::ostream &t)
//{
////	// save all groups
////	int group_size = exampi::groups.size();
////	t.write(reinterpret_cast<char *>(&group_size), sizeof(int));
////	for (auto &g : exampi::groups)
////	{
////		int value = g->get_group_id();
////		t.write(reinterpret_cast<char *>(&value), sizeof(int));
////		value = g->get_process_list().size();
////		t.write(reinterpret_cast<char *>(&value), sizeof(int));
////		for (auto p : g->get_process_list())
////		{
////			t.write(reinterpret_cast<char *>(&p), sizeof(int));
////		}
////	}
//
////	// save all communicators
////	int comm_size = exampi::communicators.size();
////	t.write(reinterpret_cast<char *>(&comm_size), sizeof(int));
////	for(auto &c : exampi::communicators)
////	{
////		int value = c->get_rank();
////		t.write(reinterpret_cast<char *>(&value), sizeof(int));
////		value = c->get_context_id_pt2pt();
////		t.write(reinterpret_cast<char *>(&value), sizeof(int));
////		value = c->get_context_id_coll();
////		t.write(reinterpret_cast<char *>(&value), sizeof(int));
////		bool intra = c->get_is_intra();
////		t.write(reinterpret_cast<char *>(&intra), sizeof(bool));
////		value = c->get_local_group()->get_group_id();
////		t.write(reinterpret_cast<char *>(&value), sizeof(int));
////		value = c->get_remote_group()->get_group_id();
////		t.write(reinterpret_cast<char *>(&value), sizeof(int));
////	}
//
//	return MPI_SUCCESS;
//}
//
//int BlockingProgress::load(std::istream &t)
//{
////	alive = true;
////	sendThread = std::thread { sendThreadProc, &alive, &outbox };
////	matchThread = std::thread { matchThreadProc, &alive, &matchList, &unexpectedList,
////	                            &matchLock, &unexpectedLock };
////
////	int comm_size, group_size;
////	int r, p2p, coll, id;
////	bool intra;
////	int num_of_processes;
////	std::list<int> ranks;
////	int rank;
////	exampi::Group *grp;
////	//restore group
////	t.read(reinterpret_cast<char *>(&group_size), sizeof(int));
////	while(group_size)
////	{
////		// todo heap allocation
////		grp = new exampi::Group();
////
////		t.read(reinterpret_cast<char *>(&id), sizeof(int));
////		grp->set_group_id(id);
////		t.read(reinterpret_cast<char *>(&num_of_processes), sizeof(int));
////		for (int i = 0; i < num_of_processes; i++)
////		{
////			t.read(reinterpret_cast<char *>(&rank), sizeof(int));
////			ranks.push_back(rank);
////		}
////		grp->set_process_list(ranks);
////		exampi::groups.push_back(grp);
////		group_size--;
////	}
////	//restore communicator
////	t.read(reinterpret_cast<char *>(&comm_size), sizeof(int));
////
////	while(comm_size)
////	{
////		exampi::Comm *com = new exampi::Comm();
////		t.read(reinterpret_cast<char *>(&r), sizeof(int));
////		com->set_rank(r);
////		t.read(reinterpret_cast<char *>(&p2p), sizeof(int));
////		t.read(reinterpret_cast<char *>(&coll), sizeof(int));
////		com->set_context(p2p, coll);
////		t.read(reinterpret_cast<char *>(&intra), sizeof(bool));
////		com->set_is_intra(intra);
////		t.read(reinterpret_cast<char *>(&id), sizeof(int));
////
////		auto it = std::find_if(exampi::groups.begin(),
////		                       exampi::groups.end(),
////		                       [id](const Group *i) -> bool {return i->get_group_id() == id;});
////		if (it == exampi::groups.end())
////		{
////			return MPIX_TRY_RELOAD;
////		}
////		else
////		{
////			com->set_local_group(*it);
////		}
////		t.read(reinterpret_cast<char *>(&id), sizeof(int));
////		it = std::find_if(exampi::groups.begin(), exampi::groups.end(),
////		                  [id](const Group *i) -> bool {return i->get_group_id() == id;});
////		if (it == exampi::groups.end())
////		{
////			return MPIX_TRY_RELOAD;
////		}
////		else
////		{
////			com->set_remote_group(*it);
////		}
////		exampi::communicators.push_back(com);
////		comm_size--;
////	}
//	return MPI_SUCCESS;
//}

} // exampi
