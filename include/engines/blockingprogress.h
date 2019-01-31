#ifndef __EXAMPI_BLOCKING_PROGRESS_H
#define __EXAMPI_BLOCKING_PROGRESS_H

#include <thread>
#include <vector>

// TODO remove these
#include <iostream>

#include "abstract/progress.h"
#include "matchers/simplematcher.h"
#include "request.h"
#include "protocolqueue.h"

namespace exampi
{

class BlockingProgress: public Progress
{
private:
	std::vector<std::thread> progress_threads;

	ProtocolQueue protocol_queue;
	Matcher *matcher;
	
public:
	BlockingProgress();
	~BlockingProgress();	

	// TODO remove these
	int init();
	int init(std::istream &t);
	void finalize();

	int post_request(Request *request);

	int stop();
	void cleanUp();

	int load(std::istream&);
	int save(std::ostream&);
};

}

#endif
