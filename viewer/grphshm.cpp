/**********************************************************************
 * File:        grphshm.c  (Formerly graphshm.c)
 * Description: Functions for the shared memory sbdaemon connection.
 * Author:      Ray Smith
 * Created:     Thu May 24 14:09:38 BST 1990
 *
 * (C) Copyright 1990, Hewlett-Packard Ltd.
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

#include          "mfcpch.h"
#include          "evntlst.h"
#ifdef __UNIX__
#include          <sys/ipc.h>
#include          <sys/shm.h>
//#include                                      <osfcn.h>
#include          <errno.h>
//#include                                      "pipes.h"
#include          "fileerr.h"
//#include                                      "grpherr.h"
//#include                                      "basefile.h"
#include          <unistd.h>
#elif defined(__MSW32__)
#include          <io.h>
#include          <fcntl.h>
#include          <process.h>
#include          <stdio.h>
#else
#endif
#include          <string.h>
#include          "grphics.h"
#include          "grphshm.h"

#define EXTERN

EXTERN SHMINFO shminfo;          /*shared memory */
EXTERN WINFD sbfds[MAXWINDOWS];  /*file descriptors */
EXTERN INT16 maxsbfd = 0;        /*no of fds in use */
#ifdef __MSW32__
                                 //event thread id
EXTERN DWORD event_id = (DWORD) - 1;
#endif

/**********************************************************************
 * start_sbdaemon
 *
 * Creates the shared memory segment and starts up the sbdaemon telling
 * it info about the segment.  This only needs to be called once.
 * It is called automatically from create_window at the first creation attempt.
 **********************************************************************/

void start_sbdaemon() {  /*start the daemon */
  #if defined (__UNIX__) || defined(__MSW32__)
  char *shmaddr;                 /*address to attach */
  char *sbenv;                   /*SB_DISPLAY_ADDR */
  INT32 sbaddr;                  /*starbase address */
  INT32 wmshm;                   //windows shared mem
  char arg1[MAX_PATH];           /*shmid argument */
  char arg2[MAX_PATH];           /*shmstart argument */
  char arg3[MAX_PATH];           /*shmsize argument */
  const char *argv[5];           /*args of sbdaemon */
                                 /*for pipe usage */
  static char pipebuffer[PIPESIZE];

  sbenv = getenv (SBADDR);       /*get SB_DISPLAY_ADDR */
  if (sbenv == NULL || sscanf (sbenv, INT32FORMAT, &sbaddr) != 1)
    sbaddr = SBDEFAULT;
  shmaddr = getenv (WMSHM);
  if (shmaddr == NULL || sscanf (shmaddr, INT32FORMAT, &wmshm) != 1)
    wmshm = WMSHMDEFAULT;
                                 /*default address */
  shmaddr = (char *) sbaddr - (wmshm + SHMSIZE + SHMOFFSET);

  if (!remote_display (arg1)) {
    shminfo.shmsize = SHMSIZE;   /*size of segment */
    #ifdef __UNIX__
    shminfo.shmid = shmget (IPC_PRIVATE, SHMSIZE, USER_RW);
    /*get shm segment */
    //              if (shminfo.shmid==-1)
    //              NO_SHM_SEGMENT.error("start_sbdaemon",ABORT,"Errno=%d",errno);
    #ifdef hp9000s800
                                 /*attach it */
    shminfo.shmstart = shmat (shminfo.shmid, 0, 0);
    if ((int) shminfo.shmstart == -1)
    #else
                                 /*attach it */
      shminfo.shmstart = shmat (shminfo.shmid, shmaddr, 0);
    //              if (shminfo.shmstart!=shmaddr)
    #endif
    //                      SHM_ATTACH_FAILED.error("start_sbdaemon",ABORT,"Errno=%d",errno);
    #else
    SECURITY_ATTRIBUTES security;//for handles

    security.nLength = sizeof (security);
    security.lpSecurityDescriptor = NULL;
                                 //make it inheritable
    security.bInheritHandle = TRUE;
                                 //anonymous
    shminfo.shmid = CreateFileMapping ((HANDLE) 0xffffffff, &security, PAGE_READWRITE, 0, shminfo.shmsize + 3 * sizeof (INT32) + EVENTSIZE * sizeof (SBD_GRAPHICS_EVENT), NULL);
    if (shminfo.shmid == NULL) {
      shminfo.shmstart = NULL;
      return;                    //quietly fail
    }
    shminfo.shmstart =
      MapViewOfFile (shminfo.shmid, FILE_MAP_WRITE, 0, 0, 0);
    if (shminfo.shmstart == NULL)
      return;
    EVENT_TAIL = 0;
    EVENT_HEAD = 0;
    #endif
                                 /*set up args */
    sprintf (arg1, "%d", shminfo.shmid);
    sprintf (arg2, "%p", shminfo.shmstart);
    sprintf (arg3, INT32FORMAT, shminfo.shmsize);
    argv[0] = SBDAEMON;          /*set up argv */
    argv[1] = arg1;
    argv[2] = arg2;
    argv[3] = arg3;
    argv[4] = NULL;
  }
  else {
    shmaddr = NULL;              //remote
    fprintf (stderr, "start_sbdaemon:using %s to connect to machine %s\n",
      REMSH, arg1);
    #ifdef __UNIX__
    shminfo.shmid = -1;
    #else
    shminfo.shmid = NULL;
    #endif
                                 /*not using shm */
    shminfo.shmstart = pipebuffer;
    shminfo.shmsize = PIPESIZE;  /*size of pipe buffer */
    #ifdef __UNIX__
                                 /*command on host */
    sprintf (arg2, "%s=0x%x; export %s; %s -1 0 " INT32FORMAT " %s", SBADDR, sbaddr, SBADDR, SBDAEMON, shminfo.shmsize, getenv (DISP));
    #else
                                 /*command on host */
    sprintf (arg2, "%s -1 0 %d %s", SBDAEMON, shminfo.shmsize, getenv (DISP));
    #endif
    argv[0] = REMSH;             /*set up argv */
    argv[1] = arg1;              /*host to use */
    argv[2] = arg2;
    argv[3] = NULL;
  }

  shminfo.usedsize = 0;          /*none used yet */

  #ifdef __UNIX__
  //   shminfo.pid=two_way_pipe(argv[0],argv,shminfo.fds);       /*start daemon*/
  #else
  if (two_way_pipe (argv[0], argv, shminfo.fds) != 0) {
    cleanup_sbdaemon();
  }
  else {
                                 //anonymous
    event_sem = CreateSemaphore (NULL, 1, 1, NULL);
    //xiaofan
    _beginthread (event_reader, 0, &shminfo.fds[INFD]);
  }
  #endif
  #endif
}


