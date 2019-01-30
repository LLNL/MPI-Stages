#ifndef __EXAMPI_BLOCKING_PROGRESS_H
#define __EXAMPI_BLOCKING_PROGRESS_H

#include <thread>
#include <vector>

// TODO remove these
#include <iostream>

#include "abstract/progress.h"
#include "request.h"

namespace exampi
{

class BlockingProgress: public Progress
{
private:
	std::vector<std::thread> progress_threads;
	
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
};

}

#endif
