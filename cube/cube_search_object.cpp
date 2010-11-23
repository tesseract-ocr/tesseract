/**********************************************************************
 * File:        cube_search_object.cpp
 * Description: Implementation of the Cube Search Object Class
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

#include "cube_search_object.h"
#include "cube_utils.h"
#include "ndminx.h"

namespace tesseract {

const bool CubeSearchObject::kUseCroppedChars = true;

CubeSearchObject::CubeSearchObject(CubeRecoContext *cntxt, CharSamp *samp)
    : SearchObject(cntxt) {
  init_ = false;
  reco_cache_ = NULL;
  samp_cache_ = NULL;
  segments_ = NULL;
  segment_cnt_ = 0;
  samp_ = samp;
  left_ = 0;
  itop_ = 0;
  space_cost_ = NULL;
  no_space_cost_ = NULL;
  wid_ = samp_->Width();
  hgt_ = samp_->Height();
  max_seg_per_char_ = cntxt_->Params()->MaxSegPerChar();
  rtl_ = (cntxt_->ReadingOrder() == CubeRecoContext::R2L);
  min_spc_gap_ =
      static_cast<int>(hgt_ * cntxt_->Params()->MinSpaceHeightRatio());
  max_spc_gap_ =
      static_cast<int>(hgt_ * cntxt_->Params()->MaxSpaceHeightRatio());
}

CubeSearchObject::~CubeSearchObject() {
  Cleanup();
}

// Cleanup
void CubeSearchObject::Cleanup() {
  // delete Recognition Cache
  if (reco_cache_) {
    for (int strt_seg = 0; strt_seg < segment_cnt_; strt_seg++)  {
      if (reco_cache_[strt_seg]) {
        for (int end_seg = 0; end_seg < segment_cnt_; end_seg++)  {
          if (reco_cache_[strt_seg][end_seg]) {
            delete reco_cache_[strt_seg][end_seg];
          }
        }
        delete []reco_cache_[strt_seg];
      }
    }
    delete []reco_cache_;
    reco_cache_ = NULL;
  }

  // delete CharSamp Cache
  if (samp_cache_) {
    for (int strt_seg = 0; strt_seg < segment_cnt_; strt_seg++)  {
      if (samp_cache_[strt_seg]) {
        for (int end_seg = 0; end_seg < segment_cnt_; end_seg++)  {
          if (samp_cache_[strt_seg][end_seg]) {
            delete samp_cache_[strt_seg][end_seg];
          }
        }
        delete []samp_cache_[strt_seg];
      }
    }
    delete []samp_cache_;
    samp_cache_ = NULL;
  }

  // delete segment list
  if (segments_) {
    for (int seg = 0; seg < segment_cnt_; seg++) {
      if (segments_[seg]) {
        delete segments_[seg];
      }
    }
    delete []segments_;
    segments_ = NULL;
  }

  if (space_cost_) {
    delete []space_cost_;
    space_cost_ = NULL;
  }

  if (no_space_cost_) {
    delete []no_space_cost_;
    no_space_cost_ = NULL;
  }

  segment_cnt_ = 0;
  init_ = false;
}

// # of segmentation points. One less than the count of segments
int CubeSearchObject::SegPtCnt() {
  if (!init_ && !Init())
    return -1;
  return segment_cnt_ - 1;
}

// init and allocate variables, perform segmentation
bool CubeSearchObject::Init() {
  if (init_)
    return true;
  if (!Segment()) {
    return false;
  }

  // init cache
  reco_cache_ = new CharAltList **[segment_cnt_];
  if (reco_cache_ == NULL) {
    fprintf(stderr, "Cube ERROR (CubeSearchObject::Init): could not "
            "allocate CharAltList array\n");
    return false;
  }

  samp_cache_ = new CharSamp **[segment_cnt_];
  if (samp_cache_ == NULL) {
    fprintf(stderr, "Cube ERROR (CubeSearchObject::Init): could not "
            "allocate CharSamp array\n");
    return false;
  }

  for (int seg = 0; seg < segment_cnt_; seg++) {
    reco_cache_[seg] = new CharAltList *[segment_cnt_];
    if (reco_cache_[seg] == NULL) {
      fprintf(stderr, "Cube ERROR (CubeSearchObject::Init): could not "
              "allocate a single segment's CharAltList array\n");
      return false;
    }

    memset(reco_cache_[seg], 0, segment_cnt_ * sizeof(*reco_cache_[seg]));

    samp_cache_[seg] = new CharSamp *[segment_cnt_];
    if (samp_cache_[seg] == NULL) {
      fprintf(stderr, "Cube ERROR (CubeSearchObject::Init): could not "
              "allocate a single segment's CharSamp array\n");
      return false;
    }

    memset(samp_cache_[seg], 0, segment_cnt_ * sizeof(*samp_cache_[seg]));
  }

  init_ = true;
  return true;
}

// returns a char sample corresponding to the bitmap between 2 seg pts
CharSamp *CubeSearchObject::CharSample(int start_pt, int end_pt) {
  // init if necessary
  if (!init_ && !Init())
    return NULL;
  // validate segment range
  if (!IsValidSegmentRange(start_pt, end_pt))
    return NULL;

  // look for the samp in the cache
  if (samp_cache_ && samp_cache_[start_pt + 1] &&
      samp_cache_[start_pt + 1][end_pt]) {
    return samp_cache_[start_pt + 1][end_pt];
  }
  // create a char samp object from the specified range of segments
  bool left_most;
  bool right_most;
  CharSamp *samp = CharSamp::FromConComps(segments_, start_pt + 1,
                                          end_pt - start_pt, NULL,
                                          &left_most, &right_most, hgt_);
  if (!samp)
    return NULL;

  if (kUseCroppedChars) {
    CharSamp *cropped_samp = samp->Crop();
    // we no longer need the orig sample
    delete samp;
    if (!cropped_samp)
      return NULL;
    samp = cropped_samp;
  }

  // get the dimensions of the new cropped sample
  int char_top = samp->Top();
  int char_wid = samp->Width();
  int char_hgt = samp->Height();

  // for cursive languages, these features correspond to whether
  // the charsamp is at the beginning or end of conncomp
  if (cntxt_->Cursive() == true) {
    // first and last char flags depend on reading order
    bool first_char = rtl_ ? right_most : left_most;
    bool last_char = rtl_ ? left_most : right_most;

    samp->SetFirstChar(first_char ? 255 : 0);
    samp->SetLastChar(last_char ? 255 : 0);
  } else {
    // for non cursive languages, these features correspond
    // to whether the charsamp is at the begining or end of the word
    samp->SetFirstChar((start_pt == -1) ? 255 : 0);
    samp->SetLastChar((end_pt == (segment_cnt_ - 1)) ? 255 : 0);
  }
  samp->SetNormTop(255 * char_top / hgt_);
  samp->SetNormBottom(255 * (char_top + char_hgt) / hgt_);
  samp->SetNormAspectRatio(255 * char_wid / (char_wid + char_hgt));

  // add to cache & return
  samp_cache_[start_pt + 1][end_pt] = samp;
  return samp;
}

Box *CubeSearchObject::CharBox(int start_pt, int end_pt) {
  if (!init_ && !Init())
    return NULL;
  if (!IsValidSegmentRange(start_pt, end_pt)) {
    fprintf(stderr, "Cube ERROR (CubeSearchObject::CharBox): invalid "
            "segment range (%d, %d)\n", start_pt, end_pt);
    return NULL;
  }

  // create a char samp object from the specified range of segments,
  // extract its dimensions into a leptonica box, and delete it
  bool left_most;
  bool right_most;
  CharSamp *samp = CharSamp::FromConComps(segments_, start_pt + 1,
                                          end_pt - start_pt, NULL,
                                          &left_most, &right_most, hgt_);
  if (!samp)
    return NULL;
  if (kUseCroppedChars) {
    CharSamp *cropped_samp = samp->Crop();
    delete samp;
    if (!cropped_samp) {
      return NULL;
    }
    samp = cropped_samp;
  }
  Box *box = boxCreate(samp->Left(), samp->Top(),
                       samp->Width(), samp->Height());
  delete samp;
  return box;
}

// call from Beam Search to return the alt list corresponding to
// recognizing the bitmap between two segmentation pts
CharAltList * CubeSearchObject::RecognizeSegment(int start_pt, int end_pt) {
  // init if necessary
  if (!init_ && !Init()) {
    fprintf(stderr, "Cube ERROR (CubeSearchObject::RecognizeSegment): could "
            "not initialize CubeSearchObject\n");
    return NULL;
  }

  // validate segment range
  if (!IsValidSegmentRange(start_pt, end_pt)) {
    fprintf(stderr, "Cube ERROR (CubeSearchObject::RecognizeSegment): invalid "
            "segment range (%d, %d)\n", start_pt, end_pt);
    return NULL;
  }

  // look for the recognition results in cache in the cache
  if (reco_cache_ && reco_cache_[start_pt + 1] &&
      reco_cache_[start_pt + 1][end_pt]) {
    return reco_cache_[start_pt + 1][end_pt];
  }

  // create the char sample corresponding to the blob
  CharSamp *samp = CharSample(start_pt, end_pt);
  if (!samp) {
    fprintf(stderr, "Cube ERROR (CubeSearchObject::RecognizeSegment): could "
            "not construct CharSamp\n");
    return NULL;
  }

  // recognize the char sample
  CharClassifier *char_classifier = cntxt_->Classifier();
  if (char_classifier) {
    reco_cache_[start_pt + 1][end_pt] = char_classifier->Classify(samp);
  } else {
    // no classifer: all characters are equally probable; add a penalty
    // that favors 2-segment characters and aspect ratios (w/h) > 1
    fprintf(stderr, "Cube WARNING (CubeSearchObject::RecognizeSegment): cube "
            "context has no character classifier!! Inventing a probability "
            "distribution.\n");
    int class_cnt = cntxt_->CharacterSet()->ClassCount();
    CharAltList *alt_list = new CharAltList(cntxt_->CharacterSet(), class_cnt);
    int seg_cnt = end_pt - start_pt;
    double prob_val = (1.0 / class_cnt) *
        exp(-abs(seg_cnt - 2.0)) *
        exp(-samp->Width() / static_cast<double>(samp->Height()));

    if (alt_list) {
      for (int class_idx = 0; class_idx < class_cnt; class_idx++) {
        alt_list->Insert(class_idx, CubeUtils::Prob2Cost(prob_val));
      }
      reco_cache_[start_pt + 1][end_pt] = alt_list;
    }
  }

  return reco_cache_[start_pt + 1][end_pt];
}

// Perform segmentation of the bitmap by detecting connected components,
// segmenting each connected component using windowed vertical pixel density
// histogram and sorting the resulting segments in reading order
bool CubeSearchObject::Segment() {
  if (!samp_)
    return false;
  segment_cnt_ = 0;
  segments_ = samp_->Segment(&segment_cnt_, rtl_,
                             cntxt_->Params()->HistWindWid(),
                             cntxt_->Params()->MinConCompSize());
  if (!segments_ || segment_cnt_ <= 0) {
    return false;
  }
  if (segment_cnt_ >= kMaxSegmentCnt) {
    return false;
  }
  return true;
}

// computes the space and no space costs at gaps between segments
bool CubeSearchObject::ComputeSpaceCosts() {
  // init if necessary
  if (!init_ && !Init())
    return false;

  // Already computed
  if (space_cost_)
    return true;

  // No segmentation points
  if (segment_cnt_ < 2)
    return false;

  // Compute the maximum x to the left of and minimum x to the right of each
  // segmentation point
  int *max_left_x = new int[segment_cnt_ - 1];
  int *min_right_x = new int[segment_cnt_ - 1];
  if (!max_left_x || !min_right_x) {
    delete []min_right_x;
    delete []max_left_x;
    return false;
  }
  if (rtl_) {
    min_right_x[0] = segments_[0]->Left();
    max_left_x[segment_cnt_ - 2] = segments_[segment_cnt_ - 1]->Right();
    for (int pt_idx = 1; pt_idx < (segment_cnt_ - 1); pt_idx++) {
      min_right_x[pt_idx] =
          MIN(min_right_x[pt_idx - 1], segments_[pt_idx]->Left());
      max_left_x[segment_cnt_ - pt_idx - 2] =
          MAX(max_left_x[segment_cnt_ - pt_idx - 1],
              segments_[segment_cnt_ - pt_idx - 1]->Right());
    }
  } else {
    min_right_x[segment_cnt_ - 2] = segments_[segment_cnt_ - 1]->Left();
    max_left_x[0] = segments_[0]->Right();
    for (int pt_idx = 1; pt_idx < (segment_cnt_ - 1); pt_idx++) {
      min_right_x[segment_cnt_ - pt_idx - 2] =
          MIN(min_right_x[segment_cnt_ - pt_idx - 1],
              segments_[segment_cnt_ - pt_idx - 1]->Left());
      max_left_x[pt_idx] =
          MAX(max_left_x[pt_idx - 1], segments_[pt_idx]->Right());
    }
  }

  // Allocate memory for space and no space costs
  // trivial cases
  space_cost_ = new int[segment_cnt_ - 1];
  no_space_cost_ = new int[segment_cnt_ - 1];
  if (!space_cost_ || !no_space_cost_) {
    delete []min_right_x;
    delete []max_left_x;
    return false;
  }

  // go through all segmentation points determining the horizontal gap between
  // the images on both sides of each break points. Use the gap to estimate
  // the probability of a space. The probability is modeled a linear function
  // of the gap width
  for (int pt_idx = 0; pt_idx < (segment_cnt_ - 1); pt_idx++) {
    // determine the gap at the segmentation point
    int gap = min_right_x[pt_idx] - max_left_x[pt_idx];
    float prob = 0.0;

    // gap is too small => no space
    if (gap < min_spc_gap_) {
      prob = 0.0;
    } else if (gap > max_spc_gap_) {
      // gap is too big => definite space
      prob = 1.0;
    } else {
      // gap is somewhere in between, compute probability
      prob = (gap - min_spc_gap_) /
          static_cast<double>(max_spc_gap_ - min_spc_gap_);
    }

    // compute cost of space and non-space
    space_cost_[pt_idx] = CubeUtils::Prob2Cost(prob) +
                          CubeUtils::Prob2Cost(0.1);
    no_space_cost_[pt_idx] = CubeUtils::Prob2Cost(1.0 - prob);
  }

  delete []min_right_x;
  delete []max_left_x;

  return true;
}

// Returns the cost of having a space before the specified segmentation point
int CubeSearchObject::SpaceCost(int pt_idx) {
  if (!space_cost_ && !ComputeSpaceCosts()) {
    // Failed to compute costs return a zero prob
    return CubeUtils::Prob2Cost(0.0);
  }
  return space_cost_[pt_idx];
}

// Returns the cost of not having a space before the specified
// segmentation point
int CubeSearchObject::NoSpaceCost(int pt_idx) {
  // If failed to compute costs, return a 1.0 prob
  if (!space_cost_ && !ComputeSpaceCosts())
    return CubeUtils::Prob2Cost(0.0);
  return no_space_cost_[pt_idx];
}

// Returns the cost of not having any spaces within the specified range
// of segmentation points
int CubeSearchObject::NoSpaceCost(int st_pt, int end_pt) {
  // If fail to compute costs, return a 1.0 prob
  if (!space_cost_ && !ComputeSpaceCosts())
    return CubeUtils::Prob2Cost(1.0);
  int no_spc_cost = 0;
  for (int pt_idx = st_pt + 1; pt_idx < end_pt; pt_idx++)
    no_spc_cost += NoSpaceCost(pt_idx);
  return no_spc_cost;
}
}
