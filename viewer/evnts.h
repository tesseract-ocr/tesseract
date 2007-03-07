/**********************************************************************
 * File:        evnts.h  (Formerly events.h)
 * Description: Header of functions and types needed for using events.
 * Author:      Ray Smith
 * Created:     Thu May 24 15:14:45 BST 1990
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

#ifndef           EVNTS_H
#define           EVNTS_H

#include          "grphics.h"

extern DLLSYM EVENT_HANDLER win_selection_handler;

DLLSYM WINDOW await_selection(              /*wait for selection */
                              WINDOW win,   /*window to wait on */
                              BOOL8 wait,   /*waiting flag */
                              float &xmin,  /*coords of selection */
                              float &ymin,  /*coords of selection */
                              float &xmax,  /*coords of selection */
                              float &ymax   /*coords of selection */
                             );
DLLSYM WINDOW await_click(             /*wait for click */
                          WINDOW win,  /*window to wait on */
                          BOOL8 wait,  /*waiting flag */
                          float &x,    /*coords of click */
                          float &y     /*coords of click */
                         );
DLLSYM WINDOW await_key(             /*wait for key */
                        WINDOW win,  /*window to wait on */
                        BOOL8 wait,  /*waiting flag */
                        char &c      /*return character */
                       );
DLLSYM WINDOW def_await_event(                           /*wait for event */
                              WINDOW win,                /*window to wait on */
                              BOOL8 wait,                /*waiting flag */
                              INT8 event_type,           /*type to wait for */
                              GRAPHICS_EVENT *out_event  /*output event */
                             );
#endif
