/**********************************************************************
 * File:        debugwin.h
 * Description: Portable debug window class.
 * Author:      Ray Smith
 * Created:     Wed Feb 21 15:36:59 MST 1996
 *
 * (C) Copyright 1996, Hewlett-Packard Co.
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

#ifndef           DEBUGWIN_H
#define           DEBUGWIN_H

#include          "host.h"
#include          "varable.h"

#ifdef __MAC__
#include          <lwindow.h>
#include          <lcommander.h>
#endif

//the following define the default position of a debug window
//if not specified at construction
#define DEBUG_WIN_XPOS    50     //default position
#define DEBUG_WIN_YPOS    30     //default position
#define DEBUG_WIN_XSIZE   700    //default size
#define DEBUG_WIN_YSIZE   300    //default size

//number of lines in the scrollable area of the window
extern DLLSYM INT_VAR_H (debug_lines, 256, "Number of lines in debug window");

//the API for the debug window is simple, see below.
//Most of its behaviour is its UI.
//It has a scrollable text area (most of the window)
//It has a stop control.
//It has a clear button.
//A dprintf to the window causes the text to be sent to the
//text area. If the stop control is set, then the dprintf
//blocks (does not display anything or return) until the stop
//is released.
//In multi-threaded apps, other threads and the UI continue to
//function during the stop. Only the calling thread is blocked.
//Pressing the clear button erases all text from the window.
//As text is sent to the window, it scrolls up so that the most
//recent output is visible. If the user has scrolled back, this
//does not happen. If the user scrolls back to the bottom, then
//the scrolling turns back on.
//If the user destroys the window, it never comes back.

class DLLSYM DEBUG_WIN
{
  public:
    //the constructor creates the window, the destructor kills it
    DEBUG_WIN (                  //constructor
      const char *title,         //of window
      inT32 xpos = DEBUG_WIN_XPOS,//initial position
      inT32 ypos = DEBUG_WIN_YPOS,//in pixels
                                 //initial size
      inT32 xsize = DEBUG_WIN_XSIZE,
                                 //in pixels
      inT32 ysize = DEBUG_WIN_YSIZE,
                                 //default scroll size (textlines)
      inT32 buflines = debug_lines);

    ~DEBUG_WIN ();               //destructor

    void dprintf (               //printf to window
      const char *format, ...);  //message

    void await_destruction();  //wait for user to close

  #ifdef __MAC__
    static void SetCommander(LCommander *pCommander);
  #endif

  private:

  #ifdef __MSW32__
    HWND handle;                 //handle to window
    char *shm_mem;               //shared memory
    char *msg_end;               //current string
    HANDLE shm_hand;             //handle to it
    HANDLE dbg_process;          //handle to it
    HANDLE dbg_thread;           //handle to it
  #endif
  #ifdef __UNIX__
    FILE *fp;                    /*return file */
  #endif

  #ifdef __MAC__
    LWindow *pWindow;
  #endif
};
#endif
