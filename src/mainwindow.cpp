// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Ktic-tac-toe contributors
#include "mainwindow.h"
#include "boardwidget.h"

#include <QAction>
#include <algorithm>
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

namespace
{
constexpr int BotDelayMs = 450; // short pause so the bot's reply is visible

QString playerName(Player p)
{
    return p == Player::X ? QStringLiteral("X") : QStringLiteral("O");
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Ktic-tac-toe"));

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(12, 12, 12, 12);

    m_scoreLabel = new QLabel(central);
    m_scoreLabel->setAlignment(Qt::AlignCenter);
    m_scoreLabel->setTextFormat(Qt::RichText);
    QFont scoreFont = m_scoreLabel->font();
    scoreFont.setPointSizeF(scoreFont.pointSizeF() * 1.25);
    m_scoreLabel->setFont(scoreFont);

    m_boardWidget = new BoardWidget(central);

    m_statusLabel = new QLabel(central);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    QFont statusFont = m_statusLabel->font();
    statusFont.setBold(true);
    statusFont.setPointSizeF(statusFont.pointSizeF() * 1.15);
    m_statusLabel->setFont(statusFont);
    m_statusLabel->setAccessibleName(tr("Game status"));

    layout->addWidget(m_scoreLabel);
    layout->addWidget(m_boardWidget, 1);
    layout->addWidget(m_statusLabel);
    setCentralWidget(central);

    connect(m_boardWidget, &BoardWidget::cellActivated, this, &MainWindow::onCellActivated);

    setupToolBar();
    setupActions();
    loadSettings();

    resize(460, 600);
    newGame();
}

void MainWindow::setupToolBar()
{
    auto *bar = addToolBar(tr("Game Options"));
    bar->setObjectName(QStringLiteral("optionsToolBar"));
    bar->setMovable(false);

    m_modeCombo = new QComboBox(bar);
    m_modeCombo->addItem(tr("Human vs Human"), int(Mode::HumanVsHuman));
    m_modeCombo->addItem(tr("Human vs Bot"), int(Mode::HumanVsBot));
    m_modeCombo->setAccessibleName(tr("Game mode"));

    m_difficultyCombo = new QComboBox(bar);
    m_difficultyCombo->addItem(tr("Easy"), int(Difficulty::Easy));
    m_difficultyCombo->addItem(tr("Medium"), int(Difficulty::Medium));
    m_difficultyCombo->addItem(tr("Hard"), int(Difficulty::Hard));
    m_difficultyCombo->addItem(tr("Impossible"), int(Difficulty::Impossible));
    m_difficultyCombo->setItemData(0, tr("Plays random moves"), Qt::ToolTipRole);
    m_difficultyCombo->setItemData(1, tr("Takes wins and blocks obvious threats"), Qt::ToolTipRole);
    m_difficultyCombo->setItemData(2, tr("Plays well, but occasionally slips"), Qt::ToolTipRole);
    m_difficultyCombo->setItemData(3, tr("Perfect play: it never loses"), Qt::ToolTipRole);
    m_difficultyCombo->setAccessibleName(tr("Bot difficulty"));

    m_sideCombo = new QComboBox(bar);
    m_sideCombo->addItem(tr("X (first)"), int(Player::X));
    m_sideCombo->addItem(tr("O (second)"), int(Player::O));
    m_sideCombo->setAccessibleName(tr("Your side"));

    m_difficultyLabel = new QLabel(tr("Difficulty:"), bar);
    m_sideLabel = new QLabel(tr("You play:"), bar);
    m_difficultyLabel->setBuddy(m_difficultyCombo);
    m_sideLabel->setBuddy(m_sideCombo);

    auto *modeLabel = new QLabel(tr("Mode:"), bar);
    modeLabel->setBuddy(m_modeCombo);

    // Options live in a compact 3-column row above the board (labels over
    // combos) so they stay visible even in a narrow window; a toolbar would
    // push them into its overflow menu.
    auto *options = new QWidget(centralWidget());
    auto *grid = new QGridLayout(options);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(2);
    const QList<QPair<QLabel *, QComboBox *>> cols{
        {modeLabel, m_modeCombo}, {m_difficultyLabel, m_difficultyCombo}, {m_sideLabel, m_sideCombo}};
    for (int c = 0; c < cols.size(); ++c) {
        cols[c].first->setParent(options);
        cols[c].second->setParent(options);
        cols[c].second->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        grid->addWidget(cols[c].first, 0, c);
        grid->addWidget(cols[c].second, 1, c);
        grid->setColumnStretch(c, 1);
    }
    qobject_cast<QVBoxLayout *>(centralWidget()->layout())->insertWidget(0, options);

    auto restart = [this] {
        saveSettings();
        newGame();
    };
    connect(m_modeCombo, &QComboBox::currentIndexChanged, this, [this, restart] {
        resetScores(); // scores from a different mode are meaningless
        restart();
    });
    connect(m_difficultyCombo, &QComboBox::currentIndexChanged, this, restart);
    connect(m_sideCombo, &QComboBox::currentIndexChanged, this, restart);
}

void MainWindow::setupActions()
{
    auto *gameMenu = menuBar()->addMenu(tr("&Game"));

    auto *newAction = gameMenu->addAction(QIcon::fromTheme(QStringLiteral("document-new")), tr("&New Game"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newGame);

    auto *resetAction = gameMenu->addAction(QIcon::fromTheme(QStringLiteral("edit-clear-history")), tr("&Reset Scores"));
    connect(resetAction, &QAction::triggered, this, [this] {
        resetScores();
        newGame();
    });

    gameMenu->addSeparator();
    auto *quitAction = gameMenu->addAction(QIcon::fromTheme(QStringLiteral("application-exit")), tr("&Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    auto *helpMenu = menuBar()->addMenu(tr("&Help"));
    auto *aboutAction = helpMenu->addAction(windowIcon(), tr("&About Ktic-tac-toe"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    auto *aboutQt = helpMenu->addAction(QIcon::fromTheme(QStringLiteral("qtcreator")), tr("About &Qt"));
    connect(aboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

    // Put "New Game" at the front of the toolbar as well.
    if (auto *bar = findChild<QToolBar *>(QStringLiteral("optionsToolBar"))) {
        bar->insertAction(bar->actions().value(0), newAction);
        bar->insertSeparator(bar->actions().value(1));
    }

    // Keys 1-9 play a cell in reading order (numpad-style layout would be
    // confusing on laptops without one).
    for (int i = 0; i < Board::Cells; ++i) {
        auto *sc = new QShortcut(QKeySequence(Qt::Key_1 + i), this);
        connect(sc, &QShortcut::activated, this, [this, i] {
            m_boardWidget->focusCell(i);
            onCellActivated(i);
        });
    }
}

void MainWindow::loadSettings()
{
    QSettings s;
    const QSignalBlocker b1(m_modeCombo), b2(m_difficultyCombo), b3(m_sideCombo);
    m_modeCombo->setCurrentIndex(std::clamp(s.value(QStringLiteral("mode"), 1).toInt(), 0, 1));
    m_difficultyCombo->setCurrentIndex(std::clamp(s.value(QStringLiteral("difficulty"), 1).toInt(), 0, 3));
    m_sideCombo->setCurrentIndex(std::clamp(s.value(QStringLiteral("side"), 0).toInt(), 0, 1));
    restoreGeometry(s.value(QStringLiteral("geometry")).toByteArray());
}

void MainWindow::saveSettings() const
{
    QSettings s;
    s.setValue(QStringLiteral("mode"), m_modeCombo->currentIndex());
    s.setValue(QStringLiteral("difficulty"), m_difficultyCombo->currentIndex());
    s.setValue(QStringLiteral("side"), m_sideCombo->currentIndex());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QSettings().setValue(QStringLiteral("geometry"), saveGeometry());
    QMainWindow::closeEvent(event);
}

MainWindow::Mode MainWindow::mode() const
{
    return static_cast<Mode>(m_modeCombo->currentData().toInt());
}

Player MainWindow::humanSide() const
{
    return static_cast<Player>(m_sideCombo->currentData().toInt());
}

bool MainWindow::isBotTurn() const
{
    return mode() == Mode::HumanVsBot && !m_board.isGameOver() && m_toMove != humanSide();
}

void MainWindow::newGame()
{
    ++m_gameId;
    m_board.reset();
    m_toMove = Player::X; // X always opens
    m_bot.setDifficulty(static_cast<Difficulty>(m_difficultyCombo->currentData().toInt()));

    const bool vsBot = mode() == Mode::HumanVsBot;
    for (QWidget *w : {static_cast<QWidget *>(m_difficultyCombo), static_cast<QWidget *>(m_difficultyLabel),
                       static_cast<QWidget *>(m_sideCombo), static_cast<QWidget *>(m_sideLabel)})
        w->setEnabled(vsBot);

    updateUi();
    m_boardWidget->focusCell(4);
    if (isBotTurn())
        scheduleBotMove();
}

void MainWindow::resetScores()
{
    m_xWins = m_oWins = m_draws = 0;
    updateScores();
}

void MainWindow::onCellActivated(int cell)
{
    if (m_board.isGameOver() || isBotTurn() || !m_board.isEmpty(cell))
        return;
    playMove(cell);
    if (isBotTurn())
        scheduleBotMove();
}

void MainWindow::playMove(int cell)
{
    if (!m_board.place(cell, m_toMove))
        return;
    m_toMove = opponent(m_toMove);
    finishIfOver();
    updateUi();
}

void MainWindow::scheduleBotMove()
{
    updateUi();
    const quint64 id = m_gameId;
    QTimer::singleShot(BotDelayMs, this, [this, id] {
        if (id == m_gameId)
            botMove();
    });
}

void MainWindow::botMove()
{
    if (!isBotTurn())
        return;
    const int cell = m_bot.chooseMove(m_board, m_toMove);
    if (cell >= 0)
        playMove(cell);
}

void MainWindow::finishIfOver()
{
    switch (m_board.state()) {
    case GameState::XWins: ++m_xWins; break;
    case GameState::OWins: ++m_oWins; break;
    case GameState::Draw:  ++m_draws; break;
    case GameState::InProgress: return;
    }
    updateScores();
}

void MainWindow::updateScores()
{
    const bool vsBot = mode() == Mode::HumanVsBot;
    auto label = [&](Player p) {
        if (!vsBot)
            return playerName(p);
        return p == humanSide() ? tr("You (%1)").arg(playerName(p)) : tr("Bot (%1)").arg(playerName(p));
    };
    const QString orange = QStringLiteral("#f67400");
    m_scoreLabel->setText(tr("%1: <b>%2</b> &nbsp;·&nbsp; Draws: <b>%3</b> &nbsp;·&nbsp; <span style=\"color:%4\">%5: <b>%6</b></span>")
                              .arg(label(Player::X).toHtmlEscaped())
                              .arg(m_xWins)
                              .arg(m_draws)
                              .arg(orange, label(Player::O).toHtmlEscaped())
                              .arg(m_oWins));
}

void MainWindow::updateUi()
{
    m_boardWidget->sync(m_board);
    m_boardWidget->setInteractive(!m_board.isGameOver() && !isBotTurn());
    updateScores();

    const bool vsBot = mode() == Mode::HumanVsBot;
    QString status;
    switch (m_board.state()) {
    case GameState::InProgress:
        if (!vsBot)
            status = tr("%1 to move").arg(playerName(m_toMove));
        else
            status = isBotTurn() ? tr("Bot is thinking…") : tr("Your move (%1)").arg(playerName(m_toMove));
        break;
    case GameState::Draw:
        status = tr("It's a draw!");
        break;
    case GameState::XWins:
    case GameState::OWins: {
        const Player w = m_board.winner();
        if (!vsBot)
            status = tr("%1 wins!").arg(playerName(w));
        else
            status = w == humanSide() ? tr("You win!") : tr("The bot wins!");
        break;
    }
    }
    if (m_board.isGameOver())
        status += QLatin1Char(' ') + tr("Press %1 to play again.").arg(QKeySequence(QKeySequence::New).toString(QKeySequence::NativeText));
    m_statusLabel->setText(status);
    statusBar()->showMessage(vsBot ? tr("Human vs Bot · %1").arg(m_difficultyCombo->currentText()) : tr("Human vs Human"));
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("About Ktic-tac-toe"),
                       tr("<h3>Ktic-tac-toe %1</h3>"
                          "<p>A small Tic-Tac-Toe game for the KDE desktop.</p>"
                          "<p>Play against a friend, or against a bot with four difficulty levels, "
                          "from <i>Easy</i> to <i>Impossible</i>.</p>"
                          "<p>Released under the "
                          "<a href=\"https://www.gnu.org/licenses/agpl-3.0.html\">GNU Affero General Public License, "
                          "version 3 or later</a>.</p>"
                          "<p>Source code: <a href=\"https://github.com/XiaoLing6028/Ktic-tac-toe\">"
                          "github.com/XiaoLing6028/Ktic-tac-toe</a></p>")
                           .arg(QApplication::applicationVersion()));
}
