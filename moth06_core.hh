#ifndef _MOTH06_CORE_HH_
#define _MOTH06_CORE_HH_

#include "moth06_common.hh"

//
// PBG archive parsing
//

// XXX(HK): Confirm
constexpr usize MAX_PBG_NAME = 256;

class PBGEntry {
public:
    u32  e_unk1;
    u32  e_unk2;
    u32  e_chck;
    u32  e_foff;
    u32  e_fsiz;
    char e_name[MAX_PBG_NAME];
};

// Only reads entry info table, not actual entry data
bool read_pbg_entries(BitStream bits, Array<PBGEntry>& entries);

// Read and decompress archive entry
bool read_pbg_entry_data(BitStream bits, const PBGEntry& file, Array<u8>& data);

//
// Game simulation
//

// Game->App interface
struct GameAppInterface {
    bool(*load_asset)(const char* path, Array<u8>& data);
    void(*dbg)(const char* message);
};

struct Game {
    GameAppInterface app;
};

void create_game(Game* game, const GameAppInterface* app_interface);

#endif // _MOTH06_CORE_HH_

