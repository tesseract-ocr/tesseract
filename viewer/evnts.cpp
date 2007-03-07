/**********************************************************************
 * File:        evnts.c  (Formerly events.c)
 * Description: Additional functions needed to receive graphics events.
 * Author:      Ray Smith
 * Created:     Thu May 24 14:13:00 BST 1990
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
#ifdef __UNIX__
#include          "sbderrs.h"
#include          "fileerr.h"
#include          "grpherr.h"
#endif
#include          "grphshm.h"
#include          "evntlst.h"
#include          "evnts.h"

#define EXTERN
DLLSYM EVENT_HANDLER win_selection_handler;
WINDOW (*await_event_func) (WINDOW, BOOL8, INT8, GRAPHICS_EVENT *) =
def_await_event;

//local non-public functions
GRAPHICS_EVENT *find_event(                 /*wait for event */
                           INT16 &fd,       /*window to wait on */
                           BOOL8 wait,      /*waiting flag */
                           INT8 event_type  /*type to look for */
                          );
                                 /*search for event */
GRAPHICS_EVENT *search_event_queue(INT16 &fd,       /*queue to search */
                                   INT8 event_type  /*type to search for */
                                  );
                                 /*search for event */
GRAPHICS_EVENT *search_single_queue(INT16 fd,        /*queue to search */
                                    INT8 event_type  /*type to search for */
                                   );

/**********************************************************************
 * await_selection
 *
 * Wait (or check) for a selection on the given (or all) fd and return it.
 **********************************************************************/

DLLSYM WINDOW await_selection(              /*wait for selection */
                              WINDOW win,   /*window to wait on */
                              BOOL8 wait,   /*waiting flag */
                              float &xmin,  /*coords of selection */
                              float &ymin,  /*coords of selection */
                              float &xmax,  /*coords of selection */
                              float &ymax   /*coords of selection */
                             ) {
  GRAPHICS_EVENT event;          /*return event */

  win = await_event (win, wait, SELECT_EVENT, &event);
  if (event.type == DESTROY_EVENT)
    return NULL;                 //was destroyed
  if (win != NULL) {
    xmin = event.x;              /*get coords */
    ymin = event.y;
    xmax = event.xmax;           /*get coords */
    ymax = event.ymax;
  }
  return win;
}


/**********************************************************************
 * await_click
 *
 * Wait (or check) for a click on the given (or all) fd and return it.
 **********************************************************************/

DLLSYM WINDOW await_click(             /*wait for click */
                          WINDOW win,  /*window to wait on */
                          BOOL8 wait,  /*waiting flag */
                          float &x,    /*coords of click */
                          float &y     /*coords of click */
                         ) {
  GRAPHICS_EVENT event;          /*return event */

  win = await_event (win, wait, DOWN_EVENT, &event);
  if (event.type == DESTROY_EVENT)
    return NULL;                 //was destroyed
  if (win != NULL) {
    x = event.x;                 /*get coords */
    y = event.y;
  }
  return win;
}


/**********************************************************************
 * await_key
 *
 * Wait (or check) for a key on the given (or all) fd and return it.
 **********************************************************************/

DLLSYM WINDOW await_key(             /*wait for key */
                        WINDOW win,  /*window to wait on */
                        BOOL8 wait,  /*waiting flag */
                        char &c      /*return character */
                       ) {
  GRAPHICS_EVENT event;          /*return event */

  win = await_event (win, wait, KEYPRESS_EVENT, &event);
  if (event.type == DESTROY_EVENT)
    return NULL;                 //was destroyed
  if (win != NULL)
    c = event.key;               /*get keypress */
  return win;
}


/**********************************************************************
 * await_event
 *
 * Wait (or check) for a event on the given (or all) fd and return it.
 **********************************************************************/

DLLSYM WINDOW def_await_event(                           /*wait for event */
                              WINDOW win,                /*window to wait on */
                              BOOL8 wait,                /*waiting flag */
                              INT8 event_type,           /*type to wait for */
                              GRAPHICS_EVENT *out_event  /*output event */
                             ) {
  GRAPHICS_EVENT *event;         /*return event */
  INT16 fd;                      //file descriptor

  if (win == NULL)
    fd = 0;
  else
    fd = win->get_fd ();
                                 /*look for one */
  event = find_event (fd, wait, event_type);
  if (event == NULL)
    return NULL;                 /*not found */
  else {
    *out_event = *event;         /*copy event */
    if (event->type != DESTROY_EVENT)
      delete event;              //free the element
    return out_event->fd;
  }
}


/**********************************************************************
 * find_event
 *
 * Search the queue for an event of a given type, and return it.
 * Read or wait until one turns up if there is not one already.
 **********************************************************************/

GRAPHICS_EVENT *find_event(                 /*wait for event */
                           INT16 &fd,       /*window to wait on */
                           BOOL8 wait,      /*waiting flag */
                           INT8 event_type  /*type to look for */
                          ) {
  GRAPHICS_EVENT *event;         /*return event */

                                 /*look for one */
  event = search_event_queue (fd, event_type);
  if (event == NULL) {
    do {
      #ifdef __UNIX__
      if (check_event (fd, wait))
      #elif defined (__MSW32__)
        if (wait)
          Sleep (50);
      if (event_waiting)
      #endif
      {
        //                              fprintf(stderr,"Got an event:searching queue %d\n",fd);
                                 /*try after reading */
        event = search_event_queue (fd, event_type);
      }
    }
    while (wait && event == NULL);
  }
  //      if (event!=NULL)
  //              event->fd=&sbfds[fd];
  return event;                  /*event located */
}


