#include "moth06.hh"

#include <stb_image.h>

static void TryParseAsset(File& file) {
	const char* ext = Str::Extension(file.entry.e_name);
	if (!ext) {
		return;
	}

	if (Str::Equal(ext, "png") || Str::Equal(ext, "jpg")) {
		file.asset_type = ASSET_TYPE_IMAGE;
		int chan = 0;
		u32* pixels = nullptr;
		if (!(pixels = (u32*)stbi_load_from_memory(file.data.Buffer(), file.data.LengthInBytes(), (int*)&file.image.width, (int*)&file.image.height, &chan, 4))) {
			Die("Failed to parse image asset %s", file.entry.e_name);
		}
		file.image.tex = Gfx::UploadTexture(pixels, file.image.width, file.image.height);
		stbi_image_free(pixels);
	}
}

bool Fs::Mount(const char* archive_path) {
	Array<u8> archive = Array<u8>();
	if (!Fs::ReadDisk(archive_path, archive)) {
		return false;
	}
	BitStream bits = BitStream(archive.View());
	Array<PBG::Entry> entries;
	if (!PBG::ReadEntries(bits, entries)) {
		Die("Tried to mount non-archvie file %s as an archive", archive_path);
		return false;
	}
	cl.fs.files.ReserveMore(entries.Length());
	for (usize i = 0; i < entries.Length(); ++i) {
		File file = { .entry = entries[i] };
		if (!PBG::ReadEntryContents(bits, file.entry, file.data)) {
			Die("Failed to read file %s::%s", archive_path, file.entry.e_name);
		}
		TryParseAsset(file);
		cl.fs.files.Append(file);
	}
    printf("Mounted %s (%u files)\n", archive_path, (u32)entries.Length());
    return true;
}

File* Fs::Read(const char* path) {
	if (!Str::StartsWith(path, "data/")) {
		Die("Read called on non-archive path %s", path);
	}
	path = &path[5];
	for (usize i = 0; i < cl.fs.files.Length(); ++i) {
		if (Str::Equal(cl.fs.files[i].entry.e_name, path)) {
			return &cl.fs.files[i];
		}
	}
	return nullptr;
}

bool Fs::ReadDisk(const char* path, Array<u8>& data) {
	std::FILE* f = std::fopen(path, "rb");
	if (f) {
		std::fseek(f, 0, SEEK_END);
		data.Resize(std::ftell(f));
		std::fseek(f, 0, SEEK_SET);
		std::fread(data.Buffer(), 1, data.LengthInBytes(), f);
		std::fclose(f);
	}
	return f;
}

bool Fs::Write(const char *path, BufferView data) {
	std::FILE* f = std::fopen(path, "wb");
	if (f) {
		std::fwrite(data.buf, 1, data.LengthInBytes(), f);
		std::fclose(f);
	}
	return f;
}
