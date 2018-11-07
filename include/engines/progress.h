#ifndef __EXAMPI_BASIC_PROGRESS_H
#define __EXAMPI_BASIC_PROGRESS_H

#include "basic.h"
#include <map>
#include <unordered_map>
#include <list>
#include <mutex>
#include <set>
#include <memory>
#include <algorithm>
#include <sigHandler.h>
#include <comm.h>
#include "transports/transport.h"

namespace exampi
{
namespace basic
{

// POD types
class Header
{
public:
	static constexpr size_t HeaderSize = (8 * 4);

	int rank;
	uint32_t tag;
	int context;
	MPI_Comm comm;
	char hdr[HeaderSize];

	Header()
	{
		std::memset(hdr, 0xD0, HeaderSize);
	}

	// TODO:  Don't forget these debugs, this can create a lot of spam
	void dump()
	{
		std::ios oldState(nullptr);
		oldState.copyfmt(std::cout);

		uint32_t *dword = (uint32_t *) hdr;
		std::cout << "\texampi::basic::Header has:\n";
		std::cout << std::setbase(16) << std::internal << std::setfill('0')
		          << "\t" << std::setw(8) << dword[0] << " " << std::setw(8)
		          << dword[1] << " " << std::setw(8) << dword[2] << " "
		          << std::setw(8) << dword[3] << " " << "\n\t" << std::setw(8)
		          << dword[4] << " " << std::setw(8) << dword[5] << " "
		          << std::setw(8) << dword[6] << " " << std::setw(8) << dword[7]
		          << " " << "\n";

		std::cout.copyfmt(oldState);
	}

	void pack()
	{
		uint16_t *word = (uint16_t *) hdr;
		uint32_t *dword = (uint32_t *) hdr;
		word[0] = 0xDEAF; // magic word
		word[1] = 22;  // protocol
		word[2] = 42;  // message type
		word[3] = 0x0; // align
		dword[2] = 0x0;  // align
		dword[3] = 0x0;  // align/reserved
		dword[4] = rank;
		dword[5] = tag;
		dword[6] = context;
		dword[7] = 0xAABBCCDD;  // CRC
		std::cout << "\tpack:\n";
		dump();

	}

	void unpack()
	{
		uint32_t *dword = (uint32_t *) hdr;
		rank = dword[4];
		tag = dword[5];
		context = dword[6];
		std::cout << "\tunpack:\n";
		dump();
		std::cout << "\tUnderstood rank as " << rank << "\n";
	}

	struct iovec getIovec()
	{
		pack();
		struct iovec iov = { hdr, HeaderSize };
		return iov;
	}
};

class Request
{
public:
	static constexpr size_t HeaderSize = (8 * 4);
protected:
	char hdr[HeaderSize];
public:
	Op op;
	int tag;
	int source;
	MPI_Comm comm;
	UserArray array;
	struct iovec temp;
	Endpoint endpoint;
	int stage;
	MPI_Status status; // maybe not needed --sf
	std::promise<MPI_Status> completionPromise;

	void pack()
	{
		uint16_t *word = (uint16_t *) hdr;
		uint32_t *dword = (uint32_t *) hdr;
		word[0] = 0xDEAF; // magic word
		word[1] = 22;  // protocol
		word[2] = 42;  // message type
		word[3] = 0x0; // function
		dword[2] = 0x0;  // align
		dword[3] = stage;  // align/reserved
		dword[4] = source;
		dword[5] = tag;
		dword[6] = comm; // context; not yet
		dword[7] = 0xAABBCCDD;  // CRC
	}

	void unpack()
	{
		uint32_t *dword = (uint32_t *) hdr;
		stage = dword[3];
		source = dword[4];
		tag = dword[5];
		comm = dword[6];
	}

	struct iovec getHeaderIovec()
	{
		pack();
		struct iovec iov = { hdr, HeaderSize };
		return iov;
	}

