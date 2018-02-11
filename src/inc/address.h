#ifndef __EXAMPI_ADDRESS_H
#define __EXAMPI_ADDRESS_H

#include <sys/types.h>
#include <sys/socket.h>

namespace exampi {

class Address
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

} // ::exampi

#endif
