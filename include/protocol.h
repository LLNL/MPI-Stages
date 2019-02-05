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

enum class ProtocolStage: int
{
	ACK				// no data, ack in response, finalizing any protocol

	EAGER,			// pack data, no ack
	
	EAGER_SEQ,		// pack data, multiple messages

	EAGER_ACK,		// data is packed, ack required
	
	EAGER_SEQ_ACK,	// packed data, multiple messages, ack required

	RENDEVOUZ_PUT,	// send buffer is packed, requesting buffer on receiver
	RENDEVOUZ_ACK,	// recv buffer is packed, requesting putting into receiver, follow by ack

	RENDEVOUZ_GET,	// send buffer is packed, requesting get from buffer and ack
};

struct ProtocolMessage
{
	ProtocolStage stage; 
	Envelope envelope;

	// payload / protocol data
	union
	{
		// nothing should exceed this
		int data[10];

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
