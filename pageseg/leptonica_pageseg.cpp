///////////////////////////////////////////////////////////////////////
// File:        leptonica_pageseg.cpp
// Description: Leptonica-based page segmenter.
// Author:      Dan Bloomberg
// Created:     Tue Aug 28 08:56:43 PDT 2007
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

#include "leptonica_pageseg.h"

#ifdef HAVE_LIBLEPT
// Include leptonica library only if autoconf (or makefile etc) tell us to.
#include "allheaders.h"
#endif

#ifdef HAVE_LIBLEPT
// ONLY available if you have Leptonica installed.

//    class LeptonicaPageSeg
//
//       Region segmentation
//           bool    GetHalftoneMask()
//           bool    GetTextlineMask()
//           bool    GetTextblockMask()
//
//       Top-level (for testing/debugging)
//           bool    GetAllRegions()
//
//

//------------------------------------------------------------------
//                      Region segmentation
//------------------------------------------------------------------
// GetHalftoneMask()
//    Input: pixs (input image, assumed to be at 300 - 400 ppi)
//          &pixht (returns halftone mask; can be NULL)
//          &baht (returns boxa of halftone mask component b.b.s; can be NULL)
//          &paht (returns pixa of halftone mask components; can be NULL)
//           debugflag (set true to write out intermediate images)
//    Return: true if ok, false on error
// Note: If there are no halftone regions, all requested data structures
//       are returned as NULL.  This is not an error.
bool LeptonicaPageSeg::GetHalftoneMask(Pix *pixs,
                              Pix **ppixht,
                              Boxa **pbaht,
                              Pixa **ppaht,
                              bool debugflag) {
  if (!pixs) {
    fprintf(stderr, "pixs not defined\n");
    return false;
  }

  int32 debug = debugflag ? 1 : 0;

  // 2x reduce, to 150 - 200 ppi
  Pix *pixr = pixReduceRankBinaryCascade(pixs, 1, 0, 0, 0);
  pixDisplayWrite(pixr, debug);

  // Get the halftone mask
  Pix *pixht2 = pixGenHalftoneMask(pixr, NULL, NULL, debug);
  pixDestroy(&pixr);
  if (!pixht2) {
    if (debug)
      printf("No halftone image parts found\n");
    if (ppixht) *ppixht = NULL;
    if (pbaht) *pbaht = NULL;
    if (ppaht) *ppaht = NULL;
    return true;
  } else {
    if (debug)
      printf("Halftone image parts found\n");
  }

  Pix *pixht = pixExpandReplicate(pixht2, 2);
  pixDisplayWrite(pixht, debug);
  pixDestroy(&pixht2);

  // Fill to capture pixels near the mask edges that were missed
  Pix *pixt = pixSeedfillBinary(NULL, pixht, pixs, 8);
  pixOr(pixht, pixht, pixt);
  pixDestroy(&pixt);

  if (ppaht) {
    Boxa *boxa = pixConnComp(pixht, ppaht, 4);
    if (pbaht) {
      *pbaht = boxa;
    } else {
      boxaDestroy(&boxa);
    }
  } else if (pbaht) {
    *pbaht = pixConnComp(pixht, NULL, 4);
  }

  if (ppixht) {
    *ppixht =pixht;
  } else {
    pixDestroy(&pixht);
  }

  return true;
}


