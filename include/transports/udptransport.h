// 
// This is a source code component of ExaMPI, a next-generation 
// implementation of the Message Passing Interface.
//
// This is a joint project of University of Tennessee at Chattanooga,
// the Lawrence Livermore National Laboratory, and Auburn University.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE', which is part of this source code package.
// 

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

struct UDPTransportPayload
{
	int payload_size;
	char payload[65507 - sizeof(Header)];
};

class UDPTransport: public Transport, virtual public Stages
{
private:
	std::mutex guard;

	int socket_recv;

	MemoryPool<Header> header_pool;
	MemoryPool<UDPTransportPayload> payload_pool;

	std::unordered_map<Header *, UDPTransportPayload *> payload_buffer;

	msghdr hdr;

	std::unordered_map<long int, sockaddr_in> cache;

	std::map<Protocol, size_t> available_protocols;

	void cache_endpoints();

public:
	UDPTransport();
	~UDPTransport();

	const std::map<Protocol, size_t> &provided_protocols() const;

	Header_uptr ordered_recv();
	void fill(Header_uptr, Request *);
	void reliable_send(const Protocol, const Request *);

	int save(std::ostream &);
	int load(std::istream &);
	int cleanup();
	int halt();
};

}

#endif
