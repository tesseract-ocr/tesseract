///////////////////////////////////////////////////////////////////////
// File:        unicharset.cpp
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

#include "unicharset.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "params.h"
#include "serialis.h"
#include "tesscallback.h"
#include "tprintf.h"
#include "unichar.h"

// Special character used in representing character fragments.
static const char kSeparator = '|';
// Special character used in representing 'natural' character fragments.
static const char kNaturalFlag = 'n';

static const int ISALPHA_MASK = 0x1;
static const int ISLOWER_MASK = 0x2;
static const int ISUPPER_MASK = 0x4;
static const int ISDIGIT_MASK = 0x8;
static const int ISPUNCTUATION_MASK = 0x10;

// Y coordinate threshold for determining cap-height vs x-height.
// TODO(rays) Bring the global definition down to the ccutil library level,
// so this constant is relative to some other constants.
static const int kMeanlineThreshold = 220;
// Let C be the number of alpha chars for which all tops exceed
// kMeanlineThreshold, and X the number of alpha chars for which all
// tops are below kMeanlineThreshold, then if X > C *
// kMinXHeightFraction and C > X * kMinCapHeightFraction or more than
// half the alpha characters have upper or lower case, then the
// unicharset "has x-height".
const double kMinXHeightFraction = 0.25;
const double kMinCapHeightFraction = 0.05;

/*static */
const char* UNICHARSET::kCustomLigatures[][2] = {
  {"ct", "\uE003"},  // c + t -> U+E003
  {"ſh", "\uE006"},  // long-s + h -> U+E006
  {"ſi", "\uE007"},  // long-s + i -> U+E007
  {"ſl", "\uE008"},  // long-s + l -> U+E008
  {"ſſ", "\uE009"},  // long-s + long-s -> U+E009
  {NULL, NULL}
};

// List of strings for the SpecialUnicharCodes. Keep in sync with the enum.
const char* UNICHARSET::kSpecialUnicharCodes[SPECIAL_UNICHAR_CODES_COUNT] = {
    " ",
    "Joined",
    "|Broken|0|1"
};

UNICHARSET::UNICHAR_PROPERTIES::UNICHAR_PROPERTIES() {
  Init();
}

// Initialize all properties to sensible default values.
void UNICHARSET::UNICHAR_PROPERTIES::Init() {
  isalpha = false;
  islower = false;
  isupper = false;
  isdigit = false;
  ispunctuation = false;
  isngram = false;
  enabled = false;
  SetRangesOpen();
  script_id = 0;
  other_case = 0;
  mirror = 0;
  normed = "";
  direction = UNICHARSET::U_LEFT_TO_RIGHT;
  fragment = NULL;
}

// Sets all ranges wide open. Initialization default in case there are
// no useful values available.
void UNICHARSET::UNICHAR_PROPERTIES::SetRangesOpen() {
  min_bottom = 0;
  max_bottom = MAX_UINT8;
  min_top = 0;
  max_top = MAX_UINT8;
  width = 0.0f;
  width_sd = 0.0f;
  bearing = 0.0f;
  bearing_sd = 0.0f;
  advance = 0.0f;
  advance_sd = 0.0f;
}

// Sets all ranges to empty. Used before expanding with font-based data.
void UNICHARSET::UNICHAR_PROPERTIES::SetRangesEmpty() {
  min_bottom = MAX_UINT8;
  max_bottom = 0;
  min_top = MAX_UINT8;
  max_top = 0;
  width = 0.0f;
  width_sd = 0.0f;
  bearing = 0.0f;
  bearing_sd = 0.0f;
  advance = 0.0f;
  advance_sd = 0.0f;
}

// Returns true if any of the top/bottom/width/bearing/advance ranges/stats
// is emtpy.
bool UNICHARSET::UNICHAR_PROPERTIES::AnyRangeEmpty() const {
  return width == 0.0f || advance == 0.0f;
}

// Expands the ranges with the ranges from the src properties.
void UNICHARSET::UNICHAR_PROPERTIES::ExpandRangesFrom(
    const UNICHAR_PROPERTIES& src) {
  UpdateRange(src.min_bottom, &min_bottom, &max_bottom);
  UpdateRange(src.max_bottom, &min_bottom, &max_bottom);
  UpdateRange(src.min_top, &min_top, &max_top);
  UpdateRange(src.max_top, &min_top, &max_top);
  if (src.width_sd > width_sd) {
    width = src.width;
    width_sd = src.width_sd;
  }
  if (src.bearing_sd > bearing_sd) {
    bearing = src.bearing;
    bearing_sd = src.bearing_sd;
  }
  if (src.advance_sd > advance_sd) {
    advance = src.advance;
    advance_sd = src.advance_sd;
  }
}

// Copies the properties from src into this.
void UNICHARSET::UNICHAR_PROPERTIES::CopyFrom(const UNICHAR_PROPERTIES& src) {
  // Apart from the fragment, everything else can be done with a default copy.
  CHAR_FRAGMENT* saved_fragment = fragment;
  *this = src;  // Bitwise copy.
  fragment = saved_fragment;
}

UNICHARSET::UNICHARSET() :
    unichars(NULL),
    ids(),
    size_used(0),
    size_reserved(0),
    script_table(NULL),
    script_table_size_used(0),
    null_script("NULL") {
  clear();
  for (int i = 0; i < SPECIAL_UNICHAR_CODES_COUNT; ++i) {
    unichar_insert(kSpecialUnicharCodes[i]);
    if (i == UNICHAR_JOINED)
      set_isngram(i, true);
  }
}

