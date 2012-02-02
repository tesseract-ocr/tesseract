/**********************************************************************
 * File:        word_unigrams.cpp
 * Description: Implementation of the Word Unigrams Class
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

#include <math.h>
#include <string>
#include <vector>
#include <algorithm>

#include "const.h"
#include "cube_utils.h"
#include "ndminx.h"
#include "word_unigrams.h"

namespace tesseract {

WordUnigrams::WordUnigrams() {
  costs_ = NULL;
  words_ = NULL;
  word_cnt_ = 0;
}

WordUnigrams::~WordUnigrams() {
  if (words_ != NULL) {
    if (words_[0] != NULL) {
      delete []words_[0];
    }

    delete []words_;
    words_ = NULL;
  }

  if (costs_ != NULL) {
    delete []costs_;
  }
}

// Load the word-list and unigrams from file and create an object
// The word list is assumed to be sorted in lexicographic order.
WordUnigrams *WordUnigrams::Create(const string &data_file_path,
                                   const string &lang) {
  string file_name;
  string str;

  file_name = data_file_path + lang;
  file_name += ".cube.word-freq";

  // load the string into memory
  if (CubeUtils::ReadFileToString(file_name, &str) == false) {
    return NULL;
  }

  // split into lines
  vector<string> str_vec;
  CubeUtils::SplitStringUsing(str, "\r\n \t", &str_vec);
  if (str_vec.size() < 2) {
    return NULL;
  }

  // allocate memory
  WordUnigrams *word_unigrams_obj = new WordUnigrams();
  if (word_unigrams_obj == NULL) {
    fprintf(stderr, "Cube ERROR (WordUnigrams::Create): could not create "
            "word unigrams object.\n");
    return NULL;
  }

  int full_len = str.length();
  int word_cnt = str_vec.size() / 2;
  word_unigrams_obj->words_ = new char*[word_cnt];
  word_unigrams_obj->costs_ = new int[word_cnt];

  if (word_unigrams_obj->words_ == NULL ||
      word_unigrams_obj->costs_ == NULL) {
    fprintf(stderr, "Cube ERROR (WordUnigrams::Create): error allocating "
            "word unigram fields.\n");
    delete word_unigrams_obj;
    return NULL;
  }

  word_unigrams_obj->words_[0] = new char[full_len];
  if (word_unigrams_obj->words_[0] == NULL) {
    fprintf(stderr, "Cube ERROR (WordUnigrams::Create): error allocating "
            "word unigram fields.\n");
    delete word_unigrams_obj;
    return NULL;
  }

  // construct sorted list of words and costs
  word_unigrams_obj->word_cnt_ = 0;
  char *char_buff = word_unigrams_obj->words_[0];
  word_cnt = 0;
  int max_cost = 0;

  for (int wrd = 0; wrd < str_vec.size(); wrd += 2) {
    word_unigrams_obj->words_[word_cnt] = char_buff;

    strcpy(char_buff, str_vec[wrd].c_str());
    char_buff += (str_vec[wrd].length() + 1);

    if (sscanf(str_vec[wrd + 1].c_str(), "%d",
               word_unigrams_obj->costs_ + word_cnt) != 1) {
      fprintf(stderr, "Cube ERROR (WordUnigrams::Create): error reading "
              "word unigram data.\n");
      delete word_unigrams_obj;
      return NULL;
    }
    // update max cost
    max_cost = MAX(max_cost, word_unigrams_obj->costs_[word_cnt]);
    word_cnt++;
  }
  word_unigrams_obj->word_cnt_ = word_cnt;

  // compute the not-in-list-cost by assuming that a word not in the list
  // [ahmadab]: This can be computed as follows:
  // - Given that the distribution of words follow Zipf's law:
  //   (F = K / (rank ^ S)), where s is slightly > 1.0
  // - Number of words in the list is N
  // - The mean frequency of a word that did not appear in the list is the
  //   area under the rest of the Zipf's curve divided by 2 (the mean)
  // - The area would be the bound integral from N to infinity =
  //   (K * S) / (N ^ (S + 1)) ~= K / (N ^ 2)
  // - Given that cost = -LOG(prob), the cost of an unlisted word would be
  //   = max_cost + 2*LOG(N)
  word_unigrams_obj->not_in_list_cost_ = max_cost +
      (2 * CubeUtils::Prob2Cost(1.0 / word_cnt));
  // success
  return word_unigrams_obj;
}

// Split input into space-separated tokens, strip trailing punctuation
// from each, determine case properties, call UTF-8 flavor of cost
// function on each word, and aggregate all into single mean word
// cost.
int WordUnigrams::Cost(const char_32 *key_str32,
                       LangModel *lang_mod,
                       CharSet *char_set) const {
  if (!key_str32)
    return 0;
  // convert string to UTF8 to split into space-separated words
  string key_str;
  CubeUtils::UTF32ToUTF8(key_str32, &key_str);
  vector<string> words;
  CubeUtils::SplitStringUsing(key_str, " \t", &words);

  // no words => no cost
  if (words.size() <= 0) {
    return 0;
  }

  // aggregate the costs of all the words
  int cost = 0;
  for (int word_idx = 0; word_idx < words.size(); word_idx++) {
    // convert each word back to UTF32 for analyzing case and punctuation
    string_32 str32;
    CubeUtils::UTF8ToUTF32(words[word_idx].c_str(), &str32);
    int len = CubeUtils::StrLen(str32.c_str());

    // strip all trailing punctuation
    string clean_str;
    int clean_len = len;
    bool trunc = false;
    while (clean_len > 0 &&
           lang_mod->IsTrailingPunc(str32.c_str()[clean_len - 1])) {
      --clean_len;
      trunc = true;
    }

    // If either the original string was not truncated (no trailing
    // punctuation) or the entire string was removed (all characters
    // are trailing punctuation), evaluate original word as is;
    // otherwise, copy all but the trailing punctuation characters
    char_32 *clean_str32 = NULL;
    if (clean_len == 0 || !trunc) {
      clean_str32 = CubeUtils::StrDup(str32.c_str());
    } else {
      clean_str32 = new char_32[clean_len + 1];
      for (int i = 0; i < clean_len; ++i) {
        clean_str32[i] = str32[i];
      }
      clean_str32[clean_len] = '\0';
    }
    ASSERT_HOST(clean_str32 != NULL);

    string str8;
    CubeUtils::UTF32ToUTF8(clean_str32, &str8);
    int word_cost = CostInternal(str8.c_str());

    // if case invariant, get costs of all-upper-case and all-lower-case
    // versions and return the min cost
    if (clean_len >= kMinLengthNumOrCaseInvariant &&
        CubeUtils::IsCaseInvariant(clean_str32, char_set)) {
      char_32 *lower_32 = CubeUtils::ToLower(clean_str32, char_set);
      if (lower_32) {
        string lower_8;
        CubeUtils::UTF32ToUTF8(lower_32, &lower_8);
        word_cost = MIN(word_cost, CostInternal(lower_8.c_str()));
        delete [] lower_32;
      }
      char_32 *upper_32 = CubeUtils::ToUpper(clean_str32, char_set);
      if (upper_32) {
        string upper_8;
        CubeUtils::UTF32ToUTF8(upper_32, &upper_8);
        word_cost = MIN(word_cost, CostInternal(upper_8.c_str()));
        delete [] upper_32;
      }
    }

    if (clean_len >= kMinLengthNumOrCaseInvariant) {
      // if characters are all numeric, incur 0 word cost
      bool is_numeric = true;
      for (int i = 0; i < clean_len; ++i) {
        if (!lang_mod->IsDigit(clean_str32[i]))
          is_numeric = false;
      }
      if (is_numeric)
        word_cost = 0;
    }
    delete [] clean_str32;
    cost += word_cost;
  }  // word_idx

  // return the mean cost
  return static_cast<int>(cost / static_cast<double>(words.size()));
}

// Search for UTF-8 string using binary search of sorted words_ array.
int WordUnigrams::CostInternal(const char *key_str) const {
  if (strlen(key_str) == 0)
    return not_in_list_cost_;
  int hi = word_cnt_ - 1;
  int lo = 0;
  while (lo <= hi) {
    int current = (hi + lo) / 2;
    int comp = strcmp(key_str, words_[current]);
    // a match
    if (comp == 0) {
      return costs_[current];
    }
    if (comp < 0) {
      // go lower
      hi = current - 1;
    } else {
      // go higher
      lo = current + 1;
    }
  }
  return not_in_list_cost_;
}
}  // namespace tesseract
