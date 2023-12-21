#include "moth06.hh"

App a = { };
EngineInterface ei = { };
GameInterface gi = { };

static void e_dbg_log(const char* fmt, ...) {
    char buf[1024];
    std::va_list va; va_start(va, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    dbgmsg("[GAME] %s", buf);
}

static void load_game() {
#ifdef HK_WINDOWS
    const char* game_dll_src = "moth06_game.dll";
    const char* game_dll_dst = "moth06_game_loaded.dll";
#endif
#ifdef HK_MACOS
    const char* game_dll_src = "libmoth06_game.dylib";
    const char* game_dll_dst = "libmoth06_game_loaded.dylib";
#endif
    if (a.game_lib) {
        SDL_UnloadObject(a.game_lib);
    }
    if (!hk::sys::copy_file(game_dll_src, game_dll_dst)) {
        die("Failed to copy %s to %s", game_dll_src, game_dll_dst);
    }
    if (!(a.game_lib = SDL_LoadObject(game_dll_dst))) {
        die("Failed to load %s", game_dll_dst);
    }
    ConnectGameFn connect_game = (ConnectGameFn)SDL_LoadFunction(a.game_lib, "connect_game");
    HK_ASSERT(connect_game);
    if (!connect_game(&ei, &gi)) {
        die("Failed to connect game");
    }
}

int main(int argc, char** argv) {
    hk::sys::create_console();

    a.argc = argc; a.argv = (const char**)argv;

    SDL_version sdlv_c = { }; SDL_VERSION(&sdlv_c);
    SDL_version sdlv_l = { }; SDL_GetVersion(&sdlv_l);
    dbgmsg("SDL v%d.%d.%d (compiled against v%d.%d.%d)",
        sdlv_l.major, sdlv_l.minor, sdlv_l.patch,
        sdlv_c.major, sdlv_c.minor, sdlv_c.patch);
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        die("Failed to initialize SDL: %s", SDL_GetError());
    }

    if (!(a.wnd = SDL_CreateWindow("Moth06", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_HIDDEN))) {
        die("Failed to create game window: %s", SDL_GetError());
    }

    GfxInitParams par = { };
    par.requested_backend = GfxBackend::Default;
    init_gfx(par);

    gi.size = sizeof(gi);

    ei.size = sizeof(ei);
    ei.dbg_log = e_dbg_log;

    load_game();

    SDL_ShowWindow(a.wnd);
    do {
        SDL_Event evt = { };
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
            case SDL_QUIT: {
                a.state |= APP_STATE_WANTS_QUIT;
            } break;
            }
        }
    } while (!(a.state & APP_STATE_WANTS_QUIT));

    // NOTE(HK): Normally I just let the OS clean everything up, but some Linux WMs don't restore the display
    // resolution when a fullscreen window dies with a non-native resolution
    SDL_DestroyWindow(a.wnd);

    return 0;
}
