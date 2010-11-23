/**********************************************************************
 * File:        tprintf.c
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

#include          "mfcpch.h"     //precompiled headers

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include          <stdio.h>
#include          <stdarg.h>
#include          "strngs.h"
#include          "params.h"
#include              "debugwin.h"
//#include                                      "ipeerr.h"
#include          "tprintf.h"
#include          "ccutil.h"

#define MAX_MSG_LEN     1024

#define EXTERN
// Since tprintf is protected by a mutex, these parameters can rmain global.
DLLSYM STRING_VAR (debug_file, "", "File to send tprintf output to");
DLLSYM BOOL_VAR (debug_window_on, FALSE,
"Send tprintf to window unless file set");

DLLSYM void
tprintf (                        //Trace printf
const char *format, ...          //special message
) {
  tesseract::tprintfMutex.Lock();
  va_list args;                  //variable args
  static FILE *debugfp = NULL;   //debug file
                                 //debug window
  static DEBUG_WIN *debugwin = NULL;
  inT32 offset = 0;              //into message
  static char msg[MAX_MSG_LEN + 1];

  va_start(args, format);  //variable list
  #ifdef __MSW32__
                                 //Format into msg
  offset += _vsnprintf (msg + offset, MAX_MSG_LEN - offset, format, args);
  #else
                                 //Format into msg
  offset += vsprintf (msg + offset, format, args);
  #endif
  va_end(args);

  if (debugfp == NULL && strlen (debug_file.string ()) > 0)
    debugfp = fopen (debug_file.string (), "w");
  else if (debugfp != NULL && strlen (debug_file.string ()) == 0) {
    fclose(debugfp);
    debugfp = NULL;
  }
  if (debugfp != NULL)
    fprintf (debugfp, "%s", msg);
  else {

    if (debug_window_on) {
      if (debugwin == NULL)
                                 //in pixels
        debugwin = new DEBUG_WIN ("Debug Window", DEBUG_WIN_XPOS, DEBUG_WIN_YPOS,
                                 //in pixels
          DEBUG_WIN_XSIZE, DEBUG_WIN_YSIZE,
          debug_lines);
      debugwin->dprintf (msg);
    }
    else {
      fprintf (stderr, "%s", msg);
    }
  }
  tesseract::tprintfMutex.Unlock();
}


/*************************************************************************
 * pause_continue()
 * UI for a debugging pause - to see an intermediate state
 * Returns TRUE to continue as normal to the next pause in the current mode;
 * FALSE to quit the current pausing mode.
 *************************************************************************/

DLLSYM BOOL8
                                 //special message
pause_continue (const char *format, ...
) {
  va_list args;                  //variable args
  char msg[1000];
  STRING str = STRING ("DEBUG PAUSE:\n");

  va_start(args, format);  //variable list
  vsprintf(msg, format, args);  //Format into msg
  va_end(args);

  #ifdef GRAPHICS_DISABLED
  // No interaction allowed -> simply go on
  return true;
  #else

  #ifdef __UNIX__
  printf ("%s\n", msg);
  printf ("Type \"c\" to cancel, anything else to continue: ");
  char c = getchar ();
  return (c != 'c');
  #endif

  #ifdef __MSW32__
  str +=
    STRING (msg) + STRING ("\nUse OK to continue, CANCEL to stop pausing");
  //   return AfxMessageBox( str.string(), MB_OKCANCEL ) == IDOK;
  return::MessageBox (NULL, msg, "IMGAPP",
    MB_APPLMODAL | MB_OKCANCEL) == IDOK;
  #endif

  #endif
}
