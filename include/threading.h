// 
// This is a source code component of ExaMPI, a next-generation 
// implementation of the Message Passing Interface.
//
// This is a joint project of University of Tennessee at Chattanooga,
// the Lawrence Livermore National Laboratory, and Auburn University.
//
// This file is subject to the terms and conditions defined in
// file 'LICENSE', which is part of this source code package.
// 

#ifndef __EXAMPI_THREADING_H
#define __EXAMPI_THREADING_H

namespace exampi
{

#ifdef THREADING_SINGLE

#define lock_guard(guard)
#define unique_lock(guard)

#else

#define lock_guard(guard) std::lock_guard<std::mutex> lock(guard);
#define unique_lock(guard) std::unique_lock<std::mutex> lock(guard);

#endif

}

#endif
