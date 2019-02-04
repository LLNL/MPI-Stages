#ifndef __EXAMPI_PROTOCOL_QUEUE_H
#define __EXAMPI_PROTOCOL_QUEUE_H

#include <mutex>
#include <list>

#include "debug.h"
#include "request.h"
#include "envelope.h"
#include "pool.h"

namespace exampi
{

enum class ProtocolType: int
{
	EAGER,
	EAGER_ACK,
	RENDEVOUZ
};

struct ProtocolMessage
{
	// protocol message type
	ProtocolType type;

	// every protocol message has an envelope
	Envelope envelope;

	// payload / protocol data
	union data
	{
		// nothing should exceed this
		int pad[11];

		// buffer, datatype, count
		Payload payload;
		
		
	};
};

typedef MemoryPool<ProtocolMessage>::unique_ptr ProtocolMessage_uptr;

//class ProtocolQueue
//{
//private:
//	std::mutex queue_lock;
//
//	std::list<ProtocolMessage> slots;
//	
//public:
//	ProtocolQueue();
//	~ProtocolQueue();
//	
//	int insert(Request *request);
//	int remove(Request *request);
//};

}

#endif
