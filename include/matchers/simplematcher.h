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

#ifndef __EXAMPI_SIMPLE_MATCHER_H
#define __EXAMPI_SIMPLE_MATCHER_H

#include <list>
#include <mutex>

#include "abstract/matcher.h"

namespace exampi
{

class SimpleMatcher : public Matcher, virtual public Stages
{
private:
	std::mutex guard;

	std::list<Request_ptr> posted_request_queue;
	std::list<Header_uptr> received_header_queue;

public:
	SimpleMatcher();

	void post_request(Request_ptr request);
	void post_header(Header_uptr header);

	std::tuple<Header_uptr, Request *> progress();

	int save(std::ostream &);
	int load(std::istream &);
	int cleanup();
	int halt();
};

}

#endif
