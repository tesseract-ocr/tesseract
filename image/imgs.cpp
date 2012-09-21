/**********************************************************************
 * File:        imgs.c  (Formerly images.c)
 * Description: Main image manipulation functions.
 * Author:      Ray Smith
 * Created:     Thu Jun 07 16:25:02 BST 1990
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

#include          "mfcpch.h"     //precompiled headers
#ifdef _WIN32
#include          <io.h>
#else
#include          <unistd.h>
#endif
#include          <string.h>
#ifdef __UNIX__
#include          <assert.h>
#endif

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "allheaders.h"

#include          "stderr.h"
#include          "tprintf.h"
#include          "imgerrs.h"
#include          "memry.h"
#include          "imgs.h"
#include          "imgunpk.h"

#define FIXED_COLOURS   32       /*number of fixed colours */
#define MIN_4BIT      48         /*4bpp range */
#define MAX_4BIT      64
#define MIN_6BIT      64         /*6bpp range */
#define MAX_6BIT      128
#define BLACK_PIX     0

const uinT8 grey_scales[FIXED_COLOURS] = {
  0, 255, 76, 227, 151, 179, 28, 104,
  149, 72, 215, 67, 53, 44, 156, 137,
  110, 153, 79, 181, 166, 218, 55, 81,
  129, 105, 179, 149, 168, 69, 84, 126
};

#undef EXTERN
#define EXTERN

// Parameter remains truly global, as it is tough to make a member of Image
// and the whole of this code is likely to go away in the future.
EXTERN INT_VAR (image_default_resolution, 300, "Image resolution dpi");

/**********************************************************************
 * IMAGE
 *
 * Contructor for an IMAGE class. Makes the image definitely illegal.
 **********************************************************************/

IMAGE::IMAGE() {  //construct an image
  bpp = 0;                       //all illegal
  fd = -1;
  image = NULL;
  photo_interp = 1;
  res = image_default_resolution;
}


/**********************************************************************
 * IMAGE::operator=
 *
 * Assign an IMAGE to another. The dest becomes the owner of the memory.
 **********************************************************************/

IMAGE & IMAGE::operator= (       //assignment
IMAGE & source                   //source image
) {
  destroy();
  bpp = source.bpp;
  photo_interp = source.photo_interp;
  bps = source.bps;
  bytespp = (bpp + 7) / 8;
  lineskip = source.lineskip;    //copy everything
  captured = source.captured;
  xsize = source.xsize;
  ysize = source.ysize;
  res = source.res;
  image = source.image;
  xdim = source.xdim;
  bufheight = source.bufheight;
  fd = source.fd;
  reader = source.reader;
  ymin = source.ymin;
  ymax = source.ymax;

  source.captured = TRUE;        //source now captured
  source.fd = -1;

  return *this;
}


/**********************************************************************
 * create
 *
 * Create an image (allocate memory) of a specific size and bpp.
 **********************************************************************/

inT8 IMAGE::create(                     //get rest of image
                   inT32 x,             //x size required
                   inT32 y,             //ysize required
                   inT8 bits_per_pixel  //bpp required
                  ) {
  uinT8 *pixels;                 //memory for image

  xdim = check_legal_image_size (x, y, bits_per_pixel);
  if (xdim < 0)
    return -1;
  pixels = (uinT8 *) alloc_big_zeros ((size_t) (xdim * y * sizeof (uinT8)));
  if (pixels == NULL) {
    MEMORY_OUT.error ("IMAGE::create", ABORT, "Size=(%d,%d)", xdim, y);
    return -1;
  }
                                 //allocate to image
  this->capture (pixels, x, y, bits_per_pixel);
  captured = FALSE;
  res = image_default_resolution;
  return 0;                      //success
}


/**********************************************************************
 * destroy
 *
 * Destroy an image, freeing memory and closing any open file.
 **********************************************************************/

void IMAGE::destroy() {  //get rid of image
  if (image != NULL && !captured) {
    free_big_mem(image);
  }
  image = NULL;
  if (fd >= 0) {
    close(fd);
    fd = -1;
  }
  bpp = 0;
}


/**********************************************************************
 * capture
 *
 * Assign a given memory area to an image to use as an image of
 * given size and bpp.
 **********************************************************************/

inT8 IMAGE::capture(                     //get rest of image
                    uinT8 *pixels,       //image memory
                    inT32 x,             //x size required
                    inT32 y,             //ysize required
                    inT8 bits_per_pixel  //bpp required
                   ) {
  destroy();
  xdim = check_legal_image_size (x, y, bits_per_pixel);
  if (xdim < 0)
    return -1;
  xsize = x;
  ysize = y;
  bufheight = y;
  bpp = bits_per_pixel;
  bps = bpp == 24 ? 8 : bpp;
  photo_interp = 1;
  bytespp = (bpp + 7) / 8;
  image = pixels;                //assign image area
  ymin = 0;
  ymax = bufheight;              //read it all
  captured = TRUE;
  res = image_default_resolution;
  return 0;                      //success
}


/**********************************************************************
 * pixel
 *
 * Get a single pixel out of the image.
 **********************************************************************/

uinT8 IMAGE::pixel(          //get rest of image
                   inT32 x,  //x coord
                   inT32 y   //y coord
                  ) {
  if (x < 0)
    x = 0;                       //silently clip
  else if (x >= xsize)
    x = xsize - 1;
  if (y < 0)
    y = 0;
  else if (y >= ysize)
    y = ysize - 1;
  check_legal_access (x, y, 1);
  switch (bpp) {
    case 5:
    case 6:
    case 8:
      return image[(ymax - 1 - y) * xdim + x];
    case 4:
      return bpp4table[image[(ymax - 1 - y) * xdim + x / 2]][x & 1];
    case 2:
      return bpp2table[image[(ymax - 1 - y) * xdim + x / 4]][x & 3];
    case 1:
      return bpp1table[image[(ymax - 1 - y) * xdim + x / 8]][x & 7];
    default:
      tprintf ("Unexpected bits per pixel %d\n", bpp);
      return 0;
  }
}


/**********************************************************************
 * check_legal_image_size
 *
 * Check that the supplied image sizes are legal.  If they are,
 * the xdim is returned, else -1.
 **********************************************************************/

inT32 check_legal_image_size(                     //get rest of image
                             inT32 x,             //x size required
                             inT32 y,             //ysize required
                             inT8 bits_per_pixel  //bpp required
                            ) {
  if (x <= 0 || y <= 0) {
    BADIMAGESIZE.error ("check_legal_image_size", TESSLOG, "(%d,%d)", x, y);
    return -1;                   //failed
  }
  if (bits_per_pixel != 1 && bits_per_pixel != 2 &&
      bits_per_pixel != 4 && bits_per_pixel != 5 &&
      bits_per_pixel != 6 && bits_per_pixel != 8 &&
      bits_per_pixel != 16 && bits_per_pixel != 24 &&
      bits_per_pixel != 32) {
    BADBPP.error ("check_legal_image_size", TESSLOG, "%d", bits_per_pixel);
    return -1;
  }
                                 //bytes per line
  return COMPUTE_IMAGE_XDIM (x, bits_per_pixel);
}


/**********************************************************************
 * copy_sub_image
 *
 * Copy a portion of one image to a portion of another image.
 * If the bpps are different, the position of the most significant
 * bit is preserved.
 **********************************************************************/

