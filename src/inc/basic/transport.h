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
    udp::Socket recvSocket;
  public:
    Transport() : endpoints(), recvSocket() {;};

    virtual void init()
    {
      recvSocket.bindPort(8080);
    }

    virtual size_t addEndpoint(const int rank, const std::vector<std::string> &opts)
    {
      uint16_t port = std::stoi(opts[1]);
      // TODO:  see basic/udp.h; need move constructor to avoid copy here
      udp::Address addr(opts[0], port);

      std::cout << "\tAssigning " << rank << " to " << opts[0] << ":" << port << "\n";
      endpoints[rank] = addr;
    }

    virtual std::future<int> send(std::vector<struct iovec> iov, int dest, MPI_Comm comm)
    {
      std::cout << "\tbasic::Transport::send(..., " << dest <<", " << comm << ")\n";
      udp::Socket s;
      udp::Message msg(iov);

      msg.send(s, endpoints[dest]);
      return std::promise<int>().get_future();
    }

    virtual std::future<int> receive(std::vector<struct iovec> iov, MPI_Comm comm)
    {
      std::cout << debug() << "basic::Transport::receive(...)" << std::endl;
      std::cout << debug() << "\tiov says size is " << iov.size() << std::endl;
      std::cout << debug() << "\t ------ " << std::endl;
      udp::Message msg(iov);

      std::cout << debug() << "basic::Transport::receive, constructed msg, calling msg.receive" << std::endl;
      msg.receive(recvSocket);
      std::cout << debug() << "basic::Transport::receive returning" << std::endl;
      return std::promise<int>().get_future();
    }

    virtual int peek(std::vector<struct iovec> iov, MPI_Comm comm)
    {
      udp::Message msg(iov);
      msg.peek(recvSocket);
      return 0;
    }
        

};

} // basic
} // exampi
#endif //guard
