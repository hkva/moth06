#pragma once

#if defined(_WIN32)
#   define MOTH06_WINDOWS
#   define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(__APPLE__) && defined(__MACH__)
#   define MOTH06_OSX
#endif

#ifdef _MSC_VER
#   pragma warning(disable: 4706)   // assignment within conditional expression
#endif

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <imgui.h>

#define Assert(...) assert(__VA_ARGS__)

//
// Fixed-size fundamental types
//

using u8    = std::uint8_t;
using u16   = std::uint16_t;
using u32   = std::uint32_t;
using u64   = std::uint64_t;
using i8    = std::int8_t;
using i16   = std::int16_t;
using i32   = std::int32_t;
using i64   = std::int64_t;

using f32   = float;

using usize = std::size_t;

#define ArrLen(arr) (sizeof(arr) / sizeof((arr)[0]))

template <typename T>
inline T& Min(T& a, T& b) {
    return (a < b) ? a : b;
}

template <typename T>
inline T& Max(T& a, T& b) {
    return (a > b) ? a : b;
}

template <typename T>
inline T RotL(T val, usize shift) {
    return (val << shift) | (val >> ((sizeof(T) * 8) - shift));
}

//
// Memory helpers
//

namespace Mem {

template <typename T>
static inline T* Alloc(usize count = 1) {
    T* result = (T*)std::calloc(count, sizeof(T));
    Assert(result);
    return result;
}

static inline void Free(void* ptr) {
    return std::free(ptr);
}

template <typename T>
static inline void Copy(T* dst, const T* src, usize count = 1) {
    Assert(dst && src);
    std::memcpy((void*)dst, (const void*)src, sizeof(T) * count);
}

template <typename T>
static inline void Zero(T* buffer, usize count = 1) {
    Assert(buffer);
    std::memset((void*)buffer, 0x0, sizeof(T) * count);
}

}

//
// String helpers
//

namespace Str {

static inline bool BaseName(char* path) {
    char* delim = nullptr;
    for (usize i = 0; path[i]; ++i) {
#ifdef MOTH06_WINDOWS
        if (path[i] == '\\') {
            delim = &path[i];
        }
#else
        if (path[i] == '/') {
            delim = &path[i];
        }
#endif
    }
    if (delim) {
        *delim = '\0';
    }
    return delim != nullptr;
}

static inline bool Equal(const char* s1, const char* s2) {
    return !std::strcmp(s1, s2);
}

static inline const char* Extension(const char* str) {
    const char* last_period = NULL;
    for (usize i = 0; str[i] != '\0'; ++i) {
        if (str[i] == '.' && str[i + 1] != '\0') {
            last_period = &str[i];
        }
    }
    return (last_period) ? &last_period[1] : nullptr;
}

static inline bool StartsWith(const char* haystack, const char* needle) {
    return !std::strncmp(haystack, needle, std::strlen(needle));
}

static inline usize Length(const char* str) {
    return strlen(str);
}

};

//
// Buffer view
//

// std::span replacement
// aka std::vector but doesn't own memory
template <typename T>
class BufferViewOf {
public:
    T* buf = nullptr;
    usize   len = 0;
public:
    BufferViewOf() = default;
    BufferViewOf(T* buf, usize len) : buf(buf), len(len) { }

    T* Buffer() { return this->buf; }
    usize Length() { return this->len; }
    usize LengthInBytes() { return len * sizeof(T); }
};

class BufferView : public BufferViewOf<u8> {
public:
    BufferView() = default;
    BufferView(u8* buf, usize len) : BufferViewOf<u8>(buf, len) { }
};

//
// Hashing
//

namespace Hash {

u32 FNV(BufferView view);
u32 FNVString(const char* str);

// 128-bit MD5 hash
class MD5Digest {
public:
    u8 m_bytes[128 / 8];
public:
    inline void Render(BufferViewOf<char> buf) {
        std::snprintf(buf.buf, buf.len, "%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x",
            m_bytes[ 0], m_bytes[ 1], m_bytes[ 2], m_bytes[ 3],
            m_bytes[ 4], m_bytes[ 5], m_bytes[ 6], m_bytes[ 7],
            m_bytes[ 8], m_bytes[ 9], m_bytes[10], m_bytes[11],
            m_bytes[12], m_bytes[13], m_bytes[14], m_bytes[15]
        );
    }
};

MD5Digest MD5(BufferView view);
MD5Digest MD5String(const char* str);

};

//
// Containers
//

// dumb std::vector replacement
// new elements are set to zero, no constructors called
// no assignmen operators called on elements outside of Append()
template <typename T>
class Array : public BufferViewOf<T> {
private:
    usize   m_cap = 0;
public:
    Array() : BufferViewOf<T>() { }
    Array(const Array<T>& ref) : Array() { *this = ref; }
    Array(usize len) : Array() { Resize(len); }
    ~Array() { Mem::Free(this->buf); }

    Array<T>& operator=(const Array<T>& rhs) {
        Resize(rhs.len);
        Mem::Copy(this->buf, rhs.buf, this->len);
        return *this;
    }

    T& operator[](usize idx) { Assert(idx < this->len); return this->buf[idx]; }

    BufferView View() { return BufferView((u8*)this->buf, this->LengthInBytes()); }
    BufferViewOf<T> ViewOf() { return *this; }

    void Resize(usize length) {
        Reserve(length);
        this->len = length;
    }

    void Reserve(usize capacity) {
        if (capacity > m_cap) {
            T* new_buf = Mem::Alloc<T>(capacity);
            Mem::Zero(new_buf, capacity);
            if (this->len > 0) {
                Mem::Copy(new_buf, this->buf, this->len);
            }
            Mem::Free(this->buf);
            this->buf = new_buf;
            m_cap = capacity;
        }
    }

