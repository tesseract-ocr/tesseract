/**********************************************************************
 * File:        char_bigrams.cpp
 * Description: Implementation of a Character Bigrams Class
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

#include <algorithm>
#include <math.h>
#include <string>
#include <vector>

#include "char_bigrams.h"
#include "cube_utils.h"
#include "ndminx.h"
#include "cube_const.h"

namespace tesseract {

CharBigrams::CharBigrams() {
  memset(&bigram_table_, 0, sizeof(bigram_table_));
}

CharBigrams::~CharBigrams() {
  if (bigram_table_.char_bigram != NULL) {
    for (int ch1 = 0; ch1 <= bigram_table_.max_char; ch1++) {
      CharBigram *char_bigram = bigram_table_.char_bigram + ch1;

      if (char_bigram->bigram != NULL) {
        delete []char_bigram->bigram;
      }
    }
    delete []bigram_table_.char_bigram;
  }
}

CharBigrams *CharBigrams::Create(const string &data_file_path,
                                 const string &lang) {
  string file_name;
  string str;

  file_name = data_file_path + lang;
  file_name += ".cube.bigrams";

  // load the string into memory
  if (!CubeUtils::ReadFileToString(file_name, &str)) {
    return NULL;
  }

  // construct a new object
  CharBigrams *char_bigrams_obj = new CharBigrams();
  if (char_bigrams_obj == NULL) {
    fprintf(stderr, "Cube ERROR (CharBigrams::Create): could not create "
            "character bigrams object.\n");
    return NULL;
  }
  CharBigramTable *table = &char_bigrams_obj->bigram_table_;

  table->total_cnt = 0;
  table->max_char = -1;
  table->char_bigram = NULL;

  // split into lines
  vector<string> str_vec;
  CubeUtils::SplitStringUsing(str, "\r\n", &str_vec);

  for (int big = 0; big < str_vec.size(); big++) {
    char_32 ch1;
    char_32 ch2;
    int cnt;
    if (sscanf(str_vec[big].c_str(), "%d %x %x", &cnt, &ch1, &ch2) != 3) {
      fprintf(stderr, "Cube ERROR (CharBigrams::Create): invalid format "
              "reading line: %s\n", str_vec[big].c_str());
      delete char_bigrams_obj;
      return NULL;
    }

    // expand the bigram table
    if (ch1 > table->max_char) {
      CharBigram *char_bigram = new CharBigram[ch1 + 1];
      if (char_bigram == NULL) {
        fprintf(stderr, "Cube ERROR (CharBigrams::Create): error allocating "
                "additional memory for character bigram table.\n");
        return NULL;
      }

      if (table->char_bigram != NULL && table->max_char >= 0) {
        memcpy(char_bigram, table->char_bigram,
          (table->max_char + 1) * sizeof(*char_bigram));

        delete []table->char_bigram;
      }
      table->char_bigram = char_bigram;

      // init
      for (int new_big = table->max_char + 1; new_big <= ch1; new_big++) {
        table->char_bigram[new_big].total_cnt = 0;
        table->char_bigram[new_big].max_char = -1;
        table->char_bigram[new_big].bigram = NULL;
      }
      table->max_char = ch1;
    }

    if (ch2 > table->char_bigram[ch1].max_char) {
      Bigram *bigram = new Bigram[ch2 + 1];
      if (bigram == NULL) {
        fprintf(stderr, "Cube ERROR (CharBigrams::Create): error allocating "
                "memory for bigram.\n");
        delete char_bigrams_obj;
        return NULL;
      }

      if (table->char_bigram[ch1].bigram != NULL &&
          table->char_bigram[ch1].max_char >= 0) {
        memcpy(bigram, table->char_bigram[ch1].bigram,
          (table->char_bigram[ch1].max_char + 1) * sizeof(*bigram));
        delete []table->char_bigram[ch1].bigram;
      }
      table->char_bigram[ch1].bigram = bigram;

      // init
      for (int new_big = table->char_bigram[ch1].max_char + 1;
           new_big <= ch2; new_big++) {
        table->char_bigram[ch1].bigram[new_big].cnt = 0;
      }
      table->char_bigram[ch1].max_char = ch2;
    }

    table->char_bigram[ch1].bigram[ch2].cnt = cnt;
    table->char_bigram[ch1].total_cnt += cnt;
    table->total_cnt += cnt;
  }

  // compute costs (-log probs)
  table->worst_cost = static_cast<int>(
      -PROB2COST_SCALE * log(0.5 / table->total_cnt));
  for (char_32 ch1 = 0; ch1 <= table->max_char; ch1++) {
    for (char_32 ch2 = 0; ch2 <= table->char_bigram[ch1].max_char; ch2++) {
      int cnt = table->char_bigram[ch1].bigram[ch2].cnt;
      table->char_bigram[ch1].bigram[ch2].cost =
          static_cast<int>(-PROB2COST_SCALE *
                           log(MAX(0.5, static_cast<double>(cnt)) /
                               table->total_cnt));
    }
  }
  return char_bigrams_obj;
}

int CharBigrams::PairCost(char_32 ch1, char_32 ch2) const {
  if (ch1 > bigram_table_.max_char) {
    return bigram_table_.worst_cost;
  }
  if (ch2 > bigram_table_.char_bigram[ch1].max_char) {
    return bigram_table_.worst_cost;
  }
  return bigram_table_.char_bigram[ch1].bigram[ch2].cost;
}

int CharBigrams::Cost(const char_32 *char_32_ptr, CharSet *char_set) const {
  if (!char_32_ptr || char_32_ptr[0] == 0) {
    return bigram_table_.worst_cost;
  }
  int cost = MeanCostWithSpaces(char_32_ptr);
  if (CubeUtils::StrLen(char_32_ptr) >= kMinLengthCaseInvariant &&
      CubeUtils::IsCaseInvariant(char_32_ptr, char_set)) {
    char_32 *lower_32 = CubeUtils::ToLower(char_32_ptr, char_set);
    if (lower_32 && lower_32[0] != 0) {
      int cost_lower = MeanCostWithSpaces(lower_32);
      cost = MIN(cost, cost_lower);
      delete [] lower_32;
    }
    char_32 *upper_32 = CubeUtils::ToUpper(char_32_ptr, char_set);
    if (upper_32 && upper_32[0] != 0) {
      int cost_upper = MeanCostWithSpaces(upper_32);
      cost = MIN(cost, cost_upper);
      delete [] upper_32;
    }
  }
  return cost;
}

int CharBigrams::MeanCostWithSpaces(const char_32 *char_32_ptr) const {
  if (!char_32_ptr)
    return bigram_table_.worst_cost;
  int len = CubeUtils::StrLen(char_32_ptr);
  int cost = 0;
  int c = 0;
  cost = PairCost(' ', char_32_ptr[0]);
  for (c = 1; c < len; c++) {
    cost += PairCost(char_32_ptr[c - 1], char_32_ptr[c]);
  }
  cost += PairCost(char_32_ptr[len - 1], ' ');
  return static_cast<int>(cost / static_cast<double>(len + 1));
}
}  // namespace tesseract
