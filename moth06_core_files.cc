#include "moth06_common.hh"
#include "moth06_core.hh"

//
// PBG archive parsing
//

bool PBG::ReadEntryList(BitStream bits, Array<FileEntry>& entries) {
    const char magic[4] = {
        bits.ReadBits<char>(),
        bits.ReadBits<char>(),
        bits.ReadBits<char>(),
        bits.ReadBits<char>(),
    };

    static constexpr char PBG_MAGIC[] = { 'P', 'B', 'G', '3' };
    if (!Mem::Equal(magic, PBG_MAGIC, ARRLEN(PBG_MAGIC))) {
        return false;
    }

    const u32 etbl_num = bits.ReadInt();
    const u32 etbl_off = bits.ReadInt();

    bits.Seek(etbl_off, 0);

    entries.Resize(etbl_num);
    for (usize i = 0; i < entries.Length(); ++i) {
        FileEntry& e = entries[i];
        e.e_unk1 = bits.ReadInt();
        e.e_unk2 = bits.ReadInt();
        e.e_chck = bits.ReadInt();
        e.e_foff = bits.ReadInt();
        e.e_fsiz = bits.ReadInt();
        bits.ReadString(e.e_name, ARRLEN(e.e_name));
    }

    return !bits.IsOverrun();
}

bool PBG::ReadEntryData(BitStream bits, const FileEntry& file, Array<u8>& data) {
    // Touhou-specific LZSS encoding options
    // XXX(HK): These are copy/pasted from PyTouhou, confirm these
    constexpr usize LZSS_DICTIONARY_SIZE    = 0x2000;
    constexpr u32   LZSS_MIN_MATCH_LENGTH   = 3;
    constexpr usize LZSS_OFFSET_BITS        = 13;
    constexpr usize LZSS_LENGTH_BITS        = 4;

    data.Resize(file.e_fsiz);
    // LZSS decompression
    bits.Seek(file.e_foff, 0);
    Array<u8> dictionary = Array<u8>(LZSS_DICTIONARY_SIZE);
    usize dictionary_head = 1;
    for (usize i = 0; i < data.Length(); ) {
        if (bits.ReadBits(1)) {
            // literal word
            const u8 octet = bits.ReadBits<u8>();
            dictionary[dictionary_head] = octet;
            dictionary_head = (dictionary_head + 1) % LZSS_DICTIONARY_SIZE;
            data[i++] = octet;
        } else {
            // control word
            const u32 cw_off = bits.ReadBits(LZSS_OFFSET_BITS);
            const u32 cw_len = bits.ReadBits(LZSS_LENGTH_BITS) + LZSS_MIN_MATCH_LENGTH;
            for (u32 j = cw_off; j < cw_off + cw_len; ++j) {
                const u8 octet = dictionary[j % LZSS_DICTIONARY_SIZE];
                dictionary[dictionary_head] = octet;
                dictionary_head = (dictionary_head + 1) % LZSS_DICTIONARY_SIZE;
                data[i++] = octet;
            }
        }
    }

    return !bits.IsOverrun();
}