// GetTextlineMask()
//    Input: pixs (input image, assumed to be at 300 - 400 ppi)
//          &pixtm (returns textline mask; can be NULL)
//          &pixvws (returns vertical whitespace mask; can be NULL)
//          &batm (returns boxa of textline mask component b.b.s; can be NULL)
//          &patm (returns pixa of textline mask components; can be NULL)
//           debugflag (set true to write out intermediate images)
//    Return: true if ok, false on error
bool LeptonicaPageSeg::GetTextlineMask(Pix *pixs,
                              Pix **ppixtm,
                              Pix **ppixvws,
                              Boxa **pbatm,
                              Pixa **ppatm,
                              bool debugflag) {
  if (!pixs) {
    fprintf(stderr, "pixs not defined\n");
    return false;
  }

  int32 debug = debugflag ? 1 : 0;

  // 2x reduce, to 150 - 200 ppi
  Pix *pixr = pixReduceRankBinaryCascade(pixs, 1, 0, 0, 0);
  pixDisplayWrite(pixr, debug);

  // Remove the halftone pixels from the image
  Pix *pixtext;
  Pix *pixht2 = pixGenHalftoneMask(pixr, &pixtext, NULL, debug);
  pixDestroy(&pixr);
  pixDestroy(&pixht2);

  // Get the textline mask at full res
  Pix *pixvws;
  Pix *pixtm2 = pixGenTextlineMask(pixtext, &pixvws, NULL, debug);
  Pix *pixt = pixExpandReplicate(pixtm2, 2);
  pixDestroy(&pixtext);
  pixDestroy(&pixtm2);

  // Small dilation to capture pixels near the mask edges that were missed
  // Do not use filling here, because the result is then used to find
  // textblocks, and a mistake here gets propagated.
  Pix *pixtm = pixDilateBrick(NULL, pixt, 3, 3);
  pixDestroy(&pixt);
  pixDisplayWrite(pixtm, debug);

  if (ppatm) {
    Boxa *boxa = pixConnComp(pixtm, ppatm, 4);
    if (pbatm) {
      *pbatm = boxa;
    } else {
      boxaDestroy(&boxa);
    }
  } else if (pbatm) {
    *pbatm = pixConnComp(pixtm, NULL, 4);
  }

  if (ppixtm) {
    *ppixtm =pixtm;
  } else {
    pixDestroy(&pixtm);
  }
  if (ppixvws) {
    *ppixvws =pixvws;
  } else {
    pixDestroy(&pixvws);
  }

  return true;
}


// GetTextblockMask()
//    Input: pixs (input image, assumed to be at 300 - 400 ppi)
//          &pixtb (returns textblock mask; can be NULL)
//          &batb (returns boxa of textblock mask component b.b; can be NULL)
//          &patb (returns pixa of textblock mask components; can be NULL)
//           debugflag (set true to write out intermediate images)
//    Return: true if ok, false on error
// Notes:
//    To obtain a set of polylines of the outer borders of each of the
//    textblock regions, use pixGetOuterBordersPtaa().
bool LeptonicaPageSeg::GetTextblockMask(Pix *pixs,
                               Pix **ppixtb,
                               Boxa **pbatb,
                               Pixa **ppatb,
                               bool debugflag) {
  if (!pixs) {
    fprintf(stderr, "pixs not defined\n");
    return false;
  }

  int32 debug = debugflag ? 1 : 0;

  // Get the textline mask at 2x reduction
  Pix *pixtm, *pixvws;
  GetTextlineMask(pixs, &pixtm, &pixvws, NULL, NULL, debugflag);
  Pix *pixtm2 = pixReduceRankBinaryCascade(pixtm, 1, 0, 0, 0);
  pixDestroy(&pixtm);

  // Get the textblock mask
  Pix *pixtb2 = pixGenTextblockMask(pixtm2, pixvws, debug);
  Pix *pixt = pixExpandReplicate(pixtb2, 2);
  pixDestroy(&pixtm2);
  pixDestroy(&pixtb2);
  pixDestroy(&pixvws);

  // Dilate to capture pixels near the mask edges that were missed
  Pix *pixtb = pixDilateBrick(NULL, pixt, 3, 3);
  pixDestroy(&pixt);
  pixDisplayWrite(pixtb, debug);

  if (ppatb) {
    Boxa *boxa = pixConnComp(pixtb, ppatb, 4);
    if (pbatb) {
      *pbatb = boxa;
    } else {
      boxaDestroy(&boxa);
    }
  } else if (pbatb) {
    *pbatb = pixConnComp(pixtb, NULL, 4);
  }

  if (ppixtb) {
    *ppixtb = pixtb;
  } else {
    pixDestroy(&pixtb);
  }

  return true;
}


