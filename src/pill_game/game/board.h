//
// Date       : 18/12/2025
// Project    : pill_game
// Author     : -Ry
//

#pragma once

#include "pill_game/pch.h"

namespace pill_game {

class PillGameBoard {
   public:
    static constexpr size_t game_board_width = 8;
    static constexpr size_t game_board_height = 16;
    static constexpr size_t game_board_size = game_board_width * game_board_height;

   private:
    std::array<BoardEntity, game_board_size> m_FlatGameBoard{EMPTY_ENTITY};

   public:
    explicit PillGameBoard() noexcept = default;
    ~PillGameBoard() noexcept = default;

   public:
    PillGameBoard(const PillGameBoard&) noexcept = default;
    PillGameBoard(PillGameBoard&&) noexcept = default;
    PillGameBoard& operator=(const PillGameBoard&) noexcept = default;
    PillGameBoard& operator=(PillGameBoard&&) noexcept = default;

   public:
    inline const auto& flat_game_board() const noexcept { return m_FlatGameBoard; }
    inline auto& flat_game_board() noexcept { return m_FlatGameBoard; }

   public:
    uint32_t enemy_count() const noexcept;

   public:
    inline const BoardEntity& operator()(uint32_t row, uint32_t col) const {
        return m_FlatGameBoard.at(row * game_board_width + col);
    }
    inline BoardEntity& operator()(uint32_t row, uint32_t col) {
        return m_FlatGameBoard.at(row * game_board_width + col);
    }
};

}  // namespace pill_game
