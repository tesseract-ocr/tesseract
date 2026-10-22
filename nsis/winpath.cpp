// Copyright (C) 2024 Stefan Weil
//
// SPDX-License-Identifier: Apache-2.0
//
// winpath - run a Windows program with extended PATH
//
// Usage:
//
//     winpath [CMD [ARGUMENT ...]]
//
// Example:
//
//     winpath cmd
//
// This will start a Windows command line with PATH extended by
// the location of the winpath executable.

// Keep windows.h lean and avoid its min/max macros, which conflict with
// the standard library.
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <process.h> // _spawnvp
#include <stdlib.h>  // _putenv_s
#include <windows.h> // GetEnvironmentVariable, GetModuleFileName
#include <string>    // std::string

// Return the full path of this executable.
static std::string exe_path() {
  std::string buffer(MAX_PATH, '\0');
  while (true) {
    DWORD len = GetModuleFileNameA(nullptr, &buffer[0],
                                   static_cast<DWORD>(buffer.size()));
    if (len == 0) {
      return std::string();
    }
    if (len < buffer.size()) {
      buffer.resize(len);
      return buffer;
    }
    // The buffer was too small, retry with a larger one.
    buffer.resize(buffer.size() * 2);
  }
}

// Return the directory containing this executable, without trailing separator.
static std::string exe_dir() {
  std::string path = exe_path();
  size_t pos = path.find_last_of('\\');
  if (pos == std::string::npos) {
    return std::string();
  }
  return path.substr(0, pos);
}

// Return the value of an environment variable, or an empty string if it is not
// set. The value is untrusted, but it is only copied here and never parsed or
// used to open a file, and std::string rules out a buffer overflow. The size is
// queried from the system instead of being guessed.
static std::string env_var(const char *name) {
  std::string buffer(MAX_PATH, '\0');
  while (true) {
    DWORD len = GetEnvironmentVariableA(name, &buffer[0],
                                        static_cast<DWORD>(buffer.size()));
    if (len == 0) {
      // The variable is not set or empty.
      return std::string();
    }
    if (len < buffer.size()) {
      buffer.resize(len);
      return buffer;
    }
    // The buffer was too small; len is the required size including the
    // terminating null character.
    buffer.resize(len);
  }
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    std::string path = exe_dir();
    std::string old_path = env_var("PATH");
    if (!old_path.empty()) {
      if (!path.empty()) {
        // An empty PATH element would search the current directory.
        path += ';';
      }
      path += old_path;
    }
    _putenv_s("PATH", path.c_str());
    _spawnvp(_P_WAIT, argv[1], argv + 1);
  }
  return 0;
}
