#include "moth06.hh"
#include "moth06_common.hh"

Game g = { };

static void moth06_main(int argc, const char* argv[]) {
    g.argc = argc; g.argv = argv;
    if (!(g.wnd = SDL_CreateWindow(APP_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, APP_WIDTH, APP_HEIGHT, 0))) {
        die("Failed to create game window: %s", SDL_GetError());
    }
    if (!(g.r = SDL_CreateRenderer(g.wnd, -1, SDL_RENDERER_ACCELERATED))) {
        die("Failed to create renderer: %s", SDL_GetError());
    }
}

#if defined(__APPLE__) && defined(__MACH__)

#include <mach-o/dyld.h>
#include <unistd.h>

int main(int argc, const char* argv[]) {
    // Reset working directory
    char path[1024] = { }; u32 path_size = sizeof(path);
    if (_NSGetExecutablePath(path, &path_size) != 0) {
        die("OSX: Failed to get executable path");
    }
    if (!Str::BaseName(path)) {
        die("OSX: Failed to truncate executable path (%s)", path);
    }
    if (chdir(path) != 0) {
        die("OSX: Failed to reset working directory to %s", path);
    }
    moth06_main(argc, argv);
    return 0;
}

#endif
