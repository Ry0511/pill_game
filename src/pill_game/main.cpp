#include "pch.h"

#include "vendor/stb_image.h"
#include "vendor/stb_rect_pack.h"
#include "vendor/stb_truetype.h"

#include "pill_game/game/board.h"

using namespace pill_game;

int main() {
    PillGameBoard board{};
    std::mt19937 rng{std::random_device{}()};

    for (uint8_t level = 1; level <= 20; level++) {
        auto diff = BoardInitParams::create_difficulty(level, true, true);
        board.init_board(diff, rng);

        PG_LOG(Info, "{:->80}", ' ');
        diff.print_init_params();
        for (uint32_t i = 0; i < GAME_BOARD_HEIGHT; ++i) {
            std::stringstream ss{};
            for (uint32_t j = 0; j < GAME_BOARD_WIDTH; ++j) {
                ss << std::to_string(board(i, j).EntityType) << ' ';
            }
            PG_LOG(Info, "  {}", ss.str());
        }
        PG_LOG(Info, "{:->80}", ' ');
    }

    return 0;
}
