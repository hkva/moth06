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

    gi.size = sizeof(gi);

    ei.size = sizeof(ei);
    ei.dbg_log = e_dbg_log;

    load_game();
    return 0;
}
