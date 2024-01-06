#include "game.hh"
#include "game_private.hh"

const EngineInterface* ei = nullptr;
#define dbgmsg(...) ei->dbg_log(__VA_ARGS__)

// PBG3 parsing

static int pbg_read_int( BitStream& b ) {
    // Touhou-specific integer encoding
    // see /docs/fileformats.md
    const usize extra_bytes = b.read_bits<usize>( 2 );
    return b.read_bits( (1 + extra_bytes) * 8 );
}

static bool pbg_parse_entries( Span<const u8> archive, Array<PBGEntry>& entries ) {
    BitStream bits = BitStream( archive );
    const char magic[4] = {
        (char)pbg_read_int( bits ),
        (char)pbg_read_int( bits ),
        (char)pbg_read_int( bits ),
        (char)pbg_read_int( bits ),
    };

    static constexpr char PBG_MAGIC[] = { 'P', 'B', 'G', '3' };
    if ( !mem::equal( magic, PBG_MAGIC, arrlen( PBG_MAGIC ) ) ) {
        dbgmsg( "Got invalid PBG file header" );
        return false;
    }

    const u32 etbl_num = pbg_read_int( bits );
    const u32 etbl_off = pbg_read_int( bits );

    bits.seek( etbl_off, 0 );

    entries.resize( etbl_num );
    for ( usize i = 0; i < entries.length(); ++i ) {
        PBGEntry& e = entries[i];
        e.e_unk1 = pbg_read_int( bits );
        e.e_unk2 = pbg_read_int( bits );
        e.e_chck = pbg_read_int( bits );
        e.e_foff = pbg_read_int( bits );
        e.e_fsiz = pbg_read_int( bits );
        bits.read_c_string( e.e_name, arrlen( e.e_name ) );
    }

    return !bits.overrun();
}

static bool pbg_decompress_data( Span<const u8> archive, const PBGEntry& file, Array<u8>& data ) {
    BitStream bits = BitStream( archive );
    // Touhou-specific LZSS encoding options
    // XXX(HK): These are copy/pasted from PyTouhou, confirm these
    constexpr usize LZSS_DICTIONARY_SIZE = 0x2000;
    constexpr u32   LZSS_MIN_MATCH_LENGTH = 3;
    constexpr usize LZSS_OFFSET_BITS = 13;
    constexpr usize LZSS_LENGTH_BITS = 4;

    data.resize( file.e_fsiz );
    // LZSS decompression
    bits.seek( file.e_foff, 0 );
    Array<u8> dictionary = Array<u8>( LZSS_DICTIONARY_SIZE );
    usize dictionary_head = 1;
    for ( usize i = 0; i < data.length(); ) {
        if ( bits.read_bits( 1 ) ) {
            // literal word
            const u8 octet = bits.read_bits<u8>();
            dictionary[dictionary_head] = octet;
            dictionary_head = (dictionary_head + 1) % LZSS_DICTIONARY_SIZE;
            data[i++] = octet;
        }
        else {
            // control word
            const u32 cw_off = bits.read_bits( LZSS_OFFSET_BITS );
            const u32 cw_len = bits.read_bits( LZSS_LENGTH_BITS ) + LZSS_MIN_MATCH_LENGTH;
            for ( u32 j = cw_off; j < cw_off + cw_len; ++j ) {
                const u8 octet = dictionary[j % LZSS_DICTIONARY_SIZE];
                dictionary[dictionary_head] = octet;
                dictionary_head = (dictionary_head + 1) % LZSS_DICTIONARY_SIZE;
                data[i++] = octet;
            }
        }
    }

    return !bits.overrun();
}

extern "C" HK_DLL_EXPORT bool connect_game(const EngineInterface* ei_, GameInterface* gi) {
    ei = ei_;
    // Cannot reload if structure layout changed
    if (ei->size != sizeof(*ei) || gi->size != sizeof(*gi)) {
        return false;
    }

    gi->pbg.parse_entries = pbg_parse_entries;
    gi->pbg.decompress_data = pbg_decompress_data;

    ei->dbg_log("Game connected");
    return true;
}
