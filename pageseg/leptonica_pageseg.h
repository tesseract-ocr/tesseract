///////////////////////////////////////////////////////////////////////
// File:        leptonica_pageseg.h
// Description: Leptonica-based page segmenter.
// Author:      Dan Bloomberg
// Created:     Tue Aug 28 08:56:44 PDT 2007
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

#ifndef LEPTONICA_PAGESEG_H
#define LEPTONICA_PAGESEG_H

class Boxa;
class Pix;
class Pixa;

class LeptonicaPageSeg {
 public:
  // GetHalftoneMask()
  //    Input: pixs (input image, assumed to be at 300 - 400 ppi)
  //          &pixht (returns halftone mask; can be NULL)
  //          &baht (returns boxa of halftone mask component b.b.s; can be NULL)
  //          &paht (returns pixa of halftone mask components; can be NULL)
  //           debugflag (set true to write out intermediate images)
  //    Return: true if ok, false on error
  // Note: If there are no halftone regions, all requested data structures
  //       are returned as NULL.  This is not an error.
  static bool GetHalftoneMask(Pix *pixs,
                              Pix **ppixht,
                              Boxa **pbaht,
                              Pixa **ppaht,
                              bool debugflag);

  // GetTextlineMask()
  //    Input: pixs (input image, assumed to be at 300 - 400 ppi)
  //          &pixtm (returns textline mask; can be NULL)
  //          &pixvws (returns vertical whitespace mask; can be NULL)
  //          &batm (returns boxa of textline mask component b.b.s; can be NULL)
  //          &patm (returns pixa of textline mask components; can be NULL)
  //           debugflag (set true to write out intermediate images)
  //    Return: true if ok, false on error
  static bool GetTextlineMask(Pix *pixs,
                              Pix **ppixtm,
                              Pix **ppixvws,
                              Boxa **pbatm,
                              Pixa **ppatm,
                              bool debugflag);

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
  static bool GetTextblockMask(Pix *pixs,
                               Pix **ppixtb,
                               Boxa **pbatb,
                               Pixa **ppatb,
                               bool debugflag);

  // GetAllRegions()
  //    Input: pixs (input image, assumed to be at 300 - 400 ppi)
  //          &pixhm (returns halftone mask; can be NULL)
  //          &pixtm (returns textline mask; can be NULL)
  //          &pixtb (returns textblock mask; can be NULL)
  //           debugflag (set true to write out intermediate images and data)
  //    Return: true if ok, false on error
  // Note: use NULL for input on each mask you don't want.
  static bool GetAllRegions(Pix *pixs,
                            Pix **ppixhm,
                            Pix **ppixtm,
                            Pix **ppixtb,
                            bool debugflag);
};

#endif  // LEPTONICA_PAGESEG_H
