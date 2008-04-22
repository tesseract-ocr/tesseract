///////////////////////////////////////////////////////////////////////
// File:        unicharset.h
// Description: Unicode character/ligature set class.
// Author:      Thomas Kielbus
// Created:     Wed Jun 28 17:05:01 PDT 2006
//
// (C) Copyright 2006, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef THIRD_PARTY_TESSERACT_CCUTIL_UNICHARSET_H__
#define THIRD_PARTY_TESSERACT_CCUTIL_UNICHARSET_H__

#include "strngs.h"
#include "unichar.h"
#include "unicharmap.h"

// The UNICHARSET class is an utility class for Tesseract that holds the
// set of characters that are used by the engine. Each character is identified
// by a unique number, from 0 to (size - 1).
class UNICHARSET {
 public:

  // Create an empty UNICHARSET
  UNICHARSET();

  ~UNICHARSET();

  // Return the UNICHAR_ID of a given unichar representation within the
  // UNICHARSET.
  const UNICHAR_ID unichar_to_id(const char* const unichar_repr) const;

  // Return the UNICHAR_ID of a given unichar representation within the
  // UNICHARSET. Only the first length characters from unichar_repr are used.
  const UNICHAR_ID unichar_to_id(const char* const unichar_repr,
                                 int length) const;

  // Return the minimum number of bytes that matches a legal UNICHAR_ID,
  // while leaving a legal UNICHAR_ID afterwards. In other words, if there
  // is both a short and a long match to the string, return the length that
  // ensures there is a legal match after it.
  int step(const char* str) const;

  // Return the unichar representation corresponding to the given UNICHAR_ID
  // within the UNICHARSET.
  const char* const id_to_unichar(UNICHAR_ID id) const;

  // Return a STRING containing debug information on the unichar, including
  // the id_to_unichar, its hex unicodes and the properties.
  STRING debug_str(UNICHAR_ID id) const;

  // Add a unichar representation to the set.
  void unichar_insert(const char* const unichar_repr);

  // Return true if the given unichar representation exists within the set.
  bool contains_unichar(const char* const unichar_repr);
  bool contains_unichar(const char* const unichar_repr, int length);

  // Return true if the given unichar representation corresponds to the given
  // UNICHAR_ID within the set.
  bool eq(UNICHAR_ID unichar_id, const char* const unichar_repr);

  // Clear the UNICHARSET (all the previous data is lost).
  void clear() {
    if (size_reserved > 0) {
      for (int i = 0; i < script_table_size_used; ++i)
        delete[] script_table[i];
      delete[] script_table;
      script_table = 0;
      script_table_size_reserved = 0;
      script_table_size_used = 0;
      delete[] unichars;
      unichars = 0;
      size_reserved = 0;
      size_used = 0;
    }
    ids.clear();
  }

  // Return the size of the set (the number of different UNICHAR it holds).
  int size() const {
    return size_used;
  }

  // Reserve enough memory space for the given number of UNICHARS
  void reserve(int unichars_number);

  // Save the content of the UNICHARSET to the given file. Return true if the
  // operation is successful.
  bool save_to_file(const char* const filename) const;

  // Load the UNICHARSET from the given file. The previous data is lost. Return
  // true if the operation is successful.
  bool load_from_file(const char* const filename);

  // Set a whitelist and/or blacklist of characters to recognize.
  // An empty or NULL whitelist enables everything (minus any blacklist).
  // An empty or NULL blacklist disables nothing.
  // The blacklist overrides the whitelist.
  // Each list is a string of utf8 character strings. Boundaries between
  // unicharset units are worked out automatically, and characters not in
  // the unicharset are silently ignored.
  void set_black_and_whitelist(const char* blacklist, const char* whitelist);

  // Set the isalpha property of the given unichar to the given value.
  void set_isalpha(UNICHAR_ID unichar_id, bool value) {
    unichars[unichar_id].properties.isalpha = value;
  }

  // Set the islower property of the given unichar to the given value.
  void set_islower(UNICHAR_ID unichar_id, bool value) {
    unichars[unichar_id].properties.islower = value;
  }

  // Set the isupper property of the given unichar to the given value.
  void set_isupper(UNICHAR_ID unichar_id, bool value) {
    unichars[unichar_id].properties.isupper = value;
  }

