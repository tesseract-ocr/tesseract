/**********************************************************************
 * File:        char_samp_enum.h
 * Description: Declaration of a Character Set Class
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

// The CharSet class encapsulates the list of 32-bit strings/characters that
// Cube supports for a specific language. The char set is loaded from the
// .unicharset file corresponding to a specific language
// Each string has a corresponding int class-id that gets used throughout Cube
// The class provides pass back and forth conversion between the class-id
// and its corresponding 32-bit string. This is done using a hash table that
// maps the string to the class id.

#ifndef CHAR_SET_H
#define CHAR_SET_H

#include <string.h>
#include <string>
#include <algorithm>

#include "string_32.h"
#include "tessdatamanager.h"
#include "unicharset.h"
#include "cube_const.h"

namespace tesseract {

class CharSet {
 public:
  CharSet();
  ~CharSet();

  // Returns true if Cube is sharing Tesseract's unicharset.
  inline bool SharedUnicharset() { return (unicharset_map_ == NULL); }

  // Returns the class id corresponding to a 32-bit string. Returns -1
  // if the string is not supported. This is done by hashing the
  // string and then looking up the string in the hash-bin if there
  // are collisions.
  inline int ClassID(const char_32 *str) const {
    int hash_val = Hash(str);
    if (hash_bin_size_[hash_val] == 0)
      return -1;
    for (int bin = 0; bin < hash_bin_size_[hash_val]; bin++) {
      if (class_strings_[hash_bins_[hash_val][bin]]->compare(str) == 0)
        return hash_bins_[hash_val][bin];
    }
    return -1;
  }
  // Same as above but using a 32-bit char instead of a string
  inline int ClassID(char_32 ch) const {
    int hash_val = Hash(ch);
    if (hash_bin_size_[hash_val] == 0)
      return -1;
    for (int bin = 0; bin < hash_bin_size_[hash_val]; bin++) {
      if ((*class_strings_[hash_bins_[hash_val][bin]])[0] == ch &&
          class_strings_[hash_bins_[hash_val][bin]]->length() == 1) {
        return hash_bins_[hash_val][bin];
      }
    }
    return -1;
  }
  // Retrieve the unicharid in Tesseract's unicharset corresponding
  // to a 32-bit string. When Tesseract and Cube share the same
  // unicharset, this will just be the class id.
  inline int UnicharID(const char_32 *str) const {
    int class_id = ClassID(str);
    if (class_id == INVALID_UNICHAR_ID)
      return INVALID_UNICHAR_ID;
    int unichar_id;
    if (unicharset_map_)
      unichar_id = unicharset_map_[class_id];
    else
      unichar_id = class_id;
    return unichar_id;
  }
  // Same as above but using a 32-bit char instead of a string
  inline int UnicharID(char_32 ch) const {
    int class_id = ClassID(ch);
    if (class_id == INVALID_UNICHAR_ID)
      return INVALID_UNICHAR_ID;
    int unichar_id;
    if (unicharset_map_)
      unichar_id = unicharset_map_[class_id];
    else
      unichar_id = class_id;
    return unichar_id;
  }
  // Returns the 32-bit string corresponding to a class id
  inline const char_32 * ClassString(int class_id) const {
    if (class_id < 0 || class_id >= class_cnt_) {
      return NULL;
    }
    return reinterpret_cast<const char_32 *>(class_strings_[class_id]->c_str());
  }
  // Returns the count of supported strings
  inline int ClassCount() const { return class_cnt_; }

  // Creates CharSet object by reading the unicharset from the
  // TessDatamanager, and mapping Cube's unicharset to Tesseract's if
  // they differ.
  static CharSet *Create(TessdataManager *tessdata_manager,
                         UNICHARSET *tess_unicharset);

  // Return the UNICHARSET cube is using for recognition internally --
  // ClassId() returns unichar_id's in this unicharset.
  UNICHARSET *InternalUnicharset() { return unicharset_; }

 private:
  // Hash table configuration params. Determined emperically on
  // the supported languages so far (Eng, Ara, Hin). Might need to be
  // tuned for speed when more languages are supported
  static const int kHashBins = 3001;
  static const int kMaxHashSize = 16;

  // Using djb2 hashing function to hash a 32-bit string
  // introduced in http://www.cse.yorku.ca/~oz/hash.html
  static inline int Hash(const char_32 *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
      hash = ((hash << 5) + hash) + c;
    return (hash%kHashBins);
  }
  // Same as above but for a single char
  static inline int Hash(char_32 ch) {
    char_32 b[2];
    b[0] = ch;
    b[1] = 0;
    return Hash(b);
  }

  // Load the list of supported chars from the given data file
  // pointer. If tess_unicharset is non-NULL, mapping each Cube class
  // id to a tesseract unicharid.
  bool LoadSupportedCharList(FILE *fp, UNICHARSET *tess_unicharset);

  // class count
  int class_cnt_;
  // hash-bin sizes array
  int hash_bin_size_[kHashBins];
  // hash bins
  int hash_bins_[kHashBins][kMaxHashSize];
  // supported strings array
  string_32  **class_strings_;
  // map from class id to secondary (tesseract's) unicharset's ids
  int *unicharset_map_;
  // A unicharset which is filled in with a Tesseract-style UNICHARSET for
  // cube's data if our unicharset is different from tesseract's.
  UNICHARSET cube_unicharset_;
  // This points to either the tess_unicharset we're passed or cube_unicharset_,
  // depending upon whether we just have one unicharset or one for each
  // tesseract and cube, respectively.
  UNICHARSET *unicharset_;
  // has the char set been initialized flag
  bool init_;
};
}

#endif  // CHAR_SET_H
