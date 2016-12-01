/**********************************************************************
 * File:        search_node.cpp
 * Description: Implementation of the Beam Search Node Class
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

#include "search_node.h"

namespace tesseract {

// The constructor updates the best paths and costs:
//  mean_char_reco_cost_ (returned by BestRecoCost()) is the mean
//    char_reco cost of the best_path, including this node.
//  best_path_reco_cost is the total char_reco_cost of the best_path,
//    but excludes the char_reco_cost of this node.
//  best_cost is the mean mixed cost, i.e., mean_char_reco_cost_ +
//    current language model cost, all weighted by the cube context's
//    RecoWgt parameter
SearchNode::SearchNode(CubeRecoContext *cntxt, SearchNode *parent_node,
                       int char_reco_cost, LangModEdge *edge, int col_idx) {
  // copy data members
  cntxt_ = cntxt;
  lang_mod_edge_ = edge;
  col_idx_ = col_idx;
  parent_node_ = parent_node;
  char_reco_cost_ = char_reco_cost;

  // the string of this node is the same as that of the language model edge
  str_ = (edge == NULL ? NULL : edge->EdgeString());

  // compute best path total reco cost
  best_path_reco_cost_ = (parent_node_ == NULL) ?  0 :
      parent_node_->CharRecoCost() + parent_node_->BestPathRecoCost();

  // update best path length
  best_path_len_ = (parent_node_ == NULL) ?
      1 : parent_node_->BestPathLength() + 1;
  if (edge != NULL && edge->IsRoot() && parent_node_ != NULL) {
    best_path_len_++;
  }

  // compute best reco cost mean cost
  mean_char_reco_cost_ = static_cast<int>(
      (best_path_reco_cost_ + char_reco_cost_) /
      static_cast<double>(best_path_len_));

  // get language model cost
  int lm_cost = LangModCost(lang_mod_edge_, parent_node_);

  // compute aggregate best cost
  best_cost_ = static_cast<int>(cntxt_->Params()->RecoWgt() *
                                (best_path_reco_cost_ + char_reco_cost_) /
                                static_cast<double>(best_path_len_)
                                ) + lm_cost;
}

SearchNode::~SearchNode() {
  if (lang_mod_edge_ != NULL) {
    delete lang_mod_edge_;
  }
}

// update the parent_node node if provides a better (less) cost
bool SearchNode::UpdateParent(SearchNode *new_parent, int new_reco_cost,
                              LangModEdge *new_edge) {
  if (lang_mod_edge_ == NULL) {
    if (new_edge != NULL) {
      return false;
    }
  } else {
    // to update the parent_node, we have to have the same target
    // state and char
    if (new_edge == NULL || !lang_mod_edge_->IsIdentical(new_edge) ||
        !SearchNode::IdenticalPath(parent_node_, new_parent)) {
      return false;
    }
  }

  // compute the path cost and combined cost of the new path
  int new_best_path_reco_cost;
  int new_cost;
  int new_best_path_len;

  new_best_path_reco_cost = (new_parent == NULL) ?
      0 : new_parent->BestPathRecoCost() + new_parent->CharRecoCost();

  new_best_path_len =
      (new_parent == NULL) ? 1 : new_parent->BestPathLength() + 1;

  // compute the new language model cost
  int new_lm_cost = LangModCost(new_edge, new_parent);

  new_cost = static_cast<int>(cntxt_->Params()->RecoWgt() *
                              (new_best_path_reco_cost + new_reco_cost) /
                              static_cast<double>(new_best_path_len)
                              ) + new_lm_cost;

  // update if it is better (less) than the current one
  if (best_cost_ > new_cost) {
    parent_node_ = new_parent;
    char_reco_cost_ = new_reco_cost;
    best_path_reco_cost_ = new_best_path_reco_cost;
    best_path_len_ = new_best_path_len;
    mean_char_reco_cost_ = static_cast<int>(
        (best_path_reco_cost_ + char_reco_cost_) /
        static_cast<double>(best_path_len_));
    best_cost_ = static_cast<int>(cntxt_->Params()->RecoWgt() *
                                  (best_path_reco_cost_ + char_reco_cost_) /
                                  static_cast<double>(best_path_len_)
                                  ) + new_lm_cost;
    return true;
  }
  return false;
}

char_32 *SearchNode::PathString() {
  SearchNode *node = this;

  // compute string length
  int len = 0;

  while (node != NULL) {
    if (node->str_ != NULL) {
      len += CubeUtils::StrLen(node->str_);
    }

    // if the edge is a root and does not have a NULL parent, account for space
    LangModEdge *lm_edge = node->LangModelEdge();
    if (lm_edge != NULL && lm_edge->IsRoot() && node->ParentNode() != NULL) {
      len++;
    }

    node = node->parent_node_;
  }

  char_32 *char_ptr = new char_32[len + 1];

  int ch_idx = len;

  node = this;
  char_ptr[ch_idx--] = 0;

  while (node != NULL) {
    int str_len = ((node->str_ == NULL) ? 0 : CubeUtils::StrLen(node->str_));
    while (str_len > 0) {
      char_ptr[ch_idx--] = node->str_[--str_len];
    }

    // if the edge is a root and does not have a NULL parent, insert a space
    LangModEdge *lm_edge = node->LangModelEdge();
    if (lm_edge != NULL && lm_edge->IsRoot() && node->ParentNode() != NULL) {
      char_ptr[ch_idx--] = (char_32)' ';
    }

    node = node->parent_node_;
  }

  return char_ptr;
}

// compares the path of two nodes and checks if its identical
bool SearchNode::IdenticalPath(SearchNode *node1, SearchNode *node2) {
  if (node1 != NULL && node2 != NULL &&
      node1->best_path_len_ != node2->best_path_len_) {
    return false;
  }

  // backtrack until either a root or a NULL edge is reached
  while (node1 != NULL && node2 != NULL) {
    if (node1->str_ != node2->str_) {
      return false;
    }

    // stop if either nodes is a root
    if (node1->LangModelEdge()->IsRoot() || node2->LangModelEdge()->IsRoot()) {
      break;
    }

    node1 = node1->parent_node_;
    node2 = node2->parent_node_;
  }

  return ((node1 == NULL && node2 == NULL) ||
          (node1 != NULL && node1->LangModelEdge()->IsRoot() &&
           node2 != NULL && node2->LangModelEdge()->IsRoot()));
}

// Computes the language model cost of a path
int SearchNode::LangModCost(LangModEdge *current_lm_edge,
                            SearchNode *parent_node) {
  int lm_cost = 0;
  int node_cnt = 0;

  do {
    // check if root
    bool is_root = ((current_lm_edge != NULL && current_lm_edge->IsRoot()) ||
                    parent_node == NULL);
    if (is_root) {
      node_cnt++;
      lm_cost += (current_lm_edge == NULL ? 0 : current_lm_edge->PathCost());
    }

    // continue until we hit a null parent
    if (parent_node == NULL) {
      break;
    }

    // get the previous language model edge
    current_lm_edge = parent_node->LangModelEdge();
    // back track
    parent_node = parent_node->ParentNode();
  } while (true);

  return static_cast<int>(lm_cost / static_cast<double>(node_cnt));
}
}  // namespace tesseract
