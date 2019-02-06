#ifndef __EXAMPI_SIMPLE_MATCHER_H
#define __EXAMPI_SIMPLE_MATCHER_H

#include <list>
#include <deque>
#include <mutex>

#include "abstract/matcher.h"
#include "protocol.h"

namespace exampi
{

class SimpleMatcher final: public Matcher
{
private:
	std::recursive_mutex guard;

	std::deque<ProtocolMessage_uptr> unexpected_message_queue;
	std::list<Request_ptr> posted_request_queue;

public:
	SimpleMatcher();

	void post_request(Request_ptr request);

	bool match(ProtocolMessage_uptr message, Match &match);

	bool progress(Match &match);
};

}

#endif 
