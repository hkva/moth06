#ifndef _TESTS_HH_
#define _TESTS_HH_

static int test_alloc_tracker = 0;
#define MOTH06_ALLOC_TRACKER test_alloc_tracker
#define CHECK_LEAKS() ASSERT(test_alloc_tracker == 0 && "memory leak!")

#include "moth06_core.hh"

#include <cstdio>

#include <chrono>
#include <filesystem>

#define LOG(...) std::printf(__VA_ARGS__); std::printf("\n"); std::fflush(stdout);

namespace tests {

static inline bool does_folder_exist(const char* path) {
    return std::filesystem::is_directory(path);
}

static inline void require_game_files() {
    if (!does_folder_exist("bin/gamefiles")) {
        std::printf("bin/gamefiles/ doesn't exist, skipping test");
    }
}

static inline bool load(const char* path, Array<u8>& bytes) {
    std::FILE* f = std::fopen(path, "rb");
    if (!f) {
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    bytes.resize(std::ftell(f));
    std::fseek(f, 0, SEEK_SET);
    std::fread(bytes.buffer(), 1, bytes.length(), f);
    std::fclose(f);
    return true;
}

static inline u64 now() {
    return std::chrono::duration(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

static inline f64 now_sec() {
    return (f64)now() / 1e9f;
}

}

#endif // _TESTS_HH_
