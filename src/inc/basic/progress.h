#ifndef __EXAMPI_BASIC_PROGRESS_H
#define __EXAMPI_BASIC_PROGRESS_H

#include <basic.h>
#include <map>
#include <unordered_map>
#include "basic/transport.h"

namespace exampi {
namespace basic {

class Progress : public exampi::i::Progress
{
  private:
    Config *config;
    Transport btransport;

    void addEndpoints()
    {
      std::cout << "In addEndpoints; size is " << (*config)["size"] << "\n";
      int size = std::stoi((*config)["size"]);
      std::vector<std::string> elem;
      for(int i = 0; i < size; i++)
      {
        elem.clear();
        std::string rank = std::to_string(i);
        elem.push_back((*config)[rank]);
        elem.push_back("8080");
        btransport.addEndpoint(i, elem);
      }
    }

  public:
    Progress(Config *c) : config(c), btransport(config) {;}
        
    virtual int init() { addEndpoints(); }

    virtual int send_data(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {

      Buf b((void *)buf, count);
      btransport.send(&b, dest, comm );
      //btransport.send(buf, count, datatype, dest, tag, comm);
      return 0;
    }
    virtual int recv_data(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
      Buf b(buf, count);
      btransport.receive(&b, comm);
      //btransport.recv(buf, count, datatype, source, tag, comm, status);
      return 0;
    }

};

} // basic
} // exampi

#endif // header guard
