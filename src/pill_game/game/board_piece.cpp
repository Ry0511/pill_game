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
    auto i = 4;
    bool banned_rotate = false;

    do {
        Rotation = (Rotation + 1U) % 4U;
        --i;
        banned_rotate = (rotate == ROTATE_NORTH && Rotation == ROTATE_SOUTH)
                        || (rotate == ROTATE_SOUTH && Rotation == ROTATE_NORTH);

    } while (!board.can_place_piece(*this) && !banned_rotate && i > 0);

    if (!board.can_place_piece(*this) || banned_rotate) {
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
    auto i = 4;
    auto banned_rotate = false;

    do {
        if (Rotation == 0) {
            Rotation = 3U;
        } else {
            --Rotation;
        }
        --i;

        banned_rotate = (rotate == ROTATE_NORTH && Rotation == ROTATE_SOUTH)
                        || (rotate == ROTATE_SOUTH && Rotation == ROTATE_NORTH);
    } while (!board.can_place_piece(*this) && !banned_rotate && i > 0);

    if (!board.can_place_piece(*this) || banned_rotate) {
        Rotation = rotate;
        return;
    }

    Left.Rotation = Rotation;
    Right.Rotation = opposite_rotation(Rotation);
}

}  // namespace pill_game