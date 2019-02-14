#include <algorithm>

#include "matchers/simplematcher.h"
#include "debug.h"
#include "universe.h"

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

	Universe &universe = Universe::get_root_universe();

	if(message->envelope.epoch != universe.epoch)
	{
		debug("silently dropping message from wrong epoch");
		return;
	}

	received_message_queue.push_back(std::move(message));
	change = true;
}

bool SimpleMatcher::progress(Match &match)
//std::tuple<bool, Request_ptr, ProtocolMessage_uptr> SimpleMatcher::progress()
{
	std::lock_guard<std::mutex> lock(guard);

	// check if work is actually available
	if(change && (posted_request_queue.size() > 0)
	        && (received_message_queue.size() > 0))
	{
		change = false;

		debug("found requests and messages to match");

		typedef std::list<ProtocolMessage_uptr>::iterator msg_iter;

		// iterate all posted receives
		for(auto riter = posted_request_queue.begin();
		        riter != posted_request_queue.end(); ++riter)
		{
			Request_ptr req = *riter;

			debug("picked up request: " << req);

			// find match in messages
			auto iterator = std::find_if(received_message_queue.begin(),
			                             received_message_queue.end(),
			                             [&](const ProtocolMessage_uptr &msg) -> bool
			{
				debug("testing match between request and protocol message: epoch " << req->envelope.epoch << " == " << msg->envelope.epoch << ", comm " << req->envelope.context << " == " << msg->envelope.context << ", source " << req->envelope.source << " == " << msg->envelope.source << ", dest " << req->envelope.destination << " == " << msg->envelope.destination << ", tag " << req->envelope.tag << " == " << msg->envelope.tag);

				// minimal matching condition set
				bool condition = (req->envelope.epoch			== msg->envelope.epoch) &&
				                 (req->envelope.context 		== msg->envelope.context) &&
                                 (req->envelope.destination		== msg->envelope.destination);

				// check for MPI_ANY_SOURCE
				if(req->envelope.source != MPI_ANY_SOURCE)
					condition = condition && (req->envelope.source == msg->envelope.source);
			
				// check for MPI_ANY_TAG
				if(req->envelope.tag != MPI_ANY_TAG)
					condition = condition && (req->envelope.tag == msg->envelope.tag);
				
				return condition;
			});

			// if matched
			if(iterator != received_message_queue.end())
			{
				debug("found match, generating match object");

				// fill match object
				match.request = req;
				match.message = std::move(*std::move_iterator<msg_iter>(iterator));

				// remove from respective lists
				posted_request_queue.erase(riter);
				received_message_queue.erase(iterator);

				debug("matched req " << match.request << " <-> " << match.message.get());

				debug("matching complete, found match");

				return true;
				//return std::make_tuple(true, req, std::move(*std::move_iterator<msg_iter>(iterator)));
			}
			else
			{
				// else go to next request

				debug("no match with request");
			}
		}
	}

	//return std::make_tuple(false, nullptr, ProtocolMessage_uptr*(nullptr));
	return false;
}

void SimpleMatcher::halt()
{
	std::lock_guard<std::mutex> lock(guard);

	// invalidate all requests
	for(auto req: posted_request_queue)
	{
		// todo do we need this? is what was given not good enough?
		//req->payload.count = 0;
		req->error = MPIX_TRY_RELOAD;

		// finalize request
		req->release();
	}

	// clear all queues
	posted_request_queue.clear();
	received_message_queue.clear();
}

}
