//
// Date       : 22/12/2025
// Project    : pill_game
// Author     : -Ry
//

#pragma once

#include <cstdint>

namespace pill_game {

using std::uint8_t;

struct BoardEntity {
    uint8_t Colour : 3 {0};      // Colour Index; 1..7
    uint8_t EntityType : 3 {0};  // refer to EType constants
    uint8_t Rotation : 2 {0};    // 0=0, 1=90, 2=180, 3=270
    // clang-format on

    constexpr bool is_empty() const noexcept {
        return *this == BoardEntity{};
    }

    constexpr bool operator==(const BoardEntity& rhs) const noexcept {
        return Colour == rhs.Colour
               && EntityType == rhs.EntityType;
    }

    constexpr bool operator!=(const BoardEntity& rhs) const noexcept { return !(*this == rhs); }

    uint32_t colour() const;
    bool has_gravity() const noexcept;
    bool is_enemy() const noexcept;
    bool is_pill() const noexcept;
    bool is_breakable() const noexcept;
};

}  // namespace pill_game