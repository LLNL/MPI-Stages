#include <ExaMPI.h>
#include <iostream>
#include <string>
#include "UDPSocket.h"

namespace exampi
{

class BasicTransport : public ITransport
{
  private:
    string address;
    uint16_t port;
    std::vector<std::string> hosts;
  public:
    BasicTransport() {};
        
    virtual void send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
      //TODO: figure out destination ip and port from dest
      address = hosts[r].cstr();
      port = 8080;
      try {
        UDPSocket sock;
        sock.send(buf, count, address, port);
      } catch (std::exception &ex) {
          std::cerr << ex.what() << endl;
          exit(1);  
      }
    }
    
    virtual int recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
      //TODO: figure out port from source
      port = 8080;
      try {
         UDPSocket sock(port);
         string sourceAddress;
         uint16_t sourcePort;
         int recvDataSize;
         for (;;) {
            recvDataSize = sock.recv(buf, count, sourceAddress, sourcePort);
            std::cout << "Received packet from " << sourceAddress << " : " << sourcePort << " : " << buf << endl;
         }
      } catch (std::exception &ex) {
          std::cerr << ex.what() <<endl;
          exit(1);
      }
      return 0;
    }

    void SetHosts(std::vector<std::string> h)
    {
      hosts = h;
    }
};

}
