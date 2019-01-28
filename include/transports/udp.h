#ifndef __EXAMPI_BASIC_UDP_H
#define __EXAMPI_BASIC_UDP_H

#include <basic.h>
#include <address.h>

#include <cstdlib>

namespace exampi
{

class Socket
{
private:
	int fd;
public:
	Socket();
	~Socket();
	void bindPort(uint16_t port);
	int getFd();
	void destroy();
};

// WARNING:  This class creates a THIRD copy of the std::vector
// Need to write a custom container and do move semantics
class Message
{
private:
	struct msghdr hdr;

	// TODO REMOVE THIS
	std::vector<struct iovec> &iov;

public:
	Message(std::vector<struct iovec> &i);

	void addBuf(exampi::Buf *b);
	void updateHeader();
	void updateHeader(Address &addr);
	void send(Socket &sock, Address &addr);
	ssize_t receive(Socket &sock);
	void peek(Socket &sock);
};

} // exampi

#endif // guard
