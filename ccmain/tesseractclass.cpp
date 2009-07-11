///////////////////////////////////////////////////////////////////////
// File:        tesseractclass.cpp
// Description: An instance of Tesseract. For thread safety, *every*
//              global variable goes in here, directly, or indirectly.
// Author:      Ray Smith
// Created:     Fri Mar 07 08:17:01 PST 2008
//
// (C) Copyright 2008, Google Inc.
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

#include "tesseractclass.h"
#include "globals.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifdef HAVE_LIBLEPT
// Include leptonica library only if autoconf (or makefile etc) tell us to.
#include "allheaders.h"
#endif

namespace tesseract {

Tesseract::Tesseract()
  : BOOL_MEMBER(tessedit_resegment_from_boxes, false,
                "Take segmentation and labeling from box file"),
    BOOL_MEMBER(tessedit_train_from_boxes, false,
                "Generate training data from boxed chars"),
    BOOL_MEMBER(tessedit_dump_pageseg_images, false,
               "Dump itermediate images made during page segmentation"),
    // The default for pageseg_mode is the old behaviour, so as not to
    // upset anything that relies on that.
    INT_MEMBER(tessedit_pageseg_mode, 2,
               "Page seg mode: 0=auto, 1=col, 2=block, 3=line, 4=word, 6=char"
               " (Values from PageSegMode enum in baseapi.h)"),
    INT_MEMBER(tessedit_accuracyvspeed, 0,
               "Accuracy V Speed tradeoff: 0 fastest, 100 most accurate"
               " (Values from AccuracyVSpeed enum in baseapi.h)"),
    BOOL_MEMBER(tessedit_train_from_boxes_word_level, false,
                "Generate training data from boxed chars at word level."),
    STRING_MEMBER(tessedit_char_blacklist, "",
                  "Blacklist of chars not to recognize"),
    STRING_MEMBER(tessedit_char_whitelist, "",
                  "Whitelist of chars to recognize"),
    BOOL_MEMBER(global_tessedit_ambigs_training, false,
                "Perform training for ambiguities"),
    pix_binary_(NULL),
    deskew_(1.0f, 0.0f),
    reskew_(1.0f, 0.0f),
    hindi_image_(false) {
}

Tesseract::~Tesseract() {
  Clear();
}

void Tesseract::Clear() {
#ifdef HAVE_LIBLEPT
  if (pix_binary_ != NULL)
    pixDestroy(&pix_binary_);
#endif
 deskew_ = FCOORD(1.0f, 0.0f);
 reskew_ = FCOORD(1.0f, 0.0f);
}

void Tesseract::SetBlackAndWhitelist() {
  // Set the white and blacklists (if any)
  unicharset.set_black_and_whitelist(tessedit_char_blacklist.string(),
                                     tessedit_char_whitelist.string());
}

}  // namespace tesseract
