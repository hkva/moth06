#include "moth06_common.hh"
#include "tests.hh"

#include "moth06_core.hh"

int main() {
    tests::require_game_files();

    // Read PBG
    {
        Array<u8> pbg_bytes = Array<u8>();
        if (!tests::load("./bin/gamefiles/紅魔郷ST.DAT", pbg_bytes)) {
            ASSERT(0 && "failed to load game archive file");
        }

        BitStream pbg_bits = BitStream(pbg_bytes.const_bytes());

        Array<pbg::FileEntry> entries = Array<pbg::FileEntry>();
        if (!pbg::read_entry_list(pbg_bits, entries)) {
            ASSERT(0 && "failed to read game archive file entries");
        }

        Array<u8> face12a = Array<u8>();
        for (usize i = 0; i < entries.length(); ++i) {
            pbg::FileEntry& f = entries[i];

            static Array<u8> contents = Array<u8>();
            if (!pbg::read_entry_data(pbg_bits, f, contents)) {
                ASSERT_NOT_REACHED();
            }
#if 1
            char fname[512]; std::snprintf(fname, sizeof(fname), "bin/%s", f.e_name);
            std::FILE* fout = std::fopen(fname, "wb"); ASSERT(fout);
            std::fwrite(contents.buffer(), 1, contents.length(), fout);
            std::fclose(fout);
#endif

            if (str::equal(f.e_name, "face12a.png")) {
                face12a = contents;
            }

            std::printf("[%d] %s\n", (int)i, f.e_name);
        }
        ASSERT(face12a.length());
        // 96681fc22212a1a5f441b391dfcd9314
        char face12a_digest[64] = { };
        hash::md5(face12a.const_bytes()).render(face12a_digest, sizeof(face12a_digest));
        std::printf("Hash of face12a.png: %s\n", face12a_digest);
        ASSERT(str::equal(face12a_digest, "96681fc22212a1a5f441b391dfcd9314"));

    }
    CHECK_LEAKS();
}
