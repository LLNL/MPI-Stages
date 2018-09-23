#ifndef __EXAMPI_BASIC_UDP_H
#define __EXAMPI_BASIC_UDP_H

#include <basic.h>
#include <address.h>

namespace exampi {
namespace basic {
namespace udp {

class Socket
{
private:
	int fd;
public:
	Socket() { fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); }
	~Socket() { close(fd); }
	void bindPort(uint16_t port)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;
		if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
			std::cout << "WARNING:  Bind failed\n";
	}
	int getFd() { return fd; }
	void destroy() {
		close(fd);
	}
};

// WARNING:  This class creates a THIRD copy of the std::vector
// Need to write a custom container and do move semantics
class Message
{
private:
	struct msghdr hdr;
	std::vector<struct iovec> iov;
public:
	Message() : iov()
{
		hdr.msg_control = NULL;
		hdr.msg_controllen = 0;
}

	Message(std::vector<struct iovec> i) : iov(i)
	{
		hdr.msg_control = NULL;
		hdr.msg_controllen = 0;
		hdr.msg_name = NULL;
		hdr.msg_namelen = 0;
	}

	void addBuf(exampi::i::Buf *b) { iov.push_back(b->iov()); }
	void updateHeader()
	{
		std::cout << "length of message" << iov[1].iov_len << " header "<< iov[0].iov_len << "\n";
		hdr.msg_iov = iov.data();
		hdr.msg_iovlen = iov.size();
	}
	void updateHeader(Address &addr)
	{
		updateHeader();
		hdr.msg_name = addr.get();
		hdr.msg_namelen = addr.size();
	}


	void send(Socket &sock, Address &addr)
	{
		updateHeader(addr);

		// debug output
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(addr.get()->sin_addr), str, INET_ADDRSTRLEN);
		std::cout << "\tbasic::Transport::udp::send\n"
				<< "\t\t" << hdr.msg_iovlen << " iovecs\n"
				<< "\t\t" << str << "\n";

		ssize_t length = sendmsg(sock.getFd(), &hdr, 0);
		std::cout << "Send to UDP " << length << "\n";
	}
	ssize_t receive(Socket &sock)
	{
		std::cout << debug() << "basic::Transport::udp::recv\n";
		updateHeader();
		ssize_t length = recvmsg(sock.getFd(), &hdr, MSG_WAITALL);
		std::cout << "Received from UDP " << length << "\n";
		std::cout << debug() << "basic::Transport::udp::recv exiting\n";
		return length;
	}
	void peek(Socket &sock)
	{
		updateHeader();
		recvmsg(sock.getFd(), &hdr, MSG_WAITALL | MSG_PEEK);
	}
};

} // udp
} // basic
} // exampi

#endif // guard
