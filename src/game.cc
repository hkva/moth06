#include "game.hh"

extern "C" HK_DLL_EXPORT bool connect_game(const EngineInterface* ei, GameInterface* gi) {
    // Cannot reload if structure layout changed
    if (ei->size != sizeof(*ei) || gi->size != sizeof(*gi)) {
        return false;
    }

    ei->dbg_log("Game connected");
    return true;
}
