#ifndef _EXAMPI_COMM_H
#define _EXAMPI_COMM_H

#include <Exampi.h>

namespace exampi {

class Comm
{
 public:
 Comm(bool _isintra, const Group *_local, const Group *_remote) : isintra(_isintra), local(_local), remote(_remote) {
    local_pt2pt = Context::get_next_context(); local_coll = Context::get_next_context(); }
  virtual ~Comm() {}

  // fix rule of 5 later 
  // Comm &operator=(Comm &rhs) = 0;

  // accessors
  const context_id get_context_id_pt2pt() {return local_pt2pt;}
  const context_id get_context_id_coll()  {return local_coll;}

  // Nawrin task for later, introduce the entire MPI API here as methods of this comm; right now, we do "Shane-mode,"
  // where the C API directly calls the Interface singledon, which is allowed to use accessors of Comm for info.
  //
  // [future version only]
  //
 protected:

  const Group *local;
  const Group *remote;

  context_id local_pt2pt;
  context_id local_coll;
  //  context_id remote_pt2pt;
  //  context_id remote_coll;

 private:


};



}


#endif
