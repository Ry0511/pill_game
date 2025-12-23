//
// Date       : 23/12/2025
// Project    : pill_game
// Author     : -Ry
//

#include "pill_game/pch.h"
#include "pill_game/game/game_renderer.h"

#include "SDL3/SDL.h"

namespace pill_game::game {

namespace {

// clang-format off
constexpr size_t TIMER_BOARD_BUILDING = 0;
constexpr size_t TIMER_ENEMY_TEX1     = 1;
constexpr size_t TIMER_ENEMY_TEX2     = 2;
constexpr size_t TIMER_PIECE_DROP     = 3;
constexpr size_t TIMER_PIECE_HMOVE    = 4;
constexpr size_t TIMER_PIECE_VMOVE    = 5;
constexpr size_t TIMER_NOISY_BOARD    = 6;
constexpr size_t TIMER_GRAVITY_TICK   = 7;
constexpr size_t TIMER_ENT_BREAK_TICK = 8;
// clang-format on

int32_t noise_max_row = 1;
int32_t noise_max_col = 1;
int32_t enemy_frame_tex1 = 0;
int32_t enemy_frame_tex2 = 0;
int32_t entity_updates_from_gravity = 0;
int32_t entities_broken_last_frame = 0;

void first_tick_setup(void);
void render_game_board(void);
void render_game_board_texture(void);

SDL_FRect enemy_texture_pos(bool alt_enemy, int frame) {
    const auto& bounds = ctx().AssetBounds[ASSET_INDEX_ENEMY];
    return SDL_FRect{
        bounds.x + (CELL_SIZE * static_cast<float>(frame)),
        bounds.y + (alt_enemy ? CELL_SIZE : 0.0F),
        CELL_SIZE,
        CELL_SIZE
    };
}

}  // namespace

void tick_scene_playing(void) {
    if (is_first_tick()) {
        first_tick_setup();
    }

    auto& board = ctx().TheBoard;

    if (timer(TIMER_ENEMY_TEX1).expired()) {
        timer(TIMER_ENEMY_TEX1).reset();
        enemy_frame_tex1 = (enemy_frame_tex1 + 1) % 2;
    }

    if (timer(TIMER_ENEMY_TEX2).expired()) {
        timer(TIMER_ENEMY_TEX2).reset();
        enemy_frame_tex2 = (enemy_frame_tex2 + 1) % 2;
    }

    render_game_board();
    render_game_board_texture();

    // Currently 'starting up'
    if (!timer(TIMER_BOARD_BUILDING).expired()) {
        return;
    }

    noise_max_row = GAME_BOARD_HEIGHT;
    noise_max_col = GAME_BOARD_WIDTH;

    if (timer(TIMER_GRAVITY_TICK).expired()) {
        timer(TIMER_GRAVITY_TICK).reset();
        entity_updates_from_gravity = board.tick_gravity();
    }

    // don't want to break things while pieces are falling
    if (timer(TIMER_ENT_BREAK_TICK).expired() && entity_updates_from_gravity == 0) {
        timer(TIMER_ENT_BREAK_TICK).reset();
        entity_updates_from_gravity = board.tick_gravity();
        if (entity_updates_from_gravity == 0) {
            entities_broken_last_frame = board.break_pieces();
        }
    }

    if (entity_updates_from_gravity > 0 || entities_broken_last_frame > 0) {
        return;
    }

    if (board.is_game_over() || board.enemy_count() == 0) {
        ctx().RequestedScene = Scene::GameFinished;
    }
}

namespace {

void first_tick_setup(void) {
    noise_max_row = 1;
    noise_max_col = 1;

    // clang-format off
    timer(TIMER_BOARD_BUILDING) = Timer{ 15.0F , 1.0F , 15.0F };
    timer(TIMER_ENEMY_TEX1)     = Timer{ 0.2F  , 1.0F , 0.2F  };
    timer(TIMER_ENEMY_TEX2)     = Timer{ 0.2F  , 1.35F, 0.2F  };
    timer(TIMER_PIECE_DROP)     = Timer{ 1.0F  , 1.0F , 1.0F  };
    timer(TIMER_PIECE_HMOVE)    = Timer{ 1.0F  , 10.0F, 1.0F  };
    timer(TIMER_PIECE_VMOVE)    = Timer{ 1.0F  , 10.0F, 1.0F  };
    timer(TIMER_NOISY_BOARD)    = Timer{ 0.3F  , 1.0F , 0.3F  };
    timer(TIMER_GRAVITY_TICK)   = Timer{ 0.33F , 1.0F , 0.33F };
    timer(TIMER_ENT_BREAK_TICK) = Timer{ 0.66F , 1.0F , 0.66F };
    // clang-format on

    // initialise the game board with the current difficulty settings
    ctx().TheBoard.init_board(
        BoardInitParams::create_difficulty(
            ctx().CurrentLevel,
            ctx().AllowPills,
            ctx().AllowBlocks
        ),
        rng()
    );
}

void render_game_board(void) {
    if (timer(TIMER_NOISY_BOARD).expired()) {
        timer(TIMER_NOISY_BOARD).reset();
        noise_max_row = std::min(noise_max_row + 1, static_cast<int32_t>(GAME_BOARD_HEIGHT));
        noise_max_col = std::min(noise_max_col + 1, static_cast<int32_t>(GAME_BOARD_WIDTH));
        if (noise_max_row == GAME_BOARD_HEIGHT) {
            timer(TIMER_BOARD_BUILDING).Value = 0.0F;
        }
    }

    auto* renderer = ctx().Renderer;
    auto* prev_texture = SDL_GetRenderTarget(renderer);

    SDL_SetRenderTarget(renderer, ctx().GameplayTexture);
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    const PillGameBoard& board = ctx().TheBoard;

    for (int32_t row = 0; row < noise_max_row; row++) {
        int32_t flipped_row = (GAME_BOARD_HEIGHT - 1) - row;
        for (int32_t col = 0; col < noise_max_col; col++) {
            const auto& ent = board(row, col);
            if (ent.is_empty()) {
                continue;
            }

            SDL_FRect dst{col * CELL_SIZE, flipped_row * CELL_SIZE, CELL_SIZE, CELL_SIZE};
            SDL_FRect src{};
            int32_t rotation = 0;

            if (ent.is_pill()) {
                src = ctx().AssetBounds[ASSET_INDEX_PILL].as<SDL_FRect>();
                if (ent.EntityType == ETYPE_SPILL) {
                    src.x += CELL_SIZE;
                } else if (ent.EntityType == ETYPE_BROKEN) {
                    src.x += CELL_SIZE * 2.0F;
                }

            } else if (ent.is_enemy()) {
                bool is_alt_enemy = ((row + col) % 2 != 0);
                src = enemy_texture_pos(
                    is_alt_enemy,
                    is_alt_enemy ? enemy_frame_tex2 : enemy_frame_tex1
                );
            }

            src.w = CELL_SIZE;
            src.h = CELL_SIZE;

            Colour colour = ent.colour();
            SDL_SetTextureColorMod(atlas(), colour.Red, colour.Green, colour.Blue);
            if (ent.Rotation != 0) {
                SDL_RenderTextureRotated(
                    renderer,
                    atlas(),
                    &src,
                    &dst,
                    static_cast<double>(ent.Rotation * 90.0),
                    nullptr,
                    SDL_FLIP_NONE
                );
            } else {
                SDL_RenderTexture(renderer, atlas(), &src, &dst);
            }
            SDL_SetTextureColorMod(atlas(), 255, 255, 255);
        }
    }

    SDL_SetRenderTarget(renderer, prev_texture);
}

void render_game_board_texture(void) {
    auto* renderer = ctx().Renderer;

    int32_t iwidth, iheight;
    SDL_GetWindowSizeInPixels(ctx().Window, &iwidth, &iheight);

    float width = static_cast<float>(iwidth);
    float height = static_cast<float>(iheight);

    // Centre draw the gameboard texture
    float board_width = (CELL_SIZE + CELL_SIZE * 0.25F) * static_cast<float>(GAME_BOARD_WIDTH);
    float board_height = (CELL_SIZE + CELL_SIZE * 0.25F) * static_cast<float>(GAME_BOARD_HEIGHT);

    // If this is the case just try to fit it into the window
    if (board_height > height) {
        board_height = (CELL_SIZE * (height / CELL_SIZE)) - CELL_SIZE;
    }

    SDL_FRect dest{
        (width * 0.5F) - (board_width * 0.5F),
        std::max(
            (height * 0.5F) - (board_height * 0.5F),
            CELL_SIZE * 0.5F
        ),
        board_width,
        board_height,
    };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderTexture(renderer, ctx().GameplayTexture, nullptr, &dest);
}

}  // namespace

}  // namespace pill_game::game