DLLSYM void copy_sub_image(                   //copy rectangle
                           IMAGE *source,     //source image
                           inT32 xstart,      //start coords
                           inT32 ystart,
                           inT32 xext,        //extent to copy
                           inT32 yext,
                           IMAGE *dest,       //destination image
                           inT32 xdest,       //destination coords
                           inT32 ydest,
                           BOOL8 adjust_grey  //shift to new bpp
                          ) {
  IMAGELINE copyline;            //copy of line
  uinT8 *copy;                   //source pointer
  inT8 shift;                    //shift factor
  inT32 pixel;                   //pixel index
  inT32 y;                       //line index
  inT32 yoffset;                 //current adjusted offset
  inT32 bytesize;                //no of bytes to copy
  inT32 srcppb;                  //pixels per byte
  BOOL8 aligned;

  if (xstart < 0 || ystart < 0 || xdest < 0 || ydest < 0)
    return;
  if (xext <= 0)
    xext = source->xsize;        //default to all
  if (xext > source->xsize - xstart)
                                 //clip to smallest
      xext = source->xsize - xstart;
  if (xext > dest->xsize - xdest)
    xext = dest->xsize - xdest;
  if (yext <= 0)
    yext = source->ysize;        //default to all
  if (yext > source->ysize - ystart)
                                 //clip to smallest
      yext = source->ysize - ystart;
  if (yext > dest->ysize - ydest)
    yext = dest->ysize - ydest;
  if (xext <= 0 || yext <= 0)
    return;                      //nothing to do

  srcppb = 8 / source->bpp;      //pixels per byte
  if (source->bpp == dest->bpp || !adjust_grey)
    shift = 0;                   //no adjustment
  else {
    shift = source->bps - dest->bps;
    if (shift < 0)
      shift = -shift;            //keep positive
  }
  aligned = source->bpp == dest->bpp;
  if (aligned && srcppb != 0) {
    aligned = xstart % srcppb == 0
      && xdest % srcppb == 0
      && (xext % srcppb == 0 || xdest + xext == dest->xsize);
  }
  for (y = 0; y < yext; y++) {
    if (ystart >= ydest)
      yoffset = y;               //top down
    else
      yoffset = yext - y - 1;    //bottom up
    source->check_legal_access (xstart, ystart + yoffset, xext);
    dest->check_legal_access (xdest, ydest + yoffset, xext);
    if (aligned) {
      bytesize = COMPUTE_IMAGE_XDIM (xext, source->bpp);
      //get bytes per line
      if (srcppb == 0)
                                 //do cheap move
        memmove (dest->image + (dest->ymax - 1 - ydest - yoffset) * dest->xdim + xdest * 3, source->image + (source->ymax - 1 - ystart - yoffset) * source->xdim + xstart * 3, (unsigned) bytesize);
      else
                                 //do cheap move
        memmove (dest->image + (dest->ymax - 1 - ydest - yoffset) * dest->xdim + xdest / srcppb, source->image + (source->ymax - 1 - ystart - yoffset) * source->xdim + xstart / srcppb, (unsigned) bytesize);
    }
    else {
      if (shift == 0) {
        source->fast_get_line (xstart, ystart + yoffset, xext,
          &copyline);
      }
      else if (source->bpp < dest->bpp) {
        source->get_line (xstart, ystart + yoffset, xext, &copyline, 0);
        if (source->bpp <= shift
        && (source->bpp == 1 || source->bpp == 4)) {
          if (source->bpp == 1) {
            for (pixel = 0, copy = copyline.pixels; pixel < xext;
              pixel++, copy++)
            if (*copy)
              *copy = 0xff;
          }
          else {
            for (pixel = 0, copy = copyline.pixels; pixel < xext;
              pixel++, copy++)
                                 //scale up
            *copy = (*copy << shift) | *copy;
          }
        }
        else {
          for (pixel = 0, copy = copyline.pixels; pixel < xext;
            pixel++)
          *copy++ <<= shift;     //scale up
        }
      }
      else {
        source->get_line (xstart, ystart + yoffset, xext, &copyline, 0);
        if (source->bpp == 24) {
          for (pixel = 0, copy = copyline.pixels + 1; pixel < xext;
          pixel++) {
            *copy >>= shift;
            copy += 3;
          }
        }
        else {
          for (pixel = 0, copy = copyline.pixels; pixel < xext;
            pixel++)
          *copy++ >>= shift;     //scale down
        }
      }
      dest->put_line (xdest, ydest + yoffset, xext, &copyline, 0);
    }
  }
}


/**********************************************************************
 * enlarge_sub_image
 *
 * Enlarge a portion of one image to a portion of another image.
 * If the bpps are different, the position of the most significant
 * bit is preserved.
 **********************************************************************/

DLLSYM void enlarge_sub_image(                   //enlarge rectangle
                              IMAGE *source,     //source image
                              inT32 xstart,      //scaled start coords
                              inT32 ystart,
                              IMAGE *dest,       //destination image
                              inT32 xdest,       //dest coords
                              inT32 ydest,
                              inT32 xext,        //destination extent
                              inT32 yext,
                              inT32 scale,       //scale factor
                              BOOL8 adjust_grey  //shift to new bpp
                             ) {
  inT8 shift;                    //shift factor
  uinT8 pixel;                   //current pixel
  inT32 srcext;                  //source extent
  inT32 xoffset;                 //column index
  inT32 yoffset;                 //line index
  inT32 xindex, yindex;          //index in super pixel
  inT32 startxindex;             //initial x index
  inT32 xscale;                  //x scale factor
  uinT8 *src;                    //source pixels
  uinT8 *destpix;                //dest pixels
  IMAGELINE copyline;            //copy of line
  IMAGELINE bigline;             //expanded line

  if (xstart < 0 || ystart < 0 || xdest < 0 || ydest < 0)
    return;

  if (xext <= 0)
    xext = dest->xsize;          //default to all
  if (xext > source->xsize * scale - xstart)
                                 //clip to smallest
    xext = source->xsize * scale - xstart;
  if (xext > dest->xsize - xdest)
    xext = dest->xsize - xdest;
  if (yext <= 0)
    yext = dest->ysize;          //default to all
  if (yext > source->ysize * scale - ystart)
    yext = source->ysize * scale - ystart;
  if (yext > dest->ysize - ydest)
    yext = dest->ysize - ydest;
  if (xext <= 0 || yext <= 0)
    return;                      //nothing to do

  xindex = xstart % scale;       //offset in super pixel
  startxindex = xindex;
  yindex = ystart % scale;
                                 //no of source pixels
  srcext = (xext + xindex + scale - 1) / scale;
  xstart /= scale;               //actual start
  ystart /= scale;
  if (adjust_grey) {
    shift = dest->bps - source->bps;
  }
  else
    shift = 0;                   //no adjustment
  bigline.init (xext * 3);
  bigline.bpp = dest->bpp == 24 ? source->bpp : dest->bpp;

  for (yoffset = 0; yoffset < yext; ystart++) {
    source->check_legal_access (xstart, ystart, srcext);
    dest->check_legal_access (xdest, ydest + yoffset, xext);
    source->fast_get_line (xstart, ystart, srcext, &copyline);
    src = copyline.pixels;
    destpix = bigline.pixels;
    xscale = scale;              //enlargement factor
    if (source->bpp == 24 && dest->bpp == 24) {
      for (xoffset = 0, xindex = startxindex; xoffset < xext;
      src += source->bytespp) {
        xoffset += xscale - xindex;
        if (xoffset > xext)
          xscale -= xoffset - xext;
        for (; xindex < xscale; xindex++) {
          *destpix++ = *src;
          *destpix++ = *(src + 1);
          *destpix++ = *(src + 2);
        }
        xindex = 0;
      }
    }
    else {
      if (source->bpp == 24)
        src++;
      for (xoffset = 0, xindex = startxindex; xoffset < xext;
      src += source->bytespp) {
        xoffset += xscale - xindex;
        if (xoffset > xext)
                                 //clip to dest limit
            xscale -= xoffset - xext;
        if (shift == 0)
          pixel = *src;
        else if (shift > 0)
          pixel = *src << shift;
        else
          pixel = *src >> (-shift);
        for (; xindex < xscale; xindex++)
          *destpix++ = pixel;    //duplicate pixel
        xindex = 0;
      }
    }
    for (; yoffset < yext && yindex < scale; yindex++, yoffset++) {
      dest->put_line (xdest, ydest + yoffset, xext, &bigline, 0);
    }
    yindex = 0;
  }
}


