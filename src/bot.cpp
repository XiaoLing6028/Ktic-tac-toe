// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Ktic-tac-toe contributors
#include "bot.h"

#include <algorithm>
#include <climits>

namespace
{
constexpr int WinScore = 10;

// Negamax with alpha-beta pruning. Returns the value of the position for the
// side to move (`toMove`). Depth is used so faster wins / slower losses score
// higher, which makes the bot finish games instead of toying with the player.
int negamax(Board &board, Player toMove, int depth, int alpha, int beta)
{
    // The previous move (by the opponent) may have ended the game.
    if (board.winner() != Player::None)
        return -(WinScore - depth);
    if (board.isFull())
        return 0;

    int best = INT_MIN;
    for (int cell = 0; cell < Board::Cells; ++cell) {
        if (!board.isEmpty(cell))
            continue;
        board.place(cell, toMove);
        const int value = -negamax(board, opponent(toMove), depth + 1, -beta, -alpha);
        board.clear(cell);
        best = std::max(best, value);
        alpha = std::max(alpha, value);
        if (alpha >= beta)
            break;
    }
    return best;
}
} // namespace

Bot::Bot(Difficulty difficulty, std::uint32_t seed)
    : m_difficulty(difficulty)
    , m_rng(seed)
{
}

int Bot::chooseMove(const Board &board, Player me)
{
    if (board.isGameOver() || me == Player::None)
        return -1;

    switch (m_difficulty) {
    case Difficulty::Easy:
        return easyMove(board);
    case Difficulty::Medium:
        return mediumMove(board, me);
    case Difficulty::Hard: {
        std::bernoulli_distribution slip(HardMistakeRate);
        return slip(m_rng) ? mediumMove(board, me) : perfectMove(board, me);
    }
    case Difficulty::Impossible:
        return perfectMove(board, me);
    }
    return easyMove(board);
}

std::array<int, Board::Cells> Bot::scoreMoves(const Board &board, Player me)
{
    std::array<int, Board::Cells> scores;
    scores.fill(INT_MIN);
    Board work = board;
    for (int cell = 0; cell < Board::Cells; ++cell) {
        if (!work.isEmpty(cell))
            continue;
        work.place(cell, me);
        // Full window per root move so every score is exact (needed to pick
        // uniformly among equally good moves).
        scores[static_cast<std::size_t>(cell)] =
            -negamax(work, opponent(me), 1, -WinScore - 1, WinScore + 1);
        work.clear(cell);
    }
    return scores;
}

int Bot::pickRandom(const std::vector<int> &cells)
{
    if (cells.empty())
        return -1;
    std::uniform_int_distribution<std::size_t> dist(0, cells.size() - 1);
    return cells[dist(m_rng)];
}

int Bot::easyMove(const Board &board)
{
    return pickRandom(board.emptyCells());
}

int Bot::mediumMove(const Board &board, Player me)
{
    if (auto win = board.findWinningMove(me))
        return *win;
    if (auto block = board.findWinningMove(opponent(me)))
        return *block;
    return pickRandom(board.emptyCells());
}

int Bot::perfectMove(const Board &board, Player me)
{
    const auto scores = scoreMoves(board, me);
    const int best = *std::max_element(scores.begin(), scores.end());
    std::vector<int> bestCells;
    for (int cell = 0; cell < Board::Cells; ++cell) {
        if (scores[static_cast<std::size_t>(cell)] == best)
            bestCells.push_back(cell);
    }
    // Random tie-break keeps games varied without ever giving up value.
    return pickRandom(bestCells);
}