UNICHARSET::~UNICHARSET() {
  clear();
}

void UNICHARSET::reserve(int unichars_number) {
  if (unichars_number > size_reserved) {
    UNICHAR_SLOT* unichars_new = new UNICHAR_SLOT[unichars_number];
    for (int i = 0; i < size_used; ++i)
      unichars_new[i] = unichars[i];
    for (int j = size_used; j < unichars_number; ++j) {
      unichars_new[j].properties.script_id = add_script(null_script);
    }
    delete[] unichars;
    unichars = unichars_new;
    size_reserved = unichars_number;
  }
}

UNICHAR_ID
UNICHARSET::unichar_to_id(const char* const unichar_repr) const {
  return ids.contains(unichar_repr) ?
    ids.unichar_to_id(unichar_repr) : INVALID_UNICHAR_ID;
}

UNICHAR_ID UNICHARSET::unichar_to_id(const char* const unichar_repr,
                                     int length) const {
  assert(length > 0 && length <= UNICHAR_LEN);
  return ids.contains(unichar_repr, length) ?
    ids.unichar_to_id(unichar_repr, length) : INVALID_UNICHAR_ID;
}

// Return the minimum number of bytes that matches a legal UNICHAR_ID,
// while leaving the rest of the string encodable. Returns 0 if the
// beginning of the string is not encodable.
// WARNING: this function now encodes the whole string for precision.
// Use encode_string in preference to repeatedly calling step.
int UNICHARSET::step(const char* str) const {
  GenericVector<UNICHAR_ID> encoding;
  GenericVector<char> lengths;
  encode_string(str, true, &encoding, &lengths, NULL);
  if (encoding.empty() || encoding[0] == INVALID_UNICHAR_ID) return 0;
  return lengths[0];
}

// Return whether the given UTF-8 string is encodable with this UNICHARSET.
// If not encodable, write the first byte offset which cannot be converted
// into the second (return) argument.
bool UNICHARSET::encodable_string(const char *str,
                                  int *first_bad_position) const {
  GenericVector<UNICHAR_ID> encoding;
  return encode_string(str, true, &encoding, NULL, first_bad_position);
}

// Encodes the given UTF-8 string with this UNICHARSET.
// Returns true if the encoding succeeds completely, false if there is at
// least one INVALID_UNICHAR_ID in the returned encoding, but in this case
// the rest of the string is still encoded.
// If lengths is not NULL, then it is filled with the corresponding
// byte length of each encoded UNICHAR_ID.
bool UNICHARSET::encode_string(const char* str, bool give_up_on_failure,
                               GenericVector<UNICHAR_ID>* encoding,
                               GenericVector<char>* lengths,
                               int* encoded_length) const {
  GenericVector<UNICHAR_ID> working_encoding;
  GenericVector<char> working_lengths;
  GenericVector<char> best_lengths;
  encoding->truncate(0);  // Just in case str is empty.
  int str_length = strlen(str);
  int str_pos = 0;
  bool perfect = true;
  while (str_pos < str_length) {
    encode_string(str, str_pos, str_length, &working_encoding, &working_lengths,
                  &str_pos, encoding, &best_lengths);
    if (str_pos < str_length) {
      // This is a non-match. Skip one utf-8 character.
      perfect = false;
      if (give_up_on_failure) break;
      int step = UNICHAR::utf8_step(str + str_pos);
      if (step == 0) step = 1;
      encoding->push_back(INVALID_UNICHAR_ID);
      best_lengths.push_back(step);
      str_pos += step;
      working_encoding = *encoding;
      working_lengths = best_lengths;
    }
  }
  if (lengths != NULL) *lengths = best_lengths;
  if (encoded_length != NULL) *encoded_length = str_pos;
  return perfect;
}

const char* UNICHARSET::id_to_unichar(UNICHAR_ID id) const {
  if (id == INVALID_UNICHAR_ID) {
    return INVALID_UNICHAR;
  }
  ASSERT_HOST(id < this->size());
  return unichars[id].representation;
}

const char* UNICHARSET::id_to_unichar_ext(UNICHAR_ID id) const {
  if (id == INVALID_UNICHAR_ID) {
    return INVALID_UNICHAR;
  }
  ASSERT_HOST(id < this->size());
  // Resolve from the kCustomLigatures table if this is a private encoding.
  if (get_isprivate(id)) {
    const char* ch = id_to_unichar(id);
    for (int i = 0; kCustomLigatures[i][0] != NULL; ++i) {
      if (!strcmp(ch, kCustomLigatures[i][1])) {
        return kCustomLigatures[i][0];
      }
    }
  }
  // Otherwise return the stored representation.
  return unichars[id].representation;
}

// Return a STRING that reformats the utf8 str into the str followed
// by its hex unicodes.
STRING UNICHARSET::debug_utf8_str(const char* str) {
  STRING result = str;
  result += " [";
  int step = 1;
  // Chop into unicodes and code each as hex.
  for (int i = 0; str[i] != '\0'; i += step) {
    char hex[sizeof(int) * 2 + 1];
    step = UNICHAR::utf8_step(str + i);
    if (step == 0) {
      step = 1;
      sprintf(hex, "%x", str[i]);
    } else {
      UNICHAR ch(str + i, step);
      sprintf(hex, "%x", ch.first_uni());
    }
    result += hex;
    result += " ";
  }
  result += "]";
  return result;
}

