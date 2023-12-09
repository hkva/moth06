#include <std/core.pat>

enum ANM_OpType : u8 {
    Delete = 0,
    SetSprite = 1,
    SetScale = 2,
    SetAlpha = 3,
    SetColor = 4,
    Jump = 5,
    ToggleMirrored = 7,
    Set3DRotations = 9,
    Set3DRotationsSpeed = 10,
    SetScaleSpeed = 11,
    Fade = 12,
    SetBlendModeAdd = 13,
    SetBlendModeAlphaBlend = 14,
    KeepStill = 15,
    SetRandomSprite = 16,
    Set3DTranslation = 17,
    MoveToLinear = 18,
    MoveToDecel = 19,
    MoveToAccel = 20,
    Wait = 21,
    InterruptLabel = 22,
    SetCornerRelativePlacement = 23,
    WaitEx = 24,
    SetAllowOffset = 25,
    SetAutomaticOrientation = 26,
    ShiftTextureX = 27,
    ShiftTextureY = 28,
    SetVisible = 29,
    ScaleIn = 30,
};

struct ANM_Op {
    u16 time;
    ANM_OpType type;
    u8 size;
    if (type == ANM_OpType::SetSprite) {
        u32 par_sprite;
    } else {
        u8 data[size];
    }
};

struct ANM_Script {
    u32 idx;
    u32 instr_off;
    ANM_Op op[while(std::mem::read_unsigned($+2, 1) != 0)] @ instr_off;
};

struct ANM_SpriteData {
    u32     idx;
    float   x;
    float   y;
    float   w;
    float   h;
};

struct ANM_Sprite {
    u32 data_off;
    ANM_SpriteData data @ data_off [[inline]];
};

struct ANM_Path {
    u32 offset;
    if (offset > 0) {
        char path[32] @ offset;
    }
};

struct ANM {
    u32 num_sprites;
    u32 num_scripts;
    u32;
    u32 width;
    u32 height;
    u32 fmt;
    u32;
    ANM_Path texture_path;
    u32;
    ANM_Path texture_a_path;
    u32 version;
    u32;
    u32 texture_offset;
    u32 has_data;
    u32 next_offset;
    u32;
    ANM_Sprite sprites[num_sprites];
    ANM_Script scripts[num_scripts];
};
ANM anm @ 0;