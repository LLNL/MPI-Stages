#ifndef __EXAMPI_BASIC_PROGRESS_H
#define __EXAMPI_BASIC_PROGRESS_H

#include <map>
#include <unordered_map>
#include <list>
#include <mutex>
#include <set>
#include <memory>
#include <algorithm>

#include "request.h"
#include "basic.h"
#include "sigHandler.h"
#include "comm.h"
#include "transports/transport.h"
#include "interfaces/interface.h"
#include "daemon.h"
#include "config.h"
#include "pool.h"
#include "async.h"

namespace exampi
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
		//std::ios oldState(nullptr);
		//oldState.copyfmt(//std::cout);

		//uint32_t *dword = (uint32_t *) hdr;
		//std::cout << "\texampi::Header has:\n";
		//std::cout << std::setbase(16) << std::internal << std::setfill('0')
		//          << "\t" << std::setw(8) << dword[0] << " " << std::setw(8)
		//          << dword[1] << " " << std::setw(8) << dword[2] << " "
		//          << std::setw(8) << dword[3] << " " << "\n\t" << std::setw(8)
		//          << dword[4] << " " << std::setw(8) << dword[5] << " "
		//          << std::setw(8) << dword[6] << " " << std::setw(8) << dword[7]
		//          << " " << "\n";

		//std::cout.copyfmt(oldState);
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
		//std::cout << "\tpack:\n";
		dump();

	}

	void unpack()
	{
		uint32_t *dword = (uint32_t *) hdr;
		rank = dword[4];
		tag = dword[5];
		context = dword[6];
		//std::cout << "\tunpack:\n";
		dump();
		//std::cout << "\tUnderstood rank as " << rank << "\n";
	}

	struct iovec getIovec()
	{
		pack();
		struct iovec iov = { hdr, HeaderSize };
		return iov;
	}
};


class BasicProgress: public Progress
{
private:
	AsyncQueue<typename MemoryPool<Request>::unique_ptr> outbox;

	MemoryPool<Request> request_pool;

	//std::list<std::unique_ptr<Request>> matchList;
	std::list<typename MemoryPool<Request>::unique_ptr> matchList;

	//std::list<std::unique_ptr<Request>> unexpectedList;
	std::list<typename MemoryPool<Request>::unique_ptr> unexpectedList;

	std::mutex matchLock;
	std::mutex unexpectedLock;

	std::thread sendThread;
	std::thread matchThread;

	bool alive;
	
	// should move to global state mpi
	// global group
	exampi::Group *group;

	// MPI_COMM_WORLD
	exampi::Comm *communicator;

	// XXX where is this used?
	typedef std::unordered_map<std::string, pthread_t> ThreadMap;
	ThreadMap tm_;

	//static void sendThreadProc(bool *alive, AsyncQueue<Request> *outbox);
	void sendThreadProc();

	//static void matchThreadProc(bool *alive,
	//                            std::list<std::unique_ptr<Request>> *matchList,
	//                            std::list<std::unique_ptr<Request>> *unexpectedList,
	//                            std::mutex *matchLock, std::mutex *unexpectedLock);
	void matchThreadProc();

public:
	BasicProgress();

	virtual int init();
	virtual int init(std::istream &t);
	virtual void finalize();

	virtual int stop();
	virtual void cleanUp();
	virtual void barrier();

	virtual std::future<MPI_Status> postSend(UserArray array, Endpoint dest,
	        int tag);
	virtual std::future<MPI_Status> postRecv(UserArray array, Endpoint source,
	        int tag);

	virtual int save(std::ostream &t);
	virtual int load(std::istream &t);
};

} // exampi

#endif // header guard
