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
      std::ofstream target(filename.str(), std::ofstream::out);

      // save the global datatype map
      //uint32_t typecount = exampi::global::datatypes.size();
      //target.write(reinterpret_cast<char *>(&typecount), sizeof(uint32_t));
      for(auto i : exampi::global::datatypes)
      {
        //i.save(target);
      }

      //exampi::global::progress->save(target);
      exampi::global::transport->save(target);
      //exampi::global::interface->save(target);
      target.close();

      errHandler handler;
      if (handler.isErrSet() != 1) {
    	  exampi::global::epoch++;

    	        std::ofstream ef(exampi::global::epochConfig);
    	        ef << exampi::global::epoch;
    	        ef.close();
      }


    } 

    virtual int load()
    {
      if(exampi::global::epoch == 0) // first init
      {
    	//std::cout << "In init checkpoint load\n";
        exampi::global::transport->init();
        exampi::global::progress->init();
        return 0;
      }
      else   // subsequent init
      {
        // get a file.  this is actually nontrivial b/c of shared filesystems; we'll salt for now
        std::stringstream filename;
        filename << exampi::global::epoch - 1 << "." << exampi::global::rank << ".cp";
        std::ifstream target(filename.str(), std::ifstream::in);

        // save the global datatype map
        //target.write(&typecount, sizeof(uint32_t));
        for(auto i : exampi::global::datatypes)
        {
         // i.save(target);
        }
        std::cout << "In re-init\n";
        exampi::global::progress->load();
        exampi::global::transport->load(target);
        exampi::global::progress->barrier();
        //exampi::global::interface->save(target);
        target.close();
        std::ifstream ef(exampi::global::epochConfig);
        ef >> exampi::global::epoch;
        ef.close();
      }
      return MPIX_SUCCESS_RESTART;
    }
};

}} // exampi::basic::

#endif
