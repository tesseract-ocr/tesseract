/**********************************************************************
 * File:        grphics.c  (Formerly graphics.c)
 * Description: Starbase stubs for connection to sbdaemon
 * Author:      Ray Smith
 * Created:     Wed May 16 08:34:32 BST 1990
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
#include          <stdlib.h>
#include          <string.h>
#ifdef __UNIX__
#include          <signal.h>
#endif
#include          "grphics.h"
//#include                                      "sbderrs.h"
//#include                                      "grpherr.h"
#include          "grphshm.h"
#include          "evntlst.h"

#define XSIZE_INCREMENT   8
#define YSIZE_INCREMENT   30

void def_overlap_picture_ops(BOOL8 update);

WINCREATEFUNC create_func = WINFD::create;
void (*overlap_func) (BOOL8) = def_overlap_picture_ops;

/**********************************************************************
 * line_color_index
 *
 * Set the colour map index for drawing lines with.
 **********************************************************************/

void WINFD::Line_color_index(              /*set line colour */
                             COLOUR index  /*index to use */
                            ) {
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = LINECOLOUR;    /*send the operator */
    newop->param.p.i = index;    /*set parameter */
  }
}


/**********************************************************************
 * perimeter_color_index
 *
 * Set the colour map index for drawing perimeters with.
 **********************************************************************/

void WINFD::Perimeter_color_index(              /*set perimeter colour */
                                  COLOUR index  /*index to use */
                                 ) {
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
                                 /*send the operator */
    newop->type = PERIMETERCOLOUR;
    newop->param.p.i = index;    /*set parameter */
  }
}


/**********************************************************************
 * fill_color_index
 *
 * Set the colour map index for drawing fills with.
 **********************************************************************/

void WINFD::Fill_color_index(              /*set fill colour */
                             COLOUR index  /*index to use */
                            ) {
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = FILLCOLOUR;    /*send the operator */
    newop->param.p.i = index;    /*set parameter */
  }
}


/**********************************************************************
 * fill_color
 *
 * Set the RGB colour for drawing fills with.
 **********************************************************************/

void WINFD::Fill_color(  /*set fill colour */
                       UINT8 red,
                       UINT8 green,
                       UINT8 blue) {
  ONEOP *newop;                  /*message structure */
  UINT32 packed_colour;

  packed_colour = (blue << 24) + (green << 16) + (red << 8);
                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = FILLCOLOUR;    /*send the operator */
                                 /*set parameter */
    newop->param.p.i = (INT32) packed_colour;
  }
}


/**********************************************************************
 * marker_color_index
 *
 * Set the colour map index for drawing markers with.
 **********************************************************************/

void WINFD::Marker_color_index(              /*set marker colour */
                               COLOUR index  /*index to use */
                              ) {
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = MARKERCOLOUR;  /*send the operator */
    newop->param.p.i = index;    /*set parameter */
  }
}


/**********************************************************************
 * text_color_index
 *
 * Set the colour map index for drawing texts with.
 **********************************************************************/

void WINFD::Text_color_index(              /*set text colour */
                             COLOUR index  /*index to use */
                            ) {
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = TEXTCOLOUR;    /*send the operator */
    newop->param.p.i = index;    /*set parameter */
  }
}


/**********************************************************************
 * text_font_index
 *
 * Set the text font index for drawing texts with.
 **********************************************************************/

void WINFD::Text_font_index(             /*set text font */
                            INT16 index  /*index to use */
                           ) {
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = TEXTFONT;      /*send the operator */
    newop->param.p.i = index;    /*set parameter */
  }
}


/**********************************************************************
 * character_height
 *
 * Set the VDC height of subsequent text.
 **********************************************************************/

void WINFD::Character_height(              /*set text height */
                             float height  /*height to use */
                            ) {
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = CHARHEIGHT;    /*send the operator */
    newop->param.p.f = height;   /*set parameter */
  }
}


/**********************************************************************
 * line_type
 *
 * Set the line type for all subsequent lines.
 **********************************************************************/

void WINFD::Line_type(             /*set line type */
                      INT16 style  /*style to use */
                     ) {
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = LINETYPE;      /*send the operator */
    newop->param.p.i = style;    /*set parameter */
  }
}


