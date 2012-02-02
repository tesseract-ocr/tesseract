 /**********************************************************************
 * File:        word_unigrams.h
 * Description: Declaration of the Word Unigrams Class
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

// The WordUnigram class holds the unigrams of the most frequent set of words
// in a language. It is an optional component of the Cube OCR engine. If
// present, the unigram cost of a word is aggregated with the other costs
// (Recognition, Language Model, Size) to compute a cost for a word.
// The word list is assumed to be sorted in lexicographic order.

#ifndef WORD_UNIGRAMS_H
#define WORD_UNIGRAMS_H

#include <string>
#include "char_set.h"
#include "lang_model.h"

namespace tesseract {
class WordUnigrams {
 public:
  WordUnigrams();
  ~WordUnigrams();
  // Load the word-list and unigrams from file and create an object
  // The word list is assumed to be sorted
  static WordUnigrams *Create(const string &data_file_path,
                              const string &lang);
  // Compute the unigram cost of a UTF-32 string. Splits into
  // space-separated tokens, strips trailing punctuation from each
  // token, evaluates case properties, and calls internal Cost()
  // function on UTF-8 version. To avoid unnecessarily penalizing
  // all-one-case words or capitalized words (first-letter
  // upper-case and remaining letters lower-case) when not all
  // versions of the word appear in the <lang>.cube.word-freq file, a
  // case-invariant cost is computed in those cases, assuming the word
  // meets a minimum length.
  int Cost(const char_32 *str32, LangModel *lang_mod,
           CharSet *char_set) const;
 protected:
  // Compute the word unigram cost of a UTF-8 string with binary
  // search of sorted words_ array.
  int CostInternal(const char *str) const;
 private:
  // Only words this length or greater qualify for all-numeric or
  // case-invariant word unigram cost.
  static const int kMinLengthNumOrCaseInvariant = 4;

  int word_cnt_;
  char **words_;
  int *costs_;
  int not_in_list_cost_;
};
}

#endif  // WORD_UNIGRAMS_H
