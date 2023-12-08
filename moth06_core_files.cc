#include "moth06_core.hh"

//
// Error handling
//

static char parser_error[512] = { 'N','o',' ','e','r','r','o','r' };

const char* get_parser_error() {
    return parser_error;
}

static void set_parser_error(const char* fmt, ...) {
    std::va_list va; va_start(va, fmt);
    std::vsnprintf(parser_error, sizeof(parser_error), fmt, va);
    va_end(va);
}

//
// PBG archive parsing
//

bool read_pbg_entries(Span<const u8> archive, Array<PBGEntry>& entries) {
    BitStream bits = BitStream(archive);
    const char magic[4] = {
        bits.read_bits<char>(),
        bits.read_bits<char>(),
        bits.read_bits<char>(),
        bits.read_bits<char>(),
    };

    static constexpr char PBG_MAGIC[] = { 'P', 'B', 'G', '3' };
    if (!mem::equal(magic, PBG_MAGIC, ARRLEN(PBG_MAGIC))) {
        set_parser_error("Invalid file header");
        return false;
    }

    const u32 etbl_num = bits.read_int();
    const u32 etbl_off = bits.read_int();

    bits.seek(etbl_off, 0);

    entries.resize(etbl_num);
    for (usize i = 0; i < entries.length(); ++i) {
        PBGEntry& e = entries[i];
        e.e_unk1 = bits.read_int();
        e.e_unk2 = bits.read_int();
        e.e_chck = bits.read_int();
        e.e_foff = bits.read_int();
        e.e_fsiz = bits.read_int();
        bits.read_c_string(e.e_name, ARRLEN(e.e_name));
    }

    return !bits.overrun();
}

bool read_pbg_entry_data(Span<const u8> archive, const PBGEntry& file, Array<u8>& data) {
    BitStream bits = BitStream(archive);
    // Touhou-specific LZSS encoding options
    // XXX(HK): These are copy/pasted from PyTouhou, confirm these
    constexpr usize LZSS_DICTIONARY_SIZE    = 0x2000;
    constexpr u32   LZSS_MIN_MATCH_LENGTH   = 3;
    constexpr usize LZSS_OFFSET_BITS        = 13;
    constexpr usize LZSS_LENGTH_BITS        = 4;

    data.resize(file.e_fsiz);
    // LZSS decompression
    bits.seek(file.e_foff, 0);
    Array<u8> dictionary = Array<u8>(LZSS_DICTIONARY_SIZE);
    usize dictionary_head = 1;
    for (usize i = 0; i < data.length(); ) {
        if (bits.read_bits(1)) {
            // literal word
            const u8 octet = bits.read_bits<u8>();
            dictionary[dictionary_head] = octet;
            dictionary_head = (dictionary_head + 1) % LZSS_DICTIONARY_SIZE;
            data[i++] = octet;
        } else {
            // control word
            const u32 cw_off = bits.read_bits(LZSS_OFFSET_BITS);
            const u32 cw_len = bits.read_bits(LZSS_LENGTH_BITS) + LZSS_MIN_MATCH_LENGTH;
            for (u32 j = cw_off; j < cw_off + cw_len; ++j) {
                const u8 octet = dictionary[j % LZSS_DICTIONARY_SIZE];
                dictionary[dictionary_head] = octet;
                dictionary_head = (dictionary_head + 1) % LZSS_DICTIONARY_SIZE;
                data[i++] = octet;
            }
        }
    }

    return !bits.overrun();
}

//
// ANM animation parsing
//

bool read_anm(Span<const u8> file, Animation& anim) {
    anim = Animation();

    ByteStream bytes = ByteStream(file);
    const u32 num_sprites = bytes.read<u32>();  // nb_sprites
    const u32 num_scripts = bytes.read<u32>();  // nb_scripts
    bytes.read<u32>();  // ?
    bytes.read<u32>();  // width
    bytes.read<u32>();  // height
    bytes.read<u32>();  // fmt
    bytes.read<u32>();  // ?
    const u32 name_off = bytes.read<u32>();  // name_offset
    bytes.read<u32>();  // ?
    const u32 name2_off = bytes.read<u32>();  // secondary_name_offset
    anim.version = bytes.read<u32>(); ASSERT(anim.version == 0);
    bytes.read<u32>();  // ?
    bytes.read<u32>();  // texture_offset
    bytes.read<u32>();  // has_data
    bytes.read<u32>();  // next_offset
    bytes.read<u32>();  // ?
    Array<u32> sprite_offsets = Array<u32>(num_sprites);
    for (u32 i = 0; i < num_sprites; ++i) {
        sprite_offsets[i] = bytes.read<u32>();
    }
    Array<u32> script_offsets = Array<u32>(num_scripts);
    for (u32 i = 0; i < num_scripts; ++i) {
        script_offsets[i] = bytes.read<u32>();
        bytes.read<u32>(); // ?
    }
    std::printf("num_scripts: %u\n", num_scripts);
    ASSERT(name_off); bytes.seek(name_off);
    bytes.read(ARRAY_SPAN(anim.primary_name));
    if (name2_off) {
        bytes.seek(name2_off);
        bytes.read(ARRAY_SPAN(anim.secondary_name));
    }

    anim.sprites.reserve(num_sprites);
    for (auto offset : sprite_offsets) {
        std::printf("sprite offset: 0x%X\n", offset);
        bytes.seek(offset);
        const AnimationSprite sprite = {
            .idx = bytes.read<u32>(),
            .x   = bytes.read<f32>(),
            .y   = bytes.read<f32>(),
            .w   = bytes.read<f32>(),
            .h   = bytes.read<f32>(),
        };
        anim.sprites.append(sprite);
    }

    // this is fucked
#if 0
    for (auto& offset : script_offsets) {
        std::printf("OP:\n");
        bytes.seek(offset);
        // static u8 op_par[0xFF] = { };
        // read opcodes
        const u16 op_time = bytes.read<u16>();
        const u8 op_code = bytes.read<u8>();
        const u8 op_size = bytes.read<u8>();
        std::printf("op_time = %u\n", op_time);
        std::printf("op_code = %u\n", op_code);
        std::printf("op_size = %u\n", op_size);
    }
#endif

    return !bytes.overrun();
}

