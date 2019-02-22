#ifndef __EXAMPI_TCPTRANSPORT_H
#define __EXAMPI_TCPTRANSPORT_H

#include <mutex>
#include <map>
#include <unordered_map>

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

class TCPTransport: public Transport
{
private:
	std::mutex guard;
	std::map<Protocol, size_t> available_protocols;
	// world_rank, socket
	std::unordered_map<int, int> connections;

	int server_socket;
	msghdr hdr;

	int connect(int);

public:
	TCPTransport();
	~TCPTransport();

	const std::map<Protocol, size_t> &provided_protocols() const;

	Header_uptr ordered_recv();

	void fill(Header_uptr, Request *);

	void reliable_send(const Protocol, const Request *);

	int save(std::ostream &);
	int load(std::istream &);
	int halt();
};

}

#endif
