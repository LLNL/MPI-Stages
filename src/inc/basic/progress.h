#ifndef __EXAMPI_BASIC_PROGRESS_H
#define __EXAMPI_BASIC_PROGRESS_H

#include <basic.h>
#include <map>
#include <unordered_map>
#include <list>
#include <mutex>
#include "basic/transport.h"

namespace exampi {
namespace basic {

// POD types
class Header
{
  public:
    static constexpr size_t HeaderSize = (8 * 4);

    int rank;
    uint32_t tag;
    int context;
    MPI_Comm comm;
    char hdr[HeaderSize];

    void buildHeader()
    {
      uint16_t *word = (uint16_t *)hdr;
      uint32_t *dword = (uint32_t *)hdr;
      word[0] = 0xDEAF; // magic word
      word[1] = 22;  // protocol
      word[2] = 42;  // message type
      word[3] = 0x0; // align
      dword[2] = 0x0;  // align
      dword[3] = 0x0;  // align/reserved
      dword[4] = 0x0;  // source tank
      dword[5] = tag;
      dword[6] = context;
      dword[7] = 0xABBA;  // CRC
    }

    struct iovec getIovec()
    {
      buildHeader();
      struct iovec iov = {hdr, HeaderSize};
      return iov;
    }
};

// demoting this from exampi.h for now --sf
class UserArray
{
  public:
    void *buf;
    Datatype *datatype;
    int count;
    struct iovec getIovec() { struct iovec iov = {buf, datatype->getExtent() * count}; return iov; }
};

class Request
{
  public:
    int dest;
    MPI_Comm comm;
    Header hdr;
    UserArray array;
    std::vector<struct iovec> getIovecs()
    {
      std::vector<struct iovec> result;
      result.push_back(hdr.getIovec());
      result.push_back(array.getIovec());
      return result;
    }
};

template<typename T>
class FutureQueue
{
  
};

class Progress : public exampi::i::Progress
{
  private:
    std::list<Request> sendsPending;
    std::list<Request> sendsComplete;
    std::list<Request> recvsPending;
    std::list<Request> recvsComplete;
    std::mutex sendLock;
    std::mutex recvLock;


    void addEndpoints()
    {
      int size = std::stoi((*exampi::global::config)["size"]);
      std::vector<std::string> elem;
      for(int i = 0; i < size; i++)
      {
        elem.clear();
        std::string rank = std::to_string(i);
        elem.push_back((*exampi::global::config)[rank]);
        elem.push_back("8080");
        exampi::global::transport->addEndpoint(i, elem);
      }
    }

    void sendWork()
    {
      
    }

  public:
    Progress() {;}
        
    virtual int init() { addEndpoints(); }

    // TODO:  We have a small issue here.  We need iovecs of this data ultimately, but iovecs don't take
    // const pointers.  We can cast away, but killing the guarantee for now.
    virtual int send_data(void* buf, size_t count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {

      Request req;
      req.dest = dest;
      req.comm = comm;
      req.hdr.tag = tag;
      req.hdr.rank = exampi::global::rank;
      req.array.buf = buf;
      req.array.datatype = &exampi::global::datatypes[datatype];
      req.array.count = count;
      exampi::global::transport->send(req.getIovecs(), req.dest, comm );
      return 0;
    }
    virtual int recv_data(void *buf, size_t count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {

      Request req;
      req.comm = comm;
      req.dest = -1;
      req.array.buf = buf;
      req.array.datatype = &exampi::global::datatypes[datatype];
      req.array.count = count;
      exampi::global::transport->receive(req.getIovecs(), comm);
      return 0;
    }

};

} // basic
} // exampi

#endif // header guard
