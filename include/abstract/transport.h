#ifndef __EXAMPI_INTERFACE_TRANSPORT_H
#define __EXAMPI_INTERFACE_TRANSPORT_H

#include <mutex>

#include "mpi.h"
#include "protocol.h"

namespace exampi
{

class Transport
{
private:
	// transport owns all protocol messages
	ProtocolMessage_uptr protocol_message_pool;

public:
	// TODO find solution for settable pool size
	Transport() : protocol_message_pool(128)
	{
		;
	}

	ProtocolMessage_uptr allocate_protocol_message()
	{
		// for request -> protocol messsage -> reliable_send()
		return protocol_message_pool.allocate();
	}

	// stages
	//virtual int save(std::ostream &r) = 0;
	//virtual int load(std::istream &r) = 0;
	//virtual int cleanUp(MPI_Comm comm) = 0;

	virtual ProtocolMessage_uptr peek() = 0;

//	virtual bool peek(ProtocolEnvelope &envelope) = 0;
//	
//	virtual ProtocolMessage_uptr fetch(ProtocolEnvelope &envelope) = 0;
//	virtual bool fill(ProtocolEnvelope &envelope, Payload &payload) = 0;

	virtual int reliable_send(ProtocolMessage_uptr message) = 0;
};

} // ::exampi

#endif
