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
#include "header.h"

namespace exampi
{

//class BasicProgress: public Progress
//{
//private:
//	AsyncQueue<typename MemoryPool<Request>::unique_ptr> outbox;
//
//	MemoryPool<Request> request_pool;
//
//	//std::list<std::unique_ptr<Request>> matchList;
//	std::list<typename MemoryPool<Request>::unique_ptr> matchList;
//
//	//std::list<std::unique_ptr<Request>> unexpectedList;
//	std::list<typename MemoryPool<Request>::unique_ptr> unexpectedList;
//
//	std::mutex matchLock;
//	std::mutex unexpectedLock;
//
//	std::thread sendThread;
//	std::thread matchThread;
//
//	bool alive;
//	
//	// should move to global state mpi
//	// global group
//	exampi::Group *group;
//
//	// MPI_COMM_WORLD
//	exampi::Comm *communicator;
//
//	// XXX where is this used?
//	typedef std::unordered_map<std::string, pthread_t> ThreadMap;
//	ThreadMap tm_;
//
//	//static void sendThreadProc(bool *alive, AsyncQueue<Request> *outbox);
//	void sendThreadProc();
//
//	//static void matchThreadProc(bool *alive,
//	//                            std::list<std::unique_ptr<Request>> *matchList,
//	//                            std::list<std::unique_ptr<Request>> *unexpectedList,
//	//                            std::mutex *matchLock, std::mutex *unexpectedLock);
//	void matchThreadProc();
//
//public:
//	BasicProgress();
//
//	virtual int init();
//	virtual int init(std::istream &t);
//	virtual void finalize();
//
//	virtual int stop();
//	virtual void cleanUp();
//	
//	virtual int handle_request(MPI_Request *request);
//
//	virtual std::future<MPI_Status> postSend(UserArray array, Endpoint dest,
//	        int tag);
//	virtual std::future<MPI_Status> postRecv(UserArray array, Endpoint source,
//	        int tag);
//
//	virtual int save(std::ostream &t);
//	virtual int load(std::istream &t);
//};

} // exampi

#endif // header guard
