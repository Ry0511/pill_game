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

#include "pill_game/game/board_entity.h"
#include "pill_game/game/board_piece.h"

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

static_assert(GAME_BOARD_WIDTH >= 6 && GAME_BOARD_WIDTH < std::numeric_limits<uint8_t>::max());
static_assert(GAME_BOARD_HEIGHT >= 8 && GAME_BOARD_HEIGHT < std::numeric_limits<uint8_t>::max());

constexpr size_t GAME_BOARD_TOP_ROW = GAME_BOARD_HEIGHT - 1;
constexpr size_t GAME_BOARD_CENTRE = (GAME_BOARD_WIDTH - 1) / 2;

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

constexpr BoardEntity EMPTY_ENTITY{0, 0, 0};

// Can't make a morpheus joke without the blue pill :sadge:
constexpr BoardEntity PILL_RED_E{0, ETYPE_PILL, ROTATE_EAST};
constexpr BoardEntity PILL_RED_W{0, ETYPE_PILL, ROTATE_WEST};
constexpr BoardEntity PILL_CYAN_E{1, ETYPE_PILL, ROTATE_EAST};
constexpr BoardEntity PILL_CYAN_W{1, ETYPE_PILL, ROTATE_WEST};
constexpr BoardEntity PILL_YELLOW_E{2, ETYPE_PILL, ROTATE_EAST};
constexpr BoardEntity PILL_YELLOW_W{3, ETYPE_PILL, ROTATE_WEST};

constexpr std::array<BoardPiece, 9> ALL_PIECES{
    BoardPiece{   PILL_RED_E,    PILL_RED_W, ROTATE_EAST, GAME_BOARD_TOP_ROW, GAME_BOARD_CENTRE},
    BoardPiece{   PILL_RED_E,   PILL_CYAN_W, ROTATE_EAST, GAME_BOARD_TOP_ROW, GAME_BOARD_CENTRE},
    BoardPiece{   PILL_RED_E, PILL_YELLOW_W, ROTATE_EAST, GAME_BOARD_TOP_ROW, GAME_BOARD_CENTRE},
    BoardPiece{  PILL_CYAN_W,    PILL_RED_W, ROTATE_EAST, GAME_BOARD_TOP_ROW, GAME_BOARD_CENTRE},
    BoardPiece{  PILL_CYAN_W,   PILL_CYAN_W, ROTATE_EAST, GAME_BOARD_TOP_ROW, GAME_BOARD_CENTRE},
    BoardPiece{  PILL_CYAN_W, PILL_YELLOW_W, ROTATE_EAST, GAME_BOARD_TOP_ROW, GAME_BOARD_CENTRE},
    BoardPiece{PILL_YELLOW_W,    PILL_RED_W, ROTATE_EAST, GAME_BOARD_TOP_ROW, GAME_BOARD_CENTRE},
    BoardPiece{PILL_YELLOW_W,   PILL_CYAN_W, ROTATE_EAST, GAME_BOARD_TOP_ROW, GAME_BOARD_CENTRE},
    BoardPiece{PILL_YELLOW_W, PILL_YELLOW_W, ROTATE_EAST, GAME_BOARD_TOP_ROW, GAME_BOARD_CENTRE},
};

}  // namespace pill_game
