///////////////////////////////////////////////////////////////////////
// File:        leptonica_pageseg_interface.cpp
// Description: Leptonica-based page segmenter interface.
// Author:      Thomas Kielbus
// Created:     Mon Aug 27 10:05:01 PDT 2007
//
// (C) Copyright 2007, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "leptonica_pageseg_interface.h"

#include "leptonica_pageseg.h"
#include "imgs.h"

#ifdef HAVE_LIBLEPT
// Include leptonica library only if autoconf (or makefile etc) tell us to.
#include "allheaders.h"
#endif

#ifdef HAVE_LIBLEPT
// ONLY available if you have Leptonica installed.

// Use the LeptonicaPageSeg class to perform text block detection. Propagates
// the error if any. LeptonicaPageSeg can also return invalid masks; in this
// case, return an empty text block mask.
bool leptonica_pageseg_get_textblock_mask(IMAGE* page_image,
                                          IMAGE* textblock_mask_image) {
  bool success = true;

  // Convert the page IMAGE to a PIX
  PIX* page_pix = page_image->ToPix();


  // Compute the textblock mask PIX
  PIX* textblock_mask_pix = NULL;
  if (LeptonicaPageSeg::GetTextblockMask(page_pix, &textblock_mask_pix,
                                         NULL, NULL, false)) {
    if (pixGetWidth(textblock_mask_pix) != page_image->get_xsize() ||
        pixGetWidth(textblock_mask_pix) != page_image->get_xsize())
      fprintf(stderr, "WARNING: Leptonica's text block mask (%dx%d)"
              " and the original image (%dx%d) differ in size !\n",
              pixGetWidth(textblock_mask_pix), pixGetHeight(textblock_mask_pix),
              page_image->get_xsize(), page_image->get_ysize());

    // Create the resulting mask image
    textblock_mask_image->destroy();
    if (pixGetWidth(textblock_mask_pix) <= 0 ||
        pixGetHeight(textblock_mask_pix) <= 0) {

      // Leptonica failed. Create an empty mask.
      fprintf(stderr, "WARNING: Leptonica's text block mask is invalid.\n");
      textblock_mask_image->create(page_image->get_xsize(),
                                   page_image->get_ysize(), 1);

    } else {
      // Leptonica succeeded. Convert textblock_mask PIX to an IMAGE
      textblock_mask_image->FromPix(textblock_mask_pix);
    }
  } else {
    success = false;
  }
  pixDestroy(&page_pix);
  pixDestroy(&textblock_mask_pix);
  return success;
}

#endif  // HAVE_LIBLEPT
