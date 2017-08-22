#ifndef EXAMPI_EXAMPI_H
#define EXAMPI_EXAMPI_H

/* Internal include for ExaMPI
 */

#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

#include <mpi.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <config.h>

namespace exampi {

namespace i {
class Checkpoint
{
  public:
    virtual void DoSomething() = 0;
};

class Fault
{
  public:
    virtual void DoSomething() = 0;
};

class Interface
{
  public:
    virtual int MPI_Init(int *argc, char ***argv) = 0;
    virtual int MPI_Finalize(void) = 0;
    virtual int MPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
    virtual int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
    virtual int MPI_Comm_rank(MPI_Comm comm, int *r) = 0;
    virtual int MPI_Comm_size(MPI_Comm comm, int *r) = 0;
};

class Memory
{
  public:
    virtual void DoSomething() = 0;
};

class Progress
{
  public:
    virtual int init() = 0;
    virtual int send_data(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
    virtual int recv_data(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
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

class Transport
{
  public:
  virtual size_t addEndpoint(const int rank, const std::vector<std::string> &opts) = 0; 
  virtual void send(Buf *buf, int dest, MPI_Comm comm) = 0;
  virtual void receive(Buf *buf, MPI_Comm comm) = 0;
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
  extern exampi::Config *config;
  extern exampi::i::Interface *interface;
  extern exampi::i::Progress *progress;
  extern exampi::i::Transport *transport;
} // global

} //exampi

#endif
