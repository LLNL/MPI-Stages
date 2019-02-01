#ifndef __EXAMPI_I_TRANSPORT_H
#define __EXAMPI_I_TRANSPORT_H

#include <mutex>

#include "mpi.h"
#include "protocol.h"
#include "pool.h"

namespace exampi
{

class Transport
{
private:
	// transport owns all protocol messages
	MemoryPool<ProtocolMessage> protocol_message_pool;

public:
	// TODO find solution for settable pool size
	Transport() : protocol_message_pool(128);
	{
		;
	}

	std::unique_ptr<ProtocolMessage> allocate_protocol_message()
	{
		return protocol_message_pool.allocate();
	}

	// stages
	virtual int save(std::ostream &r) = 0;
	virtual int load(std::istream &r) = 0;
	virtual int cleanUp(MPI_Comm comm) = 0;

	virtual std::unique_ptr<ProtocolMessage> absorb() = 0;
	//{
	//	std::lock_guard<std::mutex> lock(guard);
	//	if(protocol_queue.size() == 0)
	//		return std::unique_ptr<ProtocolMessage>(nullptr);
	//	
	//	std::unique_ptr<ProtocolMessage> msg = protocol_queue.front()
	//	protocol_queue.pop();

	//	return msg;
	//}

	virtual int reliable_send(std::unique_ptr<ProtocolMessage> message) = 0;
};

} // ::exampi

#endif
