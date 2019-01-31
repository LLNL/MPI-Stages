#include "matchers/simplematcher.h"

namespace exampi
{

int SimpleMatcher::post(Request *request)
{
	return -1;
	// simple insert into queue
	// user threads have better things to do than do matching
}

int SimpleMatcher::match(Request *request)
{
	return -1;
	// search for a match, if non found, unexpected message
}

int SimpleMatcher::progress()
{
	return -1;
	// do a generic matching run
}

}
