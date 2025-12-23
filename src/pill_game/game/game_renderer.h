//
// Date       : 21/12/2025
// Project    : pill_game
// Author     : -Ry
//

#pragma once

#include "pill_game/pch.h"

#include "pill_game/game/board.h"

struct SDL_Window;
struct SDL_Texture;
struct SDL_Renderer;

namespace pill_game::game {

// clang-format off
constexpr float  CELL_SIZE              = 32.0F;
constexpr float  MIN_WINDOW_WIDTH       = 640.0F;
constexpr float  MIN_WINDOW_HEIGHT      = 480.0F;
constexpr size_t ASSET_INDEX_ENEMY      = 0;
constexpr size_t ASSET_INDEX_PILL       = 1;
constexpr size_t ASSET_INDEX_BACKGROUND = 2;
constexpr size_t ASSET_COUNT            = 3;

// clang-format on

struct Image {
   public:
    int32_t Width;
    int32_t Height;
    int32_t Components;
    uint8_t* Data;

   public:
    Image(const fs::path&);
    ~Image();

   public:
    void white_mask() noexcept;
    int32_t pitch() const noexcept { return Width * Components; }

   public:
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(const Image&&) = delete;
    Image& operator=(const Image&&) = delete;
};

// clang-format off
struct Controller {
    uint8_t Up     : 1 {0};
    uint8_t Left   : 1 {0};
    uint8_t Right  : 1 {0};
    uint8_t Down   : 1 {0};
    uint8_t A      : 1 {0};
    uint8_t B      : 1 {0};
    uint8_t Start  : 1 {0};
    uint8_t Pause  : 1 {0};
};
// clang-format on

struct Colour {
    uint8_t Red{0};
    uint8_t Green{0};
    uint8_t Blue{0};
    uint8_t Alpha{0};

    constexpr Colour() = default;
    Colour(uint32_t colour);
};

enum class Scene : uint8_t {
    MainMenu = 0,
    GameSetup,
    Playing,
    GameFinished,
    None
};

struct Timer {
    float Value{0.0F};
    float Speed{0.0F};
    float StartingValue{0.0F};

    bool expired() const noexcept { return Value <= 0.0F; }
    void reset() noexcept { Value = StartingValue; }
};

struct FloatRect {
    float x{0.0F};
    float y{0.0F};
    float w{0.0F};
    float h{0.0F};

    template <class T>
    T as() const noexcept {
        return T{
            static_cast<decltype(T{}.x)>(x),
            static_cast<decltype(T{}.y)>(y),
            static_cast<decltype(T{}.w)>(w),
            static_cast<decltype(T{}.h)>(h)
        };
    }
};

struct GameContext {
    SDL_Renderer* Renderer{nullptr};
    SDL_Window* Window{nullptr};
    SDL_Texture* TextureAtlas{nullptr};
    SDL_Texture* GameplayTexture{nullptr};
    std::array<FloatRect, ASSET_COUNT> AssetBounds{};

    int32_t BackgroundFrame = 1;

    Controller Input{};
    bool IsPaused{false};
    bool Running{false};
    Scene CurrentScene{Scene::None};
    Scene RequestedScene{Scene::Playing};
    uint8_t CurrentLevel{20};
    bool AllowPills{true};
    bool AllowBlocks{false};

    uint64_t SceneTicks{0};
    float DeltaTime{0.0F};
    std::array<Timer, 16> Timers{};
    PillGameBoard TheBoard{};
};

std::mt19937& rng(void) noexcept;
SDL_Texture* atlas(void) noexcept;
Timer& timer(size_t index);
GameContext& ctx(void) noexcept;

inline bool is_first_tick(void) noexcept {
    return ctx().SceneTicks == 0;
}

int run_application(void);
int initialise(void) noexcept;
void shutdown(void) noexcept;

void tick_scene_main_menu(void);
void tick_scene_game_setup(void);
void tick_scene_playing(void);
void tick_scene_game_finished(void);

}  // namespace pill_game::game
