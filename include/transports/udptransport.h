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

class UDPTransport: public Transport
{
private:
	int socket_recv;

	std::mutex guard;

	msghdr hdr;

	std::unordered_map<long int, sockaddr_in> cache;	

public:
	UDPTransport();
	~UDPTransport();

	ProtocolMessage_uptr peek();
	int reliable_send(ProtocolMessage_uptr message);
};

}

#endif
