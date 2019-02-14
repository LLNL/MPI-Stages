#ifndef __EXAMPI_ABSTRACT_MATCHING_H
#define __EXAMPI_ABSTRACT_MATCHING_H

#include "request.h"
#include "header.h"

namespace exampi
{

struct Match
{
	Request_ptr request;
	//ProtocolMessage_uptr message;
	Header *header;
};

class Matcher
{
public:
	virtual void post_request(Request_ptr request) = 0;

	//virtual void post_message(const ProtocolMessage_uptr message) = 0;
	virtual void post_header(Header* header) = 0;

	virtual bool progress(Match &match) = 0;
	//virtual std::tuple<bool, Request_ptr, ProtocolMessage_uptr> progress() = 0;

	virtual void halt() = 0;
};

}

#endif
