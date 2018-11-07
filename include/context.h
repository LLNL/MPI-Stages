#ifndef __EXAMPI_CONTEXT_H
#define __EXAMPI_CONTEXT_H

#include <mutex>

namespace exampi
{

class Context
{
public:
	Context();
	~Context();
	static volatile int nextID;
	static std::mutex contextLock;
	//static int get_next_context();
protected:
private:

};


}

#endif
