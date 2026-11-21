#!/usr/bin/env bash
# Builds dist/Ktictactoe-<version>-<arch>.flatpak, ready to attach to a GitHub Release.
set -euo pipefail
cd "$(dirname "$0")"

APP_ID=io.github.XiaoLing6028.Ktictactoe
VERSION=$(sed -n 's/^project(ktictactoe VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)
ARCH=$(flatpak --default-arch)

# flatpak-builder: use the host binary if present, otherwise the Flathub one.
if command -v flatpak-builder >/dev/null; then
    BUILDER=(flatpak-builder)
else
    # Inside its sandbox XDG_DATA_HOME is redirected, so point --user at the
    # real per-user installation where the SDK was installed below.
    BUILDER=(flatpak run --env=FLATPAK_USER_DIR="$HOME/.local/share/flatpak"
             --command=flatpak-builder org.flatpak.Builder)
fi

flatpak remote-add --user --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo

# Install runtime + SDK with the host flatpak (more reliable than letting a
# sandboxed flatpak-builder do it via --install-deps-from).
flatpak install --user -y --noninteractive flathub org.kde.Platform//6.10 org.kde.Sdk//6.10

"${BUILDER[@]}" --user --force-clean \
    --repo=repo build-dir "$APP_ID.yml"

mkdir -p dist
flatpak build-bundle \
    --runtime-repo=https://dl.flathub.org/repo/flathub.flatpakrepo \
    repo "dist/Ktictactoe-$VERSION-$ARCH.flatpak" "$APP_ID"

echo "Bundle: dist/Ktictactoe-$VERSION-$ARCH.flatpak"
