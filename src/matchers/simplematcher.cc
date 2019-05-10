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

#include <algorithm>

#include "matchers/simplematcher.h"
#include "debug.h"
#include "universe.h"
#include "faulthandler.h"

namespace exampi
{

SimpleMatcher::SimpleMatcher()
{
	;
}

void SimpleMatcher::post_request(Request_ptr request)
{
	std::lock_guard<std::mutex> lock(guard);
	debug("posting request in matcher");

	posted_request_queue.push_back(request);
}

void SimpleMatcher::post_header(Header_uptr header)
{
	std::lock_guard<std::mutex> lock(guard);
	debug("posting message in matcher");

	Universe &universe = Universe::get_root_universe();

	if (header->envelope.tag == MPIX_CLEANUP_TAG)
	{
		debug("Cleanup request from fault handler");
		halt();
	}

	if(header->envelope.epoch < universe.epoch)
	{
		debug("silently dropping message from wrong epoch");
		return;
	}

	received_header_queue.push_back(std::move(header));
}

std::tuple<Header_uptr, Request *> SimpleMatcher::progress()
{
	std::lock_guard<std::mutex> lock(guard);

	// check if work is actually available
	if((posted_request_queue.size() > 0) && (received_header_queue.size() > 0))
	{
		debug("found requests and messages to match");
		debug("requests " << posted_request_queue.size() << " : headers " <<
		      received_header_queue.size());

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
				bool condition = (req->envelope.epoch               == header->envelope.epoch) &&
				(req->envelope.context             == header->envelope.context) &&
				(req->envelope.destination         == header->envelope.destination);

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

int SimpleMatcher::cleanup()
{
	FaultHandler &faulthandler = FaultHandler::get_instance();
	Universe &universe = Universe::get_root_universe();
	if (posted_request_queue.size() > 0)
	{
		faulthandler.setErrToZero();
		universe.interface->MPI_Send((void *)0, 0, MPI_INT, universe.rank,
		                             MPIX_CLEANUP_TAG, MPI_COMM_WORLD);
		faulthandler.setErrToOne();
	}
	return MPI_SUCCESS;
}

int SimpleMatcher::halt()
{
	//std::lock_guard<std::mutex> lock(guard);
	debug("posted_queue_size: " << posted_request_queue.size());
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
	return MPI_SUCCESS;
}

int SimpleMatcher::save(std::ostream &)
{
	return MPI_SUCCESS;
}

int SimpleMatcher::load(std::istream &)
{
	return MPI_SUCCESS;
}

}
