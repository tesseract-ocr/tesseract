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

struct Pix;

#define MAXIMAGEWIDTH   (900*14) /*14inch * 400dpi */
                                 /*14inch * 400dpi */
#define MAXIMAGEHEIGHT    (900*14)

#define COMPUTE_IMAGE_XDIM(xsize,bpp) ((bpp)>8 ? ((xsize)*(bpp)+7)/8 :((xsize)+8/(bpp)-1)/(8/(bpp)))

typedef inT8 (*IMAGE_OPENER) (int, inT32 *, inT32 *, inT8 *, inT8 *, inT32 *);
typedef inT8 (*IMAGE_READER) (int, uinT8 *, inT32, inT32, inT8, inT32);
typedef inT8 (*IMAGE_WRITER) (int, uinT8 *, inT32, inT32, inT8, inT8, inT32);

typedef uinT8 *COLOUR_PIX;       //array of colours
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

    inT8 read_header(                    //get file header
                     const char *name);  //name of image

    inT8 read(                  //get rest of image
              inT32 buflines);  //size of buffer

    inT8 write(                    //write image
               const char *name);  //name to write

    inT8 create(                       //create blank image
                inT32 x,               //x size required
                inT32 y,               //ysize required
                inT8 bits_per_pixel);  //bpp required

    inT8 capture(                       //capture raw image
                 uinT8 *pixels,         //pixels to capture
                 inT32 x,               //x size required
                 inT32 y,               //ysize required
                 inT8 bits_per_pixel);  //bpp required

    void destroy();  //destroy image

    inT32 get_xsize() {
      return xsize;
    }
    //access function
    inT32 get_ysize() {
      return ysize;
    }
    //access function
    inT8 get_bpp() {
      return bpp;
    }                            //access function
    inT8 get_bps() {
      return bps;
    }                            //bits per sample
    BOOL8 white_high() {  //photo interp
      return photo_interp;
    }
    uinT8 get_white_level() {  //access function
      return (1 << bpp) - 1;
    }
    inT32 get_res() {
      return res;
    }                            //access function
    void set_res(  //set resolution
                 inT32 resolution) {
      res = resolution;
    }
    uinT8 *get_buffer() {
      return image;
    }
    //access function

    uinT8 pixel(           //access pixel
                inT32 x,   //x coord
                inT32 y);  //y coord

    void fast_get_line(                      //get image line
                       inT32 x,              //coord to start at
                       inT32 y,              //line to get
                       inT32 width,          //no of pixels to get
                       IMAGELINE *linebuf);  //line to copy to

    void get_line(                     //get image line
                  inT32 x,             //coord to start at
                  inT32 y,             //line to get
                  inT32 width,         //no of pixels to get
                  IMAGELINE *linebuf,  //line to copy to
                  inT32 margins);      //size of margins
    void get_column(                     //get image column
                    inT32 x,             //coord to start at
                    inT32 y,             //line to get
                    inT32 height,        //no of pixels to get
                    IMAGELINE *linebuf,  //line to copy to
                    inT32 margins);      //size of margins

    void fast_put_line(                      //put image line
                       inT32 x,              //coord to start at
                       inT32 y,              //line to put
                       inT32 width,          //no of pixels to put
                       IMAGELINE *linebuf);  //line to copy from

    void put_line(                     //put image line
                  inT32 x,             //coord to start at
                  inT32 y,             //line to put
                  inT32 width,         //no of pixels to put
                  IMAGELINE *linebuf,  //line to copy from
                  inT32 margins);      //size of margins
    void put_column(                     //put image column
                    inT32 x,             //coord to start at
                    inT32 y,             //line to put
                    inT32 height,        //no of pixels to put
                    IMAGELINE *linebuf,  //line to copy to
                    inT32 margins);      //size of margins

    void check_legal_access(              //check coords
                            inT32 x,      //xcoord to check
                            inT32 y,
                            inT32 xext);  //ycoord to check


    // Methods to convert image types. Only available if Leptonica is available.
    Pix* ToPix();
    void FromPix(const Pix* src_pix);

    void convolver (             //Map fn over window
      inT32 win_width,           //Window width
      inT32 win_height,          //Window height
      void (*convolve) (         //Conv Function
      uinT8 ** pixels,           //Of window
      uinT8 bytespp,             //1 or 3 for colour
      inT32 win_wd,              //Window width
      inT32 win_ht,              //Window height
      uinT8 ret_white_value,     //White value to RETURN
      uinT8 * result));          //Result pixel(s)

                                 //copy rectangle
    friend DLLSYM void copy_sub_image(IMAGE *source,  //source image
                                      inT32 xstart,   //start coords
                                      inT32 ystart,
                                      inT32 xext,     //extent to copy
                                      inT32 yext,
                                      IMAGE *dest,    //destination image
                                      inT32 xdest,    //destination coords //shift to match bpp
                                      inT32 ydest,
                                      BOOL8 adjust_grey);

                                 //enlarge rectangle
    friend DLLSYM void enlarge_sub_image(IMAGE *source,       //source image
                                         inT32 xstart,        //scaled coords
                                         inT32 ystart,
                                         IMAGE *dest,         //destination image
                                         inT32 xdest,         //destination coords
                                         inT32 ydest,
                                         inT32 xext,          //extent to copy
                                         inT32 yext,
                                         inT32 scale,         //scale factor
                                         BOOL8 adjust_grey);  //shift to match bpp

                                 //reduce rectangle
    friend DLLSYM void fast_reduce_sub_image(IMAGE *source,       //source image
                                             inT32 xstart,        //start coords
                                             inT32 ystart,
                                             inT32 xext,          //extent to copy
                                             inT32 yext,
                                             IMAGE *dest,         //destination image
                                             inT32 xdest,         //destination coords
                                             inT32 ydest,
                                             inT32 scale,         //scale factor
                                             BOOL8 adjust_grey);  //shift to match bpp

                                 //reduce rectangle
    friend DLLSYM void reduce_sub_image(IMAGE *source,       //source image
                                        inT32 xstart,        //start coords
                                        inT32 ystart,
                                        inT32 xext,          //extent to copy
                                        inT32 yext,
                                        IMAGE *dest,         //destination image
                                        inT32 xdest,         //destination coords
                                        inT32 ydest,
                                        inT32 scale,         //scale factor
                                        BOOL8 adjust_grey);  //shift to match bpp

  private:
    inT8 bpp;                    //bits per pixel
    inT8 bps;                    //bits per sample
    inT8 bytespp;                //per pixel
    inT8 lineskip;               //waste bytes on line
    BOOL8 captured;              //true if buffer captured
    inT8 photo_interp;           //interpretation
    inT32 xsize, ysize;          //size of image
    inT32 res;                   //resolution
    uinT8 *image;                //the actual image
    inT32 xdim;                  //bytes per line
    inT32 bufheight;             //height of buffer
    int fd;                      //open file descriptor
    IMAGE_READER reader;         //reading function
    inT32 ymin;                  //bottom line in mem
    inT32 ymax;                  //top line in mem+1
    inT8 bufread(           //read some more
                 inT32 y);  //ycoord required
};

