#ifndef __EXAMPI_BASIC_UDP_H
#define __EXAMPI_BASIC_UDP_H

#include <basic.h>
#include <sys/types.h>
#include <sys/socket.h>

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
};

class Address : public exampi::i::Address
{
  private:
    sockaddr_in addr;
  public:
    void set(const std::string &ip, const uint16_t &port)
    {
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port);
      addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }
    Address() { memset(&addr, 0, sizeof(addr));}
    Address(std::string ip, uint16_t port)
    {
      Address();
      set(ip, port);
    }
    Address(const Address &a)
    {
      memcpy(&addr, &a.addr, sizeof(addr));
    }
    // TODO:  Move constructor

    sockaddr_in *get() { return &addr; }
    size_t size() { return sizeof(addr); }
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
      std::cout << "\tbasic::Transport::udp::send\n"
                << "\t\t" << hdr.msg_iovlen << " iovecs\n"
                << "\t\t" << str << "\n";
      sendmsg(sock.getFd(), &hdr, 0); 
    }
    void receive(Socket &sock) 
    { 
      updateHeader(); 
      recvmsg(sock.getFd(), &hdr, 0); 
    }
};

} // udp
} // basic
} // exampi

#endif // guard
