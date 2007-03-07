/**********************************************************************
 * File:        grphics.h  (Formerly graphics.h)
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

#ifndef           GRPHICS_H
#define           GRPHICS_H

//This is the main include file needed to get sbdaemon functionality
//TO BUILD A PROGRAM THAT USES SBDAEMON, YOU NEED:
//GRPHICS.CPP GRPHSHM.CPP AND EVNTLST.CPP.
//If you want to be able to wait for events, add evnts.cpp and include evnts.h.
//If you want to be able to show images, add showim.cpp and include showim.h.

#include          "sbgdefs.h"
#include          "sbgconst.h"

#define STATESIZE     13         //MUST equal EDGESTYLE+1

#define line_color_index(fd,index) fd->Line_color_index(index)
#define perimeter_color_index(fd,index) fd->Perimeter_color_index(index)
#define fill_color_index(fd,index) fd->Fill_color_index(index)
#define fill_color(fd,r,g,b) fd->Fill_color(r,g,b)
#define text_color_index(fd,index) fd->Text_color_index(index)
#define text_font_index(fd,index) fd->Text_font_index(index)
#define character_height(fd,height) fd->Character_height(height)
#define line_type(fd,style) fd->Line_type(style)
#define interior_style(fd,style,edged) fd->Interior_style(style,edged)
#define move2d(fd,x,y) fd->Move2d(x,y)
#define draw2d(fd,x,y) fd->Draw2d(x,y)
#define rectangle(fd,x1,y1,x2,y2) fd->Rectangle(x1,y1,x2,y2)
#define text2d(fd,x,y,string,xform,more) fd->Text2d(x,y,string,xform,more)
#define ellipse(fd,x_radius,y_radius,x_center,y_center,rotation) fd->Ellipse(x_radius,y_radius,x_center,y_center,rotation)
#define destroy_window(fd) fd->Destroy_window()
#define clear_view_surface(fd) fd->Clear_view_surface()
#define vdc_extent(fd,xmin,ymin,xmax,ymax) fd->Vdc_extent(xmin,ymin,xmax,ymax)
#define make_picture_current(fd) fd->Make_picture_current()
#define set_click_handler(fd,handler) fd->Set_click_handler(handler)
#define set_selection_handler(fd,handler) fd->Set_selection_handler(handler)
#define set_key_handler(fd,handler) fd->Set_key_handler(handler)
#define set_destroy_handler(fd,handler) fd->Set_destroy_handler(handler)
#define clear_event_queue(fd) fd->Clear_event_queue()
#define create_window(name,window_type,xpos,ypos,xsize,ysize,xmin,xmax,ymin,ymax,downon,moveon,upon,keyon) (*create_func)(name,window_type,xpos,ypos,xsize,ysize,xmin,xmax,ymin,ymax,downon,moveon,upon,keyon)
#define overlap_picture_ops(update) (*overlap_func)(update)
#define await_event(win,wait,type,event) (*await_event_func)(win,wait,type,event)

typedef void (*EVENT_HANDLER) (GRAPHICS_EVENT *);
                                 /*name/title of window */
typedef WINDOW (*WINCREATEFUNC) (const char *name,
INT8 window_type,                /*type of window */
INT16 xpos,                      /*coords of window */
INT16 ypos,                      /*coords of window */
INT16 xsize,                     /*size of window */
INT16 ysize,                     /*size of window */
float xmin,                      /*scrolling limits */
float xmax,                      /*to stop users */
float ymin,                      /*getting lost in */
float ymax,                      /*empty space */
BOOL8 downon,                    /*Events wanted */
BOOL8 moveon, BOOL8 upon, BOOL8 keyon);

extern WINCREATEFUNC create_func;
extern void (*overlap_func) (BOOL8);
extern WINDOW (*await_event_func) (WINDOW, BOOL8, INT8, GRAPHICS_EVENT *);

class DLLSYM WINFD
{
  public:
    //Constructors for WINFD are host dependent to match different
    //implementations and are intended to be called only by
    //create_window. Use create_window to make a window.
    static WINDOW create(                   /*create a window */
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
                         BOOL8 keyon);
    WINFD();  //constructor
    virtual ~ WINFD ();