class DLLSYM IMAGELINE           //one line of image
{
  public:
    uinT8 * pixels;              //image pixels
    inT8 bpp;                    //bits per pixel
    COLOUR_PIX operator[] (      //colour pixels
    inT32 index) {
      return &pixels[index * 3]; //coercion access op
    }

    IMAGELINE() {  //default constructor
      linewidth = 0;
      line = NULL;
      pixels = line;
      bpp = 8;
    }
    void init(                //setup size
              inT32 width) {  //size of line
      if (width <= 0)
        width = MAXIMAGEWIDTH;
      if (width > linewidth) {
        if (line != NULL)
          free_mem(line);
        linewidth = width;
        line = (uinT8 *) alloc_mem (linewidth * sizeof (uinT8));
      }
      pixels = line;
      bpp = 8;
    }
    ~IMAGELINE () {              //destructor
      if (line != NULL)
        free_mem(line);
    }

    void set_bpp(  //For colour
                 inT8 new_bpp) {
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
                                inT32 x,             //coord to start at
                                inT32 y,             //line to get
                                inT32 width,         //no of pixels to get
                                IMAGELINE *linebuf,  //line to copy to
                                inT32 margins);      //size of margins
                                 //copies a column
    friend void IMAGE::get_column(inT32 x,             //coord to start at
                                  inT32 y,             //line to get
                                  inT32 height,        //no of pixels to get
                                  IMAGELINE *linebuf,  //line to copy to
                                  inT32 margins);      //size of margins

    friend void IMAGE::put_line(                     //writes a line
                                inT32 x,             //coord to start at
                                inT32 y,             //line to put
                                inT32 width,         //no of pixels to put
                                IMAGELINE *linebuf,  //line to copy from
                                inT32 margins);      //size of margins
                                 //writes a column
    friend void IMAGE::put_column(inT32 x,             //coord to start at
                                  inT32 y,             //line to put
                                  inT32 height,        //no of pixels to put
                                  IMAGELINE *linebuf,  //line to copy from
                                  inT32 margins);      //size of margins

                                 //may just change pointer
    friend void IMAGE::fast_get_line(inT32 x,              //coord to start at
                                     inT32 y,              //line to get
                                     inT32 width,          //no of pixels to get
                                     IMAGELINE *linebuf);  //line to copy to

                                 //may just change pointer
    friend void IMAGE::fast_put_line(inT32 x,              //coord to start at
                                     inT32 y,              //line to get
                                     inT32 width,          //no of pixels to get
                                     IMAGELINE *linebuf);  //line to copy to

  private:
    uinT8 * line;                //local buffer
    inT32 linewidth;             //width of buffer
};
#endif
