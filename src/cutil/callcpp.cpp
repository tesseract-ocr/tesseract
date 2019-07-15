/**********************************************************************
 * File:        callcpp.cpp
 * Description: extern C interface calling C++ from C.
 * Author:      Ray Smith
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "callcpp.h"
#include <cstdarg>      // for va_end, va_list, va_start
#include <cstdio>       // for vsprintf
#include <memory>       // for unique_ptr
#include "scrollview.h" // for ScrollView, SVEvent, SVET_ANY, SVET_INPUT
#include "tprintf.h"    // for tprintf

void
cprintf (                        //Trace printf
const char *format, ...          //special message
) {
  va_list args;                  //variable args
  char msg[1000];

  va_start(args, format);  //variable list
  vsprintf(msg, format, args);  //Format into msg
  va_end(args);

  tprintf("%s", msg);
}


#ifndef GRAPHICS_DISABLED
ScrollView *c_create_window(                   /*create a window */
                      const char *name,  /*name/title of window */
                      int16_t xpos,        /*coords of window */
                      int16_t ypos,        /*coords of window */
                      int16_t xsize,       /*size of window */
                      int16_t ysize,       /*size of window */
                      double xmin,       /*scrolling limits */
                      double xmax,       /*to stop users */
                      double ymin,       /*getting lost in */
                      double ymax        /*empty space */
                     ) {
   return new ScrollView(name, xpos, ypos, xsize, ysize, xmax - xmin, ymax - ymin, true);
}


void c_line_color_index(  /*set color */
                        void *win,
                        C_COL index) {
 // The colors are the same as the SV ones except that SV has COLOR:NONE --> offset of 1
 auto* window = static_cast<ScrollView*>(win);
 window->Pen(static_cast<ScrollView::Color>(index + 1));
}


void c_move(  /*move pen */
            void *win,
            double x,
            double y) {
  auto* window = static_cast<ScrollView*>(win);
  window->SetCursor(static_cast<int>(x), static_cast<int>(y));
}


void c_draw(  /*move pen */
            void *win,
            double x,
            double y) {
  auto* window = static_cast<ScrollView*>(win);
  window->DrawTo(static_cast<int>(x), static_cast<int>(y));
}


void c_make_current(  /*move pen */
                    void *win) {
  auto* window = static_cast<ScrollView*>(win);
  window->Update();
}


void c_clear_window(  /*move pen */
                    void *win) {
  auto* window = static_cast<ScrollView*>(win);
  window->Clear();
}


char window_wait(ScrollView* win) {
  // Wait till an input or click event (all others are thrown away)
  char ret = '\0';
  SVEventType ev_type = SVET_ANY;
  do {
    std::unique_ptr<SVEvent> ev(win->AwaitEvent(SVET_ANY));
    ev_type = ev->type;
    if (ev_type == SVET_INPUT)
      ret = ev->parameter[0];
  } while (ev_type != SVET_INPUT && ev_type != SVET_CLICK);
  return ret;
}
#endif
