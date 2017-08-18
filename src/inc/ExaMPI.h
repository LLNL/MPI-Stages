#ifndef EXAMPI_EXAMPI_H
#define EXAMPI_EXAMPI_H

/* Internal include for ExaMPI
 */

#include <vector>

#include <mpi.h>

namespace exampi
{

// SF:  This is convenience.  We can promote to public if useful.
typedef uint32_t tag;

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

// sendable buffer that knows how to describe itself as an iovec
class IMsg
{
  public:
    virtual struct iovec AsIovec() = 0;
};

class ITransport
{
  public:
	//virtual void send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
  virtual void Send(IMsg *msg, int dest, MPI_Comm comm) = 0;
  virtual void Receive(IMsg *msg, MPI_Comm comm) = 0;
	//virtual int recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
};

}

#endif
