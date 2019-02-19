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
