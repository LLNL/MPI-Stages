#ifndef __EXAMPI_BASIC_PROGRESS_H
#define __EXAMPI_BASIC_PROGRESS_H

#include <basic.h>
#include <map>
#include <unordered_map>
#include <list>
#include <mutex>
#include <set>
#include <memory>
#include <algorithm>
#include <sigHandler.h>
#include <comm.h>
#include "basic/transport.h"

namespace exampi {
namespace basic {

// POD types
class Header {
public:
	static constexpr size_t HeaderSize = (8 * 4);

	int rank;
	uint32_t tag;
	int context;
	MPI_Comm comm;
	char hdr[HeaderSize];

	Header() {
		std::memset(hdr, 0xD0, HeaderSize);
	}

	// TODO:  Don't forget these debugs, this can create a lot of spam
	void dump() {
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

	void pack() {
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

	void unpack() {
		uint32_t *dword = (uint32_t *) hdr;
		rank = dword[4];
		tag = dword[5];
		context = dword[6];
		std::cout << "\tunpack:\n";
		dump();
		std::cout << "\tUnderstood rank as " << rank << "\n";
	}

	struct iovec getIovec() {
		pack();
		struct iovec iov = { hdr, HeaderSize };
		return iov;
	}
};

class Request {
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
	MPI_Status status; // maybe not needed --sf
	std::promise<MPI_Status> completionPromise;

	void pack() {
		uint16_t *word = (uint16_t *) hdr;
		uint32_t *dword = (uint32_t *) hdr;
		word[0] = 0xDEAF; // magic word
		word[1] = 22;  // protocol
		word[2] = 42;  // message type
		word[3] = 0x0; // function
		dword[2] = 0x0;  // align
		dword[3] = 0x0;  // align/reserved
		dword[4] = source;
		dword[5] = tag;
		dword[6] = comm; // context; not yet
		dword[7] = 0xAABBCCDD;  // CRC
	}

	void unpack() {
		uint32_t *dword = (uint32_t *) hdr;
		source = dword[4];
		tag = dword[5];
		comm = dword[6];
	}

	struct iovec getHeaderIovec() {
		pack();
		struct iovec iov = { hdr, HeaderSize };
		return iov;
	}

	std::vector<struct iovec> getHeaderIovecs() {
		std::vector<struct iovec> iov;
		iov.push_back(getHeaderIovec());
		return iov;
	}

	std::vector<struct iovec> getArrayIovecs() {
		std::vector<struct iovec> iov;
		iov.push_back(array.getIovec());
		return iov;
	}

	std::vector<struct iovec> getIovecs() {
		std::vector<struct iovec> iov;
		iov.push_back(getHeaderIovec());
		iov.push_back(array.getIovec());
		return iov;
	}

	std::vector<struct iovec> getTempIovecs() {
		std::vector<struct iovec> iov;
		iov.push_back(getHeaderIovec());
		char tempBuff[65000];
		temp.iov_base = tempBuff;
		temp.iov_len = sizeof(tempBuff);
		iov.push_back(temp);
		return iov;
	}
};

class Progress: public exampi::i::Progress {
private:
	AsyncQueue<Request> outbox;
	std::list<std::unique_ptr<Request>> matchList;
	std::list<std::unique_ptr<Request>> unexpectedList;
	std::mutex matchLock;
	std::mutex unexpectedLock;
	std::thread sendThread;
	std::thread matchThread;
	bool alive;
	std::shared_ptr<Group> group;
	exampi::Comm communicator;

	void addEndpoints() {
		int size = std::stoi((*exampi::global::config)["size"]);
		std::vector < std::string > elem;
		std::list<int> rankList;
		for (int i = 0; i < size; i++) {
			elem.clear();
			rankList.push_back(i);
			std::string rank = std::to_string(i);
			elem.push_back((*exampi::global::config)[rank]);
			elem.push_back("8080");
			exampi::global::transport->addEndpoint(i, elem);
		}
		group = std::make_shared<Group>(rankList);
	}

	static void sendThreadProc(bool *alive, AsyncQueue<Request> *outbox) {
		std::cout << debug() << "Launching sendThreadProc(...)\n";
		while (*alive) {
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
			std::mutex *matchLock, std::mutex *unexpectedLock) {
		std::cout << debug() << "Launching matchThreadProc(...)\n";
		while (*alive) {
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
			std::cout << "Context " << c << std::endl;
			auto result =
					std::find_if(matchList->begin(), matchList->end(),
							[t, s, c](const std::unique_ptr<Request> &i) -> bool {return (i->tag == t && i->source == s && i->comm == c);});
			if (result == matchList->end()) {
				matchLock->unlock();
				std::cout << "WARNING:  Failed to match incoming msg\n";
				if (t == MPIX_CLEANUP_TAG) {
					exampi::global::transport->cleanUp(0);
					exampi::global::progress->stop();
					unexpectedLock->unlock();
				}
				else {
					std::cout << debug() << "\tUnexpected message\n";
					std::unique_ptr<Request> tmp = make_unique<Request>();
					exampi::global::transport->receive(tmp->getTempIovecs(), 0);
					unexpectedList->push_back(std::move(tmp));
					unexpectedLock->unlock();
				}
			} else {
				unexpectedLock->unlock();
				std::cout << debug()
								<< "matchThread:  matched, about to receive remainder\n";
				std::cout << debug() << "\tTarget array is "
						<< (*result)->array.toString() << "\n";
				std::cout << debug() << "\tDatatype says extent is "
						<< (*result)->array.datatype->getExtent() << "\n";
				exampi::global::transport->receive((*result)->getIovecs(), 0);
				(*result)->unpack();
				(*result)->completionPromise.set_value( { .count = 0,
					.cancelled = 0, .MPI_SOURCE = (*result)->source,
					.MPI_TAG = (*result)->tag, .MPI_ERROR = MPI_SUCCESS });
				matchList->erase(result);
				matchLock->unlock();
			}

			//matchLock->unlock();
		}
	}
public:
	Progress() {
		;
	}

	virtual int init() {
		addEndpoints();
		alive = true;
		sendThread = std::thread { sendThreadProc, &alive, &outbox };
		//recvThread = std::thread{recvThreadProc, &alive, &inbox};
		matchThread = std::thread { matchThreadProc, &alive, &matchList, &unexpectedList,
			&matchLock, &unexpectedLock };

		exampi::global::groups.push_back(group);
		communicator = Comm(true, group, group);
		communicator.setRank(exampi::global::rank);
		communicator.set_context(0, 1);
		exampi::global::communicators.push_back(communicator);
		return 0;
	}

	virtual int init(std::istream &t) {

		init();
		return 0;
	}

	virtual int stop() {
		for (auto& r : matchList) {
			(r)->unpack();
			(r)->completionPromise.set_value( { .count = 0, .cancelled = 0,
				.MPI_SOURCE = (r)->source, .MPI_TAG = (r)->tag, .MPI_ERROR =
						MPIX_TRY_RELOAD });
		}
		matchList.clear();
		unexpectedList.clear();
		return 0;
	}

	virtual void cleanUp() {
		matchLock.lock();
		int size = matchList.size();
		matchLock.unlock();
		if (size > 0) {
			errHandler handler;
			handler.setErrToZero();
			exampi::global::interface->MPI_Send((void *) 0, 0, MPI_INT,
					exampi::global::rank, MPIX_CLEANUP_TAG, MPI_COMM_WORLD);
			handler.setErrToOne();
		}
	}


	virtual void barrier() {
		std::stringstream filename;
		filename << "pid." << exampi::global::rank << ".txt";
		std::ofstream t(filename.str());
		t << ::getpid();
		t.close();


		sigHandler signal;
		signal.setSignalToHandle(SIGUSR1);
		int parent_pid = std::stoi((*exampi::global::config)["ppid"]);
		kill(parent_pid, SIGUSR1);

		while (signal.isSignalSet() != 1) {
			sleep(1);
		}
		signal.setSignalToZero();
	}

	virtual std::future<MPI_Status> postSend(UserArray array, Endpoint dest,
			int tag) {
		std::cout << debug() << "\tbasic::Interface::postSend(...)\n";
		std::unique_ptr<Request> r = make_unique<Request>();
		r->op = Op::Send;
		r->source = exampi::global::rank;
		r->array = array;
		r->endpoint = dest;
		r->tag = tag;
		r->comm = dest.comm;
		auto result = r->completionPromise.get_future();
		outbox.put(std::move(r));
		return result;
	}

	virtual std::future<MPI_Status> postRecv(UserArray array, Endpoint source, int tag) {
		std::cout << debug() << "\tbasic::Interface::postRecv(...)\n";

		std::unique_ptr<Request> r = make_unique<Request>();
		r->op = Op::Receive;
		r->source = source.rank;
		r->array = array;
		r->endpoint = source;
		r->tag = tag;
		r->comm = source.comm;
		int s = source.rank;
		int c = source.comm;
		auto result = r->completionPromise.get_future();

		unexpectedLock.lock();
		matchLock.lock();
			auto res = std::find_if(unexpectedList.begin(), unexpectedList.end(),
							[tag,s, c](const std::unique_ptr<Request> &i) -> bool {i->unpack();return i->tag == tag && i->source == s && i->comm == c;});
			if (res == unexpectedList.end()) {
				unexpectedLock.unlock();
				matchList.push_back(std::move(r));
				matchLock.unlock();
			}
			else {
				matchLock.unlock();
				std::cout << "Found match in unexpectedList\n";
				(*res)->unpack();
				//memcpy(array.ptr, )
				memcpy(array.getIovec().iov_base, (*res)->temp.iov_base, array.getIovec().iov_len);

				(r)->completionPromise.set_value( { .count = 0, .cancelled = 0,
					.MPI_SOURCE = (*res)->source, .MPI_TAG = (*res)->tag, .MPI_ERROR =
					MPI_SUCCESS});
				unexpectedList.erase(res);
				unexpectedLock.unlock();

			}


		return result;
	}

	virtual int save(std::ostream &t) {
		std::cout << "In progress save\n";
		int commsz = exampi::global::communicators.size();

		t.write(reinterpret_cast<char *>(&commsz), sizeof(int));
		for(auto c : exampi::global::communicators)
		{
		    t.write(reinterpret_cast<char *>(&c.rank), sizeof(int));
		    t.write(reinterpret_cast<char *>(&c.local_pt2pt), sizeof(int));
		    t.write(reinterpret_cast<char *>(&c.local_coll), sizeof(int));
		    t.write(reinterpret_cast<char *>(&c.isintra), sizeof(bool));
		    int sz = (c.local)->getProcessList().size();
		    t.write(reinterpret_cast<char *>(&sz), sizeof(int));
		    std::list<int> ranks = (c.local)->getProcessList();
		    for (auto i : ranks) {
		    	t.write(reinterpret_cast<char *>(&i), sizeof(int));
		    }
		 }

		return MPI_SUCCESS;
	}

	virtual int load(std::istream& t) {
		std::cout << "In progress load\n";
		alive = true;
		sendThread = std::thread { sendThreadProc, &alive, &outbox };
		matchThread = std::thread { matchThreadProc, &alive, &matchList, &unexpectedList,
			&matchLock, &unexpectedLock };

		int commsz, grpsize;
		int r, p2p, coll;
		bool intra;
		std::shared_ptr<Group> grp;
		t.read(reinterpret_cast<char *>(&commsz), sizeof(int));

		while(commsz)
		{
			Comm com;
		    t.read(reinterpret_cast<char *>(&r), sizeof(int));
		    com.rank = r;
		    t.read(reinterpret_cast<char *>(&p2p), sizeof(int));
		    com.local_pt2pt = p2p;
		    t.read(reinterpret_cast<char *>(&coll), sizeof(int));
		    com.local_coll = coll;
		    t.read(reinterpret_cast<char *>(&intra), sizeof(bool));
		    com.isintra = intra;
		    t.read(reinterpret_cast<char *>(&grpsize), sizeof(int));
		    std::list<int> ranks;
		    for (int i = 0; i < grpsize; i++) {
		    	int x;
		    	t.read(reinterpret_cast<char *>(&x), sizeof(int));
		    	ranks.push_back(x);
		    }
		    grp = std::make_shared<Group>(ranks);
		    com.local = grp;
		    com.remote = grp;
		    exampi::global::communicators.push_back(com);
		    commsz--;
		}
		return MPI_SUCCESS;
	}

};

} // basic
} // exampi

#endif // header guard
