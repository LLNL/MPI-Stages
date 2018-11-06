#ifndef __EXAMPI_I_CHECKPOINT_H
#define __EXAMPI_I_CHECKPOINT_H

#include <mpi.h>

namespace exampi
{
namespace i
{

class Checkpoint
{
public:
    virtual void save() = 0;
    virtual int load() = 0;
};

}
} // exampi::i
#endif
