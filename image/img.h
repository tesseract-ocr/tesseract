/**********************************************************************
 * File:        img.h  (Formerly image.h)
 * Description: Class definition for the IMAGE class.
 * Author:      Ray Smith
 * Created:     Thu Jun 07 13:42:37 BST 1990
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

#ifndef           IMG_H
#define           IMG_H

#include          "memry.h"

#define MAXIMAGEWIDTH   (900*14) /*14inch * 400dpi */
                                 /*14inch * 400dpi */
#define MAXIMAGEHEIGHT    (900*14)

#define COMPUTE_IMAGE_XDIM(xsize,bpp) ((bpp)>8 ? ((xsize)*(bpp)+7)/8 :((xsize)+8/(bpp)-1)/(8/(bpp)))

typedef INT8 (*IMAGE_OPENER) (int, INT32 *, INT32 *, INT8 *, INT8 *, INT32 *);
typedef INT8 (*IMAGE_READER) (int, UINT8 *, INT32, INT32, INT8, INT32);
typedef INT8 (*IMAGE_WRITER) (int, UINT8 *, INT32, INT32, INT8, INT8, INT32);

typedef UINT8 *COLOUR_PIX;       //array of colours
enum COLOUR_PIX_NAME
{
  RED_PIX,
  GREEN_PIX,
  BLUE_PIX
};

class DLLSYM IMAGELINE;

class DLLSYM IMAGE               //encapsulated image
{
  public:
    IMAGE();  //constructor

    ~IMAGE () {                  //destructor
      destroy();  //free memory
    }

    IMAGE & operator= (          //assignment
      IMAGE & source);

    INT8 read_header(                    //get file header
                     const char *name);  //name of image

    INT8 read(                  //get rest of image
              INT32 buflines);  //size of buffer

    INT8 write(                    //write image
               const char *name);  //name to write

    INT8 create(                       //create blank image
                INT32 x,               //x size required
                INT32 y,               //ysize required
                INT8 bits_per_pixel);  //bpp required

    INT8 capture(                       //capture raw image
                 UINT8 *pixels,         //pixels to capture
                 INT32 x,               //x size required
                 INT32 y,               //ysize required
                 INT8 bits_per_pixel);  //bpp required

    void destroy();  //destroy image

    INT32 get_xsize() { 
      return xsize;
    }
    //access function
    INT32 get_ysize() { 
      return ysize;
    }
    //access function
    INT8 get_bpp() { 
      return bpp;
    }                            //access function
    INT8 get_bps() { 
      return bps;
    }                            //bits per sample
    BOOL8 white_high() {  //photo interp
      return photo_interp;
    }
    UINT8 get_white_level() {  //access function
      return (1 << bpp) - 1;
    }
    INT32 get_res() { 
      return res;
    }                            //access function
    void set_res(  //set resolution
                 INT32 resolution) {
      res = resolution;
    }
    UINT8 *get_buffer() { 
      return image;
    }
    //access function

    UINT8 pixel(           //access pixel
                INT32 x,   //x coord
                INT32 y);  //y coord

    void fast_get_line(                      //get image line
                       INT32 x,              //coord to start at
                       INT32 y,              //line to get
                       INT32 width,          //no of pixels to get
                       IMAGELINE *linebuf);  //line to copy to

    void get_line(                     //get image line
                  INT32 x,             //coord to start at
                  INT32 y,             //line to get
                  INT32 width,         //no of pixels to get
                  IMAGELINE *linebuf,  //line to copy to
                  INT32 margins);      //size of margins
    void get_column(                     //get image column
                    INT32 x,             //coord to start at
                    INT32 y,             //line to get
                    INT32 height,        //no of pixels to get
                    IMAGELINE *linebuf,  //line to copy to
                    INT32 margins);      //size of margins

    void fast_put_line(                      //put image line
                       INT32 x,              //coord to start at
                       INT32 y,              //line to put
                       INT32 width,          //no of pixels to put
                       IMAGELINE *linebuf);  //line to copy from

    void put_line(                     //put image line
                  INT32 x,             //coord to start at
                  INT32 y,             //line to put
                  INT32 width,         //no of pixels to put
                  IMAGELINE *linebuf,  //line to copy from
                  INT32 margins);      //size of margins
    void put_column(                     //put image column
                    INT32 x,             //coord to start at
                    INT32 y,             //line to put
                    INT32 height,        //no of pixels to put
                    IMAGELINE *linebuf,  //line to copy to
                    INT32 margins);      //size of margins

    void check_legal_access(              //check coords
                            INT32 x,      //xcoord to check
                            INT32 y,
                            INT32 xext);  //ycoord to check

    void convolver (             //Map fn over window
      INT32 win_width,           //Window width
      INT32 win_height,          //Window height
      void (*convolve) (         //Conv Function
      UINT8 ** pixels,           //Of window
      UINT8 bytespp,             //1 or 3 for colour
      INT32 win_wd,              //Window width
      INT32 win_ht,              //Window height
      UINT8 ret_white_value,     //White value to RETURN
      UINT8 * result));          //Result pixel(s)

                                 //copy rectangle
    friend DLLSYM void copy_sub_image(IMAGE *source,  //source image
                                      INT32 xstart,   //start coords
                                      INT32 ystart,
                                      INT32 xext,     //extent to copy
                                      INT32 yext,
                                      IMAGE *dest,    //destination image
                                      INT32 xdest,    //destination coords //shift to match bpp
                                      INT32 ydest,
                                      BOOL8 adjust_grey);

