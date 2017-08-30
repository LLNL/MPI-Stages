#ifndef __EXAMPI_BASIC_PROGRESS_H
#define __EXAMPI_BASIC_PROGRESS_H

#include <basic.h>
#include <map>
#include <unordered_map>
#include <list>
#include <mutex>
#include <set>
#include <memory>
#include <algorithm>
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

class Request
{
  public:
    static constexpr size_t HeaderSize = (8 * 4);
  protected:
    char hdr[HeaderSize];
  public:
    Op op;
    int tag;
    int source;
    MPI_Comm comm;
    UserArray array;
    Endpoint endpoint;
    MPI_Status status; // maybe not needed --sf
    std::promise<MPI_Status> completionPromise;

    void pack()
    {
      uint16_t *word = (uint16_t *)hdr;
      uint32_t *dword = (uint32_t *)hdr;
      word[0] = 0xDEAF; // magic word
      word[1] = 22;  // protocol
      word[2] = 42;  // message type
      word[3] = 0x0; // function
      dword[2] = 0x0;  // align
      dword[3] = 0x0;  // align/reserved
      dword[4] = source;
      dword[5] = tag;
      dword[6] = 0x00C0FFEE; // context; not yet
      dword[7] = 0xAABBCCDD;  // CRC
    }

    void unpack()
    {
      uint16_t *word = (uint16_t *)hdr;
      uint32_t *dword = (uint32_t *)hdr;
      source = dword[4];
      tag = dword[5];
      //context = dword[6];
    }

    struct iovec getHeaderIovec() { return {hdr, HeaderSize};}

    std::vector<struct iovec> getHeaderIovecs()
    {
      std::vector<struct iovec> iov;
      iov.push_back(getHeaderIovec());
      return iov;
    }

    std::vector<struct iovec> getArrayIovecs()
    {
      std::vector<struct iovec> iov;
      iov.push_back(array.getIovec());
      return iov;
    }

    std::vector<struct iovec> getIovecs()
    {
      std::vector<struct iovec> iov;
      iov.push_back(getHeaderIovec());
      iov.push_back(array.getIovec());
      return iov;
    }

};
#if 0
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
#endif

class Progress : public exampi::i::Progress
{
  private:
    AsyncQueue<Request> outbox;
    std::list<std::unique_ptr<Request>> matchList;
    std::mutex matchLock;
    std::thread sendThread;
    std::thread matchThread;
    bool alive;


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

    static void sendThreadProc(bool *alive, AsyncQueue<Request> *outbox)
    {
      std::cout << debug() << "Launching sendThreadProc(...)\n";
      while(*alive)
      {
        std::unique_ptr<Request> r = outbox->promise().get();
        std::cout << debug() << "sendThread:  got result from outbox future\n";
        exampi::global::transport->send(r->getIovecs(), r->endpoint.rank, 0);
        // TODO:  check that sending actually completed
        r->completionPromise.set_value({
            .count = 0,
            .cancelled = 0,
            .MPI_SOURCE = r->source,
            .MPI_TAG = r->tag,
            .MPI_ERROR = MPI_SUCCESS
            });
        // let r drop scope and die (unique_ptr) 
      }
    }
#if 0
    static void recvThreadProc(bool *alive, AsyncQueue<Request> *inbox)
    {
      std::cout << debug() << "Launching recvThreadProc(...)\n";
      while(*alive)
      {
        std::unique_ptr<Request> r = make_unique<Request>();
        std::cout << debug() << "recvThread:  made request, about to receive...\n";
        exampi::global::transport->receive(r->getHeaderIovecs(), 0);
        std::cout << debug() << "recvThread:  received\n";
        inbox->put(std::move(r));
      }
    }
#endif
    static void matchThreadProc(bool *alive, std::list<std::unique_ptr<Request>> *matchList, std::mutex *matchLock)
    {
      std::cout << debug() << "Launching matchThreadProc(...)\n";
      while(*alive)
      {
        std::unique_ptr<Request> r = make_unique<Request>();
        std::cout << debug() << "matchThread:  made request, about to peek...\n";
        exampi::global::transport->peek(r->getHeaderIovecs(), 0);
        std::cout << debug() << "matchThread:  received\n";
        matchLock->lock();
        int t = r->tag;
        auto result = std::find_if(matchList->begin(), matchList->end(),
            [t](const std::unique_ptr<Request> &i) -> bool { return i->tag == t; });
        if(result == matchList->end())
        {
          std::cout << "WARNING:  Failed to match incoming msg\n";
        }
        else
        {
          std::cout << debug() << "matchThread:  matched, about to receive remainder\n";
          std::cout << debug() << "\tTarget array is " << (*result)->array.toString() << "\n";
          std::cout << debug() << "\tDatatype says extent is " << (*result)->array.datatype->getExtent() << "\n";
          exampi::global::transport->receive((*result)->getIovecs(), 0); 
          (*result)->unpack();
          (*result)->completionPromise.set_value({
              .count = 0,
              .cancelled = 0,
              .MPI_SOURCE = (*result)->source,
              .MPI_TAG = (*result)->tag,
              .MPI_ERROR = MPI_SUCCESS
              });
          matchList->erase(result);
        }
        matchLock->unlock();
      }
    }
  public:
    Progress() {;}
        
