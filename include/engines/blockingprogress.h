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

#ifndef __EXAMPI_BLOCKING_PROGRESS_H
#define __EXAMPI_BLOCKING_PROGRESS_H

#include <thread>
#include <unordered_map>
#include <queue>
#include <memory>

#include "abstract/progress.h"
#include "abstract/matcher.h"
#include "abstract/transport.h"
#include "abstract/decider.h"

namespace exampi
{

class BlockingProgress: public Progress, virtual public Stages
{
private:
	bool shutdown;
	int maximum_progress_cycles;

	std::vector<std::thread> progress_threads;
	std::mutex outbox_guard;
	std::queue<Request *> outbox;

	std::unique_ptr<Matcher> matcher;
	std::unique_ptr<Transport> transporter;
	std::unique_ptr<Decider> decider;

	void progress();
	void handle_match(Header_uptr, Request *);
	void handle_request();
	void handle_send(Request *request);

public:
	BlockingProgress();
	BlockingProgress(std::unique_ptr<Matcher> matcher,
	                 std::unique_ptr<Transport> transporter,
	                 std::unique_ptr<Decider> decider);
	~BlockingProgress();

	void post_request(Request *request);

	int save(std::ostream &);
	int load(std::istream &);
	int cleanup();
	int halt();
};

}

#endif
