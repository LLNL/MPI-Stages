#ifndef __EXAMPI_BLOCKING_PROGRESS_H
#define __EXAMPI_BLOCKING_PROGRESS_H

#include <thread>
#include <unordered_map>
#include <queue>

#include "abstract/progress.h"
#include "abstract/matcher.h"
#include "abstract/transport.h"

#include "matchers/simplematcher.h"
#include "transports/udptransport.h"

#include "daemon.h"
#include "request.h"
#include "protocol.h"
#include "sigHandler.h"

namespace exampi
{

class BlockingProgress: public Progress
{
private:
	bool shutdown;
	int maximum_progress_cycles;

	std::vector<std::thread> progress_threads;

	std::mutex outbox_guard;
	std::queue<Request *> outbox;

	std::shared_ptr<Matcher> matcher;
	std::shared_ptr<Transport> transporter;

	void progress();

	int handle_match(Match &match);
	int handle_request();

	int handle_send(Request *request);

public:
	BlockingProgress();
	BlockingProgress(std::shared_ptr<Matcher> matcher,
	                 std::shared_ptr<Transport> transporter);
	~BlockingProgress();

	int post_request(Request *request);

	// mpi stages, figure out how to do this, separate into another progress inheriting?
	//void cleanup();
	int halt();
	int save(std::ostream &);
	int load(std::istream &);
};

}

#endif
