#ifndef __EXAMPI_BASIC_TCP_H
#define __EXAMPI_BASIC_TCP_H

#include <basic.h>
#include <address.h>
#include <netinet/tcp.h>

namespace exampi {
namespace basic {
namespace tcp {

class Socket
{
  private:
    int fd;
  public:
    Socket() {
    	fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    	int nagle = 1;
    	setsockopt(fd, IPPROTP_TCP, TCP_NODELAY, (cahr *)&nagle, sizeof(int));
    }
    ~Socket() { close(fd); }
    void bindPort(uint16_t port)
    {
      sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port);
      addr.sin_addr.s_addr = INADDR_ANY;

      if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        std::cout << "WARNING:  Bind failed\n";

      if (listen(fd, SOMAXCONN) == -1)
    	  std::cout << "WARNING: Listen failed\n";

    }
    int getFd() { return fd; }
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
    }

    void addBuf(exampi::i::Buf *b) { iov.push_back(b->iov()); }
    void updateHeader()
    {
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
      if (connect (sock.getFd(), (struct sockaddr *) (addr.get()), sizeof(addr)) < 0)
    	  std::cout << "WARNING: Connect failed\n";
      std::cout << "\tbasic::Transport::tcp::send\n"
                << "\t\t" << hdr.msg_iovlen << " iovecs\n"
                << "\t\t" << str << "\n";
      sendmsg(sock.getFd(), &hdr, 0);
    }
    void receive(Socket &sock, int tcpSock)
    {
      std::cout << debug() << "basic::Transport::tcp::recv\n";
      updateHeader();
      recvmsg(tcpSock, &hdr, MSG_WAITALL);
      std::cout << debug() << "basic::Transport::tcp::recv exiting\n";
    }
    void peek(Socket &sock, int &tcpSock)
    {
    	updateHeader();
    	sockaddr_in client;
    	socklen_t clientlen;
    	clientlen = sizeof(client);
    	if ((tcpSock = accept(sock.getFd(), (struct sockaddr *) &client, &clientlen)) < 0)
    	    std::cout << "WARNING: Accept failed\n";

      recvmsg(tcpSock, &hdr, MSG_WAITALL | MSG_PEEK);
    }
};
}//tcp
}//basic
}//exampi
#endif
