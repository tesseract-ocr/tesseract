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
#include "config_auto.h"
#endif

#include <cstdio>
#include <cstdarg>
#include "params.h"
#include <tesseract/strngs.h>
#include "tprintf.h"

#define MAX_MSG_LEN 2048

static STRING_VAR(debug_file, "", "File to send tprintf output to");

// Trace printf
DLLSYM void tprintf(const char *format, ...)
{
  const char* debug_file_name = debug_file.c_str();
  static FILE *debugfp = nullptr;   // debug file

  if (debug_file_name == nullptr) {
    // This should not happen.
    return;
  }

#ifdef _WIN32
  // Replace /dev/null by nul for Windows.
  if (strcmp(debug_file_name, "/dev/null") == 0) {
    debug_file_name = "nul";
    debug_file.set_value(debug_file_name);
  }
#endif

  if (debugfp == nullptr && debug_file_name[0] != '\0') {
    debugfp = fopen(debug_file_name, "wb");
  } else if (debugfp != nullptr && debug_file_name[0] == '\0') {
    fclose(debugfp);
    debugfp = nullptr;
  }

  va_list args;            // variable args
  va_start(args, format);  // variable list
  if (debugfp != nullptr) {
    vfprintf(debugfp, format, args);
  } else {
    vfprintf(stderr, format, args);
  }
  va_end(args);
}
