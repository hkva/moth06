#ifndef _MOTH06_HH_
#define _MOTH06_HH_

#include "moth06_common.hh"
#include "moth06_core.hh"

#include <SDL.h>

#define APP_TITLE   "Moth06"
#define APP_WIDTH   640
#define APP_HEIGHT  480

struct Game {
    int argc; const char** argv;
    SDL_Window*     wnd;
    SDL_Renderer*   r;
};
extern Game g;

static inline void die(const char* fmt, ...) {
    static char buf[512] = { };
    std::va_list va; va_start(va, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    std::fprintf(stderr, "ERROR: %s\n", buf);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "moth06 - ERROR", buf, g.wnd);

    std::abort();
}

#endif // _MOTH06_HH_
