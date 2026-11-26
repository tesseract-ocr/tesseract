///////////////////////////////////////////////////////////////////////
// File:        tempdir.h
// Description: Helper to create and remove a temporary directory for a
//              unit test. It replaces the POSIX-only mkdtemp()/rmdir()
//              calls, which are not available on all platforms.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_UNITTEST_UTIL_TEMPDIR_H_
#define TESSERACT_UNITTEST_UTIL_TEMPDIR_H_

#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <random>
#include <string>
#include <system_error>

namespace tesseract {

// Creates a uniquely named directory below the system temporary directory
// and returns its path. The name is built from the given prefix, a clock
// value, a per-process counter and - if the platform offers an entropy
// source - a random seed, so that concurrent test processes cannot pick the
// same name.
// Returns an empty string if the directory could not be created.
inline std::string CreateTempDir(const std::string &prefix) {
  namespace fs = std::filesystem;
  static std::atomic<unsigned> counter{0};
  const unsigned seq = counter.fetch_add(1);
  uint64_t seed = static_cast<uint64_t>(
      std::chrono::steady_clock::now().time_since_epoch().count());
  try {
    std::random_device rd;
    seed ^= static_cast<uint64_t>(rd()) << 32;
    seed ^= rd();
  } catch (const std::exception &) {
    // No entropy source available: the clock and the counter have to do.
  }
  std::error_code ec;
  const fs::path base = fs::temp_directory_path(ec);
  if (ec) {
    return "";
  }
  // Retry with an incremented counter in case the name is already taken.
  for (unsigned attempt = 0; attempt < 100; ++attempt) {
    const std::string name = prefix + std::to_string(seed) + "_" +
                             std::to_string(seq) + "_" +
                             std::to_string(attempt);
    const fs::path dir = base / name;
    if (fs::create_directory(dir, ec)) {
      return dir.string();
    }
  }
  return "";
}

// Removes the directory at the given path and everything in it.
// Does nothing if the path is empty or does not exist.
inline void RemoveTempDir(const std::string &dir) {
  if (dir.empty()) {
    return;
  }
  std::error_code ec;
  std::filesystem::remove_all(dir, ec);
}

} // namespace tesseract

#endif // TESSERACT_UNITTEST_UTIL_TEMPDIR_H_
