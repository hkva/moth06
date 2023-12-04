#include "moth06.hh"

static const char PBG_MAGIC[] = { 'P', 'B', 'G', '3' };

bool PBG::ReadEntries(BitStream& bits, Array<Entry>& entries) {
    const char magic[4] = {
        bits.ReadBits<char>(),
        bits.ReadBits<char>(),
        bits.ReadBits<char>(),
        bits.ReadBits<char>(),
    };
    if (memcmp(magic, PBG_MAGIC, sizeof(PBG_MAGIC)) != 0) {
        return 0;
    }

    const u32 etbl_num = bits.ReadInt();
    const u32 etbl_off = bits.ReadInt();

    bits.Seek(etbl_off);

    entries.Resize(etbl_num);
    for (u32 i = 0; i < etbl_num; ++i) {
        Entry& e = entries[i];
        e.e_unk1 = bits.ReadInt();
        e.e_unk2 = bits.ReadInt();
        e.e_chck = bits.ReadInt();
        e.e_fpos = bits.ReadInt();
        e.e_size = bits.ReadInt();
        bits.ReadUTF8(e.e_name, sizeof(e.e_name));
    }

    return !bits.Overrun();
}

// Touhou-specific LZSS encoding options
// XXX(HK): These are copy/pasted from PyTouhou, confirm these
#define LZSS_DICTIONARY_SIZE    0x2000
#define LZSS_MIN_MATCH_LENGTH   3
#define LZSS_OFFSET_BITS        13
#define LZSS_LENGTH_BITS        4

bool PBG::ReadEntryContents(BitStream& bits, const Entry& entry, Array<u8>& data) {
    data.Resize(entry.e_size);
    // LZSS decompression
    bits.Seek(entry.e_fpos);
    Array<u8> dictionary = Array<u8>(LZSS_DICTIONARY_SIZE);
    usize dictionary_head = 1;
    for (usize i = 0; i < data.Length(); ) {
        if (bits.ReadBits(1)) {
            // Literal world
            const u8 octet = bits.ReadBits<u8>();
            dictionary[dictionary_head] = octet;
            dictionary_head = (dictionary_head + 1) % LZSS_DICTIONARY_SIZE;
            data[i++] = octet;
        } else {
            // Control word
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
    return !bits.Overrun();
}
