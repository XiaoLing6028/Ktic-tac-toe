// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Ktic-tac-toe contributors
#include "boardwidget.h"
#include "theme.h"

#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionFocusRect>

#include <cmath>

// ---------------------------------------------------------------- CellButton

CellButton::CellButton(int index, QWidget *parent)
    : QAbstractButton(parent)
    , m_index(index)
{
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover);
    updateAccessibleName();
}

void CellButton::setMark(Player p)
{
    if (m_mark == p)
        return;
    m_mark = p;
    setCursor(p == Player::None ? Qt::PointingHandCursor : Qt::ArrowCursor);
    updateAccessibleName();
    update();
}

void CellButton::setHighlighted(bool on)
{
    if (m_highlighted != on) {
        m_highlighted = on;
        update();
    }
}

void CellButton::updateAccessibleName()
{
    const QString what = m_mark == Player::X ? tr("X") : m_mark == Player::O ? tr("O") : tr("empty");
    setAccessibleName(tr("Row %1, column %2: %3").arg(m_index / 3 + 1).arg(m_index % 3 + 1).arg(what));
}

void CellButton::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    update();
    QAbstractButton::enterEvent(event);
}

void CellButton::leaveEvent(QEvent *event)
{
    m_hovered = false;
    update();
    QAbstractButton::leaveEvent(event);
}

void CellButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF r = QRectF(rect()).adjusted(4, 4, -4, -4);
    const QPalette &pal = palette();

    // Background feedback: winning cells glow, hover/focus get a soft tint.
    QColor fill;
    if (m_highlighted) {
        fill = Theme::strike();
        fill.setAlphaF(0.25);
    } else if (m_mark == Player::None && isEnabled() && (m_hovered || isDown())) {
        fill = pal.color(QPalette::Highlight);
        fill.setAlphaF(isDown() ? 0.30 : 0.15);
    }
    if (fill.isValid()) {
        p.setPen(Qt::NoPen);
        p.setBrush(fill);
        p.drawRoundedRect(r, 6, 6);
    }
    if (hasFocus()) {
        QColor focus = pal.color(QPalette::Highlight);
        p.setPen(QPen(focus, 2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r.adjusted(1, 1, -1, -1), 6, 6);
    }

    // Marks are drawn in a centred square so they stay round when stretched.
    const qreal side = std::min(r.width(), r.height()) * 0.56;
    const QRectF m(r.center() - QPointF(side / 2, side / 2), QSizeF(side, side));
    const qreal stroke = std::max<qreal>(3.0, side * 0.16);

    if (m_mark == Player::X) {
        p.setPen(QPen(Theme::xColor(pal), stroke, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(m.topLeft(), m.bottomRight());
        p.drawLine(m.topRight(), m.bottomLeft());
    } else if (m_mark == Player::O) {
        p.setPen(QPen(Theme::oColor(), stroke));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(m.adjusted(stroke / 4, stroke / 4, -stroke / 4, -stroke / 4));
    }
}

// ------------------------------------------------------------- StrikeOverlay

StrikeOverlay::StrikeOverlay(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
}

void StrikeOverlay::paintEvent(QPaintEvent *)
{
    if (!m_visible)
        return;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const qreal w = std::max<qreal>(4.0, std::min(width(), height()) / 40.0);
    QColor c = Theme::strike();
    c.setAlphaF(0.95);
    p.setPen(QPen(c, w, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(m_line);
}

// --------------------------------------------------------------- BoardWidget

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent)
    , m_overlay(nullptr)
{
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
    setMinimumSize(200, 200);

    for (int i = 0; i < Board::Cells; ++i) {
        auto *cell = new CellButton(i, this);
        connect(cell, &QAbstractButton::clicked, this, [this, i] {
            if (m_interactive)
                Q_EMIT cellActivated(i);
        });
        cell->installEventFilter(this);
        m_cells[static_cast<std::size_t>(i)] = cell;
    }
    // Reading-order tab chain.
    for (int i = 0; i + 1 < Board::Cells; ++i)
        setTabOrder(m_cells[static_cast<std::size_t>(i)], m_cells[static_cast<std::size_t>(i + 1)]);

    m_overlay = new StrikeOverlay(this);
    m_overlay->raise();
}

void BoardWidget::sync(const Board &board)
{
    for (int i = 0; i < Board::Cells; ++i)
        m_cells[static_cast<std::size_t>(i)]->setMark(board.at(i));

    m_winLine = board.winningLine();
    for (auto *cell : m_cells)
        cell->setHighlighted(false);
    if (m_winLine) {
        for (int i : *m_winLine)
            m_cells[static_cast<std::size_t>(i)]->setHighlighted(true);
    }
    updateStrike();
}

void BoardWidget::setInteractive(bool on)
{
    m_interactive = on;
    for (auto *cell : m_cells)
        cell->setCursor(on && cell->mark() == Player::None ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void BoardWidget::focusCell(int index)
{
    if (index >= 0 && index < Board::Cells)
        m_cells[static_cast<std::size_t>(index)]->setFocus(Qt::OtherFocusReason);
}

QRect BoardWidget::squareArea() const
{
    const int side = std::min(width(), height()) - 16;
    return QRect((width() - side) / 2, (height() - side) / 2, side, side);
}

void BoardWidget::layoutCells()
{
    const QRect area = squareArea();
    const qreal step = area.width() / 3.0;
    for (int i = 0; i < Board::Cells; ++i) {
        const int col = i % 3;
        const int row = i / 3;
        const QRect r(qRound(area.left() + col * step), qRound(area.top() + row * step),
                      qRound(step), qRound(step));
        m_cells[static_cast<std::size_t>(i)]->setGeometry(r.adjusted(3, 3, -3, -3));
    }
    m_overlay->setGeometry(rect());
}

void BoardWidget::updateStrike()
{
    if (!m_winLine) {
        m_overlay->clearLine();
        return;
    }
    const QRect area = squareArea();
    const qreal step = area.width() / 3.0;
    auto centre = [&](int i) {
        return QPointF(area.left() + (i % 3 + 0.5) * step, area.top() + (i / 3 + 0.5) * step);
    };
    QPointF a = centre((*m_winLine)[0]);
    QPointF b = centre((*m_winLine)[2]);
    // Extend a little beyond the outer cell centres, like the app icon.
    const QPointF d = (b - a) * 0.18;
    m_overlay->setLine(QLineF(a - d, b + d));
}

void BoardWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layoutCells();
    updateStrike();
}

void BoardWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRect area = squareArea();
    const qreal step = area.width() / 3.0;
    const qreal inset = step * 0.08;
    QColor grid = palette().color(QPalette::WindowText);
    grid.setAlphaF(0.35);
    p.setPen(QPen(grid, std::max<qreal>(3.0, area.width() / 80.0), Qt::SolidLine, Qt::RoundCap));
    for (int k = 1; k <= 2; ++k) {
        const qreal x = area.left() + k * step;
        const qreal y = area.top() + k * step;
        p.drawLine(QPointF(x, area.top() + inset), QPointF(x, area.bottom() - inset));
        p.drawLine(QPointF(area.left() + inset, y), QPointF(area.right() - inset, y));
    }
}

// Arrow-key navigation between cells, matching what users expect from a grid.
bool BoardWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *cell = qobject_cast<CellButton *>(watched);
        const auto it = std::find(m_cells.begin(), m_cells.end(), cell);
        if (cell && it != m_cells.end()) {
            const int idx = static_cast<int>(it - m_cells.begin());
            int row = idx / 3;
            int col = idx % 3;
            switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Left:  col = (col + 2) % 3; break;
            case Qt::Key_Right: col = (col + 1) % 3; break;
            case Qt::Key_Up:    row = (row + 2) % 3; break;
            case Qt::Key_Down:  row = (row + 1) % 3; break;
            default:
                return QWidget::eventFilter(watched, event);
            }
            focusCell(row * 3 + col);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