/**********************************************************************
 * fast_reduce_sub_image
 *
 * Reduce a portion of one image to a portion of another image.
 * If the bpps are different, the position of the most significant
 * bit is preserved.
 * This is a fast but dirty version, which simply sub-samples.
 * It does not smooth as it reduces.
 **********************************************************************/

DLLSYM void fast_reduce_sub_image(                   //reduce rectangle
                                  IMAGE *source,     //source image
                                  inT32 xstart,      //start coords
                                  inT32 ystart,
                                  inT32 xext,        //extent to copy
                                  inT32 yext,
                                  IMAGE *dest,       //destination image
                                  inT32 xdest,       //destination coords
                                  inT32 ydest,
                                  inT32 scale,       //reduction factor
                                  BOOL8 adjust_grey  //shift to new bpp
                                 ) {
  inT8 shift;                    //shift factor
  inT32 xfactor;                 //run on x coord
  inT32 divisor;                 //total cell area
  inT32 xindex, yindex;          //into averaging square
  inT32 xcoord;                  //current x coord
  inT32 destext;                 //destination size
  inT32 yoffset;                 //current adjusted offset
  uinT8 *pixel;                  //ptr to source pixels
  inT32 *sums;                   //ptr to sums array
  IMAGELINE copyline;            //copy of line
  inT32 *linesums;               //averaging sums

  if (xstart < 0 || ystart < 0 || xdest < 0 || ydest < 0)
    return;
  if (xext <= 0)
    xext = source->xsize;        //default to all
  if (xext > source->xsize - xstart)
                                 //clip to smallest
      xext = source->xsize - xstart;
  if (xext > (dest->xsize - xdest) * scale)
    xext = (dest->xsize - xdest) * scale;
  if (yext <= 0)
    yext = source->ysize;        //default to all
  if (yext > source->ysize - ystart)
                                 //clip to smallest
      yext = source->ysize - ystart;
  if (yext > (dest->ysize - ydest) * scale)
    yext = (dest->ysize - ydest) * scale;
  if (xext <= 0 || yext <= 0)
    return;                      //nothing to do

  xfactor = xext % scale;        //left overs
  if (xfactor == 0)
    xfactor = scale;
                                 //destination pixels
  destext = (xext + scale - 1) / scale;
  if (adjust_grey)
                                 //shift factor
    shift = dest->bps - source->bps;
  else
    shift = 0;                   //no adjustment
  linesums = new inT32[destext * source->bytespp];

  for (yoffset = 0; yoffset < yext; ydest++) {
    source->check_legal_access (xstart, ystart + yoffset, xext);
    dest->check_legal_access (xdest, ydest, destext);
    for (xindex = destext * source->bytespp - 1; xindex >= 0; xindex--)
      linesums[xindex] = 0;      //zero sums
    for (yindex = 0; yindex < scale
    && ystart + yoffset < source->ysize; yindex += 3) {
      source->fast_get_line (xstart, ystart + yoffset, xext, &copyline);
      pixel = copyline.pixels;   //start of line
      if (source->bpp == 24) {
        for (xcoord = 1, sums = linesums; xcoord < destext;
        xcoord++, sums += 3) {
          for (xindex = 0; xindex < scale; xindex += 2) {
            *sums += *pixel++;
            *(sums + 1) += *pixel++;
            *(sums + 2) += *pixel++;
            pixel += 3;
          }
          if (scale & 1)
            pixel -= 3;          //correct position
        }
        for (xindex = 0; xindex < xfactor; xindex += 2) {
          *sums += *pixel++;
          *(sums + 1) += *pixel++;
          *(sums + 2) += *pixel++;
          pixel += 3;
        }
      }
      else {
        for (xcoord = 1, sums = linesums; xcoord < destext;
        xcoord++, sums++) {
          for (xindex = 0; xindex < scale; xindex += 2) {
            *sums += *pixel;
            pixel += 2;
          }
          if (scale & 1)
            pixel--;             //correct position
        }
        for (xindex = 0; xindex < xfactor; xindex += 2) {
          *sums += *pixel;
          pixel += 2;
        }
      }
      yoffset += 3;              //every 3 lines
    }
    if (yindex > scale)
      yoffset -= yindex - scale; //back on right scale
    copyline.init ();            //set pixels back to array
    copyline.bpp = source->bpp;
    pixel = copyline.pixels;
                                 //pixels in block
    divisor = ((yindex + 2) / 3) * ((scale + 1) / 2);
    if (shift <= 0) {
      divisor <<= (-shift);      //do greyscale correction
      for (sums = linesums, xindex = (destext - 1) * source->bytespp;
        xindex > 0; xindex--)
                                 //turn to destination value
      *pixel++ = (uinT8) (*sums++ / divisor);
      for (xindex = source->bytespp; xindex > 0; xindex--)
        *pixel++ = *sums++
          / (((yindex + 2) / 3) * ((xfactor + 1) / 2) << (-shift));
      //lastone different
    }
    else {
      for (sums = linesums, xindex = (destext - 1) * source->bytespp;
        xindex > 0; xindex--)
      *pixel++ = (uinT8) ((*sums++ << shift) / divisor);
      //destination value
      for (xindex = source->bytespp; xindex > 0; xindex--)
                                 //last one different
        *pixel++ = (*(sums++) << shift) / (((yindex + 2) / 3) * ((xfactor + 1) / 2));
    }
                                 //put in destination
    dest->put_line (xdest, ydest, destext, &copyline, 0);
  }
  delete [] linesums;
}


/**********************************************************************
 * reduce_sub_image
 *
 * Reduce a portion of one image to a portion of another image.
 * If the bpps are different, the position of the most significant
 * bit is preserved.
 **********************************************************************/

