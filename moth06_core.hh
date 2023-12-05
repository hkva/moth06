#ifndef _MOTH06_CORE_HH_
#define _MOTH06_CORE_HH_

#include "moth06_common.hh"

//
// PBG archive parsing
//

namespace pbg {

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
bool read_entry_list(BitStream bits, Array<FileEntry>& entries);

// Read and decompress archive entry
bool read_entry_data(BitStream bits, const FileEntry& file, Array<u8>& data);

};

//
// Game simulation
//

namespace game {

enum class Button {
    Up,
    Down,
    Left,
    Right,
};

enum class ButtonState {
    Press,
    Release,
};

struct InputEvent {
    Button      button;
    ButtonState state;
};

// XXX(HK): Advantage of this over function pointer?
class ResourceProvider {
public:
    virtual bool load_file(const char* path, Array<u8>& data);
};

class Simulator {
private:
    Array<InputEvent> m_queued_inputs;
    ResourceProvider* m_resources;
public:
    void init(ResourceProvider* resources);
    void add_input(InputEvent evt) { m_queued_inputs.append(evt); }
};

};

#endif // _MOTH06_CORE_HH_

