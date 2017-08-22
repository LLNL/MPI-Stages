#ifndef __EXAMPI_BASIC_TRANSPORT_H
#define __EXAMPI_BASIC_TRANSPORT_H

#include <basic.h>
#include "basic/udp.h"

namespace exampi {
namespace basic {

class Transport : public exampi::i::Transport
{
  private:
    std::string address;
    std::unordered_map<int,udp::Address> endpoints;
    uint16_t port;
  public:
    Transport() : endpoints() {};

    virtual size_t addEndpoint(const int rank, const std::vector<std::string> &opts)
    {
      uint16_t port = std::stoi(opts[1]);
      // TODO:  see basic/udp.h; need move constructor to avoid copy here
      udp::Address addr(opts[0], port);

      std::cout << "\tAssigning " << rank << " to " << opts[0] << ":" << port << "\n";
      endpoints[rank] = addr;
    }

    virtual void send(exampi::i::Buf *buf, int dest, MPI_Comm comm)
    {
      std::cout << "\tbasic::Transport::send(...)\n";
      udp::Socket s;
      udp::Message msg;

      msg.addBuf(buf);
      msg.send(s, endpoints[dest]);
    }

    virtual void receive(exampi::i::Buf *buf, MPI_Comm comm)
    {
      std::cout << "\tbasic::Transport::receive(...)\n";
      udp::Socket s;
      udp::Message msg;

      s.bindPort(8080);
      msg.addBuf(buf);
      msg.receive(s);
    }
        

};

} // basic
} // exampi
#endif //guard
