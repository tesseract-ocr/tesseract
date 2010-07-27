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

/** 14inch * 400dpi */
#define MAXIMAGEWIDTH   (900*14) 

/** 14inch * 400dpi */
#define MAXIMAGEHEIGHT    (900*14)

#define COMPUTE_IMAGE_XDIM(xsize,bpp) ((bpp)>8 ? ((xsize)*(bpp)+7)/8 :((xsize)+8/(bpp)-1)/(8/(bpp)))

typedef inT8 (*IMAGE_OPENER) (int, inT32 *, inT32 *, inT8 *, inT8 *, inT32 *);
typedef inT8 (*IMAGE_READER) (int, uinT8 *, inT32, inT32, inT8, inT32);
typedef inT8 (*IMAGE_WRITER) (int, uinT8 *, inT32, inT32, inT8, inT8, inT32);

/** array of colours */
typedef uinT8 *COLOUR_PIX;
enum COLOUR_PIX_NAME
{
  RED_PIX,
  GREEN_PIX,
  BLUE_PIX
};

class DLLSYM IMAGELINE;

/** encapsulated image */
class DLLSYM IMAGE
{
  public:
    IMAGE();  //constructor

    ~IMAGE () {                  //destructor
      destroy();  //free memory
    }

    IMAGE & operator= (          //assignment
      IMAGE & source);

    /** 
     * get file header
     * @param name name of image
     */
    inT8 read_header(const char *name);

    /** 
     * get rest of image
     * @param buflines size of buffer
     */
    inT8 read(inT32 buflines);

    /** 
     * write image
     * @param name name to write
     */
    inT8 write(const char *name);

    /** 
     * create blank image
     * @param x x size required
     * @param y y size required
     * @param bits_per_pixel bpp required
     */
    inT8 create(inT32 x,
                inT32 y,
                inT8 bits_per_pixel);

    /** 
     * capture raw image
     * @param pixels pixels to capture
     * @param x x size required
     * @param y y size required
     * @param bits_per_pixel bpp required
     */
    inT8 capture(uinT8 *pixels,
                 inT32 x,
                 inT32 y,
                 inT8 bits_per_pixel);

    /** destroy image */
    void destroy();

    /** 
     * access function
     * @return xsize
     */
    inT32 get_xsize() {
      return xsize;
    }
    /** 
     * access function
     * @return ysize
     */
    inT32 get_ysize() {
      return ysize;
    }
    /** 
     * access function
     * @return bits per pixel
     */
    inT8 get_bpp() {
      return bpp;
    }
    /** 
     * access function
     * @return bits per sample
     */
    inT8 get_bps() {
      return bps;
    }
    /** photo interp */
    BOOL8 white_high() {
      return photo_interp;
    }
    /** access function */
    uinT8 get_white_level() {
      return (1 << bpp) - 1;
    }
    /** get resolution */
    inT32 get_res() {
      return res;
    }
    /** set resolution */
    void set_res(inT32 resolution) {
      res = resolution;
    }
    uinT8 *get_buffer() {
      return image;
    }

    /** 
     * access pixel
     * @param x coord
     * @param y coord
     */
    uinT8 pixel(inT32 x,
                inT32 y);

    /** 
     * get image line
     * @param x coord to start at
     * @param y line to get
     * @param width line to get
     * @param linebuf line to copy to
     */
    void fast_get_line(inT32 x,
                       inT32 y,
                       inT32 width,
                       IMAGELINE *linebuf);

    /** 
     * get image line
     * @param x coord to start at
     * @param y line to get
     * @param width line to get
     * @param linebuf line to copy to
     * @param margins size of margins
     */
    void get_line(inT32 x,
                  inT32 y,
                  inT32 width,
                  IMAGELINE *linebuf,
                  inT32 margins);
    /** 
     * get image column
     * @param x coord to start at
     * @param y line to get
     * @param height number of pixels to get
     * @param linebuf line to copy to
     * @param margins size of margins
     */
    void get_column(inT32 x,
                    inT32 y,
                    inT32 height,
                    IMAGELINE *linebuf,
                    inT32 margins);

    /** 
     * put image line
     * @param x coord to start at
     * @param y line to put
     * @param width number of pixels to put
     * @param linebuf line to copy from
     */
    void fast_put_line(inT32 x,
                       inT32 y,
                       inT32 width,
                       IMAGELINE *linebuf);