// Return a STRING containing debug information on the unichar, including
// the id_to_unichar, its hex unicodes and the properties.
STRING UNICHARSET::debug_str(UNICHAR_ID id) const {
  if (id == INVALID_UNICHAR_ID) return STRING(id_to_unichar(id));
  const CHAR_FRAGMENT *fragment = this->get_fragment(id);
  if (fragment) {
    return fragment->to_string();
  }
  const char* str = id_to_unichar(id);
  STRING result = debug_utf8_str(str);
  // Append a for lower alpha, A for upper alpha, and x if alpha but neither.
  if (get_isalpha(id)) {
    if (get_islower(id))
      result += "a";
    else if (get_isupper(id))
      result += "A";
    else
      result += "x";
  }
  // Append 0 if a digit.
  if (get_isdigit(id)) {
    result += "0";
  }
  // Append p is a punctuation symbol.
  if (get_ispunctuation(id)) {
    result += "p";
  }
  return result;
}

// Sets the normed_ids vector from the normed string. normed_ids is not
// stored in the file, and needs to be set when the UNICHARSET is loaded.
void UNICHARSET::set_normed_ids(UNICHAR_ID unichar_id) {
  unichars[unichar_id].properties.normed_ids.truncate(0);
  if (unichar_id == UNICHAR_SPACE && id_to_unichar(unichar_id)[0] == ' ') {
    unichars[unichar_id].properties.normed_ids.push_back(UNICHAR_SPACE);
  } else if (!encode_string(unichars[unichar_id].properties.normed.string(),
                            true, &unichars[unichar_id].properties.normed_ids,
                            NULL, NULL)) {
    unichars[unichar_id].properties.normed_ids.truncate(0);
    unichars[unichar_id].properties.normed_ids.push_back(unichar_id);
  }
}

// Returns whether the unichar id represents a unicode value in the private use
// area. We use this range only internally to represent uncommon ligatures
// (eg. 'ct') that do not have regular unicode values.
bool UNICHARSET::get_isprivate(UNICHAR_ID unichar_id) const {
  UNICHAR uc(id_to_unichar(unichar_id), -1);
  int uni = uc.first_uni();
  return (uni >= 0xE000 && uni <= 0xF8FF);
}


// Sets all ranges to empty, so they can be expanded to set the values.
void UNICHARSET::set_ranges_empty() {
  for (int id = 0; id < size_used; ++id) {
    unichars[id].properties.SetRangesEmpty();
  }
}

// Sets all the properties for this unicharset given a src unicharset with
// everything set. The unicharsets don't have to be the same, and graphemes
// are correctly accounted for.
void UNICHARSET::PartialSetPropertiesFromOther(int start_index,
                                               const UNICHARSET& src) {
  for (int ch = start_index; ch < size_used; ++ch) {
    const char* utf8 = id_to_unichar(ch);
    UNICHAR_PROPERTIES properties;
    if (src.GetStrProperties(utf8, &properties)) {
      // Setup the script_id, other_case, and mirror properly.
      const char* script = src.get_script_from_script_id(properties.script_id);
      properties.script_id = add_script(script);
      const char* other_case = src.id_to_unichar(properties.other_case);
      if (contains_unichar(other_case)) {
        properties.other_case = unichar_to_id(other_case);
      } else {
        properties.other_case = ch;
      }
      const char* mirror_str = src.id_to_unichar(properties.mirror);
      if (contains_unichar(mirror_str)) {
        properties.mirror = unichar_to_id(mirror_str);
      } else {
        properties.mirror = ch;
      }
      unichars[ch].properties.CopyFrom(properties);
      set_normed_ids(ch);
    }
  }
}

// Expands the tops and bottoms and widths for this unicharset given a
// src unicharset with ranges in it. The unicharsets don't have to be the
// same, and graphemes are correctly accounted for.
void UNICHARSET::ExpandRangesFromOther(const UNICHARSET& src) {
  for (int ch = 0; ch < size_used; ++ch) {
    const char* utf8 = id_to_unichar(ch);
    UNICHAR_PROPERTIES properties;
    if (src.GetStrProperties(utf8, &properties)) {
      // Expand just the ranges from properties.
      unichars[ch].properties.ExpandRangesFrom(properties);
    }
  }
}

// Makes this a copy of src. Clears this completely first, so the automatic
// ids will not be present in this if not in src. Does NOT reorder the set!
void UNICHARSET::CopyFrom(const UNICHARSET& src) {
  clear();
  for (int ch = 0; ch < src.size_used; ++ch) {
    const UNICHAR_PROPERTIES& src_props = src.unichars[ch].properties;
    const char* utf8 = src.id_to_unichar(ch);
    unichar_insert(utf8);
    unichars[ch].properties.ExpandRangesFrom(src_props);
  }
  // Set properties, including mirror and other_case, WITHOUT reordering
  // the unicharset.
  PartialSetPropertiesFromOther(0, src);
}

