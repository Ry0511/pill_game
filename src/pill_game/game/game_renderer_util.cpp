//
// Date       : 23/12/2025
// Project    : pill_game
// Author     : -Ry
//

#include "pill_game/pch.h"
#include "pill_game/game/game_renderer.h"

#include "vendor/stb_image.h"

namespace pill_game::game {

Image::Image(const fs::path& path) {
    stbi_set_flip_vertically_on_load(false);
    Data = stbi_load(path.string().c_str(), &Width, &Height, &Components, 4);

    if (Data == nullptr) {
        throw std::runtime_error(std::format(
            "Failed to load image - {}; {}",
            path.string(),
            stbi_failure_reason()
        ));
    }
}

Image::~Image() {
    if (Data != nullptr) {
        PG_LOG(Trace, "Freeing image data {:p}", static_cast<void*>(Data));
        stbi_image_free(Data);
    }
}

void Image::white_mask() noexcept {
    for (int32_t i = 0; i < Width * Height * Components; i += Components) {
        uint32_t* colour = reinterpret_cast<uint32_t*>(Data + i);
        if (*colour == 0xFF000000) {
            colour = 0x0;
        } else if ((*colour & 0x00FFFFFF) != 0x0) {
            *colour = 0xFFFFFFFF;
        }
    }
}

Colour::Colour(uint32_t colour)
    : Red((colour & 0xFF000000) >> 24),
      Green((colour & 0x00FF0000) >> 16),
      Blue((colour & 0x0000FF00) >> 8),
      Alpha((colour & 0x000000FF)) {
}

void tick_scene_main_menu(void) {
    ctx().RequestedScene = Scene::Playing;
}

void tick_scene_game_setup(void) {
    ctx().RequestedScene = Scene::Playing;
}

void tick_scene_game_finished(void) {
    ctx().RequestedScene = Scene::Playing;
}

}  // namespace pill_game::game