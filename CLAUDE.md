# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

GLSkeleton2 — an OpenGL Core Profile rendering skeleton (C++17) for 3D graphics coursework. The `pa1` branch implements a reference solution with animated mesh rendering, OBJ loading, and wireframe/normal visualization modes.

## Build Commands

```bash
# First-time setup (clones submodules, installs system deps)
# Linux/macOS:
./scripts/setup.sh
# Windows PowerShell:
.\scripts\setup.ps1

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)

# Run the executable (from repo root)
./build/glskeleton2    # Linux/macOS
.\build\Release\glskeleton2.exe   # Windows MSVC

# Optional: embed shaders into binary for distribution
cmake -B build -DCMAKE_BUILD_TYPE=Release -DEMBED_SHADERS=ON
```

There are no tests or linters configured.

## Architecture

**Dependencies** (git submodules in `ext/`): nanogui (windowing + GLFW + GLAD + GUI), glm (math), tinyobjloader (OBJ parsing).

**Core classes:**
- `Mesh` (`include/glskeleton/mesh.h` + `src/mesh.cpp`) — RAII VAO/VBO/EBO wrapper. Loads OBJ files with vertex deduplication via `Mesh::fromOBJ()`. Interleaved vertex layout: position (loc 0) + normal (loc 1), 24 bytes/vertex.
- `Shader` (`include/glskeleton/shader.h`, header-only) — compiles/links GLSL programs, provides typed uniform setters using `glm::value_ptr`.
- `utils.h` (header-only) — platform-specific executable-relative resource path resolution (Windows/Linux/macOS).

**Application** (`src/main.cpp`): `GLSkeletonApp` extends `nanogui::Screen`. Manages a vector of meshes, runs an animation loop with random-walk rotation and sine-wave color cycling. Keyboard: W=wireframe toggle, N=normal-viz toggle.

**Shaders** (`resources/shaders/`): GLSL 3.30 core. `wireframe.vert/frag` are the main shaders (uniform `renderMode`: 0=solid color, 1=normal visualization). `basic.vert/frag` are starter templates.

**Build system**: CMake 3.22+. Post-build step copies `resources/` next to the executable. MSVC uses static CRT. Generates `compile_commands.json` for LSP/clangd.

## CI

GitHub Actions builds on Ubuntu 24.04, Ubuntu 22.04, Windows (MSVC), and macOS (Clang). Pipeline: setup script → cmake configure → cmake build.
