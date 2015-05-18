/**********************************************************************
 * File:        tess_lang_mod_edge.cpp
 * Description: Implementation of the Tesseract Language Model Edge Class
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

#include "tess_lang_mod_edge.h"
#include "const.h"
#include "unichar.h"



namespace tesseract {
// OOD constructor
TessLangModEdge::TessLangModEdge(CubeRecoContext *cntxt, int class_id) {
  root_ = false;
  cntxt_ = cntxt;
  dawg_ = NULL;
  start_edge_ = 0;
  end_edge_ = 0;
  edge_mask_ = 0;
  class_id_ = class_id;
  str_ = cntxt_->CharacterSet()->ClassString(class_id);
  path_cost_ = Cost();
}

// leading, trailing punc constructor and single byte UTF char
TessLangModEdge::TessLangModEdge(CubeRecoContext *cntxt,
    const Dawg *dawg, EDGE_REF edge_idx, int class_id) {
  root_ = false;
  cntxt_ = cntxt;
  dawg_ = dawg;
  start_edge_ = edge_idx;
  end_edge_ = edge_idx;
  edge_mask_ = 0;
  class_id_ = class_id;
  str_ = cntxt_->CharacterSet()->ClassString(class_id);
  path_cost_ = Cost();
}

// dict constructor: multi byte UTF char
TessLangModEdge::TessLangModEdge(CubeRecoContext *cntxt, const Dawg *dawg,
                                 EDGE_REF start_edge_idx, EDGE_REF end_edge_idx,
                                 int class_id) {
  root_ = false;
  cntxt_ = cntxt;
  dawg_ = dawg;
  start_edge_ = start_edge_idx;
  end_edge_ = end_edge_idx;
  edge_mask_ = 0;
  class_id_ = class_id;
  str_ = cntxt_->CharacterSet()->ClassString(class_id);
  path_cost_ = Cost();
}

char *TessLangModEdge::Description() const {
  char *char_ptr = new char[256];
  if (!char_ptr) {
    return NULL;
  }

  char dawg_str[256];
  char edge_str[32];
  if (dawg_ == (Dawg *)DAWG_OOD) {
    strcpy(dawg_str, "OOD");
  } else if (dawg_ == (Dawg *)DAWG_NUMBER) {
    strcpy(dawg_str, "NUM");
  } else if (dawg_->permuter() == SYSTEM_DAWG_PERM) {
    strcpy(dawg_str, "Main");
  } else if (dawg_->permuter() == USER_DAWG_PERM) {
    strcpy(dawg_str, "User");
  } else if (dawg_->permuter() == DOC_DAWG_PERM) {
    strcpy(dawg_str, "Doc");
  } else {
    strcpy(dawg_str, "N/A");
  }

  sprintf(edge_str, "%d", static_cast<int>(start_edge_));
  if (IsLeadingPuncEdge(edge_mask_)) {
    strcat(edge_str, "-LP");
  }
  if (IsTrailingPuncEdge(edge_mask_)) {
    strcat(edge_str, "-TP");
  }
  sprintf(char_ptr, "%s(%s)%s, Wtd Dawg Cost=%d",
          dawg_str, edge_str, IsEOW() ? "-EOW-" : "", path_cost_);

  return char_ptr;
}

int TessLangModEdge::CreateChildren(CubeRecoContext *cntxt,
                                    const Dawg *dawg,
                                    NODE_REF parent_node,
                                    LangModEdge **edge_array) {
  int edge_cnt = 0;
  NodeChildVector vec;
  dawg->unichar_ids_of(parent_node, &vec, false);  // find all children
  for (int i = 0; i < vec.size(); ++i) {
    const NodeChild &child = vec[i];
    if (child.unichar_id == INVALID_UNICHAR_ID) continue;
    edge_array[edge_cnt] =
      new TessLangModEdge(cntxt, dawg, child.edge_ref, child.unichar_id);
    if (edge_array[edge_cnt] != NULL) edge_cnt++;
  }
  return edge_cnt;
}
}
