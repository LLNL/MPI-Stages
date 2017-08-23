#ifndef __EXAMPI_BASIC_PROGRESS_H
#define __EXAMPI_BASIC_PROGRESS_H

#include <basic.h>
#include <map>
#include <unordered_map>
#include <list>
#include "basic/transport.h"

namespace exampi {
namespace basic {

class Message
{
  public:
    static constexpr size_t HeaderSize = (8 * 4);
  private:
    int rank;
    uint32_t tag;
    int context;
    MPI_Comm comm;
    std::vector<struct iovec> iov;
    char hdr[HeaderSize];
    
    struct iovec iovHdr()
    {
      struct iovec iov;
      iov.iov_base = hdr;
      iov.iov_len = HeaderSize;
      return iov;
    }

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

  public: 
    Message() : iov()
    {
      iov.push_back(iovHdr());
    }

    Message(struct iovec v) { Message();  iov.push_back(v); }
    Message(exampi::i::Buf *buf) { Message();  iov.push_back(buf->iov());}




};

class Progress : public exampi::i::Progress
{
  private:
    //std::list sends;
    //std::list recvs;

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

      std::vector<struct iovec> iov;
      struct iovec v = {buf, count};
      iov.push_back(v);
      exampi::global::transport->send(iov, dest, comm );
      return 0;
    }
    virtual int recv_data(void *buf, size_t count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
      std::vector<struct iovec> iov;
      struct iovec v = {buf, count};
      iov.push_back(v);
      exampi::global::transport->receive(iov, comm);
      return 0;
    }

};

} // basic
} // exampi

#endif // header guard
