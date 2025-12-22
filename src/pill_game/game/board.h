//
// Date       : 18/12/2025
// Project    : pill_game
// Author     : -Ry
//

#pragma once

#include "pill_game/pch.h"

namespace pill_game {

// NOTE
//  Arrays are Bottom to Top, that is, (0,0) is the Bottom Left
//

struct BoardInitParams {
    std::array<uint8_t, GAME_BOARD_HEIGHT> MaxEntitiesPerRow{0};
    std::array<uint8_t, GAME_BOARD_HEIGHT> EnemyChancePerRow{0};
    std::array<uint8_t, GAME_BOARD_HEIGHT> PillChancePerRow{0};
    std::array<uint8_t, GAME_BOARD_HEIGHT> BlockChancePerRow{0};

    void print_init_params() noexcept;

    static BoardInitParams create_difficulty(
        uint8_t level,
        bool allow_pills,
        bool allow_blocks
    ) noexcept;
};

class PillGameBoard {
   private:
    std::array<BoardEntity, GAME_BOARD_SIZE> m_FlatGameBoard{EMPTY_ENTITY};

   public:
    explicit PillGameBoard() noexcept = default;
    ~PillGameBoard() noexcept = default;

   public:
    PillGameBoard(const PillGameBoard&) noexcept = default;
    PillGameBoard(PillGameBoard&&) noexcept = default;
    PillGameBoard& operator=(const PillGameBoard&) noexcept = default;
    PillGameBoard& operator=(PillGameBoard&&) noexcept = default;

   public:
    const auto& flat_game_board() const noexcept { return m_FlatGameBoard; }
    auto& flat_game_board() noexcept { return m_FlatGameBoard; }

   public:
    void init_board(const BoardInitParams& params, std::mt19937& rng) noexcept;

   public:
    uint32_t enemy_count() const noexcept;
    bool is_game_over() const noexcept;

   public:
    // Checks to see if the given piece can drop to the next row
    bool can_piece_drop(const BoardPiece& piece) const noexcept;
    bool can_place_piece(const BoardPiece& piece) const noexcept;
    void place_piece(const BoardPiece& piece) noexcept;
    void remove_piece(const BoardPiece& piece) noexcept;

   public:
    const BoardEntity& operator()(uint32_t row, uint32_t col) const {
        return m_FlatGameBoard.at((row * GAME_BOARD_WIDTH) + col);
    }
    BoardEntity& operator()(uint32_t row, uint32_t col) {
        return m_FlatGameBoard.at((row * GAME_BOARD_WIDTH) + col);
    }

    const BoardEntity& operator()(const std::tuple<uint8_t, uint8_t>& pos) const noexcept {
        return this->operator()(
            static_cast<uint32_t>(std::get<0>(pos)),
            static_cast<uint32_t>(std::get<1>(pos))
        );
    }

    BoardEntity& operator()(const std::tuple<uint8_t, uint8_t>& pos) noexcept {
        return this->operator()(
            static_cast<uint32_t>(std::get<0>(pos)),
            static_cast<uint32_t>(std::get<1>(pos))
        );
    }
};

}  // namespace pill_game