    void ReserveMore(usize more_capacity) {
        Reserve(m_cap + more_capacity);
    }

    usize Append(const T& val) {
        const usize head = this->Length();
        Resize(head + 1);
        this->buf[head] = val;
        return head;
    }
};

constexpr usize HASH_TABLE_BAD_INDEX = SIZE_MAX;

// Hash table
template <typename T>
class HashTable {
private:
    struct Pair {
        u32 key_hash;
        T   value;
    };
private:
    Array<Pair> m_pairs;
public:
    HashTable() = default;
    ~HashTable() = default; // XXX(HK): Needed to call base destructor? Not good at C++

    T& operator[](const char* key) {
        const u32 key_hash = Hash::FNVString(key);
        usize idx = HASH_TABLE_BAD_INDEX;
        if ((idx = Find(key_hash)) == HASH_TABLE_BAD_INDEX) {
            Pair pair = { .key_hash = key_hash, .value = T(), };
            idx = m_pairs.Length();
            m_pairs.Append(pair);
        }
        return m_pairs[idx].value;
    }

    bool Has(const char* key) {
        return Find(Hash::FNVString(key)) != HASH_TABLE_BAD_INDEX;
    }

private:
    usize Find(u32 key_hash) {
        // XXX(HK): Linear search good enough?
        for (usize i = 0; i < m_pairs.Length(); ++i) {
            if (m_pairs[i].key_hash == key_hash) {
                return m_pairs[i].value;
            }
        }
        return HASH_TABLE_BAD_INDEX;
    }
};

// Bit sequence reader
class BitStream {
private:
    BufferView  m_data = { };
    usize       m_byte = 0;
    usize       m_bit = 0;
    bool        m_overrun = false;
public:
    BitStream(BufferView data) : m_data(data) { }

    void Seek(usize byte_offset) {
        m_byte  = byte_offset;
        m_bit   = 0;
    }

    bool Overrun() { return m_overrun; }

    template <typename T = u32>
    T ReadBits(usize num_bits = sizeof(T) * 8) {
        T result = T();
        if ((m_overrun |= (m_byte >= m_data.len))) {
            return T();
        }
        for (usize i = 0; i < num_bits; ++i) {
            result <<= 1;
            result |= (m_data.buf[m_byte] >> (7 - m_bit)) & 0x1;
            if (++m_bit >= 8) {
                m_bit = 0;
                if ((m_overrun |= (++m_byte >= m_data.len && i + 1 != num_bits))) {
                    return T();
                }
            }
        }
        return result;
    }

    u32 ReadInt() {
        const usize extra_bytes = ReadBits<usize>(2);
        return ReadBits((1 + extra_bytes) * 8);
    }

    usize ReadUTF8(char* out, usize out_len) {
        usize n = 0;
        for (; n < out_len && !m_overrun; ++n) {
            if ((out[n] = ReadBits<char>()) == 0) {
                break;
            }
        }
        return n;
    }
};

//
// PBG3 archives
//

namespace PBG {

struct Entry {
    u32 e_unk1; // unknown
    u32 e_unk2; // unknown
    u32 e_chck; // checksum
    u32 e_fpos; // file offset
    u32 e_size; // uncompressed size
    char e_name[256]; // file name
};

bool ReadEntries(BitStream& bits, Array<Entry>& entries);
bool ReadEntryContents(BitStream& bits, const Entry& entry, Array<u8>& data);

};

//
// Shared application code
//

static inline void Die(const char* fmt, ...) {
    char msg[512] = { 0 };
    va_list va; va_start(va, fmt);
    vsnprintf(msg, sizeof(msg), fmt, va);
    va_end(va);
    fprintf(stderr, "Error: %s\n", msg);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", msg, NULL);
    Assert(0);
}

//
// Filesystem
//

struct File;

namespace Fs {

bool Mount(const char* archive_path);
File* Read(const char* path);
bool ReadDisk(const char* path, Array<u8>& data);
bool Write(const char* path, BufferView data);

}

//
// Graphics
//

namespace Gfx {

using Texture = u32;

enum {
    BACKEND_AUTO = 0,
    BACKEND_SDL_RENDERER
};

struct InitParams {
    u8 backend;
    SDL_Window* wnd;
};

void Init(const InitParams& par);
void UIEvent(const SDL_Event* evt);
void BeginFrame(void);
void EndFrame(void);

Texture UploadTexture(u32* rgba, u32 width, u32 height);
void* GetTextureDescriptor(Texture texture);
void DebugDrawFullscreenTexture(Texture texture);

}

//
// Game code
//

enum {
    APP_STATE_WANTS_QUIT = 1 << 0,
};

enum {
    CLI_OPT_DUMP_FILES = 1 << 0,
};

enum {
    DEBUG_FLAG_UI = 1 << 0,
};

enum {
    DEBUG_SCREEN_NONE = 0,
    DEBUG_SCREEN_FILES,
    DEBUG_SCREEN_TEST
};

enum {
    ASSET_TYPE_NONE = 0,
    ASSET_TYPE_IMAGE,
};

struct File {
    PBG::Entry  entry;
    Array<u8>   data;
    u8          asset_type;
    union {
        struct {
            u32 width; u32 height;
            Gfx::Texture tex;
        } image;
    };
};

struct Client {
    struct {
        u8  state;  // Current application control state
        u8  cli;    // Command-line options
    } app;
    struct {
        u8  flags;  // Current debug options
        u8  screen; // Current debug screen
    } dbg;
    struct {
        // Array
        Array<File> files; // XXX(HK): Does this corrupt memory because of how shitty Array<> is?
    } fs;
};
extern Client cl;
