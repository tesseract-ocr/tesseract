/**********************************************************************
 * File:        mainblk.cpp  (Formerly main.c)
 * Description: Function to call from main() to setup.
 * Author:      Ray Smith
 * Created:     Tue Oct 22 11:09:40 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include <cstdlib>
#if defined(_WIN32)
#include <io.h>         // for _access
#endif

#include "fileerr.h"
#include "ccutil.h"

const ERRCODE NO_PATH =
"Warning:explicit path for executable will not be used for configs";
static const ERRCODE USAGE = "Usage";

namespace tesseract {
/**********************************************************************
 * main_setup
 *
 * Main for mithras demo program. Read the arguments and set up globals.
 **********************************************************************/

/**
 * @brief CCUtil::main_setup - set location of tessdata and name of image
 *
 * @param argv0 - paths to the directory with language files and config files.
 * An actual value of argv0 is used if not nullptr, otherwise TESSDATA_PREFIX is
 * used if not nullptr, next try to use compiled in -DTESSDATA_PREFIX. If previous
 * is not successful - use current directory.
 * @param basename - name of image
 */
void CCUtil::main_setup(const char *argv0, const char *basename) {
  imagebasename = basename;      /**< name of image */

  char *tessdata_prefix = getenv("TESSDATA_PREFIX");

  if (argv0 != nullptr && *argv0 != '\0') {
    /* Use tessdata prefix from the command line. */
    datadir = argv0;
  } else if (tessdata_prefix) {
    /* Use tessdata prefix from the environment. */
    datadir = tessdata_prefix;
#if defined(_WIN32)
  } else if (datadir == nullptr || _access(datadir.string(), 0) != 0) {
    /* Look for tessdata in directory of executable. */
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char path[_MAX_PATH];
    DWORD length = GetModuleFileName(nullptr, path, sizeof(path));
    if (length > 0 && length < sizeof(path)) {
      errno_t result = _splitpath_s(path, drive, sizeof(drive),
                                    dir, sizeof(dir), nullptr, 0, nullptr, 0);
      if (result == ERANGE) {
        tprintf("Error: Path too long: %s\n", path);
      }

      datadir = drive;
      datadir += dir;
      datadir += "/tessdata";
    }
#endif /* _WIN32 */
#if defined(TESSDATA_PREFIX)
  } else {
/* Use tessdata prefix which was compiled in. */
#define _STR(a) #a
#define _XSTR(a) _STR(a)
    datadir = _XSTR(TESSDATA_PREFIX) "/tessdata";
#undef _XSTR
#undef _STR
#endif
  }

  // datadir may still be empty:
  if (datadir.length() == 0) {
    datadir = "./";
  }

  // check for missing directory separator
  const char *lastchar = datadir.string();
  lastchar += datadir.length() - 1;
  if ((strcmp(lastchar, "/") != 0) && (strcmp(lastchar, "\\") != 0))
    datadir += "/";
}
}  // namespace tesseract
