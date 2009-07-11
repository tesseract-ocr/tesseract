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

#ifndef TESSERACT_CCUTIL_UNICHARSET_H__
#define TESSERACT_CCUTIL_UNICHARSET_H__

#include "assert.h"
#include "strngs.h"
#include "unichar.h"
#include "unicharmap.h"
#include "varable.h"

class CHAR_FRAGMENT {
 public:
  // Minimum number of characters used for fragment representation.
  static const int kMinLen = 6;
  // Maximum number of characters used for fragment representation.
  static const int kMaxLen = 3 + UNICHAR_LEN + 2;
  // Special character used in representing character fragments.
  static const char kSeparator = '|';
  // Maximum number of fragments per character.
  static const int kMaxChunks = 3;

  // Setters and Getters.
  inline void set_all(const char *unichar, int pos, int total) {
    this->set_unichar(unichar);
    this->set_pos(pos);
    this->set_total(total);
  }
  inline void set_unichar(const char *uch) {
    strncpy(this->unichar, uch, UNICHAR_LEN);
    this->unichar[UNICHAR_LEN] = '\0';
  }
  inline void set_pos(int p) { this->pos = p; }
  inline void set_total(int t) { this->total = t; }
  inline const char* get_unichar() const { return this->unichar; }
  inline int get_pos() const { return this->pos; }
  inline int get_total() const { return this->total; }

  // Returns the string that represents a fragment
  // with the given unichar, pos and total.
  static STRING to_string(const char *unichar, int pos, int total) {
    STRING result = "";
    result += kSeparator;
    result += unichar;
    char buffer[kMaxLen];
    snprintf(buffer, kMaxLen, "%c%d%c%d", kSeparator, pos, kSeparator, total);
    result += buffer;
    return result;
  }
  // Returns the string that represents this fragment.
  STRING to_string() const {
    return to_string(this->unichar, this->pos, this->total);
  }

  // Checks whether a fragment has the same unichar,
  // position and total as the given inputs.
  inline bool equals(const char *other_unichar,
                     int other_pos, int other_total) const {
    return (strcmp(this->unichar, other_unichar) == 0 &&
            this->pos == other_pos && this->total == other_total);
  }
  inline bool equals(const CHAR_FRAGMENT *other) const {
    return this->equals(other->get_unichar(),
                        other->get_pos(),
                        other->get_total());
  }

  // Checks whether a given fragment is a continuation of this fragment.
  // Assumes that the given fragment pointer is not NULL.
  inline bool is_continuation_of(const CHAR_FRAGMENT *fragment) const {
    return (strcmp(this->unichar, fragment->get_unichar()) == 0 &&
            this->total == fragment->get_total() &&
            this->pos == fragment->get_pos() + 1);
  }

  // Returns true if this fragment is a beginning fragment.
  inline bool is_beginning() const { return this->pos == 0; }

  // Returns true if this fragment is an ending fragment.
  inline bool is_ending() const { return this->pos == this->total-1; }

  // Parses the string to see whether it represents a character fragment
  // (rather than a regular character). If so, allocates memory for a new
  // CHAR_FRAGMENT instance and fills it in with the corresponding fragment
  // information. Fragments are of the form:
  // |m|1|2, meaning chunk 1 of 2 of character m.
  //
  // If parsing succeeded returns the pointer to the allocated CHAR_FRAGMENT
  // instance, otherwise (if the string does not represent a fragment or it
  // looks like it does, but parsing it as a fragment fails) returns NULL.
  //
  // Note: The caller is responsible for deallocating memory
  // associated with the returned pointer.
  static CHAR_FRAGMENT *parse_from_string(const char *str);

