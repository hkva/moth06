#include "hk.hh"

#ifdef HK_WINDOWS
#   include <Windows.h>
#endif

//
// System API
//

hk::u64 hk::sys::get_cpu_ticks() {
    // NOTE(HK): This is probably really bad and wrong
    // MSVC: Assume ARM64, just use __rdtsc - XXX(HK): wrong on multi-core systems?
    // CLANG: Use __builtin_readcyclecounter() on ARM64, otherwise read CNTVCT_EL0 on ARM64
#if defined(HK_MSVC)
    return (u64)__rdtsc();
#elif defined(HK_CLANG)
#ifdef __aarch64__
    u64 result = 0;
    asm volatile("mrs %0, cntvct_el0" : "=r" (result));
    return result;
#else
    return (u64)__builtin_readcyclecounter();
#endif
#else
#   error not implemented
#endif
}