  // Set the isdigit property of the given unichar to the given value.
  void set_isdigit(UNICHAR_ID unichar_id, bool value) {
    unichars[unichar_id].properties.isdigit = value;
  }

  // Set the script name of the given unichar to the given value.
  // Value is copied and thus can be a temporary;
  void set_script(UNICHAR_ID unichar_id, const char* value) {
    unichars[unichar_id].properties.script = add_script(value);
  }

  // Return the isalpha property of the given unichar.
  bool get_isalpha(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.isalpha;
  }

  // Return the islower property of the given unichar.
  bool get_islower(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.islower;
  }

  // Return the isupper property of the given unichar.
  bool get_isupper(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.isupper;
  }

  // Return the isdigit property of the given unichar.
  bool get_isdigit(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.isdigit;
  }

  // Return the script name of the given unichar.
  // The returned pointer will always be the same for the same script, it's
  // managed by unicharset and thus MUST NOT be deleted
  const char* get_script(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.script;
  }

  // Return the isalpha property of the given unichar representation.
  bool get_isalpha(const char* const unichar_repr) const {
    return get_isalpha(unichar_to_id(unichar_repr));
  }

  // Return the islower property of the given unichar representation.
  bool get_islower(const char* const unichar_repr) const {
    return get_islower(unichar_to_id(unichar_repr));
  }

  // Return the isupper property of the given unichar representation.
  bool get_isupper(const char* const unichar_repr) const {
    return get_isupper(unichar_to_id(unichar_repr));
  }

  // Return the isdigit property of the given unichar representation.
  bool get_isdigit(const char* const unichar_repr) const {
    return get_isdigit(unichar_to_id(unichar_repr));
  }

  // Return the script name of the given unichar representation.
  // The returned pointer will always be the same for the same script, it's
  // managed by unicharset and thus MUST NOT be deleted
  const char* get_script(const char* const unichar_repr) const {
    return get_script(unichar_to_id(unichar_repr));
  }

  // Return the isalpha property of the given unichar representation.
  // Only the first length characters from unichar_repr are used.
  bool get_isalpha(const char* const unichar_repr,
               int length) const {
    return get_isalpha(unichar_to_id(unichar_repr, length));
  }

  // Return the islower property of the given unichar representation.
  // Only the first length characters from unichar_repr are used.
  bool get_islower(const char* const unichar_repr,
               int length) const {
    return get_islower(unichar_to_id(unichar_repr, length));
  }

  // Return the isupper property of the given unichar representation.
  // Only the first length characters from unichar_repr are used.
  bool get_isupper(const char* const unichar_repr,
               int length) const {
    return get_isupper(unichar_to_id(unichar_repr, length));
  }

  // Return the isdigit property of the given unichar representation.
  // Only the first length characters from unichar_repr are used.
  bool get_isdigit(const char* const unichar_repr,
               int length) const {
    return get_isdigit(unichar_to_id(unichar_repr, length));
  }

  // Return the script name of the given unichar representation.
  // Only the first length characters from unichar_repr are used.
  // The returned pointer will always be the same for the same script, it's
  // managed by unicharset and thus MUST NOT be deleted
  const char* get_script(const char* const unichar_repr,
               int length) const {
    return get_script(unichar_to_id(unichar_repr, length));
  }

  // Return the enabled property of the given unichar.
  bool get_enabled(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.enabled;
  }

 private:

  // Uniquify the given script. For two scripts a and b, if strcmp(a, b) == 0,
  // then the returned pointer will be the same.
  // The script parameter is copied and thus can be a temporary.
  char* add_script(const char* script);

  struct UNICHAR_PROPERTIES {
    bool  isalpha;
    bool  islower;
    bool  isupper;
    bool  isdigit;
    bool  enabled;
    char* script;
  };

  struct UNICHAR_SLOT {
    char representation[UNICHAR_LEN + 1];
    UNICHAR_PROPERTIES properties;
  };

  UNICHAR_SLOT* unichars;
  UNICHARMAP ids;
  int size_used;
  int size_reserved;
  char** script_table;
  int script_table_size_used;
  int script_table_size_reserved;
  const char* null_script;
};

#endif  // THIRD_PARTY_TESSERACT_CCUTIL_UNICHARSET_H__
