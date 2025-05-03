// Copyright 2008 Google Inc. All Rights Reserved.
// Author: scharron@google.com (Samuel Charron)
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ccutil.h"
#include "tesserrstream.h"  // for tesserr
#include "tprintf.h"  // for tprintf

#include <cstdlib>

namespace tesseract {

CCUtil::CCUtil()
    : params_()
      , INT_INIT_MEMBER(ambigs_debug_level, 0, "Debug level for unichar ambiguities", &params_)
      , BOOL_MEMBER(use_ambigs_for_adaption, false,
                  "Use ambigs for deciding"
                  " whether to adapt to a character",
                  &params_) {}

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of weak vtables in every compilation unit.
CCUtil::~CCUtil() = default;

/**
 * @brief Finds the path to the tessdata directory.
 *
 * This function determines the location of the tessdata directory based on the
 * following order of precedence:
 * 1. If `argv0` is provided, use it.
 * 2. If `TESSDATA_PREFIX` environment variable is set and the path exists, use
 *    it.
 * 3. On Windows, check for a "tessdata" directory in the executable's directory
 *    and use it.
 * 4. If `TESSDATA_PREFIX` is defined at compile time, use it.
 * 5. Otherwise, use the current working directory.
 *
 * @param argv0 argument to be considered as the data directory path.
 * @return The path to the tessdata directory or current directory.
 */
static std::filesystem::path find_data_path(const std::string &argv0) {
  // If argv0 is set, always use it even if it is not a valid directory
  if (!argv0.empty()) {
    std::filesystem::path path(argv0);
    if (!std::filesystem::is_directory(path)) {
      tesserr << "Warning (tessdata): '" << argv0 << "' is not a valid directory.\n";
    }
    return path;
  }

  // Check environment variable if argv0 is not specified
  if (const char *tessdata_prefix = std::getenv("TESSDATA_PREFIX")) {
    std::filesystem::path path(tessdata_prefix);
    if (std::filesystem::exists(path)) {
      return path;
    } else {
      tprintf("Warning: TESSDATA_PREFIX %s does not exist, ignoring.\n",
              tessdata_prefix);
    }
  }

#ifdef _WIN32
  // Windows-specific: check for 'tessdata' not existing in the executable
  // directory
  wchar_t path[MAX_PATH];
  if (DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
      length > 0 && length < MAX_PATH) {
    std::filesystem::path exe_path(path);
    auto tessdata_subdir = exe_path.parent_path() / "tessdata";
    if (std::filesystem::exists(tessdata_subdir)) {
      return tessdata_subdir;
    }
  }
#endif

  // Fallback to compile-time or current directory
#ifdef TESSDATA_PREFIX
  return std::filesystem::path(TESSDATA_PREFIX) / "tessdata";
#else
  return std::filesystem::current_path();
#endif
}


/**
 * @brief CCUtil::main_setup - set location of tessdata and name of image
 *
 * @param argv0 - paths to the directory with language files and config files.
 */
void CCUtil::main_setup(const std::string &argv0, const std::string &basename) {
  imagebasename = basename; /**< name of image */
  datadir = find_data_path(argv0);
}
} // namespace tesseract
