#ifndef __EXAMPI_INTERFACE_TRANSPORT_H
#define __EXAMPI_INTERFACE_TRANSPORT_H

#include <mutex>
#include <map>

#include "mpi.h"
#include "protocol.h"

namespace exampi
{

struct Transport
{
	virtual ~Transport() {};
	
	virtual ProtocolMessage_uptr allocate_protocol_message() = 0;

	// mpi stages
	// TODO stages integration into new stuff
	//virtual int save(std::ostream &r) = 0;
	//virtual int load(std::istream &r) = 0;

	// never called
	//virtual int cleanUp(MPI_Comm comm) = 0;

	virtual const ProtocolMessage_uptr ordered_recv() = 0;
	virtual int reliable_send(const ProtocolMessage_uptr message) = 0;

	// ordered map (preference) of protocol initator and maximum message size
	// note chose size_t over long int, because -1 == inf would work
	//      but it cuts down by a large range, max size_t is enough
	virtual const std::map<Protocol, size_t> &provided_protocols() const = 0;
};

} // ::exampi

#endif
