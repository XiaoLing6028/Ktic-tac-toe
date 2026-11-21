// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Ktic-tac-toe contributors
#pragma once

#include "board.h"

#include <random>

enum class Difficulty { Easy, Medium, Hard, Impossible };

class Bot
{
public:
    // Probability that the Hard bot plays a "Medium" move instead of the
    // minimax-optimal one. Medium never misses an immediate win or block, so
    // Hard only becomes beatable through forks.
    static constexpr double HardMistakeRate = 0.3;

    explicit Bot(Difficulty difficulty = Difficulty::Medium,
                 std::uint32_t seed = std::random_device{}());

    Difficulty difficulty() const { return m_difficulty; }
    void setDifficulty(Difficulty d) { m_difficulty = d; }

    // Returns the cell (0..8) to play for `me`, or -1 if the game is over.
    int chooseMove(const Board &board, Player me);

    // Minimax value of every empty cell, from `me`'s point of view, assuming
    // `me` moves next and both sides play perfectly afterwards.
    // Positive = win (sooner is larger), 0 = draw, negative = loss.
    // Cells that are occupied get INT_MIN.
    static std::array<int, Board::Cells> scoreMoves(const Board &board, Player me);

private:
    int easyMove(const Board &board);
    int mediumMove(const Board &board, Player me);
    int perfectMove(const Board &board, Player me);
    int pickRandom(const std::vector<int> &cells);

    Difficulty m_difficulty;
    std::mt19937 m_rng;
};
