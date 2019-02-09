#ifndef __EXAMPI_PROTOCOL_QUEUE_H
#define __EXAMPI_PROTOCOL_QUEUE_H

#include <mutex>
#include <list>

#include "debug.h"
#include "envelope.h"
#include "pool.h"

namespace exampi
{

struct Request;
typedef Request* Request_ptr;

enum class Protocol: int
{
	// initiators
	EAGER,			// pack data, no ack
	EAGER_ACK,		// data is packed, ack required
	//EAGER_SEQ,		// pack data, multiple messages
	//EAGER_SEQ_ACK,	// packed data, multiple messages, ack required

	//RENDEVOUZ_PUT,	// send buffer is packed, requesting buffer on receiver
	//RENDEVOUZ_GET,	// send buffer is packed, requesting get from buffer and ack

	// followers
	//EAGER_SEQ_TERM,
	//EAGER_SEQ_TERM_ACK,
	ACK,			// no data, ack in response, finalizing any protocol

	//RENDEVOUZ_ACK,	// recv buffer is packed, requesting putting into receiver, follow by ack
};

struct ProtocolMessage
{
	Protocol stage; 
	Envelope envelope;

	virtual int pack(const Request_ptr request) = 0;
	virtual int unpack(Request_ptr request) const = 0;
};

typedef std::unique_ptr<ProtocolMessage, std::function<void(ProtocolMessage*)>> ProtocolMessage_uptr;

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
