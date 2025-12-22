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
    : Row(row), Column(col) {
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
        case ROTATE_NORTH: row = Row + 1; break;
        case ROTATE_SOUTH: row = Row - 1; break;
        case ROTATE_EAST : col = Column + 1; break;
        case ROTATE_WEST : col = Column - 1; break;
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
    if (!board.can_place_piece(*this) || Column >= GAME_BOARD_WIDTH) {
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

    auto rotate = Rotation;
    auto banned_multi_rotate = Right.Rotation;

    for (auto i = 0; i < 4; ++i) {
        Rotation = (Rotation + 1U) % 4U;

        bool banned_rotation = i > 0 && Rotation == banned_multi_rotate;
        if (!banned_rotation && board.can_place_piece(*this)) {
            break;
        }
    }

    if (!board.can_place_piece(*this)) {
        Rotation = rotate;
        return;
    }

    Left.Rotation = Rotation;
    Right.Rotation = opposite_rotation(Rotation);
}

void BoardPiece::rotate_piece_counter_clockwise(const PillGameBoard& board) noexcept {
    if (!board.can_place_piece(*this)) {
        return;
    }

    auto rotate = Rotation;
    auto banned_multi_rotate = Right.Rotation;

    for (auto i = 0; i < 4; ++i) {
        if (Rotation == 0U) {
            Rotation = 3U;
        } else {
            --Rotation;
        }

        bool banned_rotation = i > 0 && Rotation == banned_multi_rotate;
        if (!banned_rotation && board.can_place_piece(*this)) {
            break;
        }
    }

    if (!board.can_place_piece(*this)) {
        Rotation = rotate;
        return;
    }

    Left.Rotation = Rotation;
    Right.Rotation = opposite_rotation(Rotation);
}

}  // namespace pill_game