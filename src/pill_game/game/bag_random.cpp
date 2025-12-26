//
// Date       : 26/12/2025
// Project    : pill_game
// Author     : -Ry
//

#include "pill_game/pch.h"
#include "pill_game/game/bag_random.h"

namespace pill_game {

void BagRandom::reset(std::mt19937& random) noexcept {
    m_CurrentPiece = 0;
    std::shuffle(m_Pieces.begin(), m_Pieces.end(), random);
}

BoardPiece BagRandom::fetch_next(std::mt19937& random) {
    m_CurrentPiece = (m_CurrentPiece + 1) % static_cast<int32_t>(m_Pieces.size());
    if (m_CurrentPiece == 0) {
        reset(random);
    }
    return current();
}

std::array<BoardPiece, 2> BagRandom::hints() const noexcept {
    const auto size = static_cast<int32_t>(m_Pieces.size());
    std::array<BoardPiece, 2> pieces{EMPTY_PIECE};

    if ((m_CurrentPiece + 1) < size) {
        pieces[0] = m_Pieces[m_CurrentPiece+1];
    }

    if ((m_CurrentPiece + 2) < size) {
        pieces[1] = m_Pieces[m_CurrentPiece+2];
    }

    return pieces;
}

}  // namespace pill_game