 private:
  char unichar[UNICHAR_LEN + 1];
  inT16 pos;    // fragment position in the character
  inT16 total;  // total number of fragments in the character
};

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

  // Return a STRING that reformats the utf8 str into the str followed
  // by its hex unicodes.
  static STRING debug_utf8_str(const char* str);

  // Return a STRING containing debug information on the unichar, including
  // the id_to_unichar, its hex unicodes and the properties.
  STRING debug_str(UNICHAR_ID id) const;
  STRING debug_str(const char * unichar_repr) const {
    return debug_str(unichar_to_id(unichar_repr));
  }

  // Add a unichar representation to the set.
  void unichar_insert(const char* const unichar_repr);

  // Return true if the given unichar id exists within the set.
  // Relies on the fact that unichar ids are contiguous in the unicharset.
  bool contains_unichar_id(UNICHAR_ID unichar_id) const {
    return unichar_id != INVALID_UNICHAR_ID && unichar_id < size_used;
  }

  // Return true if the given unichar representation exists within the set.
  bool contains_unichar(const char* const unichar_repr) const;
  bool contains_unichar(const char* const unichar_repr, int length) const;

  // Return true if the given unichar representation corresponds to the given
  // UNICHAR_ID within the set.
  bool eq(UNICHAR_ID unichar_id, const char* const unichar_repr) const;

  // Delete CHAR_FRAGMENTs stored in properties of unichars array.
  void delete_pointers_in_unichars() {
    for (int i = 0; i < size_used; ++i) {
      if (unichars[i].properties.fragment != NULL) {
        delete unichars[i].properties.fragment;
        unichars[i].properties.fragment = NULL;
      }
    }
  }

  // Clear the UNICHARSET (all the previous data is lost).
  void clear() {
    if (size_reserved > 0) {
      for (int i = 0; i < script_table_size_used; ++i)
        delete[] script_table[i];
      delete[] script_table;
      script_table = 0;
      script_table_size_reserved = 0;
      script_table_size_used = 0;
      delete_pointers_in_unichars();
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

  // Opens the file indicated by filename and saves unicharset to that file.
  // Returns true if the operation is successful.
  bool save_to_file(const char * const filename) const {
    FILE* file = fopen(filename, "w+");
    if (file == NULL) return false;
    bool result = save_to_file(file);
    fclose(file);
    return result;
  }

  // Saves the content of the UNICHARSET to the given file.
  // Returns true if the operation is successful.
  bool save_to_file(FILE *file) const;

  // Opens the file indicated by filename and loads the UNICHARSET
  // from the given file. The previous data is lost.
  // Returns true if the operation is successful.
  bool load_from_file(const char* const filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) return false;
    bool result = load_from_file(file);
    fclose(file);
    return result;
  }

  // Loads the UNICHARSET from the given file. The previous data is lost.
  // Returns true if the operation is successful.
  bool load_from_file(FILE *file);

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

  // Set the ispunctuation property of the given unichar to the given value.
  void set_ispunctuation(UNICHAR_ID unichar_id, bool value) {
    unichars[unichar_id].properties.ispunctuation = value;
  }

  // Set the isngram property of the given unichar to the given value.
  void set_isngram(UNICHAR_ID unichar_id, bool value) {
    unichars[unichar_id].properties.isngram = value;
  }

  // Set the script name of the given unichar to the given value.
  // Value is copied and thus can be a temporary;
  void set_script(UNICHAR_ID unichar_id, const char* value) {
    unichars[unichar_id].properties.script_id = add_script(value);
  }

  // Set other_case unichar id in the properties for the given unichar id.
  void set_other_case(UNICHAR_ID unichar_id, UNICHAR_ID other_case) {
    unichars[unichar_id].properties.other_case = other_case;
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

  // Return the ispunctuation property of the given unichar.
  bool get_ispunctuation(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.ispunctuation;
  }

  // Return the isngram property of the given unichar.
  bool get_isngram(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.isngram;
  }

  // Return the script name of the given unichar.
  // The returned pointer will always be the same for the same script, it's
  // managed by unicharset and thus MUST NOT be deleted
  int get_script(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.script_id;
  }

  // Get other_case unichar id in the properties for the given unichar id.
  UNICHAR_ID get_other_case(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.other_case;
  }

  // Returns UNICHAR_ID of the corresponding lower-case unichar.
  UNICHAR_ID to_lower(UNICHAR_ID unichar_id) const {
    if (unichars[unichar_id].properties.islower) return unichar_id;
    return unichars[unichar_id].properties.other_case;
  }

  // Returns UNICHAR_ID of the corresponding upper-case unichar.
  UNICHAR_ID to_upper(UNICHAR_ID unichar_id) const {
    if (unichars[unichar_id].properties.isupper) return unichar_id;
    return unichars[unichar_id].properties.other_case;
  }

  // Return a pointer to the CHAR_FRAGMENT class if the given
  // unichar id represents a character fragment.
  const CHAR_FRAGMENT *get_fragment(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.fragment;
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

  // Return the ispunctuation property of the given unichar representation.
  bool get_ispunctuation(const char* const unichar_repr) const {
    return get_ispunctuation(unichar_to_id(unichar_repr));
  }

  // Return the script name of the given unichar representation.
  // The returned pointer will always be the same for the same script, it's
  // managed by unicharset and thus MUST NOT be deleted
  int get_script(const char* const unichar_repr) const {
    return get_script(unichar_to_id(unichar_repr));
  }

  // Return a pointer to the CHAR_FRAGMENT class struct if the given
  // unichar representation represents a character fragment.
  const CHAR_FRAGMENT *get_fragment(const char* const unichar_repr) const {
    if (unichar_repr == NULL || unichar_repr[0] == '\0' ||
        !ids.contains(unichar_repr)) {
      return NULL;
    }
    return get_fragment(unichar_to_id(unichar_repr));
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

  // Return the ispunctuation property of the given unichar representation.
  // Only the first length characters from unichar_repr are used.
  bool get_ispunctuation(const char* const unichar_repr,
                          int length) const {
    return get_ispunctuation(unichar_to_id(unichar_repr, length));
  }

  // Return the script name of the given unichar representation.
  // Only the first length characters from unichar_repr are used.
  // The returned pointer will always be the same for the same script, it's
  // managed by unicharset and thus MUST NOT be deleted
  int get_script(const char* const unichar_repr,
                 int length) const {
    return get_script(unichar_to_id(unichar_repr, length));
  }

  // Return the (current) number of scripts in the script table
  int get_script_table_size() const {
    return script_table_size_used;
  }

  // Return the script string from its id
  const char* get_script_from_script_id(int id) const {
    if (id >= script_table_size_used || id < 0)
      return null_script;
    return script_table[id];
  }

  // Returns the id from the name of the script, or 0 if script is not found.
  // Note that this is an expensive operation since it involves iteratively
  // comparing strings in the script table.  To avoid dependency on STL, we
  // won't use a hash.  Instead, the calling function can use this to lookup
  // and save the ID for relevant scripts for fast comparisons later.
  int get_script_id_from_name(const char* script_name) const;

  // Return true if the given script is the null script
  bool is_null_script(const char* script) const {
    return script == null_script;
  }

  // Uniquify the given script. For two scripts a and b, if strcmp(a, b) == 0,
  // then the returned pointer will be the same.
  // The script parameter is copied and thus can be a temporary.
  int add_script(const char* script);

  // Return the enabled property of the given unichar.
  bool get_enabled(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.enabled;
  }


  int null_sid() const { return null_sid_; }
  int common_sid() const { return common_sid_; }
  int latin_sid() const { return latin_sid_; }
  int cyrillic_sid() const { return cyrillic_sid_; }
  int greek_sid() const { return greek_sid_; }
  int han_sid() const { return han_sid_; }

 private:

  struct UNICHAR_PROPERTIES {
    bool  isalpha;
    bool  islower;
    bool  isupper;
    bool  isdigit;
    bool  ispunctuation;
    bool  isngram;
    bool  enabled;
    int   script_id;
    UNICHAR_ID other_case;  // id of the corresponding upper/lower case unichar

    // Contains meta information about the fragment if a unichar represents
    // a fragment of a character, otherwise should be set to NULL.
    // It is assumed that character fragments are added to the unicharset
    // after the corresponding 'base' characters.
    CHAR_FRAGMENT *fragment;
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

  // A few convenient script name-to-id mapping without using hash.
  // These are initialized when unicharset file is loaded.  Anything
  // missing from this list can be looked up using get_script_id_from_name.
  int null_sid_;
  int common_sid_;
  int latin_sid_;
  int cyrillic_sid_;
  int greek_sid_;
  int han_sid_;
};

#endif  // TESSERACT_CCUTIL_UNICHARSET_H__
