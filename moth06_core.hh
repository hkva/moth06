#ifndef _MOTH06_CORE_HH_
#define _MOTH06_CORE_HH_

#include "moth06_common.hh"

//
// Error handling
//

const char* get_parser_error();

//
// PBG archive parsing
//

// XXX(HK): Confirm
constexpr usize MAX_PBG_NAME = 256;

struct PBGEntry {
    u32  e_unk1;
    u32  e_unk2;
    u32  e_chck;
    u32  e_foff;
    u32  e_fsiz;
    char e_name[MAX_PBG_NAME];
};

// Only reads entry info table, not actual entry data
bool read_pbg_entries(Span<const u8> archive, Array<PBGEntry>& entries);

// Read and decompress archive entry
bool read_pbg_entry_data(Span<const u8> archive, const PBGEntry& file, Array<u8>& data);

//
// ANM animation parsing
//

/// XXX(HK): Confirm
constexpr usize MAX_ANM_NAME = 32;

struct AnimationSprite {
    u32 idx;
    f32 x;
    f32 y;
    f32 w;
    f32 h;
};

struct Animation {
    u32     version;
    char    primary_name[MAX_ANM_NAME];
    char    secondary_name[MAX_ANM_NAME];

    Array<AnimationSprite> sprites;
};

bool read_anm(Span<const u8> file, Animation& anim);

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

