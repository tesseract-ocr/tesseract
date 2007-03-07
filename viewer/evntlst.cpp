/**********************************************************************
 * File:        evntlst.c  (Formerly eventlst.c)
 * Description: Code to manipulate lists of events.
 * Author:		Ray Smith
 * Created:		Fri Nov 01 11:02:52 GMT 1991
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

#include          "mfcpch.h"
#include          "grphshm.h"
#include          "grphics.h"
#ifdef __UNIX__
#include          <unistd.h>
#include          <signal.h>
//#include                                      "pipes.h"
#include          "fileerr.h"
//#include                                      "sbderrs.h"
//#include                                      "grpherr.h"
#elif defined (__MSW32__)
#include          <io.h>
#include          <process.h>
#else
#include          <unistd.h>
#endif
#include          "evntlst.h"

#define EXTERN
                                 //anything new in queue
EXTERN BOOL8 event_waiting = FALSE;

#ifdef __UNIX__
                                 //semaphore count
static INT32 event_critical_section = 0;
                                 //true if pending
static BOOL8 event_pending = FALSE;
EXTERN BOOL8 handler_set;        //true if signal handler on
EXTERN STRING_VAR (events_logfile, "", "File to log events to");
EXTERN STRING_VAR (events_playback, "", "File to read events from");

/**********************************************************************
 * event_handler
 *
 * Handler for signal.
 * If not critical, read events now, otherwise, set pending flag.
 **********************************************************************/

void event_handler(                        //signal handler
                   int,                    //signal
                   int,                    //code
                   struct sigcontext *scp  //info for sigvector
                  ) {
  //      scp->sc_syscall_action=SIG_RESTART;                                     //restart sys calls
  event_pending = TRUE;
  if (!event_critical_section) {
    lock_events(); 
    unlock_events(); 
  }
}


/**********************************************************************
 * check_event
 *
 * See if any events are available, and if there are, return 1, else 0.
 **********************************************************************/

BOOL8 check_event(            /*test for event */
                  INT16 fd,   /*window to wait on */
                  BOOL8 wait  /*set if waiting */
                 ) {
  GRAPHICS_EVENT event;          /*event from daemon */
  SBD_GRAPHICS_EVENT sbd_event;  //event from pipe
  BOOL8 gotone;                  /*repeat while do */
  INT32 time;                    /*time to select */
  INT32 typein;                  //input type
  INT32 keyin;                   //input key
  INT32 fdin;                    //input fd
  int scanresult;                //of fscanf
  static FILE *eventsin = NULL;  //input file

                                 // && sbfds[fd].used!=1)
  if (fd < 0 || fd > maxsbfd || fd > 0) {
    //     BADSBFD.error("check_event",LOG,NULL);                  /*report error*/
    return FALSE;
  }
  time = wait ? AWAIT_TIME : CHECK_TIME;
  gotone = FALSE;
  while (!gotone
  && (events_playback.string ()[0] != '\0' || eventsin != NULL)) {
    if (eventsin == NULL && events_playback.string ()[0] != '\0') {
      if ((eventsin = fopen (events_playback.string (), "r")) == NULL)
        CANTOPENFILE.error ("check_event", LOG,
          events_playback.string ());
                                 //remove name
      events_playback.set_value (NULL);
    }
    if (eventsin != NULL) {
      scanresult =
        fscanf (eventsin,
        INT32FORMAT " " INT32FORMAT "(%f,%f) on " INT32FORMAT,
        &typein, &keyin, &event.x, &event.y, &fdin);
      if (scanresult != 5) {
        if (scanresult == EOF)
          fprintf (stderr, "Closing input event file\n");
        else
          fprintf (stderr, "Error on event input: copied %d values\n",
            scanresult);
        fclose(eventsin); 
        eventsin = NULL;
      }
      else {
        //                              fprintf(stderr,"Read input on %d\n", fdin );
        event.type = (INT8) typein;
        event.key = (char) keyin;
        event.fildes = fdin;
        add_event(&event);  //got one
        gotone = gotone || fd == 0 || fd == event.fildes;
        /*know if we got one */
      }
    }
  }
  while (!gotone) {              //     && select_read(shminfo.fds[INFD],time)!=0)                      /*event available*/
    if (read (shminfo.fds[INFD], &sbd_event, sizeof (SBD_GRAPHICS_EVENT))
      != sizeof (SBD_GRAPHICS_EVENT))
      READFAILED.error ("check_event", EXIT, "sbdaemon pipe");
    if (sbd_event.type != QUEUE_CLEAR) {
      event.fildes = sbd_event.fd;
      event.type = sbd_event.type;
      event.key = sbd_event.key;
      event.x = sbd_event.x;
      event.y = sbd_event.y;
      event.next = NULL;
      add_event(&event);  /*add event to queue */
                                 /*know if we got one */
      gotone = gotone || fd == 0 || fd == event.fildes;
    }
    else
      kick_daemon(COUNT_READS);  /*keep count accurate */
    time = CHECK_TIME;
  }
  return gotone;
}


#else
EXTERN HANDLE event_sem = NULL;  //event lock

/**********************************************************************
 * event_reader
 *
 * A separate thread that reads the input from the pipe and places
 * events in the appropriate queue. This thread also calls any appropriate
 * event handler on receiving an event.
 **********************************************************************/

