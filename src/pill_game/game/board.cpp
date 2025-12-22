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

bool PillGameBoard::is_game_over() const noexcept {
    constexpr uint32_t center_column = (GAME_BOARD_WIDTH - 1) / 2;
    constexpr uint32_t top_row = GAME_BOARD_HEIGHT - 1;
    return !this->operator()(top_row, center_column).is_empty()
           || !this->operator()(top_row, center_column + 1).is_empty();
}

void BoardInitParams::print_init_params() noexcept {
    for (uint8_t row = 0; row < GAME_BOARD_HEIGHT; ++row) {
        PG_LOG(
            Info,
            "  {:>3}, {:>3}, {:>3}, {:>3}",
            MaxEntitiesPerRow.at(row),
            EnemyChancePerRow.at(row),
            PillChancePerRow.at(row),
            BlockChancePerRow.at(row)
        );
    }
}

BoardInitParams BoardInitParams::create_difficulty(
    uint8_t level,
    bool allow_pills,
    bool allow_blocks
) noexcept {
    constexpr uint8_t min_level = 1;
    constexpr uint8_t max_level = 20;
    constexpr auto max_entities = static_cast<float>(GAME_BOARD_WIDTH);

    BoardInitParams init_params{};
    auto flevel = static_cast<float>(std::clamp(level, min_level, max_level));
    auto t = flevel / static_cast<float>(max_level);
    auto row_growth = 1.0F - std::powf(1.0F - t, 1.0F + (2.5F * t));

    auto cutoff_row = static_cast<uint8_t>(
        std::round(std::lerp(
            4.0F,
            static_cast<float>(GAME_BOARD_HEIGHT - 3),
            row_growth
        ))
    );

    float min_entities = 2.0F;
    if (level >= 15) {
        min_entities = 5.0F;
    } else if (level >= 8) {
        min_entities = 3.0F;
    }

    for (uint8_t row = 0; row < GAME_BOARD_HEIGHT; ++row) {
        if (row >= cutoff_row) {
            continue;
        }
        const float row_norm = static_cast<float>(row) / (static_cast<float>(cutoff_row) + 1);
        const float entity_density = std::max(
            t * t,
            1.0F - ((2.0F * row_norm - 1.0F) * (2.0F * row_norm - 1.0F))
        );
        float entity_factor = t * entity_density;

        init_params.MaxEntitiesPerRow.at(row) = static_cast<uint8_t>(
            std::max(min_entities, std::floorf(max_entities * entity_density))
        );
        init_params.EnemyChancePerRow.at(row) = static_cast<uint8_t>(
            std::round(std::lerp(25.0F, 90.0F, entity_density * entity_factor))
        );

        if (allow_blocks) {
            init_params.BlockChancePerRow.at(row) = static_cast<uint8_t>(
                std::round(std::lerp(2.0F, 15.0F, entity_factor))
            );
        }

        if (allow_pills) {
            init_params.PillChancePerRow.at(row) = static_cast<uint8_t>(
                std::round(std::lerp(2.0F, 20.0F, entity_factor))
            );
        }
    }

    return init_params;
}

void PillGameBoard::init_board(const BoardInitParams& params, std::mt19937& rng_device) noexcept {
    m_FlatGameBoard.fill(EMPTY_ENTITY);

    auto chance_dist = std::uniform_int_distribution<int32_t>(0, 100);
    auto colour_dist = std::uniform_int_distribution<int32_t>(0, 3);

    auto col_indicies = std::array<uint8_t, GAME_BOARD_WIDTH>{};
    for (uint8_t row = 0; row < GAME_BOARD_WIDTH; ++row) {
        col_indicies.at(row) = row;
    }

    for (uint32_t row = 0; row < GAME_BOARD_HEIGHT; ++row) {
        int32_t entity_count = 0;

        auto max_entities = static_cast<int32_t>(params.MaxEntitiesPerRow.at(row));
        auto enemy_chance = static_cast<int32_t>(params.EnemyChancePerRow.at(row));
        auto pill_chance = static_cast<int32_t>(params.PillChancePerRow.at(row));
        auto block_chance = static_cast<int32_t>(params.BlockChancePerRow.at(row));

        auto etype_chances = std::array<std::tuple<int32_t, uint8_t>, 3>{
            std::make_tuple(pill_chance, ETYPE_SPILL),
            std::make_tuple(block_chance, ETYPE_BLOCK),
            std::make_tuple(enemy_chance, ETYPE_ENEMY)
        };

        std::shuffle(col_indicies.begin(), col_indicies.end(), rng_device);

        for (uint8_t col : col_indicies) {
            if (max_entities <= entity_count) {
                break;
            }

            std::shuffle(etype_chances.begin(), etype_chances.end(), rng_device);
            auto val = static_cast<int32_t>(chance_dist(rng_device));
            BoardEntity& entity = this->operator()(row, col);

            for (const auto& etype_chance : etype_chances) {
                const auto& [chance, etype] = etype_chance;
                if (val <= chance) {
                    entity.EntityType = etype;
                    entity.Colour = colour_dist(rng_device);
                    ++entity_count;
                }
            }
        }
    }
}

bool PillGameBoard::can_piece_drop(const BoardPiece& piece) const noexcept {
    if (piece.Row == 0) {
        return false;
    }

    const auto& [l_row, l_col] = piece.left_piece_pos();
    const auto& [r_row, r_col] = piece.right_piece_pos();

    if (l_row == 0 || r_row == 0) {
        return false;
    }

    // Right piece is pointing directly down
    if (piece.Rotation == ROTATE_SOUTH) {
        return this->operator()(r_row - 1, r_col).is_empty();
    }

    // Left piece is pointing directly down
    if (piece.Rotation == ROTATE_NORTH) {
        return this->operator()(l_row - 1, l_col).is_empty();
    }

    // check that both places are empty
    return this->operator()(r_row - 1, r_col).is_empty()
           && this->operator()(l_row - 1, l_col).is_empty();
}

bool PillGameBoard::can_place_piece(const BoardPiece& piece) const noexcept {

    const auto&[rrow, rcol] = piece.right_piece_pos();
    const auto&[lrow, lcol] = piece.left_piece_pos();

    if (rcol < 0 || rcol >= GAME_BOARD_WIDTH || rrow < 0 || rrow >= GAME_BOARD_HEIGHT) {
        return false;
    }

    if (lcol < 0 || lcol >= GAME_BOARD_WIDTH || lrow < 0 || lrow >= GAME_BOARD_HEIGHT) {
        return false;
    }

    return this->operator()(piece.left_piece_pos()).is_empty()
           && this->operator()(rrow, rcol).is_empty();
}

void PillGameBoard::place_piece(const BoardPiece& piece) noexcept {
    this->operator()(piece.left_piece_pos()) = piece.Left;
    this->operator()(piece.right_piece_pos()) = piece.Right;
}

void PillGameBoard::remove_piece(const BoardPiece& piece) noexcept {
    this->operator()(piece.left_piece_pos()) = EMPTY_ENTITY;
    this->operator()(piece.right_piece_pos()) = EMPTY_ENTITY;
}

}  // namespace pill_game