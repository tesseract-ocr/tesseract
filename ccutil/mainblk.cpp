/**********************************************************************
 * File:        mainblk.c  (Formerly main.c)
 * Description: Function to call from main() to setup.
 * Author:					Ray Smith
 * Created:					Tue Oct 22 11:09:40 BST 1991
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

#include          "fileerr.h"
#ifdef __UNIX__
#include          <unistd.h>
#include          <signal.h>
#else
#include          <io.h>
#endif
#include          <stdlib.h>
#include          "ccutil.h"

#define VARDIR        "configs/" /**< variables files */
#define EXTERN

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
 * An actual value of argv0 is used if not NULL, otherwise TESSDATA_PREFIX is
 * used if not NULL, next try to use compiled in -DTESSDATA_PREFIX. If previous
 * is not sucessul - use current directory.
 * @param basename - name of image
 */
void CCUtil::main_setup(const char *argv0, const char *basename) {
  imagebasename = basename;      /**< name of image */

  if (argv0 != NULL) {
    datadir = argv0;
  } else {
    if (getenv("TESSDATA_PREFIX")) {
      datadir = getenv("TESSDATA_PREFIX");
    } else {
#ifdef TESSDATA_PREFIX
#define _STR(a) #a
#define _XSTR(a) _STR(a)
    datadir = _XSTR(TESSDATA_PREFIX);
#undef _XSTR
#undef _STR
#endif
    }
  }

  // datadir may still be empty:
  if (datadir.length() == 0) {
    datadir = "./";
  } else {
    // Remove tessdata from the end if present, as we will add it back!
    int length = datadir.length();
    if (length >= 8 && strcmp(&datadir[length - 8], "tessdata") == 0)
      datadir.truncate_at(length - 8);
    else if (length >= 9 && strcmp(&datadir[length - 9], "tessdata/") == 0)
      datadir.truncate_at(length - 9);
  }

  // check for missing directory separator
  const char *lastchar = datadir.string();
  lastchar += datadir.length() - 1;
  if ((strcmp(lastchar, "/") != 0) && (strcmp(lastchar, "\\") != 0))
    datadir += "/";

  datadir += m_data_sub_dir;     /**< data directory */
}
}  // namespace tesseract
