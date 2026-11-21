// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Ktic-tac-toe contributors
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

// Pure game logic: no Qt dependency so it can be unit tested in isolation.

enum class Player : std::uint8_t { None, X, O };

constexpr Player opponent(Player p)
{
    return p == Player::X ? Player::O : (p == Player::O ? Player::X : Player::None);
}

enum class GameState { InProgress, XWins, OWins, Draw };

class Board
{
public:
    static constexpr int Cells = 9;
    using Line = std::array<int, 3>;

    // All eight winning lines, cells indexed 0..8 in reading order.
    static constexpr std::array<Line, 8> Lines{{
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // rows
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // columns
        {0, 4, 8}, {2, 4, 6},            // diagonals
    }};

    Board() = default;

    Player at(int cell) const { return m_cells.at(static_cast<std::size_t>(cell)); }
    bool isEmpty(int cell) const { return at(cell) == Player::None; }

    // Places a mark. Returns false (and leaves the board untouched) if the
    // cell is out of range, occupied, or the game is already over.
    bool place(int cell, Player p);
    // Clears a cell; used by search algorithms to undo moves.
    void clear(int cell) { m_cells.at(static_cast<std::size_t>(cell)) = Player::None; }
    void reset() { m_cells.fill(Player::None); }

    Player winner() const;
    std::optional<Line> winningLine() const;
    bool isFull() const;
    bool isDraw() const { return isFull() && winner() == Player::None; }
    bool isGameOver() const { return winner() != Player::None || isFull(); }
    GameState state() const;

    int moveCount() const;
    std::vector<int> emptyCells() const;

    // Cell that would complete a line for `p` right now, if any.
    std::optional<int> findWinningMove(Player p) const;

    bool operator==(const Board &) const = default;

private:
    std::array<Player, Cells> m_cells{};
};
