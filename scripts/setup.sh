#!/usr/bin/env bash
#
# setup.sh -- Install build dependencies and initialize git submodules.
#
# Usage:
#   chmod +x scripts/setup.sh
#   ./scripts/setup.sh
#

set -euo pipefail

# Detect standalone (piped) execution vs. running inside the repo
if [[ -n "${BASH_SOURCE[0]:-}" && "${BASH_SOURCE[0]}" != *"/dev/"* ]]; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
else
    PROJECT_DIR=""
fi

echo "=== glskeleton setup ==="

# --- Install system dependencies ---

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo ">> Detected Linux"

    if command -v apt-get &>/dev/null; then
        echo ">> Installing dependencies via apt..."
        sudo apt-get update
        sudo apt-get install -y \
            cmake build-essential git ninja-build \
            libgl1-mesa-dev libglu1-mesa-dev \
            libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
            pkg-config

        # Optional Wayland support
        echo ">> Installing optional Wayland development packages..."
        sudo apt-get install -y \
            libwayland-dev wayland-protocols libxkbcommon-dev libdbus-1-dev \
            || echo ">> Wayland packages not available. X11 will be used."
    else
        echo ">> Non-apt Linux detected. Please install the following manually:"
        echo "   cmake, build-essential, git, Mesa GL/GLU dev, X11 dev libs"
    fi

elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo ">> Detected macOS"

    if command -v brew &>/dev/null; then
        echo ">> Installing dependencies via Homebrew..."
        brew install cmake ninja
    else
        echo ">> Homebrew not found. Please install CMake manually:"
        echo "   https://cmake.org/download/"
    fi

else
    echo ">> Unsupported OS: $OSTYPE"
    echo ">> Please install cmake, git, and OpenGL dev libraries manually."
fi

# --- Initialize git submodules ---

if [[ -n "$PROJECT_DIR" ]] && git -C "$PROJECT_DIR" rev-parse --is-inside-work-tree &>/dev/null; then
    echo ">> Initializing git submodules..."
    cd "$PROJECT_DIR"
    git submodule update --init --recursive

    echo ""
    echo "=== Setup complete ==="
else
    echo ""
    echo "=== Dependency installation complete ==="
    echo ""
    echo "Next steps:"
    echo "  git clone --recursive <your_pa_url>"
fi
