#ifndef _GAME_HH_
#define _GAME_HH_

#include "hk.hh"

//
// Engine->Game interface
//

struct GameInterface {
    usize size;
    // Game loop
};

//
// Game->Engine interface
//

struct EngineInterface {
    usize size;
    // Debugging
    void (*dbg_log)(const char* fmt, ...);
};

typedef bool(*ConnectGameFn)(const EngineInterface* ei, GameInterface* gi);

#endif // _GAME_HH_
