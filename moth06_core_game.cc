#include "moth06_core.hh"

static const char* fmt_(const char* fmt, ...) {
    static char buf[512];
    std::va_list va; va_start(va, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);
    return buf;
}
#define DBG(...) game->app.dbg(fmt_(__VA_ARGS__))

void create_game(Game* game, const GameAppInterface* app_interface) {
    game->app = *app_interface;
    DBG("Connected game!");
}

