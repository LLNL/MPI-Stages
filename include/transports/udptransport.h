#ifndef __EXAMPI_UDPTRANSPORT_H
#define __EXAMPI_UDPTRANSPORT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "abstract/transport.h"
#include "universe.h"
#include "protocol.h"

namespace exampi
{

class UDPTransport: public Transport
{
private:
	int socket_recv;

public:
	UDPTransport();
	~UDPTransport();

	ProtocolMessage_uptr absorb();
	int reliable_send(ProtocolMessage_uptr message);
};

}

#endif
