//
// Date       : 22/12/2025
// Project    : pill_game
// Author     : -Ry
//
#pragma once

#include <cstdint>
#include <tuple>

#include "pill_game/game/board_entity.h"

namespace pill_game {

class PillGameBoard;

using std::uint8_t;

struct BoardPiece {
    BoardEntity Left{};
    BoardEntity Right{};
    uint8_t Rotation{};
    int8_t Row{};
    int8_t Column{};

    BoardPiece();

    constexpr BoardPiece(
        BoardEntity left,
        BoardEntity right,
        uint8_t rotation,
        int8_t row,
        int8_t column
    ) noexcept
        : Left{left}, Right{right}, Rotation{rotation}, Row{row}, Column(column) {};

    BoardPiece(const PillGameBoard& board, uint32_t row, uint32_t col) noexcept;

    std::tuple<int8_t, int8_t> left_piece_pos() const noexcept;
    std::tuple<int8_t, int8_t> right_piece_pos() const noexcept;

    void move_left(const PillGameBoard& board) noexcept;
    void move_right(const PillGameBoard& board) noexcept;
    void rotate_piece_clockwise(const PillGameBoard& board) noexcept;
    void rotate_piece_counter_clockwise(const PillGameBoard& board) noexcept;
    void rotate_piece(bool clockwise) noexcept;
    void shift_piece() noexcept;
};

}  // namespace pill_game