DLLSYM void reduce_sub_image(                   //reduce rectangle
                             IMAGE *source,     //source image
                             inT32 xstart,      //start coords
                             inT32 ystart,
                             inT32 xext,        //extent to copy
                             inT32 yext,
                             IMAGE *dest,       //destination image
                             inT32 xdest,       //destination coords
                             inT32 ydest,
                             inT32 scale,       //reduction factor
                             BOOL8 adjust_grey  //shift to new bpp
                            ) {
  inT8 shift;                    //shift factor
  inT32 xfactor;                 //run on x coord
  inT32 divisor;                 //total cell area
  inT32 div2;                    //total cell area divided by 2
  inT32 xindex, yindex;          //into averaging square
  inT32 xcoord;                  //current x coord
  inT32 destext;                 //destination size
  inT32 yoffset;                 //current adjusted offset
  uinT8 *pixel;                  //ptr to source pixels
  inT32 *sums;                   //ptr to sums array
  IMAGELINE copyline;            //copy of line
  inT32 *linesums;               //averaging sums

  if (xstart < 0 || ystart < 0 || xdest < 0 || ydest < 0)
    return;
  if (xext <= 0)
    xext = source->xsize;        //default to all
  if (xext > source->xsize - xstart)
                                 //clip to smallest
      xext = source->xsize - xstart;
  if (xext > (dest->xsize - xdest) * scale)
    xext = (dest->xsize - xdest) * scale;
  if (yext <= 0)
    yext = source->ysize;        //default to all
  if (yext > source->ysize - ystart)
                                 //clip to smallest
      yext = source->ysize - ystart;
  if (yext > (dest->ysize - ydest) * scale)
    yext = (dest->ysize - ydest) * scale;
  if (xext <= 0 || yext <= 0)
    return;                      //nothing to do

  xfactor = xext % scale;        //left overs
  if (xfactor == 0)
    xfactor = scale;
                                 //destination pixels
  destext = (xext + scale - 1) / scale;
  if (adjust_grey)
                                 //shift factor
    shift = dest->bps - source->bps;
  else
    shift = 0;                   //no adjustment
  linesums = new inT32[destext * source->bytespp];

  for (yoffset = 0; yoffset < yext; ydest++) {
    source->check_legal_access (xstart, ystart + yoffset, xext);
    dest->check_legal_access (xdest, ydest, destext);
    for (xindex = 0; xindex < (destext) * source->bytespp; xindex++)
      linesums[xindex] = 0;      //zero sums
    for (yindex = 0; yindex < scale && ystart + yoffset < source->ysize;
    yindex++) {
      source->fast_get_line (xstart, ystart + yoffset, xext, &copyline);
      pixel = copyline.pixels;   //start of line
      if (source->bpp == 24) {
        for (xcoord = 1, sums = linesums; xcoord < destext;
        xcoord++, sums += 3) {
          for (xindex = 0; xindex < scale; xindex++) {
            *sums += *pixel++;
            *(sums + 1) += *pixel++;
            *(sums + 2) += *pixel++;
          }
        }
        for (xindex = 0; xindex < xfactor; xindex++) {
          *sums += *pixel++;
          *(sums + 1) += *pixel++;
          *(sums + 2) += *pixel++;
        }
      }
      else {
        for (xcoord = 1, sums = linesums; xcoord < destext;
        xcoord++, sums++) {
          for (xindex = 0; xindex < scale; xindex++)
            *sums += *pixel++;
        }
        for (xindex = 0; xindex < xfactor; xindex++)
          *sums += *pixel++;
      }
      yoffset++;                 //next line
    }
    copyline.init ();            //set pixels back to array
    copyline.set_bpp (source->bpp);
    pixel = copyline.pixels;
    divisor = yindex * scale;
    if (divisor == 0) {
      tprintf
        ("Impossible:divisor=0!, yindex=%d, scale=%d, yoffset=%d,yext=%d\n",
        yindex, scale, yoffset, yext);
      break;
    }
    if (shift <= 0) {
      divisor <<= (-shift);      //do greyscale correction
      div2 = divisor / 2;
      for (sums = linesums, xindex = (destext - 1) * source->bytespp;
        xindex > 0; xindex--)
      *pixel++ = (uinT8) ((div2 + *sums++) / divisor);
      //turn to destination value
      div2 = (yindex * xfactor << (-shift)) / 2;
      for (xindex = source->bytespp; xindex > 0; xindex--)
        *pixel++ =
          (uinT8) ((div2 + *sums++) / (yindex * xfactor << (-shift)));
      //lastone different
    }
    else {
      div2 = divisor / 2;
      for (sums = linesums, xindex = (destext - 1) * source->bytespp;
        xindex > 0; xindex--)
      *pixel++ = (uinT8) ((div2 + (*sums++ << shift)) / divisor);
      //destination value
      div2 = (yindex * xfactor) / 2;
      for (xindex = source->bytespp; xindex > 0; xindex--)
        *pixel++ =
          (uinT8) ((div2 + (*sums++ << shift)) / (yindex * xfactor));
      //last one different
    }
                                 //put in destination
    dest->put_line (xdest, ydest, destext, &copyline, 0);
  }
  delete [] linesums;
}


/**********************************************************************
 * invert_image
 *
 * Invert the given image (the slow way.)
 **********************************************************************/

DLLSYM void invert_image(              /*invert the image */
                         IMAGE *image  /*image ot invert */
                        ) {
  uinT8 mask;                    //bit mask
  uinT8 bytespp;                 //bytes per pixel
  inT32 xsize, ysize;            /*size of image */
  inT32 xindex, yindex;          /*index into image */
  uinT8 *pixel;                  /*current pixel */
  IMAGELINE line;                /*line of image */

  bytespp = image->get_bpp () == 24 ? 3 : 1;
  xsize = image->get_xsize ();   /*find sizes */
  ysize = image->get_ysize ();
                                 //pixel mask
  mask = (1 << image->get_bpp ()) - 1;
                                 /*do each line */
  for (yindex = ysize - 1; yindex >= 0; yindex--) {
    image->fast_get_line (0, yindex, xsize, &line);
    for (pixel = line.pixels, xindex = xsize * bytespp; xindex > 0;
      xindex--) {
      *pixel = (*pixel) ^ mask;  //invert image only
      ++pixel;
    }
                                 /*put it back */
    image->fast_put_line (0, yindex, xsize, &line);
  }
}


/**********************************************************************
 * bias_sub_image
 *
 * Add a constant to a portion of an image.
 **********************************************************************/

DLLSYM void bias_sub_image(                //bias rectangle
                           IMAGE *source,  //source image
                           inT32 xstart,   //start coords
                           inT32 ystart,
                           inT32 xext,     //extent to copy
                           inT32 yext,
                           uinT8 bias      //number to add
                          ) {
  IMAGELINE copyline;            //copy of line
  uinT8 *copy;                   //source pointer
  inT32 pixel;                   //pixel index
  inT32 y;                       //line index
  uinT8 bytespp;                 //bytes per pixel

  if (xstart < 0 || ystart < 0)
    return;
  if (xext <= 0)
    xext = source->get_xsize (); //default to all
  if (xext > source->get_xsize () - xstart)
                                 //clip to smallest
    xext = source->get_xsize () - xstart;
  if (yext <= 0)
    yext = source->get_ysize (); //default to all
  if (yext > source->get_ysize () - ystart)
                                 //clip to smallest
    yext = source->get_ysize () - ystart;
  if (xext <= 0 || yext <= 0)
    return;                      //nothing to do

  bytespp = source->get_bpp () == 24 ? 3 : 1;
  for (y = 0; y < yext; y++) {
    source->check_legal_access (xstart, ystart + y, xext);
    source->fast_get_line (xstart, ystart + y, xext, &copyline);
    for (pixel = xext * bytespp, copy = copyline.pixels; pixel > 0;
      pixel--, copy++)
    *copy += bias;               //add bias

    source->fast_put_line (xstart, ystart + y, xext, &copyline);
  }
}


