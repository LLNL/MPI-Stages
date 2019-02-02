#include "matchers/simplematcher.h"

namespace exampi
{

SimpleMatcher::SimpleMatcher() : new_receives(0)
{
	;
}

void SimpleMatcher::post_request(Request_ptr request)
{
	std::lock_guard<std::mutex> lock(guard);

	posted_receive_queue.push_back(request);
	++new_receives;
}

bool match(ProtocolMessage_uptr message, Match &match)
{
	std::lock_guard<std::mutex> lock(guard);

	// do search
	auto iterator = std::find_if(posted_request_queue.begin(),
	                             posted_request_queue.end(),
	                             [message](const Request_ptr request) -> bool
	                             {
									// TODO support WILD CARD TAG AND SOURCE
	                                return (message->envelope.epoch        == request->envelope.epoch) &&
	                                       (message->envelope.communicator == request->envelope.communicator) &&
	                                       (message->envelope.source       == request->envelope.source) &&
	                                       (message->envelope.destination  == request->envelope.destination) &&
	                                       (message->envelope.tag          == request->envelope.tag);
	                             });

	// found match
	if(bool matched = (iterator != posted_request_queue.end()))
	{
		// return corresponding request
		Request_ptr request = *iterator;
		
		// remove request from prq
		posted_request_queue.remove(request);

		match.request = request;
		match.message = std::move(message);
	}
	
	return matched;
}

bool SimpleMatcher::progress(Match &match)
{
	std::lock_guard<std::mutex> lock(guard);
	
	// check if work is actually available
	if(has_work())
		// TODO spread work over threads accessing
		//		not always front
		ProtocolMessage_uptr message = unexpected_message_queue.front();

		return match(message, match);
	}
	else
		return false;
}

bool SimpleMatcher::has_work()
{
	return (new_receives > 0);
}

}
