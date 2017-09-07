#ifndef __EXAMPI_I_CHECKPOINT_H
#define __EXAMPI_I_CHECKPOINT_H

#include <mpi.h>

namespace exampi {
namespace i {

class Checkpoint
{
  public:
    virtual void save(std::string descriptor) = 0;
    virtual void load(std::string descriptor) = 0;
};

}} // exampi::i
#endif