	std::vector<struct iovec> getHeaderIovecs()
	{
		std::vector<struct iovec> iov;
		iov.push_back(getHeaderIovec());
		return iov;
	}

	std::vector<struct iovec> getArrayIovecs()
	{
		std::vector<struct iovec> iov;
		iov.push_back(array.getIovec());
		return iov;
	}

	std::vector<struct iovec> getIovecs()
	{
		std::vector<struct iovec> iov;
		iov.push_back(getHeaderIovec());
		iov.push_back(array.getIovec());
		return iov;
	}

	std::vector<struct iovec> getTempIovecs()
	{
		std::vector<struct iovec> iov;
		iov.push_back(getHeaderIovec());
		char tempBuff[65000];
		temp.iov_base = tempBuff;
		temp.iov_len = sizeof(tempBuff);
		iov.push_back(temp);
		return iov;
	}
};

class Progress: public exampi::i::Progress
{
private:
	AsyncQueue<Request> outbox;
	std::list<std::unique_ptr<Request>> matchList;
	std::list<std::unique_ptr<Request>> unexpectedList;
	std::mutex matchLock;
	std::mutex unexpectedLock;
	std::thread sendThread;
	std::thread matchThread;
	bool alive;
	exampi::Group *group;
	exampi::Comm *communicator;
	typedef std::unordered_map<std::string, pthread_t> ThreadMap;
	ThreadMap tm_;

	void addEndpoints()
	{
		int size = std::stoi((*exampi::global::config)["size"]);
		std::vector < std::string > elem;
		std::list<int> rankList;
		for (int i = 0; i < size; i++)
		{
			elem.clear();
			rankList.push_back(i);
			std::string rank = std::to_string(i);
			elem.push_back((*exampi::global::config)[rank]);
			elem.push_back("8080");
			exampi::global::transport->addEndpoint(i, elem);
		}
		group = new Group(rankList);
	}

	static void sendThreadProc(bool *alive, AsyncQueue<Request> *outbox)
	{
		std::cout << debug() << "Launching sendThreadProc(...)\n";
		while (*alive)
		{
			std::unique_ptr<Request> r = outbox->promise().get();
			std::cout << debug()
			          << "sendThread:  got result from outbox future\n";
			exampi::global::transport->send(r->getIovecs(), r->endpoint.rank,
			                                0);
			// TODO:  check that sending actually completed
			r->completionPromise.set_value( { .count = 0, .cancelled = 0,
			                                  .MPI_SOURCE = r->source, .MPI_TAG = r->tag, .MPI_ERROR =
			                                      MPI_SUCCESS });
			// let r drop scope and die (unique_ptr)
		}
	}

