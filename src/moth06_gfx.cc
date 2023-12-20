#include "moth06.hh"

static struct {
    GfxBackend backend;
} gfx = { };

void init_gfx(const GfxInitParams& params) {
    if ((gfx.backend = params.requested_backend) == GfxBackend::Default) {
        gfx.backend = GfxBackend::SDLRenderer;
    }
}