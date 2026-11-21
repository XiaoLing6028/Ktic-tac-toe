// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Ktic-tac-toe contributors
#include "mainwindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("ktictactoe"));
    QApplication::setApplicationDisplayName(QStringLiteral("Ktic-tac-toe"));
    QApplication::setApplicationVersion(QStringLiteral(KTICTACTOE_VERSION));
    QApplication::setOrganizationName(QStringLiteral("ktictactoe")); // ~/.config/ktictactoe/
    // Lets Plasma/Wayland match windows to the .desktop file (taskbar icon etc.).
    QGuiApplication::setDesktopFileName(QStringLiteral(KTICTACTOE_APP_ID));
    // Prefer the installed theme icon, fall back to the copy compiled in.
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral(KTICTACTOE_APP_ID),
                                                 QIcon(QStringLiteral(":/icons/ktictactoe.svg"))));

    MainWindow window;
    window.show();
    return app.exec();
}
