#ifndef __EXAMPI_I_PROGRESS_H
#define __EXAMPI_I_PROGRESS_H

#include <mpi.h>
#include <endpoint.h>

namespace exampi {
namespace i {

class Progress
{
  public:
    virtual int init() = 0;
    virtual void finalize() = 0;
    virtual int init(std::istream &t) = 0;
    virtual void barrier() = 0;
    virtual std::future<MPI_Status> postSend(UserArray array, Endpoint dest, int tag) = 0;
    virtual std::future<MPI_Status> postRecv(UserArray array, Endpoint source, int tag) = 0;
    virtual int save(std::ostream &t) = 0;
    virtual int load(std::istream& t) = 0;
    virtual int stop() = 0;
    virtual void cleanUp() = 0;
};

}} // ::exampi::i

#endif // __EXAMPI_I_PROGRESS_H
