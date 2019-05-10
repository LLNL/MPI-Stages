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

#ifndef __EXAMPI_EXCEPTIONS_H
#define __EXAMPI_EXCEPTIONS_H

namespace exampi
{

class PersistentOffloadOperationError: public std::exception
{
	const char *what() const noexcept override
	{
		return "An operation which is not persistent offload was given.";
	}
};

class BsendCopyError: public std::exception
{
	const char *what() const noexcept override
	{
		return "std::memcpy failed to copy Bsend user buffer.";
	}
};

}

#endif
