// Copyright 2006 Google Inc. All Rights Reserved.
// Author: <rays@google.com> (Ray Smith)
//

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif
#ifdef HAVE_LIBLEPT
#include "allheaders.h"
#endif
#include "svshowim.h"
#include "scrollview.h"

// Override of a tesseract function to display an image in a window.
// This function redirects the display to ScrollView instead of the
// stubbed-out functions in tesseract.

void sv_show_sub_image(IMAGE*    source,         // Image to show.
                       inT32     xstart,         // Start image coords.
                       inT32     ystart,
                       inT32     xext,           // Size of rectangle to show.
                       inT32     yext,
                       ScrollView*    window,         // Window to draw in.
                       inT32     xpos,           // Place to show bottom-left.
                       inT32     ypos) {         // Y position.
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