// For each id in src, if it does not occur in this, add it, as in
// SetPropertiesFromOther, otherwise expand the ranges, as in
// ExpandRangesFromOther.
void UNICHARSET::AppendOtherUnicharset(const UNICHARSET& src) {
  int initial_used = size_used;
  for (int ch = 0; ch < src.size_used; ++ch) {
    const UNICHAR_PROPERTIES& src_props = src.unichars[ch].properties;
    const char* utf8 = src.id_to_unichar(ch);
    if (ch >= SPECIAL_UNICHAR_CODES_COUNT && src_props.AnyRangeEmpty()) {
      // Only use fully valid entries.
      tprintf("Bad properties for index %d, char %s: "
              "%d,%d %d,%d %g,%g %g,%g %g,%g\n",
              ch, utf8, src_props.min_bottom, src_props.max_bottom,
              src_props.min_top, src_props.max_top,
              src_props.width, src_props.width_sd,
              src_props.bearing, src_props.bearing_sd,
              src_props.advance, src_props.advance_sd);
      continue;
    }
    int id = size_used;
    if (contains_unichar(utf8)) {
      id = unichar_to_id(utf8);
      // Just expand current ranges.
      unichars[id].properties.ExpandRangesFrom(src_props);
    } else {
      unichar_insert(utf8);
      unichars[id].properties.SetRangesEmpty();
    }
  }
  // Set properties, including mirror and other_case, WITHOUT reordering
  // the unicharset.
  PartialSetPropertiesFromOther(initial_used, src);
}

// Returns true if the acceptable ranges of the tops of the characters do
// not overlap, making their x-height calculations distinct.
bool UNICHARSET::SizesDistinct(UNICHAR_ID id1, UNICHAR_ID id2) const {
  int overlap = MIN(unichars[id1].properties.max_top,
                    unichars[id2].properties.max_top) -
                MAX(unichars[id1].properties.min_top,
                    unichars[id2].properties.min_top);
  return overlap <= 0;
}

// Internal recursive version of encode_string above.
// Seeks to encode the given string as a sequence of UNICHAR_IDs such that
// each UNICHAR_ID uses the least possible part of the utf8 str.
// It does this by depth-first tail recursion on increasing length matches
// to the UNICHARSET, saving the first encountered result that encodes the
// maximum total length of str. It stops on a failure to encode to make
// the overall process of encoding a partially failed string more efficient.
// See unicharset.h for definition of the args.
void UNICHARSET::encode_string(const char* str, int str_index, int str_length,
                               GenericVector<UNICHAR_ID>* encoding,
                               GenericVector<char>* lengths,
                               int* best_total_length,
                               GenericVector<UNICHAR_ID>* best_encoding,
                               GenericVector<char>* best_lengths) const {
  if (str_index > *best_total_length) {
    // This is the best result so far.
    *best_total_length = str_index;
    *best_encoding = *encoding;
    if (best_lengths != NULL)
      *best_lengths = *lengths;
  }
  if (str_index == str_length) return;
  int encoding_index = encoding->size();
  // Find the length of the first matching unicharset member.
  int length = ids.minmatch(str + str_index);
  if (length == 0 || str_index + length > str_length) return;
  do {
    if (ids.contains(str + str_index, length)) {
      // Successful encoding so far.
      UNICHAR_ID id = ids.unichar_to_id(str + str_index, length);
      encoding->push_back(id);
      lengths->push_back(length);
      encode_string(str, str_index + length, str_length, encoding, lengths,
                    best_total_length, best_encoding, best_lengths);
      if (*best_total_length == str_length)
        return;  // Tail recursion success!
      // Failed with that length, truncate back and try again.
      encoding->truncate(encoding_index);
      lengths->truncate(encoding_index);
    }
    int step = UNICHAR::utf8_step(str + str_index + length);
    if (step == 0) step = 1;
    length += step;
  } while (length <= UNICHAR_LEN && str_index + length <= str_length);
}

// Gets the properties for a grapheme string, combining properties for
// multiple characters in a meaningful way where possible.
// Returns false if no valid match was found in the unicharset.
// NOTE that script_id, mirror, and other_case refer to this unicharset on
// return and will need translation if the target unicharset is different.
bool UNICHARSET::GetStrProperties(const char* utf8_str,
                                  UNICHAR_PROPERTIES* props) const {
  props->Init();
  props->SetRangesEmpty();
  int total_unicodes = 0;
  GenericVector<UNICHAR_ID> encoding;
  if (!encode_string(utf8_str, true, &encoding, NULL, NULL))
    return false;  // Some part was invalid.
  for (int i = 0; i < encoding.size(); ++i) {
    int id = encoding[i];
    const UNICHAR_PROPERTIES& src_props = unichars[id].properties;
    // Logical OR all the bools.
    if (src_props.isalpha) props->isalpha = true;
    if (src_props.islower) props->islower = true;
    if (src_props.isupper) props->isupper = true;
    if (src_props.isdigit) props->isdigit = true;
    if (src_props.ispunctuation) props->ispunctuation = true;
    if (src_props.isngram) props->isngram = true;
    if (src_props.enabled) props->enabled = true;
    // Min/max the tops/bottoms.
    UpdateRange(src_props.min_bottom, &props->min_bottom, &props->max_bottom);
    UpdateRange(src_props.max_bottom, &props->min_bottom, &props->max_bottom);
    UpdateRange(src_props.min_top, &props->min_top, &props->max_top);
    UpdateRange(src_props.max_top, &props->min_top, &props->max_top);
    float bearing = props->advance + src_props.bearing;
    if (total_unicodes == 0 || bearing < props->bearing) {
      props->bearing = bearing;
      props->bearing_sd = props->advance_sd + src_props.bearing_sd;
    }
    props->advance += src_props.advance;
    props->advance_sd += src_props.advance_sd;
    // With a single width, just use the widths stored in the unicharset.
    props->width = src_props.width;
    props->width_sd = src_props.width_sd;
    // Use the first script id, other_case, mirror, direction.
    // Note that these will need translation, except direction.
    if (total_unicodes == 0) {
      props->script_id = src_props.script_id;
      props->other_case = src_props.other_case;
      props->mirror = src_props.mirror;
      props->direction = src_props.direction;
    }
    // The normed string for the compound character is the concatenation of
    // the normed versions of the individual characters.
    props->normed += src_props.normed;
    ++total_unicodes;
  }
  if (total_unicodes > 1) {
    // Estimate the total widths from the advance - bearing.
    props->width = props->advance - props->bearing;
    props->width_sd = props->advance_sd + props->bearing_sd;
  }
  return total_unicodes > 0;
}

