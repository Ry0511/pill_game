//
// Date       : 19/12/2025
// Project    : pill_game
// Author     : -Ry
//

#include "pill_game/pch.h"
#include "pill_game/game/board.h"

namespace pill_game {

uint32_t PillGameBoard::enemy_count() const noexcept {
    uint32_t count = 0;
    for (const auto& entity : m_FlatGameBoard) {
        if (entity.EntityType == ETYPE_ENEMY) {
            ++count;
        }
    }
    return count;
}

}  // namespace pill_game