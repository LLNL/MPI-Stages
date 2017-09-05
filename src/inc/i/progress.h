#ifndef __EXAMPI_I_PROGRESS_H
#define __EXAMPI_H_PROGRESS_H

#include <mpi.h>
#include <endpoint.h>

namespace exampi {
namespace i {

class Progress
{
  public:
    virtual int init() = 0;
    virtual void barrier() = 0;
    virtual int send_data(void *buf, size_t count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) = 0;
    virtual int recv_data(void *buf, size_t count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) = 0;
    //virtual std::promise postSend(const void *, MPI_Datatype datatype, )
    virtual std::future<MPI_Status> postSend(UserArray array, Endpoint dest, int tag) = 0;
    virtual std::future<MPI_Status> postRecv(UserArray array, int tag) = 0;
    //virtual std::future<MPI_Request> nextCompletion() = 0;
};

}} // ::exampi::i

#endif
