//
// Date       : 26/12/2025
// Project    : pill_game
// Author     : -Ry
//

#pragma once

#include "pill_game/pch.h"

namespace pill_game {

class BagRandom {
   private:
    int32_t m_CurrentPiece{0};
    std::array<BoardPiece, ALL_PIECES.size()> m_Pieces{ALL_PIECES};

   public:
    explicit BagRandom() noexcept = default;
    ~BagRandom() noexcept = default;

   public:
    BagRandom(const BagRandom&) = default;
    BagRandom& operator=(const BagRandom&) = default;
    BagRandom(BagRandom&&) = default;
    BagRandom& operator=(BagRandom&&) = default;

   public:
    void reset(std::mt19937& random) noexcept;

   public:
    BoardPiece current() const noexcept { return m_Pieces.at(m_CurrentPiece); }
    std::array<BoardPiece, 2> hints() const noexcept;

   public:
    BoardPiece fetch_next(std::mt19937& random);
};

}  // namespace pill_game