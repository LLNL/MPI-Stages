#ifndef __EXAMPI_ABSTRACT_PROGRESS_H
#define __EXAMPI_ABSTRACT_PROGRESS_H

#include <mpi.h>

#include "request.h"

namespace exampi
{

class Progress
{
public:
	virtual ~Progress() {}

	// handle user request object
	virtual int post_request(Request *request) = 0;

	// mpi stages
	virtual int save(std::ostream &t) = 0;
	virtual int load(std::istream &t) = 0;

	// stages recovery send cleanup to daemon, this gets triggered by sigusr2 from daemon
	virtual void cleanUp() = 0;
};

} // ::exampi

#endif // __EXAMPI_PROGRESS_H
