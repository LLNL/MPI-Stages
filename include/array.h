#ifndef __EXAMPI_ARRAY_H
#define __EXAMPI_ARRAY_H

#include <mpi.h>
#include <datatype.h>


namespace exampi
{

class UserArray
{
public:
	void *ptr;
	Datatype *datatype;
	size_t count;
	struct iovec getIovec()
	{
		struct iovec iov = {ptr, datatype->getExtent() *count};
		return iov;
	}
	// only really useful for debugging:
	std::string toString()
	{
		std::stringstream stream;
		stream <<"UserArray{ptr=" << ptr << ",datatype=" << datatype << ",count=" << count << "}";
		return stream.str();
	}
};



} // ::exampi

#endif
