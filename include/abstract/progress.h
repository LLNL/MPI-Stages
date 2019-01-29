#ifndef __EXAMPI_ABSTRACT_PROGRESS_H
#define __EXAMPI_ABSTRACT_PROGRESS_H

#include <mpi.h>
#include <endpoint.h>

namespace exampi
{

class Progress
{
public:
	// addendpoints, launch threads, make MPI_COMM_WORLD?
	virtual int init() = 0;

	// same as init
	virtual int init(std::istream &t) = 0;

	// delete all communicators, groups, stop threads
	virtual void finalize() = 0;

	// daemon barrier direct call
	// TODO remove! this should be handled directly with calling daemon, this is not a "MPI level barrier"
	virtual void barrier() = 0;

	// send message across wire
	virtual std::future<MPI_Status> postSend(UserArray array, Endpoint dest,
	        int tag) = 0;

	// recv message from wire
	virtual std::future<MPI_Status> postRecv(UserArray array, Endpoint source,
	        int tag) = 0;

	virtual int handle_request(MPI_Request *request) = 0;

	// save groups and communicators
	virtual int save(std::ostream &t) = 0;
	
	// start threads, restore groups, communicators
	virtual int load(std::istream &t) = 0;

	// nullify match list
	virtual int stop() = 0;
	
	// stages recovery send cleanup to daemon, this gets triggered by sigusr2 from daemon
	virtual void cleanUp() = 0;
};

} // ::exampi

#endif // __EXAMPI_PROGRESS_H