void event_reader(             /*read events */
                  void *param  /*file descriptor */
                 ) {
  #ifndef __MAC__
                                 //real file descriptor
  HANDLE *fdptr = (HANDLE *) param;
  HANDLE fd = *fdptr;            //real file descriptor
  unsigned long nread;           //bytes read
  SBD_GRAPHICS_EVENT event;      /*event from daemon */
  GRAPHICS_EVENT real_event;     //converted format
  char pipe_char[2];             //from pipe
  INT32 pipe_index;              //index to event queue

  event_id = GetCurrentThreadId ();
  while (ReadFile (fd, pipe_char, 2, &nread, NULL) != 0 && nread == 2) {
    pipe_index = EVENT_HEAD;
    event = EVENT_INDEX (pipe_index);
    pipe_index++;
    if (pipe_index >= EVENTSIZE)
      pipe_index = 0;
    EVENT_HEAD = pipe_index;
    if (event.type != QUEUE_CLEAR) {
      real_event.fildes = event.fd;
      real_event.type = event.type;
      real_event.key = event.key;
      real_event.x = event.x;
      real_event.y = event.y;
      real_event.next = NULL;
      add_event(&real_event);  /*add event to queue */
    }
    else
      kick_daemon(COUNT_READS);  /*got acknowledge */
  }
  CloseHandle(fd); 
  *fdptr = NULL;
  _endthread(); 
  #endif
}
#endif

/**********************************************************************
 * add_event
 *
 * Adds an event from the sbdaemon to the correct event queue.
 **********************************************************************/

void add_event(                       /*add an event */
               GRAPHICS_EVENT *event  /*event to add */
              ) {
  GRAPHICS_EVENT *newevent;      /*new event */
  EVENT_HANDLER handler;         //avoid race hazards
  GRAPHICS_EVENT sel_event;      //selection
                                 //last button down
  static GRAPHICS_EVENT last_down;

  #ifdef __UNIX__
  static FILE *eventsout = NULL; //output file
  static STRING outname;         //output name

                                 //doing anything
  if (events_logfile.string ()[0] != '\0' || eventsout != NULL) {
    if (eventsout != NULL        //already open
    && outname != (STRING &) events_logfile) {
      fclose(eventsout);  //finished that one
      eventsout = NULL;
    }
                                 //needs opening
    if (eventsout == NULL && events_logfile.string ()[0] != '\0') {
                                 //save name
      outname = events_logfile.string ();
      if ((eventsout = fopen (outname.string (), "w")) == NULL)
        CANTOPENFILE.error ("add_event", LOG, outname.string ());
    }
    if (eventsout != NULL)
      fprintf (eventsout, "%d %d(%f,%f) on %d\n",
        event->type, event->key, event->x, event->y, event->fildes);
  }
  #endif
  //      fprintf(stderr,"Received event of type %d at (%f,%f) on %d\n",
  //              event->type,event->x,event->y,event->fildes);
  event->fd = &sbfds[event->fildes];
  switch (event->type) {
    case DOWN_EVENT:
      last_down = *event;        //save it
      handler = sbfds[event->fildes].click_handler;
      if (handler != NULL) {
        (*handler) (event);
        return;                  //done it
      }
      break;
    case UP_EVENT:
      sel_event = *event;
      if (last_down.x > event->x)
        sel_event.xmax = last_down.x;
      else {
        sel_event.xmax = event->x;
        sel_event.x = last_down.x;
      }
      if (last_down.y > event->y)
        sel_event.ymax = last_down.y;
      else {
        sel_event.ymax = event->y;
        sel_event.y = last_down.y;
      }
      handler = sbfds[event->fildes].selection_handler;
      if (handler != NULL) {
        (*handler) (&sel_event);
        return;                  //done it
      }
      break;
    case KEYPRESS_EVENT:
      handler = sbfds[event->fildes].key_handler;
      if (handler != NULL) {
        (*handler) (event);
        return;                  //done it
      }
      break;
    case DESTROY_EVENT:
      handler = sbfds[event->fildes].destroy_handler;
      if (handler != NULL) {
        (*handler) (event);
        //                      return;                                                                         //done it
      }
      break;
  }
  lock_events(); 
  newevent = new GRAPHICS_EVENT;
  if (newevent != NULL) {
    *newevent = *event;          /*copy event */
                                 /*first event */
    if (sbfds[event->fildes].events == NULL)
      sbfds[event->fildes].events = newevent;
    else
                                 /*add to end */
      sbfds[event->fildes].lastevent->next = newevent;
                                 /*is new end */
    sbfds[event->fildes].lastevent = newevent;
    newevent->next = NULL;
  }
  event_waiting = TRUE;          //added to queue
  unlock_events(); 
}


/**********************************************************************
 * lock_events
 *
 * Lock out event handler to protect data structures.
 **********************************************************************/

void lock_events() {  //lock
  #ifdef __UNIX__
  event_critical_section++;
  #elif defined (__MSW32__)
  WaitForSingleObject (event_sem, (unsigned long) -1);
  #endif
}


/**********************************************************************
 * unlock_events
 *
 * Unlock. If event pending deal with it.
 **********************************************************************/

void unlock_events() {  //lock
  #ifdef __UNIX__
  if (event_pending && event_critical_section == 1) {
                                 //get all events
    while (check_event (0, FALSE));
    event_pending = FALSE;
  }
  event_critical_section--;
  #elif defined (__MSW32__)
                                 //release it
  ReleaseSemaphore (event_sem, 1, NULL);
  #endif
}
