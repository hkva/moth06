static unsigned long hk_alloc_tracker = 0;
#define HK_ALLOC_TRACKER hk_alloc_tracker
#define HK_KEEP_NAMESPACE

#include "test.hh"
#include "hk.hh"

#include <vector>

#define CHECK_LEAKS() HK_ASSERT(hk_alloc_tracker == 0 && "Memory leaked!")

class DummyClass {
private:
    hk::i32* m_dummy;
public:
    DummyClass() {
        m_dummy = hk::mem::alloc<hk::i32>();
        *m_dummy = 1234;
    }
    ~DummyClass() {
        hk::mem::free(m_dummy);
    }
    DummyClass& operator=(const DummyClass& rhs) {
        HK_ASSERT(m_dummy);
        *m_dummy = *rhs.m_dummy;
        return *this;
    }
private:
    DummyClass(DummyClass&) = delete;
    DummyClass(const DummyClass&) = delete;
    DummyClass& operator=(DummyClass&) = delete;
};

int main() {
    {
        static hk::i32 test_arr[10] = { };
        HK_ASSERT(hk::arrlen(test_arr) == 10);
    }

    // Span<T>
    {
        static hk::i32 test_arr[5] = { 1, 2, 3, 4, 5 };
        hk::Span<hk::i32> test_span = hk::array_span(test_arr);
        HK_ASSERT(test_span.length() == hk::arrlen(test_arr));
        hk::usize idx = 0;
        for (auto& i : test_span) {
            HK_ASSERT(i == test_arr[idx++]);
        }
    }

    // Array<T>
    {
        {
            hk::Array<hk::i32> test_array = hk::Array<hk::i32>();
            for (hk::usize i = 0; i < 100; ++i) {
                test_array.append(i % 10);
            }
            hk::usize idx = 0;
            for (auto i : test_array) {
                HK_ASSERT(i == idx++ % 10);
            }
        }
        CHECK_LEAKS();

        {
            hk::Array<DummyClass> test_array = hk::Array<DummyClass>();
            test_array.reserve(100);
            test_array.resize(50);
            test_array.append(DummyClass());
        }
        CHECK_LEAKS();

        {
            hk::Array<hk::Array<DummyClass>> test_array = hk::Array<hk::Array<DummyClass>>();
            for (hk::usize i = 0; i < 100; ++i) {
                hk::Array<DummyClass> test_array2 = hk::Array<DummyClass>();
                test_array2.resize(std::rand() % 100);
                test_array.append(test_array2);
            }
        }
        CHECK_LEAKS();

        // Check performance vs. std::vector
        {
            hk::u64 base_time = 0;
            {
                const hk::u64 t1 = hk::sys::get_cpu_ticks();
                for (hk::i32 i = 1; i <= 1000; ++i) {
                    std::vector<hk::i32> test_vec = std::vector<hk::i32>();
                    for (hk::i32 j = 1; j <= 1000; ++j) {
                        test_vec.push_back(i % j);
                    }
                }
                const hk::u64 t2 = hk::sys::get_cpu_ticks();
                base_time = t2 - t1;
                std::printf("std::vector benchmark took %" PRIu64 " cycles\n", base_time);
            }
            {
                const hk::u64 t1 = hk::sys::get_cpu_ticks();
                for (hk::i32 i = 1; i <= 1000; ++i) {
                    hk::Array<hk::i32> test_vec = hk::Array<hk::i32>();
                    for (hk::i32 j = 1; j <= 1000; ++j) {
                        test_vec.append(i % j);
                    }
                }
                const hk::u64 t2 = hk::sys::get_cpu_ticks();
                const hk::u64 dt = t2 - t1;
                std::printf("Array<T> benchmark took     %" PRIu64 " cycles (%.2f%% execution time)\n", dt, ((hk::f64)dt / (hk::f64)base_time) * 100.0f);
            }
        }
        CHECK_LEAKS();
    }

    // BitStream
    {
        const hk::u8 buffer[1] = { 0b01011101 };
        hk::BitStream bits = hk::BitStream(hk::array_span(buffer).const_bytes());
        HK_ASSERT(bits.read_bits(1) == 0);
        HK_ASSERT(bits.read_bits(1) == 1);
        HK_ASSERT(bits.read_bits(1) == 0);
        HK_ASSERT(bits.read_bits(1) == 1);
        HK_ASSERT(bits.read_bits(2) == 3);
        HK_ASSERT(bits.read_bits(1) == 0);
        HK_ASSERT(bits.read_bits(1) == 1);
        HK_ASSERT(!bits.overrun());
        bits.read_bits(1);
        HK_ASSERT(bits.overrun());
    }
    CHECK_LEAKS();
}