// TODO(rays) clean-up the order of functions to match unicharset.h.

unsigned int UNICHARSET::get_properties(UNICHAR_ID id) const {
  unsigned int properties = 0;
  if (this->get_isalpha(id))
    properties |= ISALPHA_MASK;
  if (this->get_islower(id))
    properties |= ISLOWER_MASK;
  if (this->get_isupper(id))
    properties |= ISUPPER_MASK;
  if (this->get_isdigit(id))
    properties |= ISDIGIT_MASK;
  if (this->get_ispunctuation(id))
    properties |= ISPUNCTUATION_MASK;
  return properties;
}

char UNICHARSET::get_chartype(UNICHAR_ID id) const {
  if (this->get_isupper(id)) return 'A';
  if (this->get_islower(id)) return 'a';
  if (this->get_isalpha(id)) return 'x';
  if (this->get_isdigit(id)) return '0';
  if (this->get_ispunctuation(id)) return 'p';
  return 0;
}

void UNICHARSET::unichar_insert(const char* const unichar_repr) {
  if (!ids.contains(unichar_repr)) {
    if (strlen(unichar_repr) > UNICHAR_LEN) {
      fprintf(stderr, "Utf8 buffer too big, size=%d for %s\n",
              int(strlen(unichar_repr)), unichar_repr);
      return;
    }
    if (size_used == size_reserved) {
      if (size_used == 0)
        reserve(8);
      else
        reserve(2 * size_used);
    }

    strcpy(unichars[size_used].representation, unichar_repr);
    this->set_script(size_used, null_script);
    // If the given unichar_repr represents a fragmented character, set
    // fragment property to a pointer to CHAR_FRAGMENT class instance with
    // information parsed from the unichar representation. Use the script
    // of the base unichar for the fragmented character if possible.
    CHAR_FRAGMENT *frag = CHAR_FRAGMENT::parse_from_string(unichar_repr);
    this->unichars[size_used].properties.fragment = frag;
    if (frag != NULL && this->contains_unichar(frag->get_unichar())) {
      this->unichars[size_used].properties.script_id =
        this->get_script(frag->get_unichar());
    }
    this->unichars[size_used].properties.enabled = true;
    ids.insert(unichar_repr, size_used);
    ++size_used;
  }
}

bool UNICHARSET::contains_unichar(const char* const unichar_repr) const {
  return ids.contains(unichar_repr);
}

bool UNICHARSET::contains_unichar(const char* const unichar_repr,
                                  int length) const {
  if (length == 0) {
    return false;
  }
  return ids.contains(unichar_repr, length);
}

bool UNICHARSET::eq(UNICHAR_ID unichar_id,
                    const char* const unichar_repr) const {
  return strcmp(this->id_to_unichar(unichar_id), unichar_repr) == 0;
}

bool UNICHARSET::save_to_string(STRING *str) const {
  const int kFileBufSize = 1024;
  char buffer[kFileBufSize + 1];
  snprintf(buffer, kFileBufSize, "%d\n", this->size());
  *str = buffer;
  for (UNICHAR_ID id = 0; id < this->size(); ++id) {
    int min_bottom, max_bottom, min_top, max_top;
    get_top_bottom(id, &min_bottom, &max_bottom, &min_top, &max_top);
    float width, width_sd;
    get_width_stats(id, &width, &width_sd);
    float bearing, bearing_sd;
    get_bearing_stats(id, &bearing, &bearing_sd);
    float advance, advance_sd;
    get_advance_stats(id, &advance, &advance_sd);
    unsigned int properties = this->get_properties(id);
    if (strcmp(this->id_to_unichar(id), " ") == 0) {
      snprintf(buffer, kFileBufSize, "%s %x %s %d\n", "NULL", properties,
              this->get_script_from_script_id(this->get_script(id)),
              this->get_other_case(id));
    } else {
      snprintf(buffer, kFileBufSize,
              "%s %x %d,%d,%d,%d,%g,%g,%g,%g,%g,%g %s %d %d %d %s\t# %s\n",
              this->id_to_unichar(id), properties,
              min_bottom, max_bottom, min_top, max_top, width, width_sd,
              bearing, bearing_sd, advance, advance_sd,
              this->get_script_from_script_id(this->get_script(id)),
              this->get_other_case(id), this->get_direction(id),
              this->get_mirror(id), this->get_normed_unichar(id),
              this->debug_str(id).string());
    }
    *str += buffer;
  }
  return true;
}

