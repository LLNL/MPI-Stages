#ifndef __EXAMPI_UDPTRANSPORT_H
#define __EXAMPI_UDPTRANSPORT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>
#include <unordered_map>

#include "abstract/transport.h"
#include "protocol.h"

namespace exampi
{

struct UDPProtocolMessage: public ProtocolMessage
{
	int payload[10];
	
	int pack(const Request_ptr request);
	int unpack(Request_ptr request) const;
};

class UDPTransport: public Transport
{
private:
	std::mutex guard;

	int socket_recv;

	MemoryPool<UDPProtocolMessage> message_pool;

	msghdr hdr;

	// TODO caching per rank does not work, needs per communicator...
	std::unordered_map<long int, sockaddr_in> cache;

public:
	UDPTransport();
	~UDPTransport();

	ProtocolMessage_uptr allocate_protocol_message();

	const ProtocolMessage_uptr ordered_recv();

	int reliable_send(const ProtocolMessage_uptr message);

	const std::map<Protocol, size_t> &provided_protocols() const;
};

}

#endif
