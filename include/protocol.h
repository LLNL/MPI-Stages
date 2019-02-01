#ifndef __EXAMPI_PROTOCOL_QUEUE_H
#define __EXAMPI_PROTOCOL_QUEUE_H

#include <mutex>
#include <list>

#include "debug.h"
#include "request.h"
#include "envelope.h"

namespace exampi
{

enum class Protocol: int
{
	Eager,
	Rendevouz
};

class ProtocolMessage
{
	// every protocol message has an envelope
	Envelope envelope;
	Protocol protocol;

	// payload / protocol data
};

class ProtocolQueue
{
private:
	std::mutex queue_lock;

	std::list<ProtocolMessage> slots;
	
public:
	ProtocolQueue();
	~ProtocolQueue();
	
	int insert(Request *request);
	int remove(Request *request);
};

}

#endif
