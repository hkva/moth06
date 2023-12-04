#include "moth06_common.hh"
#include "tests.hh"

#include <vector>

class DummyClass {
private:
    int* m_dummy;
public:
    DummyClass() {
        m_dummy = mem::alloc<int>();
        *m_dummy = 1234;
    }
    ~DummyClass() {
        mem::free(m_dummy);
    }
    DummyClass& operator=(const DummyClass& rhs) {
        ASSERT(m_dummy);
        *m_dummy = *rhs.m_dummy;
        return *this;
    }
private:
    DummyClass(DummyClass&)                     = delete;
    DummyClass(const DummyClass&)               = delete;
    DummyClass& operator=(DummyClass&)          = delete;
};

int main(void) {
    {
        ASSERT(next_power_of_2(3) == 4);
    }
    CHECK_LEAKS();

    {
        int test_array[4] = { 5, 10, 15, 20 };
        Span<int> test_view = Span<int>(test_array, ARRLEN(test_array));
        Span<const int> const_test_view = Span<const int>(test_array, ARRLEN(test_array));

        Span<u8> test_view_bytes = test_view.bytes();
        const Span<const u8> const_test_view_bytes = test_view.const_bytes();

        test_view_bytes[0];
        const_test_view_bytes[0];
    }
    CHECK_LEAKS();

    {
        Array<int> test_array = Array<int>();
        for (int i = 0; i < 100; ++i) {
            test_array.append(i % 10);
        }
    }
    CHECK_LEAKS();

    {
        Array<DummyClass> test_array = Array<DummyClass>();
        test_array.reserve(100);
        test_array.resize(50);
        test_array.append(DummyClass());
    }
    CHECK_LEAKS();

    {
        Array<Array<DummyClass>> test_array = Array<Array<DummyClass>>();
        for (usize i = 0; i < 100; ++i) {
            Array<DummyClass> test_array2 = Array<DummyClass>();
            test_array2.resize(std::rand() % 100);
            test_array.append(test_array2);
        }
    }

    {
        u64 then = tests::now();
        for (u32 i = 0; i < 1000; ++i) {
            std::vector<u32> test_vec = std::vector<u32>();
            for (u32 j = 0; j < 1000; ++j) {
                test_vec.push_back(i % j);
            }
        }
        CHECK_LEAKS();
        u64 base_time = tests::now() - then;
        std::printf("std::vector bench:         %lluns\tN/A\n", base_time);
        then = tests::now();
        for (u32 i = 0; i < 1000; ++i) {
            Array<u32> test_arr = Array<u32>(AllocationStrategy::Linear);
            for (u32 j = 0; j < 1000; ++j) {
                test_arr.append(i % j);
            }
        }
        CHECK_LEAKS();
        u64 t = tests::now() - then;
        std::printf("Array (f(x) = x) bench:    %lluns\t%.02f%%\n", t, (f64)t / (f64)base_time * 100.0f);
        then = tests::now();
        for (u32 i = 0; i < 1000; ++i) {
            Array<u32> test_arr = Array<u32>(AllocationStrategy::Double);
            for (u32 j = 0; j < 1000; ++j) {
                test_arr.append(i % j);
            }
        }
        CHECK_LEAKS();
        t = tests::now() - then;
        std::printf("Array (f(x) = 2x) bench:   %lluns\t%.02f%%\n", t, (f64)t / (f64)base_time * 100.0f);
        then = tests::now();
        for (u32 i = 0; i < 1000; ++i) {
            Array<u32> test_arr = Array<u32>(AllocationStrategy::Double);
            for (u32 j = 0; j < 1000; ++j) {
                test_arr.append(i % j);
            }
        }
        CHECK_LEAKS();
        t = tests::now() - then;
        std::printf("Array (f(x) = 2^x) bench:  %lluns\t%.02f%%\n", t, (f64)t / (f64)base_time * 100.0f);
    }
    CHECK_LEAKS();

    {
        const u8 buffer[1] = { 0b01011101 };
        BitStream bits = BitStream(Span<const u8>(buffer, 1));
        ASSERT(bits.read_bits(1) == 0);
        ASSERT(bits.read_bits(1) == 1);
        ASSERT(bits.read_bits(1) == 0);
        ASSERT(bits.read_bits(1) == 1);
        ASSERT(bits.read_bits(2) == 3);
        ASSERT(bits.read_bits(1) == 0);
        ASSERT(bits.read_bits(1) == 1);
        ASSERT(!bits.overrun());
        bits.read_bits(1);
        ASSERT(bits.overrun());
    }
    CHECK_LEAKS();

    {
        char digest[64] = { };

        // XXX(HK): Test FNV hash

        hash::md5_string("The quick brown fox jumps over the lazy dog").render(digest, ARRLEN(digest));
        ASSERT(str::equal(digest, "9e107d9d372bb6826bd81d3542a419d6"));
    }
    CHECK_LEAKS();
}
