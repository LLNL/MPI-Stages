#ifndef __EXAMPI_UDPTRANSPORT_H
#define __EXAMPI_UDPTRANSPORT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>
#include <unordered_map>
#include <exception>
#include <map>

#include "abstract/transport.h"

namespace exampi
{

class UDPTransportCreationException: public std::exception
{
	const char *what() const noexcept override
	{
		return "UDPTransport failed to create socket.";
	}
};

class UDPTransportBindingException: public std::exception
{
	const char *what() const noexcept override
	{
		return "UDPTransport failed to bind socket.";
	}
};

class UDPTransportSendError: public std::exception
{
	const char *what() const noexcept override
	{
		return "UDPTransport reliable send failed.";
	}
};

class UDPTransportFillError: public std::exception
{
	const char *what() const noexcept override
	{
		return "UDPTransport failed to fill payload of request with message.";
	}
};

class UDPTransportHeaderReceiveError: public std::exception
{
	const char *what() const noexcept override
	{
		return "UDPTransport failed to receive header.";
	}
};


class UDPTransportPayloadReceiveError: public std::exception
{
	const char *what() const noexcept override
	{
		return "UDPTransport failed to receive payload.";
	}
};

struct UDPTransportPayload
{
	int payload[(1024 - sizeof(Header))/sizeof(int)];
};

class UDPTransport: public Transport
{
private:
	std::mutex guard;

	int socket_recv;

	MemoryPool<Header> header_pool;
	MemoryPool<UDPTransportPayload> payload_pool;

	std::unordered_map<const Header *, UDPTransportPayload *> payload_buffer;
	
	msghdr hdr;

	// TODO caching per rank does not work, needs per communicator...
	std::unordered_map<long int, sockaddr_in> cache;

	std::map<Protocol, size_t> available_protocols;

public:
	UDPTransport();
	~UDPTransport();

	Header *ordered_recv();
	void fill(const Header*, Request*);

	void reliable_send(const Protocol, const Request *);

	const std::map<Protocol, size_t> &provided_protocols() const;

	// todo isolate mpi stages
	int save(std::ostream &r);
	int load(std::istream &r);
	int halt();
};

}

#endif