/**********************************************************************
 * starbase_to_normal
 *
 * Copy a portion of one image to a portion of another image.
 * This function maps the colour tables used on the screen to
 * greyscale values in the way "normally" expected.
 **********************************************************************/

DLLSYM void starbase_to_normal(                     //copy rectangle
                               IMAGE *source,       //source image
                               inT32 xstart,        //start coords
                               inT32 ystart,
                               inT32 xext,          //extent to copy
                               inT32 yext,
                               IMAGE *dest,         //destination image
                               inT32 xdest,         //destination coords
                               inT32 ydest,
                               BOOL8 preserve_grey  //shift to new bpp
                              ) {
  IMAGELINE copyline;            //copy of line
  uinT8 *copy;                   //source pointer
  inT8 shift4;                   //shift factor
  inT8 shift6;                   //shift factor
  inT8 colour_shift;             //shift of colours
  uinT8 white_level;             //dest white value
  inT32 pixel;                   //pixel index
  inT32 y;                       //line index
  inT32 yoffset;                 //current adjusted offset
  inT8 srcppb;                   //pixels per byte

  if (xstart < 0 || ystart < 0 || xdest < 0 || ydest < 0)
    return;
  if (xext <= 0)
    xext = source->get_xsize (); //default to all
  if (xext > source->get_xsize () - xstart)
                                 //clip to smallest
    xext = source->get_xsize () - xstart;
  if (xext > dest->get_xsize () - xdest)
    xext = dest->get_xsize () - xdest;
  if (yext <= 0)
    yext = source->get_ysize (); //default to all
  if (yext > source->get_ysize () - ystart)
                                 //clip to smallest
    yext = source->get_ysize () - ystart;
  if (yext > dest->get_ysize () - ydest)
    yext = dest->get_ysize () - ydest;
  if (xext <= 0 || yext <= 0)
    return;                      //nothing to do

                                 //pixels per byte
  srcppb = 8 / source->get_bpp ();
  shift4 = 4 - dest->get_bpp (); //for different bpps
  shift6 = 6 - dest->get_bpp ();
                                 //for grey preserve
  colour_shift = 8 - dest->get_bpp ();
  white_level = dest->get_white_level ();
  for (y = 0; y < yext; y++) {
    if (ystart >= ydest)
      yoffset = y;               //top down
    else
      yoffset = yext - y - 1;    //bottom up
    source->check_legal_access (xstart, ystart + yoffset, xext);
    dest->check_legal_access (xdest, ydest + yoffset, xext);
    source->get_line (xstart, ystart + yoffset, xext, &copyline, 0);
    for (pixel = 0, copy = copyline.pixels; pixel < xext; pixel++) {
      if (*copy < FIXED_COLOURS && preserve_grey)
        *copy = grey_scales[*copy] >> colour_shift;
      else if (*copy < FIXED_COLOURS) {
        if (*copy == BLACK_PIX)
          *copy = white_level;   //black->white
        else
          *copy = 0;             //others->black
      }
      else if (*copy >= MIN_4BIT && *copy < MAX_4BIT) {
        if (shift4 < 0)
          *copy = (*copy - MIN_4BIT) << (-shift4);
        else
          *copy = (*copy - MIN_4BIT) >> shift4;
      }
      else if (*copy >= MIN_6BIT && *copy < MAX_6BIT) {
        if (shift6 < 0)
          *copy = (*copy - MIN_6BIT) << (-shift6);
        else
          *copy = (*copy - MIN_6BIT) >> shift6;
      }
      else {
        *copy = white_level;     //white the rest
      }
      copy++;
    }
    dest->put_line (xdest, ydest + yoffset, xext, &copyline, 0);
  }
}


/**********************************************************************
 * fast_get_line
 *
 * Get a line of image into the supplied image line buffer.
 * The image is converted to 8bpp by simple assignment.
 * If the image is aleady 8 or 6bpp, no copy is done and a pointer
 * to the correct image section is put in the line buffer.
 **********************************************************************/

void IMAGE::fast_get_line(                    //get image line
                          inT32 x,            //coord to start at
                          inT32 y,            //line to get
                          inT32 width,        //no of pixels to get
                          IMAGELINE *linebuf  //line to copy to
                         ) {
  if (width > 0 && bpp > 4) {
    check_legal_access(x, y, width);
                                 //get pointer only
    linebuf->pixels = image + xdim * (ymax - 1 - y) + x * bytespp;
  }
  else
                                 //just copy it
    this->get_line (x, y, width, linebuf, 0);
  linebuf->bpp = bpp;
}


/**********************************************************************
 * get_line
 *
 * Get a line of image into the supplied image line buffer.
 * The image is converted to 8bpp by simple assignment.
 **********************************************************************/

void IMAGE::get_line(                     //get image line
                     inT32 x,             //coord to start at
                     inT32 y,             //line to get
                     inT32 width,         //no of pixels to get
                     IMAGELINE *linebuf,  //line to copy to
                     inT32 margins        //size of margins
                    ) {
  uinT8 *src;                    // source pointer
  uinT8 *dest;                   // destination pointer
  const uinT8 *unpacksrc;        // unpacking pointer
  inT8 bit;                      // bit index
  inT8 pixperbyte;               // pixels per byte
  uinT8 white;                   // white colour
  inT32 pixel;                   // pixel index

  this->check_legal_access (x, y, width);
  if (width > xsize - x)
    width = xsize - x;           //clip to image
  width *= bytespp;
  linebuf->init (width + margins * bytespp * 2);
  linebuf->bpp = bpp;
                                 //start of line
  src = image + xdim * (ymax - 1 - y);
  dest = linebuf->line;          //destination line
  linebuf->pixels = dest;
  white = (1 << bpp) - 1;        //max value of pixel
  for (pixel = margins * bytespp; pixel > 0; pixel--) {
    *dest++ = white;             //margins are white
  }
  if (width > 0) {
    if (bpp > 4) {
      src += x;                  //offset
                                 //easy way
      memmove (dest, src, (unsigned) width);
    }
    else if (bpp == 4) {
      src += x / 2;              //offset on line
      if (x & 1) {
                                 //get coded nibble
        *dest++ = bpp4table[*src++][1];
        width--;
      }
      while (width >= 2) {
                                 //get coded bits
        unpacksrc = bpp4table[*src++];
        *dest++ = *unpacksrc++;
        *dest++ = *unpacksrc++;  //copy nibbles
        width -= 2;
      }
      if (width) {
                                 //get coded nibble
        *dest++ = bpp4table[*src++][0];
      }
    }
    else if (bpp == 2) {
      pixperbyte = 4;
      src += x / 4;              //offset on line
      bit = (inT8) (x % 4);      //offset in byte
      width += bit;
      while (width > 0) {        //until all done
        if (width < pixperbyte)
                                 //less on last byte
          pixperbyte = (inT8) width;
                                 //get coded bits
        unpacksrc = &bpp2table[*src++][bit];
        for (; bit < pixperbyte; bit++)
          *dest++ = *unpacksrc++;//copy bytes
        width -= pixperbyte;
        bit = 0;
      }
    }
    else {
      pixperbyte = 8;
      src += x / 8;              //offset on line
      bit = (inT8) (x % 8);      //offset in byte
      width += bit;
      while (width > 0) {        //until all done
        if (width < pixperbyte)
                                 //less on last byte
          pixperbyte = (inT8) width;
                                 //get coded bits
        unpacksrc = &bpp1table[*src++][bit];
        for (; bit < pixperbyte; bit++)
          *dest++ = *unpacksrc++;//copy bytes
        width -= pixperbyte;
        bit = 0;
      }
    }
  }
  for (pixel = margins * bytespp; pixel > 0; pixel--) {
    *dest++ = white;             //margins are white
  }
}


