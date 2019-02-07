#ifndef __EXAMPI_SIMPLE_MATCHER_H
#define __EXAMPI_SIMPLE_MATCHER_H

#include <list>
#include <deque>
#include <mutex>

#include "abstract/matcher.h"
#include "request.h"
#include "protocol.h"

namespace exampi
{

class SimpleMatcher final: public Matcher
{
private:
	std::mutex guard;

	std::list<Request_ptr> posted_request_queue;
	std::list<ProtocolMessage_uptr> received_message_queue;

	bool change;

public:
	SimpleMatcher();

	void post_request(Request_ptr request);
	void post_message(ProtocolMessage_uptr message);

	//bool match(ProtocolMessage_uptr message, Match &match);

	bool progress(Match &match);
};

}

#endif 
