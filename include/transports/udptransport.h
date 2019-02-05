#ifndef __EXAMPI_UDPTRANSPORT_H
#define __EXAMPI_UDPTRANSPORT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>

#include "abstract/transport.h"
#include "universe.h"
#include "protocol.h"

namespace exampi
{

class UDPTransport: public Transport
{
private:
	int socket_recv;

	std::mutex guard;

	msghdr hdr;

public:
	UDPTransport();
	~UDPTransport();

	bool peek(ProtocolMessage_uptr &message);
	int reliable_send(ProtocolMessage_uptr message);
};

}

#endif
