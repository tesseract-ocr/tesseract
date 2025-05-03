/**********************************************************************
 * File:        tprintf.cpp
 * Description: Trace version of printf - portable between UX and NT
 * Author:      Phil Cheatle
 *
 * (C) Copyright 1995, Hewlett-Packard Ltd.
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "tesserrstream.h"
#include "tprintf.h"

#include "params.h"

#include <climits> // for INT_MAX
#include <cstdarg>
#include <cstdio>

namespace tesseract {

INT_VAR(log_level, INT_MAX, "Logging level");

static STRING_VAR(debug_file, "", "File to send tprintf output to");

// File for debug output.
FILE *debugfp;

// Set output for log messages.
// The output is written to stderr if debug_file is empty.
// Otherwise it is written to debug_file.
// It is possible to switch between stderr and debug_file output:
// tprintf("write to configured output\n");
// debug_file = "";
// tprintf("write to stderr\n");
// debug_file = "/tmp/log";
// tprintf("write to /tmp/log\n");
// debug_file = "";
// tprintf("write to stderr\n");
FILE *get_debugfp() {
  if (debug_file.empty()) {
    // Write to stderr.
    if (debugfp != stderr && debugfp != nullptr) {
      fclose(debugfp);
    }
    debugfp = stderr;
  } else if (debugfp == stderr || debugfp == nullptr) {
    // Write to file.
#ifdef _WIN32
    if (debug_file == "/dev/null") {
      // Replace /dev/null by nul for Windows.
      debug_file = "nul";
    }
#endif
    debugfp = fopen(debug_file.c_str(), "wb");
  }
  return debugfp;
}

TessErrStream tesserr;

} // namespace tesseract
