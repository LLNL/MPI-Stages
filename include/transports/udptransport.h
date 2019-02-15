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

class UDPTransport: public Transport
{
private:
	std::recursive_mutex guard;

	int socket_recv;

	MemoryPool<Header> header_pool;
	std::unordered_map<const Header *, void*> data_buffer;

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

	// TODO remove mpi stages
	int save(std::ostream &r);
	int load(std::istream &r);
	int halt();
};

}

#endif