// TODO(rays) Replace with TFile everywhere.
class InMemoryFilePointer {
 public:
  InMemoryFilePointer(const char *memory, int mem_size)
      : memory_(memory), fgets_ptr_(memory), mem_size_(mem_size) { }

  char *fgets(char *orig_dst, int size) {
    const char *src_end = memory_ + mem_size_;
    char *dst_end = orig_dst + size - 1;
    if (size < 1) {
      return fgets_ptr_ < src_end ? orig_dst : NULL;
    }

    char *dst = orig_dst;
    char ch = '^';
    while (fgets_ptr_ < src_end && dst < dst_end && ch != '\n') {
      ch = *dst++ = *fgets_ptr_++;
    }
    *dst = 0;
    return (dst == orig_dst) ? NULL : orig_dst;
  }

 private:
  const char *memory_;
  const char *fgets_ptr_;
  const int mem_size_;
};

bool UNICHARSET::load_from_inmemory_file(
    const char *memory, int mem_size, bool skip_fragments) {
  InMemoryFilePointer mem_fp(memory, mem_size);
  TessResultCallback2<char *, char *, int> *fgets_cb =
      NewPermanentTessCallback(&mem_fp, &InMemoryFilePointer::fgets);
  bool success = load_via_fgets(fgets_cb, skip_fragments);
  delete fgets_cb;
  return success;
}

class LocalFilePointer {
 public:
  LocalFilePointer(FILE *stream) : fp_(stream) {}
  char *fgets(char *dst, int size) {
    return ::fgets(dst, size, fp_);
  }
 private:
  FILE *fp_;
};

bool UNICHARSET::load_from_file(FILE *file, bool skip_fragments) {
  LocalFilePointer lfp(file);
  TessResultCallback2<char *, char *, int> *fgets_cb =
      NewPermanentTessCallback(&lfp, &LocalFilePointer::fgets);
  bool success = load_via_fgets(fgets_cb, skip_fragments);
  delete fgets_cb;
  return success;
}

bool UNICHARSET::load_from_file(tesseract::TFile *file, bool skip_fragments) {
  TessResultCallback2<char *, char *, int> *fgets_cb =
      NewPermanentTessCallback(file, &tesseract::TFile::FGets);
  bool success = load_via_fgets(fgets_cb, skip_fragments);
  delete fgets_cb;
  return success;
}

bool UNICHARSET::load_via_fgets(
    TessResultCallback2<char *, char *, int> *fgets_cb,
    bool skip_fragments) {
  int unicharset_size;
  char buffer[256];

  this->clear();
  if (fgets_cb->Run(buffer, sizeof(buffer)) == NULL ||
      sscanf(buffer, "%d", &unicharset_size) != 1) {
    return false;
  }
  this->reserve(unicharset_size);
  for (UNICHAR_ID id = 0; id < unicharset_size; ++id) {
    char unichar[256];
    unsigned int properties;
    char script[64];

    strcpy(script, null_script);
    int min_bottom = 0;
    int max_bottom = MAX_UINT8;
    int min_top = 0;
    int max_top = MAX_UINT8;
    float width = 0.0f;
    float width_sd = 0.0f;
    float bearing = 0.0f;
    float bearing_sd = 0.0f;
    float advance = 0.0f;
    float advance_sd = 0.0f;
    // TODO(eger): check that this default it ok
    // after enabling BiDi iterator for Arabic+Cube.
    int direction = UNICHARSET::U_LEFT_TO_RIGHT;
    UNICHAR_ID other_case = id;
    UNICHAR_ID mirror = id;
    char normed[64];
    int v = -1;
    if (fgets_cb->Run(buffer, sizeof (buffer)) == NULL ||
        ((v = sscanf(buffer,
                     "%s %x %d,%d,%d,%d,%g,%g,%g,%g,%g,%g %63s %d %d %d %63s",
                     unichar, &properties,
                     &min_bottom, &max_bottom, &min_top, &max_top,
                     &width, &width_sd, &bearing, &bearing_sd,
                     &advance, &advance_sd, script, &other_case,
                     &direction, &mirror, normed)) != 17 &&
         (v = sscanf(buffer,
                     "%s %x %d,%d,%d,%d,%g,%g,%g,%g,%g,%g %63s %d %d %d",
                     unichar, &properties,
                     &min_bottom, &max_bottom, &min_top, &max_top,
                     &width, &width_sd, &bearing, &bearing_sd,
                     &advance, &advance_sd, script, &other_case,
                     &direction, &mirror)) != 16 &&
          (v = sscanf(buffer, "%s %x %d,%d,%d,%d %63s %d %d %d",
                      unichar, &properties,
                      &min_bottom, &max_bottom, &min_top, &max_top,
                      script, &other_case, &direction, &mirror)) != 10 &&
          (v = sscanf(buffer, "%s %x %d,%d,%d,%d %63s %d", unichar, &properties,
                      &min_bottom, &max_bottom, &min_top, &max_top,
                      script, &other_case)) != 8 &&
          (v = sscanf(buffer, "%s %x %63s %d", unichar, &properties,
                      script, &other_case)) != 4 &&
          (v = sscanf(buffer, "%s %x %63s",
                      unichar, &properties, script)) != 3 &&
          (v = sscanf(buffer, "%s %x", unichar, &properties)) != 2)) {
      return false;
    }

    // Skip fragments if needed.
    CHAR_FRAGMENT *frag = NULL;
    if (skip_fragments && (frag = CHAR_FRAGMENT::parse_from_string(unichar))) {
      int num_pieces = frag->get_total();
      delete frag;
      // Skip multi-element fragments, but keep singles like UNICHAR_BROKEN in.
      if (num_pieces > 1)
        continue;
    }
    // Insert unichar into unicharset and set its properties.
    if (strcmp(unichar, "NULL") == 0)
      this->unichar_insert(" ");
    else
      this->unichar_insert(unichar);

    this->set_isalpha(id, properties & ISALPHA_MASK);
    this->set_islower(id, properties & ISLOWER_MASK);
    this->set_isupper(id, properties & ISUPPER_MASK);
    this->set_isdigit(id, properties & ISDIGIT_MASK);
    this->set_ispunctuation(id, properties & ISPUNCTUATION_MASK);
    this->set_isngram(id, false);
    this->set_script(id, script);
    this->unichars[id].properties.enabled = true;
    this->set_top_bottom(id, min_bottom, max_bottom, min_top, max_top);
    this->set_width_stats(id, width, width_sd);
    this->set_bearing_stats(id, bearing, bearing_sd);
    this->set_advance_stats(id, advance, advance_sd);
    this->set_direction(id, static_cast<UNICHARSET::Direction>(direction));
    ASSERT_HOST(other_case < unicharset_size);
    this->set_other_case(id, (v>3) ? other_case : id);
    ASSERT_HOST(mirror < unicharset_size);
    this->set_mirror(id, (v>8) ? mirror : id);
    this->set_normed(id, (v>16) ? normed : unichar);
  }
  post_load_setup();
  return true;
}

