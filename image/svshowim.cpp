// Copyright 2006 Google Inc. All Rights Reserved.
// Author: <rays@google.com> (Ray Smith)
//

#include "svshowim.h"
#include "scrollview.h"
// The jpeg library still has INT32 as long, which is no good for 64 bit.
#ifdef HAVE_LIBLEPT
#define INT32 WRONGINT32
#include "allheaders.h"
#undef INT32
#endif

// Override of a tesseract function to display an image in a window.
// This function redirects the display to ScrollView instead of the
// stubbed-out functions in tesseract.

void sv_show_sub_image(IMAGE*    source,         // Image to show.
                       INT32     xstart,         // Start image coords.
                       INT32     ystart,
                       INT32     xext,           // Size of rectangle to show.
                       INT32     yext,
                       ScrollView*    window,         // Window to draw in.
                       INT32     xpos,           // Place to show bottom-left.
                       INT32     ypos) {         // Y position.
#ifdef HAVE_LIBLEPT
  Pix* pix;
  if (xstart != 0 || ystart != 0 ||
      xext != source->get_xsize() || yext != source->get_ysize()) { 
    IMAGE sub_im;
    sub_im.create(xext, yext, source->get_bpp());
    copy_sub_image(source, xstart, ystart, xext, yext, &sub_im, 0, 0, false);
    pix = sub_im.ToPix();
  } else {
    pix = source->ToPix();
  }
  window->Image(pix, xpos, window->TranslateYCoordinate(yext) + ypos);
  pixDestroy(&pix);
#endif
}
