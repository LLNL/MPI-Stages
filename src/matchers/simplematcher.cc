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

void SimpleMatcher::post_header(Header_uptr header)
{
	std::lock_guard<std::mutex> lock(guard);
	debug("posting message in matcher");

	Universe &universe = Universe::get_root_universe();

	if(header->envelope.epoch < universe.epoch)
	{
		debug("silently dropping message from wrong epoch");
		return;
	}

	received_header_queue.push_back(std::move(header));

	change = true;
}

std::tuple<Header_uptr, Request *> SimpleMatcher::progress()
{
	std::lock_guard<std::mutex> lock(guard);

	// check if work is actually available
	if(change && (posted_request_queue.size() > 0)
	        && (received_header_queue.size() > 0))
	{
		change = false;

		debug("found requests and messages to match");

		typedef std::list<Header_uptr>::iterator hdr_iter;

		// iterate all posted receives
		for(auto riter = posted_request_queue.begin();
		        riter != posted_request_queue.end(); ++riter)

		{
			Request_ptr req = *riter;

			debug("picked up request: " << req);

			// find match in messages
			auto iterator = std::find_if(received_header_queue.begin(),
			                             received_header_queue.end(),
			                             [&](Header_uptr &header) -> bool
			{
				// todo make convience debug_print_header(header)
				debug("testing match between request <-> header: epoch " << req->envelope.epoch << " == " << header->envelope.epoch << ", comm " << req->envelope.context << " == " << header->envelope.context << ", source " << req->envelope.source << " == " << header->envelope.source << ", dest " << req->envelope.destination << " == " << header->envelope.destination << ", tag " << req->envelope.tag << " == " << header->envelope.tag);

				// minimal matching condition set
				bool condition = (req->envelope.epoch			== header->envelope.epoch) &&
								(req->envelope.context 			== header->envelope.context) &&
							(req->envelope.destination			== header->envelope.destination);

				// check for MPI_ANY_SOURCE
				if(req->envelope.source != MPI_ANY_SOURCE)
					condition = condition && (req->envelope.source == header->envelope.source);

				// check for MPI_ANY_TAG
				if(req->envelope.tag != MPI_ANY_TAG)
					condition = condition && (req->envelope.tag == header->envelope.tag);

				return condition;
			});

			// if matched
			if(iterator != received_header_queue.end())
			{
				debug("found match, generating match object");

				Header_uptr header = std::move(*std::move_iterator<hdr_iter>(iterator));

				debug("header " << header.get() << " == " << req);

				// remove from respective lists
				posted_request_queue.erase(riter);
				received_header_queue.erase(iterator);

				debug("matching complete, found match");

				return std::make_tuple(std::move(header), req);
			}
			else
			{
				// else go to next request

				debug("no match with request");
			}
		}
	}

	return std::make_tuple(std::unique_ptr<Header>(nullptr), nullptr);
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
	//received_message_queue.clear();
}

}
