/**********************************************************************
 * File:        sbgtypes.h  (Formerly sbtypes.h)
 * Description: Structures for the Starbase/X daemon interface.
 * Author:      Ray Smith
 * Created:     Wed May 16 09:26:35 BST 1990
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

#ifndef           SBGTYPES_H
#define           SBGTYPES_H

//This file contains all the symbols and types that are not of concern
//to the user, but are common to both the client and the sbdaemon side.

#include          "sbgconst.h"
#ifdef __UNIX__
#include          <sys/types.h>
#endif

#define CHECK_TIME      1000     /*1 millisecond */
#define AWAIT_TIME      999999   /*1 second */
#define INFD        0            /*index to fds */
#define OUTFD       1            /*for read/write */
#define EVENTSIZE     16         //event buffer
#define PRIMITIVES      (*(INT32*)((char*)shminfo.shmstart+shminfo.shmsize))
#define EVENT_TAIL      (*(INT32*)((char*)shminfo.shmstart+shminfo.shmsize+sizeof(INT32)))
#define EVENT_HEAD      (*(INT32*)((char*)shminfo.shmstart+shminfo.shmsize+2*sizeof(INT32)))
#define EVENT_INDEX(index)  (((SBD_GRAPHICS_EVENT*)((char*)shminfo.shmstart+shminfo.shmsize+3*sizeof(INT32)))[index])

typedef enum {
  LINECOLOUR,                    /*line_color_index */
  PERIMETERCOLOUR,               /*perimeter_color_index */
  FILLCOLOUR,                    /*fill_color_index */
  MARKERCOLOUR,                  /*marker_color_index */
  TEXTCOLOUR,                    /*text_color_index */
  TEXTFONT,                      /*text_font_index */
  CHARHEIGHT,                    /*character_height */
  LINETYPE,                      /*line_type */
  MARKERTYPE,                    /*marker_type */
  MARKERSIZE,                    /*marker_size */
  MARKERMODE,                    /*mode in markersize */
  INTERIORSTYLE,                 /*interior_style */
  EDGESTYLE,                     /*edge in interior_style */

  MOVE2D,                        /*move2d */
  DRAW2D,                        /*draw2d */
  RECTANGLE,                     /*rectangle */
  TEXT_ALIGNMENT,                //alginment
  POLYLINE2D,                    /*polyline2d */
  POLYGON2D,                     /*polygon2d */
  POLYMARKER2D,                  /*polymarker2d */
  TEXT2D,                        /*text2d */
  APPENDTEXT,                    /*append_text */
  ELLIPSE,                       /*ellipse */
  ARC,                           /*arc */

  SHOWIMAGE,                     /*display image */
  SHOWLINE,                      /*send image line */

  CREATE,                        /*create_window */
  DESTROY,                       /*destroy_window */
  CLEAR,                         /*clear_view_surface */
  VDCEXTENT,                     /*vdc_extent */
  MAKECURRENT,                   /*make_picture_current */
  SETSIGNALS,                    //add callback
  SETECHO,                       //change cursor
  SYNCWIN,                       //sychronize
  RE_COMP_COLMAP                 //Re-compute colourmap
} CALL_TYPE;                     /*type of call */

typedef union
{
  INT16 fd;                      /*input fd */
  void *next;                    /*turns to link */
} HEADUNION;                     /*header of structure */

typedef union
{
  float f;                       /*parameter union */
  INT32 i;                       /*to reduce structures */
} PARAMUNION;

typedef struct
{
  PARAMUNION p;                  /*single parameter */
} ONEPARAM;

typedef struct
{
  PARAMUNION p[2];               /*two params */
} TWOPARAMS;

typedef struct
{
  PARAMUNION p[4];               /*4 params */
} FOURPARAMS;

typedef struct
{
  PARAMUNION p[8];               /*8 params */
} EIGHTPARAMS;

typedef struct
{
  HEADUNION header;              /*fd/next op */
  CALL_TYPE type;                /*call type */
  ONEPARAM param;                /*single parameter */
} ONEOP;

