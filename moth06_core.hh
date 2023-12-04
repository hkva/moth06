#ifndef _MOTH06_CORE_HH_
#define _MOTH06_CORE_HH_

#include "moth06_common.hh"

//
// PBG archive parsing
//

namespace PBG {

// XXX(HK): Confirm
constexpr usize MAX_FILE_LENGTH = 256;

class FileEntry {
public:
    u32  e_unk1;
    u32  e_unk2;
    u32  e_chck;
    u32  e_foff;
    u32  e_fsiz;
    char e_name[MAX_FILE_LENGTH];
};

// Read archive entry info
bool ReadEntryList(BitStream bits, Array<FileEntry>& entries);

// Read and decompress archive entry
bool ReadEntryData(BitStream bits, const FileEntry& file, Array<u8>& data);

};

#endif // _MOTH06_CORE_HH_
