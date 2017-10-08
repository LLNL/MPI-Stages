#ifndef EXAMPI_EXAMPI_H
#define EXAMPI_EXAMPI_H

/* Internal include for ExaMPI
 */

#include <vector>
#include <string>
#include <list>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <sstream>

#include <mpi.h>
#include <datatypes.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <future>

#include <config.h>
#include <datatype.h>

#include <array.h>
#include <i/interface.h>
#include <i/progress.h>
#include <i/transport.h>
#include <i/checkpoint.h>

namespace exampi {

// unique_ptr is c++11, but make_unique is c++14 headexplode.gif --sf
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

static inline std::thread::id thisThread() { return std::this_thread::get_id(); }

static inline std::string debug()
{
  std::stringstream stream;
  stream << "\t[0x" << std::hex << std::setfill('0') << std::setw(8) << thisThread() << "] ";
  return stream.str();
}

static inline std::string mpiStatusString(MPI_Status st)
{
  std::stringstream stream;
  stream << "MPI_Status{MPI_SOURCE=" << st.MPI_SOURCE
         << ", MPI_TAG = " << st.MPI_TAG 
         << ", MPI_ERROR = " << st.MPI_ERROR << "}";
  return stream.str();
}

enum class Op : int
{
  Send,
  Receive
};

// TODO:  should really wrap this into a class --sf
static inline void iovMove(std::vector<struct iovec> toVec, std::vector<struct iovec> fromVec)
{
  auto toPos = toVec.begin();
  auto fromPos = fromVec.begin();
  struct iovec to = *toPos;
  struct iovec from = *fromPos;
  bool done = false;
  while(!done)
  {
    size_t sz = std::min(to.iov_len, from.iov_len);
    memmove(to.iov_base, from.iov_base, sz);
    to.iov_len -= sz;
    from.iov_len -= sz;
    if(to.iov_len <= 0) to = *(++toPos);
    if(from.iov_len <= 0) from = *(++fromPos);
    if((toPos == toVec.end()) || (fromPos == fromVec.end())) done = true;
  }
  
}

template<typename T>
class AsyncQueue
{
  typedef std::unique_ptr<T> DataType;
  typedef std::promise<DataType> PromiseType;
  typedef std::unique_ptr<PromiseType> PromisePtrType;
  typedef std::list<DataType> ListType;
  typedef std::list<PromisePtrType> PromiseListType;
  protected:
    ListType data;
    PromiseListType promises;
    std::mutex putLock;
    std::mutex getLock;

    void test()
    {
      std::cout << debug() << "\tAQ:  testing\n";
      if(promises.size() > 0)
        if(data.size() > 0)
        {
          std::cout << debug() << "\tAQ:  data and promises, advancing\n";
          promises.front()->set_value(std::move(data.front()));
          promises.pop_front();
          data.pop_front();
        }
      std::cout << debug() << "\tAQ:  done testing\n";
    }
  public:
    AsyncQueue()
    {
      std::cout << debug() << "\tAsyncQueue:  constructing\n"; 
    }

    std::future<DataType> promise()
    { 
      std::cout << debug() << "\tAQ: Promise requested.  data(" << data.size() << ") promises(" << promises.size() << ")\n";
      promises.push_back(make_unique<PromiseType>());
      std::cout << debug() << "\tAQ: Promise pushed; about to get_future...\n";
      auto result = promises.back()->get_future();
      test();
      return result;
    }

    void put(DataType&& v)
    {
      std::cout << debug() << "\tAQ:  Putting\n";
      data.push_back(std::move(v));
      test();
    }
};


#if 0
namespace relay {
// policies
// disabled for now
namespace policy
{
struct SignalNext
{
  protected:
  void signalQueue() 
  { 
    q.front()->signal(); 
    q.pop_front();
  }
};

struct SignalAll
{
  protected:
  void signalQueue()
  {
    for(sinkPtr &s : q)
    {
      s->signal();
    }
  }
};

struct SignalAllOnce
{
  protected:
  void signalQueue()
  {
    for(sinkPtr &s : q)
    {
      s->signal();
    }
    q.clear();
  }
};
} // policy
template<typename SignalPolicy>
class Relay : private SignalPolicy
{
  protected:
    std::mutex mutex;
    std::condition_variable cv;
    std::deque<Relay *> q;
    int signals;
    int incomingConnections;
    void notify()
    {
      incomingConnections++;
    }
  public:
    Relay() : signals(0) {;}
    void add(Relay *r)
    {
      q.push_back(r);
    }
    void signal()
    {
      std::unique_lock<std::mutex> lock(mutex);
      signals++;
      lock.unlock();
      testForCompletion();
    }
    void wait()
    {
      std::unique_lock<std::mutex> lock(mutex);
      cv.wait(lock, []{return needed < 1;});
    }
};

}

class Promise
{
};
#endif
namespace i {
class Fault
{
  public:
    virtual void DoSomething() = 0;
};

class Memory
{
  public:
    virtual void DoSomething() = 0;
};

// send/recv buffer that knows how to describe itself as an iovec
class Buf
{
  public:
    virtual struct iovec iov() = 0;
};

// as above, but as a vector
class BufV
{
  public:
    virtual struct iovec *AsIovecV() = 0;
};

class Address
{
  public:
    virtual size_t size() = 0;
};

} // i

class Tag
{
  public:
    uint32_t bits;
    bool operator==(Tag &b) { return bits == b.bits; }
    bool test(Tag &t) { return bits == t.bits; }
    bool test(Tag &t, Tag &mask) { return (bits & mask.bits) == (t.bits & mask.bits); }
};

// This may not need to go here -- it may be a property of e.g. the progress module
// Single buffer wrong; ok for now
// Decorated MPI-class transmit
class Message
{
  public:
    int rank;
    MPI_Comm communicator;
    Tag tag;
    int context;
    i::Buf *buf;
    
};

// global symbol decls
namespace global
{
  extern int rank;
  extern int worldSize;
  extern std::string epochConfig;
  extern int epoch;
  extern exampi::Config *config;
  extern exampi::i::Interface *interface;
  extern exampi::i::Progress *progress;
  extern exampi::i::Transport *transport;
  extern std::unordered_map<MPI_Datatype, exampi::Datatype> datatypes;
  extern exampi::i::Checkpoint *checkpoint;
} // global

} //exampi

#endif
