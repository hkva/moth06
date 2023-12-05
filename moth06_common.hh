#ifndef _MOTH06_COMMON_HH_
#define _MOTH06_COMMON_HH_

#if defined(_WIN32)
#   define MOTH06_WINDOWS
#   define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(__APPLE__) && defined(__MACH__)
#   define MOTH06_OSX
#endif

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>

//
// Macros
//

#define STRINGIFY_(x)   #x
#define STRINGIFY(x)    (STRINGIFY_(x))

#define ASSERT(...) assert(__VA_ARGS__)
#define ASSERT_NOT_REACHED() ASSERT(0 && "invalid location reached!")

//
// Fundamental types
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
using f64   = double;

using usize = std::size_t;

#define ARRLEN(arr) (sizeof(arr) / sizeof((arr)[0]))

template <typename T>
static inline T rotate_left(T val, usize shift) {
    return (val << shift) | (val >> ((sizeof(T) * 8) - shift));
}

template <typename T>
static inline T next_power_of_2(T val) {
    return (val << 1) & ~val;
}

template <typename T>
static inline T min(const T& v1, const T& v2) {
    return (v1 < v2) ? v1 : v2;
}

template <typename T>
static inline T max(const T& v1, const T& v2) {
    return (v1 > v2) ? v1 : v2;
}

//
// Memory helpers
//

namespace mem {

template <typename T>
static inline T* alloc(usize count = 1) {
    ASSERT(count);
#ifdef MOTH06_ALLOC_TRACKER
    MOTH06_ALLOC_TRACKER += 1;
#endif
    return (T*)std::calloc(sizeof(T), count);
}

template <typename T>
static inline void free(T* ptr) {
#ifdef MOTH06_ALLOC_TRACKER
    if (ptr) {
        MOTH06_ALLOC_TRACKER -= 1;
    }
#endif
    std::free((void*)ptr);
}

template <typename T>
static inline void copy(T* dst, const T* src, usize count = 1) {
    ASSERT(dst && src && count);
    std::memcpy((void*)dst, (const void*)src, sizeof(T) * count);
}

template <typename T>
static inline bool equal(const T* mem1, const T* mem2, usize count = 1) {
    ASSERT(mem1 && mem2 && count);
    return !std::memcmp((const void*)mem1, (const void*)mem2, sizeof(T) * count);
}

}

//
// String helpers
//

namespace str {

static inline bool equal(const char* s1, const char* s2) {
    return !std::strcmp(s1, s2);
}

static inline usize length(const char* str) {
    return std::strlen(str);
}

}

//
// Containers
//

// std::span replacement
template <typename T>
class Span {
public:
    class Iterator {
    private:
        T* m_ptr;
    public:
        Iterator(T* ptr) : m_ptr(ptr) { }
        Iterator operator++() { ++m_ptr; return *this; }
        bool operator!=(const Iterator& rhs) const { return m_ptr != rhs.m_ptr; }
        T& operator*() const { return *m_ptr; }
    };
protected:
    T*      m_buffer = nullptr;
    usize   m_length = 0;
public:
    Span() = default;
    Span(T* buffer, usize length) : m_buffer(buffer), m_length(length) { }

    T& operator[](usize idx)                { ASSERT(idx < m_length); return m_buffer[idx]; }
    const T& operator[](usize idx) const    { ASSERT(idx < m_length); return m_buffer[idx]; }

    T* buffer() { return m_buffer; }
    usize length() { return m_length; }

    Iterator begin()    { return Iterator(m_buffer); }
    Iterator end()      { return Iterator(m_buffer + m_length); }

    // Raw byte view
    Span<u8> bytes() const { return Span<u8>((u8*)m_buffer, sizeof(T) * m_length); }
    // Raw byte view (const)
    Span<const u8> const_bytes() const { return Span<const u8>((const u8*)m_buffer, sizeof(T) * m_length); }
};

// Describes how containers allocate memory as they grow
enum class AllocationStrategy {
    Linear,
    Double,
    Exponential,
};

// std::vector replacement
template <typename T>
class Array : public Span<T> {
private:
    usize               m_capacity = 0;
    AllocationStrategy  m_strategy = AllocationStrategy::Exponential;
public:
    Array(AllocationStrategy strategy = AllocationStrategy::Exponential) : Span<T>(), m_strategy(strategy) { }
    Array(usize size, AllocationStrategy strategy = AllocationStrategy::Exponential) : Span<T>(), m_strategy(strategy) { resize(size); }
    Array(const Array<T>& other) { copy(other); }
    ~Array() { resize(0); mem::free(this->m_buffer); }