//------------------------------------------------------------------
//                 Top-level (for testing/debugging)
//------------------------------------------------------------------
// GetAllRegions()
//    Input: pixs (input image, assumed to be at 300 - 400 ppi)
//          &pixhm (returns halftone mask; can be NULL)
//          &pixtm (returns textline mask; can be NULL)
//          &pixtb (returns textblock mask; can be NULL)
//           debugflag (set true to write out intermediate images and data)
//    Return: true if ok, false on error
// Note: use NULL for input on each mask you don't want.
bool LeptonicaPageSeg::GetAllRegions(Pix *pixs,
                            Pix **ppixhm,
                            Pix **ppixtm,
                            Pix **ppixtb,
                            bool debugflag) {
  if (!pixs || (pixGetDepth(pixs) != 1)) {
    fprintf(stderr, "pixs not read or not 1 bpp\n");
    return 1;
  }

  int32 w, h;
  pixGetDimensions(pixs, &w, &h, NULL);
  int32 debug = debugflag ? 1 : 0;

  // Segment the page
  Boxa *batm = NULL;
  Boxa *batb = NULL;
  Pixa *patm = NULL;
  Pixa *patb = NULL;
  Pix *pixhm = NULL;
  Pix *pixtm = NULL;
  Pix *pixtb = NULL;

  startTimer();
  LeptonicaPageSeg::GetHalftoneMask(pixs, &pixhm, NULL, NULL, false);
  if (debug)
    printf("Halftone segmentation time: %f sec\n", stopTimer());

  startTimer();
  LeptonicaPageSeg::GetTextlineMask(pixs, &pixtm, NULL, &batm, &patm, false);
  if (debug)
    printf("Textline segmentation time: %f sec\n", stopTimer());

  startTimer();
  LeptonicaPageSeg::GetTextblockMask(pixs, &pixtb, &batb, &patb, debugflag);
  if (debug)
    printf("Textblock segmentation time: %f sec\n", stopTimer());

  // Display the textlines
  if (debug) {
    Pix *pixt = pixaDisplayRandomCmap(patm, w, h);
    pixcmapResetColor(pixGetColormap(pixt), 0, 255, 255, 255);  // white bg
    pixDisplay(pixt, 100, 100);
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);
  }

  // Display the textblocks
  if (debug) {
    Pix *pixt = pixaDisplayRandomCmap(patb, w, h);
    pixcmapResetColor(pixGetColormap(pixt), 0, 255, 255, 255);
    pixDisplay(pixt, 100, 100);
    pixDisplayWrite(pixt, 1);
    pixDestroy(&pixt);
  }

  // Identify the outlines of each textblock
  if (debug) {
    Ptaa *ptaa = pixGetOuterBordersPtaa(pixtb);
    Pix *pixt = pixRenderRandomCmapPtaa(pixtb, ptaa, 8, 1);
    PixColormap *cmap = pixGetColormap(pixt);
    pixcmapResetColor(cmap, 0, 130, 130, 130);
    pixDisplayWrite(pixt, debug);
    pixDestroy(&pixt);
    ptaaWrite("junk_ptaa_outlines.ptaa", ptaa, 1);
    ptaaDestroy(&ptaa);
  }

  // Save b.b. for textblocks
  if (debug) {
    Boxa *ba1 = boxaSelectBySize(batb, 3, 3, L_SELECT_IF_BOTH,
                                 L_SELECT_IF_GTE, NULL);
    boxaWrite("junk_textblock.boxa", ba1);
    boxaDestroy(&ba1);
  }

  if (ppixhm) {
    *ppixhm = pixhm;
  } else {
    pixDestroy(&pixhm);
  }
  if (ppixtm) {
    *ppixtm = pixtm;
  } else {
    pixDestroy(&pixtm);
  }
  if (ppixtb) {
    *ppixtb = pixtb;
  } else {
    pixDestroy(&pixtb);
  }

  boxaDestroy(&batm);
  boxaDestroy(&batb);
  pixaDestroy(&patm);
  pixaDestroy(&patb);
  pixDestroy(&pixs);
  return true;
}

#endif  // HAVE_LIBLEPT
