// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Ktic-tac-toe contributors
//
// Dependency-free tests for the board and the bots. Run via `ctest`.
// The key property checked exhaustively: the Impossible bot never loses,
// whichever side it plays and whatever the opponent does.

#include "board.h"
#include "bot.h"

#include <cstdio>
#include <cstdlib>
#include <string>

namespace
{
int g_failures = 0;

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond);          \
            ++g_failures;                                                                          \
        }                                                                                          \
    } while (0)

Board fromString(const std::string &s) // e.g. "XO.X....O"
{
    Board b;
    for (int i = 0; i < Board::Cells; ++i) {
        if (s[static_cast<std::size_t>(i)] == 'X')
            b.place(i, Player::X);
        else if (s[static_cast<std::size_t>(i)] == 'O')
            b.place(i, Player::O);
    }
    return b;
}

void testBoard()
{
    Board b;
    CHECK(b.state() == GameState::InProgress);
    CHECK(b.emptyCells().size() == 9);
    CHECK(!b.place(-1, Player::X));
    CHECK(!b.place(9, Player::X));
    CHECK(b.place(4, Player::X));
    CHECK(!b.place(4, Player::O)); // occupied

    // Every line is detected for both players.
    for (const auto &line : Board::Lines) {
        for (Player p : {Player::X, Player::O}) {
            Board w;
            for (int c : line)
                w.place(c, p);
            CHECK(w.winner() == p);
            CHECK(w.winningLine() == line);
            CHECK(w.state() == (p == Player::X ? GameState::XWins : GameState::OWins));
            CHECK(!w.place(w.emptyCells().front(), opponent(p))); // game over
        }
    }

    // A full board with no line is a draw.
    Board d = fromString("XOXXOOOXX");
    CHECK(d.isFull());
    CHECK(d.winner() == Player::None);
    CHECK(d.isDraw());
    CHECK(d.state() == GameState::Draw);

    // Win on the last move is a win, not a draw.
    Board lastWin = fromString("XOXOXOOXX");
    CHECK(lastWin.isFull());
    CHECK(lastWin.winner() == Player::X);
    CHECK(!lastWin.isDraw());

    CHECK(fromString("XX.OO....").findWinningMove(Player::X) == 2);
    CHECK(fromString("XX.OO....").findWinningMove(Player::O) == 5);
    CHECK(!fromString("X...O....").findWinningMove(Player::X));
}

void testEasy()
{
    Bot bot(Difficulty::Easy, 1);
    Board b = fromString("XOXOX.O..");
    for (int i = 0; i < 200; ++i) {
        const int m = bot.chooseMove(b, Player::O);
        CHECK(m >= 0 && b.isEmpty(m));
    }
    CHECK(bot.chooseMove(fromString("XXXOO...."), Player::O) == -1); // game over
}

void testMedium()
{
    for (std::uint32_t seed = 0; seed < 50; ++seed) {
        Bot bot(Difficulty::Medium, seed);
        // Takes the win rather than blocking.
        CHECK(bot.chooseMove(fromString("OO.XX...."), Player::O) == 2);
        // Blocks an obvious threat.
        CHECK(bot.chooseMove(fromString("XX..O...."), Player::O) == 2);
        CHECK(bot.chooseMove(fromString("X...X...."), Player::O) == 8);
    }
}

// Explores every possible opponent reply; the bot answers with its own choice
// (repeated with several seeds so random tie-breaks are covered too).
void exploreNeverLoses(Board &board, Player toMove, Player botSide, Bot &bot, long &games)
{
    if (board.isGameOver()) {
        ++games;
        CHECK(board.winner() != opponent(botSide));
        return;
    }
    if (toMove == botSide) {
        const int m = bot.chooseMove(board, botSide);
        CHECK(m >= 0 && board.isEmpty(m));
        board.place(m, botSide);
        exploreNeverLoses(board, opponent(toMove), botSide, bot, games);
        board.clear(m);
        return;
    }
    for (int c : board.emptyCells()) {
        board.place(c, toMove);
        exploreNeverLoses(board, opponent(toMove), botSide, bot, games);
        board.clear(c);
    }
}

void testImpossibleNeverLoses()
{
    long games = 0;
    for (std::uint32_t seed = 0; seed < 8; ++seed) {
        for (Player side : {Player::X, Player::O}) {
            Bot bot(Difficulty::Impossible, seed);
            Board b;
            exploreNeverLoses(b, Player::X, side, bot, games);
        }
    }
    std::printf("Impossible bot: %ld complete games explored, none lost\n", games);

    // Minimax scores: empty board is a draw with perfect play.
    const auto scores = Bot::scoreMoves(Board{}, Player::X);
    for (int s : scores)
        CHECK(s == 0);
    // Prefers the immediate win.
    Bot bot(Difficulty::Impossible, 3);
    CHECK(bot.chooseMove(fromString("XX.OO....") , Player::X) == 2);
}

void testHardIsStrongButFallible()
{
    // Hard must still never miss an immediate win or block.
    for (std::uint32_t seed = 0; seed < 100; ++seed) {
        Bot bot(Difficulty::Hard, seed);
        CHECK(bot.chooseMove(fromString("OO.XX...."), Player::O) == 2);
        CHECK(bot.chooseMove(fromString("XX..O...."), Player::O) == 2);
    }
    // ...but across many games it is sometimes beaten (otherwise it would be
    // indistinguishable from Impossible). Opponent: corner-fork strategy.
    int losses = 0;
    for (std::uint32_t seed = 0; seed < 400; ++seed) {
        Bot hard(Difficulty::Hard, seed);
        Bot perfect(Difficulty::Impossible, seed + 1000);
        Board b;
        Player toMove = Player::X;
        while (!b.isGameOver()) {
            b.place(toMove == Player::X ? perfect.chooseMove(b, Player::X) : hard.chooseMove(b, Player::O), toMove);
            toMove = opponent(toMove);
        }
        losses += b.winner() == Player::X;
    }
    std::printf("Hard bot lost %d/400 games against perfect play\n", losses);
    CHECK(losses > 0);
    CHECK(losses < 400);
}
} // namespace

int main()
{
    testBoard();
    testEasy();
    testMedium();
    testImpossibleNeverLoses();
    testHardIsStrongButFallible();
    if (g_failures) {
        std::fprintf(stderr, "%d check(s) failed\n", g_failures);
        return EXIT_FAILURE;
    }
    std::puts("All tests passed");
    return EXIT_SUCCESS;
}
