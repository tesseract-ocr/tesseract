/**********************************************************************
 * File:        lang_model.h
 * Description: Declaration of the Language Model Edge Base Class
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

// The LanguageModel class abstracts a State machine that is modeled as a Trie
// structure. The state machine models the language being recognized by the OCR
// Engine
// This is an abstract class that is to be inherited by any language model

#ifndef LANG_MODEL_H
#define LANG_MODEL_H

#include "lang_mod_edge.h"
#include "char_altlist.h"
#include "char_set.h"
#include "tuning_params.h"

namespace tesseract {
class LangModel {
 public:
  LangModel() {
    ood_enabled_ = true;
    numeric_enabled_ = true;
    word_list_enabled_ = true;
    punc_enabled_ = true;
  }
  virtual ~LangModel() {}

  // Returns an edge pointer to the Root
  virtual LangModEdge *Root() = 0;
  // Returns the edges that fan-out of the specified edge and their count
  virtual LangModEdge **GetEdges(CharAltList *alt_list,
                                 LangModEdge *parent_edge,
                                 int *edge_cnt) = 0;
  // Returns is a sequence of 32-bit characters are valid within this language
  // model or net. And EndOfWord flag is specified. If true, the sequence has
  // to end on a valid word. The function also optionally returns the list
  // of language model edges traversed to parse the string
  virtual bool IsValidSequence(const char_32 *str, bool eow_flag,
                               LangModEdge **edge_array = NULL) = 0;
  virtual bool IsLeadingPunc(char_32 ch) = 0;
  virtual bool IsTrailingPunc(char_32 ch) = 0;
  virtual bool IsDigit(char_32 ch) = 0;

  // accessor functions
  inline bool OOD() { return ood_enabled_; }
  inline bool Numeric() { return numeric_enabled_; }
  inline bool WordList() { return word_list_enabled_; }
  inline bool Punc() { return punc_enabled_; }
  inline void SetOOD(bool ood) { ood_enabled_ = ood; }
  inline void SetNumeric(bool numeric) { numeric_enabled_ = numeric; }
  inline void SetWordList(bool word_list) { word_list_enabled_ = word_list; }
  inline void SetPunc(bool punc_enabled) { punc_enabled_ = punc_enabled; }

 protected:
  bool ood_enabled_;
  bool numeric_enabled_;
  bool word_list_enabled_;
  bool punc_enabled_;
};
}

#endif  // LANG_MODEL_H
