#ifndef __EXAMPI_INTERFACE_TRANSPORT_H
#define __EXAMPI_INTERFACE_TRANSPORT_H

// todo remove with mpi stages
#include <fstream>
#include <map>

#include "header.h"
#include "request.h"
#include "protocol.h"

namespace exampi
{

struct Transport
{
	virtual ~Transport() {};

	// todo mpi stages
	virtual int save(std::ostream &r) = 0;
	virtual int load(std::istream &r) = 0;
	virtual int halt() = 0;

	// never called
	//virtual int cleanUp(MPI_Comm comm) = 0;

	// ordered map (preference) of protocol initator and maximum message size
	// note chose size_t over long int, because -1 == inf would work
	//      but it cuts down by a large range, max size_t is enough
	virtual const std::map<Protocol, size_t> &provided_protocols() const = 0;

	// receive a header or nullptr
	virtual Header* ordered_recv() = 0;

	// fill header into matching request
	virtual void fill(const Header*, Request*) = 0;

	// send request with protocol
	virtual void reliable_send(const Protocol, const Request*) = 0;
};

} // ::exampi

#endif
