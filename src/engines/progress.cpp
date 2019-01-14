#include "engines/progress.h"

namespace exampi
{

void BasicProgress::addEndpoints()
{
	// read in size
	Config &config = Config::get_instance();

	debugpp("BasicProgress addEngpoints " << config["size"]);
	//int size = std::stoi(config["size"]);
	int size = std::stoi(std::string(std::getenv("EXAMPI_WORLD_SIZE")));

	// read in endpoints
	std::vector < std::string > elem;
	std::list<int> rankList;
	for (int i = 0; i < size; i++)
	{
		// TODO something breaks!
		elem.clear();
		rankList.push_back(i);

		std::string rank = std::to_string(i);

		std::string remote = config[rank];
		debugpp(remote);

		size_t beg = remote.find_first_of(":");
		std::string port = remote.substr(beg+1);
		std::string ip = remote.substr(0, beg); 
		debugpp("ip " << ip);

		debugpp("ports " << port);

		elem.push_back(ip);
		elem.push_back(port);

		exampi::transport->addEndpoint(i, elem);
	}
	group = new Group(rankList);
}

void BasicProgress::sendThreadProc(bool *alive, AsyncQueue<Request> *outbox)
{
	debug_add_thread("send");
	debugpp("Launching sendThreadProc(...)");

	while (*alive)
	{
		debugpp("sendThread: fetching promise");
		std::unique_ptr<Request> r = outbox->promise().get();
		debugpp("sendThread:  got result from outbox future");

		// send message to remote in this thread
		exampi::transport->send(r->getIovecs(), r->endpoint.rank, 0);
		debugpp("sendThread: sent message");
		// TODO:  check that sending actually completed
		r->completionPromise.set_value( { .count = 0, .cancelled = 0,
		                                  .MPI_SOURCE = r->source, .MPI_TAG = r->tag, .MPI_ERROR = MPI_SUCCESS });
		// let r drop scope and die (unique_ptr)
		debugpp("sendThread: completed message");
	}
}

void BasicProgress::matchThreadProc(bool *alive,
                                    std::list<std::unique_ptr<Request>> *matchList,
                                    std::list<std::unique_ptr<Request>> *unexpectedList,
                                    std::mutex *matchLock, std::mutex *unexpectedLock)
{
	debug_add_thread("match");
	debugpp("Launching matchThreadProc(...)");

	while (*alive)
	{
		debugpp("matchThread: before request");
		// TODO this is a malloc!
		std::unique_ptr<Request> r = make_unique<Request>();
		debugpp("matchThread:  made request, about to peek...");

		exampi::transport->peek(r->getHeaderIovecs(), 0);
		debugpp("matchThread:  finished peeking");
		r->unpack();

		debugpp("matchThread:  received, unexpectedlock locking");
		unexpectedLock->lock();
		debugpp("matchThread:  match lock locking");
		matchLock->lock();
		debugpp("matchThread:  locked everything");
		int t = r->tag;
		int s = r->source;
		int c = r->comm;
		int e = r->stage;

		debugpp("matchThread context " << c);

		// search for match
		auto result =
		    std::find_if(matchList->begin(), matchList->end(),
		                 [t, s, c, e](const std::unique_ptr<Request> &i) -> bool {return (i->tag == t && i->source == s && i->stage == e && i->comm == c);});

		// failed to find match
		if (result == matchList->end())
		{
			//
			matchLock->unlock();
			debugpp("WARNING:  Failed to match incoming msg");

			if (t == MPIX_CLEANUP_TAG)
			{
				exampi::transport->cleanUp(0);
				exampi::progress->stop();
				unexpectedLock->unlock();
			}
			else
			{
				if (e != exampi::epoch)
				{
					unexpectedLock->unlock();
					debugpp("WARNING: Message from last stage (discarded)");
				}
				else
				{
					debugpp("\tUnexpected message\n");
					std::unique_ptr<Request> tmp = make_unique<Request>();
					ssize_t length;
					exampi::transport->receive(tmp->getTempIovecs(), 0, &length);
					tmp->status.count = length - 32;
					unexpectedList->push_back(std::move(tmp));
					unexpectedLock->unlock();
				}
			}
		}
		else
		{
			unexpectedLock->unlock();

			debugpp("matchThread:  matched, about to receive remainder");
			debugpp("\tTarget array is " << (*result)->array.toString());
			debugpp("\tDatatype says extent is " << (*result)->array.datatype->getExtent());

			ssize_t length;
			exampi::transport->receive((*result)->getIovecs(), 0, &length);
			(*result)->unpack();

			// set MPI_Status for calling thread
			(*result)->completionPromise.set_value( { .count = length - 32,
			                                        .cancelled = 0, .MPI_SOURCE = (*result)->source,
			                                        .MPI_TAG = (*result)->tag, .MPI_ERROR = MPI_SUCCESS });
			matchList->erase(result);
			matchLock->unlock();
			debugpp(" matching done, matchthread done");
		}

		//matchLock->unlock();
	}
}

int BasicProgress::init()
{
	addEndpoints();
	alive = true;

	sendThread = std::thread { sendThreadProc, &alive, &outbox };

	//recvThread = std::thread{recvThreadProc, &alive, &inbox};

	matchThread = std::thread { matchThreadProc, &alive, &matchList, &unexpectedList,
	                            &matchLock, &unexpectedLock };

	exampi::groups.push_back(group);
	communicator = new Comm(true, group, group);
	communicator->set_rank(exampi::rank);
	communicator->set_context(0, 1);
	exampi::communicators.push_back(communicator);
	return 0;
}

int BasicProgress::init(std::istream &t)
{
	init();
	return 0;
}

void BasicProgress::finalize()
{
	for(auto &&com : exampi::communicators)
	{
		delete com;
	}
	exampi::communicators.clear();

	for (auto &&group : exampi::groups)
	{
		delete group;
	}
	exampi::groups.clear();

	alive = false;
	matchList.clear();
	unexpectedList.clear();
	matchLock.unlock();
	unexpectedLock.unlock();
	ThreadMap::const_iterator it = tm_.find("1");
	if (it != tm_.end())
	{
		pthread_cancel(it->second);
		tm_.erase("1");
		//std::cout << "Thread " << "1" << " killed:" << std::endl;
	}
	it = tm_.find("2");
	if (it != tm_.end())
	{
		pthread_cancel(it->second);
		tm_.erase("2");
		//std::cout << "Thread " << "2" << " killed:" << std::endl;
	}
}

int BasicProgress::stop()
{
	for (auto &r : matchList)
	{
		(r)->unpack();
		(r)->completionPromise.set_value( { .count = 0, .cancelled = 0,
		                                    .MPI_SOURCE = (r)->source, .MPI_TAG = (r)->tag, .MPI_ERROR =
		                                        MPIX_TRY_RELOAD });
	}
	matchList.clear();
	unexpectedList.clear();
	return 0;
}

void BasicProgress::cleanUp()
{
	// what is this?
	sigHandler handler;
	handler.setSignalToHandle(SIGUSR1);

	// read parent pid from file
	//int parent_pid = std::stoi((*exampi::config)["ppid"]);


	// write out pid and epoch
	//std::stringstream filename;
	//filename << "pid." << exampi::rank << ".txt";

	//std::ofstream t(filename.str());
	//t << ::getpid() << std::endl;
	//t << exampi::epoch << std::endl;
	//t.close();


	// send SIGUSR1 signal to daemon
	// TODO convert to socket comms
	Daemon &daemon = Daemon::get_instance();
	daemon.send_clean_up();

	//kill(parent_pid, SIGUSR1);

	// TODO what is this?
	matchLock.lock();
	int size = matchList.size();
	matchLock.unlock();
	if (size > 0)
	{
		exampi::handler->setErrToZero();
		exampi::BasicInterface::get_instance()->MPI_Send((void *) 0, 0, MPI_INT,
		        exampi::rank, MPIX_CLEANUP_TAG, MPI_COMM_WORLD);
		exampi::handler->setErrToOne();
	}

	/* Checkpoint/restart
	 * exit(0);
	 */
}


void BasicProgress::barrier()
{
	Daemon &daemon = Daemon::get_instance();

	daemon.barrier();

	//// this is the MPI_init barrier release

	//// write process id to file with rank
	//std::stringstream filename;
	//filename << "pid." << exampi::rank << ".txt";
	//std::ofstream t(filename.str());
	//t << ::getpid();
	//t.close();

	//// TODO convert to socket communication
	//sigHandler signal;
	//signal.setSignalToHandle(SIGUSR1);

	//// send usr1 to parent
	//// TODO replace with socket comms
	//int parent_pid = std::stoi((*exampi::config)["ppid"]);
	//kill(parent_pid, SIGUSR1);

	//// wait for "com" return
	//// busy wait on signal
	//while (signal.isSignalSet() != 1)
	//{
	//	sleep(1);
	//}
	//signal.setSignalToZero();
}

std::future<MPI_Status> BasicProgress::postSend(UserArray array, Endpoint dest,
        int tag)
{
	debugpp("basic::Interface::postSend(...)");

	// create request
	std::unique_ptr<Request> r = make_unique<Request>();
	r->op = Op::Send;
	r->source = exampi::rank;
	r->stage = exampi::epoch;
	r->array = array;
	r->endpoint = dest;
	r->tag = tag;
	r->comm = dest.comm;

	//
	auto result = r->completionPromise.get_future();

	// give to send thread
	outbox.put(std::move(r));

	return result;
}

std::future<MPI_Status> BasicProgress::postRecv(UserArray array,
        Endpoint source, int tag)
{
	debugpp("basic::Interface::postRecv(...)");

	// make request
	std::unique_ptr<Request> r = make_unique<Request>();
	r->op = Op::Receive;
	r->source = source.rank;
	r->array = array;
	r->endpoint = source;
	r->tag = tag;
	r->comm = source.comm;
	r->stage = exampi::epoch;
	int s = source.rank;
	int c = source.comm;
	int e = exampi::epoch;
	auto result = r->completionPromise.get_future();

	// search unexpected message queue
	debugpp("searching unexpected message queue");
	unexpectedLock.lock();
	matchLock.lock();
	auto res = std::find_if(unexpectedList.begin(), unexpectedList.end(),
	                        [tag,s, c, e](const std::unique_ptr<Request> &i) -> bool {i->unpack(); return i->tag == tag && i->source == s && i->stage == e && i->comm == c;});

	//
	if (res == unexpectedList.end())
	{
		debugpp("NO match in unexpectedList, push");

		// put request into match list for later matching
		unexpectedLock.unlock();
		matchList.push_back(std::move(r));
		matchLock.unlock();
	}
	else
	{
		// found in UMQ
		matchLock.unlock();

		debugpp("Found match in unexpectedList");

		(*res)->unpack();
		//memcpy(array.ptr, )
		memcpy(array.getIovec().iov_base, (*res)->temp.iov_base,
		       array.getIovec().iov_len);
		(r)->completionPromise.set_value( { .count = (*res)->status.count, .cancelled = 0,
		                                    .MPI_SOURCE = (*res)->source, .MPI_TAG = (*res)->tag, .MPI_ERROR = MPI_SUCCESS});
		unexpectedList.erase(res);
		unexpectedLock.unlock();

	}

	return result;
}

int BasicProgress::save(std::ostream &t)
{
	//save group
	int group_size = exampi::groups.size();
	t.write(reinterpret_cast<char *>(&group_size), sizeof(int));
	for (auto &g : exampi::groups)
	{
		int value = g->get_group_id();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = g->get_process_list().size();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		for (auto p : g->get_process_list())
		{
			t.write(reinterpret_cast<char *>(&p), sizeof(int));
		}
	}

	//save communicator
	int comm_size = exampi::communicators.size();
	t.write(reinterpret_cast<char *>(&comm_size), sizeof(int));
	for(auto &c : exampi::communicators)
	{
		int value = c->get_rank();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = c->get_context_id_pt2pt();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = c->get_context_id_coll();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		bool intra = c->get_is_intra();
		t.write(reinterpret_cast<char *>(&intra), sizeof(bool));
		value = c->get_local_group()->get_group_id();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
		value = c->get_remote_group()->get_group_id();
		t.write(reinterpret_cast<char *>(&value), sizeof(int));
	}

	return MPI_SUCCESS;
}

int BasicProgress::load(std::istream &t)
{
	alive = true;
	sendThread = std::thread { sendThreadProc, &alive, &outbox };
	matchThread = std::thread { matchThreadProc, &alive, &matchList, &unexpectedList,
	                            &matchLock, &unexpectedLock };

	int comm_size, group_size;
	int r, p2p, coll, id;
	bool intra;
	int num_of_processes;
	std::list<int> ranks;
	int rank;
	exampi::Group *grp;
	//restore group
	t.read(reinterpret_cast<char *>(&group_size), sizeof(int));
	while(group_size)
	{
		// todo heap allocation
		grp = new exampi::Group();

		t.read(reinterpret_cast<char *>(&id), sizeof(int));
		grp->set_group_id(id);
		t.read(reinterpret_cast<char *>(&num_of_processes), sizeof(int));
		for (int i = 0; i < num_of_processes; i++)
		{
			t.read(reinterpret_cast<char *>(&rank), sizeof(int));
			ranks.push_back(rank);
		}
		grp->set_process_list(ranks);
		exampi::groups.push_back(grp);
		group_size--;
	}
	//restore communicator
	t.read(reinterpret_cast<char *>(&comm_size), sizeof(int));

	while(comm_size)
	{
		exampi::Comm *com = new exampi::Comm();
		t.read(reinterpret_cast<char *>(&r), sizeof(int));
		com->set_rank(r);
		t.read(reinterpret_cast<char *>(&p2p), sizeof(int));
		t.read(reinterpret_cast<char *>(&coll), sizeof(int));
		com->set_context(p2p, coll);
		t.read(reinterpret_cast<char *>(&intra), sizeof(bool));
		com->set_is_intra(intra);
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
			com->set_local_group(*it);
		}
		t.read(reinterpret_cast<char *>(&id), sizeof(int));
		it = std::find_if(exampi::groups.begin(), exampi::groups.end(),
		                  [id](const Group *i) -> bool {return i->get_group_id() == id;});
		if (it == exampi::groups.end())
		{
			return MPIX_TRY_RELOAD;
		}
		else
		{
			com->set_remote_group(*it);
		}
		exampi::communicators.push_back(com);
		comm_size--;
	}
	return MPI_SUCCESS;
}

} // exampi
