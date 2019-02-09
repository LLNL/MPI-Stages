#include <algorithm>

#include "matchers/simplematcher.h"
#include "debug.h"

namespace exampi
{

SimpleMatcher::SimpleMatcher() : change(false)
{
	;
}

void SimpleMatcher::post_request(Request_ptr request)
{
	std::lock_guard<std::mutex> lock(guard);
	debug("posting request in matcher");

	posted_request_queue.push_back(request);
	change = true;
}

void SimpleMatcher::post_message(ProtocolMessage_uptr message)
{
	std::lock_guard<std::mutex> lock(guard);
	debug("posting message in matcher");

	received_message_queue.push_back(std::move(message));
	change = true;
}

bool SimpleMatcher::progress(Match &match)
{
	std::lock_guard<std::mutex> lock(guard);
	
	// check if work is actually available
	if(change && (posted_request_queue.size() > 0) && (received_message_queue.size() > 0))
	{
		change = false;

		debug("found requests and messages to match");

		// TODO this is an inefficient implementation, O(n^2)
		// but atleast logically separated receive and match

		typedef std::list<ProtocolMessage_uptr>::iterator msg_iter;

		// iterate all posted receives
		for(auto riter = posted_request_queue.begin(); riter != posted_request_queue.end(); ++riter)
		{
			Request_ptr req = *riter;

			debug("picked up request: " << req);

			// find match in messages
			auto iterator = std::find_if(received_message_queue.begin(),
	                             	 	 received_message_queue.end(),
	                             	 	 [&](const ProtocolMessage_uptr &msg) -> bool
	                             	 	 {
											debug("testing match between request and protocol message: epoch " << req->envelope.epoch << " == " << msg->envelope.epoch << ", comm " << req->envelope.communicator << " == " << msg->envelope.communicator << ", source " << req->envelope.source << " == " << msg->envelope.source << ", dest " << req->envelope.destination << " == " << msg->envelope.destination << ", tag " << req->envelope.tag << " == " << msg->envelope.tag);
											
											// TODO support WILD CARD ANY_TAG AND ANY_SOURCE
	                                		return (req->envelope.epoch        == msg->envelope.epoch) &&
	                                       	   	   (req->envelope.communicator == msg->envelope.communicator) &&
	                                       	   	   (req->envelope.source       == msg->envelope.source) &&
	                                       	   	   (req->envelope.destination  == msg->envelope.destination) &&
	                                       	   	   (req->envelope.tag          == msg->envelope.tag);
	                             	 	 });

			// if matched
			if(iterator != received_message_queue.end())
			{
				debug("found match, generating match object");

				// fill match object
				match.request = req;
				match.message = std::move(*std::move_iterator<msg_iter>(iterator));

				// remove from respective lists
				debug("removing list items");
				posted_request_queue.erase(riter);
				received_message_queue.erase(iterator);

				debug("matching complete, found match");

				return true;
			}
			else
			{
				// else go to next request

				debug("no match with request");
			}
		}
					
	}

	return false;
}

}
