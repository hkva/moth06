#include "moth06_common.hh"

#include <cstdio>

//
// Hashing
//

void hash::MD5Digest::render(char* str, usize len) {
    std::snprintf(str, len, "%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x",
        bytes[ 0], bytes[ 1], bytes[ 2], bytes[ 3],
        bytes[ 4], bytes[ 5], bytes[ 6], bytes[ 7],
        bytes[ 8], bytes[ 9], bytes[10], bytes[11],
        bytes[12], bytes[13], bytes[14], bytes[15]
    );
}

u32 hash::fnv(Span<const u8> data) {
    // https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
    // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
    constexpr u32 FNV_PRIME  = 0x01000193;
    constexpr u32 FNV_OFFSET = 0x811c9dc5;
    u32 result = FNV_OFFSET;
    for (usize i = 0; i < data.length(); ++i) {
        result ^= (u32)data[i];
        result *= FNV_PRIME;
    }
    return result;
}

u32 hash::fnv_string(const char* string) {
    // XXX(HK): Optimize for strings (avoid std::strlen() in BufferView::from_string())
    return hash::fnv(Span<const char>(string, str::length(string)).const_bytes());
}

hash::MD5Digest hash::md5(Span<const u8> data) {
    hash::MD5Digest result = { };

    // XXX(HK): Documentation overload because I don't understand this algorithm
    //          This is really slow, make it faster
    // https://en.wikipedia.org/wiki/MD5
    // https://github.com/B-Con/crypto-algorithms/blob/master/md5.c

    constexpr static u32 MD5_SHIFT_TABLE[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
    };

    constexpr static u32 MD5_SIN_TABLE[64] = {
        0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
        0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
        0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
        0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
        0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
        0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
        0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED,
        0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
        0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
        0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
        0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05,
        0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
        0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039,
        0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
        0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
        0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391,
    };

    // The MD5 algorithm expects data in 64-byte blocks. Data should be followed immediately by a "one" bit, then
    // padded with zeroes until the last 8 bytes of the last block, where the size of the input in bytes is written.
    // 
    // https://www.desmos.com/calculator/hypjdhc7v7
    Array<u8> inp = Array<u8>(); inp.resize((((data.length() + 8) / 64) + 1) * 64);
    
    // Set initial buffer
    mem::copy(inp.buffer(), data.buffer(), data.length());

    // Set "one" bit
    inp[data.length()] = 1 << 7;

    // Write size in bits
    const u64 size_bits = data.length() * 8;
    mem::copy(&inp[inp.length() - sizeof(u64)], (const u8*)&size_bits, sizeof(u64));

    // Set initial state
    u32 state_A = 0x67452301;
    u32 state_B = 0xEFCDAB89;
    u32 state_C = 0x98BADCFE;
    u32 state_D = 0x10325476;

    // Process 512-bit chunks
    ASSERT(inp.length() % 64 == 0);
    for (usize i = 0; i < inp.length() / 64; ++i) {
        u32 a = state_A;
        u32 b = state_B;
        u32 c = state_C;
        u32 d = state_D;

        // https://en.wikipedia.org/wiki/MD5#Algorithm
        auto F = [](u32 b, u32 c, u32 d) { return (b & c) | (~b & d); };
        auto G = [](u32 b, u32 c, u32 d) { return (b & d) | (c & ~d); };
        auto H = [](u32 b, u32 c, u32 d) { return b ^ c ^ d; };
        auto I = [](u32 b, u32 c, u32 d) { return c ^ (b | ~d); };
        for (u32 j = 0; j < 64; ++j) {
            u32 f = 0;
            u32 g = 0;
            if (j < 16) {
                f = F(b, c, d);
                g = j;
            }
            else if (j < 32) {
                f = G(b, c, d);
                g = (j * 5 + 1) % 16;
            }
            else if (j < 48) {
                f = H(b, c, d);
                g = (j * 3 + 5) % 16;
            }
            else {
                f = I(b, c, d);
                g = (j * 7) % 16;
            }
            f = f + a + MD5_SIN_TABLE[j] + ((u32*)&inp[i * 64])[g];
            a = d;
            d = c;
            c = b;
            b = b + rotate_left(f, MD5_SHIFT_TABLE[j]);
        }

        state_A += a;
        state_B += b;
        state_C += c;
        state_D += d;
    }

    const u32 result32[4] = {
        state_A,
        state_B,
        state_C,
        state_D,
    };
    mem::copy(result.bytes, (u8*)result32, ARRLEN(result.bytes));

    return result;
}

hash::MD5Digest hash::md5_string(const char* string) {
    return hash::md5(Span<const char>(string, str::length(string)).const_bytes());
}