                                 //enlarge rectangle
    friend DLLSYM void enlarge_sub_image(IMAGE *source,       //source image
                                         INT32 xstart,        //scaled coords
                                         INT32 ystart,
                                         IMAGE *dest,         //destination image
                                         INT32 xdest,         //destination coords
                                         INT32 ydest,
                                         INT32 xext,          //extent to copy
                                         INT32 yext,
                                         INT32 scale,         //scale factor
                                         BOOL8 adjust_grey);  //shift to match bpp

                                 //reduce rectangle
    friend DLLSYM void fast_reduce_sub_image(IMAGE *source,       //source image
                                             INT32 xstart,        //start coords
                                             INT32 ystart,
                                             INT32 xext,          //extent to copy
                                             INT32 yext,
                                             IMAGE *dest,         //destination image
                                             INT32 xdest,         //destination coords
                                             INT32 ydest,
                                             INT32 scale,         //scale factor
                                             BOOL8 adjust_grey);  //shift to match bpp

                                 //reduce rectangle
    friend DLLSYM void reduce_sub_image(IMAGE *source,       //source image
                                        INT32 xstart,        //start coords
                                        INT32 ystart,
                                        INT32 xext,          //extent to copy
                                        INT32 yext,
                                        IMAGE *dest,         //destination image
                                        INT32 xdest,         //destination coords
                                        INT32 ydest,
                                        INT32 scale,         //scale factor
                                        BOOL8 adjust_grey);  //shift to match bpp

  private:
    INT8 bpp;                    //bits per pixel
    INT8 bps;                    //bits per sample
    INT8 bytespp;                //per pixel
    INT8 lineskip;               //waste bytes on line
    BOOL8 captured;              //true if buffer captured
    INT8 photo_interp;           //interpretation
    INT32 xsize, ysize;          //size of image
    INT32 res;                   //resolution
    UINT8 *image;                //the actual image
    INT32 xdim;                  //bytes per line
    INT32 bufheight;             //height of buffer
    int fd;                      //open file descriptor
    IMAGE_READER reader;         //reading function
    INT32 ymin;                  //bottom line in mem
    INT32 ymax;                  //top line in mem+1
    INT8 bufread(           //read some more
                 INT32 y);  //ycoord required
};

class DLLSYM IMAGELINE           //one line of image
{
  public:
    UINT8 * pixels;              //image pixels
    INT8 bpp;                    //bits per pixel
    COLOUR_PIX operator[] (      //colour pixels
    INT32 index) {
      return &pixels[index * 3]; //coercion access op
    }

    IMAGELINE() {  //default constructor
      linewidth = 0;
      line = NULL;
      pixels = line;
      bpp = 8;
    }
    void init(                //setup size
              INT32 width) {  //size of line
      if (width <= 0)
        width = MAXIMAGEWIDTH;
      if (width > linewidth) {
        if (line != NULL)
          free_mem(line); 
        linewidth = width;
        line = (UINT8 *) alloc_mem (linewidth * sizeof (UINT8));
      }
      pixels = line;
      bpp = 8;
    }
    ~IMAGELINE () {              //destructor
      if (line != NULL)
        free_mem(line); 
    }

    void set_bpp(  //For colour
                 INT8 new_bpp) {
      if (new_bpp <= 8)
        bpp = 8;
      else
        bpp = 24;
    }

    void init() { 
      if (line == NULL)
        init (0);
      else {
        pixels = line;
        bpp = 8;
      }
    }

    friend void IMAGE::get_line(                     //copies a line
                                INT32 x,             //coord to start at
                                INT32 y,             //line to get
                                INT32 width,         //no of pixels to get
                                IMAGELINE *linebuf,  //line to copy to
                                INT32 margins);      //size of margins
                                 //copies a column
    friend void IMAGE::get_column(INT32 x,             //coord to start at
                                  INT32 y,             //line to get
                                  INT32 height,        //no of pixels to get
                                  IMAGELINE *linebuf,  //line to copy to
                                  INT32 margins);      //size of margins

    friend void IMAGE::put_line(                     //writes a line
                                INT32 x,             //coord to start at
                                INT32 y,             //line to put
                                INT32 width,         //no of pixels to put
                                IMAGELINE *linebuf,  //line to copy from
                                INT32 margins);      //size of margins
                                 //writes a column
    friend void IMAGE::put_column(INT32 x,             //coord to start at
                                  INT32 y,             //line to put
                                  INT32 height,        //no of pixels to put
                                  IMAGELINE *linebuf,  //line to copy from
                                  INT32 margins);      //size of margins

                                 //may just change pointer
    friend void IMAGE::fast_get_line(INT32 x,              //coord to start at
                                     INT32 y,              //line to get
                                     INT32 width,          //no of pixels to get
                                     IMAGELINE *linebuf);  //line to copy to

                                 //may just change pointer
    friend void IMAGE::fast_put_line(INT32 x,              //coord to start at
                                     INT32 y,              //line to get
                                     INT32 width,          //no of pixels to get
                                     IMAGELINE *linebuf);  //line to copy to

  private:
    UINT8 * line;                //local buffer
    INT32 linewidth;             //width of buffer
};
#endif
