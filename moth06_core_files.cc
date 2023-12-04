#include "moth06_core.hh"

//
// PBG archive parsing
//

bool pbg::read_entry_list(BitStream bits, Array<FileEntry>& entries) {
    const char magic[4] = {
        bits.read_bits<char>(),
        bits.read_bits<char>(),
        bits.read_bits<char>(),
        bits.read_bits<char>(),
    };

    static constexpr char PBG_MAGIC[] = { 'P', 'B', 'G', '3' };
    if (!mem::equal(magic, PBG_MAGIC, ARRLEN(PBG_MAGIC))) {
        return false;
    }

    const u32 etbl_num = bits.read_int();
    const u32 etbl_off = bits.read_int();

    bits.seek(etbl_off, 0);

    entries.resize(etbl_num);
    for (usize i = 0; i < entries.length(); ++i) {
        FileEntry& e = entries[i];
        e.e_unk1 = bits.read_int();
        e.e_unk2 = bits.read_int();
        e.e_chck = bits.read_int();
        e.e_foff = bits.read_int();
        e.e_fsiz = bits.read_int();
        bits.read_string(BufferView<char>(e.e_name, ARRLEN(e.e_name)));
    }

    return !bits.overrun();
}

bool pbg::read_entry_data(BitStream bits, const FileEntry& file, Array<u8>& data) {
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