    Array<T>& operator=(const Array<T>& other) { copy(other); return *this; }

    void copy(const Array<T>& other) {
        resize(0);
        reserve(other.m_capacity);
        resize(other.m_length);
        for (usize i = 0; i < this->m_length; ++i) {
            (*this)[i] = other[i];
        }
    }

    void reserve(usize capacity) {
        if (capacity > m_capacity) {
            switch (m_strategy) {
                case AllocationStrategy::Linear: break;
                case AllocationStrategy::Double: { capacity *= 2; }; break;
                case AllocationStrategy::Exponential: { capacity = next_power_of_2(capacity); }; break;
                default: ASSERT_NOT_REACHED();
            }
            T* new_buffer = mem::alloc<T>(capacity);
            if (this->m_length > 0) {
                mem::copy(new_buffer, this->m_buffer, this->m_length);
            }
            mem::free(this->m_buffer);
            this->m_buffer = new_buffer;
            m_capacity = capacity;
        }
    }

    void resize(usize length) {
        reserve(length);
        // grow
        if (length > this->m_length) {
            new(&this->m_buffer[this->m_length]) T[length - this->m_length];
        }
        // shrink
        else if (length < this->m_length) {
            for (usize i = length; i < this->m_length; ++i) {
                this->m_buffer[i].~T();
            }
        }
        this->m_length = length;
    }

    usize append(const T& val) {
        resize(this->m_length + 1);
        this->m_buffer[this->m_length - 1] = val;
        return this->m_length - 1;
    }
};

// Binary reader
class BitStream {
private:
    Span<const u8>  m_bytes = { };
    usize           m_cur_byte = 0;
    usize           m_cur_bit = 0;
    bool            m_overrun = false;
public:
    BitStream() = default;
    BitStream(Span<const u8> bytes) : BitStream() { m_bytes = bytes; }

    bool overrun() { return m_overrun; }

    void seek(usize byte_off, usize bit_off) {
        m_cur_byte = byte_off;
        m_cur_bit = bit_off;
    }

    template <typename T = u32>
    T read_bits(usize num_bits = sizeof(T) * 8) {
        T result = T();
        if ((m_overrun |= (m_cur_byte >= m_bytes.length()))) {
            return T();
        }
        for (usize i = 0; i < num_bits; ++i) {
            result <<= 1;
            result |= (m_bytes[m_cur_byte] >> (7 - m_cur_bit)) & 0x1;
            if (++m_cur_bit >= 8) {
                m_cur_bit = 0;
                if ((m_overrun |= (++m_cur_byte >= m_bytes.length() && i + 1 != num_bits))) {
                    return T();
                }
            }
        }
        return result;
    }

    // Touhou-specific integer encoding
    u32 read_int() {
        const usize extra_bytes = read_bits<usize>(2);
        return read_bits((1 + extra_bytes) * 8);
    }

    // Read a null-terminated ASCII string
    usize read_string(char* str, usize len) {
        usize n = 0;
        for (; n < len; ++n) {
            if ((str[n] = read_bits<char>()) == '\0') {
                break;
            }
        }
        return n;
    }
};

//
// Hashing
//

namespace hash {

// 128-bit MD5 digest
class MD5Digest {
public:
    u8 bytes[16];
public:
    void render(char* str, usize len);
};

u32 fnv(Span<const u8> data);
u32 fnv_string(const char* string);

MD5Digest md5(Span<const u8> data);
MD5Digest md5_string(const char* string);

}

template <typename T>
class HashMap {
private:
    struct Entry {
        u32 e_hash;
        T   e_val;
    };
private:
    static constexpr usize BAD_INDEX = ~(usize)0;
private:
    Array<Entry> m_entries;
public:
    HashMap() = default;
    ~HashMap() = default;

    T& operator[](const char* key) { return get(key); }

    T& get(const char* key) {
        const u32 key_hash = hash::fnv_string(key);
        usize idx = index_of(key_hash);
        if (idx == BAD_INDEX) {
            idx = m_entries.append({
                .e_hash = key_hash,
                .e_val = T(),
            });
        }
        return m_entries[idx].e_val;
    }

    bool has(const char* key) {
        return index_of(hash::fnv_string(key)) != BAD_INDEX;
    }
private:
    // Returns BAD_INDEX on failure
    usize index_of(u32 key_hash) {
        for (usize i = 0; i < m_entries.length(); ++i) {
            if (m_entries[i].e_hash == key_hash) {
                return i;
            }
        }
        return BAD_INDEX;
    }
};

#endif // _MOTH06_COMMON_HH_
