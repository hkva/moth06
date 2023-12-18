#include "hk.hh"

#ifdef HK_WINDOWS
#   include <Windows.h>
#endif

//
// System API
//

hk::u64 hk::sys::get_cpu_ticks() {
#ifdef HK_MSVC
    return (u64)__rdtsc();
#else
#   error not implemented
#endif
}