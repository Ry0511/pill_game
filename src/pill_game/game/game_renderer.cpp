//
// Date       : 21/12/2025
// Project    : pill_game
// Author     : -Ry
//

#include "pill_game/pch.h"
#include "game_renderer.h"
#include "pill_game/game/board.h"

#include "SDL3/SDL.h"

#include "vendor/stb_rect_pack.h"

namespace pill_game::game {

namespace {

void process_events(void);
void process_input(const SDL_Event& event);
void tick_game(void);

}  // namespace

int run_application() {
    auto ret = initialise();
    if (ret != 0) {
        PG_LOG(Err, "initialisation had non-zero {} exit code; exiting...", ret);
        return ret;
    }

    int exit_code = 0;
    ctx().Running = true;

    constexpr uint64_t target_tick_rate{8};
    uint64_t ticks_last_frame{SDL_GetTicks()};

    SDL_ShowWindow(ctx().Window);

    while (ctx().Running) {
        uint64_t start_ticks{SDL_GetTicks()};

        auto* renderer = ctx().Renderer;
        ctx().DeltaTime = static_cast<float>(start_ticks - ticks_last_frame) / 1000.0F;
        ticks_last_frame = start_ticks;

        tick_audio();

        // Render background image to window
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        auto src = asset(ASSET_INDEX_BACKGROUND).as<SDL_FRect>();
        src.w = src.h = 2.0F;
        src.y = (static_cast<float>(ctx().BackgroundFrame) * 2.0F);

        SDL_SetTextureColorMod(atlas(), 255, 255, 255);
        SDL_SetTextureScaleMode(atlas(), SDL_SCALEMODE_NEAREST);
        SDL_RenderTextureTiled(renderer, atlas(), &src, 32.0F, nullptr);
        SDL_SetTextureScaleMode(atlas(), SDL_SCALEMODE_LINEAR);

        try {
            process_events();
            tick_game();
            ++ctx().SceneTicks;
        } catch (const std::exception& ex) {
            PG_LOG(Err, "an exception occurred - {}", ex.what());
            ctx().Running = false;
            exit_code = -1;
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(
            renderer,
            50,
            50,
            std::format(
                "{:.3F}, {}, {}",
                ctx().DeltaTime,
                ctx().SceneTicks,
                ctx().Running ? "Is Running" : "Is Not Running"
            )
                .c_str()
        );

        SDL_RenderPresent(renderer);

        // primitive frame cap
        uint64_t end_ticks{SDL_GetTicks()};
        uint64_t diff_ticks{end_ticks - start_ticks};
        if (diff_ticks < target_tick_rate) {
            SDL_Delay(static_cast<uint32_t>(diff_ticks));
        }
    }

    shutdown();
    return exit_code;
}

namespace {

void process_events(void) {
    SDL_Event event{};

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                ctx().Running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                process_input(event);
                break;

            default:
                break;
        }
    }
}

void process_input(const SDL_Event& event) {
    const uint8_t val = (event.type == SDL_EVENT_KEY_DOWN) ? 1 : 0;

    // clang-format off
    switch (event.key.key) {
        case SDLK_UP    : ctx().Input.Up    = val; break;
        case SDLK_DOWN  : ctx().Input.Down  = val; break;
        case SDLK_LEFT  : ctx().Input.Left  = val; break;
        case SDLK_S     : ctx().Input.Left  = val; break;
        case SDLK_RIGHT : ctx().Input.Right = val; break;
        case SDLK_D     : ctx().Input.Right = val; break;
        case SDLK_Z     : ctx().Input.B     = val; break;
        case SDLK_K     : ctx().Input.B     = val; break;
        case SDLK_X     : ctx().Input.A     = val; break;
        case SDLK_L     : ctx().Input.A     = val; break;
        case SDLK_ESCAPE: ctx().Input.Pause = val; break;
        case SDLK_RETURN: ctx().Input.Start = val; break;
        case SDLK_E     : ctx().Input.Start = val; break;
    }
    // clang-format on

    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE) {
        ctx().TheBoard.init_board(
            BoardInitParams::create_difficulty(
                ctx().CurrentLevel,
                ctx().AllowPills,
                ctx().AllowBlocks
            ),
            rng()
        );
    }
}

void tick_game(void) {
    // Switch scenes
    if (ctx().RequestedScene != Scene::None) {
        ctx().CurrentScene = ctx().RequestedScene;
        ctx().SceneTicks = 0;
        ctx().RequestedScene = Scene::None;
        ctx().Timers.fill(Timer{});
    }

    const float delta_step = ctx().DeltaTime * (ctx().IsPaused ? 0.0F : 1.0F);

    for (Timer& timer : ctx().Timers) {
        timer.Value = std::max(timer.Value - (timer.Speed * delta_step), 0.0F);
    }

    // clang-format off
    switch (ctx().CurrentScene) {
        case Scene::MainMenu    : tick_scene_main_menu();     break;
        case Scene::GameSetup   : tick_scene_game_setup();    break;
        case Scene::Playing     : tick_scene_playing();       break;
        case Scene::GameFinished: tick_scene_game_finished(); break;
        default: break;
    }
    // clang-format on
}

}  // namespace

}  // namespace pill_game::game