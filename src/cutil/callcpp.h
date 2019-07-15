/**********************************************************************
 * File:        callcpp.h
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

#ifndef CALLCPP_H
#define CALLCPP_H

#include "params.h"
#include "unichar.h"

class ScrollView;

typedef enum {
  Black,
  White,
  Red,
  Yellow,
  Green,
  Cyan,
  Blue,
  Magenta,
  Aquamarine,
  Dark_SLATE_BLUE,
  Light_BLUE,
  Medium_BLUE,
  Midnight_BLUE,
  Navy_BLUE,
  Sky_BLUE,
  Slate_BLUE,
  Steel_BLUE,
  Coral,
  Brown,
  Sandy_BROWN,
  Gold,
  GoldENROD,
  Dark_GREEN,
  Dark_OLIVE_GREEN,
  Forest_GREEN,
  Lime_GREEN,
  Pale_GREEN,
  Yellow_GREEN,
  Light_GREY,
  Dark_SLATE_GREY,
  Dim_GREY,
  Grey,
  Khaki,
  Maroon,
  Orange,
  Orchid,
  Pink,
  Plum,
  Indian_RED,
  Orange_RED,
  Violet_RED,
  Salmon,
  Tan,
  Turqoise,
  Dark_TURQUOISE,
  Violet,
  Wheat,
  Green_YELLOW
} C_COL;                         /*starbase colours */

void cprintf (                   //Trace printf
const char *format, ...          //special message
);
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
                     );
void c_line_color_index(  /*set color */
                        void *win,
                        C_COL index);
void c_move(  /*move pen */
            void *win,
            double x,
            double y);
void c_draw(  /*move pen */
            void *win,
            double x,
            double y);
void c_make_current(  /*move pen */
                    void *win);
void c_clear_window(  /*move pen */
                    void *win);
char window_wait(ScrollView* win);

#endif
