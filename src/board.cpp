// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Ktic-tac-toe contributors
#include "board.h"

bool Board::place(int cell, Player p)
{
    if (cell < 0 || cell >= Cells || p == Player::None)
        return false;
    if (!isEmpty(cell) || isGameOver())
        return false;
    m_cells[static_cast<std::size_t>(cell)] = p;
    return true;
}

std::optional<Board::Line> Board::winningLine() const
{
    for (const Line &line : Lines) {
        const Player p = at(line[0]);
        if (p != Player::None && p == at(line[1]) && p == at(line[2]))
            return line;
    }
    return std::nullopt;
}

Player Board::winner() const
{
    const auto line = winningLine();
    return line ? at((*line)[0]) : Player::None;
}

bool Board::isFull() const
{
    for (Player p : m_cells) {
        if (p == Player::None)
            return false;
    }
    return true;
}

GameState Board::state() const
{
    switch (winner()) {
    case Player::X:
        return GameState::XWins;
    case Player::O:
        return GameState::OWins;
    case Player::None:
        break;
    }
    return isFull() ? GameState::Draw : GameState::InProgress;
}

int Board::moveCount() const
{
    int n = 0;
    for (Player p : m_cells)
        n += (p != Player::None);
    return n;
}

std::vector<int> Board::emptyCells() const
{
    std::vector<int> out;
    out.reserve(Cells);
    for (int i = 0; i < Cells; ++i) {
        if (isEmpty(i))
            out.push_back(i);
    }
    return out;
}

std::optional<int> Board::findWinningMove(Player p) const
{
    for (const Line &line : Lines) {
        int mine = 0;
        int empty = -1;
        int emptyCount = 0;
        for (int c : line) {
            if (at(c) == p)
                ++mine;
            else if (isEmpty(c)) {
                empty = c;
                ++emptyCount;
            }
        }
        if (mine == 2 && emptyCount == 1)
            return empty;
    }
    return std::nullopt;
}
