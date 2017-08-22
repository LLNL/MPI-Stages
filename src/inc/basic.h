#ifndef __EXAMPI_BASIC_H
#define __EXAMPI_BASIC_H

#include <ExaMPI.h>
#include <UDPSocket.h>
#include <config.h>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <sys/uio.h>

namespace exampi {
namespace basic  {

const std::string runtimeConfig("mpihosts.stdin.tmp");

class Buf : public exampi::i::Buf
{
  private:
    struct iovec v;
  public:
    Buf(void *p, size_t sz) {v.iov_base = p; v.iov_len = sz;}
    virtual struct iovec iov() { return v;}
};


} // namespace basic
} // namespace exampi
#endif //guard