/**********************************************************************
 * cleanup_sbdaemon
 *
 * Free system resources for when the daemon has failed or been killed.
 **********************************************************************/
void cleanup_sbdaemon() {  /*forget about the daemon */
  #ifdef __MSW32__
  if (shminfo.fds[INFD] != NULL) {
    CloseHandle (shminfo.fds[INFD]);
    shminfo.fds[INFD] = 0;
  }
  if (shminfo.fds[OUTFD] != NULL) {
    CloseHandle (shminfo.fds[OUTFD]);
    shminfo.fds[OUTFD] = 0;
  }
  if (shminfo.shmstart != NULL) {
    UnmapViewOfFile (shminfo.shmstart);
    shminfo.shmstart = NULL;
  }
  if (shminfo.shmid != NULL) {
    CloseHandle (shminfo.shmid);
    shminfo.shmid = NULL;
  }
  if (event_sem != NULL) {
    CloseHandle(event_sem);
    event_sem = NULL;
  }
  #elif defined(__UNIX__)
  if (shminfo.fds[INFD] > 0) {
    close (shminfo.fds[INFD]);
    shminfo.fds[INFD] = 0;
  }
  if (shminfo.fds[OUTFD] > 0) {
    close (shminfo.fds[OUTFD]);
    shminfo.fds[OUTFD] = 0;
  }
  shminfo.shmstart = NULL;
  #endif
}


/**********************************************************************
 * remote_display
 *
 * Returns TRUE if the DISPLAY environment variable points to a
 * Remote display, and sets the name to the name of the host.
 * Otherwise, returns FALSE.
 **********************************************************************/

