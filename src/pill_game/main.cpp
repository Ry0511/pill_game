#include "pch.h"

#include "vendor/stb_image.h"
#include "vendor/stb_rect_pack.h"
#include "vendor/stb_truetype.h"

using namespace pill_game;

int main() {
    stbi_set_flip_vertically_on_load(false);
    int width, height, channels;
    stbi_uc* img_data = stbi_load("assets/Enemy_SpriteSheet.png", &width, &height, &channels, 4);
    PG_LOG(Info, "Enemy_SpriteSheet {}x{}x{}", width, height, channels);
    stbi_image_free(img_data);
    return 0;
}
