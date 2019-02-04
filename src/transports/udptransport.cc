#include "transports/udptransport.h"

namespace exampi
{

UDPTransport::UDPTransport() : Transport()
{
	// create
	socket_recv = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_recv == 0)
	{
		// TODO handle error
	}

	// setsockopt to reuse address
	int opt = 1;
	if (setsockopt(server_recv, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 

	// bind
	Universe& universe = Universe::get_root_universe();

	int port = std::stoi(std::string(std::getenv("EXAMPI_UDP_TRANSPORT_BASE"))) + universe.rank;
	
	struct sockaddr_in address_local;
	address_local.sin_family = AF_INET;
	address_local.sin_addr.s_addr = INADDR_ANY;
	address_local.sin_port = htons(port);

	if(bind(server_recv, address_local, sizeof(address_local)) < 0)
	{
		// TODO handle error
	}
}

UDPTransport::~UDPTransport()
{
	close(socket_recv);
}

ProtocolMessage_uptr UDPTransport::absorb()
{
	struct msghdr hdr;
	hdr.msg_name = NULL;
	hdr.msg_iov 
	hdr.msg_controllen = 0;
	
	// recv message via udp
	ssize_t err = recvmsg(socket_recv, &hdr, MSG_DONTWAIT);

	if(err <= 0)
		// nothing received
		return ProtocolMessage_uptr(nullptr);

	//  
	ProtocolMessage_uptr ptr = allocate_protocol_message();
}

int UDPTransport::reliable_send(ProtocolMessage_uptr message)
{
	// output message via udp
	// TODO
	ssize_t err = sendmsg(socket_recv, );
}

}
