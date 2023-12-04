#include "moth06_common.hh"
#include "tests.hh"

#include "moth06_core.hh"

int main() {
    Tests::RequireGameFiles();

    // Read PBG
    {
        Array<u8> pbg_bytes = Array<u8>();
        if (!Tests::Load("./gamefiles/紅魔郷ST.DAT", pbg_bytes)) {
            ASSERT(0 && "failed to load game archive file");
        }

        BitStream pbg_bits = BitStream(pbg_bytes.ConstBytes());

        Array<PBG::FileEntry> entries = Array<PBG::FileEntry>();
        if (!PBG::ReadEntryList(pbg_bits, entries)) {
            ASSERT(0 && "failed to read game archive file entries");
        }

        Array<u8> face12a = Array<u8>();
        for (usize i = 0; i < entries.Length(); ++i) {
            PBG::FileEntry& f = entries[i];

            static Array<u8> contents = Array<u8>();
            if (!PBG::ReadEntryData(pbg_bits, f, contents)) {
                ASSERT_NOT_REACHED();
            }
#if 1
            std::FILE* fout = std::fopen(f.e_name, "wb"); ASSERT(fout);
            std::fwrite(contents.Buffer(), 1, contents.Length(), fout);
            std::fclose(fout);
#endif

            if (Str::Equal(f.e_name, "face12a.png")) {
                face12a = contents;
            }

            std::printf("[%d] %s\n", (int)i, f.e_name);
        }
        ASSERT(face12a.Length());
        // 96681fc22212a1a5f441b391dfcd9314
        char face12a_digest[64] = { };
        Hash::MD5(face12a.ConstBytes()).Render(face12a_digest, sizeof(face12a_digest));
        std::printf("Hash of face12a.png: %s\n", face12a_digest);
        ASSERT(Str::Equal(face12a_digest, "96681fc22212a1a5f441b391dfcd9314"));

    }
    CHECK_LEAKS();
}
