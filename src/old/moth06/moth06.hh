#ifndef _MOTH06_HH_
#define _MOTH06_HH_

#include "SDL_events.h"
#include "moth06_common/moth06_common.hh"
#include "moth06_game/moth06_game.hh"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "imgui.h"

#define APP_TITLE   "moth06"
#define APP_WIDTH   640
#define APP_HEIGHT  480

enum {
    APP_FLAG_HEADLESS = 1 << 0,
};

enum {
    APP_STATE_WANTS_QUIT    = 1 << 0,
    APP_STATE_WANTS_RELOAD  = 1 << 1,
    APP_STATE_DEBUG_MENU    = 1 << 2,
};

struct Application {
    usize argc; const char** argv;
    u8 flags;
    u8 state;

    void* game_lib;

    SDL_Window* wnd;
};
extern Application  a;
extern Game         g;

static inline void die(const char* fmt, ...) {
    static char buf[512];
    std::va_list va; va_start(va, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    std::fprintf(stderr, "ERROR: %s\n", buf);
}

static inline void dbgmsg(const char* fmt, ...) {
    static char buf[512];
    std::va_list va; va_start(va, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    std::fprintf(stdout, "%s\n", buf);
}

//
// Graphics
//

namespace gfx {

enum class Backend {
    Default,
    SDLRenderer,
};

struct InitOptions {
    Backend requested_backend;
};

void init(const InitOptions* opts);

void handle_ui_event(const SDL_Event* evt);

void begin_frame();
void end_frame();

}

#endif // _MOTH06_HH_
