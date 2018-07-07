/**********************************************************************
 * File:        tprintf.cpp
 * Description: Trace version of printf - portable between UX and NT
 * Author:      Phil Cheatle
 * Created:     Wed Jun 28 15:01:15 BST 1995
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
#include "ccutil.h"
#include "params.h"
#include "strngs.h"
#include "tprintf.h"

#define MAX_MSG_LEN     65536

static STRING_VAR(debug_file, "", "File to send tprintf output to");

// Trace printf
DLLSYM void tprintf(const char *format, ...)
{
  tesseract::tprintfMutex.Lock();
  va_list args;                  // variable args
  static FILE *debugfp = nullptr;   // debug file
                                 // debug window
  int32_t offset = 0;              // into message
  static char msg[MAX_MSG_LEN + 1];

  va_start(args, format);  // variable list
  // Format into msg
  #ifdef _WIN32
  offset += _vsnprintf(msg + offset, MAX_MSG_LEN - offset, format, args);
  if (strcmp(debug_file.string(), "/dev/null") == 0)
    debug_file.set_value("nul");
  #else
  offset += vsnprintf(msg + offset, MAX_MSG_LEN - offset, format, args);
  #endif
  va_end(args);

  if (debugfp == nullptr && strlen(debug_file.string()) > 0) {
    debugfp = fopen(debug_file.string(), "wb");
  } else if (debugfp != nullptr && strlen(debug_file.string()) == 0) {
    fclose(debugfp);
    debugfp = nullptr;
  }
  if (debugfp != nullptr)
    fprintf(debugfp, "%s", msg);
  else
    fprintf(stderr, "%s", msg);
  tesseract::tprintfMutex.Unlock();
}
