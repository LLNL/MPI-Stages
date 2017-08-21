#ifndef EXAMPI_EXAMPI_H
#define EXAMPI_EXAMPI_H

/* Internal include for ExaMPI
 */

#include <vector>
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


namespace exampi
{

class ICheckpoint
{
  public:
    virtual void DoSomething() = 0;
};

class IFault
{
  public:
    virtual void DoSomething() = 0;
};

class IInterface
{
  public:
    virtual int MPI_Init(int *argc, char ***argv) = 0;
    virtual int MPI_Finalize(void) = 0;
    virtual int MPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
    virtual int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
};

class IMemory
{
  public:
    virtual void DoSomething() = 0;
};

class IProgress
{
  public:
    virtual int send_data(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
    virtual int recv_data(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
};

// send/recv buffer that knows how to describe itself as an iovec
class IBuf
{
  public:
    virtual struct iovec iov() = 0;
};

// as above, but as a vector
class IBufV
{
  public:
    virtual struct iovec *AsIovecV() = 0;
};

// Decorated MPI-class transmit
class IMessage
{
  public:
    int rank;
    uint32_t tag;
    int context;
    MPI_Comm communicator;
};

class ITransport
{
  public:
	//virtual void send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
  virtual void send(IBuf *buf, int dest, MPI_Comm comm) = 0;
  virtual void receive(IBuf *buf, MPI_Comm comm) = 0;
	//virtual int recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
};

}

#endif