/**********************************************************************
 * search_event_queue
 *
 * Search the event queue(s) for events of a particular type.
 * If found, it is removed from the queue and returned.
 **********************************************************************/

GRAPHICS_EVENT *search_event_queue(                 /*search for event */
                                   INT16 &fd,       /*queue to search */
                                   INT8 event_type  /*type to search for */
                                  ) {
  GRAPHICS_EVENT *event;         /*event from daemon */
  INT16 testfd;                  /*test window */

  if (fd < 0 || fd > maxsbfd || fd > 0 && sbfds[fd].used != 1) {
    return NULL;
  }
  if (fd > 0)
                                 /*just one to search */
    return search_single_queue (fd, event_type);
  else {
    for (testfd = 1; testfd < maxsbfd; testfd++) {
      if (sbfds[testfd].used) {
        event = search_single_queue (testfd, event_type);
        if (event != NULL) {
          fd = testfd;           /*successful window */
          return event;          /*got one */
        }
      }
    }
  }
  return NULL;                   /*found nothing */
}


/**********************************************************************
 * search_single_queue
 *
 * Search the event queue for events of a particular type.
 * If found, it is removed from the queue and returned.
 **********************************************************************/

GRAPHICS_EVENT *search_single_queue(                 /*search for event */
                                    INT16 fd,        /*queue to search */
                                    INT8 event_type  /*type to search for */
                                   ) {
  GRAPHICS_EVENT *event;         /*event from daemon */
  GRAPHICS_EVENT *prev;          /*previous event */
  GRAPHICS_EVENT *event2;        /*2nd event */
  GRAPHICS_EVENT *prev2;         /*2nd previous */
  GRAPHICS_EVENT *nextevent;     /*next event in list */
  BOOL8 any_destroy = FALSE;

  lock_events(); 
  event_waiting = FALSE;         //done a scan
  prev = NULL;                   /*previous event */
  event2 = NULL;                 /*2nd event */
  if (event_type == ANY_EVENT) {
    event = sbfds[fd].events;    /*start of queue */
  }
  else if (event_type == SELECT_EVENT) {
    for (prev = NULL, event = sbfds[fd].events; event != NULL
    && event->type != DOWN_EVENT; event = nextevent) {
      //Delete up events that are in the list prior to a down event
      nextevent = event->next;   /*next in list */
      if (event->type == UP_EVENT) {
        if (prev == NULL)
                                 /*new head */
          sbfds[fd].events = nextevent;
        else
          prev->next = nextevent;/*delete event */
        if (nextevent == NULL)
                                 /*new last element */
          sbfds[fd].lastevent = prev;
        delete event;
      }
      else
        prev = event;
      if (event->type == DESTROY_EVENT)
        any_destroy = TRUE;
    }
    if (event == NULL) {
      unlock_events(); 
      if (any_destroy)
        return search_single_queue (fd, DESTROY_EVENT);
      return NULL;               /*no good */
    }
    for (prev2 = event, event2 = event->next; event2 != NULL
      && event2->type != UP_EVENT;
      prev2 = event2, event2 = event2->next);
    if (event2 == NULL) {
      unlock_events(); 
      return NULL;               /*no good */
    }
    if (prev2 != event) {        /*got some intervening */
      for (prev2 = event->next; prev2 != event2; prev2 = nextevent) {
        nextevent = prev2->next;
        delete prev2;
      }
    }
    event->next = event2->next;  /*cut out event2 */
    event2->next = NULL;         /*event is new end */

    event->xmax = event2->x;     /*get coords */
    event->ymax = event2->y;
    if (event->x > event->xmax) {
      event->xmax = event->x;
      event->x = event2->x;      /*get coords */
    }
    if (event->y > event->ymax) {
      event->ymax = event->y;
      event->y = event2->y;
    }
    delete event2;               //free the element
  }
  else {
    for (prev = NULL, event = sbfds[fd].events; event != NULL
      && event->type != DESTROY_EVENT
    && event->type != event_type; event = nextevent) {
      nextevent = event->next;   /*next in list */
                                 /*delete up in front of down */
      if (event->type == UP_EVENT && event_type == DOWN_EVENT) {
        if (prev == NULL)
                                 /*new head */
          sbfds[fd].events = nextevent;
        else
          prev->next = nextevent;/*delete event */
        if (nextevent == NULL)
                                 /*new last element */
          sbfds[fd].lastevent = prev;
        delete event;
      }
      else
        prev = event;            /*trailing ptr */
    }
  }
  if (event == NULL) {
    unlock_events(); 
    return NULL;                 /*no good */
  }
  if (event->type != DESTROY_EVENT) {
    if (prev == NULL)
                                 /*new head */
      sbfds[fd].events = event->next;
    else
      prev->next = event->next;  /*delete event */
    if (event->next == NULL)
      sbfds[fd].lastevent = prev;/*new last event */
    event->next = NULL;          /*possible 2nd event */
  }
  unlock_events(); 
  return event;                  /*got one!! */
}
