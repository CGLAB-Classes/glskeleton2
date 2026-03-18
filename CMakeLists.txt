cmake_minimum_required(VERSION 3.22)
project(glskeleton LANGUAGES C CXX)

# --- Project-wide settings (only when glskeleton is the top-level project) ---

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
      Debug Release MinSizeRel RelWithDebInfo)
  endif()
endif()

# Static CRT on MSVC
if (MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if (CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# --- INTERFACE targets for project-wide options and warnings -----------------

add_library(glskeleton_options INTERFACE)
add_library(glskeleton::options ALIAS glskeleton_options)
target_compile_features(glskeleton_options INTERFACE cxx_std_17)

if (APPLE)
  target_compile_definitions(glskeleton_options INTERFACE GL_SILENCE_DEPRECATION=1)
endif()

if (MSVC)
  target_compile_options(glskeleton_options INTERFACE /MP)
  target_compile_definitions(glskeleton_options INTERFACE _CRT_SECURE_NO_WARNINGS)
endif()

add_library(glskeleton_warnings INTERFACE)
add_library(glskeleton::warnings ALIAS glskeleton_warnings)

if (MSVC)
  target_compile_options(glskeleton_warnings INTERFACE /W4)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(glskeleton_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Wno-unused-parameter)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(glskeleton_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# Ninja: force colored diagnostics
if (CMAKE_GENERATOR STREQUAL "Ninja")
  target_compile_options(glskeleton_warnings INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-fcolor-diagnostics>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fdiagnostics-color=always>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-fcolor-diagnostics>
    $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
  )
endif()

# --- Dependencies ------------------------------------------------------------

add_subdirectory(ext)

# --- Main executable ---------------------------------------------------------

add_executable(glskeleton)

target_include_directories(glskeleton PRIVATE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

# Add your source files here.
target_sources(glskeleton
  PRIVATE
    src/main.cpp
)

target_link_libraries(glskeleton
  PRIVATE
    glskeleton::options
    glskeleton::warnings
    nanogui
    glm::glm
    tinyobjloader
)

# Copy resources (shaders, etc.) next to the executable
add_custom_command(TARGET glskeleton POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:glskeleton>/resources
  COMMENT "Copying resources to build output directory"
)

# Copy compile_commands.json to project root for clangd/LSP support
if (CMAKE_EXPORT_COMPILE_COMMANDS AND NOT CMAKE_GENERATOR MATCHES "Visual Studio")
  add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_BINARY_DIR}/compile_commands.json
      ${PROJECT_SOURCE_DIR}/compile_commands.json
    COMMENT "Copying compile_commands.json to project root"
  )
endif()
