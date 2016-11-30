/**********************************************************************
 * File:        word_list_lang_model.cpp
 * Description: Implementation of the Word List Language Model Class
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

#include <string>
#include <vector>
#include "word_list_lang_model.h"
#include "cube_utils.h"

#include "ratngs.h"
#include "trie.h"

namespace tesseract {
WordListLangModel::WordListLangModel(CubeRecoContext *cntxt) {
  cntxt_ = cntxt;
  dawg_ = NULL;
  init_ = false;
}

WordListLangModel::~WordListLangModel() {
  Cleanup();
}

// Cleanup
void WordListLangModel::Cleanup() {
  if (dawg_ != NULL) {
    delete dawg_;
    dawg_ = NULL;
  }
  init_ = false;
}

// Initialize the language model
bool WordListLangModel::Init() {
  if (init_ == true) {
    return true;
  }
  // The last parameter to the Trie constructor (the debug level) is set to
  // false for now, until Cube has a way to express its preferred debug level.
  dawg_ = new Trie(DAWG_TYPE_WORD, "", NO_PERM,
                   cntxt_->CharacterSet()->ClassCount(), false);
  init_ = true;
  return true;
}

// return a pointer to the root
LangModEdge * WordListLangModel::Root() {
  return NULL;
}

// return the edges emerging from the current state
LangModEdge **WordListLangModel::GetEdges(CharAltList *alt_list,
                                          LangModEdge *edge,
                                          int *edge_cnt) {
  // initialize if necessary
  if (init_ == false) {
    if (Init() == false) {
      return NULL;
    }
  }

  (*edge_cnt) = 0;

  EDGE_REF edge_ref;

  TessLangModEdge *tess_lm_edge = reinterpret_cast<TessLangModEdge *>(edge);

  if (tess_lm_edge == NULL) {
    edge_ref = 0;
  } else {
    edge_ref = tess_lm_edge->EndEdge();

    // advance node
    edge_ref = dawg_->next_node(edge_ref);
    if (edge_ref == 0) {
      return NULL;
    }
  }

  // allocate memory for edges
  LangModEdge **edge_array = new LangModEdge *[kMaxEdge];

  // now get all the emerging edges
  (*edge_cnt) += TessLangModEdge::CreateChildren(cntxt_, dawg_, edge_ref,
                                                 edge_array + (*edge_cnt));

  return edge_array;
}

// returns true if the char_32 is supported by the language model
// TODO(ahmadab) currently not implemented
bool WordListLangModel::IsValidSequence(const char_32 *sequence,
                                        bool terminal, LangModEdge **edges) {
  return false;
}

// Recursive helper function for WordVariants().
void WordListLangModel::WordVariants(const CharSet &char_set,
                                     string_32 prefix_str32,
                                     WERD_CHOICE *word_so_far,
                                     string_32 str32,
                                     vector<WERD_CHOICE *> *word_variants) {
  int str_len = str32.length();
  if (str_len == 0) {
    if (word_so_far->length() > 0) {
      word_variants->push_back(new WERD_CHOICE(*word_so_far));
    }
  } else {
    // Try out all the possible prefixes of the str32.
    for (int len = 1; len <= str_len; len++) {
      // Check if prefix is supported in character set.
      string_32 str_pref32 = str32.substr(0, len);
      int class_id = char_set.ClassID(reinterpret_cast<const char_32 *>(
          str_pref32.c_str()));
      if (class_id <= 0) {
        continue;
      } else {
        string_32 new_prefix_str32 = prefix_str32 + str_pref32;
        string_32 new_str32 = str32.substr(len);
        word_so_far->append_unichar_id(class_id, 1, 0.0, 0.0);
        WordVariants(char_set, new_prefix_str32, word_so_far, new_str32,
                     word_variants);
        word_so_far->remove_last_unichar_id();
      }
    }
  }
}

// Compute all the variants of a 32-bit string in terms of the class-ids
// This is needed for languages that have ligatures. A word can then have more
// than one spelling in terms of the class-ids
void WordListLangModel::WordVariants(const CharSet &char_set,
                                     const UNICHARSET *uchset, string_32 str32,
                                     vector<WERD_CHOICE *> *word_variants) {
  for (int i = 0; i < word_variants->size(); i++) {
    delete (*word_variants)[i];
  }
  word_variants->clear();
  string_32 prefix_str32;
  WERD_CHOICE word_so_far(uchset);
  WordVariants(char_set, prefix_str32, &word_so_far, str32, word_variants);
}

// add a new UTF-8 string to the lang model
bool WordListLangModel::AddString(const char *char_ptr) {
  if (!init_ && !Init()) {  // initialize if necessary
    return false;
  }

  string_32 str32;
  CubeUtils::UTF8ToUTF32(char_ptr, &str32);
  if (str32.length() < 1) {
    return false;
  }
  return AddString32(str32.c_str());
}

// add a new UTF-32 string to the lang model
bool WordListLangModel::AddString32(const char_32 *char_32_ptr) {
  if (char_32_ptr == NULL) {
    return false;
  }
  // get all the word variants
  vector<WERD_CHOICE *> word_variants;
  WordVariants(*(cntxt_->CharacterSet()), cntxt_->TessUnicharset(),
               char_32_ptr, &word_variants);

  if (word_variants.size() > 0) {
    // find the shortest variant
    int shortest_word = 0;
    for (int word = 1; word < word_variants.size(); word++) {
      if (word_variants[shortest_word]->length() >
          word_variants[word]->length()) {
        shortest_word = word;
      }
    }
    // only add the shortest grapheme interpretation of string to the word list
    dawg_->add_word_to_dawg(*word_variants[shortest_word]);
  }
  for (int i = 0; i < word_variants.size(); i++) { delete word_variants[i]; }
  return true;
}

}
