/**********************************************************************
 * File:        callcpp.cpp
 * Description: extern C interface calling C++ from C.
 * Author:		Ray Smith
 * Created:		Sun Feb 04 20:39:23 MST 1996
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

#include "mfcpch.h"
#include "errcode.h"
#ifdef __UNIX__
#include <assert.h>
#include <stdarg.h>
#endif
#include <time.h>
#include "memry.h"
#include "scrollview.h"
//#include          "evnts.h"
#include "varable.h"
#include "callcpp.h"
#include "tprintf.h"
//#include                                      "strace.h"
#include "host.h"
#include "unichar.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

//extern "C" {

INT_VAR (tess_cp_mapping0, 0, "Mappings for class pruner distance");
INT_VAR (tess_cp_mapping1, 1, "Mappings for class pruner distance");
INT_VAR (tess_cp_mapping2, 2, "Mappings for class pruner distance");
INT_VAR (tess_cp_mapping3, 3, "Mappings for class pruner distance");
INT_VAR (record_matcher_output, 0, "Record detailed matcher info");
INT_VAR (il1_adaption_test, 0, "Dont adapt to i/I at beginning of word");
double_VAR (permuter_pending_threshold, 0.0,
"Worst conf for using pending dictionary");
//Global matcher info from the class pruner.
inT32 cp_maps[4];
//Global info to control writes of matcher info
char blob_answer[UNICHAR_LEN + 1]; //correct char
char *word_answer;                 //correct word
inT32 bits_in_states;              //no of bits in states

void setup_cp_maps() {
  cp_maps[0] = tess_cp_mapping0;
  cp_maps[1] = tess_cp_mapping1;
  cp_maps[2] = tess_cp_mapping2;
  cp_maps[3] = tess_cp_mapping3;
}

void
cprintf (                        //Trace printf
const char *format, ...          //special message
) {
  va_list args;                  //variable args
  char msg[1000];

  va_start(args, format);  //variable list
  vsprintf(msg, format, args);  //Format into msg
  va_end(args);

  tprintf ("%s", msg);
}


#ifndef GRAPHICS_DISABLED
ScrollView *c_create_window(                   /*create a window */
                      const char *name,  /*name/title of window */
                      inT16 xpos,        /*coords of window */
                      inT16 ypos,        /*coords of window */
                      inT16 xsize,       /*size of window */
                      inT16 ysize,       /*size of window */
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
 ScrollView* window = (ScrollView*) win;
 window->Pen((ScrollView::Color) (index + 1));
}


void c_move(  /*move pen */
            void *win,
            double x,
            double y) {
  ScrollView* window = (ScrollView*) win;
  window->SetCursor((int) x, (int) y);
}


void c_draw(  /*move pen */
            void *win,
            double x,
            double y) {
  ScrollView* window = (ScrollView*) win;
  window->DrawTo((int) x, (int) y);
}


void c_make_current(  /*move pen */
                    void *win) {
  ScrollView* window = (ScrollView*) win;
  window->Update();
}


void c_clear_window(  /*move pen */
                    void *win) {
  ScrollView* window = (ScrollView*) win;
  window->Clear();
}


char window_wait(ScrollView* win) {
  SVEvent* ev;
  // Wait till an input or click event (all others are thrown away)
  char ret = '\0';
  SVEventType ev_type = SVET_ANY;
  do {
    ev = win->AwaitEvent(SVET_ANY);
    ev_type = ev->type;
    if (ev_type == SVET_INPUT)
      ret = ev->parameter[0];
    delete ev;
  } while (ev_type != SVET_INPUT && ev_type != SVET_CLICK);
  return ret;
}
#endif

void reverse32(void *ptr) {
  char tmp;
  char *cptr = (char *) ptr;

  tmp = *cptr;
  *cptr = *(cptr + 3);
  *(cptr + 3) = tmp;
  tmp = *(cptr + 1);
  *(cptr + 1) = *(cptr + 2);
  *(cptr + 2) = tmp;
}


void reverse16(void *ptr) {
  char tmp;
  char *cptr = (char *) ptr;

  tmp = *cptr;
  *cptr = *(cptr + 1);
  *(cptr + 1) = tmp;
}


//};
