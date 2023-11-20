#include "moth06.hh"

#include <fluidsynth.h>

Client cl = { };

void DrawDebugUI() {
    if (!(cl.dbg.flags & DEBUG_FLAG_UI)) {
        return;
    }

    static const char* extra_menu_info = nullptr;

    f32 menu_bar_height = 0;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Hide debug menu")) {
                cl.dbg.flags &= ~DEBUG_FLAG_UI;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) {
                cl.app.state |= APP_STATE_WANTS_QUIT;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            static struct {
                const char* name; u8 screen;
            } screens[] = {
                { .name = "Game", .screen = DEBUG_SCREEN_NONE, },
                { .name = "Files", .screen = DEBUG_SCREEN_FILES, },
                { .name = "Test", .screen = DEBUG_SCREEN_TEST, },
            };
            for (usize i = 0; i < ArrLen(screens); ++i) {
                if (ImGui::MenuItem(screens[i].name, NULL, cl.dbg.screen == screens[i].screen)) {
                    cl.dbg.screen = screens[i].screen;
                }
                if (i == 0) {
                    ImGui::Separator();
                }
            }
            ImGui::EndMenu();
        }
        ImGui::Separator();
        ImGui::Text("Debug menu - Toggle with F3");
        if (extra_menu_info) {
            ImGui::Separator();
            ImGui::Text("%s", extra_menu_info);
        }
        menu_bar_height = ImGui::GetWindowHeight();
        ImGui::EndMainMenuBar();
    }

    // ImGui::SetNextWindow
    ImGui::SetNextWindowPos(ImVec2(0, menu_bar_height - 1));
    ImGui::SetNextWindowSize(ImVec2(640, 480 - menu_bar_height + 1));
    const ImGuiWindowFlags DEBUG_WINDOW_FLAGS = ImGuiWindowFlags_NoDecoration & ~ImGuiWindowFlags_NoScrollbar;
    switch (cl.dbg.screen) {
    case DEBUG_SCREEN_FILES: {
        static usize info_mem_usage = 0;
        static char info[256] = { };
        snprintf(info, sizeof(info), "%u files, %u mem", (u32)cl.fs.files.Length(), (u32)info_mem_usage);
        extra_menu_info = info;

        if (ImGui::Begin("Files", NULL, DEBUG_WINDOW_FLAGS)) {
            if (ImGui::BeginTable("Files##Table", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("File");
                ImGui::TableSetupColumn("Info");
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                static usize selected_file_idx = 0;
                static Hash::MD5Digest selected_file_hash = Hash::MD5(cl.fs.files[selected_file_idx].data.View());
                if (ImGui::BeginChild("Files##Table##Left")) {
                    info_mem_usage = 0;
                    for (usize i = 0; i < cl.fs.files.Length(); ++i) {
                        info_mem_usage += cl.fs.files[i].data.Length();
                        if (ImGui::Selectable(cl.fs.files[i].entry.e_name, selected_file_idx == i)) { 
                            selected_file_idx = i;
                            selected_file_hash = Hash::MD5(cl.fs.files[selected_file_idx].data.View());
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::TableSetColumnIndex(1);
                Assert(selected_file_idx < cl.fs.files.Length());
                File& selected_file = cl.fs.files[selected_file_idx];
                char hexdigest[64]; selected_file_hash.Render(BufferViewOf<char>(hexdigest, sizeof(hexdigest)));
                ImGui::SeparatorText("PBG Archive Entry");
                ImGui::Text("    e_unk1: 0x%x", selected_file.entry.e_unk1);
                ImGui::Text("    e_unk2: 0x%x", selected_file.entry.e_unk2);
                ImGui::Text("    e_chck: 0x%x", selected_file.entry.e_chck);
                ImGui::Text("    e_fpos: 0x%x", selected_file.entry.e_fpos);
                ImGui::Text("    e_name: %s", selected_file.entry.e_name);
                ImGui::SeparatorText("Data");
                ImGui::Text("    Size: %u bytes", (u32)selected_file.data.LengthInBytes());
                ImGui::Text("    MD5:  %s", hexdigest); ImGui::SameLine(); if (ImGui::SmallButton("Copy")) { SDL_SetClipboardText(hexdigest); }
                switch (selected_file.asset_type) {
                case ASSET_TYPE_IMAGE: {
                    ImGui::SeparatorText("Image");
                    ImGui::Text("    Resolution: %ux%u", (u32)selected_file.image.width, (u32)selected_file.image.height);
                    ImVec2 avail = ImGui::GetContentRegionAvail();
                    avail.x -= 5; avail.y -= 5;
                    f32 scale_fac = 1.0f;
                    if (avail.x < selected_file.image.width) {
                        scale_fac = avail.x / (f32)selected_file.image.width;
                    }
                    if (avail.y / scale_fac < selected_file.image.height) {
                        scale_fac = avail.y / (f32)selected_file.image.height;
                    }
                    ImVec2 res = ImVec2((f32)selected_file.image.width * scale_fac, (f32)selected_file.image.height * scale_fac);
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 28);
                    ImGui::Image(Gfx::GetTextureDescriptor(selected_file.image.tex), res);
                } break;
                }
                ImGui::EndTable();
            }

            ImGui::End();
        }
    } break;
    case DEBUG_SCREEN_TEST: {
        if (ImGui::Begin("Test window", NULL, DEBUG_WINDOW_FLAGS)) {
            ImGui::Text("This is a test window");
            ImGui::ShowDemoWindow();
            ImGui::End();
        }
    } break;
    default: {
        extra_menu_info = nullptr;
    }
}
}

void Run(usize argc, const char* argv[]) {
    for (usize i = 1; i < argc; ++i) {
        const char* a = argv[i];
        if (Str::Equal(a, "--dump-files")) {
            cl.app.cli |= CLI_OPT_DUMP_FILES;
        }
    }

    // create window first so we have a graphics context while uploading textures
    SDL_Window* w = SDL_CreateWindow("moth06", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
    if (!w) {
        Die("Failed to create game window: %s", SDL_GetError());
    }

    const Gfx::InitParams gfxpar = { .backend = Gfx::BACKEND_AUTO, .wnd = w, };
    Gfx::Init(gfxpar);

    const char* ARCHIVE_NAMES[] = {
        "cm.DAT",
        "in.DAT",
        "md.DAT",
        "st.DAT",
    };
    for (usize i = 0; i < ArrLen(ARCHIVE_NAMES); ++i){
        Fs::Mount(ARCHIVE_NAMES[i]);
    }
    
    if (cl.app.cli & CLI_OPT_DUMP_FILES) {
        for (usize i = 0; i < cl.fs.files.Length(); ++i) {
            Fs::Write(cl.fs.files[i].entry.e_name, cl.fs.files[i].data.View());
        }
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        Die("Failed to initialize SDL: %s", SDL_GetError());
    }

    // Draw bg
    File* bg = Fs::Read("data/th06logo.jpg");
    if (!bg) {
        Die("Missing bg");
    }

    // Load midi source from memory
    const char* filename = "data/th06_01.mid";
    File* mus_mid = Fs::Read(filename);
    if (!mus_mid) {
        Die("Missing file %s", filename);
    }

    Hash::MD5Digest digest = Hash::MD5String("The quick brown fox jumps over the lazy dog");
    char buf[64]; digest.Render(BufferViewOf<char>(buf, sizeof(buf)));
    std::printf("Digest: %s\n", buf);

    fluid_settings_t* fsettings = new_fluid_settings();
    fluid_synth_t* fsynth = new_fluid_synth(fsettings);
    fluid_player_t* fplayer = new_fluid_player(fsynth);
    new_fluid_audio_driver(fsettings, fsynth);

    fluid_synth_sfload(fsynth, "moth06_data/UHD3.sf2", 1);

    printf("Playing %s...\n", filename);
    fluid_player_add_mem(fplayer, mus_mid->data.Buffer(), mus_mid->data.Length());
    fluid_player_play(fplayer);

    while (!(cl.app.state & APP_STATE_WANTS_QUIT)) {
        SDL_Event evt = { };
        while (SDL_PollEvent(&evt)) {
            Gfx::UIEvent(&evt);
            switch (evt.type) {
            case SDL_KEYDOWN: {
                switch (evt.key.keysym.sym) {
                case SDLK_F3: {
                    cl.dbg.flags ^= DEBUG_FLAG_UI;
                } break;
                }
            } break;
            case SDL_QUIT: {
                cl.app.state |= APP_STATE_WANTS_QUIT;
            } break;
            }
        }

        Gfx::BeginFrame();
            Gfx::DebugDrawFullscreenTexture(bg->image.tex);
            DrawDebugUI();
        Gfx::EndFrame();
    }
}

//
// OS-specific entry points
//

#ifdef MOTH06_WINDOWS

#include <Windows.h>

int WinMain(HINSTANCE idc1, HINSTANCE idc2, LPSTR idc3, int idc4) {
    (void)idc1; (void)idc2; (void)idc3; (void)idc4;
    // Reset working directory
    char path[MAX_PATH] = { 0 };
    if (!GetModuleFileNameA(NULL, path, sizeof(path))) {
        Die("Windows: Failed to get executable path");
    }
    if (!Str::BaseName(path)) {
        Die("Windows: Failed to truncate executable path (%s)", path);
    }
    if (!SetCurrentDirectoryA(path)) {
        Die("Windows: Failed to reset working directory to %s", path);
    }

    if (AllocConsole()) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    Run(__argc, (const char**)__argv);
}

#endif

#ifdef MOTH06_OSX

#include <mach-o/dyld.h>
#include <unistd.h>

int main(int argc, char** argv) {
    // Reset working directory
    char path[1024] = { }; u32 path_size = sizeof(path);
    if (_NSGetExecutablePath(path, &path_size) != 0) {
        Die("OSX: Failed to get executable path");
    }
    if (!Str::BaseName(path)) {
        Die("OSX: Failed to truncate executable path (%s)", path);
    }
    if (chdir(path) != 0) {
        Die("OSX: Failed to reset working directory to %s", path);
    }

    Run(argc, (const char**)argv);
}

#endif
