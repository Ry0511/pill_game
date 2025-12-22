//
// Date       : 21/12/2025
// Project    : pill_game
// Author     : -Ry
//

#include "pill_game/pch.h"
#include "pill_game/game.h"
#include "pill_game/game/board.h"

#include "SDL3/SDL.h"

#include "vendor/stb_image.h"
#include "vendor/stb_rect_pack.h"

namespace pill_game::game {

namespace {

// TODO: Need to split out the game logic and the drawing logic

int application_loop(void);

void process_events(void);
void process_controller_input(void);
void draw_game_board(void);

void load_sprite_atlas(void);
void init_background_texture(void);
void init_gameboard_texture(void);

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

struct Colour { uint8_t r, g, b, a; };
struct Vec2   { float x, y;         };

// clang-format on

// All board sprites are assumed to be 32x32
constexpr float board_sprite_size = 32.0F;
constexpr float max_piece_move_speed = 1.0F / 8.0F;
constexpr float gravity_tick_rate = 1.0F / 15.0F;

float cell_size = 32.0F;
bool app_should_continue{true};

Controller controller{};
SDL_Window* app_window{nullptr};
SDL_Renderer* renderer{nullptr};

SDL_Texture* background_texture{nullptr};
SDL_Texture* gameboard_texture{nullptr};

SDL_Texture* sprite_atlas{nullptr};
Vec2 enemy_offset{};
Vec2 pill_offset{};

bool ticking_gravity{false};
float gravity_timer{0.0F};
int32_t gravity_affected_pieces{0};
int32_t pieces_popped{0};

BoardPiece current_piece{};
float piece_drop_timer{};
float piece_speed_factor{3.0F};
float piece_move_timer{max_piece_move_speed};

bool enemy_sprites_use_frame_two{false};

uint8_t current_level = 5;
BoardInitParams the_init_params = BoardInitParams::create_difficulty(current_level, false, false);
PillGameBoard the_board{};
std::mt19937 the_random{};

struct RawImage {
    uint8_t* Data{nullptr};
    int32_t Width{0};
    int32_t Height{0};
    int32_t Channels{0};

    explicit RawImage(const fs::path& path) {
        stbi_set_flip_vertically_on_load(false);
        Data = stbi_load(path.string().c_str(), &Width, &Height, &Channels, 4);
    }

    ~RawImage() {
        if (Data) {
            stbi_image_free(Data);
        }
    }

    RawImage(const RawImage& other) = delete;
    RawImage& operator=(const RawImage& other) = delete;

    explicit RawImage(RawImage&& other) noexcept {
        if (Data) {
            stbi_image_free(Data);
        }
        Data = other.Data;
        Width = other.Width;
        Height = other.Height;
        Channels = other.Channels;
    }

