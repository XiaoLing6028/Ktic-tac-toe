// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Ktic-tac-toe contributors
#pragma once

#include <QColor>
#include <QPalette>

// Colours taken from the KDE Breeze palette (and the app icon). X follows the
// text colour so it stays readable with both light and dark colour schemes.
namespace Theme
{
inline QColor xColor(const QPalette &pal) { return pal.color(QPalette::WindowText); }
inline QColor oColor() { return QColor(0xf6, 0x74, 0x00); } // Breeze "Beware Orange"
inline QColor strike() { return QColor(0xfd, 0xbc, 0x4b); } // Breeze "Icon Yellow"
} // namespace Theme
