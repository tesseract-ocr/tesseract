/**********************************************************************
 * File:        cube_search_object.h
 * Description: Declaration of the Cube Search Object Class
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

// The CubeSearchObject class represents a char_samp (a word bitmap) that is
// being searched for characters (or recognizeable entities).
// The Class detects the connected components and peforms an oversegmentation
// on each ConComp. The result of which is a list of segments that are ordered
// in reading order.
// The class provided methods that inquire about the number of segments, the
// CharSamp corresponding to any segment range and the recognition results
// of any segment range
// An object of Class CubeSearchObject is used by the BeamSearch algorithm
// to recognize a CharSamp into a list of word alternates

#ifndef CUBE_SEARCH_OBJECT_H
#define CUBE_SEARCH_OBJECT_H

#include "search_object.h"
#include "char_samp.h"
#include "conv_net_classifier.h"
#include "cube_reco_context.h"
#include "allheaders.h"

namespace tesseract {
class CubeSearchObject : public SearchObject {
 public:
  CubeSearchObject(CubeRecoContext *cntxt, CharSamp *samp);
  ~CubeSearchObject();

  // returns the Segmentation Point count of the CharSamp owned by the class
  int SegPtCnt();
  // Recognize the set of segments given by the specified range and return
  // a list of possible alternate answers
  CharAltList * RecognizeSegment(int start_pt, int end_pt);
  // Returns the CharSamp corresponding to the specified segment range
  CharSamp *CharSample(int start_pt, int end_pt);
  // Returns a leptonica box corresponding to the specified segment range
  Box *CharBox(int start_pt, int end_pt);
  // Returns the cost of having a space before the specified segmentation pt
  int SpaceCost(int seg_pt);
  // Returns the cost of not having a space before the specified
  // segmentation pt
  int NoSpaceCost(int seg_pt);
  // Returns the cost of not having any spaces within the specified range
  // of segmentation points
  int NoSpaceCost(int seg_pt, int end_pt);

 private:
  // Maximum reasonable segment count
  static const int kMaxSegmentCnt = 128;
  // Use cropped samples
  static const bool kUseCroppedChars;

  // reading order flag
  bool rtl_;
  // cached dimensions of char samp
  int left_;
  int itop_;
  int wid_;
  int hgt_;
  // minimum and maximum and possible inter-segment gaps for spaces
  int min_spc_gap_;
  int max_spc_gap_;
  // initialization flag
  bool init_;
  // maximum segments per character: Cached from tuning parameters object
  int max_seg_per_char_;
  // char sample to be processed
  CharSamp *samp_;
  // segment count
  int segment_cnt_;
  // segments of the processed char samp
  ConComp **segments_;
  // Cache data members:
  // There are two caches kept; a CharSamp cache and a CharAltList cache
  // Each is a 2-D array of CharSamp and CharAltList pointers respectively
  // hence the triple pointer.
  CharAltList ***reco_cache_;
  CharSamp ***samp_cache_;
  // Cached costs of space and no-space after every segment. Computed only
  // in phrase mode
  int *space_cost_;
  int *no_space_cost_;

  // init and allocate variables, perform segmentation
  bool Init();
  // Cleanup
  void Cleanup();
  // Perform segmentation of the bitmap by detecting connected components,
  // segmenting each connected component using windowed vertical pixel density
  // histogram and sorting the resulting segments in reading order
  // Returns true on success
  bool Segment();
  // validate the segment ranges.
  inline bool IsValidSegmentRange(int start_pt, int end_pt) {
    return (end_pt > start_pt && start_pt >= -1 && start_pt < segment_cnt_ &&
            end_pt >= 0 && end_pt <= segment_cnt_ &&
            end_pt <= (start_pt + max_seg_per_char_));
  }
  // computes the space and no space costs at gaps between segments
  // return true on success
  bool ComputeSpaceCosts();
};
}

#endif  // CUBE_SEARCH_OBJECT_H