/**********************************************************************
 * get_column
 *
 * Get a column of image into the supplied image line buffer.
 * The image is converted to 8bpp by simple assignment.
 **********************************************************************/

void IMAGE::get_column(                     //get image column
                       inT32 x,             //coord to start at
                       inT32 y,             //line to get
                       inT32 height,        //no of pixels to get
                       IMAGELINE *linebuf,  //line to copy to
                       inT32 margins        //size of margins
                      ) {
  uinT8 *src;                    //source pointer
  uinT8 *dest;                   //destination pointer
  inT8 bit;                      //bit index
  inT8 pixperbyte;               //pixels per byte
  uinT8 white;                   //white colour
  inT32 pixel;                   //pixel index

                                 //test coords
  this->check_legal_access (x, y, 1);
                                 //test coords
  this->check_legal_access (x, y + height - 1, 1);
  if (height > ysize - y)
    height = ysize - y;          //clip to image
  linebuf->init (height * bytespp + margins * bytespp * 2);
                                 //start of line
  src = image + xdim * (ymax - 1 - y);
  dest = linebuf->line;          //destination line
  linebuf->pixels = dest;
  white = (1 << bpp) - 1;        //max value of pixel
  for (pixel = margins * bytespp; pixel > 0; pixel--) {
    *dest++ = white;             //margins are white
  }
  if (height > 0) {
    if (bpp == 24) {
      src += x * bytespp;        //offset
      for (; height > 0; --height) {
        *dest++ = *src;          //copy bytes
        *dest++ = *(src + 1);
        *dest++ = *(src + 2);
        src -= xdim;
      }
    }
    else if (bpp > 4) {
      src += x;
      for (; height > 0; --height) {
        *dest++ = *src;          //copy bytes
        src -= xdim;
      }
    }
    else if (bpp == 4) {
      src += x / 2;              //offset on line
      if (x & 1) {
        for (; height > 0; --height) {
                                 //get coded nibble
          *dest++ = bpp4table[*src][1];
          src -= xdim;
        }
      }
      else {
        for (; height > 0; --height) {
                                 //get coded nibble
          *dest++ = bpp4table[*src][0];
          src -= xdim;
        }
      }
    }
    else if (bpp == 2) {
      pixperbyte = 4;
      src += x / 4;              //offset on line
      bit = (inT8) (x % 4);      //offset in byte
      for (; height > 0; --height) {
                                 //get coded bits
        *dest++ = bpp2table[*src][bit];
        src -= xdim;
      }
    }
    else {
      pixperbyte = 8;
      src += x / 8;              //offset on line
      bit = (inT8) (x % 8);      //offset in byte
      for (; height > 0; --height) {
                                 //get coded bits
        *dest++ = bpp1table[*src][bit];
        src -= xdim;
      }
    }
  }
  for (pixel = margins * bytespp; pixel > 0; pixel--) {
    *dest++ = white;             //margins are white
  }
}


/**********************************************************************
 * fast_put_line
 *
 * Put a line buffer back into the image.
 * If the line buffer merely points back into the image, nothing is done.
 * Otherwise, put_line is used to copy the line back.
 **********************************************************************/

void IMAGE::fast_put_line(                    //put image line
                          inT32 x,            //coord to start at
                          inT32 y,            //line to get
                          inT32 width,        //no of pixels to put
                          IMAGELINE *linebuf  //line to copy to
                         ) {
  if (width > 0 && (bpp <= 4 || linebuf->pixels == linebuf->line))
                                 //just copy it
    put_line (x, y, width, linebuf, 0);
}


/**********************************************************************
 * put_line
 *
 * Put the supplied line buffer into the image.
 * The image is converted from 8bpp by simple assignment.
 **********************************************************************/

void IMAGE::put_line(                     //put image line
                     inT32 x,             //coord to start at
                     inT32 y,             //line to get
                     inT32 width,         //no of pixels to get
                     IMAGELINE *linebuf,  //line to copy to
                     inT32 margins        //margins in buffer
                    ) {
  uinT8 *src;                    //source pointer
  uinT8 *dest;                   //destination pointer
  inT8 bit;                      //bit index
  uinT8 pixel;                   //collected bits
  inT8 pixperbyte;               //pixels in a byte
  inT8 bytesperpix;              //in source

  this->check_legal_access (x, y, width);
  if (width > xsize - x)
    width = xsize - x;           //clip to image
  if (width <= 0)
    return;                      //nothing to do
                                 //source line
  src = linebuf->pixels + margins;
                                 //start of line
  dest = image + xdim * (ymax - 1 - y);

  if (linebuf->bpp == 24) {
    src++;
    bytesperpix = 3;
  }
  else
    bytesperpix = 1;
  if (bpp == 24 && linebuf->bpp == 24) {
    dest += x * bytespp;
    width *= bytespp;
    memmove (dest, src - 1, (unsigned) width);
  }
  else if (bpp == 24) {
    src--;
    dest += x * bytespp;
    while (width > 0) {
      pixel = *src++;
      *dest++ = pixel;
      *dest++ = pixel;
      *dest++ = pixel;
      width--;
    }
  }
  else if (bpp > 4) {
    dest += x;                   //offset
    if (linebuf->bpp == 24) {
      while (width > 0) {
        *dest++ = *src;
        src += 3;
        width--;
      }
    }
    else
                                 //easy way
      memmove (dest, src, (unsigned) width);
  }
  else if (bpp == 4) {
    dest += x / 2;               //offset on line
    if (x & 1) {
      *dest &= 0xf0;             //clean odd byte
      *dest++ |= *src & 0x0f;    //and copy it
      src += bytesperpix;
      width--;
    }
    while (width >= 2) {
      pixel = *src << 4;         //left pixel
      src += bytesperpix;
      pixel |= *src & 0x0f;      //right pixel
      src += bytesperpix;
      *dest++ = pixel;
      width -= 2;
    }
    if (width) {
      *dest &= 0x0f;             //clean odd byte
      *dest |= *src << 4;
    }
  }
  else if (bpp == 2) {
    pixperbyte = 4;
    dest += x / 4;               //offset on line
    bit = (inT8) (x % 4);        //offset in byte
    width += bit;
    pixel = *dest >> (8 - bit - bit);
    while (width >= 4) {         //until all done
      for (; bit < 4; bit++) {
        pixel <<= 2;             //make space for new one
        pixel |= *src & 3;
        src += bytesperpix;
      }
      *dest++ = pixel;           //new pixel
      width -= 4;
      bit = 0;
    }
    if (width > 0) {             //until all done
      for (bit = 0; bit < width; bit++) {
        pixel <<= 2;             //make space for new one
        pixel |= *src & 3;
        src += bytesperpix;
      }
      pixel <<= (8 - bit - bit); //shift rest
                                 //keep trainling bits
      pixel |= *dest & ((1 << (8 - bit - bit)) - 1);
      *dest++ = pixel;           //new pixel
    }
  }
  else {
    pixperbyte = 8;
    dest += x / 8;               //offset on line
    bit = (inT8) (x % 8);        //offset in byte
    width += bit;
    pixel = *dest >> (8 - bit);
    while (width >= 8) {         //until all done
      for (; bit < 8; bit++) {
        pixel <<= 1;             //make space for new one
        pixel |= *src & 1;
        src += bytesperpix;
      }
      *dest++ = pixel;           //new pixel
      width -= 8;
      bit = 0;
    }
    width -= bit;
    if (width > 0) {             //until all done
      while (width > 0) {
        pixel <<= 1;             //make space for new one
        pixel |= *src & 1;
        src += bytesperpix;
        bit++;
        width--;
      }
      pixel <<= (8 - bit);       //shift rest
                                 //keep trainling bits
      pixel |= *dest & ((1 << (8 - bit)) - 1);
      *dest++ = pixel;           //new pixel
    }
  }
}


