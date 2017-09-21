#ifndef __EXAMPI_I_INTERFACE_H
#define __EXAMPI_I_INTERFACE_H

#include <mpi.h>

namespace exampi {
namespace i {

class Interface
{
  public:
    virtual int MPI_Init(int *argc, char ***argv) = 0;
    virtual int MPI_Finalize(void) = 0;
    virtual int MPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
    virtual int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
    virtual int MPI_Isend(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) = 0;
    virtual int MPI_Irecv(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request) = 0;
    virtual int MPI_Wait(MPI_Request *request, MPI_Status *status) = 0;
    virtual int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm) = 0;
    virtual int MPI_Comm_rank(MPI_Comm comm, int *r) = 0;
    virtual int MPI_Comm_size(MPI_Comm comm, int *r) = 0;
    virtual int MPI_Checkpoint(int *savedEpoch) = 0;

    //virtual int MPIX_Checkpoint(MPI_Comm comm) = 0;
};

}} // exampi::i
#endif
