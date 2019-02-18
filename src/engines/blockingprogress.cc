#include <cstring>

#include "engines/blockingprogress.h"
#include "errors.h"

namespace exampi
{

BlockingProgress::BlockingProgress() :
	BlockingProgress(
	    std::unique_ptr<Matcher>(new SimpleMatcher()),
	    std::unique_ptr<Transport>(new UDPTransport()),
	    std::unique_ptr<Decider>(new SimpleDecider())
	)
{
	;
}

BlockingProgress::BlockingProgress(std::unique_ptr<Matcher> matcher,
                                   std::unique_ptr<Transport> transporter,
                                   std::unique_ptr<Decider> decider) :
	shutdown(false),
	maximum_progress_cycles(10),
	matcher(std::move(matcher)),
	transporter(std::move(transporter)),
	decider(std::move(decider))
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
	if(!shutdown)
	{
		// join all threads
		debug("halting all progress threads");
		shutdown = true;

		debug("informed all threads to shutdown");
		for(auto &&thr : this->progress_threads)
		{
			thr.join();
		}

		debug("joined all threads");
	}
}

void BlockingProgress::post_request(Request *request)
{
	// user threads relinquishes control here
	if(request->operation == Operation::Receive)
	{
		// insert into matching mechanism
		matcher->post_request(request);

		debug("handed request Receive to matcher");

		return;
	}
	else if(request->operation == Operation::Bsend)
	{
		// note a bsend forces packing
		// note we do this in the user thread to ensure correctness

		// copy user buffer
		// todo size_t size = request->payload.datatype.size() * request->payload.count;
		size_t size = sizeof(int) * request->payload.count;
		void *tmp_buffer = malloc(size);

		void *err = std::memcpy(tmp_buffer, request->payload.buffer, size);
		if(err == nullptr)
		{
			throw BsendCopyError();
		}

		// replace request payload
		request->payload.buffer = tmp_buffer;
		// todo request->payload.datatype = ;
		// todo request->payload.count = ;
	}

	// acquire safety
	std::lock_guard<std::mutex> lock(outbox_guard);

	// queue for send
	outbox.push(request);

	debug("put *send request into outbox");
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
		if(Header *header = transporter->ordered_recv())
		{
			debug("progress thread, receiving message");

			matcher->post_header(header);
		}

		// match message if any, this is inflow
		else if(Match match; matcher->progress(match))
		//else if(auto [match, request, msg] = matcher->progress())
		{
			debug("progress thread, matched message -> handling match");

			handle_match(match);
		}

		// emit message if any, this is outflow
		else if(outbox.size() > 0)
		{
			debug("progress thread, handling request");

			handle_request();

			debug("sent message");
		}

		// otherwise go to sleep
		else
		{
			cycles = 0;
			// todo debug record number of sleep cycles for debugging since last work
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

void BlockingProgress::handle_match(Match &match)
{
	// note this is the big switch or dictionary protocol handling

	debug("handling matched request <-> protocol message");

	// todo put this into a dictionary/map
	int err = -1;
	//switch(match.message->stage)
	switch(match.header->protocol)
	{
		case Protocol::EAGER_ACK:
		{
			debug("protocol message: EAGER_ACK");

			// return ACK message
			transporter->reliable_send(Protocol::ACK, match.request);

			[[fallthrough]];
		}

		case Protocol::EAGER:
		{
			debug("protocol message: EAGER");

			transporter->fill(match.header, match.request);

			match.request->release();

			debug("request completed and released");
			break;
		}

		case Protocol::ACK:
		{
			debug("protocol message: ACK");

			match.request->release();

			debug("request completed and released");
			break;
		}
	}
}

void BlockingProgress::handle_request()
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

		// todo currently only sends are implemented here
		//      will require a switch statement on type of operation
		handle_send(request);
	}
	else
	{
		debug("false handle_request");
	}
}

void BlockingProgress::handle_send(Request *request)
{
	// request -> ProtocolMessage
	Universe &universe = Universe::get_root_universe();
	Protocol protocol = decider->decide(request, universe);

	// send message
	transporter->reliable_send(protocol, request);

	// complete request
	request->release();
}

int BlockingProgress::halt()
{
	// stop all progress threads
	debug("stopping all progress threads");
	shutdown = true;

	// todo mpi stages unify with deconstruction
	for(auto &&thr : this->progress_threads)
	{
		thr.join();
	}

	debug("joined all threads");

	int err = MPI_SUCCESS;
	// todo mpi stages
	//err = matcher->halt();
	matcher->halt();
	if(err != MPI_SUCCESS)
	{
		debug("failed to halt matcher");
		return err;
	}

	err = transporter->halt();
	if(err != MPI_SUCCESS)
	{
		debug("failed to halt transporter");
		return err;
	}

	return err;
}

int BlockingProgress::save(std::ostream &t)
{
	// delegate further
	int err = MPI_SUCCESS;

	// todo mpi stages save endpoints?
	//err = transporter->save(t);

	//err = matcher->save(t);

	return err;
}

int BlockingProgress::load(std::istream &t)
{
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
