#ifndef __EXAMPI_BASIC_CHECKPOINT_H
#define __EXAMPI_BASIC_CHECKPOINT_H

#include <basic.h>
#include "basic/progress.h"

namespace exampi {
namespace basic {

class Checkpoint : public exampi::i::Checkpoint
  public:
    virtual void save(std::string desc)
    {
      // need to save
    } 

}} // exampi::basic::

#endif
