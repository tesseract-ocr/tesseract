/**********************************************************************
 * File:        search_column.cpp
 * Description: Implementation of the Beam Search Column Class
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

#include "search_column.h"
#include <stdlib.h>

namespace tesseract {

SearchColumn::SearchColumn(int col_idx, int max_node) {
  col_idx_ = col_idx;
  node_cnt_ = 0;
  node_array_ = NULL;
  max_node_cnt_ = max_node;
  node_hash_table_ = NULL;
  init_ = false;
  min_cost_ = INT_MAX;
  max_cost_ = 0;
}

// Cleanup data
void SearchColumn::Cleanup() {
  if (node_array_ != NULL) {
    for (int node_idx = 0; node_idx < node_cnt_; node_idx++) {
      if (node_array_[node_idx] != NULL) {
        delete node_array_[node_idx];
      }
    }

    delete []node_array_;
    node_array_ = NULL;
  }
  FreeHashTable();
  init_ = false;
}

SearchColumn::~SearchColumn() {
  Cleanup();
}

// Initializations
bool SearchColumn::Init() {
  if (init_ == true) {
    return true;
  }

  // create hash table
  if (node_hash_table_ == NULL) {
    node_hash_table_ = new SearchNodeHashTable();
    if (node_hash_table_ == NULL) {
      return false;
    }
  }

  init_ = true;

  return true;
}

// Prune the nodes if necessary. Pruning is done such that a max
// number of nodes is kept, i.e., the beam width
void SearchColumn::Prune() {
  // no need to prune
  if (node_cnt_ <= max_node_cnt_) {
    return;
  }

  // compute the cost histogram
  memset(score_bins_, 0, sizeof(score_bins_));
  int cost_range = max_cost_ - min_cost_ + 1;
  for (int node_idx = 0; node_idx < node_cnt_; node_idx++) {
    int cost_bin = static_cast<int>(
        ((node_array_[node_idx]->BestCost() - min_cost_) *
         kScoreBins) / static_cast<double>(cost_range));
    if (cost_bin >= kScoreBins) {
      cost_bin = kScoreBins - 1;
    }
    score_bins_[cost_bin]++;
  }

  // determine the pruning cost by scanning the cost histogram from
  // least to greatest cost bins and finding the cost at which the
  // max number of nodes is exceeded
  int pruning_cost = 0;
  int new_node_cnt = 0;
  for (int cost_bin = 0; cost_bin < kScoreBins; cost_bin++) {
    if (new_node_cnt > 0 &&
        (new_node_cnt + score_bins_[cost_bin]) > max_node_cnt_) {
      pruning_cost = min_cost_ + ((cost_bin * cost_range) / kScoreBins);
      break;
    }
    new_node_cnt += score_bins_[cost_bin];
  }

  // prune out all the nodes above this cost
  for (int node_idx = new_node_cnt = 0; node_idx < node_cnt_; node_idx++) {
    // prune this node out
    if (node_array_[node_idx]->BestCost() > pruning_cost ||
        new_node_cnt > max_node_cnt_) {
      delete node_array_[node_idx];
    } else {
      // keep it
      node_array_[new_node_cnt++] = node_array_[node_idx];
    }
  }
  node_cnt_ = new_node_cnt;
}

// sort all nodes
void SearchColumn::Sort() {
  if (node_cnt_ > 0 && node_array_ != NULL) {
    qsort(node_array_, node_cnt_, sizeof(*node_array_),
          SearchNode::SearchNodeComparer);
  }
}

// add a new node
SearchNode *SearchColumn::AddNode(LangModEdge *edge, int reco_cost,
                                  SearchNode *parent_node,
                                  CubeRecoContext *cntxt) {
  // init if necessary
  if (init_ == false && Init() == false) {
    return NULL;
  }

  // find out if we have an node with the same edge
  // look in the hash table
  SearchNode *new_node = node_hash_table_->Lookup(edge, parent_node);
  // node does not exist
  if (new_node == NULL) {
    new_node = new SearchNode(cntxt, parent_node, reco_cost, edge, col_idx_);
    if (new_node == NULL) {
      return NULL;
    }

    // if the max node count has already been reached, check if the cost of
    // the new node exceeds the max cost. This indicates that it will be pruned
    // and so there is no point adding it
    if (node_cnt_ >= max_node_cnt_ && new_node->BestCost() > max_cost_) {
      delete new_node;
      return NULL;
    }

    // expand the node buffer if necc
    if ((node_cnt_ % kNodeAllocChunk) == 0) {
      // alloc a new buff
      SearchNode **new_node_buff =
          new SearchNode *[node_cnt_ + kNodeAllocChunk];
      if (new_node_buff == NULL) {
        delete new_node;
        return NULL;
      }

      // free existing after copying contents
      if (node_array_ != NULL) {
        memcpy(new_node_buff, node_array_, node_cnt_ * sizeof(*new_node_buff));
        delete []node_array_;
      }

      node_array_ = new_node_buff;
    }

    // add the node to the hash table only if it is non-OOD edge
    // because the langmod state is not unique
    if (edge->IsOOD() == false) {
      if (!node_hash_table_->Insert(edge, new_node)) {
        tprintf("Hash table full!!!");
        delete new_node;
        return NULL;
      }
    }

    node_array_[node_cnt_++] = new_node;

  } else {
    // node exists before
    // if no update occurred, return NULL
    if (new_node->UpdateParent(parent_node, reco_cost, edge) == false) {
      new_node = NULL;
    }

    // free the edge
    if (edge != NULL) {
      delete edge;
    }
  }

  // update Min and Max Costs
  if (new_node != NULL) {
    if (min_cost_ > new_node->BestCost()) {
      min_cost_ = new_node->BestCost();
    }

    if (max_cost_ < new_node->BestCost()) {
      max_cost_ = new_node->BestCost();
    }
  }

  return new_node;
}

SearchNode *SearchColumn::BestNode() {
  SearchNode *best_node = NULL;

  for (int node_idx = 0; node_idx < node_cnt_; node_idx++) {
    if (best_node == NULL ||
        best_node->BestCost() > node_array_[node_idx]->BestCost()) {
      best_node = node_array_[node_idx];
    }
  }

  return best_node;
}
}  // namespace tesseract
