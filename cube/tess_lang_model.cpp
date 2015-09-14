/**********************************************************************
 * File:        tess_lang_model.cpp
 * Description: Implementation of the Tesseract Language Model Class
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

// The TessLangModel class abstracts the Tesseract language model. It inherits
// from the LangModel class. The Tesseract language model encompasses several
// Dawgs (words from training data, punctuation, numbers, document words).
// On top of this Cube adds an OOD state machine
// The class provides methods to traverse the language model in a generative
// fashion. Given any node in the DAWG, the language model can generate a list
// of children (or fan-out) edges

#include <string>
#include <vector>

#include "char_samp.h"
#include "cube_utils.h"
#include "dict.h"
#include "tesseractclass.h"
#include "tess_lang_model.h"
#include "tessdatamanager.h"
#include "unicharset.h"

namespace tesseract {
// max fan-out (used for preallocation). Initialized here, but modified by
// constructor
int TessLangModel::max_edge_ = 4096;

// Language model extra State machines
const Dawg *TessLangModel::ood_dawg_ = reinterpret_cast<Dawg *>(DAWG_OOD);
const Dawg *TessLangModel::number_dawg_ = reinterpret_cast<Dawg *>(DAWG_NUMBER);

// number state machine
const int TessLangModel::num_state_machine_[kStateCnt][kNumLiteralCnt] = {
  {0, 1, 1, NUM_TRM, NUM_TRM},
  {NUM_TRM, 1, 1, 3, 2},
  {NUM_TRM, NUM_TRM, 1, NUM_TRM, 2},
  {NUM_TRM, NUM_TRM, 3, NUM_TRM, 2},
};
const int TessLangModel::num_max_repeat_[kStateCnt] = {3, 32, 8, 3};

// thresholds and penalties
int TessLangModel::max_ood_shape_cost_ = CubeUtils::Prob2Cost(1e-4);

TessLangModel::TessLangModel(const string &lm_params,
                             const string &data_file_path,
                             bool load_system_dawg,
                             TessdataManager *tessdata_manager,
                             CubeRecoContext *cntxt) {
  cntxt_ = cntxt;
  has_case_ = cntxt_->HasCase();
  // Load the rest of the language model elements from file
  LoadLangModelElements(lm_params);
  // Load word_dawgs_ if needed.
  if (tessdata_manager->SeekToStart(TESSDATA_CUBE_UNICHARSET)) {
    word_dawgs_ = new DawgVector();
    if (load_system_dawg &&
        tessdata_manager->SeekToStart(TESSDATA_CUBE_SYSTEM_DAWG)) {
      // The last parameter to the Dawg constructor (the debug level) is set to
      // false, until Cube has a way to express its preferred debug level.
      *word_dawgs_ +=  new SquishedDawg(tessdata_manager->GetDataFilePtr(),
                                        DAWG_TYPE_WORD,
                                        cntxt_->Lang().c_str(),
                                        SYSTEM_DAWG_PERM, false);
    }
  } else {
    word_dawgs_ = NULL;
  }
}

// Cleanup an edge array
void TessLangModel::FreeEdges(int edge_cnt, LangModEdge **edge_array) {
  if (edge_array != NULL) {
    for (int edge_idx = 0; edge_idx < edge_cnt; edge_idx++) {
      if (edge_array[edge_idx] != NULL) {
        delete edge_array[edge_idx];
      }
    }
    delete []edge_array;
  }
}

// Determines if a sequence of 32-bit chars is valid in this language model
// starting from the specified edge. If the eow_flag is ON, also checks for
// a valid EndOfWord. If final_edge is not NULL, returns a pointer to the last
// edge
bool TessLangModel::IsValidSequence(LangModEdge *edge,
                                    const char_32 *sequence,
                                    bool eow_flag,
                                    LangModEdge **final_edge) {
  // get the edges emerging from this edge
  int edge_cnt = 0;
  LangModEdge **edge_array = GetEdges(NULL, edge, &edge_cnt);

  // find the 1st char in the sequence in the children
  for (int edge_idx = 0; edge_idx < edge_cnt; edge_idx++) {
    // found a match
    if (sequence[0] == edge_array[edge_idx]->EdgeString()[0]) {
      // if this is the last char
      if (sequence[1] == 0) {
        // succeed if we are in prefix mode or this is a terminal edge
        if (eow_flag == false || edge_array[edge_idx]->IsEOW()) {
          if (final_edge != NULL) {
            (*final_edge) = edge_array[edge_idx];
            edge_array[edge_idx] = NULL;
          }

          FreeEdges(edge_cnt, edge_array);
          return true;
        }
      } else {
        // not the last char continue checking
        if (IsValidSequence(edge_array[edge_idx], sequence + 1, eow_flag,
                            final_edge) == true) {
          FreeEdges(edge_cnt, edge_array);
          return true;
        }
      }
    }
  }

  FreeEdges(edge_cnt, edge_array);
  return false;
}

// Determines if a sequence of 32-bit chars is valid in this language model
// starting from the root. If the eow_flag is ON, also checks for
// a valid EndOfWord. If final_edge is not NULL, returns a pointer to the last
// edge
bool TessLangModel::IsValidSequence(const char_32 *sequence, bool eow_flag,
                                    LangModEdge **final_edge) {
  if (final_edge != NULL) {
    (*final_edge) = NULL;
  }

  return IsValidSequence(NULL, sequence, eow_flag, final_edge);
}

bool TessLangModel::IsLeadingPunc(const char_32 ch) {
  return lead_punc_.find(ch) != string::npos;
}

bool TessLangModel::IsTrailingPunc(const char_32 ch) {
  return trail_punc_.find(ch) != string::npos;
}

bool TessLangModel::IsDigit(const char_32 ch) {
  return digits_.find(ch) != string::npos;
}

// The general fan-out generation function. Returns the list of edges
// fanning-out of the specified edge and their count. If an AltList is
// specified, only the class-ids with a minimum cost are considered
LangModEdge ** TessLangModel::GetEdges(CharAltList *alt_list,
                                       LangModEdge *lang_mod_edge,
                                       int *edge_cnt) {
  TessLangModEdge *tess_lm_edge =
      reinterpret_cast<TessLangModEdge *>(lang_mod_edge);
  LangModEdge **edge_array = NULL;
  (*edge_cnt) = 0;

  // if we are starting from the root, we'll instantiate every DAWG
  // and get the all the edges that emerge from the root
  if (tess_lm_edge == NULL) {
    // get DAWG count from Tesseract
    int dawg_cnt = NumDawgs();
    // preallocate the edge buffer
    (*edge_cnt) = dawg_cnt * max_edge_;
    edge_array = new LangModEdge *[(*edge_cnt)];
    if (edge_array == NULL) {
      return NULL;
    }

    for (int dawg_idx = (*edge_cnt) = 0; dawg_idx < dawg_cnt; dawg_idx++) {
      const Dawg *curr_dawg = GetDawg(dawg_idx);
      // Only look through word Dawgs (since there is a special way of
      // handling numbers and punctuation).
      if (curr_dawg->type() == DAWG_TYPE_WORD) {
        (*edge_cnt) += FanOut(alt_list, curr_dawg, 0, 0, NULL, true,
                              edge_array + (*edge_cnt));
      }
    }  // dawg

    (*edge_cnt) += FanOut(alt_list, number_dawg_, 0, 0, NULL, true,
                          edge_array + (*edge_cnt));

    // OOD: it is intentionally not added to the list to make sure it comes
    // at the end
    (*edge_cnt) += FanOut(alt_list, ood_dawg_, 0, 0, NULL, true,
                          edge_array + (*edge_cnt));

    // set the root flag for all root edges
    for (int edge_idx = 0; edge_idx < (*edge_cnt); edge_idx++) {
      edge_array[edge_idx]->SetRoot(true);
    }
  } else {  // not starting at the root
    // preallocate the edge buffer
    (*edge_cnt) = max_edge_;
    // allocate memory for edges
    edge_array = new LangModEdge *[(*edge_cnt)];
    if (edge_array == NULL) {
      return NULL;
    }

    // get the FanOut edges from the root of each dawg
    (*edge_cnt) = FanOut(alt_list,
                         tess_lm_edge->GetDawg(),
                         tess_lm_edge->EndEdge(), tess_lm_edge->EdgeMask(),
                         tess_lm_edge->EdgeString(), false, edge_array);
  }
  return edge_array;
}

// generate edges from an NULL terminated string
// (used for punctuation, operators and digits)
int TessLangModel::Edges(const char *strng, const Dawg *dawg,
                         EDGE_REF edge_ref, EDGE_REF edge_mask,
                         LangModEdge **edge_array) {
  int edge_idx,
    edge_cnt = 0;

  for (edge_idx = 0; strng[edge_idx] != 0; edge_idx++) {
    int class_id = cntxt_->CharacterSet()->ClassID((char_32)strng[edge_idx]);
    if (class_id != INVALID_UNICHAR_ID) {
      // create an edge object
      edge_array[edge_cnt] = new TessLangModEdge(cntxt_, dawg, edge_ref,
                                                 class_id);
      if (edge_array[edge_cnt] == NULL) {
        return 0;
      }

      reinterpret_cast<TessLangModEdge *>(edge_array[edge_cnt])->
          SetEdgeMask(edge_mask);
      edge_cnt++;
    }
  }

  return edge_cnt;
}

// generate OOD edges
int TessLangModel::OODEdges(CharAltList *alt_list, EDGE_REF edge_ref,
                            EDGE_REF edge_ref_mask, LangModEdge **edge_array) {
  int class_cnt = cntxt_->CharacterSet()->ClassCount();
  int edge_cnt = 0;
  for (int class_id = 0; class_id < class_cnt; class_id++) {
    // produce an OOD edge only if the cost of the char is low enough
    if ((alt_list == NULL ||
         alt_list->ClassCost(class_id) <= max_ood_shape_cost_)) {
      // create an edge object
      edge_array[edge_cnt] = new TessLangModEdge(cntxt_, class_id);
      if (edge_array[edge_cnt] == NULL) {
        return 0;
      }

      edge_cnt++;
    }
  }

  return edge_cnt;
}

// computes and returns the edges that fan out of an edge ref
int TessLangModel::FanOut(CharAltList *alt_list, const Dawg *dawg,
                          EDGE_REF edge_ref, EDGE_REF edge_mask,
                          const char_32 *str, bool root_flag,
                          LangModEdge **edge_array) {
  int edge_cnt = 0;
  NODE_REF next_node = NO_EDGE;

  // OOD
  if (dawg == reinterpret_cast<Dawg *>(DAWG_OOD)) {
    if (ood_enabled_ == true) {
      return OODEdges(alt_list, edge_ref, edge_mask, edge_array);
    } else {
      return 0;
    }
  } else if (dawg == reinterpret_cast<Dawg *>(DAWG_NUMBER)) {
    // Number
    if (numeric_enabled_ == true) {
      return NumberEdges(edge_ref, edge_array);
    } else {
      return 0;
    }
  } else if (IsTrailingPuncEdge(edge_mask)) {
    // a TRAILING PUNC MASK, generate more trailing punctuation and return
    if (punc_enabled_ == true) {
      EDGE_REF trail_cnt = TrailingPuncCount(edge_mask);
      return Edges(trail_punc_.c_str(), dawg, edge_ref,
                   TrailingPuncEdgeMask(trail_cnt + 1), edge_array);
    } else {
      return 0;
    }
  } else if (root_flag == true || edge_ref == 0) {
    // Root, generate leading punctuation and continue
    if (root_flag) {
      if (punc_enabled_ == true) {
        edge_cnt += Edges(lead_punc_.c_str(), dawg, 0, LEAD_PUNC_EDGE_REF_MASK,
                          edge_array);
      }
    }
    next_node = 0;
  } else {
    // a node in the main trie
    bool eow_flag = (dawg->end_of_word(edge_ref) != 0);

    // for EOW
    if (eow_flag == true) {
      // generate trailing punctuation
      if (punc_enabled_ == true) {
        edge_cnt += Edges(trail_punc_.c_str(), dawg, edge_ref,
                          TrailingPuncEdgeMask((EDGE_REF)1), edge_array);
        // generate a hyphen and go back to the root
        edge_cnt += Edges("-/", dawg, 0, 0, edge_array + edge_cnt);
      }
    }

    // advance node
    next_node = dawg->next_node(edge_ref);
    if (next_node == 0 || next_node == NO_EDGE) {
      return edge_cnt;
    }
  }

  // now get all the emerging edges if word list is enabled
  if (word_list_enabled_ == true && next_node != NO_EDGE) {
    // create child edges
    int child_edge_cnt =
      TessLangModEdge::CreateChildren(cntxt_, dawg, next_node,
                                      edge_array + edge_cnt);
    int strt_cnt = edge_cnt;

    // set the edge mask
    for (int child = 0; child < child_edge_cnt; child++) {
      reinterpret_cast<TessLangModEdge *>(edge_array[edge_cnt++])->
          SetEdgeMask(edge_mask);
    }

    // if we are at the root, create upper case forms of these edges if possible
    if (root_flag == true) {
      for (int child = 0; child < child_edge_cnt; child++) {
        TessLangModEdge *child_edge =
            reinterpret_cast<TessLangModEdge *>(edge_array[strt_cnt + child]);

        if (has_case_ == true) {
          const char_32 *edge_str = child_edge->EdgeString();
          if (edge_str != NULL && islower(edge_str[0]) != 0 &&
              edge_str[1] == 0) {
            int class_id =
                cntxt_->CharacterSet()->ClassID(toupper(edge_str[0]));
            if (class_id != INVALID_UNICHAR_ID) {
              // generate an upper case edge for lower case chars
              edge_array[edge_cnt] = new TessLangModEdge(cntxt_, dawg,
                  child_edge->StartEdge(), child_edge->EndEdge(), class_id);

              if (edge_array[edge_cnt] != NULL) {
                reinterpret_cast<TessLangModEdge *>(edge_array[edge_cnt])->
                    SetEdgeMask(edge_mask);
                edge_cnt++;
              }
            }
          }
        }
      }
    }
  }
  return edge_cnt;
}

// Generate the edges fanning-out from an edge in the number state machine
int TessLangModel::NumberEdges(EDGE_REF edge_ref, LangModEdge **edge_array) {
  EDGE_REF new_state,
    state;

  inT64 repeat_cnt,
    new_repeat_cnt;

  state = ((edge_ref & NUMBER_STATE_MASK) >> NUMBER_STATE_SHIFT);
  repeat_cnt = ((edge_ref & NUMBER_REPEAT_MASK) >> NUMBER_REPEAT_SHIFT);

  if (state < 0 || state >= kStateCnt) {
    return 0;
  }

  // go through all valid transitions from the state
  int edge_cnt = 0;

  EDGE_REF new_edge_ref;

  for (int lit = 0; lit < kNumLiteralCnt; lit++) {
    // move to the new state
    new_state = num_state_machine_[state][lit];
    if (new_state == NUM_TRM) {
      continue;
    }

    if (new_state == state) {
      new_repeat_cnt = repeat_cnt + 1;
    } else {
      new_repeat_cnt = 1;
    }

    // not allowed to repeat beyond this
    if (new_repeat_cnt > num_max_repeat_[state]) {
      continue;
    }

    new_edge_ref = (new_state << NUMBER_STATE_SHIFT) |
        (lit << NUMBER_LITERAL_SHIFT) |
        (new_repeat_cnt << NUMBER_REPEAT_SHIFT);

    edge_cnt += Edges(literal_str_[lit]->c_str(), number_dawg_,
                      new_edge_ref, 0, edge_array + edge_cnt);
  }

  return edge_cnt;
}

// Loads Language model elements from contents of the <lang>.cube.lm file
bool TessLangModel::LoadLangModelElements(const string &lm_params) {
  bool success = true;
  // split into lines, each corresponding to a token type below
  vector<string> str_vec;
  CubeUtils::SplitStringUsing(lm_params, "\r\n", &str_vec);
  for (int entry = 0; entry < str_vec.size(); entry++) {
    vector<string> tokens;
    // should be only two tokens: type and value
    CubeUtils::SplitStringUsing(str_vec[entry], "=", &tokens);
    if (tokens.size() != 2)
      success = false;
    if (tokens[0] == "LeadPunc") {
      lead_punc_ = tokens[1];
    } else if (tokens[0] == "TrailPunc") {
      trail_punc_ = tokens[1];
    } else if (tokens[0] == "NumLeadPunc") {
      num_lead_punc_ = tokens[1];
    } else if (tokens[0] == "NumTrailPunc") {
      num_trail_punc_ = tokens[1];
    } else if (tokens[0] == "Operators") {
      operators_ = tokens[1];
    } else if (tokens[0] == "Digits") {
      digits_ = tokens[1];
    } else if (tokens[0] == "Alphas") {
      alphas_ = tokens[1];
    } else {
      success = false;
    }
  }

  RemoveInvalidCharacters(&num_lead_punc_);
  RemoveInvalidCharacters(&num_trail_punc_);
  RemoveInvalidCharacters(&digits_);
  RemoveInvalidCharacters(&operators_);
  RemoveInvalidCharacters(&alphas_);

  // form the array of literal strings needed for number state machine
  // It is essential that the literal strings go in the order below
  literal_str_[0] = &num_lead_punc_;
  literal_str_[1] = &num_trail_punc_;
  literal_str_[2] = &digits_;
  literal_str_[3] = &operators_;
  literal_str_[4] = &alphas_;

  return success;
}

void TessLangModel::RemoveInvalidCharacters(string *lm_str) {
  CharSet *char_set = cntxt_->CharacterSet();
  tesseract::string_32 lm_str32;
  CubeUtils::UTF8ToUTF32(lm_str->c_str(), &lm_str32);

  int len = CubeUtils::StrLen(lm_str32.c_str());
  char_32 *clean_str32 = new char_32[len + 1];
  if (!clean_str32)
    return;
  int clean_len = 0;
  for (int i = 0; i < len; ++i) {
    int class_id = char_set->ClassID((char_32)lm_str32[i]);
    if (class_id != INVALID_UNICHAR_ID) {
      clean_str32[clean_len] = lm_str32[i];
      ++clean_len;
    }
  }
  clean_str32[clean_len] = 0;
  if (clean_len < len) {
    lm_str->clear();
    CubeUtils::UTF32ToUTF8(clean_str32, lm_str);
  }
  delete [] clean_str32;
}

int TessLangModel::NumDawgs() const {
  return (word_dawgs_ != NULL) ?
      word_dawgs_->size() : cntxt_->TesseractObject()->getDict().NumDawgs();
}

// Returns the dawgs with the given index from either the dawgs
// stored by the Tesseract object, or the word_dawgs_.
const Dawg *TessLangModel::GetDawg(int index) const {
  if (word_dawgs_ != NULL) {
    ASSERT_HOST(index < word_dawgs_->size());
    return (*word_dawgs_)[index];
  } else {
    ASSERT_HOST(index < cntxt_->TesseractObject()->getDict().NumDawgs());
    return cntxt_->TesseractObject()->getDict().GetDawg(index);
  }
}
}
