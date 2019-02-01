#include "matchers/simplematcher.h"

namespace exampi
{

int SimpleMatcher::post(Request *request)
{
	return -1;
	// simple insert into queue
	// user threads have better things to do than do matching
	
	// increment
}

int SimpleMatcher::match(Request *request)
{
	return -1;
	// search for a match, if non found, unexpected message
}

int SimpleMatcher::progress()
{
	// need quick check if something has changed
	return -1;
	// do a generic matching run
}

}

//	// search unexpected message queue
//	debugpp("searching unexpected message queue");
//	unexpectedlock.lock();
//	matchlock.lock();
//	auto res = std::find_if(unexpectedlist.begin(), unexpectedlist.end(),
//	                        [tag,s, c, e](const std::unique_ptr<request> &i) -> bool {i->unpack(); return i->tag == tag && i->source == s && i->stage == e && i->comm == c;});
//
//	//
//	if (res == unexpectedlist.end())
//	{
//		debugpp("no match in unexpectedlist, push");
//
//		// put request into match list for later matching
//		unexpectedlock.unlock();
//		matchlist.push_back(std::move(r));
//		matchlock.unlock();
//	}
//	else
//	{
//		// found in umq
//		matchlock.unlock();
//
//		debugpp("found match in unexpectedlist");
//
//		(*res)->unpack();
//		//memcpy(array.ptr, )
//		memcpy(array.getiovec().iov_base, (*res)->temp.iov_base,
//		       array.getiovec().iov_len);
//		(r)->completionpromise.set_value( { .count = (*res)->status.count, .cancelled = 0,
//		                                    .mpi_source = (*res)->source, .mpi_tag = (*res)->tag, .mpi_error = mpi_success});
//		unexpectedlist.erase(res);
//		unexpectedlock.unlock();
//
//	}
//
//	return result;
//}

