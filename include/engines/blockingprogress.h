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

class BlockingProgress: public Progress
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
};

}

#endif
