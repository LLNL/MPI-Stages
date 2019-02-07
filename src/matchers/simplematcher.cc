#include <algorithm>

#include "matchers/simplematcher.h"

namespace exampi
{

SimpleMatcher::SimpleMatcher()
{
	;
}

void SimpleMatcher::post_request(Request_ptr request)
{
	std::lock_guard<std::mutex> lock(guard);

	posted_request_queue.push_back(request);
}

void SimpleMatcher::post_message(ProtocolMessage_uptr message)
{
	std::lock_guard<std::mutex> lock(guard);

	received_message_queue.push_back(std::move(message));
}

bool SimpleMatcher::match(ProtocolMessage_uptr message, Match &match)
{
	// do search
	ProtocolMessage *msg = message.get();
	
	auto iterator = std::find_if(posted_request_queue.begin(),
	                             posted_request_queue.end(),
	                             [msg](const Request_ptr request) -> bool
	                             {
									// TODO support WILD CARD ANY_TAG AND ANY_SOURCE
	                                return (msg->envelope.epoch        == request->envelope.epoch) &&
	                                       (msg->envelope.communicator == request->envelope.communicator) &&
	                                       (msg->envelope.source       == request->envelope.source) &&
	                                       (msg->envelope.destination  == request->envelope.destination) &&
	                                       (msg->envelope.tag          == request->envelope.tag);
	                             });

	// found match
	bool matched = (iterator != posted_request_queue.end());

	if(matched)
	{
		// return corresponding request
		Request_ptr request = *iterator;
		
		// remove request from prq
		posted_request_queue.remove(request);

		// return match
		match.request = request;
		match.message = std::move(message);
	}
	
	// no match, then absorb message in UMQ
	else
	{
		// TODO no allowed for progress call
		unexpected_message_queue.push_back(std::move(message));
	}
	
	return matched;
}

bool SimpleMatcher::progress(Match &match)
{
	std::lock_guard<std::mutex> lock(guard);
	
	// check if work is actually available
	if((posted_request_queue.size() > 0) && (unexpected_message_queue.size() > 0))
	{
		// with multiple threads need to keep in mind FIFO, single lock works
		ProtocolMessage_uptr message = std::move(unexpected_message_queue.front());

		bool matched = match(std::move(message), match);
		if(matched)	{
			unexpected_message_queue.pop_front();
		}
		else
		{
			unexpected_message_queue.push_front(std::move(message));
		}
	}
	
	return false;
}

}
