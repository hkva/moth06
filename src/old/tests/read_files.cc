#include "moth06_common/moth06_common.hh"
#include "moth06_game/moth06_game.hh"

#include "tests.hh"


struct Asset {
    PBGEntry info;
    Array<u8> data;
};

int main() {
    tests::require_game_files();

    Array<Asset> assets = Array<Asset>();

    // Try reading a PBG file
    {
        Array<u8> pbg_bytes = Array<u8>();
        if (!tests::load("./bin/gamefiles/紅魔郷ST.DAT", pbg_bytes)) {
            ASSERT(0 && "failed to load game archive file");
        }

        Array<PBGEntry> entries = Array<PBGEntry>();
        if (!read_pbg_entries(pbg_bytes.const_bytes(), entries)) {
            ASSERT(0 && "failed to read game archive file entries");
        }

        Array<u8> face12a = Array<u8>();
        for (usize i = 0; i < entries.length(); ++i) {
            PBGEntry& f = entries[i];

            static Array<u8> contents = Array<u8>();
            if (!read_pbg_entry_data(pbg_bytes.const_bytes(), f, contents)) {
                ASSERT_NOT_REACHED();
            }

            if (str::equal(f.e_name, "face12a.png")) {
                face12a = contents;
            }

            std::printf("[%d] %s\n", (int)i, f.e_name);
        }
        ASSERT(face12a.length());
        // 96681fc22212a1a5f441b391dfcd9314
        char face12a_digest[64] = { };
        hash::md5(face12a.const_bytes()).render(face12a_digest, sizeof(face12a_digest));
        std::printf("Hash of face12a.png: %s\n", face12a_digest);
        ASSERT(str::equal(face12a_digest, "96681fc22212a1a5f441b391dfcd9314"));

    }
    CHECK_LEAKS();

    static auto mount_archive = [&](const char* path) {
        Array<u8> pbg_bytes = Array<u8>();
        if (!tests::load(path, pbg_bytes)) {
            LOG("Failed to load %s", path); ASSERT(0);
        }
        Array<PBGEntry> entries = Array<PBGEntry>();
        if (!read_pbg_entries(pbg_bytes.const_bytes(), entries)) {
            ASSERT(0 && "failed to read game archive file entries");
        }
        for (auto& entry : entries) {
            static Array<u8> contents = Array<u8>();
            if (!read_pbg_entry_data(pbg_bytes.const_bytes(), entry, contents)) {
                LOG("Failed to load %s", entry.e_name); ASSERT(0);
            }
            Asset a = {
                .info = entry,
                .data = contents,
            };
            assets.append(a);
#if 1
            char fname[512]; std::snprintf(fname, sizeof(fname), "bin/%s", entry.e_name);
            std::FILE* fout = std::fopen(fname, "wb"); ASSERT(fout);
            std::fwrite(contents.buffer(), 1, contents.length(), fout);
            std::fclose(fout);
#endif
        }
        LOG("Mounted %s", path);
    };

    static auto get_asset = [&](const char* name, Array<u8>& data) {
        for (auto& asset : assets) {
            if (str::equal(asset.info.e_name, name)) {
                data = asset.data;
                return;
            }
        }
        ASSERT_NOT_REACHED();
    };

    mount_archive("./bin/gamefiles/紅魔郷CM.DAT");
    mount_archive("./bin/gamefiles/紅魔郷ED.DAT");
    mount_archive("./bin/gamefiles/紅魔郷IN.DAT");
    mount_archive("./bin/gamefiles/紅魔郷MD.DAT");
    mount_archive("./bin/gamefiles/紅魔郷ST.DAT");
    mount_archive("./bin/gamefiles/紅魔郷TL.DAT");

    static struct {
        u32 images;
        u32 anims;
        u32 sounds;
        u32 midi;
        u32 loops;
        u32 stages;
        u32 enemies;
        u32 replays;
        u32 endings;
        u32 text;
        u32 data;

        u32 unknown;
    } count = { };

    for (auto& asset : assets) {
        const char* ext = str::extension(asset.info.e_name);
        if (str::equal(ext, "png") || str::equal(ext, "jpg")) {
            count.images++;
            LOG("%s: Image", asset.info.e_name);
        } else if (str::equal(ext, "wav")) {
            count.sounds++;
            LOG("%s: Sound", asset.info.e_name);
        } else if (str::equal(ext, "mid")) {
            count.midi++;
            LOG("%s: MIDI sound", asset.info.e_name);
        } else if (str::equal(ext, "anm")) {
            count.anims++;
            Animation anim = Animation();
            if (!read_anm(asset.data.const_bytes(), anim)) {
                LOG("Failed to load animation %s", asset.info.e_name); ASSERT(0);
            }
            LOG("%s: Animation", asset.info.e_name);
            LOG("    .version:              %u", anim.version);
            LOG("    .texture_path:         %s", anim.texture_path);
            LOG("    .texture_alpha_path:   %s", anim.texture_alpha_path);
            LOG("    .sprites:              AnimationSprite[%u]", (u32)anim.sprites.length());
            LOG("    .scripts:              AnimationScript[%u]", (u32)anim.scripts.length());
        } else if (str::equal(ext, "pos")) {
            count.loops++;
            LOG("%s: Music loop marker", asset.info.e_name);
        } else if (str::equal(ext, "std")) {
            count.stages++;
            LOG("%s: Stage", asset.info.e_name);
        } else if (str::equal(ext, "ecl")) {
            count.enemies++;
            LOG("%s: Enemy AI bytecode", asset.info.e_name);
        } else if (str::equal(ext, "rpy")) {
            count.replays++;
            LOG("%s: Replay", asset.info.e_name);
        } else if (str::equal(ext, "end")) {
            count.endings++;
            LOG("%s: Ending script", asset.info.e_name);
        } else if (str::equal(ext, "txt")) {
            count.text++;
            LOG("%s: Text", asset.info.e_name);
        } else if (str::equal(ext, "dat")) {
            count.data++;
            LOG("%s: Raw data", asset.info.e_name);
        } else {
            count.unknown++;
            LOG("!!!!! %s: Unknown", asset.info.e_name);
        }
    }

    LOG("==================================================");
    LOG("total assets: %u", (u32)assets.length());
    LOG("  images   %u", count.images);
    LOG("  anims    %u", count.anims);
    LOG("  sounds   %u", count.sounds);
    LOG("  midi     %u", count.midi);
    LOG("  loops    %u", count.loops);
    LOG("  stages   %u", count.stages);
    LOG("  enemies  %u", count.enemies);
    LOG("  replays  %u", count.replays);
    LOG("  ending   %u", count.endings);
    LOG("  text     %u", count.text);
    LOG("  raw data %u", count.data);
    if (count.unknown > 0) {
        LOG("  unknown  %u", count.unknown);
    }
    LOG("==================================================");

    // Animation parsing
    {
        LOG("Parsing etama3.anm:");
        Array<u8> bytes = Array<u8>();
        get_asset("etama3.anm", bytes);
        Animation anim = Animation();
        if (!read_anm(bytes.const_bytes(), anim)) {
            ASSERT_NOT_REACHED();
        }
        LOG("Sprites: (%u):", (u32)anim.sprites.length());
        for (auto& s : anim.sprites) {
            LOG("    { idx = %u, x = %f, y = %f, w = %f, h = %f }", s.idx, s.x, s.y, s.w, s.h);
        }
        LOG("Scripts: (%u):", (u32)anim.scripts.length());
        for (auto& s : anim.scripts) {
            LOG("    { idx = %u, ops = [", s.idx);
            const char* OP_NAMES[] = {
                "End",
                "SetSprite",
                "SetScale",
                "SetAlpha",
                "SetColor",
                "Jump",
                "Unknown1",
                "ToggleMirrored",
                "Unknown2",
                "Set3DRotations",
                "Set3DRotationsSpeed",
                "SetScaleSpeed",
                "Fade",
                "SetBlendModeAdd",
                "SetBlendModeAlphaBlend",
                "KeepStill",
                "SetRandomSprite",
                "Set3DTranslation",
                "MoveToLinear",
                "MoveToDecel",
                "MoveToAccel",
                "Wait",
                "InterruptLabel",
                "SetCornerRelativePlacement",
                "WaitEx",
                "SetAllowedOffset",
                "SetAutoOrientation",
                "ShiftTextureX",
                "ShiftTextureY",
                "SetVisible",
                "ScaleIn",
            };
            for (auto& op : s.ops) {
                ASSERT((usize)op.type < ARRLEN(OP_NAMES));
                char args[256] = { 'N','o','n','e' };
                switch (op.type) {
                    case AnimationScriptOpType::SetSprite: { snprintf(args, sizeof(args), "{ idx = %u }", op.set_sprite); } break;
                    default: break;
                }
                LOG("        { time = %u, type = %s, size = %u, args = %s }", op.time, OP_NAMES[(usize)op.type], op.size, args);
            }
            LOG("    ]}");
        }
    }
}

