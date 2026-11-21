// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Ktic-tac-toe contributors
#pragma once

#include "board.h"

#include <QAbstractButton>
#include <QWidget>

#include <array>
#include <optional>

// One square of the 3x3 grid. A real (focusable, accessible) button that
// paints its own X / O mark so it scales with the window.
class CellButton : public QAbstractButton
{
    Q_OBJECT
public:
    CellButton(int index, QWidget *parent = nullptr);

    void setMark(Player p);
    Player mark() const { return m_mark; }
    void setHighlighted(bool on);

    QSize sizeHint() const override { return {110, 110}; }
    QSize minimumSizeHint() const override { return {56, 56}; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void updateAccessibleName();

    int m_index;
    Player m_mark = Player::None;
    bool m_highlighted = false;
    bool m_hovered = false;
};

// Draws the strike through a winning line on top of the cells.
class StrikeOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit StrikeOverlay(QWidget *parent);
    void setLine(QLineF line) { m_line = line; m_visible = true; update(); }
    void clearLine() { m_visible = false; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLineF m_line;
    bool m_visible = false;
};

class BoardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoardWidget(QWidget *parent = nullptr);

    // Mirror a Board into the widgets.
    void sync(const Board &board);
    // When false, clicks are ignored (e.g. while the bot is thinking).
    void setInteractive(bool on);
    void focusCell(int index);

    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int w) const override { return w; }

Q_SIGNALS:
    void cellActivated(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QRect squareArea() const;
    void layoutCells();
    void updateStrike();

    std::array<CellButton *, Board::Cells> m_cells{};
    StrikeOverlay *m_overlay;
    std::optional<Board::Line> m_winLine;
    bool m_interactive = true;
};