	static void matchThreadProc(bool *alive,
	                            std::list<std::unique_ptr<Request>> *matchList,
	                            std::list<std::unique_ptr<Request>> *unexpectedList,
	                            std::mutex *matchLock, std::mutex *unexpectedLock)
	{
		std::cout << debug() << "Launching matchThreadProc(...)\n";
		while (*alive)
		{
			std::unique_ptr<Request> r = make_unique<Request>();
			std::cout << debug()
			          << "matchThread:  made request, about to peek...\n";
			exampi::global::transport->peek(r->getHeaderIovecs(), 0);
			r->unpack();

			std::cout << debug() << "matchThread:  received\n";
			unexpectedLock->lock();
			matchLock->lock();
			int t = r->tag;
			int s = r->source;
			int c = r->comm;
			int e = r->stage;
			std::cout << "Context " << c << std::endl;

			auto result =
			    std::find_if(matchList->begin(), matchList->end(),
			                 [t, s, c, e](const std::unique_ptr<Request> &i) -> bool {return (i->tag == t && i->source == s && i->stage == e && i->comm == c);});
			if (result == matchList->end())
			{
				matchLock->unlock();
				std::cout << "WARNING:  Failed to match incoming msg\n";
				if (t == MPIX_CLEANUP_TAG)
				{
					exampi::global::transport->cleanUp(0);
					exampi::global::progress->stop();
					unexpectedLock->unlock();
				}
				else
				{
					if (e != exampi::global::epoch)
					{
						unexpectedLock->unlock();
						std::cout << "WARNING: Message from last stage (discarded)\n";
					}
					else
					{
						std::cout << debug() << "\tUnexpected message\n";
						std::unique_ptr<Request> tmp = make_unique<Request>();
						ssize_t length;
						exampi::global::transport->receive(tmp->getTempIovecs(), 0, &length);
						tmp->status.count = length - 32;
						unexpectedList->push_back(std::move(tmp));
						unexpectedLock->unlock();
					}
				}
			}
			else
			{
				unexpectedLock->unlock();
				std::cout << debug()
				          << "matchThread:  matched, about to receive remainder\n";
				std::cout << debug() << "\tTarget array is "
				          << (*result)->array.toString() << "\n";
				std::cout << debug() << "\tDatatype says extent is "
				          << (*result)->array.datatype->getExtent() << "\n";
				ssize_t length;
				exampi::global::transport->receive((*result)->getIovecs(), 0, &length);
				(*result)->unpack();
				(*result)->completionPromise.set_value( { .count = length - 32,
				                                        .cancelled = 0, .MPI_SOURCE = (*result)->source,
				                                        .MPI_TAG = (*result)->tag, .MPI_ERROR = MPI_SUCCESS });
				matchList->erase(result);
				matchLock->unlock();
			}

			//matchLock->unlock();
		}
	}
public:
	Progress()
	{
		;
	}

	virtual int init()
	{
		addEndpoints();
		alive = true;
		sendThread = std::thread { sendThreadProc, &alive, &outbox };
		//recvThread = std::thread{recvThreadProc, &alive, &inbox};
		matchThread = std::thread { matchThreadProc, &alive, &matchList, &unexpectedList,
		                            &matchLock, &unexpectedLock };

		exampi::global::groups.push_back(group);
		communicator = new Comm(true, group, group);
		communicator->set_rank(exampi::global::rank);
		communicator->set_context(0, 1);
		exampi::global::communicators.push_back(communicator);
		return 0;
	}

	virtual int init(std::istream &t)
	{

		init();
		return 0;
	}

	virtual void finalize()
	{
		for(auto &&com : exampi::global::communicators)
		{
			delete com;
		}
		exampi::global::communicators.clear();

		for (auto &&group : exampi::global::groups)
		{
			delete group;
		}
		exampi::global::groups.clear();

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
			std::cout << "Thread " << "1" << " killed:" << std::endl;
		}
		it = tm_.find("2");
		if (it != tm_.end())
		{
			pthread_cancel(it->second);
			tm_.erase("2");
			std::cout << "Thread " << "2" << " killed:" << std::endl;
		}
	}

	virtual int stop()
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

	virtual void cleanUp()
	{
		sigHandler handler;
		handler.setSignalToHandle(SIGUSR1);
		int parent_pid = std::stoi((*exampi::global::config)["ppid"]);
		std::stringstream filename;
		filename << "pid." << exampi::global::rank << ".txt";
		std::ofstream t(filename.str());
		t << ::getpid() << std::endl;
		t << exampi::global::epoch << std::endl;
		t.close();
		kill(parent_pid, SIGUSR1);

		matchLock.lock();
		int size = matchList.size();
		matchLock.unlock();
		if (size > 0)
		{
			exampi::global::handler->setErrToZero();
			exampi::global::interface->MPI_Send((void *) 0, 0, MPI_INT,
				                                    exampi::global::rank, MPIX_CLEANUP_TAG, MPI_COMM_WORLD);
			exampi::global::handler->setErrToOne();
		}
		/* Checkpoint/restart
		 * exit(0);
		 */
	}


