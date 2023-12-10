#include "moth06.hh"

Application a = { };
Game g = { };

static void moth06_dbg_game(const char* message) {
    dbgmsg("[core] %s", message);
}

static bool moth06_load_file(const char* path, Array<u8>& data) {
    (void)path; (void)data;
    return false;
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

    if (!(a.flags & APP_FLAG_HEADLESS)) {
        if (!(a.wnd = SDL_CreateWindow(APP_TITLE,
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, APP_WIDTH, APP_HEIGHT, 0))) {
            die("Failed to create game window: %s", SDL_GetError());
        }
        if (!(a.r = SDL_CreateRenderer(a.wnd, -1, SDL_RENDERER_ACCELERATED))) {
            die("Failed to create renderer: %s", SDL_GetError());
        }
    }

    const GameAppInterface appiface = {
        .dbg        = moth06_dbg_game,
        .load_asset = moth06_load_file,
    };
    create_game(&g, &appiface);

    // NOTE(HK):
    //   * Game simulation must happen at 60hz (except for when testing demos!)
    //   * When the user presses the X to close the window, the game instantly
    //     closes and nothing is saved. The game process needs to be ready to die
    //     at any time without bad things happening.
    do {
        // Queue user events - live from user
        if (!(a.flags & APP_FLAG_HEADLESS)) {
            SDL_Event evt = { };
            while (SDL_PollEvent(&evt)) {
            }
        }
    } while (false);
}

// XXX(HK): SDLmain
int main(int argc, const char* argv[]) {
    moth06_main(argc, argv);
    return 0;
}

