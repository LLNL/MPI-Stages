#ifndef __EXAMPI_I_TRANSPORT_H
#define __EXAMPI_I_TRANSPORT_H

#include <mpi.h>

namespace exampi {
namespace i {

class Transport
{
  public:
  virtual void init() = 0;
  virtual size_t addEndpoint(const int rank, const std::vector<std::string> &opts) = 0; 
  virtual std::future<int> send(std::vector<struct iovec> iov, int dest, MPI_Comm comm) = 0;
  virtual std::future<int> receive(std::vector<struct iovec> iov, MPI_Comm comm) = 0;
  virtual int peek(std::vector<struct iovec> iov, MPI_Comm comm) = 0;
  virtual int save(std::ostream &r) = 0;
  virtual int load(std::istream &r) = 0;
};

}} // ::exampi::i

#endif 
