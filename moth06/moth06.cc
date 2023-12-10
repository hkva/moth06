#include "moth06.hh"
#include "SDL_events.h"
#include "SDL_loadso.h"
#include "moth06_common/moth06_common.hh"

Application a = { };
Game g = { };

#define MOTH06_DYNAMIC_RELOAD

static void moth06_dbg_game(const char* message) {
    dbgmsg("[game] %s", message);
}

static bool moth06_load_file(const char* path, Array<u8>& data) {
    (void)path; (void)data;
    return false;
}

static bool reload_game() {
#ifdef MOTH06_OSX
    const char* dll_path_src = "libmoth06-game.dylib";
    const char* dll_path_dst = "libmoth06-game-dev.dylib";
#endif
    const char* dll_path = dll_path_src;
    // unload old lib
    if (a.game_lib != nullptr) {
        SDL_UnloadObject(a.game_lib);
    }
#ifdef MOTH06_DYNAMIC_RELOAD
    // copy dll
    if (!fs::copy(dll_path_src, dll_path_dst)) {
        die("Error while reloading game: Failed to copy %s to %s", dll_path_src, dll_path_dst);
    }
    dll_path = dll_path_dst;
#endif
    if (!(a.game_lib = SDL_LoadObject(dll_path))) {
        die("Error while reloading game: Failed to load %s", dll_path);
    }
    decltype(create_game)* create_game_func = (decltype(create_game)*)SDL_LoadFunction(a.game_lib, "create_game");
    ASSERT(create_game_func);

    const GameAppInterface appiface = {
        .dbg        = moth06_dbg_game,
        .load_asset = moth06_load_file,
    };
    create_game_func(&g, &appiface);

    return true;
}

static void moth06_main(usize argc, const char* argv[]) {
    a.argc = argc; a.argv = argv;
    for (usize i = 1; i < a.argc; ++i) {
        const char* f = a.argv[i];
        if (str::equal(f, "--headless")) {
            a.flags |= APP_FLAG_HEADLESS;
        } else {
            die("Unknown command-line argument: %s", f);
        }
    }

    reload_game();

    if (!(a.flags & APP_FLAG_HEADLESS)) {
        if (!(a.wnd = SDL_CreateWindow(APP_TITLE,
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, APP_WIDTH, APP_HEIGHT, 0))) {
            die("Failed to create game window: %s", SDL_GetError());
        }
        if (!(a.r = SDL_CreateRenderer(a.wnd, -1, SDL_RENDERER_ACCELERATED))) {
            die("Failed to create renderer: %s", SDL_GetError());
        }
    }

    // NOTE(HK):
    //   * Game simulation must happen at 60hz (except for when testing demos!)
    //   * When the user presses the X to close the window, the game instantly
    //     closes and nothing is saved. The game code needs to be ready to die
    //     at any time without bad things happening.
    do {
        // Reload code
        if (a.state & APP_STATE_WANTS_RELOAD) {
            reload_game();
            a.state &= ~APP_STATE_WANTS_RELOAD;
        }

        // Queue user events - live from user
        if (!(a.flags & APP_FLAG_HEADLESS)) {
            SDL_Event evt = { };
            while (SDL_PollEvent(&evt)) {
                switch (evt.type) {
                    case SDL_QUIT: {
                        a.state |= APP_STATE_WANTS_QUIT;
                    } break;
                    case SDL_KEYDOWN: {
                        switch (evt.key.keysym.sym){
                            case SDLK_r: {
                                a.state |= APP_STATE_WANTS_RELOAD;
                            } break;
                        }
                    } break;
                };
            }
        }
    } while (!(a.state & APP_STATE_WANTS_QUIT));
}

#ifdef MOTH06_OSX
#   include <mach-o/dyld.h>
#   include <unistd.h>
#endif

// XXX(HK): SDLmain
int main(int argc, const char* argv[]) {
    char path[1024] = { 0 }; u32 path_size = sizeof(path);
#ifdef MOTH06_OSX
    if (_NSGetExecutablePath(path, &path_size) != 0) {
        die("Failed to get executable path");
    }
#endif
    str::basename(path);
#ifdef MOTH06_OSX
    // works on linux too
    if (chdir(path) != 0) {
        die("Failed to reset working directory to %s", path);
    }
    dbgmsg("Reset working directory to %s", path);
#endif
    moth06_main(argc, argv);
    return 0;
}

