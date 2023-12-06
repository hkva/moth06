#ifndef _MOTH06_HH_
#define _MOTH06_HH_

#include "moth06_common.hh"
#include "moth06_core.hh"

#include <SDL.h>

#define APP_TITLE   "moth06"
#define APP_WIDTH   640
#define APP_HEIGHT  480

enum {
    APP_FLAG_HEADLESS = 1 << 0,
};

enum {
    APP_STATE_WANTS_QUIT = 1 << 0,
};

struct Application {
    usize argc; const char** argv;
    u8 flags;
    u8 state;

    SDL_Window* wnd;
    SDL_Renderer* r;
};
extern Application      a;
extern game::Simulator  g;

static inline void die(const char* fmt, ...) {
    static char buf[512];
    std::va_list va; va_start(va, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    std::fprintf(stderr, "ERROR: %s\n", buf);
}

#endif // _MOTH06_HH_
