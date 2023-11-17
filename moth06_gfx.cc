#include "moth06.hh"

// Dear ImGui backends
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

union TextureData {
	SDL_Texture* sdl;
};

static struct {
	u8 backend;
	union {
		struct {
			SDL_Renderer* r;
		} sdl;
	};
	Array<TextureData> textures;
} gfx = { };

void Gfx::Init(const InitParams& par) {
	if ((gfx.backend = par.backend) == BACKEND_AUTO) {
		gfx.backend = BACKEND_SDL_RENDERER;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui::GetStyle().ScrollbarRounding = 0.0f;

	switch (gfx.backend) {
	case BACKEND_SDL_RENDERER: {
		if (!(gfx.sdl.r = SDL_CreateRenderer(par.wnd, -1, SDL_RENDERER_ACCELERATED))) {
			Die("Failed to create SDL renderer: %s", SDL_GetError());
		}
		if (!ImGui_ImplSDL2_InitForSDLRenderer(par.wnd, gfx.sdl.r) || !ImGui_ImplSDLRenderer2_Init(gfx.sdl.r)) {
			Die("Failed to initialize Dear ImGui backend for SDL2_Renderer");
		}
	} break;
	};

}

void Gfx::UIEvent(const SDL_Event* evt) {
	ImGui_ImplSDL2_ProcessEvent(evt);
}

void Gfx::BeginFrame(void) {
	switch (gfx.backend) {
	case BACKEND_SDL_RENDERER: {
		SDL_RenderClear(gfx.sdl.r);
		ImGui_ImplSDLRenderer2_NewFrame();
	} break;
	}
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void Gfx::EndFrame(void) {
	ImGui::Render();
	switch (gfx.backend) {
	case BACKEND_SDL_RENDERER: {
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(gfx.sdl.r);
	} break;
	}
}

Gfx::Texture Gfx::UploadTexture(u32* rgba, u32 width, u32 height) {
	TextureData tex = { };
	switch (gfx.backend) {
	case BACKEND_SDL_RENDERER: {
		SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(rgba, width, height, 32, width * sizeof(u32), 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		if (!surf) {
			Die("Error while creating surface for texture: %s", SDL_GetError());
		}
		if (!(tex.sdl = SDL_CreateTextureFromSurface(gfx.sdl.r, surf))) {
			Die("Error while creating texture: %s", SDL_GetError());
		}
		SDL_FreeSurface(surf);
	} break;
	}
	return (u32)gfx.textures.Append(tex);;
}

void* Gfx::GetTextureDescriptor(Texture texture) {
	switch (gfx.backend) {
	case BACKEND_SDL_RENDERER: {
		return (void*)gfx.textures[texture].sdl;
	} break;
	}
	return nullptr;
}

void Gfx::DebugDrawFullscreenTexture(Texture texture) {
	switch (gfx.backend) {
	case BACKEND_SDL_RENDERER: {
		SDL_RenderCopy(gfx.sdl.r, gfx.textures[texture].sdl, nullptr, nullptr);
	} break;
	}
}