	virtual void barrier()
	{
		std::stringstream filename;
		filename << "pid." << exampi::global::rank << ".txt";
		std::ofstream t(filename.str());
		t << ::getpid();
		t.close();


		sigHandler signal;
		signal.setSignalToHandle(SIGUSR1);
		int parent_pid = std::stoi((*exampi::global::config)["ppid"]);
		kill(parent_pid, SIGUSR1);

		while (signal.isSignalSet() != 1)
		{
			sleep(1);
		}
		signal.setSignalToZero();
	}

	virtual std::future<MPI_Status> postSend(UserArray array, Endpoint dest,
	        int tag)
	{
		std::cout << debug() << "\tbasic::Interface::postSend(...)\n";
		std::unique_ptr<Request> r = make_unique<Request>();
		r->op = Op::Send;
		r->source = exampi::global::rank;
		r->stage = exampi::global::epoch;
		r->array = array;
		r->endpoint = dest;
		r->tag = tag;
		r->comm = dest.comm;
		auto result = r->completionPromise.get_future();
		outbox.put(std::move(r));
		return result;
	}

	virtual std::future<MPI_Status> postRecv(UserArray array, Endpoint source, int tag)
	{
		std::cout << debug() << "\tbasic::Interface::postRecv(...)\n";

		std::unique_ptr<Request> r = make_unique<Request>();
		r->op = Op::Receive;
		r->source = source.rank;
		r->array = array;
		r->endpoint = source;
		r->tag = tag;
		r->comm = source.comm;
		r->stage = exampi::global::epoch;
		int s = source.rank;
		int c = source.comm;
		int e = exampi::global::epoch;
		auto result = r->completionPromise.get_future();

		unexpectedLock.lock();
		matchLock.lock();
		auto res = std::find_if(unexpectedList.begin(), unexpectedList.end(),
		                        [tag,s, c, e](const std::unique_ptr<Request> &i) -> bool {i->unpack(); return i->tag == tag && i->source == s && i->stage == e && i->comm == c;});
		if (res == unexpectedList.end())
		{
			unexpectedLock.unlock();
			matchList.push_back(std::move(r));
			matchLock.unlock();
		}
		else
		{
			matchLock.unlock();
			std::cout << "Found match in unexpectedList\n";
			(*res)->unpack();
			//memcpy(array.ptr, )
			memcpy(array.getIovec().iov_base, (*res)->temp.iov_base, array.getIovec().iov_len);
			(r)->completionPromise.set_value( { .count = (*res)->status.count, .cancelled = 0,
			                                    .MPI_SOURCE = (*res)->source, .MPI_TAG = (*res)->tag, .MPI_ERROR =
			                                        MPI_SUCCESS});
			unexpectedList.erase(res);
			unexpectedLock.unlock();

		}


		return result;
	}

	virtual int save(std::ostream &t)
	{
		//save group
		int group_size = exampi::global::groups.size();
		t.write(reinterpret_cast<char *>(&group_size), sizeof(int));
		for (auto &g : exampi::global::groups)
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
		int comm_size = exampi::global::communicators.size();
		t.write(reinterpret_cast<char *>(&comm_size), sizeof(int));
		for(auto &c : exampi::global::communicators)
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

	virtual int load(std::istream &t)
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
			exampi::global::groups.push_back(grp);
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

			auto it = std::find_if(exampi::global::groups.begin(), exampi::global::groups.end(),
			                       [id](const Group *i) -> bool {return i->get_group_id() == id;});
			if (it == exampi::global::groups.end())
			{
				return MPIX_TRY_RELOAD;
			}
			else
			{
				com->set_local_group(*it);
			}
			t.read(reinterpret_cast<char *>(&id), sizeof(int));
			it = std::find_if(exampi::global::groups.begin(), exampi::global::groups.end(),
			                  [id](const Group *i) -> bool {return i->get_group_id() == id;});
			if (it == exampi::global::groups.end())
			{
				return MPIX_TRY_RELOAD;
			}
			else
			{
				com->set_remote_group(*it);
			}
			exampi::global::communicators.push_back(com);
			comm_size--;
		}
		return MPI_SUCCESS;
	}

};

} // basic
} // exampi

#endif // header guard