BOOL8 remote_display(            //check for remote
                     char *name  //name of host
                    ) {
  #if defined (__UNIX__) || defined(__MSW32__)
  char *xenv;                    /*DISPLAY environ */
  char *nameend;                 //end of name
  #ifdef __UNIX__
  char thishost[MAX_PATH];       //current host
  #endif

  xenv = getenv (DISP);          /*get display variable */
  if (xenv != NULL) {
    strcpy(name, xenv);
    nameend = strchr (name, ':');
    if (nameend != NULL)
      *nameend = '\0';           /*chop display off */
    nameend = strchr (name, '.');
    if (nameend != NULL)
      *nameend = '\0';           /*chop resolv off */
    #ifdef __UNIX__
    if (strcmp (name, LOCAL1) && strcmp (name, LOCAL2)
    && gethostname (thishost, MAX_PATH) >= 0) {
      nameend = strchr (thishost, '.');
      if (nameend != NULL)
        *nameend = '\0';         /*chop resolv off */
      if (strcmp (name, thishost)) {
        return TRUE;
      }
    }
    #else
    return TRUE;
    #endif
  }
  #endif
  return FALSE;
}


/**********************************************************************
 * getshm
 *
 * Get the next element of the shared memory.  If there is no more room
 * in the segment, kick the daemon to get it to empty it out and then
 * restart the buffer once it acknowledges the cleanout.
 **********************************************************************/

DLLSYM void *getshm(            /*get memory */
                    INT32 size  /*required size */
                   ) {
  void *segment;                 /*return segment */

  if (shminfo.shmstart == NULL)
    return NULL;                 //no daemon connection
  size = (size + 3) & ~3;
  if (size > shminfo.shmsize)
    return NULL;
                                 /*too full? */
  if (shminfo.usedsize + size > shminfo.shmsize
  || shminfo.usedsize < 0) {     /*or read pending */
    kick_daemon(AWAIT_BUFFER);  /*get it to read */
  }
                                 /*address of segment */
  segment = (char *) shminfo.shmstart + shminfo.usedsize;
  shminfo.usedsize += size;      /*sum used sizes */
  return segment;
}


/**********************************************************************
 * kick_daemon
 *
 * Tell the daemon to read the shared memory and perform all the
 * operations in it.  This function blocks until the daemon has
 * emptied the queue.
 **********************************************************************/

