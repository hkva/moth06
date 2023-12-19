#ifndef _HK_HH_
#define _HK_HH_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <inttypes.h>
#include <new>

//
// Platform detection
//

#ifdef _WIN32
#   define HK_WINDOWS
#   define HK_PLATFORM_NAME "Windows"
#endif

#ifdef __MACH__
#   define HK_MACOS
#   define HK_PLATFORM_NAME "MacOS"
#endif

#ifdef _MSC_VER
#   define HK_MSVC
#   define HK_COMPILER_NAME "MSVC"
#   pragma warning(disable: 4706) // Assignment within conditional expression
#endif

#ifdef __clang__
#   define HK_CLANG
#   define HK_COMPILER_NAME "clang"
#endif

//
// Macros
//

// Assert - always evaluates
#define HK_ASSERT(...) assert(__VA_ARGS__)

// Assert - only in debug builds
#define HK_DEBUG_ASSERT(...) assert(__VA_ARGS__)

#define HK_STRINGIFY_(x) #x
#define HK_STRINGIFY(x) HK_STRINGIFY_(x)


namespace hk {

//
// Core types
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

// Static c-style array length
template <typename T, usize S>
constexpr static inline usize arrlen(T(&)[S]) {
    return S;
}

template <typename T>
constexpr static inline T min(T left, T right) {
    return left < right ? left : right;
}

template <typename T>
constexpr static inline T max(T left, T right) {
    return left > right ? left : right;
}

//
// Memory utilities
//

namespace mem {

template <typename T>
static inline T* alloc(const usize count = 1) {
    HK_DEBUG_ASSERT(count);
#ifdef HK_ALLOC_TRACKER
    ++HK_ALLOC_TRACKER;
#endif
    return (T*)std::calloc(sizeof(T), count);
}

template <typename T>
static inline void free(T* ptr) {
#ifdef HK_ALLOC_TRACKER
    if (ptr) {
        --HK_ALLOC_TRACKER;
    }
#endif
    std::free((void*)ptr);
}

template <typename T>
static inline void copy(T* dst, const T* src, usize count = 1) {
    HK_DEBUG_ASSERT(dst && src && count);
    std::memcpy((void*)dst, (const void*)src, sizeof(T) * count);
}

}

//
// Linear memory iterator
//
template <typename T>
class Iterator {
private:
    T* m_ptr;
public:
    Iterator(T* ptr) : m_ptr(ptr) { }
    Iterator operator++() { ++m_ptr; return *this; }
    T& operator*() const { return *m_ptr; }
    bool operator!=(const Iterator& rhs) { return m_ptr != rhs.m_ptr; }
};

//
// Span
//
template <typename T>
class Span {
private:
    T*      m_buffer = nullptr;
    usize   m_length = 0;
public:
    Span() = default;
    Span(T* buffer, usize length) : m_buffer(buffer), m_length(length) { }

    T& operator[](usize idx)                { HK_DEBUG_ASSERT(idx < m_length); return m_buffer[idx]; }
    const T& operator[](usize idx) const    { HK_DEBUG_ASSERT(idx < m_length); return m_buffer[idx]; }

    Iterator<T> begin() { return Iterator(m_buffer); }
    Iterator<T> end()   { return Iterator(m_buffer + m_length); }

    T* buffer() { return m_buffer; }
    usize length() { return m_length; }

    Span<u8> bytes() const { return Span<u8>((u8*)m_buffer, sizeof(T) * m_length); }
    Span<const u8> const_bytes() const { return Span<const u8>((const u8*)m_buffer, sizeof(T) * m_length); }
};

template <typename T, usize S>
static Span<T> array_span(T(&arr)[S]) {
    return Span<T>(arr, S);
}

//
// Dynamic array
//
template <typename T>
class Array {
private:
    T*      m_buffer = nullptr;
    usize   m_length = 0;
    usize   m_capacity = 0;
public:
    Array() = default;
    Array(const Array<T>& other) { copy(other); }
    ~Array() { resize(0); mem::free(m_buffer); }

    Array<T>& operator=(const Array<T>& other) { copy(other); return *this; }

    T& operator[](usize idx) { HK_DEBUG_ASSERT(idx < m_length); return m_buffer[idx]; }
    const T& operator[](usize idx) const { HK_DEBUG_ASSERT(idx < m_length); return m_buffer[idx]; }

    Iterator<T> begin() { return Iterator(m_buffer); }
    Iterator<T> end() { return Iterator(m_buffer + m_length); }

    T* buffer() { return m_buffer; }
    usize length() { return m_length; }

    Span<u8> bytes() const { return Span<u8>((u8*)m_buffer, sizeof(T) * m_length); }
    Span<const u8> const_bytes() const { return Span<const u8>((const u8*)m_buffer, sizeof(T) * m_length); }

    void copy(const Array<T>& rhs) {
        resize(0);
        reserve(rhs.m_capacity);
        resize(rhs.m_length);
        for (usize i = 0; i < m_length; ++i) {
            (*this)[i] = rhs[i];
        }
    }

    void reserve(usize capacity) {
        if (capacity > m_capacity) {
            // NOTE(HK): After testing on multiple PC platforms, it seems
            // that f(x) = 2x seems to be the optimal allocation strategy
            capacity = max(capacity, m_capacity * 2);
            T* new_buffer = mem::alloc<T>(capacity); HK_DEBUG_ASSERT(new_buffer);
            if (m_length > 0) {
                mem::copy(new_buffer, m_buffer, m_length);
            }
            mem::free(m_buffer);
            m_buffer = new_buffer;
            m_capacity = capacity;
        }
    }

    void resize(usize length) {
        reserve(length);
        // grow
        if (length > m_length) {
            new(&m_buffer[m_length]) T[length - m_length];
        }
        // shrink
        else if (length < m_length) {
            for (usize i = length; i < m_length; ++i) {
                m_buffer[i].~T();
            }
        }
        m_length = length;
    }

    usize append(const T& val) {
        resize(m_length + 1);
        m_buffer[m_length - 1] = val;
        return m_length - 1;
    }
};

//
// Binary reader
//
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

    usize read_c_string(char* str, usize len) {
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
// System API
//

namespace sys {

// Get the CPU timestamp
u64 get_cpu_ticks();

}

}

#ifndef HK_KEEP_NAMESPACE
using namespace hk;
#endif

#endif // _HK_HH_