    RawImage& operator=(RawImage&& other) noexcept {
        if (Data) {
            stbi_image_free(Data);
        }
        Data = other.Data;
        Width = other.Width;
        Height = other.Height;
        Channels = other.Channels;
        return *this;
    }
};

}  // namespace

int run_application() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        PG_LOG(Err, "Failed to initialize SDL - {}", SDL_GetError());
        return -1;
    }

    if (!SDL_CreateWindowAndRenderer(
            "Pill Game",
            1280,
            768,
            SDL_WINDOW_RESIZABLE,
            &app_window,
            &renderer
        )) {
        PG_LOG(Err, "Failed to create window or renderer - {}", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_SetWindowMinimumSize(app_window, 600, 400);

    return application_loop();
}

namespace {

int application_loop(void) {
    load_sprite_atlas();

    the_random = std::mt19937{std::random_device{}()};
    the_board.init_board(the_init_params, the_random);

    SDL_ShowWindow(app_window);

    constexpr uint64_t target_tickrate = 4;
    uint64_t last_ticks = SDL_GetTicks();
    uint64_t tick_diff{0};
    float delta = 0.0F;
    float frame_theta = 0.0F;

    init_background_texture();
    init_gameboard_texture();

    while (app_should_continue) {
        // reset
        process_events();

        if (the_board.is_game_over()) {
            the_board.init_board(the_init_params, the_random);
        }

        uint64_t current_ticks = SDL_GetTicks();
        tick_diff = current_ticks - last_ticks;
        delta = static_cast<float>(tick_diff) / 1000.0F;
        last_ticks = current_ticks;
        frame_theta += delta;

        if (frame_theta >= 1.0F) {
            enemy_sprites_use_frame_two = !enemy_sprites_use_frame_two;
            frame_theta = 0.0F;
        }

        int iwidth{0};
        int iheight{0};
        float width{0.0F};
        float height{0.0F};
        SDL_GetWindowSizeInPixels(app_window, &iwidth, &iheight);
        width = static_cast<float>(iwidth);
        height = static_cast<float>(iheight);

        gravity_timer = std::max(gravity_timer - delta, -1.0F);

        if (ticking_gravity) {
            if (gravity_timer <= 0.0F) {
                gravity_affected_pieces = the_board.tick_gravity();
                gravity_timer = gravity_tick_rate;
            }
            ticking_gravity = (gravity_affected_pieces <= 0);
        } else if (gravity_affected_pieces <= 0) {
            ticking_gravity = false;
        }

        if (!ticking_gravity) {
            piece_move_timer -= delta;
            process_controller_input();

            piece_drop_timer += delta * piece_speed_factor;
            if (piece_drop_timer >= 1.0F) {
                piece_drop_timer = -0.5F;

                if (the_board.can_piece_drop(current_piece)) {
                    current_piece.Row -= 1;
                } else {
                    the_board.place_piece(current_piece);
                    ticking_gravity = true;
                    gravity_timer = -1.0F;
                    current_piece = {};
                }
            }
        }

        draw_game_board();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTextureTiled(renderer, background_texture, nullptr, 32.0F, nullptr);

        // Centre draw the gameboard texture
        float board_width = (cell_size + cell_size * 0.25F) * static_cast<float>(GAME_BOARD_WIDTH);
        float board_height = (cell_size + cell_size * 0.25F) * static_cast<float>(GAME_BOARD_HEIGHT);

        // If this is the case just try to fit it into the window
        if (board_height > height) {
            board_height = (cell_size * (height / cell_size)) - cell_size;
        }

        SDL_FRect board_dest{
            (width * 0.5F) - (board_width * 0.5F),
            std::max(
                (height * 0.5F) - (board_height * 0.5F),
                cell_size * 0.5F
            ),
            board_width,
            board_height,
        };
        SDL_RenderTexture(renderer, gameboard_texture, nullptr, &board_dest);

        SDL_RenderPresent(renderer);

        // simple framerate cap
        auto diff = SDL_GetTicks() - current_ticks;
        if (diff < target_tickrate) {
            SDL_Delay(static_cast<uint32_t>(target_tickrate - diff));
        }
    }

    SDL_DestroyTexture(background_texture);
    SDL_DestroyTexture(gameboard_texture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(app_window);
    SDL_Quit();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// | APPLICATION LOOP FUNCTIONS |
////////////////////////////////////////////////////////////////////////////////

void process_events(void) {
    SDL_Event event{};

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                app_should_continue = false;
                break;
            case SDL_EVENT_KEY_DOWN:

                // clang-format off
                switch (event.key.key) {
                case SDLK_UP    : controller.Up    = 1; break;
                case SDLK_DOWN  : controller.Down  = 1; break;
                case SDLK_LEFT  : controller.Left  = 1; break;
                case SDLK_RIGHT : controller.Right = 1; break;
                case SDLK_X     : controller.A     = 1; break;
                case SDLK_Z     : controller.B     = 1; break;
                case SDLK_ESCAPE: controller.Pause = 1; break;
                case SDLK_RETURN: controller.Start = 1; break;
                }
                // clang-format on

                if (event.key.key == SDLK_SPACE) {
                    the_board.init_board(the_init_params, the_random);
                }

                break;
            case SDL_EVENT_KEY_UP:
                // clang-format off
                switch (event.key.key) {
                case SDLK_UP    : controller.Up    = 0; break;
                case SDLK_DOWN  : controller.Down  = 0; break;
                case SDLK_LEFT  : controller.Left  = 0; break;
                case SDLK_RIGHT : controller.Right = 0; break;
                case SDLK_X     : controller.A     = 0; break;
                case SDLK_Z     : controller.B     = 0; break;
                case SDLK_ESCAPE: controller.Pause = 0; break;
                case SDLK_RETURN: controller.Start = 0; break;
                }
                // clang-format on
                break;
            default:
                break;
        }
    }
}

void process_controller_input(void) {
    if (controller.Left && piece_move_timer <= 0.0F) {
        current_piece.move_left(the_board);
        piece_move_timer = max_piece_move_speed;
    }

    if (controller.Right && piece_move_timer <= 0.0F) {
        current_piece.move_right(the_board);
        piece_move_timer = max_piece_move_speed;
    }

    if (controller.A) {
        current_piece.rotate_piece_clockwise(the_board);
        controller.A = 0;
    }

    if (controller.B) {
        current_piece.rotate_piece_counter_clockwise(the_board);
        controller.B = 0;
    }

    if (controller.Down) {
        piece_speed_factor = 15.0F;
    } else {
        piece_speed_factor = 1.0F;
    }
}

void draw_game_board(void) {
    auto* original_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, gameboard_texture);
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    auto draw_piece = [](uint32_t row, uint32_t col, const BoardEntity& entity) -> void {
        uint32_t colour = entity.colour();
        uint8_t r = (colour & 0xFF000000) >> 24;
        uint8_t g = (colour & 0x00FF0000) >> 16;
        uint8_t b = (colour & 0x0000FF00) >> 8;
        uint8_t a = (colour & 0x000000FF);

        uint32_t flipped_row = (GAME_BOARD_HEIGHT - 1) - row;
        SDL_FRect rect{
            static_cast<float>(col) * cell_size,
            static_cast<float>(flipped_row) * cell_size,
            cell_size,
            cell_size
        };

        bool requires_rotation = entity.EntityType == ETYPE_PILL;
        float yoffset = requires_rotation ? 0.0F : 32.0F;

        SDL_FRect img_src{
            pill_offset.x,
            pill_offset.y + yoffset,
            board_sprite_size,
            board_sprite_size,
        };
        SDL_SetTextureColorMod(sprite_atlas, r, g, b);

        if (!requires_rotation) {
            SDL_RenderTexture(renderer, sprite_atlas, &img_src, &rect);
        } else {
            SDL_RenderTextureRotated(
                renderer,
                sprite_atlas,
                &img_src,
                &rect,
                90.0 * static_cast<double>(entity.Rotation),
                nullptr,
                SDL_FLIP_NONE
            );
        }
    };

    const auto& [lr, lc] = current_piece.left_piece_pos();
    draw_piece(lr, lc, current_piece.Left);

    const auto& [rr, rc] = current_piece.right_piece_pos();
    draw_piece(rr, rc, current_piece.Right);

    for (uint32_t row = 0; row < GAME_BOARD_HEIGHT; ++row) {
        uint32_t flipped_row = (GAME_BOARD_HEIGHT - 1) - row;

        for (uint32_t col = 0; col < GAME_BOARD_WIDTH; ++col) {
            const BoardEntity& entity = the_board(row, col);

            if (entity.EntityType == ETYPE_NONE) {
                continue;
            }

            uint32_t colour = entity.colour();
            uint8_t r = (colour & 0xFF000000) >> 24;
            uint8_t g = (colour & 0x00FF0000) >> 16;
            uint8_t b = (colour & 0x0000FF00) >> 8;
            uint8_t a = (colour & 0x000000FF);

            SDL_FRect rect{
                static_cast<float>(col) * cell_size,
                static_cast<float>(flipped_row) * cell_size,
                cell_size,
                cell_size
            };

            if (entity.is_enemy()) {
                bool is_alt_enemy = (entity.Colour % 2 == 0);
                float xoffset = (enemy_sprites_use_frame_two) ? 32.0F : 0.0F;
                float yoffset = is_alt_enemy ? 32.0F : 0.0F;

                SDL_FRect img_src{
                    enemy_offset.x + xoffset,
                    enemy_offset.y + yoffset,
                    board_sprite_size,
                    board_sprite_size,
                };
                SDL_SetTextureColorMod(sprite_atlas, r, g, b);
                SDL_RenderTexture(renderer, sprite_atlas, &img_src, &rect);
            } else if (entity.is_pill()) {
                draw_piece(row, col, entity);
            }
        }
    }

    SDL_SetRenderTarget(renderer, original_target);
}

////////////////////////////////////////////////////////////////////////////////
// | LOAD FUNCTIONS |
////////////////////////////////////////////////////////////////////////////////

void load_sprite_atlas(void) {
    const fs::path assets_path = fs::current_path() / "assets";

    RawImage enemy_image{assets_path / "Enemy2_32x32.png"};
    RawImage pill_image{assets_path / "Piece_SpriteSheet.png"};

    constexpr int32_t size = 256;
    stbrp_node nodes[size]{};

    stbrp_context ctx{};
    stbrp_init_target(&ctx, size, size, &nodes[0], std::size(nodes));

    stbrp_rect rects_to_pack[]{
        stbrp_rect{0, enemy_image.Width, enemy_image.Height},
        stbrp_rect{1,  pill_image.Width,  pill_image.Height},
    };

    stbrp_pack_rects(&ctx, &rects_to_pack[0], std::size(rects_to_pack));

    int max_width = std::max(enemy_image.Width, pill_image.Width);
    int max_height = std::max(enemy_image.Height, pill_image.Height);
    sprite_atlas = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STATIC,
        size,
        size
    );

    auto pack_into_atlas = [](const RawImage& img, const stbrp_rect& rect) -> void {
        if (!rect.was_packed) {
            PG_LOG(Err, "Failed to pack rect {} {}x{}", rect.id, rect.w, rect.h);
        }
        SDL_Rect sdl_rect{rect.x, rect.y, rect.w, rect.h};

        for (int32_t i = 0; i < img.Width * img.Height * img.Channels; i += 4) {
            Colour& p = *reinterpret_cast<Colour*>(img.Data + i);
            if (p.r != 0 || p.g != 0 || p.b != 0) {
                p = Colour{255, 255, 255, 255};
            } else if (p.a == 255) {
                p = Colour{0, 0, 0, 0};
            }
        }

        SDL_UpdateTexture(
            sprite_atlas,
            &sdl_rect,
            img.Data,
            img.Width * img.Channels
        );
    };

    pack_into_atlas(enemy_image, rects_to_pack[0]);
    enemy_offset = Vec2{
        static_cast<float>(rects_to_pack[0].x),
        static_cast<float>(rects_to_pack[0].y)
    };

    pack_into_atlas(pill_image, rects_to_pack[1]);
    pill_offset = Vec2{
        static_cast<float>(rects_to_pack[1].x),
        static_cast<float>(rects_to_pack[1].y)
    };
}

void init_background_texture(void) {
    constexpr size_t tex_size = 2;
    constexpr size_t channels = 4;

    const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888);
    uint32_t magenta = SDL_MapRGBA(fmt, nullptr, 91, 28, 119, 150);
    uint32_t img_data[tex_size * tex_size]{
        magenta,
        0,
        0,
        magenta
    };

    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        tex_size,
        tex_size,
        SDL_PIXELFORMAT_RGBA8888,
        &img_data[0],
        tex_size * channels
    );

    background_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureScaleMode(background_texture, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surface);
}

void init_gameboard_texture(void) {
    int32_t width = static_cast<int32_t>(cell_size) * static_cast<int32_t>(GAME_BOARD_WIDTH);
    int32_t height = static_cast<int32_t>(cell_size) * static_cast<int32_t>(GAME_BOARD_HEIGHT);
    gameboard_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
}

}  // namespace

}  // namespace pill_game::game