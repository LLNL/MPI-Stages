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
#include <sigHandler.h>
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
          //r->array = {const_cast<void *>(buf), &(exampi::global::datatypes[datatype]), szcount};
          //exampi::global::transport->receive((*r)->getIovecs(), 0);
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
      return 0;
    }

    virtual int init(std::istream &t)
    {
      // nothing to restore (currently only valid if no pending comm.)
      init();
      return 0;
    }

    virtual int stop()
    {
      alive = false;
      return 0;
    }


    virtual void barrier()
    {
    	std::stringstream filename;
    	filename << "pid." << exampi::global::rank << ".txt";
    	std::ofstream t(filename.str());
    	t << ::getpid();
    	t.close();

    	sigHandler signal;
    	signal.setSignalToHandle(SIGUSR1);
    	int parent_pid = std::stoi((*exampi::global::config)["ppid"]);
    	kill(parent_pid, SIGUSR1);

    	while(signal.isSignalSet() != 1) {
    		sleep(1);
    	}
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

    virtual int save(std::ostream &t)
    {
      // Assuming no pending comm, nothing to do
      return MPI_SUCCESS;
    }

    virtual int load(std::istream &t)
    {
      std::cout << "In progress load";
      alive = true;
      sendThread = std::thread{sendThreadProc, &alive, &outbox};
      //recvThread = std::thread{recvThreadProc, &alive, &inbox};
      matchThread = std::thread{matchThreadProc, &alive, &matchList, &matchLock};
      return MPI_SUCCESS;
    }


};

} // basic
} // exampi

#endif // header guard