    /** 
     * put image line
     * @param x coord to start at
     * @param y line to put
     * @param width number of pixels to put
     * @param linebuf line to copy from
     * @param margins size of margins
     */
    void put_line(inT32 x,
                  inT32 y,
                  inT32 width,
                  IMAGELINE *linebuf,
                  inT32 margins);
    /** 
     * put image column
     * @param x coord to start at
     * @param y line to put
     * @param height number of pixels to put
     * @param linebuf line to copy to
     * @param margins size of margins
     */
    void put_column(inT32 x,
                    inT32 y,
                    inT32 height,
                    IMAGELINE *linebuf,
                    inT32 margins);

    /** 
     * check coordinates
     * @param x xcoord to check
     * @param y ycoord to check
     */
    void check_legal_access(inT32 x,
                            inT32 y,
                            inT32 xext);


    /** Methods to convert image types. Only available if Leptonica is available. */
    Pix* ToPix();
    void FromPix(const Pix* src_pix);

    /** 
     * Map function over window
     * @param win_width Window width
     * @param win_height Window height
     * @param convolve Conv function
     */
    void convolver (
      inT32 win_width,
      inT32 win_height,
      void (*convolve) (
      uinT8 ** pixels,           ///< Of window
      uinT8 bytespp,             ///< 1 or 3 for colour
      inT32 win_wd,              ///< Window width
      inT32 win_ht,              ///< Window height
      uinT8 ret_white_value,     ///< White value to RETURN
      uinT8 * result             ///< Result pixel(s)
      ));

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
    inT8 bpp;                    ///< bits per pixel
    inT8 bps;                    ///< bits per sample
    inT8 bytespp;                ///< per pixel
    inT8 lineskip;               ///< waste bytes on line
    BOOL8 captured;              ///< true if buffer captured
    inT8 photo_interp;           ///< interpretation
    inT32 xsize, ysize;          ///< size of image
    inT32 res;                   ///< resolution
    uinT8 *image;                ///< the actual image
    inT32 xdim;                  ///< bytes per line
    inT32 bufheight;             ///< height of buffer
    int fd;                      ///< open file descriptor
    IMAGE_READER reader;         ///< reading function
    inT32 ymin;                  ///< bottom line in mem
    inT32 ymax;                  ///< top line in mem+1
    /**
     * read some more
     * @param y ycoord required
     */
    inT8 bufread(inT32 y);
};

class DLLSYM IMAGELINE           //one line of image
{
  public:
    uinT8 * pixels;              ///< image pixels
    inT8 bpp;                    ///< bits per pixel
    /** colour pixels */
    COLOUR_PIX operator[] (
    inT32 index) {
      return &pixels[index * 3]; //coercion access op
    }

    /** default constructor */
    IMAGELINE() {
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

    /** For colour */
    void set_bpp(inT8 new_bpp) {
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

    /**
     * copies a line
     * @param x coord to start at
     * @param y line to get
     * @param width no of pixels to get
     * @param linebuf line to copy to
     * @param margins size of margins
     */
    friend void IMAGE::get_line(inT32 x,
                                inT32 y,
                                inT32 width,
                                IMAGELINE *linebuf,
                                inT32 margins);
    /**
     * copies a column
     * @param x coord to start at
     * @param y line to get
     * @param height no of pixels to get
     * @param linebuf line to copy to
     * @param margins size of margins
     */
    friend void IMAGE::get_column(inT32 x,
                                  inT32 y,
                                  inT32 height,
                                  IMAGELINE *linebuf,
                                  inT32 margins);

    /**
     * writes a line
     * @param x coord to start at
     * @param y line to get
     * @param width no of pixels to put
     * @param linebuf line to copy to
     * @param margins size of margins
     */
    friend void IMAGE::put_line(inT32 x,
                                inT32 y,
                                inT32 width,
                                IMAGELINE *linebuf,
                                inT32 margins);

    /**
     * writes a column
     * @param x coord to start at
     * @param y line to get
     * @param height no of pixels to put
     * @param linebuf line to copy to
     * @param margins size of margins
     */
    friend void IMAGE::put_column(inT32 x,
                                  inT32 y,
                                  inT32 height,
                                  IMAGELINE *linebuf,
                                  inT32 margins);

    /**
     * @note may just change pointer
     * @param x coord to start at
     * @param y line to get
     * @param width no of pixels to get
     * @param linebuf line to copy to
     */
    friend void IMAGE::fast_get_line(inT32 x,
                                     inT32 y,
                                     inT32 width,
                                     IMAGELINE *linebuf);

    /**
     * @note may just change pointer
     * @param x coord to start at
     * @param y line to get
     * @param width no of pixels to put
     * @param linebuf line to copy to
     */
    friend void IMAGE::fast_put_line(inT32 x,
                                     inT32 y,
                                     inT32 width,
                                     IMAGELINE *linebuf);

  private:
    uinT8 * line;                ///< local buffer
    inT32 linewidth;             ///< width of buffer
};
#endif
