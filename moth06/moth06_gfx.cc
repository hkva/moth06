#include "SDL_render.h"
#include "imgui.h"
#include "moth06.hh"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "moth06_common/moth06_common.hh"

// NOTE(HK): Yes, these backends are implemented by switching on an enum
// This game doesn't require any hardcore rendering beyond a basic 2D batch
// renderer so I think this is OK!

static struct {
    gfx::Backend backend;

    union {
        struct {
            SDL_Renderer* r;
        } sdl;
    };
} _gfx = { };

void gfx::init(const InitOptions* opts) {
    if ((_gfx.backend = opts->requested_backend) == Backend::Default) {
        _gfx.backend = Backend::SDLRenderer;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScrollbarRounding = 0.0f;

    switch (_gfx.backend) {
        case Backend::SDLRenderer: {
            if (!(_gfx.sdl.r = SDL_CreateRenderer(a.wnd, -1, 0))) {
                die("gfx: Failed to create SDL renderer: %s", SDL_GetError());
            }
            if (!ImGui_ImplSDL2_InitForSDLRenderer(a.wnd, _gfx.sdl.r) || !ImGui_ImplSDLRenderer2_Init(_gfx.sdl.r)) {
			    die("Failed to initialize Dear ImGui backend for SDL2_Renderer");
            }
	
        } break;
        default: ASSERT_NOT_REACHED();
    }

    dbgmsg("Initialized graphics");
}

void gfx::handle_ui_event(const SDL_Event *evt) {
    ImGui_ImplSDL2_ProcessEvent(evt);
}

void gfx::begin_frame() {
    switch (_gfx.backend) {
        case Backend::SDLRenderer: {
            SDL_RenderClear(_gfx.sdl.r);
            ImGui_ImplSDLRenderer2_NewFrame();
        } break;
        default: ASSERT_NOT_REACHED();
    }
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void gfx::end_frame() {
    ImGui::Render();
    switch (_gfx.backend) {
        case Backend::SDLRenderer: {
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
            SDL_RenderPresent(_gfx.sdl.r);
        } break;
        default: ASSERT_NOT_REACHED();
    }
}
