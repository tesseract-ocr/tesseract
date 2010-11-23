/**********************************************************************
 * File:        debugwin.cpp
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
#ifdef _MSC_VER
#pragma warning(disable:4554)  // Precedence warnings (line 101)
#endif

#include          "mfcpch.h"     //precompiled headers
#include          <stdarg.h>
#include          "debugwin.h"
#include          "params.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

// Should remain a global parameter, since this is only used for debug editor.
DLLSYM INT_VAR (debug_lines, 256, "Number of lines in debug window");

#ifndef GRAPHICS_DISABLED

#ifdef __MAC__
#include          <ltextedit.h>
#include          <lwindow.h>
//#include                                              <console.h>

#define scrl_SCROLLER   101
#define text_FLOWED     100

// Should remain a global variable, since this is only used for debug editor.
static LCommander *pCommander = NULL;
#endif

                                 //NT implementation
#if defined(__MSW32__) && !defined(_CONSOLE)

#include          <io.h>
#define ID_DEBUG_MSG       32779

/**********************************************************************
 * DEBUG_WIN::DEBUG_WIN
 *
 * Create a debug window with size according to the arguments.
 **********************************************************************/

DEBUG_WIN::DEBUG_WIN(                    //constructor
                     const char *title,  //of window
                     inT32 xpos,         //initial position
                     inT32 ypos,         //in pixels
                     inT32 xsize,        //initial size
                     inT32 ysize,        //in pixels
                     inT32 buflines      //default scroll size
                    ) {
  char cmd[1024];
  int parm;                      //output from scrolrwin
  STARTUPINFO start_info;        //process control
  PROCESS_INFORMATION proc_info; //process ids
  SECURITY_ATTRIBUTES security;  //for handles

  handle = NULL;
  shm_hand = NULL;
  shm_mem = NULL;
  msg_end = NULL;
  dbg_process = NULL;            //save handles
  dbg_thread = NULL;
  security.nLength = sizeof (security);
  security.lpSecurityDescriptor = NULL;
  security.bInheritHandle = TRUE;//make it inheritable
                                 //anonymous
  shm_hand = CreateFileMapping ((HANDLE) 0xffffffff, &security, PAGE_READWRITE, 0, 4096, NULL);
  if (shm_hand == NULL)
    return;                      //failed
  shm_mem = (char *) MapViewOfFile (shm_hand, FILE_MAP_WRITE, 0, 0, 0);
  if (shm_mem == NULL)
    return;
  shm_mem[5] = 0;
  sprintf (cmd, "scrolwin.exe %d %d", buflines, shm_hand);
  GetStartupInfo(&start_info);  //clone our stuff
  if (!CreateProcess (NULL, cmd, NULL, NULL, TRUE,
    CREATE_NO_WINDOW | DETACHED_PROCESS | CREATE_SUSPENDED,
    NULL, NULL, &start_info, &proc_info))
    return;

                                 //save handles
  dbg_process = proc_info.hProcess;
  dbg_thread = proc_info.hThread;
  if (ResumeThread (dbg_thread) != 1)
    return;
  do
  Sleep (100);
  while (shm_mem[5] == 0);       //wait for handle
  parm = ((((uinT8) shm_mem[4] << 8) + (uinT8) shm_mem[3] << 8)
    + (uinT8) shm_mem[2] << 8) + (uinT8) shm_mem[1];
  handle = (HWND) parm;
  if (handle != NULL) {
                                 //setup window
    ::SetWindowText (handle, title);
    ::MoveWindow (handle, xpos, ypos, xsize, ysize, TRUE);
    ::ShowWindow (handle, SW_SHOW);
  }
}


/**********************************************************************
 * DEBUG_WIN::DEBUG_WIN
 *
 * Destroy a debug window.
 **********************************************************************/

DEBUG_WIN::~DEBUG_WIN (
//destructor
) {
  if (IsWindow (handle))
    ::SendMessage (handle, WM_COMMAND, IDOK, 0);
  if (shm_mem != NULL)
    UnmapViewOfFile(shm_mem);
  if (shm_hand != NULL)
    CloseHandle(shm_hand);
  if (dbg_thread != NULL)
    CloseHandle(dbg_thread);
  if (dbg_process == NULL)
    CloseHandle(dbg_process);

}


