#ifndef _TESTS_HH_
#define _TESTS_HH_

static int test_alloc_tracker = 0;
#define MOTH06_ALLOC_TRACKER test_alloc_tracker
#define CHECK_LEAKS() ASSERT(test_alloc_tracker == 0 && "memory leak!")

#include "moth06_core.hh"

#include <cstdio>

#include <chrono>
#include <filesystem>

namespace Tests {

static inline bool DoesFolderExist(const char* path) {
    return std::filesystem::is_directory(path);
}

static inline void RequireGameFiles() {
    if (!DoesFolderExist("gamefiles")) {
        std::printf("gamefiles/ doesn't exist, skipping test");
    }
}

static inline bool Load(const char* path, Array<u8>& bytes) {
    std::FILE* f = std::fopen(path, "rb");
    if (!f) {
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    bytes.Resize(std::ftell(f));
    std::fseek(f, 0, SEEK_SET);
    std::fread(bytes.Buffer(), 1, bytes.Length(), f);
    std::fclose(f);
    return true;
}

static inline u64 Now() {
    return std::chrono::duration(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

static inline f64 NowSeconds() {
    return (f64)Now() / 1e9f;
}

}

#endif // _TESTS_HH_