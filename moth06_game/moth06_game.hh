#ifndef _MOTH06_CORE_HH_
#define _MOTH06_CORE_HH_

#include "moth06_common/moth06_common.hh"

#define GAME_API extern "C"

//
// Error handling
//

GAME_API const char* get_parser_error();

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
GAME_API bool read_pbg_entries(Span<const u8> archive, Array<PBGEntry>& entries);

// Read and decompress archive entry
GAME_API bool read_pbg_entry_data(Span<const u8> archive, const PBGEntry& file, Array<u8>& data);

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

enum class AnimationScriptOpType : u8 {
    End                         = 0,
    SetSprite                   = 1,
    SetScale                    = 2,
    SetAlpha                    = 3,
    SetColor                    = 4,
    Jump                        = 5,
    Unknown1                    = 6, // XXX
    ToggleMirrored              = 7,
    Unknown2                    = 8, // XXX
    Set3DRotations              = 9,
    Set3DRotationsSpeed         = 10,
    SetScaleSpeed               = 11,
    Fade                        = 12,
    SetBlendModeAdd             = 13,
    SetBlendModeAlphaBlend      = 14,
    KeepStill                   = 15,
    SetRandomSprite             = 16,
    Set3DTranslation            = 17,
    MoveToLinear                = 18,
    MoveToDecel                 = 19,
    MoveToAccel                 = 20,
    Wait                        = 21,
    InterruptLabel              = 22,
    SetCornerRelativePlacement  = 23,
    WaitEx                      = 24,
    SetAllowedOffset            = 25,
    SetAutoOrientation          = 26,
    ShiftTextureX               = 27,
    ShiftTextureY               = 28,
    SetVisible                  = 29,
    ScaleIn                     = 30,
};

struct AnimationScriptOp {
    u16 time;
    AnimationScriptOpType type;
    u8 size;

    union {
        u32 set_sprite;
        // XXX more
    };
};

struct AnimationScript {
    u32 idx;
    Array<AnimationScriptOp> ops;
};

struct Animation {
    u32     version;
    char    texture_path[MAX_ANM_NAME];
    char    texture_alpha_path[MAX_ANM_NAME];

    Array<AnimationSprite> sprites;
    Array<AnimationScript> scripts;
};

GAME_API bool read_anm(Span<const u8> file, Animation& anim);

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

GAME_API void create_game(Game* game, const GameAppInterface* app_interface);

#endif // _MOTH06_CORE_HH_

