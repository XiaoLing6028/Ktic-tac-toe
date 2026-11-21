# Ktic-tac-toe

A small, KDE-friendly Tic-Tac-Toe game written in C++ with Qt 6.

- **Human vs Human**: two players take turns on the same computer (X always opens).
- **Human vs Bot**: four difficulty levels:
  - **Easy** plays random moves.
  - **Medium** takes an immediate win, blocks an immediate threat, and otherwise plays randomly.
  - **Hard** plays perfect minimax moves, but 30% of the time it plays a Medium move instead, so you can beat it with a fork.
  - **Impossible** uses full minimax. It never loses (this is checked exhaustively by the test suite).

The game follows your Plasma color scheme (light or dark). You can play it entirely from the keyboard: arrow keys and Space, or keys **1–9**. **Ctrl+N** starts a new game.

<img width="458" height="627" alt="image" src="https://github.com/user-attachments/assets/1d4b1861-8a54-463b-bec5-87822690c767" />


## Source tree

```
CMakeLists.txt                              build, install rules, tests
io.github.XiaoLing6028.Ktictactoe.yml       Flatpak manifest (KDE runtime 6.10)
build-flatpak.sh                            one-shot script that builds the .flatpak bundle
src/board.{h,cpp}                           board state, win/draw detection (no Qt)
src/bot.{h,cpp}                             Easy/Medium/Hard/Impossible bots (negamax + alpha-beta)
src/boardwidget.{h,cpp}                     3x3 grid widget: cells, grid lines, win strike
src/mainwindow.{h,cpp}                      window, menus, toolbar, game flow, scores
src/theme.h                                 Breeze colours
src/main.cpp                                entry point
tests/test_logic.cpp                        board and bot tests (run by ctest)
data/io.github.XiaoLing6028.Ktictactoe.desktop desktop entry
data/io.github.XiaoLing6028.Ktictactoe.metainfo.xml  AppStream metadata
data/icons/ktictactoe.svg, ktictactoe-*.png app icon (scalable + 16…256 px)
```

The icon is installed as `share/icons/hicolor/{scalable,16x16,…,256x256}/apps/io.github.XiaoLing6028.Ktictactoe.{svg,png}`. The desktop file refers to it as `Icon=io.github.XiaoLing6028.Ktictactoe`.

## Building the Flatpak bundle

Requirements: `flatpak`, and either `flatpak-builder` or the Flathub `org.flatpak.Builder` app.

```sh
./build-flatpak.sh
# => dist/Ktictactoe-1.0.0-x86_64.flatpak
```

The script runs the following commands:

```sh
flatpak remote-add --user --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo

flatpak install --user flathub org.kde.Platform//6.10 org.kde.Sdk//6.10

# Builds against the KDE 6.10 SDK and runs ctest.
flatpak-builder --user --force-clean \
    --repo=repo build-dir io.github.XiaoLing6028.Ktictactoe.yml
#   No host flatpak-builder? Use the Flathub app instead. FLATPAK_USER_DIR is
#   needed because its sandbox can't otherwise see your --user installation:
#   flatpak run --env=FLATPAK_USER_DIR="$HOME/.local/share/flatpak" \
#       --command=flatpak-builder org.flatpak.Builder --user --force-clean \
#       --repo=repo build-dir io.github.XiaoLing6028.Ktictactoe.yml

flatpak build-bundle \
    --runtime-repo=https://dl.flathub.org/repo/flathub.flatpakrepo \
    repo dist/Ktictactoe-1.0.0-x86_64.flatpak io.github.XiaoLing6028.Ktictactoe
```

Attach `dist/Ktictactoe-1.0.0-x86_64.flatpak` to a GitHub Release. Users install it like this:

```sh
flatpak install --user Ktictactoe-1.0.0-x86_64.flatpak   # pulls the KDE runtime from Flathub if missing
flatpak run io.github.XiaoLing6028.Ktictactoe
```

## Native build (without Flatpak)

Requires Qt 6.5+ (Widgets, Svg) and CMake ≥ 3.21:

```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build
```

## Changing the application ID

The ID `io.github.XiaoLing6028.Ktictactoe` assumes the GitHub repository `github.com/XiaoLing6028/Ktic-tac-toe`. If yours lives elsewhere, change the ID in all of these places:
the `APP_ID` default in `CMakeLists.txt`, the names of both `data/` files, the `Icon=` line, the `<id>`/`<launchable>`/`<url>` tags in the metainfo, and the manifest's name and `app-id`.

## License

Ktic-tac-toe is free software, licensed under the GNU Affero General Public License, version 3 or (at your option) any later version (`AGPL-3.0-or-later`). See [LICENSE](LICENSE).

The AppStream metadata file is licensed under CC0-1.0, as AppStream requires.
## Disclaimer

This is an unofficial community project and is not affiliated with or endorsed by KDE.
KDE® is a registered trademark of KDE e.V. Qt is a registered trademark of The Qt Company Ltd.
