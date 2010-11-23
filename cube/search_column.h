/**********************************************************************
 * File:        search_column.h
 * Description: Declaration of the Beam Search Column Class
 * Author:    Ahmad Abdulkader
 * Created:   2008
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

// The SearchColumn class abstracts a column in the lattice that is created
// by the BeamSearch during the recognition process
// The class holds the lattice nodes. New nodes are added by calls to AddNode
// made from the BeamSearch
// The class maintains a hash table of the nodes to be able to lookup nodes
// quickly using their lang_mod_edge. This is needed to merge similar paths
// in the lattice

#ifndef SEARCH_COLUMN_H
#define SEARCH_COLUMN_H

#include "search_node.h"
#include "lang_mod_edge.h"
#include "cube_reco_context.h"

namespace tesseract {

class SearchColumn {
 public:
  SearchColumn(int col_idx, int max_node_cnt);
  ~SearchColumn();
  // Accessor functions
  inline int ColIdx() const { return col_idx_; }
  inline int NodeCount() const { return node_cnt_; }
  inline SearchNode **Nodes() const { return node_array_; }

  // Prune the nodes if necessary. Pruning is done such that a max
  // number of nodes is kept, i.e., the beam width
  void Prune();
  SearchNode *AddNode(LangModEdge *edge, int score,
                      SearchNode *parent, CubeRecoContext *cntxt);
  // Returns the node with the least cost
  SearchNode *BestNode();
  // Sort the lattice nodes. Needed for visualization
  void Sort();
  // Free up the Hash Table. Added to be called by the Beam Search after
  // a column is pruned to reduce memory foot print
  void FreeHashTable() {
    if (node_hash_table_ != NULL) {
      delete node_hash_table_;
      node_hash_table_ = NULL;
    }
  }

 private:
  static const int kNodeAllocChunk = 1024;
  static const int kScoreBins = 1024;
  bool init_;
  int min_cost_;
  int max_cost_;
  int max_node_cnt_;
  int node_cnt_;
  int col_idx_;
  int score_bins_[kScoreBins];
  SearchNode **node_array_;
  SearchNodeHashTable *node_hash_table_;

  // Free node array and hash table
  void Cleanup();
  // Create hash table
  bool Init();
};
}

#endif  // SEARCH_COLUMN_H
