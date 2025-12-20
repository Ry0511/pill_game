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

#include "pill_game/logging.h"

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

// clang-format off
constexpr uint32_t COLOUR_RED    = 0xC40233FF;
constexpr uint32_t COLOUR_CYAN   = 0x00CED1FF;
constexpr uint32_t COLOUR_YELLOW = 0xFDFF00FF;
constexpr uint32_t COLOUR_GREEN  = 0x7CFC00FF;
constexpr uint32_t COLOUR_BLUE   = 0x0000FFFF;
constexpr uint32_t COLOUR_BLACK  = 0x000000FF;
constexpr uint32_t COLOUR_WHITE  = 0xFFFAFAFF;

// BoardEntity.Colour indexes into this array
constexpr std::array<uint32_t, 7> ENTITY_COLOURS{
    COLOUR_BLACK,
    COLOUR_RED,
    COLOUR_CYAN,
    COLOUR_YELLOW,
    COLOUR_GREEN,
    COLOUR_BLUE,
    COLOUR_WHITE,
};

constexpr uint8_t ETYPE_NONE  = 0; // Empty type
constexpr uint8_t ETYPE_PIECE = 1; // Piece type
constexpr uint8_t ETYPE_ENEMY = 2; // Enemy type
constexpr uint8_t ETYPE_BLOCK = 3; // Unbreakable block

struct BoardEntity {
    uint8_t Colour     : 3{0};  // Colour Index; 1..7
    uint8_t EntityType : 2{0};  // refer to EType constants
    uint8_t Rotation   : 2{0};  // 0=0, 1=90, 2=180, 3=270
    uint8_t HasGravity : 1{0};  // Should gravity be applid to this entity?
    // clang-format on

    constexpr bool is_empty() const noexcept {
        return *this == BoardEntity{};
    }

    constexpr bool operator==(const BoardEntity& rhs) const noexcept {
        return Colour == rhs.Colour
               && EntityType == rhs.EntityType
               && HasGravity == rhs.HasGravity;
    }
    constexpr bool operator!=(const BoardEntity& rhs) const noexcept { return !(*this == rhs); }
    constexpr uint32_t colour() const { return ENTITY_COLOURS[Colour]; }
};

constexpr BoardEntity EMPTY_ENTITY{0,0,0,0};

}  // namespace pill_game
