#ifndef _MOTH06_HH_
#define _MOTH06_HH_

#include "hk.hh"
#include "game.hh"

#include <imgui.h>
#include <SDL.h>

//
// Utility functions
//

static inline void die(const char* fmt, ...) {
    char buf[1024];
    std::va_list va; va_start(va, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    // XXX
    std::fprintf(stderr, "%s\n", buf);
    HK_ASSERT(0);
}

static inline void dbgmsg_(const char* fmt, ...) {
    char buf[1024];
    std::va_list va; va_start(va, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    // XXX
    std::fprintf(stderr, "+%.3f | %s\n", (f32)SDL_GetTicks() / 1e3f, buf);
    std::fflush(stderr);
}

//
// Application
//

enum {
    APP_STATE_WANTS_QUIT = 1 << 0,
    APP_STATE_DEBUG_UI = 1 << 1,
};

struct App {
    usize argc; const char** argv;
    void* game_lib;
    SDL_Window* wnd;
    u8 state;
};

extern App a;

// tests
void moth06_test();

//
// Graphics
//

enum class GfxBackend {
    SDLRenderer,

    Default,
};

struct GfxInitParams {
    GfxBackend requested_backend;
};

void init_gfx(const GfxInitParams& params);
void begin_frame();
void end_frame();
void handle_ui_event( const SDL_Event* evt );


#endif // _MOTH06_HH_
