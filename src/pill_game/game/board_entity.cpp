//
// Date       : 22/12/2025
// Project    : pill_game
// Author     : -Ry
//

#include "pill_game\pch.h"
#include "pill_game\game\board_entity.h"

namespace pill_game {

uint32_t BoardEntity::colour() const {
    return ENTITY_COLOURS.at(Colour);
}

bool BoardEntity::has_gravity() const noexcept {
    return EntityType != ETYPE_ENEMY && EntityType != ETYPE_BLOCK;
}

bool BoardEntity::is_enemy() const noexcept {
    return EntityType == ETYPE_ENEMY;
}

bool BoardEntity::is_pill() const noexcept {
    return EntityType == ETYPE_PILL || EntityType == ETYPE_SPILL;
}

}  // namespace pill_game