void kick_daemon(           /*empty queue */
                 INT8 mode  /*control mode */
                ) {
  #ifndef __MAC__
  SBD_GRAPHICS_EVENT event;      /*event from daemon */
  GRAPHICS_EVENT real_event;     //converted format
  #ifdef __MSW32__
  unsigned long nwrite;
  unsigned long nread;           //bytes read
  char pipe_char[2];             //char from pipe
  INT32 pipe_index;              //index to event queue
  #endif
  static INT16 reads_pending = 0;/*acknowledges pending */

  if (mode == COUNT_READS) {
    lock_events();
    reads_pending--;             /*got a read */
    unlock_events();
    return;
  }
  if (shminfo.shmstart == NULL)
    return;                      //no connection
  if (shminfo.usedsize > 0) {
    #ifdef __UNIX__
    if (write
      (shminfo.fds[OUTFD], (const char *) &shminfo.usedsize,
      sizeof (INT32)) != sizeof (INT32))
      WRITEFAILED.error ("kick_daemon", EXIT, "sbdaemon pipe");
    #else
    PRIMITIVES = shminfo.usedsize;
    if (WriteFile (shminfo.fds[OUTFD], "xx", 2, &nwrite, NULL) == 0
    || nwrite != 2) {
      cleanup_sbdaemon();
      return;
    }
    #endif
    #ifdef __UNIX__
    if (shminfo.shmid < 0) {
      if (write (shminfo.fds[OUTFD], (const char *) shminfo.shmstart,
        shminfo.usedsize) != shminfo.usedsize)
        WRITEFAILED.error ("kick_daemon", EXIT, "sbdaemon pipe");
      #else
      if (shminfo.shmid == NULL) {
        if (WriteFile (shminfo.fds[OUTFD], (const char *) shminfo.shmstart,
          shminfo.usedsize, &nwrite, NULL) == 0
        || nwrite != (UINT32) shminfo.usedsize) {
          cleanup_sbdaemon();
          return;
        }
        #endif
        shminfo.usedsize = 0;    /*can use it now */
      }
      else
        shminfo.usedsize = -1;   /*need to wait */
      lock_events();
      reads_pending++;           /*acknowledges due */
      unlock_events();
    }
    if (mode == FLUSH_IN || reads_pending > MAX_PENDING || mode == AWAIT_BUFFER
      #ifdef __UNIX__
      && shminfo.shmid < 0)
    #else
      && shminfo.shmid != NULL)
        #endif
    {
      while (reads_pending > 0) {
        #ifdef __MSW32__
        if (event_id == GetCurrentThreadId ()) {
          if (ReadFile (shminfo.fds[INFD], pipe_char, 2, &nread, NULL) != 0
          && nread == 2) {
            pipe_index = EVENT_HEAD;
              event = EVENT_INDEX (pipe_index);
              pipe_index++;
              if (pipe_index >= EVENTSIZE)
                pipe_index = 0;
                EVENT_HEAD = pipe_index;
                #endif
                #ifdef __UNIX__
                if (read
                  (shminfo.fds[INFD], &event,
                  sizeof (SBD_GRAPHICS_EVENT)) !=
                  sizeof (SBD_GRAPHICS_EVENT))
                  READFAILED.error ("kick_daemon", EXIT, "sbdaemon pipe");
                  #endif
            if (event.type != QUEUE_CLEAR) {
              real_event.fildes = event.fd;
                real_event.type = event.type;
                real_event.key = event.key;
                real_event.x = event.x;
                real_event.y = event.y;
                real_event.next = NULL;
                                 /*add event to queue */
                add_event(&real_event);
            }
            else
              reads_pending--;   /*got acknowledge */
              #ifdef __MSW32__
          }
        }
        else
          Sleep (50);
            #endif
      }
      if (shminfo.usedsize < 0)  //must be reentrant
          shminfo.usedsize = 0;  /*none used now */
    }
    #endif
  }

  #ifdef __MSW32__
  /**********************************************************************
   * two_way_pipe
   *
   * Open the process and connect a 2 way pipe to its stdin and stdout.
   **********************************************************************/

  int
    two_way_pipe (               //do one file
    const char *file,            //program to run
    const char *argv[],          //args to execvp
    HANDLE fds[]                 //output fds
  ) {
    int argind;                  //argument index
      HANDLE infds[2];           //input fds
      HANDLE outfds[2];          //output fds
      HANDLE sends[2];           //fds for child
      HANDLE process;            //current process
      STARTUPINFO start_info;    //start information
                                 //process info
      PROCESS_INFORMATION proc_info;
      char cmd[MAX_PATH * 2];    //final command line

      if (CreatePipe (&infds[0], &infds[1], NULL, PIPESIZE) == 0
        || CreatePipe (&outfds[0], &outfds[1], NULL, PIPESIZE) == 0)
        return -1;
    /*	if (_pipe(infds,PIPESIZE,_O_BINARY)<0
      || _pipe(outfds,PIPESIZE,_O_BINARY)<0)
        return -1;	*/
        process = GetCurrentProcess ();
        if (DuplicateHandle (process, outfds[0],
          process, &sends[0], GENERIC_READ, TRUE,
          DUPLICATE_CLOSE_SOURCE) == 0)
          return -1;
          if (DuplicateHandle (process, infds[1],
            process, &sends[1], GENERIC_WRITE, TRUE,
            DUPLICATE_CLOSE_SOURCE) == 0)
            return -1;

            cmd[0] = '\0';
    for (argind = 0; argv[argind] != NULL; argind++) {
      if (argind != 0)
          strcat (cmd, " ");
          strcat (cmd, argv[argind]);
    }

    GetStartupInfo(&start_info);
      start_info.wShowWindow = FALSE;
      start_info.hStdInput = sends[0];
      start_info.hStdOutput = sends[1];
      start_info.dwFlags = STARTF_USESTDHANDLES;
      if (!CreateProcess (NULL, (char *) cmd, NULL, NULL, TRUE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED, NULL, NULL,
        &start_info, &proc_info))
        return -1;
        CloseHandle (sends[0]);
        CloseHandle (sends[1]);
        CloseHandle (proc_info.hProcess);
        ResumeThread (proc_info.hThread);
        CloseHandle (proc_info.hThread);
        fds[INFD] = infds[0];
        fds[OUTFD] = outfds[1];
        return 0;
  }
  #endif
