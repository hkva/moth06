#include "moth06.hh"

#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

#define dbgmsg(...) dbgmsg_( "GFX  | " __VA_ARGS__ );

static struct {
	GfxBackend backend;
	union {
		struct {
			SDL_Renderer* r;
		} sdlr;
	};
} gfx = { };

void init_gfx( const GfxInitParams& params ) {
	if ( (gfx.backend = params.requested_backend) == GfxBackend::Default ) {
		gfx.backend = GfxBackend::SDLRenderer;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui::GetIO().IniFilename = nullptr;
	switch ( gfx.backend ) {
	case GfxBackend::SDLRenderer: {
		if ( !(gfx.sdlr.r = SDL_CreateRenderer( a.wnd, -1, SDL_RENDERER_ACCELERATED )) ) {
			die( "Failed to create SDL renderer: %s", SDL_GetError() );
		}
		ImGui_ImplSDL2_InitForSDLRenderer( a.wnd, gfx.sdlr.r );
		ImGui_ImplSDLRenderer2_Init( gfx.sdlr.r );
		dbgmsg( "Initialized SDL renderer" );
	} break;
	}
}

void begin_frame() {
	switch ( gfx.backend ) {
	case GfxBackend::SDLRenderer: {
		SDL_SetRenderDrawColor( gfx.sdlr.r, 0x0F, 0x0F, 0x0F, 0xFF );
		SDL_RenderClear( gfx.sdlr.r );
		ImGui_ImplSDLRenderer2_NewFrame();
	} break;
	}
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void end_frame() {
	// ImGui::ShowDemoWindow();
	ImGui::Render();
	switch ( gfx.backend ) {
	case GfxBackend::SDLRenderer: {
		ImGui_ImplSDLRenderer2_RenderDrawData( ImGui::GetDrawData() );
		SDL_RenderPresent( gfx.sdlr.r );
	} break;
	}
}

void handle_ui_event( const SDL_Event* evt ) {
	ImGui_ImplSDL2_ProcessEvent( evt );
}
