//
// Date       : 23/12/2025
// Project    : pill_game
// Author     : -Ry
//

#include "pill_game/pch.h"
#include "pill_game/game/game_renderer.h"

#include "SDL3/SDL.h"
#include "vendor/stb_rect_pack.h"

namespace pill_game::game {

namespace {

GameContext game_context;
std::mt19937 random_engine{0};

void load_assets(void);

}  // namespace

GameContext& ctx(void) noexcept {
    return game_context;
}

SDL_Texture* atlas(void) noexcept {
    return game_context.TextureAtlas;
}

const FloatRect& asset(size_t index) {
    return game_context.AssetBounds.at(index);
}

Timer& timer(size_t index) {
    return game_context.Timers.at(index);
}

std::mt19937& rng(void) noexcept {
    return random_engine;
}

int initialise(void) noexcept {
    if (!SDL_SetAppMetadata("Pill Game", "0.0", "com.ry.pillgame")) {
        PG_LOG(Warn, "Failed to set app metadata - {}", SDL_GetError());
    }

    game_context = GameContext();
    random_engine = std::mt19937(std::random_device{}());

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        PG_LOG(Err, "Failed to initialize SDL - {}", SDL_GetError());
        return -1;
    }

    if (!SDL_CreateWindowAndRenderer(
            "Pill Game",
            1280,
            768,
            SDL_WINDOW_RESIZABLE,
            &ctx().Window,
            &ctx().Renderer
        )) {
        PG_LOG(Err, "Failed to create window or renderer - {}", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_SetWindowMinimumSize(
        ctx().Window,
        static_cast<int>(MIN_WINDOW_WIDTH),
        static_cast<int>(MIN_WINDOW_HEIGHT)
    );

    ctx().GameplayTexture = SDL_CreateTexture(
        ctx().Renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        GAME_BOARD_WIDTH * static_cast<int32_t>(CELL_SIZE),
        GAME_BOARD_HEIGHT * static_cast<int32_t>(CELL_SIZE)
    );

    try {
        load_assets();
    } catch (const std::exception& ex) {
        PG_LOG(Err, "error during initialisation - ", ex.what());
        shutdown();
        return -1;
    }

    try {
        init_audio();
    } catch (const std::exception& ex) {
        PG_LOG(Warn, "{}", ex.what());
        PG_LOG(Warn, "No audio will be played");
        ctx().AudioStream = nullptr;
        ctx().AudioDeviceId = 0;
        ctx().AudioSources = {};
    }

    return 0;
}

void shutdown(void) noexcept {
    SDL_DestroyTexture(ctx().TextureAtlas);
    SDL_DestroyTexture(ctx().GameplayTexture);
    SDL_DestroyRenderer(ctx().Renderer);
    SDL_DestroyWindow(ctx().Window);
    SDL_Quit();
}

namespace {

void load_assets(void) {
    constexpr int32_t atlas_size = 256;
    constexpr int32_t bg_width = 2;
    constexpr int32_t bg_size = bg_width * bg_width * 2;

    fs::path assets_path = fs::current_path() / "assets";

    Image enemy_img{assets_path / "Enemies.png"};
    enemy_img.white_mask();

    Image pill_img{assets_path / "Pieces.png"};
    pill_img.white_mask();

    // Backgrounds
    const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888);
    uint32_t magenta = SDL_MapRGBA(fmt, nullptr, 91, 28, 119, 150);
    uint32_t green = SDL_MapRGBA(fmt, nullptr, 44, 160, 28, 150);
    uint32_t black = 0x0;

    std::array<uint32_t, bg_size> bg_images{black};

    // Magenta & Black Background
    bg_images[0 * 2 + 0] = magenta;
    bg_images[0 * 2 + 1] = black;
    bg_images[1 * 2 + 0] = black;
    bg_images[1 * 2 + 1] = magenta;

    // Green & Black Background
    bg_images[2 * 2 + 0] = green;
    bg_images[2 * 2 + 1] = black;
    bg_images[3 * 2 + 0] = black;
    bg_images[3 * 2 + 1] = green;

    // Pack the assets into a single atlas
    std::array<stbrp_node, atlas_size> nodes{};
    stbrp_context packing_ctx{};
    stbrp_init_target(&packing_ctx, atlas_size, atlas_size, nodes.data(), atlas_size);

    std::array<stbrp_rect, ASSET_COUNT> rects_to_pack{
        stbrp_rect{     ASSET_INDEX_ENEMY, enemy_img.Width, enemy_img.Height},
        stbrp_rect{      ASSET_INDEX_PILL,  pill_img.Width,  pill_img.Height},
        stbrp_rect{ASSET_INDEX_BACKGROUND,        bg_width,     bg_width * 2}
    };

    int exit = stbrp_pack_rects(
        &packing_ctx,
        rects_to_pack.data(),
        static_cast<int32_t>(rects_to_pack.size())
    );
    if (exit != 1) {
        throw std::runtime_error{"failed to pack required assets into atlas"};
    }

    // store the asset starting positions in the packed atlas
    for (const stbrp_rect& rect : rects_to_pack) {
        auto& asset = ctx().AssetBounds.at(rect.id);
        asset = FloatRect{
            static_cast<float>(rect.x),
            static_cast<float>(rect.y),
            static_cast<float>(rect.w),
            static_cast<float>(rect.h)
        };
    }

    ctx().TextureAtlas = SDL_CreateTexture(
        ctx().Renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STATIC,
        atlas_size,
        atlas_size
    );

    if (ctx().TextureAtlas == nullptr) {
        throw std::runtime_error{"failed to create texture atlas"};
    }

    auto write_into_texture = [](const FloatRect& rect, const void* data, const int32_t pitch) {
        auto sdl_rect = rect.as<SDL_Rect>();
        SDL_UpdateTexture(ctx().TextureAtlas, &sdl_rect, data, pitch);
    };

    write_into_texture(asset(ASSET_INDEX_ENEMY), enemy_img.Data, enemy_img.pitch());
    write_into_texture(asset(ASSET_INDEX_PILL), pill_img.Data, pill_img.pitch());
    write_into_texture(asset(ASSET_INDEX_BACKGROUND), bg_images.data(), bg_width * 4);
}

}  // namespace

}  // namespace pill_game::game