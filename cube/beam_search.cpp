/**********************************************************************
 * File:        beam_search.cpp
 * Description: Class to implement Beam Word Search Algorithm
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

#include "beam_search.h"
#include "tesseractclass.h"

namespace tesseract {

BeamSearch::BeamSearch(CubeRecoContext *cntxt, bool word_mode) {
  cntxt_ = cntxt;
  seg_pt_cnt_ = 0;
  col_cnt_ = 1;
  col_ = NULL;
  word_mode_ = word_mode;
}

// Cleanup the lattice corresponding to the last search
void BeamSearch::Cleanup() {
  if (col_ != NULL) {
    for (int col = 0; col < col_cnt_; col++) {
      delete col_[col];
    }
    delete []col_;
  }
  col_ = NULL;
}

BeamSearch::~BeamSearch() {
  Cleanup();
}

// Creates a set of children nodes emerging from a parent node based on
// the character alternate list and the language model.
void BeamSearch::CreateChildren(SearchColumn *out_col, LangModel *lang_mod,
                                SearchNode *parent_node,
                                LangModEdge *lm_parent_edge,
                                CharAltList *char_alt_list, int extra_cost) {
  // get all the edges from this parent
  int edge_cnt;
  LangModEdge **lm_edges = lang_mod->GetEdges(char_alt_list,
                                              lm_parent_edge, &edge_cnt);
  if (lm_edges) {
    // add them to the ending column with the appropriate parent
    for (int edge = 0; edge < edge_cnt; edge++) {
      // add a node to the column if the current column is not the
      // last one, or if the lang model edge indicates it is valid EOW
      if (!cntxt_->NoisyInput() && out_col->ColIdx() >= seg_pt_cnt_ &&
          !lm_edges[edge]->IsEOW()) {
        // free edge since no object is going to own it
        delete lm_edges[edge];
        continue;
      }

      // compute the recognition cost of this node
      int recognition_cost =  MIN_PROB_COST;
      if (char_alt_list && char_alt_list->AltCount() > 0) {
        recognition_cost = MAX(0, char_alt_list->ClassCost(
            lm_edges[edge]->ClassID()));
        // Add the no space cost. This should zero in word mode
        recognition_cost += extra_cost;
      }

      // Note that the edge will be freed inside the column if
      // AddNode is called
      if (recognition_cost >= 0) {
        out_col->AddNode(lm_edges[edge], recognition_cost, parent_node,
                         cntxt_);
      } else {
        delete lm_edges[edge];
      }
    }  // edge
    // free edge array
    delete []lm_edges;
  }  // lm_edges
}

// Performs a beam search in the specified search using the specified
// language model; returns an alternate list of possible words as a result.
WordAltList * BeamSearch::Search(SearchObject *srch_obj, LangModel *lang_mod) {
  // verifications
  if (!lang_mod)
    lang_mod = cntxt_->LangMod();
  if (!lang_mod) {
    fprintf(stderr, "Cube ERROR (BeamSearch::Search): could not construct "
            "LangModel\n");
    return NULL;
  }

  // free existing state
  Cleanup();

  // get seg pt count
  seg_pt_cnt_ = srch_obj->SegPtCnt();
  if (seg_pt_cnt_ < 0) {
    return NULL;
  }
  col_cnt_ = seg_pt_cnt_ + 1;

  // disregard suspicious cases
  if (seg_pt_cnt_ > 128) {
    fprintf(stderr, "Cube ERROR (BeamSearch::Search): segment point count is "
            "suspiciously high; bailing out\n");
    return NULL;
  }

  // alloc memory for columns
  col_ = new SearchColumn *[col_cnt_];
  memset(col_, 0, col_cnt_ * sizeof(*col_));

  // for all possible segments
  for (int end_seg = 1; end_seg <= (seg_pt_cnt_ + 1); end_seg++) {
    // create a search column
    col_[end_seg - 1] = new SearchColumn(end_seg - 1,
                                         cntxt_->Params()->BeamWidth());

    // for all possible start segments
    int init_seg = MAX(0, end_seg - cntxt_->Params()->MaxSegPerChar());
    for (int strt_seg = init_seg; strt_seg < end_seg; strt_seg++) {
      int parent_nodes_cnt;
      SearchNode **parent_nodes;

      // for the root segment, we do not have a parent
      if (strt_seg == 0) {
        parent_nodes_cnt = 1;
        parent_nodes = NULL;
      } else {
        // for all the existing nodes in the starting column
        parent_nodes_cnt = col_[strt_seg - 1]->NodeCount();
        parent_nodes = col_[strt_seg - 1]->Nodes();
      }

      // run the shape recognizer
      CharAltList *char_alt_list = srch_obj->RecognizeSegment(strt_seg - 1,
                                                              end_seg - 1);
      // for all the possible parents
      for (int parent_idx = 0; parent_idx < parent_nodes_cnt; parent_idx++) {
        // point to the parent node
        SearchNode *parent_node = !parent_nodes ? NULL
            : parent_nodes[parent_idx];
        LangModEdge *lm_parent_edge = !parent_node ? lang_mod->Root()
            : parent_node->LangModelEdge();

        // compute the cost of not having spaces within the segment range
        int contig_cost = srch_obj->NoSpaceCost(strt_seg - 1, end_seg - 1);

        // In phrase mode, compute the cost of not having a space before
        // this character
        int no_space_cost = 0;
        if (!word_mode_ && strt_seg > 0) {
          no_space_cost = srch_obj->NoSpaceCost(strt_seg - 1);
        }

        // if the no space cost is low enough
        if ((contig_cost + no_space_cost) < MIN_PROB_COST) {
          // Add the children nodes
          CreateChildren(col_[end_seg - 1], lang_mod, parent_node,
                         lm_parent_edge, char_alt_list,
                         contig_cost + no_space_cost);
        }

        // In phrase mode and if not starting at the root
        if (!word_mode_ && strt_seg > 0) {  // parent_node must be non-NULL
          // consider starting a new word for nodes that are valid EOW
          if (parent_node->LangModelEdge()->IsEOW()) {
            // get the space cost
            int space_cost = srch_obj->SpaceCost(strt_seg - 1);
            // if the space cost is low enough
            if ((contig_cost + space_cost) < MIN_PROB_COST) {
              // Restart the language model and add nodes as children to the
              // space node.
              CreateChildren(col_[end_seg - 1], lang_mod, parent_node, NULL,
                             char_alt_list, contig_cost + space_cost);
            }
          }
        }
      }  // parent
    }  // strt_seg

    // prune the column nodes
    col_[end_seg - 1]->Prune();

    // Free the column hash table. No longer needed
    col_[end_seg - 1]->FreeHashTable();
  }  // end_seg

  WordAltList *alt_list = CreateWordAltList(srch_obj);
  return alt_list;
}

// Creates a Word alternate list from the results in the lattice.
WordAltList *BeamSearch::CreateWordAltList(SearchObject *srch_obj) {
  // create an alternate list of all the nodes in the last column
  int node_cnt = col_[col_cnt_ - 1]->NodeCount();
  SearchNode **srch_nodes = col_[col_cnt_ - 1]->Nodes();
  CharBigrams *bigrams = cntxt_->Bigrams();
  WordUnigrams *word_unigrams = cntxt_->WordUnigramsObj();

  // Save the index of the best-cost node before the alt list is
  // sorted, so that we can retrieve it from the node list when backtracking.
  best_presorted_node_idx_ = 0;
  int best_cost = -1;

  if (node_cnt <= 0)
    return NULL;

  // start creating the word alternate list
  WordAltList *alt_list = new WordAltList(node_cnt + 1);
  for (int node_idx = 0; node_idx < node_cnt; node_idx++) {
    // recognition cost
    int recognition_cost = srch_nodes[node_idx]->BestCost();
    // compute the size cost of the alternate
    char_32 *ch_buff = NULL;
    int size_cost = SizeCost(srch_obj, srch_nodes[node_idx], &ch_buff);
    // accumulate other costs
    if (ch_buff) {
      int cost = 0;
      // char bigram cost
      int bigram_cost = !bigrams ? 0 :
          bigrams->Cost(ch_buff, cntxt_->CharacterSet());
      // word unigram cost
      int unigram_cost = !word_unigrams ? 0 :
          word_unigrams->Cost(ch_buff, cntxt_->LangMod(),
                              cntxt_->CharacterSet());
      // overall cost
      cost = static_cast<int>(
          (size_cost * cntxt_->Params()->SizeWgt()) +
          (bigram_cost * cntxt_->Params()->CharBigramWgt()) +
          (unigram_cost * cntxt_->Params()->WordUnigramWgt()) +
          (recognition_cost * cntxt_->Params()->RecoWgt()));

      // insert into word alt list
      alt_list->Insert(ch_buff, cost,
                       static_cast<void *>(srch_nodes[node_idx]));
      // Note that strict < is necessary because WordAltList::Sort()
      // uses it in a bubble sort to swap entries.
      if (best_cost < 0 || cost < best_cost) {
        best_presorted_node_idx_ = node_idx;
        best_cost = cost;
      }
      delete []ch_buff;
    }
  }

  // sort the alternates based on cost
  alt_list->Sort();
  return alt_list;
}

// Returns the lattice column corresponding to the specified column index.
SearchColumn *BeamSearch::Column(int col) const {
  if (col < 0 || col >= col_cnt_ || !col_)
    return NULL;
  return col_[col];
}

// Returns the best node in the last column of last performed search.
SearchNode *BeamSearch::BestNode() const {
  if (col_cnt_ < 1 || !col_ || !col_[col_cnt_ - 1])
    return NULL;

  int node_cnt = col_[col_cnt_ - 1]->NodeCount();
  SearchNode **srch_nodes = col_[col_cnt_ - 1]->Nodes();
  if (node_cnt < 1 || !srch_nodes || !srch_nodes[0])
    return NULL;
  return srch_nodes[0];
}

// Returns the string corresponding to the specified alt.
char_32 *BeamSearch::Alt(int alt) const {
  // get the last column of the lattice
  if (col_cnt_ <= 0)
    return NULL;

  SearchColumn *srch_col = col_[col_cnt_ - 1];
  if (!srch_col)
    return NULL;

  // point to the last node in the selected path
  if (alt >= srch_col->NodeCount() || srch_col->Nodes() == NULL) {
    return NULL;
  }

  SearchNode *srch_node = srch_col->Nodes()[alt];
  if (!srch_node)
    return  NULL;

  // get string
  char_32 *str32 = srch_node->PathString();
  if (!str32)
    return NULL;

  return str32;
}

// Backtracks from the specified node index and returns the corresponding
// character mapped segments and character count. Optional return
// arguments are the char_32 result string and character bounding
// boxes, if non-NULL values are passed in.
CharSamp **BeamSearch::BackTrack(SearchObject *srch_obj, int node_index,
                                 int *char_cnt, char_32 **str32,
                                 Boxa **char_boxes) const {
  // get the last column of the lattice
  if (col_cnt_ <= 0)
    return NULL;
  SearchColumn *srch_col = col_[col_cnt_ - 1];
  if (!srch_col)
    return NULL;

  // point to the last node in the selected path
  if (node_index >= srch_col->NodeCount() || !srch_col->Nodes())
    return NULL;

  SearchNode *srch_node = srch_col->Nodes()[node_index];
  if (!srch_node)
    return NULL;
  return BackTrack(srch_obj, srch_node, char_cnt, str32, char_boxes);
}

// Backtracks from the specified node index and returns the corresponding
// character mapped segments and character count. Optional return
// arguments are the char_32 result string and character bounding
// boxes, if non-NULL values are passed in.
CharSamp **BeamSearch::BackTrack(SearchObject *srch_obj, SearchNode *srch_node,
                                 int *char_cnt, char_32 **str32,
                                 Boxa **char_boxes) const {
  if (!srch_node)
    return NULL;

  if (str32) {
    delete [](*str32);  // clear existing value
    *str32 = srch_node->PathString();
    if (!*str32)
      return NULL;
  }

  if (char_boxes && *char_boxes) {
    boxaDestroy(char_boxes);  // clear existing value
  }

  CharSamp **chars;
  chars = SplitByNode(srch_obj, srch_node, char_cnt, char_boxes);
  if (!chars && str32)
    delete []*str32;
  return chars;
}

// Backtracks from the given lattice node and return the corresponding
// char mapped segments and character count. The character bounding
// boxes are optional return arguments, if non-NULL values are passed in.
CharSamp **BeamSearch::SplitByNode(SearchObject *srch_obj,
                                   SearchNode *srch_node,
                                   int *char_cnt,
                                   Boxa **char_boxes) const {
  // Count the characters (could be less than the path length when in
  // phrase mode)
  *char_cnt = 0;
  SearchNode *node = srch_node;
  while (node) {
    node = node->ParentNode();
    (*char_cnt)++;
  }

  if (*char_cnt == 0)
    return NULL;

  // Allocate box array
  if (char_boxes) {
    if (*char_boxes)
      boxaDestroy(char_boxes);  // clear existing value
    *char_boxes = boxaCreate(*char_cnt);
    if (*char_boxes == NULL)
      return NULL;
  }

  // Allocate memory for CharSamp array.
  CharSamp **chars = new CharSamp *[*char_cnt];

  int ch_idx = *char_cnt - 1;
  int seg_pt_cnt = srch_obj->SegPtCnt();
  bool success=true;
  while (srch_node && ch_idx >= 0) {
    // Parent node (could be null)
    SearchNode *parent_node = srch_node->ParentNode();

    // Get the seg pts corresponding to the search node
    int st_col = !parent_node ? 0 : parent_node->ColIdx() + 1;
    int st_seg_pt = st_col <= 0 ? -1 : st_col - 1;
    int end_col = srch_node->ColIdx();
    int end_seg_pt = end_col >= seg_pt_cnt ? seg_pt_cnt : end_col;

    // Get a char sample corresponding to the segmentation points
    CharSamp *samp = srch_obj->CharSample(st_seg_pt, end_seg_pt);
    if (!samp) {
      success = false;
      break;
    }
    samp->SetLabel(srch_node->NodeString());
    chars[ch_idx] = samp;
    if (char_boxes) {
      // Create the corresponding character bounding box
      Box *char_box = boxCreate(samp->Left(), samp->Top(),
                                samp->Width(), samp->Height());
      if (!char_box) {
        success = false;
        break;
      }
      boxaAddBox(*char_boxes, char_box, L_INSERT);
    }
    srch_node = parent_node;
    ch_idx--;
  }
  if (!success) {
    delete []chars;
    if (char_boxes)
      boxaDestroy(char_boxes);
    return NULL;
  }

  // Reverse the order of boxes.
  if (char_boxes) {
    int char_boxa_size = boxaGetCount(*char_boxes);
    int limit = char_boxa_size / 2;
    for (int i = 0; i < limit; ++i) {
      int box1_idx = i;
      int box2_idx = char_boxa_size - 1 - i;
      Box *box1 = boxaGetBox(*char_boxes, box1_idx, L_CLONE);
      Box *box2 = boxaGetBox(*char_boxes, box2_idx, L_CLONE);
      boxaReplaceBox(*char_boxes, box2_idx, box1);
      boxaReplaceBox(*char_boxes, box1_idx, box2);
    }
  }
  return chars;
}

// Returns the size cost of a string for a lattice path that
// ends at the specified lattice node.
int BeamSearch::SizeCost(SearchObject *srch_obj, SearchNode *node,
                         char_32 **str32) const {
  CharSamp **chars = NULL;
  int char_cnt = 0;
  if (!node)
    return 0;
  // Backtrack to get string and character segmentation
  chars = BackTrack(srch_obj, node, &char_cnt, str32, NULL);
  if (!chars)
    return WORST_COST;
  int size_cost = (cntxt_->SizeModel() == NULL) ? 0 :
      cntxt_->SizeModel()->Cost(chars, char_cnt);
  delete []chars;
  return size_cost;
}
}  // namespace tesesract