/**********************************************************************
 * marker_type
 *
 * Set the marker type for all subsequent lines.
 **********************************************************************/

void WINFD::Marker_type(            /*set marker type */
                        INT16 type  /*type to use */
                       ) {
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = MARKERTYPE;    /*send the operator */
    newop->param.p.i = type;     /*set parameter */
  }
}


/**********************************************************************
 * interior_style
 *
 * Set the fill type and boundary presence for arcs, ellipses etc.
 **********************************************************************/

void WINFD::Interior_style(              /*set polygon style */
                           INT16 style,  /*style to use */
                           INT16 edged   /*draw edge or not */
                          ) {
  TWOOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (TWOOP *) getshm (sizeof (TWOOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = INTERIORSTYLE; /*send the operator */
    newop->param.p[0].i = style; /*set parameter */
    newop->param.p[1].i = edged;
  }
}


/**********************************************************************
 * marker_size
 *
 * Set the size of markers in polymarker2d.
 **********************************************************************/

void WINFD::Marker_size(            /*set marker size */
                        float size  /*size to use */
                       ) {
  TWOOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (TWOOP *) getshm (sizeof (TWOOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = MARKERSIZE;    /*send the operator */
    newop->param.p[0].f = size;  /*set parameter */
    newop->param.p[1].i = FALSE;
  }
}


/**********************************************************************
 * move2d
 *
 * Move the pen to the given position.
 **********************************************************************/

void WINFD::Move2d(          /*move the pen */
                   float x,  /*coords to move to */
                   float y   /*coords to move to */
                  ) {
  TWOOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (TWOOP *) getshm (sizeof (TWOOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = MOVE2D;        /*send the operator */
    newop->param.p[0].f = x;     /*set parameters */
    newop->param.p[1].f = y;
  }
}


/**********************************************************************
 * draw2d
 *
 * Draw from current position to the given position using the current colour
 **********************************************************************/

void WINFD::Draw2d(          /*draw the pen */
                   float x,  /*coords to draw to */
                   float y   /*coords to draw to */
                  ) {
  TWOOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (TWOOP *) getshm (sizeof (TWOOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = DRAW2D;        /*send the operator */
    newop->param.p[0].f = x;     /*set parameters */
    newop->param.p[1].f = y;
  }
}


/**********************************************************************
 * rectangle
 *
 * Draw a rectangle using current perimeter/interior controls.
 **********************************************************************/

void WINFD::Rectangle(           /*draw a rectangle */
                      float x1,  /*coords to draw to */
                      float y1,  /*coords to draw to */
                      float x2,  /*coords to draw to */
                      float y2   /*coords to draw to */
                     ) {
  FOUROP *newop;                 /*message structure */

                                 /*get some space */
  newop = (FOUROP *) getshm (sizeof (FOUROP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = RECTANGLE;     /*send the operator */
    newop->param.p[0].f = x1;    /*set parameters */
    newop->param.p[1].f = y1;
    newop->param.p[2].f = x2;
    newop->param.p[3].f = y2;
  }
}


/**********************************************************************
 * text_alignment
 *
 * Control text position.
 **********************************************************************/

void WINFD::Text_alignment(                 /*draw a rectangle */
                           INT32 h_select,  //horizontal
                           INT32 v_select,  //vertical
                           float horiz,     /*coords to draw to */
                           float vert       /*coords to draw to */
                          ) {
  FOUROP *newop;                 /*message structure */

                                 /*get some space */
  newop = (FOUROP *) getshm (sizeof (FOUROP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = TEXT_ALIGNMENT;/*send the operator */
    newop->param.p[0].i = h_select;
    newop->param.p[1].i = v_select;
    newop->param.p[2].f = horiz;
    newop->param.p[3].f = vert;
  }
}


/**********************************************************************
 * polyline2d
 *
 * Draw a polyline using current line colour/type.
 **********************************************************************/

void
WINFD::Polyline2d (              /*draw a polyline */
float clist[],                   /*coordinate list */
INT16 numpts,                    /*number of coords */
INT16 flags                      /*does it have move/draws */
) {
  POLYOP *newop;                 /*message structure */
  INT32 floatcount;              /*no of floats */

  floatcount = flags ? numpts * 3/*move/draw flags in */
    : numpts * 2;                /*no move/draw flags */
                                 /*get some space */
  newop = (POLYOP *) getshm (sizeof (POLYOP) + sizeof (float) * (floatcount - 1));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = POLYLINE2D;    /*send the operator */
                                 /*pointer to array */
    newop->param.clist = newop->clist;
    newop->param.numpts = numpts;/*other params */
    newop->param.flags = flags;
    memcpy (newop->clist, clist, (UINT32) floatcount * sizeof (float));
  }
}


/**********************************************************************
 * polygon2d
 *
 * Draw a polygon using current line colour/type.
 **********************************************************************/

void
WINFD::Polygon2d (               /*draw a polygon */
float clist[],                   /*coordinate list */
INT16 numpts,                    /*number of coords */
INT16 flags                      /*does it have move/draws */
) {
  POLYOP *newop;                 /*message structure */
  INT32 floatcount;              /*no of floats */

  floatcount = flags ? numpts * 3/*move/draw flags in */
    : numpts * 2;                /*no move/draw flags */
                                 /*get some space */
  newop = (POLYOP *) getshm (sizeof (POLYOP) + sizeof (float) * (floatcount - 1));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = POLYGON2D;     /*send the operator */
                                 /*pointer to array */
    newop->param.clist = newop->clist;
    newop->param.numpts = numpts;/*other params */
    newop->param.flags = flags;
    memcpy (newop->clist, clist, (UINT32) floatcount * sizeof (float));
  }
}


/**********************************************************************
 * polymarker2d
 *
 * Draw a polymarker using current marker colour/type.
 **********************************************************************/

void
WINFD::Polymarker2d (            /*draw a polymarker */
float clist[],                   /*coordinate list */
INT16 numpts,                    /*number of coords */
INT16 flags                      /*does it have move/draws */
) {
  POLYOP *newop;                 /*message structure */
  INT32 floatcount;              /*no of floats */

  floatcount = flags ? numpts * 3/*move/draw flags in */
    : numpts * 2;                /*no move/draw flags */
                                 /*get some space */
  newop = (POLYOP *) getshm (sizeof (POLYOP) + sizeof (float) * (floatcount - 1));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = POLYMARKER2D;  /*send the operator */
                                 /*pointer to array */
    newop->param.clist = newop->clist;
    newop->param.numpts = numpts;/*other params */
    newop->param.flags = flags;
    memcpy (newop->clist, clist, (UINT32) floatcount * sizeof (float));
  }
}


/**********************************************************************
 * text2d
 *
 * Draw a text string using current font, colour, height.
 **********************************************************************/

void WINFD::Text2d(                     /*draw a text */
                   float x,             /*coords of text */
                   float y,
                   const char *string,  /*text to draw */
                   INT16 xform,         /*transform */
                   INT16 more           /*more text? */
                  ) {
  TEXTOP *newop;                 /*message structure */
  INT16 length;                  /*length of string */

  length = strlen (string) + 1;  /*include null */
  length += 3;
  length &= ~3;                  /*round up to words */
                                 /*get some space */
  newop = (TEXTOP *) getshm (sizeof (TEXTOP) + length - 4);
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = TEXT2D;        /*send the operator */
    newop->param.x = x;          /*copy parameters */
    newop->param.y = y;
    newop->param.string = newop->chars;
    newop->param.xform = xform;
    newop->param.more = more;
                                 /*copy the string */
    strcpy (newop->chars, string);
  }
}


/**********************************************************************
 * append_text
 *
 * Draw a text string using current font, colour, height.
 **********************************************************************/

void WINFD::Append_text(                     /*draw a text */
                        const char *string,  /*text to draw */
                        INT16 xform,         /*transform */
                        INT16 more           /*more text? */
                       ) {
  APPENDOP *newop;               /*message structure */
  INT16 length;                  /*length of string */

  length = strlen (string) + 1;  /*include null */
  length += 3;
  length &= ~3;                  /*round up to words */
                                 /*get some space */
  newop = (APPENDOP *) getshm (sizeof (APPENDOP) + length - 4);
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = APPENDTEXT;    /*send the operator */
    newop->param.string = newop->chars;
    newop->param.xform = xform;
    newop->param.more = more;
                                 /*copy the string */
    strcpy (newop->chars, string);
  }
}


/**********************************************************************
 * ellipse
 *
 * Draw an ellipse using current perimeter/interior controls.
 **********************************************************************/

void WINFD::Ellipse(                 /*draw a ellipse */
                    float x_radius,  /*radii of ellipse */
                    float y_radius,  /*radii of ellipse */
                    float x_center,  /*centre of ellipse */
                    float y_center,  /*centre of ellipse */
                    float rotation   /*rotation of ellipse */
                   ) {
  EIGHTOP *newop;                /*message structure */

                                 /*get some space */
  newop = (EIGHTOP *) getshm (sizeof (EIGHTOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = ELLIPSE;       /*send the operator */
                                 /*set parameters */
    newop->param.p[0].f = x_radius;
    newop->param.p[1].f = y_radius;
    newop->param.p[2].f = x_center;
    newop->param.p[3].f = y_center;
    newop->param.p[4].f = rotation;
  }
}


/**********************************************************************
 * arc
 *
 * Draw an arc using current perimeter/interior controls.
 **********************************************************************/

void WINFD::Arc(                  /*draw a arc */
                float x_radius,   /*radii of arc */
                float y_radius,   /*radii of arc */
                float x_center,   /*centre of arc */
                float y_center,   /*centre of arc */
                float start,      /*ends of arc */
                float stop,       /*ends of arc */
                float rotation,   /*rotation of arc */
                INT16 close_type  /*type of closure */
               ) {
  EIGHTOP *newop;                /*message structure */

                                 /*get some space */
  newop = (EIGHTOP *) getshm (sizeof (EIGHTOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = ARC;           /*send the operator */
                                 /*set parameters */
    newop->param.p[0].f = x_radius;
    newop->param.p[1].f = y_radius;
    newop->param.p[2].f = x_center;
    newop->param.p[3].f = y_center;
    newop->param.p[4].f = start;
    newop->param.p[5].f = stop;
    newop->param.p[6].f = rotation;
    newop->param.p[7].i = close_type;
  }
}


/**********************************************************************
 * create_window
 *
 * Create a window and register event handlers on the window.
 * The return "file descriptor" is used in subsequent starbase
 * primitives to refer to this window.
 **********************************************************************/

WINDOW WINFD::create(                   /*create a window */
                     const char *name,  /*name/title of window */
                     INT8 window_type,  /*type of window */
                     INT16 xpos,        /*coords of window */
                     INT16 ypos,        /*coords of window */
                     INT16 xsize,       /*size of window */
                     INT16 ysize,       /*size of window */
                     float xmin,        /*scrolling limits */
                     float xmax,        /*to stop users */
                     float ymin,        /*getting lost in */
                     float ymax,        /*empty space */
                     BOOL8 downon,      /*Events wanted */
                     BOOL8 moveon,
                     BOOL8 upon,
                     BOOL8 keyon) {
  INT16 fd;                      //integer index
  CREATEOP *newop;               /*create structure */
  WINDOW win;                    //output

  if (xmin == xmax || ymin == ymax)
    return NO_WINDOW;
  if (maxsbfd == 0) {
    maxsbfd = 1;                 /*don't use 0 */
    start_sbdaemon();  /*startup daemon */
  }

                                 /*find a free one */
  for (fd = 1; fd < maxsbfd && sbfds[fd].used; fd++);
  if (fd == maxsbfd) {           /*need a new one */
    if (maxsbfd == MAXWINDOWS)
      return NO_WINDOW;
    maxsbfd++;
  }
  win = &sbfds[fd];              //this
  win->fd = fd;
  win->used = TRUE;              /*it is in use */
  win->downevent = downon;
  win->moveevent = moveon;
  win->upevent = upon;
  win->keyevent = keyon;
  win->click_handler = NULL;
  win->selection_handler = NULL;
  win->key_handler = NULL;
  win->destroy_handler = NULL;
  win->events = NULL;
  win->lastevent = NULL;

  newop = (CREATEOP *) getshm (sizeof (CREATEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*file descriptor */
    newop->type = CREATE;        /*set operator type */
    newop->window_type = window_type;
                                 /*copy window name */
    strncpy (newop->name, name, MAXWINDOWNAME - 1);
    newop->name[MAXWINDOWNAME - 1] = '\0';
    newop->xpos = xpos;
    newop->ypos = ypos;
    newop->xsize = xsize;
    newop->ysize = ysize;
    newop->xmin = xmin;
    newop->xmax = xmax;
    newop->ymin = ymin;
    newop->ymax = ymax;
    newop->downon = downon;
    newop->moveon = moveon;
    newop->upon = upon;
    newop->keyon = keyon;
  }
  return win;                    /*file descriptor */
}


/**********************************************************************
 * WINFD::WINFD
 *
 * Constructor to initialize a WINFD entry.
 **********************************************************************/

WINFD::WINFD() {  //constructor
  fd = -1;
  used = FALSE;
  downevent = FALSE;
  moveevent = FALSE;
  upevent = FALSE;
  keyevent = FALSE;
  click_handler = NULL;
  selection_handler = NULL;
  key_handler = NULL;
  destroy_handler = NULL;
  events = NULL;
  lastevent = NULL;
}


WINFD::~WINFD () {
}


/**********************************************************************
 * destroy_window
 *
 * Destroy a window and free the file descriptor
 **********************************************************************/

void WINFD::Destroy_window() {  /*destroy a window */
  ONEOP *newop;                  /*destroy structure */

  if (fd < 1 || fd > maxsbfd || sbfds[fd].used == FALSE) {
    return;
  }
  else {
    Clear_event_queue(); 
    sbfds[fd].used = FALSE;      /*it is not in use */
    sbfds[fd].click_handler = NULL;

    newop = (ONEOP *) getshm (sizeof (ONEOP));
    if (newop != NULL) {
      newop->header.fd = fd;     /*file descriptor */
      newop->type = DESTROY;     /*set operator type */
    }
  }
}


/**********************************************************************
 * Clear_event_queue
 *
 * Clear the queue of events for this window.
 **********************************************************************/

void WINFD::Clear_event_queue() {  /*clear events */
  INT16 fd;                      //current window
  GRAPHICS_EVENT *event;         /*current event */
  GRAPHICS_EVENT *nextevent;     /*next in list */

  if (this == NULL) {
    for (fd = 1; fd < maxsbfd; fd++) {
      if (sbfds[fd].used) {
        sbfds[fd].Clear_event_queue ();
      }
    }
  }
  else {
    for (event = events; event != NULL; event = nextevent) {
      nextevent = event->next;
      delete event;              //free them all
    }
    events = NULL;               /*there are none now */
  }
}


/**********************************************************************
 * clear_view_surface
 *
 * Clear the window and empty the display list, discarding images also.
 **********************************************************************/

void WINFD::Clear_view_surface() {  /*clear window */
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = CLEAR;         /*send the operator */
  }
}


/**********************************************************************
 * re_compute_colourmap
 *
 * Tell SBD to recalc_colourmap for this window.
 **********************************************************************/

void WINFD::Re_compute_colourmap() {  /*Mark need to recalc */
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = RE_COMP_COLMAP;/*send the operator */
  }
  /*
    ONE DAY THE PC VERSION WILL SUPPORT COLOUR - BUT NOT TODAY

    Among the things that will need doing is to change the size of..
      PCSTUBSPEC           stubspecs[SYNCWIN+1];
    in pcsbdg.[ch] to RE_COMP_COLMAP+1
  */
}


/**********************************************************************
 * vdc_extent
 *
 * Shift/scale the window to focus on the given region.
 **********************************************************************/

void WINFD::Vdc_extent(             /*set window focus */
                       float Xmin,  /*min values */
                       float Ymin,  /*min values */
                       float Xmax,  /*max values */
                       float Ymax   /*max values */
                      ) {
  EIGHTOP *newop;                /*message structure */

                                 /*get some space */
  newop = (EIGHTOP *) getshm (sizeof (EIGHTOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = VDCEXTENT;     /*send the operator */
    newop->param.p[0].f = Xmin;  /*set parameters */
    newop->param.p[1].f = Ymin;
    newop->param.p[2].f = 0.0f;
    newop->param.p[3].f = Xmax;
    newop->param.p[4].f = Ymax;
    newop->param.p[5].f = 0.0f;
  }
}


/**********************************************************************
 * set_echo
 *
 * Set the starbase echo/cursor.
 **********************************************************************/

void WINFD::Set_echo(                      /*set window echo */
                     ECHO_TYPE echo_type,  //type of echo
                     float xorig,          /*min values */
                     float yorig           /*min values */
                    ) {
  FOUROP *newop;                 /*message structure */

                                 /*get some space */
  newop = (FOUROP *) getshm (sizeof (FOUROP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = SETECHO;       /*send the operator */
    newop->param.p[0].i = echo_type;
    newop->param.p[1].f = xorig;
    newop->param.p[2].f = yorig;
  }
}


/**********************************************************************
 * overlap_picture_ops
 *
 * Clear the output buffer without waiting for acknowledge response.
 * If update is TRUE, make_picture_currents are issued on all windows,
 * but they are still not waited for.
 **********************************************************************/

DLLSYM void def_overlap_picture_ops(              /*flush output */
                                    BOOL8 update  /*send make_ */
                                   ) {
  ONEOP *newop;                  /*message structure */
  INT16 fd;                      /*file descriptor */

  if (update) {
    for (fd = 1; fd < maxsbfd; fd++) {
      if (sbfds[fd].used) {
                                 /*get some space */
        newop = (ONEOP *) getshm (sizeof (ONEOP));
        if (newop != NULL) {
          newop->header.fd = fd; /*send the fd */
                                 /*send the operator */
          newop->type = MAKECURRENT;
        }
      }
    }
  }
  kick_daemon(FLUSH_OUT);  /*empty shm */
}


/**********************************************************************
 * make_picture_current
 *
 * Clear the buffer and make sure the image is up-to-date.
 * If a window of 0 is given, all windows are updated.
 **********************************************************************/

void WINFD::Make_picture_current() {  /*update window */
  ONEOP *newop;                  /*message structure */

  if (this == NULL || fd <= 0) {
    overlap_picture_ops(TRUE); 
  }
  else {
                                 /*get some space */
    newop = (ONEOP *) getshm (sizeof (ONEOP));
    if (newop != NULL) {
      newop->header.fd = fd;     /*send the fd */
      newop->type = MAKECURRENT; /*send the operator */
      kick_daemon(FLUSH_IN);  /*empty shm */
    }
  }
}


/**********************************************************************
 * synchronize_windows
 *
 * Make zoom, scroll and resize operations ripple over other window(s).
 **********************************************************************/

void WINFD::Synchronize_windows(            /*set line colour */
                                WINDOW fd2  //other window
                               ) {
  ONEOP *newop;                  /*message structure */

                                 /*get some space */
  newop = (ONEOP *) getshm (sizeof (ONEOP));
  if (newop != NULL) {
    newop->header.fd = fd;       /*send the fd */
    newop->type = SYNCWIN;       /*send the operator */
    newop->param.p.i = fd2->fd;  /*set parameter */
  }
}


/**********************************************************************
 * set_click_handler
 *
 * Set a callback function for click events.
 **********************************************************************/

void WINFD::Set_click_handler(                       //set callback function
                              EVENT_HANDLER handler  //handler function
                             ) {
  click_handler = handler;       //remember it
}


/**********************************************************************
 * set_selection_handler
 *
 * Set a callback function for selection events.
 **********************************************************************/

void WINFD::Set_selection_handler(                       //set callback function
                                  EVENT_HANDLER handler  //handler function
                                 ) {
  selection_handler = handler;   //remember it
}


/**********************************************************************
 * set_key_handler
 *
 * Set a callback function for key events.
 **********************************************************************/

void WINFD::Set_key_handler(                       //set callback function
                            EVENT_HANDLER handler  //handler function
                           ) {
  key_handler = handler;         //remember it
}


/**********************************************************************
 * set_destroy_handler
 *
 * Set a callback function for destroy events.
 **********************************************************************/

void WINFD::Set_destroy_handler(                       //set callback function
                                EVENT_HANDLER handler  //handler function
                               ) {
  destroy_handler = handler;     //remember it
}
