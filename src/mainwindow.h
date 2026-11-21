// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Ktic-tac-toe contributors
#pragma once

#include "board.h"
#include "bot.h"

#include <QMainWindow>

class BoardWidget;
class QComboBox;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    enum class Mode { HumanVsHuman, HumanVsBot };

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupActions();
    void setupToolBar();
    void loadSettings();
    void saveSettings() const;

    void newGame();
    void resetScores();
    void onCellActivated(int cell);
    void playMove(int cell);
    void scheduleBotMove();
    void botMove();
    void finishIfOver();
    void updateUi();
    void updateScores();
    void showAbout();

    bool isBotTurn() const;
    Mode mode() const;
    Player humanSide() const;

    Board m_board;
    Bot m_bot;
    Player m_toMove = Player::X;
    quint64 m_gameId = 0; // invalidates stale bot timers after "New Game"

    int m_xWins = 0;
    int m_oWins = 0;
    int m_draws = 0;

    BoardWidget *m_boardWidget = nullptr;
    QLabel *m_scoreLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QComboBox *m_modeCombo = nullptr;
    QComboBox *m_difficultyCombo = nullptr;
    QComboBox *m_sideCombo = nullptr;
    QLabel *m_difficultyLabel = nullptr;
    QLabel *m_sideLabel = nullptr;
};