/**********************************************************************
 * put_column
 *
 * Put the supplied column buffer into the image.
 * The image is converted from 8bpp by simple assignment.
 **********************************************************************/

void IMAGE::put_column(                     //put image column
                       inT32 x,             //coord to start at
                       inT32 y,             //line to get
                       inT32 height,        //no of pixels to get
                       IMAGELINE *linebuf,  //line to copy to
                       inT32 margins        //margins in buffer
                      ) {
  uinT8 *src;                    //source pointer
  uinT8 *dest;                   //destination pointer
  inT8 bit;                      //bit index
  uinT8 pixel;                   //collected bits
  inT8 bytesperpix;              //in source

  this->check_legal_access (x, y, 1);
  this->check_legal_access (x, y + height - 1, 1);
  if (height > ysize - y)
    height = ysize - y;          //clip to image
  if (height <= 0)
    return;                      //nothing to do
                                 //source line
  src = linebuf->pixels + margins;
                                 //start of line
  dest = image + xdim * (ymax - 1 - y);

  if (linebuf->bpp == 24) {
    src++;
    bytesperpix = 3;
  }
  else
    bytesperpix = 1;

  if (bpp == 24 && linebuf->bpp == 24) {
    dest += x * bytesperpix;
    src--;
    for (; height > 0; --height) {
      *dest = *src++;
      *(dest + 1) = *src++;
      *(dest + 2) = *src++;
      dest -= xdim;
    }
  }
  else if (bpp == 24) {
    src--;
    dest += x * bytesperpix;
    for (; height > 0; --height) {
      pixel = *src++;
      *dest = pixel;
      *(dest + 1) = pixel;
      *(dest + 2) = pixel;
      dest -= xdim;
    }
  }
  else if (bpp > 4) {
    dest += x;                   //offset
    for (; height > 0; --height) {
      *dest = *src;
      src += bytesperpix;
      dest -= xdim;
    }
  }
  else if (bpp == 4) {
    dest += x / 2;               //offset on line
    if (x & 1) {
      for (; height > 0; --height) {
        *dest &= 0xf0;           //clean odd byte
        *dest |= *src & 0x0f;    //and copy it
        src += bytesperpix;
        dest -= xdim;
      }
    }
    else {
      for (; height > 0; --height) {
        *dest &= 0x0f;           //clean odd byte
        *dest |= *src << 4;
        src += bytesperpix;
        dest -= xdim;
      }
    }
  }
  else if (bpp == 2) {
    dest += x / 4;               //offset on line
    bit = (inT8) (x % 4);        //offset in byte
    bit = 6 - bit - bit;         //bit shift
    pixel = ~(3 << bit);         //mask
    for (; height > 0; --height) {
                                 //change 2 bits
      *dest = (*dest & pixel) | ((*src & 3) << bit);
      src += bytesperpix;
      dest -= xdim;
    }
  }
  else {
    dest += x / 8;               //offset on line
    bit = (inT8) (x % 8);        //offset in byte
    bit = 7 - bit;
    pixel = ~(1 << bit);
    for (; height > 0; --height) {
                                 //change 1 bit
      *dest = (*dest & pixel) | ((*src & 1) << bit);
      src += bytesperpix;
      dest -= xdim;
    }
  }
}


/**********************************************************************
 * check_legal_access
 *
 * Check that x,y are within the bounds of the image.
 * Call bufread if necessary to get the image into memory.
 **********************************************************************/

void IMAGE::check_legal_access(            //check coords are legal
                               inT32 x,    //coords to check
                               inT32 y,
                               inT32 xext  //xextent
                              ) {
  if (x < 0 || x >= xsize || y < 0 || y >= ysize || x + xext > xsize)
    BADIMAGECOORDS.error ("IMAGE::check_legal_access",
      ABORT, "(%d+%d,%d)", x, xext, y);
  if (y < ymin || y >= ymax)
    BADIMAGESEEK.error ("IMAGE::check_legal_access", ABORT, "(%d,%d)", x, y);
}

/**********************************************************************
 * ToPix
 *
 * Make a Pix from this image.
 **********************************************************************/
Pix* IMAGE::ToPix() {
  int width = this->get_xsize();
  int height = this->get_ysize();
  int bpp = this->get_bpp();
  Pix* pix = pixCreate(width, height, bpp == 24 ? 32 : bpp);
  l_uint32* data = pixGetData(pix);
  IMAGELINE line;
  if (bpp == 24) {
    line.init(width * 3);
    line.set_bpp(24);
  } else {
    line.init(width);
  }
  switch (bpp) {
  case 1:
    for (int y = height - 1 ; y >= 0; --y) {
      this->get_line(0, y, width, &line, 0);
      for (int x = 0; x < width; ++x) {
        if (line.pixels[x])
          CLEAR_DATA_BIT(data, x);
        else
          SET_DATA_BIT(data, x);
      }
      data += pixGetWpl(pix);
    }
    break;

  case 8:
    // Greyscale just copies the bytes in the right order.
    for (int y = height - 1 ; y >= 0; --y) {
      this->get_line(0, y, width, &line, 0);
      for (int x = 0; x < width; ++x)
        SET_DATA_BYTE(data, x, line.pixels[x]);
      data += pixGetWpl(pix);
    }
    break;

  case 24:
    // Put the colors in the correct places in the line buffer.
    for (int y = height - 1 ; y >= 0; --y) {
      this->get_line(0, y, width, &line, 0);
      for (int x = 0; x < width; ++x, ++data) {
        SET_DATA_BYTE(data, COLOR_RED, line[x][RED_PIX]);
        SET_DATA_BYTE(data, COLOR_GREEN, line[x][GREEN_PIX]);
        SET_DATA_BYTE(data, COLOR_BLUE, line[x][BLUE_PIX]);
      }
    }
    break;

  default:
    tprintf("Cannot convert image to Pix with bpp = %d\n", bpp);
  }
  return pix;
}

