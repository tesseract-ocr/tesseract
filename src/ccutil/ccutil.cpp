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
#include "tprintf.h"  // for tprintf

#include <cstdlib>
#include <cstring>    // for std::strrchrA
#include <filesystem> // for std::filesystem

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
 * @brief CCUtil::main_setup - set location of tessdata and name of image
 *
 * @param argv0 - paths to the directory with language files and config files.
 * An actual value of argv0 is used if not nullptr, otherwise TESSDATA_PREFIX is
 * used if not nullptr, next try to use compiled in -DTESSDATA_PREFIX. If
 * previous is not successful - use current directory.
 * @param basename - name of image
 */
void CCUtil::main_setup(const std::string &argv0, const std::string &basename) {
  imagebasename = basename; /**< name of image */

  const char *tessdata_prefix = getenv("TESSDATA_PREFIX");

  // Ignore TESSDATA_PREFIX if there is no matching filesystem entry.
  if (tessdata_prefix != nullptr && !std::filesystem::exists(tessdata_prefix)) {
    tprintf("Warning: TESSDATA_PREFIX %s does not exist, ignore it\n", tessdata_prefix);
    tessdata_prefix = nullptr;
  }

  if (!argv0.empty()) {
    /* Use tessdata prefix from the command line. */
    datadir = argv0;
  } else if (tessdata_prefix) {
    /* Use tessdata prefix from the environment. */
    datadir = tessdata_prefix;
#if defined(_WIN32)
  } else if (datadir.empty() || !std::filesystem::exists(datadir)) {
    /* Look for tessdata in directory of executable. */
    char path[_MAX_PATH];
    DWORD length = GetModuleFileName(nullptr, path, sizeof(path));
    if (length > 0 && length < sizeof(path)) {
      char *separator = std::strrchr(path, '\\');
      if (separator != nullptr) {
        *separator = '\0';
        std::string subdir = path;
        subdir += "/tessdata";
        if (std::filesystem::exists(subdir)) {
          datadir = subdir;
        }
      }
    }
#endif /* _WIN32 */
  }

  // datadir may still be empty:
  if (datadir.empty()) {
#if defined(TESSDATA_PREFIX)
    // Use tessdata prefix which was compiled in.
    datadir = TESSDATA_PREFIX "/tessdata/";
    // Note that some software (for example conda) patches TESSDATA_PREFIX
    // in the binary, so it might be shorter. Recalculate its length.
    datadir.resize(std::strlen(datadir.c_str()));
#else
    datadir = "./";
#endif /* TESSDATA_PREFIX */
  }

  // check for missing directory separator
  const char lastchar = datadir.back();
  if (lastchar != '/' && lastchar != '\\') {
    datadir += '/';
  }
}

} // namespace tesseract