/**********************************************************************
 * dprintf
 *
 * Print a message to the debug window.
 * Like printf, this function can cope with messages which do not end
 * in newline, but nothing is printed until the newline is received.
 **********************************************************************/

void
DEBUG_WIN::dprintf (             //debug printf
const char *format, ...          //special message
) {
  va_list args;                  //variable args
  char *msg_start;               //for printing

  if (!IsWindow (handle))
    return;                      //destroyed
  if (msg_end == NULL)
    msg_end = shm_mem + 1;
  va_start(args, format);  //variable list
                                 //Format into msg
  vsprintf(msg_end, format, args);
  va_end(args);
  if (*msg_end == '\0')
    return;
  msg_start = shm_mem + 1;
  do {
                                 //end of line
    msg_end = strchr (msg_start, '\n');
    if (msg_end == NULL) {
      if (msg_start != shm_mem + 1)
                                 //bring to front
        strcpy (shm_mem + 1, msg_start);
                                 //current end
      msg_end = shm_mem + 1 + strlen (shm_mem + 1);
      return;
    }
    *msg_end = '\0';
    while (IsWindow (handle) && shm_mem[0])
      Sleep (500);
    if (IsWindow (handle)) {
                                 //Visual C++2.0 macro
      ::SendMessage (handle, WM_COMMAND, ID_DEBUG_MSG, (DWORD) (msg_start - shm_mem));
    }
    msg_start = msg_end + 1;
  }
  while (*msg_start != '\0');
  msg_end = shm_mem + 1;         //buffer empty
}


/**********************************************************************
 * await_destruction
 *
 * Wait for the user to close the debug window. Then return.
 **********************************************************************/

void DEBUG_WIN::await_destruction() {  //wait for user to close
  WaitForSingleObject (dbg_process, (unsigned long) -1);
}
#endif                           //NT Implmentation

                                 //UNIX implementation
#if defined(__UNIX__) || defined(_CONSOLE)
#ifdef __UNIX__
#include          <unistd.h>
#include          <signal.h>
#endif
//#include                                      "basefile.h"

/**********************************************************************
 * DEBUG_WIN::DEBUG_WIN
 *
 * Create a debug window with size according to the arguments.
 * Create an hpterm window with a pipe connected to it.
 **********************************************************************/

DEBUG_WIN::DEBUG_WIN(                    //constructor
                     const char *title,  //of window
                     inT32 xpos,         //initial position
                     inT32 ypos,         //in pixels
                     inT32 xsize,        //initial size
                     inT32 ysize,        //in pixels
                     inT32 buflines      //default scroll size
                    ) {
  #ifdef __UNIX__
  inT32 length;                  /*length of name */
  char command[MAX_PATH];        /*pipe command */
  pid_t pid;                     /*process id */
  char host[MAX_PATH];           //remote host
  BOOL8 remote;                  //remote host

  //      remote=remote_display(host);                                                                    //check remote host
  remote = FALSE;
  if (remote)
                                 //do it remotely
    length = sprintf (command, "remsh %s 'DISPLAY=%s;export DISPLAY;", host, getenv ("DISPLAY"));
  else
    length = 0;
  length += sprintf (command + length, "trap \"\" 1 2 3 13 15\n");
  length +=
    sprintf (command + length,
    "/usr/bin/xterm -sb -sl " INT32FORMAT " -geometry "
    INT32FORMAT "x" INT32FORMAT "", buflines, xsize / 8, ysize / 16);
  if (xpos >= 0)
    command[length++] = '+';
  length += sprintf (command + length, INT32FORMAT, xpos);
  if (ypos >= 0)
    command[length++] = '+';
  length +=
    sprintf (command + length,
    INT32FORMAT " -title \"%s\" -n \"%s\" -e /bin/sh -c ", ypos,
    title, title);
  pid = getpid ();               /*random number */
  length +=
    sprintf (command + length,
    "\"stty opost; tty >/tmp/debug%d; while [ -s /tmp/debug%d ]\ndo\nsleep 1\ndone\" &\n",
    pid, pid);
  length +=
    sprintf (command + length, "trap \"rm -f /tmp/debug%d; kill -9 $!\" 0\n",
    pid);
  length += sprintf (command + length, "trap \"exit\" 1 2 3 13 15\n");
  length +=
    sprintf (command + length,
    "while [ ! -s /tmp/debug%d ]\ndo\nsleep 1\ndone\n", pid);
  length += sprintf (command + length, "trap \"\" 1 2 3 13 15\n");
  length += sprintf (command + length, "ofile=`cat /tmp/debug%d`\n", pid);
  length +=
    sprintf (command + length, "cat -u - >$ofile; rm /tmp/debug%d\n", pid);
  if (remote) {
    command[length++] = '\'';    //terminate remsh
    command[length] = '\0';
  }
  fp = popen (command, "w");     /*create window */
  if (fp != NULL) {
                                 /*set no buffering */
    if (setvbuf (fp, NULL, _IONBF, BUFSIZ)) {
      pclose(fp);
      fp = NULL;
    }
  }
  #endif
}


