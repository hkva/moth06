#ifndef _MOTH06_HH_
#define _MOTH06_HH_

#include "hk.hh"
#include "game.hh"

#include <SDL.h>

static inline void die(const char* fmt, ...) {
    char buf[1024];
    std::va_list va; va_start(va, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    // XXX
    std::fprintf(stderr, "%s\n", buf);
    HK_ASSERT(0);
}

static inline void dbgmsg(const char* fmt, ...) {
    char buf[1024];
    std::va_list va; va_start(va, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    // XXX
    std::fprintf(stderr, "%s\n", buf);
}

struct App {
    usize argc; const char** argv;
    void* game_lib;
};

extern App a;

#endif // _MOTH06_HH_
