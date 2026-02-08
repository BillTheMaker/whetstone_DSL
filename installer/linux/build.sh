#!/usr/bin/env bash
# Whetstone Editor - Linux Build Script
# Prerequisites: GCC/Clang, CMake 3.20+, vcpkg or system packages
# Usage: ./build.sh [--config Release|Debug] [--vcpkg-root /path/to/vcpkg] [--system-packages] [--package deb|tar]

set -euo pipefail

CONFIG="Release"
VCPKG_ROOT="${VCPKG_ROOT:-}"
USE_SYSTEM_PACKAGES=false
PACKAGE_TYPE=""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EDITOR_DIR="$(cd "$SCRIPT_DIR/../../editor" && pwd)"
BUILD_DIR="$EDITOR_DIR/build"
TRIPLET="x64-linux"

# --- Parse arguments ---

while [[ $# -gt 0 ]]; do
    case "$1" in
        --config)       CONFIG="$2"; shift 2 ;;
        --vcpkg-root)   VCPKG_ROOT="$2"; shift 2 ;;
        --system-packages) USE_SYSTEM_PACKAGES=true; shift ;;
        --package)      PACKAGE_TYPE="$2"; shift 2 ;;
        -h|--help)
            echo "Usage: $0 [--config Release|Debug] [--vcpkg-root PATH] [--system-packages] [--package deb|tar]"
            exit 0 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

step() {
    echo ""
    echo "==> $1"
}

# --- Check prerequisites ---

step "Checking prerequisites"

if ! command -v cmake &>/dev/null; then
    echo "ERROR: CMake not found. Install cmake 3.20+."
    exit 1
fi
echo "  CMake: $(cmake --version | head -1)"

if command -v g++ &>/dev/null; then
    echo "  GCC: $(g++ --version | head -1)"
elif command -v clang++ &>/dev/null; then
    echo "  Clang: $(clang++ --version | head -1)"
else
    echo "ERROR: No C++ compiler found. Install g++ or clang++."
    exit 1
fi

# --- Install system dependencies if requested ---

if $USE_SYSTEM_PACKAGES; then
    step "Installing system packages"

    if command -v apt-get &>/dev/null; then
        sudo apt-get update -qq
        sudo apt-get install -y -qq \
            build-essential cmake \
            libsdl2-dev libgl1-mesa-dev libglew-dev \
            nlohmann-json3-dev
    elif command -v dnf &>/dev/null; then
        sudo dnf install -y \
            gcc-c++ cmake \
            SDL2-devel mesa-libGL-devel glew-devel \
            json-devel
    elif command -v pacman &>/dev/null; then
        sudo pacman -S --noconfirm --needed \
            base-devel cmake \
            sdl2 mesa glew \
            nlohmann-json
    else
        echo "WARNING: Unknown package manager. Install dependencies manually:"
        echo "  SDL2-dev, OpenGL dev, nlohmann-json, glad"
    fi

    echo "  System packages installed."
fi

# --- Bootstrap vcpkg if needed ---

if ! $USE_SYSTEM_PACKAGES; then
    if [[ -z "$VCPKG_ROOT" ]]; then
        # Try common locations
        for candidate in "$HOME/vcpkg" "/opt/vcpkg" "/usr/local/vcpkg"; do
            if [[ -x "$candidate/vcpkg" ]]; then
                VCPKG_ROOT="$candidate"
                break
            fi
        done
    fi

    if [[ -z "$VCPKG_ROOT" ]]; then
        step "Bootstrapping vcpkg"
        VCPKG_ROOT="$HOME/vcpkg"
        git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
        "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics
    fi

    echo "  vcpkg: $VCPKG_ROOT"

    step "Installing vcpkg dependencies"
    "$VCPKG_ROOT/vcpkg" install \
        "nlohmann-json:$TRIPLET" \
        "sdl2:$TRIPLET" \
        "imgui[docking-experimental,opengl3-binding]:$TRIPLET" \
        "glad:$TRIPLET" \
        --recurse

    echo "  All vcpkg packages installed."
fi

# --- Configure CMake ---

step "Configuring CMake ($CONFIG)"

mkdir -p "$BUILD_DIR"

CMAKE_ARGS=(
    -S "$EDITOR_DIR"
    -B "$BUILD_DIR"
    -DCMAKE_BUILD_TYPE="$CONFIG"
)

if ! $USE_SYSTEM_PACKAGES && [[ -n "$VCPKG_ROOT" ]]; then
    CMAKE_ARGS+=(
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
        -DVCPKG_TARGET_TRIPLET="$TRIPLET"
    )
fi

cmake "${CMAKE_ARGS[@]}"

# --- Build ---

step "Building ($CONFIG)"

cmake --build "$BUILD_DIR" --config "$CONFIG" --parallel "$(nproc)"

echo "  Build succeeded."

# --- Package (optional) ---

if [[ -n "$PACKAGE_TYPE" ]]; then
    step "Packaging ($PACKAGE_TYPE)"

    STAGING_DIR="$SCRIPT_DIR/staging"
    rm -rf "$STAGING_DIR"
    mkdir -p "$STAGING_DIR/usr/local/bin"

    for exe in whetstone_editor orchestrator; do
        if [[ -f "$BUILD_DIR/$exe" ]]; then
            cp "$BUILD_DIR/$exe" "$STAGING_DIR/usr/local/bin/"
            echo "  Staged $exe"
        fi
    done

    case "$PACKAGE_TYPE" in
        deb)
            mkdir -p "$STAGING_DIR/DEBIAN"
            cat > "$STAGING_DIR/DEBIAN/control" <<CTRL
Package: whetstone-editor
Version: 0.1.0
Section: devel
Priority: optional
Architecture: amd64
Depends: libsdl2-2.0-0, libgl1
Maintainer: Whetstone DSL Project
Description: Whetstone DSL Editor and Orchestrator
 A visual editor for the Whetstone domain-specific language
 with semantic annotation support.
CTRL
            dpkg-deb --build "$STAGING_DIR" "$SCRIPT_DIR/whetstone-editor_0.1.0_amd64.deb"
            echo "  Created whetstone-editor_0.1.0_amd64.deb"
            ;;
        tar)
            tar -czf "$SCRIPT_DIR/whetstone-editor-0.1.0-linux-x64.tar.gz" \
                -C "$STAGING_DIR/usr/local" bin/
            echo "  Created whetstone-editor-0.1.0-linux-x64.tar.gz"
            ;;
        *)
            echo "WARNING: Unknown package type '$PACKAGE_TYPE'. Use 'deb' or 'tar'."
            ;;
    esac
fi

step "Done!"
echo "  Build output: $BUILD_DIR"
echo ""
