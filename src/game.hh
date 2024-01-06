#ifndef _GAME_HH_
#define _GAME_HH_

#include "hk.hh"

//
// Engine->Game interface
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

struct GameInterface {
	usize size;
	// PBG parsing
	struct {
		bool(*parse_entries)(Span<const u8> archive, Array<PBGEntry>& entries);
		bool(*decompress_data)(Span<const u8> archive, const PBGEntry& file, Array<u8>& data);
	} pbg;
};

//
// Game->Engine interface
//

struct EngineInterface {
	usize size;
	// Debugging
	void (*dbg_log)(const char* fmt, ...);
	// Asset loading
	bool (*load_asset)(const char* path, Array<u8>& data);
};

typedef bool(*ConnectGameFn)(const EngineInterface* ei, GameInterface* gi);

#endif // _GAME_HH_
