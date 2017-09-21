#ifndef __EXAMPI_BASIC_CHECKPOINT_H
#define __EXAMPI_BASIC_CHECKPOINT_H

#include <basic.h>
#include "basic/progress.h"

namespace exampi {
namespace basic {

class Checkpoint : public exampi::i::Checkpoint
{
  public:
    virtual void save()
    {
      // get a file.  this is actually nontrivial b/c of shared filesystems; we'll salt for now
      std::stringstream filename;
      filename << exampi::global::epoch << "." << exampi::global::rank << ".cp";
      std::ofstream target(filename.str(), std::ofstream::binary);

      // save the global datatype map
      uint32_t typecount = exampi::global::datatypes.size();
      target.write(reinterpret_cast<char *>(&typecount), sizeof(uint32_t));
      for(auto i : exampi::global::datatypes)
      {
        //i.save(target);
      }

      exampi::global::progress->save(target);
      exampi::global::transport->save(target);
      //exampi::global::interface->save(target);

      exampi::global::epoch++;
     
    } 

    virtual void load()
    {
      if(exampi::global::epoch == 0) // first init
      {
        exampi::global::transport->init();
        exampi::global::progress->init();
      }
      else   // subsequent init
      {
        // get a file.  this is actually nontrivial b/c of shared filesystems; we'll salt for now
        std::stringstream filename;
        filename << exampi::global::epoch << "." << exampi::global::rank << ".cp";
        std::ifstream target(filename.str(), std::ofstream::binary);

        // save the global datatype map
        uint32_t typecount;
        //target.write(&typecount, sizeof(uint32_t));
        for(auto i : exampi::global::datatypes)
        {
         // i.save(target);
        }

        exampi::global::progress->load(target);
        exampi::global::transport->load(target);
        //exampi::global::interface->save(target);
      }
    }
};

}} // exampi::basic::

#endif
