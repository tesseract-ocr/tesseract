/**********************************************************************
 * File:        sbgconst.h  (Formerly sbconst.h)
 * Description: Header file of constants needed by graphics code.
 * Author:      Ray Smith
 * Created:     Thu May 24 14:19:43 BST 1990
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

#ifndef           SBGCONST_H
#define           SBGCONST_H

//This file contains all the symbols and constants that the user sees
//that are not defined by Starbase.

#include          "host.h"

#define WINDOWNAMESIZE    13     /*max size of name */
#define MAXWINDOWS      64       /*max allowed windows */
#define MAXWINDOWNAME   1024     /*max name length */

#define NO_WINDOW     0          /*no legal window */
#define SMDWINDOW     0          /*use smd, no window */
#define SCROLLINGWIN    1        /*scrolling window */
#define FULLSIZEWIN     2        /*non-scrolling window */
#define DCWIN       3            /*dc drawing only */

#define M_DOT       0            /*marker_types */
#define M_PLUS        1          /*starbase markers */
#define M_ASTERISK      2
#define M_CIRCLE      3
#define M_CROSS       4
#define M_TRIANGLE      5
#define M_SQUARE      6
#define M_DIAMOND     7
#define M_CROSSED_SQUARE  8

class WINFD;
typedef WINFD *WINDOW;           //compatible with old code

typedef enum {
  BLACK,
  WHITE,
  RED,
  YELLOW,
  GREEN,
  CYAN,
  BLUE,
  MAGENTA,
  AQUAMARINE,
  DARK_SLATE_BLUE,
  LIGHT_BLUE,
  MEDIUM_BLUE,
  MIDNIGHT_BLUE,
  NAVY_BLUE,
  SKY_BLUE,
  SLATE_BLUE,
  STEEL_BLUE,
  CORAL,
  BROWN,
  SANDY_BROWN,
  GOLD,
  GOLDENROD,
  DARK_GREEN,
  DARK_OLIVE_GREEN,
  FOREST_GREEN,
  LIME_GREEN,
  PALE_GREEN,
  YELLOW_GREEN,
  LIGHT_GREY,
  DARK_SLATE_GREY,
  DIM_GREY,
  GREY,
  KHAKI,
  MAROON,
  ORANGE,
  ORCHID,
  PINK,
  PLUM,
  INDIAN_RED,
  ORANGE_RED,
  VIOLET_RED,
  SALMON,
  TAN,
  TURQUOISE,
  DARK_TURQUOISE,
  VIOLET,
  WHEAT,
  GREEN_YELLOW
} COLOUR;                        /*starbase colours */

enum ECHO_TYPE
{
  NO_ECHO,
  BEST_ECHO,
  CROSS_HAIR_ECHO,
  SMALL_X_ECHO,
  RUBBER_LINE_ECHO,
  RUBBER_BOX_ECHO,
  ALPHA_ECHO
};

/*Event types*/
#define QUEUE_CLEAR     0        /*queue is empty */
#define DOWN_EVENT      1        /*button press */
#define MOVE_EVENT      2        /*pointer move */
#define UP_EVENT      3          /*button release */
#define KEYPRESS_EVENT    4      /*key pressed */
#define SELECT_EVENT    5        /*press-release pair */
#define ANY_EVENT     6          /*any type */
#define DESTROY_EVENT   7

typedef struct graphicsevent
{
  struct graphicsevent *next;    /*next event */
  WINDOW fd;                     //structure of window
  INT16 fildes;                  //unix only
  INT8 type;                     /*event type */
  char key;                      /*keypress */
  float x, y;                    /*position of event */
  float xmax, ymax;              //for selection
} GRAPHICS_EVENT;                /*event type */

typedef void (*EVENT_HANDLER) (GRAPHICS_EVENT *);
#endif
