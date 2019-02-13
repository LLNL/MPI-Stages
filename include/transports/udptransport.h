#ifndef __EXAMPI_UDPTRANSPORT_H
#define __EXAMPI_UDPTRANSPORT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>
#include <unordered_map>
#include <exception>

#include "abstract/transport.h"
#include "protocol.h"

namespace exampi
{

struct UDPProtocolMessage: public ProtocolMessage
{
	~UDPProtocolMessage() final {}
	int size() final
	{
		return ProtocolMessage::size() + sizeof(payload);
	}

	int payload[10];

	int pack(const Request_ptr request) final;
	int unpack(Request_ptr request) const final;
};

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

	MemoryPool<UDPProtocolMessage> message_pool;

	msghdr hdr;

	// TODO caching per rank does not work, needs per communicator...
	std::unordered_map<long int, sockaddr_in> cache;

	std::map<Protocol, size_t> available_protocols;

public:
	UDPTransport();
	~UDPTransport();

	ProtocolMessage_uptr allocate_protocol_message();

	const ProtocolMessage_uptr ordered_recv();

	int reliable_send(const ProtocolMessage_uptr message);

	const std::map<Protocol, size_t> &provided_protocols() const;

	// mpi stages
	int save(std::ostream &r);
	int load(std::istream &r);
	int halt();
};

}

#endif
