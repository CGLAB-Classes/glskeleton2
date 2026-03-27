#pragma once

/// @file utils.h
/// @brief Platform utilities for glskeleton.

#include <filesystem>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace glskeleton {

/// Return the directory containing the running executable.
///
/// Works on Windows, Linux, and macOS regardless of the current
/// working directory.
inline std::filesystem::path getExecutableDir() {
#if defined(_WIN32)
  wchar_t buf[MAX_PATH];
  GetModuleFileNameW(nullptr, buf, MAX_PATH);
  return std::filesystem::canonical(buf).parent_path();
#elif defined(__linux__)
  return std::filesystem::canonical("/proc/self/exe").parent_path();
#elif defined(__APPLE__)
  char buf[1024];
  uint32_t size = sizeof(buf);
  _NSGetExecutablePath(buf, &size);
  return std::filesystem::canonical(buf).parent_path();
#else
#error "Unsupported platform"
#endif
}

/// Return the absolute path to the resources directory next to the executable.
inline std::filesystem::path getResourceDir() {
  return getExecutableDir() / "resources";
}

} // namespace glskeleton
