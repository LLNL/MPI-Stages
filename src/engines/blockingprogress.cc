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

#include <cstring>

#include "engines/blockingprogress.h"
#include "exceptions.h"
#include "daemon.h"
#include "matchers/simplematcher.h"
#include "transports/udptransport.h"
#include "deciders/simpledecider.h"

#include "transports/tcptransport.h"

namespace exampi
{

BlockingProgress::BlockingProgress() :
	BlockingProgress(
	    std::unique_ptr<Matcher>(new SimpleMatcher()),
	    std::unique_ptr<Transport>(new UDPTransport()),
	    //std::unique_ptr<Transport>(new TCPTransport()),
	    std::unique_ptr<Decider>(new SimpleDecider())
	)
{

}

BlockingProgress::BlockingProgress(std::unique_ptr<Matcher> matcher,
                                   std::unique_ptr<Transport> transporter,
                                   std::unique_ptr<Decider> decider) :
	shutdown(false),
	maximum_progress_cycles(10),
	matcher(std::move(matcher)),
	transporter(std::move(transporter)),
	decider(std::move(decider))
{
	// todo fetch progress thread count from config
	//for(size_t tidx = 0; tidx < thread_num; ++tidx)
	for(size_t tidx = 0; tidx < 1; ++tidx)
	{
		debug("starting progress thread " << tidx);
		// launch thread executing progress function
		std::thread thr(&BlockingProgress::progress, this);
		progress_threads.push_back(std::move(thr));
	}
	debug("progress threads running");
}

BlockingProgress::~BlockingProgress()
{
	if(!shutdown)
	{
		// join all threads
		debug("halting all progress threads");
		shutdown = true;

		debug("informed all threads to shutdown");
		for(auto &&thr : this->progress_threads)
		{
			thr.join();
		}

		debug("joined all threads");
	}
}

void BlockingProgress::post_request(Request *request)
{
	// user threads relinquishes control here
	if(request->operation == Operation::Receive)
	{
		// insert into matching mechanism
		matcher->post_request(request);

		debug("handed request Receive to matcher");

		return;
	}
	else if(request->operation == Operation::Bsend)
	{
		// note a bsend forces packing
		// note we do this in the user thread to ensure correctness

		// copy user buffer
		// todo size_t size = request->payload.datatype.size() * request->payload.count;
		size_t size = sizeof(int) * request->payload.count;
		void *tmp_buffer = malloc(size);

		void *err = std::memcpy(tmp_buffer, request->payload.buffer, size);
		if(err == nullptr)
		{
			throw BsendCopyError();
		}

		// replace request payload
		request->payload.buffer = tmp_buffer;
		// todo request->payload.datatype = ;
		// todo request->payload.count = ;
	}

	// acquire safety
	std::lock_guard<std::mutex> lock(outbox_guard);

	// queue for send
	outbox.push(request);

	debug("put *send request into outbox");
}

void BlockingProgress::progress()
{
	// debug_add_thread()

	int cycles = 0;

	// keep progress threads alive
	while(!this->shutdown)
	{
		// note this is actually just a slow poll
		// 		blocking would be woken up by post_request, transporter ordered_recv()

		// absorb message if any, this is inflow
		if(Header_uptr header = transporter->ordered_recv())
		{
			debug("progress thread, receiving message");

			matcher->post_header(std::move(header));
		}

		// match message if any, this is inflow
		else if(auto [header, request] = matcher->progress(); request != nullptr)
		{
			debug("progress thread, matched header " << header.get() << " == request " <<
			      request);

			handle_match(std::move(header), request);
		}

		// emit message if any, this is outflow
		else if(outbox.size() > 0)
		{
			debug("progress thread, handling request");

			handle_request();

			debug("sent message");
		}

		// otherwise go to sleep
		else
		{
			cycles = 0;
			// todo debug record number of sleep cycles for debugging since last work
			std::this_thread::yield();
		}

		// if we are doing work, check work amount
		cycles += 1;
		if(cycles >= maximum_progress_cycles)
		{
			debug("progress thread, going to sleep forcefully");

			cycles = 0;
			std::this_thread::yield();
		}
	}

	debug("progress thread exiting " << std::this_thread::get_id());
}

void BlockingProgress::handle_match(Header_uptr header, Request *request)
{
	// note this is the big switch or dictionary protocol handling

	debug("handling matched request <-> protocol message");

	// todo put this into a dictionary/map
	switch(header->protocol)
	{
		case Protocol::EAGER_ACK:
		{
			debug("protocol message: EAGER_ACK");

			// return ACK message
			transporter->reliable_send(Protocol::ACK, request);

			[[fallthrough]];
		}

		case Protocol::EAGER:
		{
			debug("protocol message: EAGER");

			transporter->fill(std::move(header), request);

			request->release();

			debug("request completed and released");
			break;
		}

		case Protocol::ACK:
		{
			debug("protocol message: ACK");

			request->release();

			debug("request completed and released");
			break;
		}
	}
}

void BlockingProgress::handle_request()
{
	std::unique_lock<std::mutex> lock(outbox_guard);

	// emit message if there is a request
	if(outbox.size() > 0)
	{
		debug("fetching request");

		// fetch request
		Request *request = outbox.front();
		outbox.pop();

		lock.unlock();

		// todo currently only sends are implemented here
		//      will require a switch statement on type of operation
		handle_send(request);
	}
	else
	{
		debug("false handle_request");
	}
}

void BlockingProgress::handle_send(Request *request)
{
	// request -> ProtocolMessage
	Universe &universe = Universe::get_root_universe();
	Protocol protocol = decider->decide(request, universe);

	// send message
	transporter->reliable_send(protocol, request);

	// complete request
	request->release();
}

int BlockingProgress::halt()
{
	return MPI_SUCCESS;
}

int BlockingProgress::cleanup()
{
	return matcher->cleanup();
}

int BlockingProgress::save(std::ostream &t)
{
	// delegate further
	int err = MPI_SUCCESS;

	err = transporter->save(t);

	return err;
}

int BlockingProgress::load(std::istream &t)
{
	int err = MPI_SUCCESS;

	err = transporter->load(t);

	return err;
}

} // exampi
