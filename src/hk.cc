#include "hk.hh"

#ifdef HK_WINDOWS
#   include <Windows.h>
#endif
#ifdef HK_MACOS
#   include <unistd.h>
#   include <mach-o/dyld.h>
#endif

//
// System API
//

bool hk::sys::chdir(const char* path) {
#ifdef HK_WINDOWS
    return SetCurrentDirectoryA(path);
#else
    return ::chdir(path) == 0;
#endif
}

bool hk::sys::copy_file(const char* src_path, const char* dst_path) {
    std::FILE* f_src = std::fopen(src_path, "rb");
    std::FILE* f_dst = std::fopen(dst_path, "wb");
    if (!f_src || !f_dst) {
        return false;
    }
    u8 buf[512] = { };
    while (true) {
        usize len = std::fread(buf, 1, arrlen(buf), f_src);
        if (len == 0) {
            break;
        }
        std::fwrite(buf, 1, len, f_dst);
    }
    std::fclose(f_src);
    std::fclose(f_dst);
    return true;
}

void hk::sys::create_console() {
#ifdef HK_WINDOWS
    if (AllocConsole()) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif
}

bool hk::sys::get_exe_path(char* path, usize len) {
#ifdef HK_WINDOWS
    return GetModuleFileNameA(NULL, path, (DWORD)len);
#elif defined(HK_MACOS)
    u32 len2 = (u32)len;
    return _NSGetExecutablePath(path, &len2) == 0;
#else
#   error not implemented
#endif
}

hk::u64 hk::sys::get_cpu_ticks() {
    // NOTE(HK): This is probably really bad and wrong
    // MSVC: Assume ARM64, just use __rdtsc - XXX(HK): wrong on multi-core systems?
    // CLANG: Use __builtin_readcyclecounter() on AMD64, otherwise read CNTVCT_EL0 on ARM64
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
