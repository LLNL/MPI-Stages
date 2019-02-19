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
