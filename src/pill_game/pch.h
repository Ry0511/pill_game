//
// Date       : 18/12/2025
// Project    : pill_game
// Author     : -Ry
//

#pragma once

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <cctype>
#include <charconv>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <cwctype>
#include <random>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <bitset>

#include "util/logging.h"

namespace pill_game {

using namespace pill_game::logging;

// NOLINTNEXTLINE
namespace fs = std::filesystem;

using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::int8_t;

using std::uint16_t;
using std::uint32_t;
using std::uint8_t;

constexpr size_t GAME_BOARD_WIDTH = 8;
constexpr size_t GAME_BOARD_HEIGHT = 16;
constexpr size_t GAME_BOARD_SIZE = GAME_BOARD_WIDTH * GAME_BOARD_HEIGHT;
static_assert(GAME_BOARD_WIDTH <= std::numeric_limits<uint8_t>::max());
static_assert(GAME_BOARD_HEIGHT <= std::numeric_limits<uint8_t>::max());

// clang-format off
constexpr uint32_t COLOUR_RED    = 0x970054FF;
constexpr uint32_t COLOUR_CYAN   = 0x3078E7FF;
constexpr uint32_t COLOUR_YELLOW = 0xB0A41AFF;
constexpr uint32_t COLOUR_GREEN  = 0x7CFC00FF;
constexpr uint32_t COLOUR_BLUE   = 0x0000FFFF;
constexpr uint32_t COLOUR_BLACK  = 0x000000FF;
constexpr uint32_t COLOUR_WHITE  = 0xFFFAFAFF;

// BoardEntity.Colour indexes into this array
constexpr std::array<uint32_t, 7> ENTITY_COLOURS{
    COLOUR_RED,
    COLOUR_CYAN,
    COLOUR_YELLOW,
    COLOUR_GREEN,
    COLOUR_BLUE,
    COLOUR_WHITE,
    COLOUR_BLACK,
};

constexpr uint8_t ETYPE_NONE   = 0;  // Empty type
constexpr uint8_t ETYPE_ENEMY  = 1;  // Enemy type
constexpr uint8_t ETYPE_BLOCK  = 2;  // Unbreakable block
constexpr uint8_t ETYPE_PILL   = 5;  // Full Pill type
constexpr uint8_t ETYPE_SPILL  = 6;  // Single Pill type
constexpr uint8_t ETYPE_GBLOCK = 7;  // Unbreakable Gravity block

constexpr uint8_t ROTATE_NORTH = 0;
constexpr uint8_t ROTATE_EAST  = 1;
constexpr uint8_t ROTATE_SOUTH = 2;
constexpr uint8_t ROTATE_WEST  = 3;

struct BoardEntity {
    uint8_t Colour     : 3{0};  // Colour Index; 1..7
    uint8_t EntityType : 3{0};  // refer to EType constants
    uint8_t Rotation   : 2{0};  // 0=0, 1=90, 2=180, 3=270
    // clang-format on

    constexpr bool is_empty() const noexcept {
        return *this == BoardEntity{};
    }

    constexpr bool operator==(const BoardEntity& rhs) const noexcept {
        return Colour == rhs.Colour
               && EntityType == rhs.EntityType;
    }
    constexpr bool operator!=(const BoardEntity& rhs) const noexcept { return !(*this == rhs); }

    constexpr uint32_t colour() const { return ENTITY_COLOURS.at(Colour); }

    constexpr bool has_gravity() const noexcept {
        return EntityType >= ETYPE_PILL;
    }

    constexpr bool is_enemy() const noexcept {
        return EntityType == ETYPE_ENEMY;
    }

    constexpr bool is_pill() const noexcept {
        return EntityType == ETYPE_PILL || EntityType == ETYPE_SPILL;
    }
};

constexpr BoardEntity EMPTY_ENTITY{0, 0, 0};

// Can't make a morpheus joke without the blue pill :sadge:
constexpr BoardEntity RED_PILL{0, ETYPE_PILL, ROTATE_NORTH};
constexpr BoardEntity CYAN_PILL{1, ETYPE_PILL, ROTATE_NORTH};
constexpr BoardEntity YELLOW_PILL{2, ETYPE_PILL, ROTATE_NORTH};

struct BoardPiece {
    BoardEntity Left{EMPTY_ENTITY};
    BoardEntity Right{EMPTY_ENTITY};
    uint8_t Rotation{0};
    uint8_t Column{0};

    constexpr BoardPiece() noexcept = default;

    constexpr BoardPiece(const BoardEntity& left, const BoardEntity& right) noexcept
        : Left{left}, Right{right} {
        Left.Rotation = ROTATE_EAST;
        Right.Rotation = ROTATE_WEST;
    };
};

constexpr std::array<BoardPiece, 9> ALL_PIECES{
    BoardPiece{   RED_PILL,    RED_PILL},
    BoardPiece{   RED_PILL,   CYAN_PILL},
    BoardPiece{   RED_PILL, YELLOW_PILL},

    BoardPiece{  CYAN_PILL,    RED_PILL},
    BoardPiece{  CYAN_PILL,   CYAN_PILL},
    BoardPiece{  CYAN_PILL, YELLOW_PILL},

    BoardPiece{YELLOW_PILL,    RED_PILL},
    BoardPiece{YELLOW_PILL,   CYAN_PILL},
    BoardPiece{YELLOW_PILL, YELLOW_PILL},
};

}  // namespace pill_game
