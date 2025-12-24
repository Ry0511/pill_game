//
// Date       : 22/12/2025
// Project    : pill_game
// Author     : -Ry
//

#include "pill_game/pch.h"
#include "pill_game/game/board_piece.h"
#include "pill_game/game/board.h"

namespace pill_game {

namespace {

uint8_t opposite_rotation(uint8_t rotate) {
    // clang-format off
    switch (rotate) {
        case ROTATE_NORTH: return ROTATE_SOUTH; break;
        case ROTATE_SOUTH: return ROTATE_NORTH; break;
        case ROTATE_EAST: return ROTATE_WEST; break;
        case ROTATE_WEST: return ROTATE_EAST; break;
    }
    // clang-format on
    return rotate;
}

}  // namespace

BoardPiece::BoardPiece()
    : Left(PILL_RED_E),
      Right(PILL_CYAN_W),
      Rotation(ROTATE_EAST),
      Row(GAME_BOARD_TOP_ROW),
      Column(GAME_BOARD_CENTRE) {
}

BoardPiece::BoardPiece(
    const PillGameBoard& board,
    uint32_t row,
    uint32_t col
) noexcept
    : Row(static_cast<int8_t>(row)), Column(static_cast<int8_t>(col)) {
    Left = board(row, col);
    Rotation = Left.Rotation;
    Right = board(right_piece_pos());
}

std::tuple<int8_t, int8_t> BoardPiece::left_piece_pos() const noexcept {
    return std::make_tuple(Row, Column);
}

std::tuple<int8_t, int8_t> BoardPiece::right_piece_pos() const noexcept {
    int8_t row{static_cast<int8_t>(Row)};
    int8_t col{static_cast<int8_t>(Column)};

    // clang-format off
    switch (Rotation) {
        case ROTATE_NORTH: row += 1; break;
        case ROTATE_SOUTH: row -= 1; break;
        case ROTATE_EAST : col += 1; break;
        case ROTATE_WEST : col -= 1; break;
    }
    // clang-format on

    return std::make_tuple(row, col);
}

void BoardPiece::move_left(const PillGameBoard& board) noexcept {
    if (!board.can_place_piece(*this) || Column <= 0) {
        return;
    }

    --Column;
    if (!board.can_place_piece(*this)) {
        ++Column;
    }
}

void BoardPiece::move_right(const PillGameBoard& board) noexcept {
    if (!board.can_place_piece(*this) || std::cmp_greater_equal(Column, GAME_BOARD_WIDTH)) {
        return;
    }

    ++Column;
    if (!board.can_place_piece(*this)) {
        --Column;
    }
}

void BoardPiece::rotate_piece_clockwise(const PillGameBoard& board) noexcept {
    if (!board.can_place_piece(*this)) {
        return;
    }

    BoardPiece p{*this};
    p.rotate_piece(true);

    if (!board.can_place_piece(p)) {
        p.shift_piece();
    }

    if (!board.can_place_piece(p)) {
        return;
    }

    p.Left.Rotation = p.Rotation;
    p.Right.Rotation = opposite_rotation(p.Rotation);
    *this = p;
}

void BoardPiece::rotate_piece_counter_clockwise(const PillGameBoard& board) noexcept {
    if (!board.can_place_piece(*this)) {
        return;
    }

    BoardPiece p{*this};
    p.rotate_piece(false);

    if (!board.can_place_piece(p)) {
        p.shift_piece();
    }

    if (!board.can_place_piece(p)) {
        return;
    }

    p.Left.Rotation = p.Rotation;
    p.Right.Rotation = opposite_rotation(p.Rotation);
    *this = p;
}

void BoardPiece::rotate_piece(bool clockwise) noexcept {

    if (clockwise) {
        Rotation = (Rotation + 1U) % 4U;
        return;
    }

    if (Rotation == 0) {
        Rotation = 3;
    } else {
        Rotation--;
    }
}

void BoardPiece::shift_piece() noexcept {
    int8_t row_shift{0};
    int8_t col_shift{0};

    //
    // X X X    X X X    X X X    G X X    X X X
    // R G X -> R X X -> G R X -> R X X -> R G X
    // X X X    G X X    X X X    X X X    X X X
    //
    // R G X    R X X    G R X    X G X    R G X
    // X X X -> G X X -> X X X -> X R X -> X X X
    // X X X    X X X    X X X    X X X    X X X
    //

    // clang-format off
    switch (Rotation) {
        case ROTATE_NORTH: row_shift = {-1}; break;
        case ROTATE_SOUTH: row_shift = {+1}; break;
        case ROTATE_EAST : col_shift = {-1}; break;
        case ROTATE_WEST : col_shift = {+1}; break;
    }
    // clang-format on

    Row = static_cast<int8_t>(Row + row_shift);
    Column = static_cast<int8_t>(Column + col_shift);
}

}  // namespace pill_game