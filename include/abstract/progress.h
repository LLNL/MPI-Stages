#ifndef __EXAMPI_ABSTRACT_PROGRESS_H
#define __EXAMPI_ABSTRACT_PROGRESS_H

#include <mpi.h>

#include "request.h"

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

	virtual int post_request(Request *request) = 0;

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
