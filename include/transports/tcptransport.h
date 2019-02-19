#ifndef __EXAMPI_TCPTRANSPORT_H
#define __EXAMPI_TCPTRANSPORT_H

#include <mutex>
#include <map>
#include <unordered_map>
#include <sys/socket.h>

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

class TCPTransport: public Transport
{
private:
	std::mutex guard;
	std::map<Protocol, size_t> available_protocols;
	std::unordered_map<int, int> connections;

	int server;
	msghdr hdr;

	int connect(int);

public:
	TCPTransport();
	~TCPTransport();

	const std::map<Protocol, size_t> &provided_protocols() const;

	Header *ordered_recv();
	void fill(const Header *, Request *);
	void reliable_send(const Protocol, const Request *);

	// todo mpi stages
	int save(std::ostream &);
	int load(std::istream &);
	int halt();
};

}

#endif
