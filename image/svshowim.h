// Copyright 2006 Google Inc. All Rights Reserved.
// Author: <rays@google.com> (Ray Smith)
//

#ifndef OCR_TESSERACT_SVSHOWIM_H__
#define OCR_TESSERACT_SVSHOWIM_H__

#include "host.h"
#include "img.h"

class ScrollView;

// Override of a tesseract function to display an image in a window.
// This function redirects the display to ScrollView instead of the
// stubbed-out functions in tesseract.
void sv_show_sub_image(IMAGE*    source,         // Image to show.
                       INT32     xstart,         // Bottom-left coords.
                       INT32     ystart,
                       INT32     xext,           // Size of rectangle to show.
                       INT32     yext,
                       ScrollView*    win,            // Window to draw in.
                       INT32     xpos,           // Place to show bottom-left.
                       INT32     ypos);          // Y position.

#endif  // OCR_TESSERACT_SVSHOWIM_H__