/**********************************************************************
 * DEBUG_WIN::DEBUG_WIN
 *
 * Close the file and destroy the window.
 **********************************************************************/

DEBUG_WIN::~DEBUG_WIN (
//destructor
) {
  #ifdef __UNIX__
  pclose(fp);
  #endif
}


/**********************************************************************
 * dprintf
 *
 * Print a message to the debug window.
 * Like printf, this function can cope with messages which do not end
 * in newline, but nothing is printed until the newline is received.
 **********************************************************************/

void
DEBUG_WIN::dprintf (             //debug printf
const char *format, ...          //special message
) {
  va_list args;                  //variable args

  va_start(args, format);  //variable list
  #ifdef __UNIX__
  vfprintf(fp, format, args);  //Format into msg
  #else
                                 //Format into msg
  vfprintf(stderr, format, args);
  #endif
  va_end(args);
}


/**********************************************************************
 * await_destruction
 *
 * Wait for the user to close the debug window. Then return.
 **********************************************************************/

void DEBUG_WIN::await_destruction() {  //wait for user to close
  #ifdef __UNIX__
  signal(SIGPIPE, SIG_IGN);
  while (!ferror (fp)) {
    sleep (1);
    fputc (0, fp);               //send nulls until error
  }
  #endif
}
#endif                           //UNIX Implmentation

#ifdef __MAC__                   //NT implementation
#include          <stdio.h>
//#include                                                      "textwindow.h"
#include          <lwindow.h>
#include          "ipcbase.h"    //must be last include

// Until I can figure a way to do this without linking in PowerPlant,
// the debug window will just have empty functions so compilation can take place.

/**********************************************************************
 * DEBUG_WIN::SetCommander
 *
 * Mac-specific function to set the commander for the next debug window
 **********************************************************************/
void DEBUG_WIN::SetCommander(LCommander *pNew) {
  pCommander = pNew;
}


/**********************************************************************
 * DEBUG_WIN::DEBUG_WIN
 *
 * Create a debug window with size according to the arguments.
 * Create an hpterm window with a pipe connected to it.
 **********************************************************************/

DEBUG_WIN::DEBUG_WIN(                    //constructor
                     const char *title,  //of window
                     inT32 xpos,         //initial position
                     inT32 ypos,         //in pixels
                     inT32 xsize,        //initial size
                     inT32 ysize,        //in pixels
                     inT32 buflines      //default scroll size
                    ) {
  inT32 length;                  /*length of name */

  // don't replace this DebugStr() with a call to DEBUG_WIN!

  //if (pCommander==NULL) DebugStr("\pDEBUG_WIN::DEBUG_WIN(), Commander not set");

  // create the window

  //pWindow=LWindow::CreateWindow(2700,pCommander);
}


/**********************************************************************
 * DEBUG_WIN::DEBUG_WIN
 *
 * Close the file and destroy the window.
 **********************************************************************/

DEBUG_WIN::~DEBUG_WIN (
//destructor
) {
}


/**********************************************************************
 * dprintf
 *
 * Print a message to the debug window.
 * Like printf, this function can cope with messages which do not end
 * in newline, but nothing is printed until the newline is received.
 **********************************************************************/

void
DEBUG_WIN::dprintf (             //debug printf
const char *format, ...          //special message
) { 
}
#endif                           //Mac Implmentation

#else                            // Non graphical debugger

DEBUG_WIN::DEBUG_WIN( const char*, inT32, inT32, inT32, inT32, inT32 ) {
}

DEBUG_WIN::~DEBUG_WIN () {
}

void DEBUG_WIN::dprintf (const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}

void await_destruction() {
}


#endif
