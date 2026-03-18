# ============================================================================
# External dependencies for glskeleton
# ============================================================================

# --- Verify submodules -------------------------------------------------------

foreach(required_path IN ITEMS
  "${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/glfw/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/glm/CMakeLists.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader/CMakeLists.txt"
)
  if (NOT EXISTS "${required_path}")
    message(FATAL_ERROR
      "A required dependency is missing: ${required_path}\n"
      "Run: git submodule update --init --recursive")
  endif()
endforeach()

# --- NanoGUI -----------------------------------------------------------------
# Provides: windowing (GLFW), OpenGL loader (GLAD), vector graphics (NanoVG),
#           and GUI widgets. Forces OpenGL backend.
#
# On Linux, Wayland support is optional. If the required dev packages are
# not installed, fall back to X11-only so the build does not fail.

if (UNIX AND NOT APPLE)
  find_package(PkgConfig QUIET)
  if (PkgConfig_FOUND)
    pkg_check_modules(_wayland QUIET
      wayland-client wayland-cursor wayland-egl xkbcommon dbus-1)
    if (NOT _wayland_FOUND)
      set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
      message(STATUS "Wayland dev packages not found. Building with X11 only.")
    endif()
  else()
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
  endif()
endif()

set(NANOGUI_BACKEND "OpenGL" CACHE STRING "" FORCE)
set(NANOGUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(NANOGUI_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(NANOGUI_BUILD_PYTHON OFF CACHE BOOL "" FORCE)
set(NANOGUI_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(nanogui)

# --- GLM ---------------------------------------------------------------------

add_subdirectory(glm)

# Older GLM versions (0.9.9.x) only create a "glm" target.
# Newer versions (1.0+) create "glm::glm" directly.
if (TARGET glm AND NOT TARGET glm::glm)
  add_library(glm::glm ALIAS glm)
endif()

# --- tinyobjloader -----------------------------------------------------------

set(TINYOBJLOADER_BUILD_TEST_LOADER OFF CACHE BOOL "" FORCE)
set(TINYOBJLOADER_BUILD_OBJ_STICHER OFF CACHE BOOL "" FORCE)

add_subdirectory(tinyobjloader)