typedef struct
{
  HEADUNION header;              /*fd/next op */
  CALL_TYPE type;                /*call type */
  TWOPARAMS param;               /*single parameter */
} TWOOP;

typedef struct
{
  HEADUNION header;              /*fd/next op */
  CALL_TYPE type;                /*call type */
  FOURPARAMS param;              /*single parameter */
} FOUROP;

typedef struct
{
  HEADUNION header;              /*fd/next op */
  CALL_TYPE type;                /*call type */
  EIGHTPARAMS param;             /*single parameter */
} EIGHTOP;

typedef struct
{
  float *clist;                  /*coord list */
  INT32 numpts;                  /*number of coords */
  INT32 flags;                   /*move/draws on/off */
} POLYPARAM;                     /*polygon params */

typedef struct
{
  HEADUNION header;              /*fd/next */
  CALL_TYPE type;                /*operator type */
  POLYPARAM param;               /*parameters */
  float polyxmin, polyxmax;      /*bounding box */
  float polyymin, polyymax;      /*of polyline */
  float clist[1];                /*place holder */
} POLYOP;                        /*poly line/marker */

typedef struct
{
  float x, y;                    /*coords of text */
  char *string;                  /*string to draw */
  INT32 xform;                   /*coord transform */
  INT32 more;                    /*any more text */
} TEXTPARAM;                     /*text parameters */

typedef struct
{
  char *string;                  /*string to draw */
  INT32 xform;                   /*coord transform */
  INT32 more;                    /*any more text */
} APPENDPARAM;                   /*text parameters */

typedef struct
{
  HEADUNION header;              /*fd/next */
  CALL_TYPE type;                /*operator type */
  TEXTPARAM param;               /*parameters */
  char chars[4];                 /*place holder */
} TEXTOP;                        /*poly line/marker */

typedef struct
{
  HEADUNION header;              /*fd/next */
  CALL_TYPE type;                /*operator type */
  APPENDPARAM param;             /*parameters */
  char chars[4];                 /*place holder */
} APPENDOP;                      /*poly line/marker */

typedef struct
{
  HEADUNION header;              /*fd/next */
  CALL_TYPE type;                /*operator type */
  INT32 size;                    /*size of structure */
  UINT8 line[2];                 /*image line */
} IMAGEOP;                       /*image passing */

typedef struct
{
  HEADUNION header;              /*fd/next */
  CALL_TYPE type;                /*operator type */
  INT16 xpos, ypos;              /*initial position */
  INT16 xsize, ysize;            /*initial size */
  float xmin, xmax;              /*scrolling limits */
  float ymin, ymax;
  BOOL8 downon;                  /*events required */
  BOOL8 moveon;
  BOOL8 upon;
  BOOL8 keyon;
  INT32 window_type;             /*false for SMD */
  char name[MAXWINDOWNAME];      /*name of window */
} CREATEOP;

typedef struct
{
  #ifdef __UNIX__
  int fds[2];                    /*I/O files */
  int shmid;                     /*shared memory id */
  #else
  #ifdef __PCDEMON__
  int fds[2];                    /*I/O files */
  #else
  HANDLE fds[2];                 /*I/O files */
  #endif
  HANDLE shmid;                  //handle to it
  #endif
  void *shmstart;                /*addr of shm seg */
  INT32 usedsize;                /*amount used */
  INT32 shmsize;                 /*size of shm seg */
  #ifdef __UNIX
  pid_t pid;                     /*child process id */
  #endif
} SHMINFO;                       /*shared memory info */

typedef struct sbdgraphicsevent
{
  struct sbdgraphicsevent *next; /*next event */
  INT16 fd;                      //unix only
  INT8 type;                     /*event type */
  char key;                      /*keypress */
  float x, y;                    /*position of event */
} SBD_GRAPHICS_EVENT;            /*event type */

//typedef void                          (*SBFUNC)(WINFD*,...);          /*starbase function*/
typedef void (*DELFUNC) (void *, INT32);
                                 /*deletion function */
typedef INT16 SBDWINDOW;
#endif
