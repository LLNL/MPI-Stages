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
#include "interfaces/interface.h"
#include "daemon.h"
#include "config.h"

namespace exampi
{

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

class BasicProgress: public Progress
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

	void addEndpoints();

	static void sendThreadProc(bool *alive, AsyncQueue<Request> *outbox);
	static void matchThreadProc(bool *alive,
	                            std::list<std::unique_ptr<Request>> *matchList,
	                            std::list<std::unique_ptr<Request>> *unexpectedList,
	                            std::mutex *matchLock, std::mutex *unexpectedLock);

public:
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
