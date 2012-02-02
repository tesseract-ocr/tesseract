/**********************************************************************
 * File:        char_bigrams.h
 * Description: Declaration of a Character Bigrams Class
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

// The CharBigram class represents the interface to the character bigram
// table used by Cube
// A CharBigram object can be constructed from the Char Bigrams file
// Given a sequence of characters, the "Cost" method returns the Char Bigram
// cost of the string according to the table

#ifndef CHAR_BIGRAMS_H
#define CHAR_BIGRAMS_H

#include <string>
#include "char_set.h"

namespace tesseract {

// structure representing a single bigram value
struct Bigram {
  int cnt;
  int cost;
};

// structure representing the char bigram array of characters
// following a specific character
struct CharBigram {
  int total_cnt;
  char_32 max_char;
  Bigram *bigram;
};

// structure representing the whole bigram table
struct CharBigramTable {
  int total_cnt;
  int worst_cost;
  char_32 max_char;
  CharBigram *char_bigram;
};

class CharBigrams {
 public:
  CharBigrams();
  ~CharBigrams();
  // Construct the CharBigrams class from a file
  static CharBigrams *Create(const string &data_file_path,
                             const string &lang);
  // Top-level function to return the mean character bigram cost of a
  // sequence of characters.  If char_set is not NULL, use
  // tesseract functions to return a case-invariant cost.
  // This avoids unnecessarily penalizing all-one-case words or
  // capitalized words (first-letter upper-case and remaining letters
  // lower-case).
  int Cost(const char_32 *str, CharSet *char_set) const;

 protected:
  // Returns the character bigram cost of two characters.
  int PairCost(char_32 ch1, char_32 ch2) const;
  // Returns the mean character bigram cost of a sequence of
  // characters. Adds a space at the beginning and end to account for
  // cost of starting and ending characters.
  int MeanCostWithSpaces(const char_32 *char_32_ptr) const;

 private:
  // Only words this length or greater qualify for case-invariant character
  // bigram cost.
  static const int kMinLengthCaseInvariant = 4;


  CharBigramTable bigram_table_;
};
}

#endif  // CHAR_BIGRAMS_H
