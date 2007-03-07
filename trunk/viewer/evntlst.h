/**********************************************************************
 * File:        evntlst.h  (Formerly eventlst.h)
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

#ifndef           EVNTLST_H
#define           EVNTLST_H

#include          "sbgtypes.h"
extern BOOL8 event_waiting;      //anything new in queue
#ifdef __UNIX__
#include          "varable.h"

extern BOOL8 handler_set;        //true if signal handler on
extern STRING_VAR_H (events_logfile, "", "File to log events to");
extern STRING_VAR_H (events_playback, "", "File to read events from");
void event_handler(                        //signal handler
                   int,                    //signal
                   int,                    //code
                   struct sigcontext *scp  //info for sigvector
                  );
BOOL8 check_event(            /*test for event */
                  INT16 fd,   /*window to wait on */
                  BOOL8 wait  /*set if waiting */
                 );
#else                            /*  */
extern HANDLE event_sem;         //event lock
void event_reader(             /*read events */
                  void *param  /*file descriptor */
                 );
#endif
void add_event(                       /*add an event */
               GRAPHICS_EVENT *event  /*event to add */
              );
void lock_events();  //lock
void unlock_events();  //lock
#endif