    virtual int init() 
    { 
      addEndpoints(); 
      alive = true;
      sendThread = std::thread{sendThreadProc, &alive, &outbox};
      //recvThread = std::thread{recvThreadProc, &alive, &inbox};
      matchThread = std::thread{matchThreadProc, &alive, &matchList, &matchLock};
    }

    virtual int stop()
    {
      alive = false;
    }


    virtual void barrier()
    {
    }

    // TODO:  We have a small issue here.  We need iovecs of this data ultimately, but iovecs don't take
    // const pointers.  We can cast away, but killing the guarantee for now.
    virtual int send_data(void* buf, size_t count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
      std::cout << debug() << "\tbasic::Interface::send(...)\n";

      Request req;
      req.endpoint.rank = dest;
      req.comm = comm;
      req.tag = tag;
      req.endpoint.rank = exampi::global::rank;
      req.array.ptr = buf;
      req.array.datatype = &exampi::global::datatypes[datatype];
      req.array.count = count;
      exampi::global::transport->send(req.getIovecs(), req.endpoint.rank, comm );
      return 0;
    }
    virtual int recv_data(void *buf, size_t count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
      std::cout << debug() << "\tbasic::Interface::recv(...)\n";

      Request req;
      req.comm = comm;
      req.endpoint.rank = -1;
      req.array.ptr = buf;
      req.array.datatype = &exampi::global::datatypes[datatype];
      req.array.count = count;
      exampi::global::transport->receive(req.getIovecs(), comm);
      req.unpack();
      // TODO:  Respect MPI_STATUS_IGNORE
      if(status)
      {
        std::cout << "\tstatus present, writing\n";
        status->MPI_SOURCE = req.source;
        status->MPI_TAG = req.tag;
      }
      return 0;
    }

    virtual std::future<MPI_Status> postSend(UserArray array, Endpoint dest, int tag)
    {
      std::cout << debug() << "\tbasic::Interface::postSend(...)\n";
      std::unique_ptr<Request> r = make_unique<Request>();
      r->op = Op::Send;
      r->source = exampi::global::rank;
      r->array = array;
      r->endpoint = dest;
      r->tag = tag;
      auto result = r->completionPromise.get_future();
      outbox.put(std::move(r));
      return result;
    }

    virtual std::future<MPI_Status> postRecv(UserArray array, int tag)
    {
      std::cout << debug() << "\tbasic::Interface::postRecv(...)\n";
      std::unique_ptr<Request> r = make_unique<Request>();
      r->op = Op::Receive;
      r->array = array;
      r->endpoint.invalid();
      r->tag = tag;
      auto result = r->completionPromise.get_future();
      matchList.push_back(std::move(r));
      return result;
    }

};

} // basic
} // exampi

#endif // header guard
