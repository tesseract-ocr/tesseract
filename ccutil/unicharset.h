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

#include "errcode.h"
#include "genericvector.h"
#include "helpers.h"
#include "strngs.h"
#include "tesscallback.h"
#include "unichar.h"
#include "unicharmap.h"

// Enum holding special values of unichar_id. Every unicharset has these.
// Warning! Keep in sync with kSpecialUnicharCodes.
enum SpecialUnicharCodes {
  UNICHAR_SPACE,
  UNICHAR_JOINED,
  UNICHAR_BROKEN,

  SPECIAL_UNICHAR_CODES_COUNT
};

class CHAR_FRAGMENT {
 public:
  // Minimum number of characters used for fragment representation.
  static const int kMinLen = 6;
  // Maximum number of characters used for fragment representation.
  static const int kMaxLen = 3 + UNICHAR_LEN + 2;
  // Maximum number of fragments per character.
  static const int kMaxChunks = 5;

  // Setters and Getters.
  inline void set_all(const char *unichar, int pos, int total, bool natural) {
    set_unichar(unichar);
    set_pos(pos);
    set_total(total);
    set_natural(natural);
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
  static STRING to_string(const char *unichar, int pos, int total,
                          bool natural);
  // Returns the string that represents this fragment.
  STRING to_string() const {
    return to_string(unichar, pos, total, natural);
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

  // Returns true if the fragment was a separate component to begin with,
  // ie did not need chopping to be isolated, but may have been separated
  // out from a multi-outline blob.
  inline bool is_natural() const { return natural; }
  void set_natural(bool value) { natural = value; }

  // Parses the string to see whether it represents a character fragment
  // (rather than a regular character). If so, allocates memory for a new
  // CHAR_FRAGMENT instance and fills it in with the corresponding fragment
  // information. Fragments are of the form:
  // |m|1|2, meaning chunk 1 of 2 of character m, or
  // |:|1n2, meaning chunk 1 of 2 of character :, and no chopping was needed
  // to divide the parts, as they were already separate connected components.
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
  // True if the fragment was a separate component to begin with,
  // ie did not need chopping to be isolated, but may have been separated
  // out from a multi-outline blob.
  bool natural;
  inT16 pos;    // fragment position in the character
  inT16 total;  // total number of fragments in the character
};

// The UNICHARSET class is an utility class for Tesseract that holds the
// set of characters that are used by the engine. Each character is identified
// by a unique number, from 0 to (size - 1).
class UNICHARSET {
 public:
  // Custom list of characters and their ligature forms (UTF8)
  // These map to unicode values in the private use area (PUC) and are supported
  // by only few font families (eg. Wyld, Adobe Caslon Pro).
  static const char* kCustomLigatures[][2];

  // List of strings for the SpecialUnicharCodes. Keep in sync with the enum.
  static const char* kSpecialUnicharCodes[SPECIAL_UNICHAR_CODES_COUNT];

  // ICU 2.0 UCharDirection enum (from third_party/icu/include/unicode/uchar.h)
  enum Direction {
      U_LEFT_TO_RIGHT               = 0,
      U_RIGHT_TO_LEFT               = 1,
      U_EUROPEAN_NUMBER             = 2,
      U_EUROPEAN_NUMBER_SEPARATOR   = 3,
      U_EUROPEAN_NUMBER_TERMINATOR  = 4,
      U_ARABIC_NUMBER               = 5,
      U_COMMON_NUMBER_SEPARATOR     = 6,
      U_BLOCK_SEPARATOR             = 7,
      U_SEGMENT_SEPARATOR           = 8,
      U_WHITE_SPACE_NEUTRAL         = 9,
      U_OTHER_NEUTRAL               = 10,
      U_LEFT_TO_RIGHT_EMBEDDING     = 11,
      U_LEFT_TO_RIGHT_OVERRIDE      = 12,
      U_RIGHT_TO_LEFT_ARABIC        = 13,
      U_RIGHT_TO_LEFT_EMBEDDING     = 14,
      U_RIGHT_TO_LEFT_OVERRIDE      = 15,
      U_POP_DIRECTIONAL_FORMAT      = 16,
      U_DIR_NON_SPACING_MARK        = 17,
      U_BOUNDARY_NEUTRAL            = 18,
      U_CHAR_DIRECTION_COUNT
  };

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
  // while leaving the rest of the string encodable. Returns 0 if the
  // beginning of the string is not encodable.
  // WARNING: this function now encodes the whole string for precision.
  // Use encode_string in preference to repeatedly calling step.
  int step(const char* str) const;
  // As step except constraining the search to unichar-ids that are
  // self-normalized. Unlike step, does not encode the whole string, therefore
  // should be used on short strings (like those obtained from
  // get_normed_unichar.)
  int normed_step(const char* str) const;

  // Return whether the given UTF-8 string is encodable with this UNICHARSET.
  // If not encodable, write the first byte offset which cannot be converted
  // into the second (return) argument.
  bool encodable_string(const char *str, int *first_bad_position) const;

  // Encodes the given UTF-8 string with this UNICHARSET.
  // Any part of the string that cannot be encoded (because the utf8 can't
  // be broken up into pieces that are in the unicharset) then:
  // if give_up_on_failure, stops and returns a partial encoding,
  // else continues and inserts an INVALID_UNICHAR_ID in the returned encoding.
  // Returns true if the encoding succeeds completely, false if there is at
  // least one failure.
  // If lengths is not NULL, then it is filled with the corresponding
  // byte length of each encoded UNICHAR_ID.
  // If encoded_length is not NULL then on return it contains the length of
  // str that was encoded. (if give_up_on_failure the location of the first
  // failure, otherwise strlen(str).)
  bool encode_string(const char* str, bool give_up_on_failure,
                     GenericVector<UNICHAR_ID>* encoding,
                     GenericVector<char>* lengths,
                     int* encoded_length) const;

  // Return the unichar representation corresponding to the given UNICHAR_ID
  // within the UNICHARSET.
  const char* const id_to_unichar(UNICHAR_ID id) const;

  // Return the UTF8 representation corresponding to the given UNICHAR_ID after
  // resolving any private encodings internal to Tesseract. This method is
  // preferrable to id_to_unichar for outputting text that will be visible to
  // external applications.
  const char* const id_to_unichar_ext(UNICHAR_ID id) const;

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
    return unichar_id != INVALID_UNICHAR_ID && unichar_id < size_used &&
        unichar_id >= 0;
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
    if (script_table != NULL) {
      for (int i = 0; i < script_table_size_used; ++i)
        delete[] script_table[i];
      delete[] script_table;
      script_table = NULL;
      script_table_size_used = 0;
    }
    if (unichars != NULL) {
      delete_pointers_in_unichars();
      delete[] unichars;
      unichars = NULL;
    }
    script_table_size_reserved = 0;
    size_reserved = 0;
    size_used = 0;
    ids.clear();
    top_bottom_set_ = false;
    script_has_upper_lower_ = false;
    script_has_xheight_ = false;
    null_sid_ = 0;
    common_sid_ = 0;
    latin_sid_ = 0;
    cyrillic_sid_ = 0;
    greek_sid_ = 0;
    han_sid_ = 0;
    hiragana_sid_ = 0;
    katakana_sid_ = 0;
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
    FILE* file = fopen(filename, "w+b");
    if (file == NULL) return false;
    bool result = save_to_file(file);
    fclose(file);
    return result;
  }

  // Saves the content of the UNICHARSET to the given file.
  // Returns true if the operation is successful.
  bool save_to_file(FILE *file) const;

  // Load a unicharset from a unicharset file that has been loaded into
  // the given memory buffer.
  // Returns true if the operation is successful.
  bool load_from_inmemory_file(const char* const memory, int mem_size,
                               bool skip_fragments);
  // Returns true if the operation is successful.
  bool load_from_inmemory_file(const char* const memory, int mem_size) {
    return load_from_inmemory_file(memory, mem_size, false);
  }

  // Opens the file indicated by filename and loads the UNICHARSET
  // from the given file. The previous data is lost.
  // Returns true if the operation is successful.
  bool load_from_file(const char* const filename, bool skip_fragments) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) return false;
    bool result = load_from_file(file, skip_fragments);
    fclose(file);
    return result;
  }
  // returns true if the operation is successful.
  bool load_from_file(const char* const filename) {
    return load_from_file(filename, false);
  }

  // Loads the UNICHARSET from the given file. The previous data is lost.
  // Returns true if the operation is successful.
  bool load_from_file(FILE *file, bool skip_fragments);
  bool load_from_file(FILE *file) { return load_from_file(file, false); }

  // Sets up internal data after loading the file, based on the char
  // properties. Called from load_from_file, but also needs to be run
  // during set_unicharset_properties.
  void post_load_setup();

  // Returns true if right_to_left scripts are significant in the unicharset,
  // but without being so sensitive that "universal" unicharsets containing
  // characters from many scripts, like orientation and script detection,
  // look like they are right_to_left.
  bool major_right_to_left() const;

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

  // Set the direction property of the given unichar to the given value.
  void set_direction(UNICHAR_ID unichar_id, UNICHARSET::Direction value) {
    unichars[unichar_id].properties.direction = value;
  }

  // Set mirror unichar id in the properties for the given unichar id.
  void set_mirror(UNICHAR_ID unichar_id, UNICHAR_ID mirror) {
    unichars[unichar_id].properties.mirror = mirror;
  }

  // Record normalized version of unichar with the given unichar_id.
  void set_normed(UNICHAR_ID unichar_id, const char* normed) {
    unichars[unichar_id].properties.normed = normed;
    unichars[unichar_id].properties.normed_ids.truncate(0);
  }
  // Sets the normed_ids vector from the normed string. normed_ids is not
  // stored in the file, and needs to be set when the UNICHARSET is loaded.
  void set_normed_ids(UNICHAR_ID unichar_id);

  // Return the isalpha property of the given unichar.
  bool get_isalpha(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return false;
    ASSERT_HOST(contains_unichar_id(unichar_id));
    return unichars[unichar_id].properties.isalpha;
  }

  // Return the islower property of the given unichar.
  bool get_islower(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return false;
    ASSERT_HOST(contains_unichar_id(unichar_id));
    return unichars[unichar_id].properties.islower;
  }

  // Return the isupper property of the given unichar.
  bool get_isupper(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return false;
    ASSERT_HOST(contains_unichar_id(unichar_id));
    return unichars[unichar_id].properties.isupper;
  }

  // Return the isdigit property of the given unichar.
  bool get_isdigit(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return false;
    ASSERT_HOST(contains_unichar_id(unichar_id));
    return unichars[unichar_id].properties.isdigit;
  }

  // Return the ispunctuation property of the given unichar.
  bool get_ispunctuation(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return false;
    ASSERT_HOST(contains_unichar_id(unichar_id));
    return unichars[unichar_id].properties.ispunctuation;
  }

  // Return the isngram property of the given unichar.
  bool get_isngram(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return false;
    ASSERT_HOST(contains_unichar_id(unichar_id));
    return unichars[unichar_id].properties.isngram;
  }

  // Returns whether the unichar id represents a unicode value in the private
  // use area.
  bool get_isprivate(UNICHAR_ID unichar_id) const;

  // Returns true if the ids have useful min/max top/bottom values.
  bool top_bottom_useful() const {
    return top_bottom_set_;
  }
  // Sets all ranges to empty, so they can be expanded to set the values.
  void set_ranges_empty();
  // Sets all the properties for this unicharset given a src_unicharset with
  // everything set. The unicharsets don't have to be the same, and graphemes
  // are correctly accounted for.
  void SetPropertiesFromOther(const UNICHARSET& src) {
    PartialSetPropertiesFromOther(0, src);
  }
  // Sets properties from Other, starting only at the given index.
  void PartialSetPropertiesFromOther(int start_index, const UNICHARSET& src);
  // Expands the tops and bottoms and widths for this unicharset given a
  // src_unicharset with ranges in it. The unicharsets don't have to be the
  // same, and graphemes are correctly accounted for.
  void ExpandRangesFromOther(const UNICHARSET& src);
  // Makes this a copy of src. Clears this completely first, so the automattic
  // ids will not be present in this if not in src.
  void CopyFrom(const UNICHARSET& src);
  // For each id in src, if it does not occur in this, add it, as in
  // SetPropertiesFromOther, otherwise expand the ranges, as in
  // ExpandRangesFromOther.
  void AppendOtherUnicharset(const UNICHARSET& src);
  // Returns true if the acceptable ranges of the tops of the characters do
  // not overlap, making their x-height calculations distinct.
  bool SizesDistinct(UNICHAR_ID id1, UNICHAR_ID id2) const;
  // Returns the min and max bottom and top of the given unichar in
  // baseline-normalized coordinates, ie, where the baseline is
  // kBlnBaselineOffset and the meanline is kBlnBaselineOffset + kBlnXHeight
  // (See normalis.h for the definitions).
  void get_top_bottom(UNICHAR_ID unichar_id,
                      int* min_bottom, int* max_bottom,
                      int* min_top, int* max_top) const {
    if (INVALID_UNICHAR_ID == unichar_id) {
      *min_bottom = *min_top = 0;
      *max_bottom = *max_top = 256;  // kBlnCellHeight
      return;
    }
    ASSERT_HOST(contains_unichar_id(unichar_id));
    *min_bottom = unichars[unichar_id].properties.min_bottom;
    *max_bottom = unichars[unichar_id].properties.max_bottom;
    *min_top = unichars[unichar_id].properties.min_top;
    *max_top = unichars[unichar_id].properties.max_top;
  }
  void set_top_bottom(UNICHAR_ID unichar_id,
                      int min_bottom, int max_bottom,
                      int min_top, int max_top) {
    unichars[unichar_id].properties.min_bottom =
        static_cast<uinT8>(ClipToRange(min_bottom, 0, MAX_UINT8));
    unichars[unichar_id].properties.max_bottom =
        static_cast<uinT8>(ClipToRange(max_bottom, 0, MAX_UINT8));
    unichars[unichar_id].properties.min_top =
        static_cast<uinT8>(ClipToRange(min_top, 0, MAX_UINT8));
    unichars[unichar_id].properties.max_top =
        static_cast<uinT8>(ClipToRange(max_top, 0, MAX_UINT8));
  }
  // Returns the width range of the given unichar in baseline-normalized
  // coordinates, ie, where the baseline is kBlnBaselineOffset and the
  // meanline is kBlnBaselineOffset + kBlnXHeight.
  // (See normalis.h for the definitions).
  void get_width_range(UNICHAR_ID unichar_id,
                       int* min_width, int* max_width) const {
    if (INVALID_UNICHAR_ID == unichar_id) {
      *min_width = 0;
      *max_width = 256;  // kBlnCellHeight;
      return;
    }
    ASSERT_HOST(contains_unichar_id(unichar_id));
    *min_width = unichars[unichar_id].properties.min_width;
    *max_width = unichars[unichar_id].properties.max_width;
  }
  void set_width_range(UNICHAR_ID unichar_id, int min_width, int max_width) {
    unichars[unichar_id].properties.min_width =
        static_cast<inT16>(ClipToRange(min_width, 0, MAX_INT16));
    unichars[unichar_id].properties.max_width =
        static_cast<inT16>(ClipToRange(max_width, 0, MAX_INT16));
  }
  // Returns the range of the x-bearing of the given unichar in
  // baseline-normalized coordinates, ie, where the baseline is
  // kBlnBaselineOffset and the meanline is kBlnBaselineOffset + kBlnXHeight.
  // (See normalis.h for the definitions).
  void get_bearing_range(UNICHAR_ID unichar_id,
                         int* min_bearing, int* max_bearing) const {
    if (INVALID_UNICHAR_ID == unichar_id) {
      *min_bearing = *max_bearing = 0;
      return;
    }
    ASSERT_HOST(contains_unichar_id(unichar_id));
    *min_bearing = unichars[unichar_id].properties.min_bearing;
    *max_bearing = unichars[unichar_id].properties.max_bearing;
  }
  void set_bearing_range(UNICHAR_ID unichar_id,
                         int min_bearing, int max_bearing) {
    unichars[unichar_id].properties.min_bearing =
        static_cast<inT16>(ClipToRange(min_bearing, 0, MAX_INT16));
    unichars[unichar_id].properties.max_bearing =
        static_cast<inT16>(ClipToRange(max_bearing, 0, MAX_INT16));
  }
  // Returns the range of the x-advance of the given unichar in
  // baseline-normalized coordinates, ie, where the baseline is
  // kBlnBaselineOffset and the meanline is kBlnBaselineOffset + kBlnXHeight.
  // (See normalis.h for the definitions).
  void get_advance_range(UNICHAR_ID unichar_id,
                         int* min_advance, int* max_advance) const {
    if (INVALID_UNICHAR_ID == unichar_id) {
      *min_advance = *max_advance = 0;
      return;
    }
    ASSERT_HOST(contains_unichar_id(unichar_id));
    *min_advance = unichars[unichar_id].properties.min_advance;
    *max_advance = unichars[unichar_id].properties.max_advance;
  }
  void set_advance_range(UNICHAR_ID unichar_id,
                         int min_advance, int max_advance) {
    unichars[unichar_id].properties.min_advance =
        static_cast<inT16>(ClipToRange(min_advance, 0, MAX_INT16));
    unichars[unichar_id].properties.max_advance =
        static_cast<inT16>(ClipToRange(max_advance, 0, MAX_INT16));
  }

  // Return the script name of the given unichar.
  // The returned pointer will always be the same for the same script, it's
  // managed by unicharset and thus MUST NOT be deleted
  int get_script(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return null_sid_;
    ASSERT_HOST(contains_unichar_id(unichar_id));
    return unichars[unichar_id].properties.script_id;
  }

  // Return the character properties, eg. alpha/upper/lower/digit/punct,
  // as a bit field of unsigned int.
  unsigned int get_properties(UNICHAR_ID unichar_id) const;

  // Return the character property as a single char.  If a character has
  // multiple attributes, the main property is defined by the following order:
  //   upper_case : 'A'
  //   lower_case : 'a'
  //   alpha      : 'x'
  //   digit      : '0'
  //   punctuation: 'p'
  char get_chartype(UNICHAR_ID unichar_id) const;

  // Get other_case unichar id in the properties for the given unichar id.
  UNICHAR_ID get_other_case(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return INVALID_UNICHAR_ID;
    ASSERT_HOST(contains_unichar_id(unichar_id));
    return unichars[unichar_id].properties.other_case;
  }

  // Returns the direction property of the given unichar.
  Direction get_direction(UNICHAR_ID unichar_id) const {
     if (INVALID_UNICHAR_ID == unichar_id) return UNICHARSET::U_OTHER_NEUTRAL;
     ASSERT_HOST(contains_unichar_id(unichar_id));
     return unichars[unichar_id].properties.direction;
   }

  // Get mirror unichar id in the properties for the given unichar id.
  UNICHAR_ID get_mirror(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return INVALID_UNICHAR_ID;
    ASSERT_HOST(contains_unichar_id(unichar_id));
    return unichars[unichar_id].properties.mirror;
  }

  // Returns UNICHAR_ID of the corresponding lower-case unichar.
  UNICHAR_ID to_lower(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return INVALID_UNICHAR_ID;
    ASSERT_HOST(contains_unichar_id(unichar_id));
    if (unichars[unichar_id].properties.islower) return unichar_id;
    return unichars[unichar_id].properties.other_case;
  }

  // Returns UNICHAR_ID of the corresponding upper-case unichar.
  UNICHAR_ID to_upper(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return INVALID_UNICHAR_ID;
    ASSERT_HOST(contains_unichar_id(unichar_id));
    if (unichars[unichar_id].properties.isupper) return unichar_id;
    return unichars[unichar_id].properties.other_case;
  }

  // Returns true if this UNICHARSET has the special codes in
  // SpecialUnicharCodes available. If false then there are normal unichars
  // at these codes and they should not be used.
  bool has_special_codes() const {
    return get_fragment(UNICHAR_BROKEN) != NULL &&
        strcmp(id_to_unichar(UNICHAR_BROKEN),
               kSpecialUnicharCodes[UNICHAR_BROKEN]) == 0;
  }

  // Return a pointer to the CHAR_FRAGMENT class if the given
  // unichar id represents a character fragment.
  const CHAR_FRAGMENT *get_fragment(UNICHAR_ID unichar_id) const {
    if (INVALID_UNICHAR_ID == unichar_id) return NULL;
    ASSERT_HOST(contains_unichar_id(unichar_id));
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

  // Return the character properties, eg. alpha/upper/lower/digit/punct,
  // of the given unichar representation
  unsigned int get_properties(const char* const unichar_repr) const {
    return get_properties(unichar_to_id(unichar_repr));
  }

  char get_chartype(const char* const unichar_repr) const {
    return get_chartype(unichar_to_id(unichar_repr));
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

  // Returns normalized version of unichar with the given unichar_id.
  const char *get_normed_unichar(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.normed.string();
  }
  // Returns a vector of UNICHAR_IDs that represent the ids of the normalized
  // version of the given id. There may be more than one UNICHAR_ID in the
  // vector if unichar_id represents a ligature.
  const GenericVector<UNICHAR_ID>& normed_ids(UNICHAR_ID unichar_id) const {
    return unichars[unichar_id].properties.normed_ids;
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
  int hiragana_sid() const { return hiragana_sid_; }
  int katakana_sid() const { return katakana_sid_; }
  int default_sid() const { return default_sid_; }

  // Returns true if the unicharset has the concept of upper/lower case.
  bool script_has_upper_lower() const {
    return script_has_upper_lower_;
  }
  // Returns true if the unicharset has the concept of x-height.
  // script_has_xheight can be true even if script_has_upper_lower is not,
  // when the script has a sufficiently predominant top line with ascenders,
  // such as Devanagari and Thai.
  bool script_has_xheight() const {
    return script_has_xheight_;
  }

 private:

  struct UNICHAR_PROPERTIES {
    UNICHAR_PROPERTIES();
    // Initializes all properties to sensible default values.
    void Init();
    // Sets all ranges wide open. Initialization default in case there are
    // no useful values available.
    void SetRangesOpen();
    // Sets all ranges to empty. Used before expanding with font-based data.
    void SetRangesEmpty();
    // Returns true if any of the top/bottom/width/bearing/advance ranges is
    // emtpy.
    bool AnyRangeEmpty() const;
    // Expands the ranges with the ranges from the src properties.
    void ExpandRangesFrom(const UNICHAR_PROPERTIES& src);
    // Copies the properties from src into this.
    void CopyFrom(const UNICHAR_PROPERTIES& src);

    bool  isalpha;
    bool  islower;
    bool  isupper;
    bool  isdigit;
    bool  ispunctuation;
    bool  isngram;
    bool  enabled;
    // Possible limits of the top and bottom of the bounding box in
    // baseline-normalized coordinates, ie, where the baseline is
    // kBlnBaselineOffset and the meanline is kBlnBaselineOffset + kBlnXHeight
    // (See normalis.h for the definitions).
    uinT8 min_bottom;
    uinT8 max_bottom;
    uinT8 min_top;
    uinT8 max_top;
    // Limits on the widths of bounding box, also in baseline-normalized coords.
    inT16 min_width;
    inT16 max_width;
    // Limits on the x-bearing and advance, also in baseline-normalized coords.
    inT16 min_bearing;
    inT16 max_bearing;
    inT16 min_advance;
    inT16 max_advance;
    int   script_id;
    UNICHAR_ID other_case;  // id of the corresponding upper/lower case unichar
    Direction direction;  // direction of this unichar
    // Mirror property is useful for reverse DAWG lookup for words in
    // right-to-left languages (e.g. "(word)" would be in
    // '[open paren]' 'w' 'o' 'r' 'd' '[close paren]' in a UTF8 string.
    // However, what we want in our DAWG is
    // '[open paren]', 'd', 'r', 'o', 'w', '[close paren]' not
    // '[close paren]', 'd', 'r', 'o', 'w', '[open paren]'.
    UNICHAR_ID mirror;
    // A string of unichar_ids that represent the corresponding normed string.
    // For awkward characters like em-dash, this gives hyphen.
    // For ligatures, this gives the string of normal unichars.
    GenericVector<UNICHAR_ID> normed_ids;
    STRING normed;  // normalized version of this unichar
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

  // Internal recursive version of encode_string above.
  // str is the start of the whole string.
  // str_index is the current position in str.
  // str_length is the length of str.
  // encoding is a working encoding of str.
  // lengths is a working set of lengths of each element of encoding.
  // best_total_length is the longest length of str that has been successfully
  // encoded so far.
  // On return:
  // best_encoding contains the encoding that used the longest part of str.
  // best_lengths (may be null) contains the lengths of best_encoding.
  void encode_string(const char* str, int str_index, int str_length,
                     GenericVector<UNICHAR_ID>* encoding,
                     GenericVector<char>* lengths,
                     int* best_total_length,
                     GenericVector<UNICHAR_ID>* best_encoding,
                     GenericVector<char>* best_lengths) const;

  // Gets the properties for a grapheme string, combining properties for
  // multiple characters in a meaningful way where possible.
  // Returns false if no valid match was found in the unicharset.
  // NOTE that script_id, mirror, and other_case refer to this unicharset on
  // return and will need redirecting if the target unicharset is different.
  bool GetStrProperties(const char* utf8_str,
                        UNICHAR_PROPERTIES* props) const;

  // Load ourselves from a "file" where our only interface to the file is
  // an implementation of fgets().  This is the parsing primitive accessed by
  // the public routines load_from_file() and load_from_inmemory_file().
  bool load_via_fgets(TessResultCallback2<char *, char *, int> *fgets_cb,
                      bool skip_fragments);

  UNICHAR_SLOT* unichars;
  UNICHARMAP ids;
  int size_used;
  int size_reserved;
  char** script_table;
  int script_table_size_used;
  int script_table_size_reserved;
  const char* null_script;
  // True if the unichars have their tops/bottoms set.
  bool top_bottom_set_;
  // True if the unicharset has significant upper/lower case chars.
  bool script_has_upper_lower_;
  // True if the unicharset has a significant mean-line with significant
  // ascenders above that.
  bool script_has_xheight_;

  // A few convenient script name-to-id mapping without using hash.
  // These are initialized when unicharset file is loaded.  Anything
  // missing from this list can be looked up using get_script_id_from_name.
  int null_sid_;
  int common_sid_;
  int latin_sid_;
  int cyrillic_sid_;
  int greek_sid_;
  int han_sid_;
  int hiragana_sid_;
  int katakana_sid_;
  // The most frequently occurring script in the charset.
  int default_sid_;
};

#endif  // TESSERACT_CCUTIL_UNICHARSET_H__