                                 /*set line colour */
    virtual void Line_color_index(COLOUR index);  /*index to use */
                                 /*set perimeter colour */
    virtual void Perimeter_color_index(COLOUR index);  /*index to use */
                                 /*set fill colour */
    virtual void Fill_color_index(COLOUR index);  /*index to use */
    virtual void Fill_color(  /*set RGB fill colour */
                            UINT8 red,
                            UINT8 green,
                            UINT8 blue);
                                 /*set marker colour */
    virtual void Marker_color_index(COLOUR index);  /*index to use */
                                 /*set text colour */
    virtual void Text_color_index(COLOUR index);  /*index to use */
                                 /*set text font */
    virtual void Text_font_index(INT16 index);  /*index to use */
                                 /*set text height */
    virtual void Character_height(float height);  /*height to use */
    virtual void Line_type(               /*set line type */
                           INT16 style);  /*style to use */
    virtual void Marker_type(              /*set marker type */
                             INT16 type);  /*type to use */
    virtual void Interior_style(               /*set polygon style */
                                INT16 style,   /*style to use */
                                INT16 edged);  /*draw edge or not */
    virtual void Marker_size(              /*set marker size */
                             float size);  /*size to use */
    virtual void Move2d(           /*move the pen */
                        float x,   /*coords to move to */
                        float y);  /*coords to move to */
    virtual void Draw2d(           /*draw the pen */
                        float x,   /*coords to draw to */
                        float y);  /*coords to draw to */
    virtual void Rectangle(            /*draw a rectangle */
                           float x1,   /*coords to draw to */
                           float y1,   /*coords to draw to */
                           float x2,   /*coords to draw to */
                           float y2);  /*coords to draw to */
    virtual void Text_alignment(                 /*draw a rectangle */
                                INT32 h_select,  //horizontal
                                INT32 v_select,  //vertical
                                float horiz,     /*coords to draw to */
                                float vert);     /*coords to draw to */
    virtual void Polyline2d (    /*draw a polyline */
      float clist[],             /*coordinate list */
      INT16 numpts,              /*number of coords */
      INT16 flags);              /*does it have move/draws */
    virtual void Polygon2d (     /*draw a polygon */
      float clist[],             /*coordinate list */
      INT16 numpts,              /*number of coords */
      INT16 flags);              /*does it have move/draws */
    virtual void Polymarker2d (  /*draw a polymarker */
      float clist[],             /*coordinate list */
      INT16 numpts,              /*number of coords */
      INT16 flags);              /*does it have move/draws */
    virtual void Text2d(                     /*draw a text */
                        float x,             /*coords of text */
                        float y,
                        const char *string,  /*text to draw */
                        INT16 xform,         /*transform */
                        INT16 more);         /*more text? */
    void Append_text(                     /*draw a text */
                     const char *string,  /*text to draw */
                     INT16 xform,         /*transform */
                     INT16 more);         /*more text? */
    virtual void Ellipse(                  /*draw a ellipse */
                         float x_radius,   /*radii of ellipse */
                         float y_radius,   /*radii of ellipse */
                         float x_center,   /*centre of ellipse */
                         float y_center,   /*centre of ellipse */
                         float rotation);  /*rotation of ellipse */
    virtual void Arc(                    /*draw a arc */
                     float x_radius,     /*radii of arc */
                     float y_radius,     /*radii of arc */
                     float x_center,     /*centre of arc */
                     float y_center,     /*centre of arc */
                     float start,        /*ends of arc */
                     float stop,         /*ends of arc */
                     float rotation,     /*rotation of arc */
                     INT16 close_type);  /*type of closure */
                                 /*destroy a window */
    virtual void Destroy_window(); 
                                 /*clear window */
    virtual void Clear_view_surface(); 
                                 /*Mark need to recalc */
    virtual void Re_compute_colourmap(); 
    virtual void Vdc_extent(              /*set window focus */
                            float xmin,   /*min values */
                            float ymin,   /*min values */
                            float xmax,   /*max values */
                            float ymax);  /*max values */
    void Set_echo(                      /*set window echo */
                  ECHO_TYPE echo_type,  //type of echo
                  float xorig,          /*min values */
                  float yorig);         /*min values */
                                 /*update window */
    virtual void Make_picture_current(); 
                                 /*flush output */
    friend void def_overlap_picture_ops(BOOL8 update);  /*send make_ */
                                 /*set line colour */
    virtual void Synchronize_windows(WINDOW fd2);  //other window
    /*	void					Show_sub_image(				//show rectangle
        IMAGE*				source,						//source image
        INT32				xstart,						//start coords
        INT32				ystart,
        INT32				xext,						//extent to copy
        INT32				yext,
        INT32				xdest,						//destination coords
        INT32				ydest);*/

    INT16 get_fd() {  //access
      return fd;
    }

    void Set_selection_handler(                         //set callback function
                               EVENT_HANDLER handler);  //handler function
    void Set_key_handler(                         //set callback function
                         EVENT_HANDLER handler);  //handler function
    void Set_destroy_handler(                         //set callback function
                             EVENT_HANDLER handler);  //handler function
    void Set_click_handler(                         //set callback function
                           EVENT_HANDLER handler);  //handler function
                                 /*delete all events */
    virtual void Clear_event_queue(); 

    //internal maintenance functions
    friend void add_event(                         /*add an event */
                          GRAPHICS_EVENT *event);  /*event to add */
                                 /*search for event */
    friend GRAPHICS_EVENT *search_event_queue(INT16 &fd,         /*queue to search */
                                              INT8 event_type);  /*type to search for */
                                 /*search for event */
    friend GRAPHICS_EVENT *search_single_queue(INT16 fd,          /*queue to search */
                                               INT8 event_type);  /*type to search for */

  protected:
    EVENT_HANDLER click_handler; //callback function
                                 //callback function
    EVENT_HANDLER selection_handler;
    EVENT_HANDLER key_handler;   //callback function
                                 //callback function
    EVENT_HANDLER destroy_handler;
  private:
    void get_lock() { 
    }                            //wait for lock
    void get_lock_for_draw() { 
    }                            //kill draw thread
    void release_lock() { 
    }                            //let it go
    void get_core_lock() { 
    }                            //wait for lock
    void release_core_lock() { 
    }                            //let it go

    INT16 fd;                    //"file descriptor"
    BOOL8 used;                  /*true if fd in use */
    BOOL8 downevent;             /*event flags */
    BOOL8 moveevent;
    BOOL8 upevent;
    BOOL8 keyevent;
    GRAPHICS_EVENT *events;      /*event queue */
    GRAPHICS_EVENT *lastevent;   /*event queue */

};
#endif
