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

#ifndef __EXAMPI_TCPTRANSPORT_H
#define __EXAMPI_TCPTRANSPORT_H

#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>

#include "abstract/transport.h"
#include "pool.h"

namespace exampi
{

class TCPTransportSocketCreationFailed: public std::exception
{
	const char *what() const noexcept override
	{
		return "TCPTransport failed to create socket for sending.";
	}
};

class TCPTransportConnectionFailed: public std::exception
{
	const char *what() const noexcept override
	{
		return "TCPTransport failed to connection client socket for sending.";
	}
};

class TCPTransportNonBlockError: public std::exception
{
	const char *what() const noexcept override
	{
		return "TCPTransport failed to set non-blocking mode for server socket.";
	}
};

class TCPTransportSendError: public std::exception
{
	const char *what() const noexcept override
	{
		return "TCPTransport reliable send failed.";
	}
};

class TCPTransportBindError: public std::exception
{
	const char *what() const noexcept override
	{
		return "TCPTransport failed to bind socket.";
	}
};

class TCPTransportListenError: public std::exception
{
	const char *what() const noexcept override
	{
		return "TCPTransport failed on socket listen.";
	}
};

class TCPTransportPollError: public std::exception
{
	const char *what() const noexcept override
	{
		return "TCPTransport polling on sockets failed.";
	}
};

class TCPTransportFillError: public std::exception
{
	const char *what() const noexcept override
	{
		return "TCPTransport failed to fill a message.";
	}
};

class TCPTransportPeekError: public std::exception
{
	const char *what() const noexcept override
	{
		return "TCPTransport failed to peek for the message header.";
	}
};

class TCPTransport: public Transport, virtual public Stages
{
private:
	std::mutex guard;
	std::map<Protocol, size_t> available_protocols;
	// world_rank, socket fd
	std::unordered_map<int, int> connections;
	std::vector<struct pollfd> fds;
	MemoryPool<Header> header_pool;

	int server_socket;
	msghdr hdr;

	size_t arrivals;
	size_t peeked;

	int rank_connect(int);
	Header_uptr iterate();

public:
	TCPTransport();
	~TCPTransport();

	const std::map<Protocol, size_t> &provided_protocols() const;

	Header_uptr ordered_recv();
	void fill(Header_uptr, Request *);
	void reliable_send(const Protocol, const Request *);

	int save(std::ostream &);
	int load(std::istream &);
	int cleanup();
	int halt();
};

}

#endif
