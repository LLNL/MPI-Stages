#ifndef __EXAMPI_REQUEST_H
#define __EXAMPI_REQUEST_H

#include "ExaMPI.h"
#include "endpoint.h"

namespace exampi
{

class Request
{
public:
	static constexpr size_t HeaderSize = (8 * 4);

protected:
	char hdr[HeaderSize];

public:
	Op op;
	int tag;
	int source;
	MPI_Comm comm;
	UserArray array;

	// FIXME remove iovec, scope violation, socket level things should live in UDPTransport
	struct iovec temp;

	Endpoint endpoint;
	int stage;
	MPI_Status status; // maybe not needed --sf

	std::promise<MPI_Status> completionPromise;

	void pack()
	{
		uint16_t *word = (uint16_t *) hdr;
		uint32_t *dword = (uint32_t *) hdr;
		word[0] = 0xDEAF; // magic word
		word[1] = 22;  // protocol
		word[2] = 42;  // message type
		word[3] = 0x0; // function
		dword[2] = 0x0;  // align
		dword[3] = stage;  // align/reserved
		dword[4] = source;
		dword[5] = tag;
		dword[6] = comm; // context; not yet
		dword[7] = 0xAABBCCDD;  // CRC
	}

	void unpack()
	{
		uint32_t *dword = (uint32_t *) hdr;
		stage = dword[3];
		source = dword[4];
		tag = dword[5];
		comm = dword[6];
	}

	struct iovec getHeaderIovec()
	{
		pack();
		struct iovec iov = { hdr, HeaderSize };
		return iov;
	}

	std::vector<struct iovec> getHeaderIovecs()
	{
		std::vector<struct iovec> iov;
		iov.push_back(getHeaderIovec());
		return iov;
	}

	std::vector<struct iovec> getArrayIovecs()
	{
		std::vector<struct iovec> iov;
		iov.push_back(array.getIovec());
		return iov;
	}

	std::vector<struct iovec> getIovecs()
	{
		std::vector<struct iovec> iov;
		iov.push_back(getHeaderIovec());
		iov.push_back(array.getIovec());
		return iov;
	}

	std::vector<struct iovec> getTempIovecs()
	{
		std::vector<struct iovec> iov;
		iov.push_back(getHeaderIovec());
		char tempBuff[65000];
		temp.iov_base = tempBuff;
		temp.iov_len = sizeof(tempBuff);
		iov.push_back(temp);
		return iov;
	}
};

}

#endif