// Sets up internal data after loading the file, based on the char
// properties. Called from load_from_file, but also needs to be run
// during set_unicharset_properties.
void UNICHARSET::post_load_setup() {
  // Number of alpha chars with the case property minus those without,
  // in order to determine that half the alpha chars have case.
  int net_case_alphas = 0;
  int x_height_alphas = 0;
  int cap_height_alphas = 0;
  top_bottom_set_ = false;
  for (UNICHAR_ID id = 0; id < size_used; ++id) {
    int min_bottom = 0;
    int max_bottom = MAX_UINT8;
    int min_top = 0;
    int max_top = MAX_UINT8;
    get_top_bottom(id, &min_bottom, &max_bottom, &min_top, &max_top);
    if (min_top > 0)
      top_bottom_set_ = true;
    if (get_isalpha(id)) {
      if (get_islower(id) || get_isupper(id))
        ++net_case_alphas;
      else
        --net_case_alphas;
      if (min_top < kMeanlineThreshold && max_top < kMeanlineThreshold)
        ++x_height_alphas;
      else if (min_top > kMeanlineThreshold && max_top > kMeanlineThreshold)
        ++cap_height_alphas;
    }
    set_normed_ids(id);
  }

  script_has_upper_lower_ = net_case_alphas > 0;
  script_has_xheight_ = script_has_upper_lower_ ||
      (x_height_alphas > cap_height_alphas * kMinXHeightFraction &&
       cap_height_alphas > x_height_alphas * kMinCapHeightFraction);

  null_sid_ = get_script_id_from_name(null_script);
  ASSERT_HOST(null_sid_ == 0);
  common_sid_ = get_script_id_from_name("Common");
  latin_sid_ = get_script_id_from_name("Latin");
  cyrillic_sid_ = get_script_id_from_name("Cyrillic");
  greek_sid_ = get_script_id_from_name("Greek");
  han_sid_ = get_script_id_from_name("Han");
  hiragana_sid_ = get_script_id_from_name("Hiragana");
  katakana_sid_ = get_script_id_from_name("Katakana");

  // Compute default script. Use the highest-counting alpha script, that is
  // not the common script, as that still contains some "alphas".
  int* script_counts = new int[script_table_size_used];
  memset(script_counts, 0, sizeof(*script_counts) * script_table_size_used);
  for (int id = 0; id < size_used; ++id) {
    if (get_isalpha(id)) {
      ++script_counts[get_script(id)];
    }
  }
  default_sid_ = 0;
  for (int s = 1; s < script_table_size_used; ++s) {
    if (script_counts[s] > script_counts[default_sid_] && s != common_sid_)
      default_sid_ = s;
  }
  delete [] script_counts;
}

// Returns true if right_to_left scripts are significant in the unicharset,
// but without being so sensitive that "universal" unicharsets containing
// characters from many scripts, like orientation and script detection,
// look like they are right_to_left.
bool UNICHARSET::major_right_to_left() const {
  int ltr_count = 0;
  int rtl_count = 0;
  for (int id = 0; id < size_used; ++id) {
    int dir = get_direction(id);
    if (dir == UNICHARSET::U_LEFT_TO_RIGHT) ltr_count++;
    if (dir == UNICHARSET::U_RIGHT_TO_LEFT ||
        dir == UNICHARSET::U_RIGHT_TO_LEFT_ARABIC ||
        dir == UNICHARSET::U_ARABIC_NUMBER) rtl_count++;
  }
  return rtl_count > ltr_count;
}

