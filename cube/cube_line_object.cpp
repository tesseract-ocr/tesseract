/**********************************************************************
 * File:        cube_line_object.cpp
 * Description: Implementation of the Cube Line Object Class
 * Author:    Ahmad Abdulkader
 * Created:   2007
 *
 * (C) Copyright 2008, Google Inc.
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

#include <algorithm>
#include "cube_line_object.h"

namespace tesseract {
CubeLineObject::CubeLineObject(CubeRecoContext *cntxt, Pix *pix) {
  line_pix_ = pix;
  own_pix_ = false;
  processed_ = false;
  cntxt_ = cntxt;
  phrase_cnt_ = 0;
  phrases_ = NULL;
}

CubeLineObject::~CubeLineObject() {
  if (line_pix_ != NULL && own_pix_ == true) {
    pixDestroy(&line_pix_);
    line_pix_ = NULL;
  }

  if (phrases_ != NULL) {
    for (int phrase_idx = 0; phrase_idx < phrase_cnt_; phrase_idx++) {
      if (phrases_[phrase_idx] != NULL) {
        delete phrases_[phrase_idx];
      }
    }

    delete []phrases_;
    phrases_ = NULL;
  }
}

// Recognize the specified pix as one line returning the recognized
bool CubeLineObject::Process() {
  // do nothing if pix had already been processed
  if (processed_) {
    return true;
  }

  // validate data
  if (line_pix_ == NULL || cntxt_ == NULL) {
    return false;
  }

  // create a CharSamp
  CharSamp *char_samp = CubeUtils::CharSampleFromPix(line_pix_, 0, 0,
                                                     line_pix_->w,
                                                     line_pix_->h);
  if (char_samp == NULL) {
    return false;
  }

  // compute connected components.
  int con_comp_cnt = 0;
  ConComp **con_comps = char_samp->FindConComps(&con_comp_cnt,
      cntxt_->Params()->MinConCompSize());
  // no longer need char_samp, delete it
  delete char_samp;
  // no connected components, bail out
  if (con_comp_cnt <= 0 || con_comps == NULL) {
    return false;
  }

  // sort connected components based on reading order
  bool rtl = (cntxt_->ReadingOrder() == tesseract::CubeRecoContext::R2L);
  qsort(con_comps, con_comp_cnt, sizeof(*con_comps), rtl ?
      ConComp::Right2LeftComparer : ConComp::Left2RightComparer);

  // compute work breaking threshold as a ratio of line height
  bool ret_val = false;
  int word_break_threshold = ComputeWordBreakThreshold(con_comp_cnt, con_comps,
                                                       rtl);
  if (word_break_threshold > 0) {
    // over-allocate phrases object buffer
    phrases_ = new CubeObject *[con_comp_cnt];
    // create a phrase if the horizontal distance between two consecutive
    // concomps is higher than threshold
    int start_con_idx = 0;
    int current_phrase_limit = rtl ? con_comps[0]->Left() :
                                     con_comps[0]->Right();

    for (int con_idx = 1; con_idx <= con_comp_cnt; con_idx++) {
      bool create_new_phrase = true;
      // if not at the end, compute the distance between two consecutive
      // concomps
      if (con_idx < con_comp_cnt) {
        int dist = 0;
        if (cntxt_->ReadingOrder() == tesseract::CubeRecoContext::R2L) {
          dist = current_phrase_limit - con_comps[con_idx]->Right();
        } else {
          dist = con_comps[con_idx]->Left() - current_phrase_limit;
        }
        create_new_phrase = (dist > word_break_threshold);
      }

      // create a new phrase
      if (create_new_phrase) {
        // create a phrase corresponding to a range on components
        bool left_most;
        bool right_most;
        CharSamp *phrase_char_samp =
            CharSamp::FromConComps(con_comps, start_con_idx,
                                   con_idx - start_con_idx, NULL,
                                   &left_most, &right_most,
                                   line_pix_->h);
        if (phrase_char_samp == NULL) {
          break;
        }
        phrases_[phrase_cnt_] = new CubeObject(cntxt_, phrase_char_samp);
        // set the ownership of the charsamp to the cube object
        phrases_[phrase_cnt_]->SetCharSampOwnership(true);
        phrase_cnt_++;
        // advance the starting index to the current index
        start_con_idx = con_idx;
        // set the limit of the newly starting phrase (if any)
        if (con_idx < con_comp_cnt) {
          current_phrase_limit = rtl ? con_comps[con_idx]->Left() :
                                       con_comps[con_idx]->Right();
        }
      } else {
        // update the limit of the current phrase
        if (cntxt_->ReadingOrder() == tesseract::CubeRecoContext::R2L) {
          current_phrase_limit = MIN(current_phrase_limit,
                                     con_comps[con_idx]->Left());
        } else {
          current_phrase_limit = MAX(current_phrase_limit,
                                     con_comps[con_idx]->Right());
        }
      }
    }
    ret_val = true;
  }

  // clean-up connected comps
  for (int con_idx = 0; con_idx < con_comp_cnt; con_idx++) {
    delete con_comps[con_idx];
  }
  delete []con_comps;

  // success
  processed_ = true;
  return ret_val;
}

// Compute the least word breaking threshold that is required to produce a
// valid set of phrases. Phrases are validated using the Aspect ratio
// constraints specified in the language specific Params object
int CubeLineObject::ComputeWordBreakThreshold(int con_comp_cnt,
                                              ConComp **con_comps, bool rtl) {
  // initial estimate of word breaking threshold
  int word_break_threshold =
      static_cast<int>(line_pix_->h * cntxt_->Params()->MaxSpaceHeightRatio());
  bool valid = false;

  // compute the resulting words and validate each's aspect ratio
  do {
    // group connected components into words based on breaking threshold
    int start_con_idx = 0;
    int current_phrase_limit = (rtl ? con_comps[0]->Left() :
                                      con_comps[0]->Right());
    int min_x = con_comps[0]->Left();
    int max_x = con_comps[0]->Right();
    int min_y = con_comps[0]->Top();
    int max_y = con_comps[0]->Bottom();
    valid = true;
    for (int con_idx = 1; con_idx <= con_comp_cnt; con_idx++) {
      bool create_new_phrase = true;
      // if not at the end, compute the distance between two consecutive
      // concomps
      if (con_idx < con_comp_cnt) {
        int dist = 0;
        if (rtl) {
          dist = current_phrase_limit - con_comps[con_idx]->Right();
        } else {
          dist = con_comps[con_idx]->Left() - current_phrase_limit;
        }
        create_new_phrase = (dist > word_break_threshold);
      }

      // create a new phrase
      if (create_new_phrase) {
        // check aspect ratio. Break if invalid
        if ((max_x - min_x + 1) >
            (cntxt_->Params()->MaxWordAspectRatio() * (max_y - min_y + 1))) {
          valid = false;
          break;
        }
        // advance the starting index to the current index
        start_con_idx = con_idx;
        // set the limit of the newly starting phrase (if any)
        if (con_idx < con_comp_cnt) {
          current_phrase_limit = rtl ? con_comps[con_idx]->Left() :
                                       con_comps[con_idx]->Right();
          // re-init bounding box
          min_x = con_comps[con_idx]->Left();
          max_x = con_comps[con_idx]->Right();
          min_y = con_comps[con_idx]->Top();
          max_y = con_comps[con_idx]->Bottom();
        }
      } else {
        // update the limit of the current phrase
        if (rtl) {
          current_phrase_limit = MIN(current_phrase_limit,
                                     con_comps[con_idx]->Left());
        } else {
          current_phrase_limit = MAX(current_phrase_limit,
                                     con_comps[con_idx]->Right());
        }
        // update bounding box
        UpdateRange(con_comps[con_idx]->Left(),
                    con_comps[con_idx]->Right(), &min_x, &max_x);
        UpdateRange(con_comps[con_idx]->Top(),
                    con_comps[con_idx]->Bottom(), &min_y, &max_y);
      }
    }

    // return the breaking threshold if all broken word dimensions are valid
    if (valid) {
      return word_break_threshold;
    }

    // decrease the threshold and try again
    word_break_threshold--;
  } while (!valid && word_break_threshold > 0);

  // failed to find a threshold that achieves the target aspect ratio.
  // Just use the default threshold
  return  static_cast<int>(line_pix_->h *
                           cntxt_->Params()->MaxSpaceHeightRatio());
}
}
