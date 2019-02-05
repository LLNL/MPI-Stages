#ifndef __EXAMPI_SIMPLE_MATCHER_H
#define __EXAMPI_SIMPLE_MATCHER_H

#include <list>
#include <queue>
#include <mutex>

#include "abstract/matcher.h"
#include "protocol.h"

namespace exampi
{

class SimpleMatcher final: public Matcher
{
private:
	std::recursive_mutex guard;

	std::queue<ProtocolMessage_uptr> unexpected_message_queue;
	std::list<Request_ptr> posted_request_queue;

public:
	SimpleMatcher();

	void post_request(Request_ptr request);

	bool has_work();

	bool match(ProtocolMessage_uptr message, Match &match);
	bool progress(Match &match);
};

}

#endif 
