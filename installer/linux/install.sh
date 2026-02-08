#!/usr/bin/env bash
# Whetstone Editor - Linux Install Script
# Installs system dependencies, builds from source, and installs to /usr/local/bin
# Usage: ./install.sh [--prefix /usr/local] [--no-build]

set -euo pipefail

PREFIX="/usr/local"
DO_BUILD=true

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
EDITOR_DIR="$REPO_DIR/editor"
BUILD_DIR="$EDITOR_DIR/build"

# --- Parse arguments ---

while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix)   PREFIX="$2"; shift 2 ;;
        --no-build) DO_BUILD=false; shift ;;
        -h|--help)
            echo "Usage: $0 [--prefix /usr/local] [--no-build]"
            echo "  --prefix PATH    Install prefix (default: /usr/local)"
            echo "  --no-build       Skip build, install pre-built binaries only"
            exit 0 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

step() {
    echo ""
    echo "==> $1"
}

# --- Install system dependencies ---

step "Installing system dependencies"

if command -v apt-get &>/dev/null; then
    sudo apt-get update -qq
    sudo apt-get install -y -qq \
        build-essential cmake git \
        libsdl2-dev libgl1-mesa-dev \
        nlohmann-json3-dev
elif command -v dnf &>/dev/null; then
    sudo dnf install -y \
        gcc-c++ cmake git \
        SDL2-devel mesa-libGL-devel \
        json-devel
elif command -v pacman &>/dev/null; then
    sudo pacman -S --noconfirm --needed \
        base-devel cmake git \
        sdl2 mesa \
        nlohmann-json
else
    echo "WARNING: Unknown package manager. Please install manually:"
    echo "  - C++ compiler (g++ or clang++)"
    echo "  - CMake 3.20+"
    echo "  - SDL2 development libraries"
    echo "  - OpenGL development libraries"
    echo "  - nlohmann-json"
fi

# --- Build from source ---

if $DO_BUILD; then
    step "Building from source"

    # Use the build script if available
    if [[ -x "$SCRIPT_DIR/build.sh" ]]; then
        "$SCRIPT_DIR/build.sh" --config Release --system-packages
    else
        mkdir -p "$BUILD_DIR"
        cmake -S "$EDITOR_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
        cmake --build "$BUILD_DIR" --config Release --parallel "$(nproc)"
    fi
fi

# --- Install binaries ---

step "Installing to $PREFIX/bin"

sudo mkdir -p "$PREFIX/bin"

for exe in whetstone_editor orchestrator; do
    if [[ -f "$BUILD_DIR/$exe" ]]; then
        sudo install -m 755 "$BUILD_DIR/$exe" "$PREFIX/bin/$exe"
        echo "  Installed $exe"
    else
        echo "  WARNING: $exe not found in build directory"
    fi
done

# --- Create desktop entry ---

step "Creating desktop entry"

DESKTOP_FILE="$HOME/.local/share/applications/whetstone-editor.desktop"
mkdir -p "$(dirname "$DESKTOP_FILE")"

cat > "$DESKTOP_FILE" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Whetstone Editor
Comment=Visual editor for the Whetstone domain-specific language
Exec=$PREFIX/bin/whetstone_editor %F
Terminal=false
Categories=Development;IDE;TextEditor;
MimeType=text/x-whetstone;text/x-semanno;
Keywords=whetstone;dsl;semantic;annotation;
DESKTOP

echo "  Created $DESKTOP_FILE"

# --- Create MIME types ---

MIME_DIR="$HOME/.local/share/mime/packages"
mkdir -p "$MIME_DIR"

cat > "$MIME_DIR/whetstone.xml" <<MIME
<?xml version="1.0" encoding="UTF-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
  <mime-type type="text/x-whetstone">
    <comment>Whetstone Source File</comment>
    <glob pattern="*.whet"/>
  </mime-type>
  <mime-type type="text/x-semanno">
    <comment>Semantic Annotation File</comment>
    <glob pattern="*.semanno"/>
  </mime-type>
</mime-info>
MIME

if command -v update-mime-database &>/dev/null; then
    update-mime-database "$HOME/.local/share/mime" 2>/dev/null || true
fi

echo "  Registered .whet and .semanno MIME types"

step "Installation complete!"
echo "  Binaries: $PREFIX/bin/whetstone_editor, $PREFIX/bin/orchestrator"
echo "  Desktop entry: $DESKTOP_FILE"
echo ""