/**********************************************************************
 * FromPix
 *
 * Copy from the given Pix into this image.
 **********************************************************************/
void IMAGE::FromPix(const Pix* src_pix) {
  // Leptonica doesn't const its inputs, but we don't change the input.
  Pix* pix = const_cast<Pix*>(src_pix);
  Pix* destroy_this_pix = NULL;

  int depth = pixGetDepth(pix);
  if (depth > 1 && depth < 8) {
    // Convert funny depths to 8 bit.
    destroy_this_pix = pixConvertTo8(pix, false);
    pix = destroy_this_pix;
    depth = pixGetDepth(pix);
  }
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  const l_uint32* data = pixGetData(pix);
  this->create(width, height, depth == 32 ? 24 : depth);
  // For each line in the image, fill the IMAGELINE class and put it into the
  // destination image. Note that Tesseract stores images with the
  // bottom at y=0 and 0 is always black in grey and binary.
  IMAGELINE line;
  if (depth == 32) {
    line.init(width * 3);
    line.set_bpp(24);
  } else {
    line.init(width);
  }
  switch (depth) {
  case 1:
    // Binary images just flip the data bit.
    for (int y = height - 1 ; y >= 0; --y) {
      for (int x = 0; x < width; ++x)
        line.pixels[x] = GET_DATA_BIT((void *)data, x) ^ 1;
      this->put_line(0, y, width, &line, 0);
      data += pixGetWpl(pix);
    }
    break;

  case 8:
    // Greyscale just copies the bytes in the right order.
    for (int y = height - 1 ; y >= 0; --y) {
      for (int x = 0; x < width; ++x)
        line.pixels[x] = GET_DATA_BYTE((void *)data, x);
      this->put_line(0, y, width, &line, 0);
      data += pixGetWpl(pix);
    }
    break;

  case 32:
    // Put the colors in the correct places in the line buffer.
    for (int y = height - 1 ; y >= 0; --y) {
      for (int x = 0; x < width; ++x, ++data) {
        line[x][RED_PIX] = GET_DATA_BYTE((void *)data, COLOR_RED);
        line[x][GREEN_PIX] = GET_DATA_BYTE((void *)data, COLOR_GREEN);
        line[x][BLUE_PIX] = GET_DATA_BYTE((void *)data, COLOR_BLUE);
      }
      this->put_line(0, y, width, &line, 0);
    }
    break;

  default:
    tprintf("Cannot convert Pix to image with bpp = %d\n", depth);
  }
  if (destroy_this_pix != NULL)
    pixDestroy(&destroy_this_pix);
}

/*************************************************************************
 * convolver()
 *
 * Calls the specified function for each pixel in the image, passing in an m x n
 * window of the image, centred on the pixel.  The convolution function returns
 * a new value for the pixel, based on the window.
 *
 * At the edges of the image, the window is padded to white pixels.
 *************************************************************************/

void
IMAGE::convolver (               //Map fn over window
inT32 win_width,                 //Window width
inT32 win_height,                //Window height
void (*convolve) (               //Conv Function
uinT8 ** pixels,                 //Of window
uinT8 bytespp,                   //1 or 3 for colour
inT32 win_wd,                    //Window width
inT32 win_ht,                    //Window height
uinT8 ret_white_value,           //White value to RETURN
uinT8 * result)                  //Ptr to result pix
) {
  IMAGELINE new_row;             //Replacement pixels
  IMAGELINE *old_rows;           //Rows being processed
  inT32 oldest_imline;           //Next imline to replace
  uinT8 **window;                //ptrs to pixel rows
  uinT8 **winmax;                //ptrs to pixel rows
  uinT8 **win;                   //ptrs to pixel rows
  inT32 current_row;             //Row being calculated
  inT32 current_col;             //Col being calculated
  inT32 row = 0;                 //Next row to get

  inT32 i, j;
  uinT8 *pix;
  uinT8 *max;
  inT32 xmargin = win_width / 2;
  inT32 ymargin = win_height / 2;
  uinT8 white = get_white_level ();
  const uinT8 max_white = 255;
  float white_scale = (float) 255 / get_white_level ();

  if (((win_width % 2) == 0) ||
    ((win_height % 2) == 0) ||
    (win_height < 3) ||
    (win_width < 3) || (win_height > ysize / 2) || (win_width > xsize / 2))
    BADWINDOW.error ("IMAGE::convolver",
      ABORT, "(%d x %d)", win_width, win_height);

  new_row.init (xsize * bytespp);
  new_row.set_bpp (bpp);
  old_rows = new IMAGELINE[win_height];
  for (i = 0; i < win_height; i++) {
    old_rows[i].init ((xsize + 2 * xmargin) * bytespp);
    old_rows[i].set_bpp (bpp);
  }

  window = (uinT8 **) alloc_mem (win_height * sizeof (uinT8 *));
  winmax = window + win_height;

  /* Make bottom border */
  for (oldest_imline = 0; oldest_imline < ymargin; oldest_imline++) {
    pix = old_rows[oldest_imline].pixels;
    max = pix + (xsize + 2 * xmargin) * bytespp;
    while (pix < max)
      *pix++ = max_white;
  }
  /* Initialise remaining rows but one*/
  for (; oldest_imline < win_height - 1; oldest_imline++) {
    get_line (0, row++, xsize, &old_rows[oldest_imline], xmargin);
    if (max_white != white) {
      pix = old_rows[oldest_imline].pixels;
      max = pix + (xsize + 2 * xmargin) * bytespp;
      while (pix < max) {
        *pix = (uinT8) (*pix * white_scale);
        ++pix;
      }
    }
  }

  /* Image Processing */

  for (current_row = 0; current_row < ysize;) {
    /* Get next row and re-initialise window array */
    if (row < ysize) {
      get_line (0, row++, xsize, &old_rows[oldest_imline], xmargin);
      if (max_white != white) {
        pix = old_rows[oldest_imline].pixels;
        max = pix + (xsize + 2 * xmargin) * bytespp;
        while (pix < max) {
          *pix = (uinT8) (*pix * white_scale);
          ++pix;
        }
      }
    }
    else {
      pix = old_rows[oldest_imline].pixels;
      max = pix + (xsize + 2 * xmargin) * bytespp;
      while (pix < max)
        *pix++ = max_white;
    }
    oldest_imline++;
    if (oldest_imline >= win_height)
      oldest_imline = 0;

    /* Process line */
    pix = new_row.pixels;
    for (current_col = 0; current_col < xsize;) {
      /* Set up window ptrs */
      if (current_col == 0) {
        j = oldest_imline;
        for (i = 0; i < win_height; i++) {
          window[i] = old_rows[j++].pixels;
          if (j >= win_height)
            j = 0;
        }
      }
      else {
        for (win = window; win < winmax; (*win++) += bytespp);
        //Move along rows
      }

      convolve(window, bytespp, win_width, win_height, white, pix);
      pix += bytespp;
      current_col++;
    }

    put_line (0, current_row, xsize, &new_row, 0);
    new_row.init ();
    new_row.set_bpp (bpp);
    current_row++;
  }
}
