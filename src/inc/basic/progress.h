#ifndef __EXAMPI_BASIC_PROGRESS_H
#define __EXAMPI_BASIC_PROGRESS_H

#include <basic.h>
#include <map>
#include <unordered_map>
#include <list>
#include <mutex>
#include <set>
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

    Header() { std::memset(hdr, 0xD0, HeaderSize); }

    // TODO:  Don't forget these debugs, this can create a lot of spam
    void dump()
    {
      std::ios oldState(nullptr);
      oldState.copyfmt(std::cout);

      uint32_t *dword = (uint32_t *)hdr;
      std::cout << "\texampi::basic::Header has:\n";
      std::cout << std::setbase(16) << std::internal << std::setfill('0')
        << "\t"
        << std::setw(8) << dword[0] << " "
        << std::setw(8) << dword[1] << " "
        << std::setw(8) << dword[2] << " "
        << std::setw(8) << dword[3] << " "
        << "\n\t"
        << std::setw(8) << dword[4] << " "
        << std::setw(8) << dword[5] << " "
        << std::setw(8) << dword[6] << " "
        << std::setw(8) << dword[7] << " "
        << "\n";

      std::cout.copyfmt(oldState);
    }

    void pack()
    {
      uint16_t *word = (uint16_t *)hdr;
      uint32_t *dword = (uint32_t *)hdr;
      word[0] = 0xDEAF; // magic word
      word[1] = 22;  // protocol
      word[2] = 42;  // message type
      word[3] = 0x0; // align
      dword[2] = 0x0;  // align
      dword[3] = 0x0;  // align/reserved
      dword[4] = rank;
      dword[5] = tag;
      dword[6] = context;
      dword[7] = 0xAABBCCDD;  // CRC
      std::cout << "\tpack:\n";
      dump();

    }

    void unpack()
    {
      uint16_t *word = (uint16_t *)hdr;
      uint32_t *dword = (uint32_t *)hdr;
      rank = dword[4];
      tag = dword[5];
      context = dword[6];
      std::cout << "\tunpack:\n";
      dump();
      std::cout << "\tUnderstood rank as " << rank << "\n";
    }

    struct iovec getIovec()
    {
      pack();
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
#if 0
    void rankZeroBarrierRecv()
    {
      int reported = 1;
      std::set<int> awake;
      awake.insert(0);
      Request req;
      while(reported < exampi::global::worldSize)
      {
        exampi::global::transport->receive(req.getIovecs(), 0);
        req.hdr.unpack();
        awake.insert(req.hdr.rank);
        reported = awake.count();
      }
    }

    void rankZeroBarrierSend()
    {
      Request req;
      
    }

    void sendWork()
    {
      
    }
#endif

  public:
    Progress() {;}
        
    virtual int init() { addEndpoints(); }

    virtual void barrier()
    {
    }

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
      req.hdr.unpack();
      // TODO:  Respect MPI_STATUS_IGNORE
      if(status)
      {
        std::cout << "\tstatus present, writing\n";
        status->MPI_SOURCE = req.hdr.rank;
        status->MPI_TAG = req.hdr.tag;
      }
      return 0;
    }

};

} // basic
} // exampi

#endif // header guard
