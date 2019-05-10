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