// Set a whitelist and/or blacklist of characters to recognize.
// An empty or NULL whitelist enables everything (minus any blacklist).
// An empty or NULL blacklist disables nothing.
// An empty or NULL blacklist has no effect.
void UNICHARSET::set_black_and_whitelist(const char* blacklist,
                                         const char* whitelist,
                                         const char* unblacklist) {
  bool def_enabled = whitelist == NULL || whitelist[0] == '\0';
  // Set everything to default
  for (int ch = 0; ch < size_used; ++ch)
    unichars[ch].properties.enabled = def_enabled;
  if (!def_enabled) {
    // Enable the whitelist.
    GenericVector<UNICHAR_ID> encoding;
    encode_string(whitelist, false, &encoding, NULL, NULL);
    for (int i = 0; i < encoding.size(); ++i) {
      if (encoding[i] != INVALID_UNICHAR_ID)
        unichars[encoding[i]].properties.enabled = true;
    }
  }
  if (blacklist != NULL && blacklist[0] != '\0') {
    // Disable the blacklist.
    GenericVector<UNICHAR_ID> encoding;
    encode_string(blacklist, false, &encoding, NULL, NULL);
    for (int i = 0; i < encoding.size(); ++i) {
      if (encoding[i] != INVALID_UNICHAR_ID)
        unichars[encoding[i]].properties.enabled = false;
    }
  }
  if (unblacklist != NULL && unblacklist[0] != '\0') {
    // Re-enable the unblacklist.
    GenericVector<UNICHAR_ID> encoding;
    encode_string(unblacklist, false, &encoding, NULL, NULL);
    for (int i = 0; i < encoding.size(); ++i) {
      if (encoding[i] != INVALID_UNICHAR_ID)
        unichars[encoding[i]].properties.enabled = true;
    }
  }
}

// Returns true if there are any repeated unicodes in the normalized
// text of any unichar-id in the unicharset.
bool UNICHARSET::AnyRepeatedUnicodes() const {
  int start_id = 0;
  if (has_special_codes()) start_id = SPECIAL_UNICHAR_CODES_COUNT;
  for (int id = start_id; id < size_used; ++id) {
    // Convert to unicodes.
    GenericVector<int> unicodes;
    if (UNICHAR::UTF8ToUnicode(get_normed_unichar(id), &unicodes) &&
        unicodes.size() > 1) {
      for (int u = 1; u < unicodes.size(); ++u) {
        if (unicodes[u - 1] == unicodes[u]) return true;
      }
    }
  }
  return false;
}

int UNICHARSET::add_script(const char* script) {
  for (int i = 0; i < script_table_size_used; ++i) {
    if (strcmp(script, script_table[i]) == 0)
      return i;
  }
  if (script_table_size_reserved == 0) {
    script_table_size_reserved = 8;
    script_table = new char*[script_table_size_reserved];
  } else if (script_table_size_used >= script_table_size_reserved) {
    assert(script_table_size_used == script_table_size_reserved);
    script_table_size_reserved += script_table_size_reserved;
    char** new_script_table = new char*[script_table_size_reserved];
    memcpy(new_script_table, script_table, script_table_size_used * sizeof(char*));
    delete[] script_table;
    script_table = new_script_table;
  }
  script_table[script_table_size_used] = new char[strlen(script) + 1];
  strcpy(script_table[script_table_size_used], script);
  return script_table_size_used++;
}

// Returns the string that represents a fragment
// with the given unichar, pos and total.
STRING CHAR_FRAGMENT::to_string(const char *unichar, int pos, int total,
                                bool natural) {
  if (total == 1) return STRING(unichar);
  STRING result = "";
  result += kSeparator;
  result += unichar;
  char buffer[kMaxLen];
  snprintf(buffer, kMaxLen, "%c%d%c%d", kSeparator, pos,
           natural ? kNaturalFlag : kSeparator, total);
  result += buffer;
  return result;
}

CHAR_FRAGMENT *CHAR_FRAGMENT::parse_from_string(const char *string) {
  const char *ptr = string;
  int len = strlen(string);
  if (len < kMinLen || *ptr != kSeparator) {
    return NULL;  // this string can not represent a fragment
  }
  ptr++;  // move to the next character
  int step = 0;
  while ((ptr + step) < (string + len) && *(ptr + step) != kSeparator) {
    step += UNICHAR::utf8_step(ptr + step);
  }
  if (step == 0 || step > UNICHAR_LEN) {
    return NULL;  // no character for unichar or the character is too long
  }
  char unichar[UNICHAR_LEN + 1];
  strncpy(unichar, ptr, step);
  unichar[step] = '\0';  // null terminate unichar
  ptr += step;  // move to the next fragment separator
  int pos = 0;
  int total = 0;
  bool natural = false;
  char *end_ptr = NULL;
  for (int i = 0; i < 2; i++) {
    if (ptr > string + len || *ptr != kSeparator) {
      if (i == 1 && *ptr == kNaturalFlag)
        natural = true;
      else
        return NULL;  // Failed to parse fragment representation.
    }
    ptr++;  // move to the next character
    i == 0 ? pos = static_cast<int>(strtol(ptr, &end_ptr, 10))
      : total = static_cast<int>(strtol(ptr, &end_ptr, 10));
    ptr = end_ptr;
  }
  if (ptr != string + len) {
    return NULL;  // malformed fragment representation
  }
  CHAR_FRAGMENT *fragment = new CHAR_FRAGMENT();
  fragment->set_all(unichar, pos, total, natural);
  return fragment;
}

int UNICHARSET::get_script_id_from_name(const char* script_name) const {
  for (int i = 0; i < script_table_size_used; ++i) {
    if (strcmp(script_name, script_table[i]) == 0)
      return i;
  }
  return 0;  // 0 is always the null_script
}
