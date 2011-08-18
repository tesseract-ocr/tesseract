/**********************************************************************
 * File:        tess_lang_model.h
 * Description: Declaration of the Tesseract Language Model Class
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

#ifndef TESS_LANG_MODEL_H
#define TESS_LANG_MODEL_H

#include <string>

#include "char_altlist.h"
#include "cube_reco_context.h"
#include "cube_tuning_params.h"
#include "dict.h"
#include "lang_model.h"
#include "tessdatamanager.h"
#include "tess_lang_mod_edge.h"

namespace tesseract {

const int kStateCnt = 4;
const int kNumLiteralCnt = 5;

class TessLangModel : public LangModel {
 public:
  TessLangModel(const string &lm_params,
                const string &data_file_path,
                bool load_system_dawg,
                TessdataManager *tessdata_manager,
                CubeRecoContext *cntxt);
  ~TessLangModel() {
    if (word_dawgs_ != NULL) {
      word_dawgs_->delete_data_pointers();
      delete word_dawgs_;
    }
  }

  // returns a pointer to the root of the language model
  inline TessLangModEdge *Root() {
    return NULL;
  }

  // The general fan-out generation function. Returns the list of edges
  // fanning-out of the specified edge and their count. If an AltList is
  // specified, only the class-ids with a minimum cost are considered
  LangModEdge **GetEdges(CharAltList *alt_list,
                         LangModEdge *edge,
                         int *edge_cnt);
  // Determines if a sequence of 32-bit chars is valid in this language model
  // starting from the root. If the eow_flag is ON, also checks for
  // a valid EndOfWord. If final_edge is not NULL, returns a pointer to the last
  // edge
  bool IsValidSequence(const char_32 *sequence, bool eow_flag,
                       LangModEdge **final_edge = NULL);
  bool IsLeadingPunc(char_32 ch);
  bool IsTrailingPunc(char_32 ch);
  bool IsDigit(char_32 ch);

  void RemoveInvalidCharacters(string *lm_str);
 private:
  // static LM state machines
  static const Dawg *ood_dawg_;
  static const Dawg *number_dawg_;
  static const int num_state_machine_[kStateCnt][kNumLiteralCnt];
  static const int num_max_repeat_[kStateCnt];
  // word_dawgs_ should only be loaded if cube has its own version of the
  // unicharset (different from the one used by tesseract) and therefore
  // can not use the dawgs loaded for tesseract (since the unichar ids
  // encoded in the dawgs differ).
  DawgVector *word_dawgs_;

  static int max_edge_;
  static int max_ood_shape_cost_;

  // remaining language model elements needed by cube. These get loaded from
  // the .lm file
  string lead_punc_;
  string trail_punc_;
  string num_lead_punc_;
  string num_trail_punc_;
  string operators_;
  string digits_;
  string alphas_;
  // String of characters in RHS of each line of <lang>.cube.lm
  // Each element is hard-coded to correspond to a specific token type
  // (see LoadLangModelElements)
  string *literal_str_[kNumLiteralCnt];
  // Recognition context needed to access language properties
  // (case, cursive,..)
  CubeRecoContext *cntxt_;
  bool has_case_;

  // computes and returns the edges that fan out of an edge ref
  int FanOut(CharAltList *alt_list,
             const Dawg *dawg, EDGE_REF edge_ref, EDGE_REF edge_ref_mask,
             const char_32 *str, bool root_flag, LangModEdge **edge_array);
  // generate edges from an NULL terminated string
  // (used for punctuation, operators and digits)
  int Edges(const char *strng, const Dawg *dawg,
            EDGE_REF edge_ref, EDGE_REF edge_ref_mask,
            LangModEdge **edge_array);
  // Generate the edges fanning-out from an edge in the number state machine
  int NumberEdges(EDGE_REF edge_ref, LangModEdge **edge_array);
  // Generate OOD edges
  int OODEdges(CharAltList *alt_list, EDGE_REF edge_ref,
               EDGE_REF edge_ref_mask, LangModEdge **edge_array);
  // Cleanup an edge array
  void FreeEdges(int edge_cnt, LangModEdge **edge_array);
  // Determines if a sequence of 32-bit chars is valid in this language model
  // starting from the specified edge. If the eow_flag is ON, also checks for
  // a valid EndOfWord. If final_edge is not NULL, returns a pointer to the last
  // edge
  bool IsValidSequence(LangModEdge *edge, const char_32 *sequence,
                       bool eow_flag, LangModEdge **final_edge);
  // Parse language model elements from the given string, which should
  // have been loaded from <lang>.cube.lm file, e.g. in CubeRecoContext
  bool LoadLangModelElements(const string &lm_params);

  // Returns the number of word Dawgs in the language model.
  int NumDawgs() const;

  // Returns the dawgs with the given index from either the dawgs
  // stored by the Tesseract object, or the word_dawgs_.
  const Dawg *GetDawg(int index) const;
};
}  // tesseract

#endif  // TESS_LANG_MODEL_H
