/**********************************************************************
 * File:        beam_search.h
 * Description: Declaration of Beam Word Search Algorithm Class
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

// The Beam Search class implements a Beam Search algorithm for the
// N-best paths through the lattice of a search object using a language model
// The search object is a segmented bitmap of a word image. The language model
// is a state machine that defines valid sequences of characters
// The cost of each path is the combined (product) probabilities of the
// characters along the path. The character probabilities are computed using
// the character classifier member of the RecoContext
// The BeamSearch class itself holds the state of the last search it performed
// using its "Search" method. Subsequent class to the Search method erase the
// states of previously done searches

#ifndef BEAM_SEARCH_H
#define BEAM_SEARCH_H

#include "search_column.h"
#include "word_altlist.h"
#include "search_object.h"
#include "lang_model.h"
#include "cube_utils.h"
#include "cube_reco_context.h"
#include "allheaders.h"

namespace tesseract {

class BeamSearch {
 public:
  explicit BeamSearch(CubeRecoContext *cntxt, bool word_mode = true);
  ~BeamSearch();
  // Performs a beam search in the specified search using the specified
  // language model; returns an alternate list of possible words as a result.
  WordAltList *Search(SearchObject *srch_obj, LangModel *lang_mod = NULL);
  // Returns the best node in the last column of last performed search.
  SearchNode *BestNode() const;
  // Returns the string corresponding to the specified alt.
  char_32 *Alt(int alt) const;
  // Backtracks from the specified lattice node and returns the corresponding
  // character-mapped segments, character count, char_32 result string, and
  // character bounding boxes (if char_boxes is not NULL). If the segments
  // cannot be constructed, returns NULL, and all result arguments
  // will be NULL.
  CharSamp **BackTrack(SearchObject *srch_obj, int node_index,
                       int *char_cnt, char_32 **str32, Boxa **char_boxes) const;
  // Same as above, except it takes a pointer to a search node object
  // instead of node index.
  CharSamp **BackTrack(SearchObject *srch_obj, SearchNode *node,
                       int *char_cnt, char_32 **str32, Boxa **char_boxes) const;
  // Returns the size cost of a specified string of a lattice
  // path that ends at the specified lattice node.
  int SizeCost(SearchObject *srch_obj, SearchNode *node,
               char_32 **str32 = NULL) const;
  // Returns the word unigram cost of the given string, possibly
  // stripping out a single trailing punctuation character.
  int WordUnigramCost(char_32 *str32, WordUnigrams* word_unigrams) const;

  // Supplementary functions needed for visualization
  // Return column count of the lattice.
  inline int ColCnt() const { return col_cnt_; }
  // Returns the lattice column corresponding to the specified column index.
  SearchColumn *Column(int col_idx) const;
  // Return the index of the best node in the last column of the
  // best-cost path before the alternates list is sorted.
  inline int BestPresortedNodeIndex() const {
    return best_presorted_node_idx_;
  };

 private:
  // Maximum reasonable segmentation point count
  static const int kMaxSegPointCnt = 128;
  // Recognition context object; the context holds the character classifier
  // and the tuning parameters object
  CubeRecoContext *cntxt_;
  // Count of segmentation pts
  int seg_pt_cnt_;
  // Lattice column count; currently redundant with respect to seg_pt_cnt_
  // but that might change in the future
  int col_cnt_;
  // Array of lattice columns
  SearchColumn **col_;
  // Run in word or phrase mode
  bool word_mode_;
  // Node index of best-cost node, before alternates are merged and sorted
  int best_presorted_node_idx_;
  // Cleans up beam search state
  void Cleanup();
  // Creates a Word alternate list from the results in the lattice.
  // This function computes a cost for each node in the final column
  // of the lattice, which is a weighted average of several costs:
  // size cost, character bigram cost, word unigram cost, and
  // recognition cost from the beam search. The weights are the
  // CubeTuningParams, which are learned together with the character
  // classifiers.
  WordAltList *CreateWordAltList(SearchObject *srch_obj);
  // Creates a set of children nodes emerging from a parent node based on
  // the character alternate list and the language model.
  void CreateChildren(SearchColumn *out_col, LangModel *lang_mod,
                      SearchNode *parent_node, LangModEdge *lm_parent_edge,
                      CharAltList *char_alt_list, int extra_cost);
  // Backtracks from the given lattice node and returns the corresponding
  // char mapped segments, character count, and character bounding boxes (if
  // char_boxes is not NULL). If the segments cannot be constructed,
  // returns NULL, and all result arguments will be NULL.
  CharSamp **SplitByNode(SearchObject *srch_obj, SearchNode *srch_node,
                         int* char_cnt, Boxa **char_boxes) const;
};
}

#endif  // BEAM_SEARCH_H
