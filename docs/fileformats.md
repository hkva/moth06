* [PBG archives (.dat)](#pbg-archives-dat)

## PBG archives (.dat)

Prior work: https://hg.linkmauve.fr/touhou/file/tip/formats/src/th06/pbg3.rs

**Note**: Data is processed bit-by-bit rather than byte-by-byte

**Note**: Integers are encoded with a variable size. They are 1 byte (8 bits) by default, where the preceeding two bits determine the number of extra bytes.

Pseudocode for reading integers would look something like this

```c++
int BitStream::ReadInt() {
    const size_t num_bits = ReadBits(2);
    return ReadBits((1 + num_bits) * 8);
}
```

#### PBG file header

```c
struct PBGHeader {
    char hdr_magic[4];  // "PBG3"
    int  hdr_etoff;     // Entry table file offset
    int  hdr_etsiz;     // Number of entries in the entry table
};
```

#### PBG Entry header

```c
struct PBGEntry {
    int e_unk1; // XXX: Unknown
    int e_unk2; // XXX: Unknown
    int e_chck; // Decompressed data checksum
    int e_foff; // Compressed data file offset
    int e_fsiz; // Decompressed data size
};
```


#### PBG Entry data

The start of the LZSS-compressed data is byte-aligned, but everything else is bit-packed (including LZSS control words!). Use the following parameters (taken from PyTouhou):

```c
#define LZSS_DICTIONARY_SIZE    0x2000
#define LZSS_MIN_MATCH_LENGTH   3
#define LZSS_OFFSET_BITS        13
#define LZSS_LENGTH_BITS        4
```