// 
// This is a source code component of ExaMPI, a next-generation 
// implementation of the Message Passing Interface.
//
// This is a joint project of University of Tennessee at Chattanooga,
// the Lawrence Livermore National Laboratory, and Auburn University.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE', which is part of this source code package.
// 

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
typedef Request *Request_ptr;

enum class Protocol: int
{
	// initiators
	EAGER,				// pack data, no ack
	EAGER_ACK,			// data is packed, ack required
	//SEQ,				// pack data, multiple messages
	//SEQ_ACK,			// packed data, multiple messages, ack required

	//EAGER_PUT,		// placed data into buffer on receiver side, preallocated
	//EAGER_PUT_ACK,	// placed data into buffer require acknowledge

	//RENDEZVOUS_PUT,	// send buffer is packed, requesting buffer on receiver
	//RENDEZVOUS_GET,	// send buffer is packed, requesting get from buffer and ack

	// followers
	//SEQ_TERM,
	//SEQ_TERM_ACK,
	ACK,				// no data, ack in response, finalizing any protocol

	//RENDEZVOUS_INFO,
	//RENDEZVOUS_ACK,	// recv buffer is packed, requesting putting into receiver, follow by ack
};

//struct ProtocolMessage
//{
//	Protocol stage;
//	Envelope envelope;
//
//	virtual ~ProtocolMessage() {}
//	virtual int size()
//	{
//		return sizeof(stage) + sizeof(envelope);
//	}
//
//	virtual int pack(const Request_ptr request) = 0;
//	virtual int unpack(Request_ptr request) const = 0;
//};

//typedef std::unique_ptr<ProtocolMessage, std::function<void(ProtocolMessage *)>>
//        ProtocolMessage_uptr;

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
