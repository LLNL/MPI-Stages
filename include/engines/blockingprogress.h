#ifndef __EXAMPI_BLOCKING_PROGRESS_H
#define __EXAMPI_BLOCKING_PROGRESS_H

#include <thread>

#include "abstract/progress.h"

namespace exampi
{

class BlockingProgress: public Progress
{
private:
	std::vector<std::thread> progress_threads;
	
public:
	BlockingProgress();
	~BlockingProgress();	

	postSend();
	postRecv();
};

}

#endif
