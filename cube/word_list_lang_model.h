/**********************************************************************
 * File:        word_list_lang_model.h
 * Description: Declaration of the Word List Language Model Class
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

// The WordListLangModel class abstracts a language model that is based on
// a list of words. It inherits from the LangModel abstract class
// Besides providing the methods inherited from the LangModel abstract class,
// the class provided methods to add new strings to the Language Model:
// AddString & AddString32

#ifndef WORD_LIST_LANG_MODEL_H
#define WORD_LIST_LANG_MODEL_H

#include <vector>

#include "cube_reco_context.h"
#include "lang_model.h"
#include "tess_lang_mod_edge.h"

namespace tesseract {

class Trie;

class WordListLangModel : public LangModel {
 public:
  explicit WordListLangModel(CubeRecoContext *cntxt);
  ~WordListLangModel();
  // Returns an edge pointer to the Root
  LangModEdge *Root();
  // Returns the edges that fan-out of the specified edge and their count
  LangModEdge **GetEdges(CharAltList *alt_list,
                         LangModEdge *edge,
                         int *edge_cnt);
  // Returns is a sequence of 32-bit characters are valid within this language
  // model or net. And EndOfWord flag is specified. If true, the sequence has
  // to end on a valid word. The function also optionally returns the list
  // of language model edges traversed to parse the string
  bool IsValidSequence(const char_32 *sequence,
                       bool eow_flag,
                       LangModEdge **edges);
  bool IsLeadingPunc(char_32 ch) { return false; }  // not yet implemented
  bool IsTrailingPunc(char_32 ch) { return false; }  // not yet implemented
  bool IsDigit(char_32 ch) { return false; }  // not yet implemented
  // Adds a new UTF-8 string to the language model
  bool AddString(const char *char_ptr);
  // Adds a new UTF-32 string to the language model
  bool AddString32(const char_32 *char_32_ptr);
  // Compute all the variants of a 32-bit string in terms of the class-ids.
  // This is needed for languages that have ligatures. A word can then have
  // more than one spelling in terms of the class-ids.
  static void WordVariants(const CharSet &char_set, const UNICHARSET *uchset,
                           string_32 str32,
                           vector<WERD_CHOICE *> *word_variants);
 private:
  // constants needed to configure the language model
  static const int kMaxEdge = 512;

  CubeRecoContext *cntxt_;
  Trie *dawg_;
  bool init_;
  // Initialize the language model
  bool Init();
  // Cleanup
  void Cleanup();
  // Recursive helper function for WordVariants().
  static void WordVariants(
      const CharSet &char_set,
      string_32 prefix_str32, WERD_CHOICE *word_so_far,
      string_32 str32,
      vector<WERD_CHOICE *> *word_variants);
};
}  // tesseract

#endif  // WORD_LIST_LANG_MODEL_H
