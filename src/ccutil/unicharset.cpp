///////////////////////////////////////////////////////////////////////
// File:        unicharset.cpp
// Description: Unicode character/ligature set class.
// Author:      Thomas Kielbus
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

#include "params.h"

#include <tesseract/unichar.h>
#include "serialis.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iomanip> // for std::setw
#include <locale>  // for std::locale::classic
#include <sstream> // for std::istringstream, std::ostringstream

namespace tesseract {

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
const char *UNICHARSET::kCustomLigatures[][2] = {
    {"ct", "\uE003"}, // c + t -> U+E003
    {"ſh", "\uE006"}, // long-s + h -> U+E006
    {"ſi", "\uE007"}, // long-s + i -> U+E007
    {"ſl", "\uE008"}, // long-s + l -> U+E008
    {"ſſ", "\uE009"}, // long-s + long-s -> U+E009
    {nullptr, nullptr}};

// List of mappings to make when ingesting strings from the outside.
// The substitutions clean up text that should exist for rendering of
// synthetic data, but not in the recognition set.
const char *UNICHARSET::kCleanupMaps[][2] = {
    {"\u0640", ""},   // TATWEEL is deleted.
    {"\ufb01", "fi"}, // fi ligature->fi pair.
    {"\ufb02", "fl"}, // fl ligature->fl pair.
    {nullptr, nullptr}};

// List of strings for the SpecialUnicharCodes. Keep in sync with the enum.
const char *UNICHARSET::kSpecialUnicharCodes[SPECIAL_UNICHAR_CODES_COUNT] = {
    " ", "Joined", "|Broken|0|1"};

const char *UNICHARSET::null_script = "NULL";

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
  fragment = nullptr;
}

// Sets all ranges wide open. Initialization default in case there are
// no useful values available.
void UNICHARSET::UNICHAR_PROPERTIES::SetRangesOpen() {
  min_bottom = 0;
  max_bottom = UINT8_MAX;
  min_top = 0;
  max_top = UINT8_MAX;
  width = 0.0f;
  width_sd = 0.0f;
  bearing = 0.0f;
  bearing_sd = 0.0f;
  advance = 0.0f;
  advance_sd = 0.0f;
}

// Sets all ranges to empty. Used before expanding with font-based data.
void UNICHARSET::UNICHAR_PROPERTIES::SetRangesEmpty() {
  min_bottom = UINT8_MAX;
  max_bottom = 0;
  min_top = UINT8_MAX;
  max_top = 0;
  width = 0.0f;
  width_sd = 0.0f;
  bearing = 0.0f;
  bearing_sd = 0.0f;
  advance = 0.0f;
  advance_sd = 0.0f;
}

// Returns true if any of the top/bottom/width/bearing/advance ranges/stats
// is empty.
bool UNICHARSET::UNICHAR_PROPERTIES::AnyRangeEmpty() const {
  return width == 0.0f || advance == 0.0f;
}

// Expands the ranges with the ranges from the src properties.
void UNICHARSET::UNICHAR_PROPERTIES::ExpandRangesFrom(
    const UNICHAR_PROPERTIES &src) {
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
void UNICHARSET::UNICHAR_PROPERTIES::CopyFrom(const UNICHAR_PROPERTIES &src) {
  // Apart from the fragment, everything else can be done with a default copy.
  CHAR_FRAGMENT *saved_fragment = fragment;
  *this = src; // Bitwise copy.
  fragment = saved_fragment;
}

UNICHARSET::UNICHARSET()
    : ids(), script_table(nullptr), script_table_size_used(0) {
  clear();
  for (int i = 0; i < SPECIAL_UNICHAR_CODES_COUNT; ++i) {
    unichar_insert(kSpecialUnicharCodes[i]);
    if (i == UNICHAR_JOINED) {
      set_isngram(i, true);
    }
  }
}

UNICHARSET::~UNICHARSET() {
  clear();
}

UNICHAR_ID
UNICHARSET::unichar_to_id(const char *const unichar_repr) const {
  std::string cleaned =
      old_style_included_ ? unichar_repr : CleanupString(unichar_repr);
  return ids.contains(cleaned.data(), cleaned.size())
             ? ids.unichar_to_id(cleaned.data(), cleaned.size())
             : INVALID_UNICHAR_ID;
}

UNICHAR_ID UNICHARSET::unichar_to_id(const char *const unichar_repr,
                                     int length) const {
  assert(length > 0 && length <= UNICHAR_LEN);
  std::string cleaned(unichar_repr, length);
  if (!old_style_included_) {
    cleaned = CleanupString(unichar_repr, length);
  }
  return ids.contains(cleaned.data(), cleaned.size())
             ? ids.unichar_to_id(cleaned.data(), cleaned.size())
             : INVALID_UNICHAR_ID;
}

// Return the minimum number of bytes that matches a legal UNICHAR_ID,
// while leaving the rest of the string encodable. Returns 0 if the
// beginning of the string is not encodable.
// WARNING: this function now encodes the whole string for precision.
// Use encode_string in preference to repeatedly calling step.
int UNICHARSET::step(const char *str) const {
  std::vector<UNICHAR_ID> encoding;
  std::vector<char> lengths;
  encode_string(str, true, &encoding, &lengths, nullptr);
  if (encoding.empty() || encoding[0] == INVALID_UNICHAR_ID) {
    return 0;
  }
  return lengths[0];
}

// Return whether the given UTF-8 string is encodable with this UNICHARSET.
// If not encodable, write the first byte offset which cannot be converted
// into the second (return) argument.
bool UNICHARSET::encodable_string(const char *str,
                                  unsigned *first_bad_position) const {
  std::vector<UNICHAR_ID> encoding;
  return encode_string(str, true, &encoding, nullptr, first_bad_position);
}

// Encodes the given UTF-8 string with this UNICHARSET.
// Returns true if the encoding succeeds completely, false if there is at
// least one INVALID_UNICHAR_ID in the returned encoding, but in this case
// the rest of the string is still encoded.
// If lengths is not nullptr, then it is filled with the corresponding
// byte length of each encoded UNICHAR_ID.
// WARNING: Caller must guarantee that str has already been cleaned of codes
// that do not belong in the unicharset, or encoding may fail.
// Use CleanupString to perform the cleaning.
bool UNICHARSET::encode_string(const char *str, bool give_up_on_failure,
                               std::vector<UNICHAR_ID> *encoding,
                               std::vector<char> *lengths,
                               unsigned *encoded_length) const {
  std::vector<UNICHAR_ID> working_encoding;
  std::vector<char> working_lengths;
  std::vector<char> best_lengths;
  encoding->clear(); // Just in case str is empty.
  auto str_length = strlen(str);
  unsigned str_pos = 0;
  bool perfect = true;
  while (str_pos < str_length) {
    encode_string(str, str_pos, str_length, &working_encoding, &working_lengths,
                  &str_pos, encoding, &best_lengths);
    if (str_pos < str_length) {
      // This is a non-match. Skip one utf-8 character.
      perfect = false;
      if (give_up_on_failure) {
        break;
      }
      int step = UNICHAR::utf8_step(str + str_pos);
      if (step == 0) {
        step = 1;
      }
      encoding->push_back(INVALID_UNICHAR_ID);
      best_lengths.push_back(step);
      str_pos += step;
      working_encoding = *encoding;
      working_lengths = best_lengths;
    }
  }
  if (lengths != nullptr) {
    *lengths = std::move(best_lengths);
  }
  if (encoded_length != nullptr) {
    *encoded_length = str_pos;
  }
  return perfect;
}

const char *UNICHARSET::id_to_unichar(UNICHAR_ID id) const {
  if (id == INVALID_UNICHAR_ID) {
    return INVALID_UNICHAR;
  }
  ASSERT_HOST(static_cast<unsigned>(id) < this->size());
  return unichars[id].representation;
}

const char *UNICHARSET::id_to_unichar_ext(UNICHAR_ID id) const {
  if (id == INVALID_UNICHAR_ID) {
    return INVALID_UNICHAR;
  }
  ASSERT_HOST(static_cast<unsigned>(id) < this->size());
  // Resolve from the kCustomLigatures table if this is a private encoding.
  if (get_isprivate(id)) {
    const char *ch = id_to_unichar(id);
    for (int i = 0; kCustomLigatures[i][0] != nullptr; ++i) {
      if (!strcmp(ch, kCustomLigatures[i][1])) {
        return kCustomLigatures[i][0];
      }
    }
  }
  // Otherwise return the stored representation.
  return unichars[id].representation;
}

// Return a string that reformats the utf8 str into the str followed
// by its hex unicodes.
std::string UNICHARSET::debug_utf8_str(const char *str) {
  std::string result = str;
  result += " [";
  int step = 1;
  // Chop into unicodes and code each as hex.
  for (int i = 0; str[i] != '\0'; i += step) {
    char hex[sizeof(int) * 2 + 1];
    step = UNICHAR::utf8_step(str + i);
    if (step == 0) {
      step = 1;
      snprintf(hex, sizeof(hex), "%x", str[i]);
    } else {
      UNICHAR ch(str + i, step);
      snprintf(hex, sizeof(hex), "%x", ch.first_uni());
    }
    result += hex;
    result += " ";
  }
  result += "]";
  return result;
}

// Return a string containing debug information on the unichar, including
// the id_to_unichar, its hex unicodes and the properties.
std::string UNICHARSET::debug_str(UNICHAR_ID id) const {
  if (id == INVALID_UNICHAR_ID) {
    return std::string(id_to_unichar(id));
  }
  const CHAR_FRAGMENT *fragment = this->get_fragment(id);
  if (fragment) {
    return fragment->to_string();
  }
  const char *str = id_to_unichar(id);
  std::string result = debug_utf8_str(str);
  // Append a for lower alpha, A for upper alpha, and x if alpha but neither.
  if (get_isalpha(id)) {
    if (get_islower(id)) {
      result += "a";
    } else if (get_isupper(id)) {
      result += "A";
    } else {
      result += "x";
    }
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
  unichars[unichar_id].properties.normed_ids.clear();
  if (unichar_id == UNICHAR_SPACE && id_to_unichar(unichar_id)[0] == ' ') {
    unichars[unichar_id].properties.normed_ids.push_back(UNICHAR_SPACE);
  } else if (!encode_string(unichars[unichar_id].properties.normed.c_str(),
                            true, &unichars[unichar_id].properties.normed_ids,
                            nullptr, nullptr)) {
    unichars[unichar_id].properties.normed_ids.clear();
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
  for (auto &uc : unichars) {
    uc.properties.SetRangesEmpty();
  }
}

// Sets all the properties for this unicharset given a src unicharset with
// everything set. The unicharsets don't have to be the same, and graphemes
// are correctly accounted for.
void UNICHARSET::PartialSetPropertiesFromOther(int start_index,
                                               const UNICHARSET &src) {
  for (unsigned ch = start_index; ch < unichars.size(); ++ch) {
    const char *utf8 = id_to_unichar(ch);
    UNICHAR_PROPERTIES properties;
    if (src.GetStrProperties(utf8, &properties)) {
      // Setup the script_id, other_case, and mirror properly.
      const char *script = src.get_script_from_script_id(properties.script_id);
      properties.script_id = add_script(script);
      const char *other_case = src.id_to_unichar(properties.other_case);
      if (contains_unichar(other_case)) {
        properties.other_case = unichar_to_id(other_case);
      } else {
        properties.other_case = ch;
      }
      const char *mirror_str = src.id_to_unichar(properties.mirror);
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
void UNICHARSET::ExpandRangesFromOther(const UNICHARSET &src) {
  for (unsigned ch = 0; ch < unichars.size(); ++ch) {
    const char *utf8 = id_to_unichar(ch);
    UNICHAR_PROPERTIES properties;
    if (src.GetStrProperties(utf8, &properties)) {
      // Expand just the ranges from properties.
      unichars[ch].properties.ExpandRangesFrom(properties);
    }
  }
}

// Makes this a copy of src. Clears this completely first, so the automatic
// ids will not be present in this if not in src. Does NOT reorder the set!
void UNICHARSET::CopyFrom(const UNICHARSET &src) {
  clear();
  for (unsigned ch = 0; ch < src.unichars.size(); ++ch) {
    const UNICHAR_PROPERTIES &src_props = src.unichars[ch].properties;
    const char *utf8 = src.id_to_unichar(ch);
    unichar_insert_backwards_compatible(utf8);
    unichars[ch].properties.ExpandRangesFrom(src_props);
  }
  // Set properties, including mirror and other_case, WITHOUT reordering
  // the unicharset.
  PartialSetPropertiesFromOther(0, src);
}

// For each id in src, if it does not occur in this, add it, as in
// SetPropertiesFromOther, otherwise expand the ranges, as in
// ExpandRangesFromOther.
void UNICHARSET::AppendOtherUnicharset(const UNICHARSET &src) {
  int initial_used = unichars.size();
  for (unsigned ch = 0; ch < src.unichars.size(); ++ch) {
    const UNICHAR_PROPERTIES &src_props = src.unichars[ch].properties;
    const char *utf8 = src.id_to_unichar(ch);
    int id = unichars.size();
    if (contains_unichar(utf8)) {
      id = unichar_to_id(utf8);
      // Just expand current ranges.
      unichars[id].properties.ExpandRangesFrom(src_props);
    } else {
      unichar_insert_backwards_compatible(utf8);
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
  int overlap = std::min(unichars[id1].properties.max_top,
                         unichars[id2].properties.max_top) -
                std::max(unichars[id1].properties.min_top,
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
void UNICHARSET::encode_string(const char *str, int str_index, int str_length,
                               std::vector<UNICHAR_ID> *encoding,
                               std::vector<char> *lengths,
                               unsigned *best_total_length,
                               std::vector<UNICHAR_ID> *best_encoding,
                               std::vector<char> *best_lengths) const {
  if (str_index > static_cast<int>(*best_total_length)) {
    // This is the best result so far.
    *best_total_length = str_index;
    *best_encoding = *encoding;
    if (best_lengths != nullptr) {
      *best_lengths = *lengths;
    }
  }
  if (str_index == str_length) {
    return;
  }
  int encoding_index = encoding->size();
  // Find the length of the first matching unicharset member.
  int length = ids.minmatch(str + str_index);
  if (length == 0 || str_index + length > str_length) {
    return;
  }
  do {
    if (ids.contains(str + str_index, length)) {
      // Successful encoding so far.
      UNICHAR_ID id = ids.unichar_to_id(str + str_index, length);
      encoding->push_back(id);
      lengths->push_back(length);
      encode_string(str, str_index + length, str_length, encoding, lengths,
                    best_total_length, best_encoding, best_lengths);
      if (static_cast<int>(*best_total_length) == str_length) {
        return; // Tail recursion success!
      }
      // Failed with that length, truncate back and try again.
      encoding->resize(encoding_index);
      lengths->resize(encoding_index);
    }
    int step = UNICHAR::utf8_step(str + str_index + length);
    if (step == 0) {
      step = 1;
    }
    length += step;
  } while (length <= UNICHAR_LEN && str_index + length <= str_length);
}

// Gets the properties for a grapheme string, combining properties for
// multiple characters in a meaningful way where possible.
// Returns false if no valid match was found in the unicharset.
// NOTE that script_id, mirror, and other_case refer to this unicharset on
// return and will need translation if the target unicharset is different.
bool UNICHARSET::GetStrProperties(const char *utf8_str,
                                  UNICHAR_PROPERTIES *props) const {
  props->Init();
  props->SetRangesEmpty();
  int total_unicodes = 0;
  std::vector<UNICHAR_ID> encoding;
  if (!encode_string(utf8_str, true, &encoding, nullptr, nullptr)) {
    return false; // Some part was invalid.
  }
  for (auto it : encoding) {
    int id = it;
    const UNICHAR_PROPERTIES &src_props = unichars[id].properties;
    // Logical OR all the bools.
    if (src_props.isalpha) {
      props->isalpha = true;
    }
    if (src_props.islower) {
      props->islower = true;
    }
    if (src_props.isupper) {
      props->isupper = true;
    }
    if (src_props.isdigit) {
      props->isdigit = true;
    }
    if (src_props.ispunctuation) {
      props->ispunctuation = true;
    }
    if (src_props.isngram) {
      props->isngram = true;
    }
    if (src_props.enabled) {
      props->enabled = true;
    }
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
  if (this->get_isalpha(id)) {
    properties |= ISALPHA_MASK;
  }
  if (this->get_islower(id)) {
    properties |= ISLOWER_MASK;
  }
  if (this->get_isupper(id)) {
    properties |= ISUPPER_MASK;
  }
  if (this->get_isdigit(id)) {
    properties |= ISDIGIT_MASK;
  }
  if (this->get_ispunctuation(id)) {
    properties |= ISPUNCTUATION_MASK;
  }
  return properties;
}

char UNICHARSET::get_chartype(UNICHAR_ID id) const {
  if (this->get_isupper(id)) {
    return 'A';
  }
  if (this->get_islower(id)) {
    return 'a';
  }
  if (this->get_isalpha(id)) {
    return 'x';
  }
  if (this->get_isdigit(id)) {
    return '0';
  }
  if (this->get_ispunctuation(id)) {
    return 'p';
  }
  return 0;
}

void UNICHARSET::unichar_insert(const char *const unichar_repr,
                                OldUncleanUnichars old_style) {
  if (old_style == OldUncleanUnichars::kTrue) {
    old_style_included_ = true;
  }
  std::string cleaned =
      old_style_included_ ? unichar_repr : CleanupString(unichar_repr);
  if (!cleaned.empty() && !ids.contains(cleaned.data(), cleaned.size())) {
    const char *str = cleaned.c_str();
    std::vector<int> encoding;
    if (!old_style_included_ &&
        encode_string(str, true, &encoding, nullptr, nullptr)) {
      return;
    }
    unichars.emplace_back();
    auto &u = unichars.back();
    int index = 0;
    do {
      if (index >= UNICHAR_LEN) {
        fprintf(stderr, "Utf8 buffer too big, size>%d for %s\n", UNICHAR_LEN,
                unichar_repr);
        return;
      }
      u.representation[index++] = *str++;
    } while (*str != '\0');
    u.representation[index] = '\0';
    this->set_script(unichars.size() - 1, null_script);
    // If the given unichar_repr represents a fragmented character, set
    // fragment property to a pointer to CHAR_FRAGMENT class instance with
    // information parsed from the unichar representation. Use the script
    // of the base unichar for the fragmented character if possible.
    CHAR_FRAGMENT *frag = CHAR_FRAGMENT::parse_from_string(u.representation);
    u.properties.fragment = frag;
    if (frag != nullptr && this->contains_unichar(frag->get_unichar())) {
      u.properties.script_id = this->get_script(frag->get_unichar());
    }
    u.properties.enabled = true;
    ids.insert(u.representation, unichars.size() - 1);
  }
}

bool UNICHARSET::contains_unichar(const char *const unichar_repr) const {
  std::string cleaned =
      old_style_included_ ? unichar_repr : CleanupString(unichar_repr);
  return ids.contains(cleaned.data(), cleaned.size());
}

bool UNICHARSET::contains_unichar(const char *const unichar_repr,
                                  int length) const {
  if (length == 0) {
    return false;
  }
  std::string cleaned(unichar_repr, length);
  if (!old_style_included_) {
    cleaned = CleanupString(unichar_repr, length);
  }
  return ids.contains(cleaned.data(), cleaned.size());
}

bool UNICHARSET::eq(UNICHAR_ID unichar_id,
                    const char *const unichar_repr) const {
  return strcmp(this->id_to_unichar(unichar_id), unichar_repr) == 0;
}

bool UNICHARSET::save_to_string(std::string &str) const {
  const int kFileBufSize = 1024;
  char buffer[kFileBufSize + 1];
  snprintf(buffer, kFileBufSize, "%zu\n", this->size());
  str = buffer;
  for (unsigned id = 0; id < this->size(); ++id) {
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
      str += buffer;
    } else {
      std::ostringstream stream;
      stream.imbue(std::locale::classic());
      stream << this->id_to_unichar(id) << ' ' << properties << ' '
             << min_bottom << ',' << max_bottom << ',' << min_top << ','
             << max_top << ',' << width << ',' << width_sd << ',' << bearing
             << ',' << bearing_sd << ',' << advance << ',' << advance_sd << ' '
             << this->get_script_from_script_id(this->get_script(id)) << ' '
             << this->get_other_case(id) << ' ' << this->get_direction(id)
             << ' ' << this->get_mirror(id) << ' '
             << this->get_normed_unichar(id) << "\t# "
             << this->debug_str(id).c_str() << '\n';
      str += stream.str().c_str();
    }
  }
  return true;
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
  using namespace std::placeholders; // for _1, _2
  std::function<char *(char *, int)> fgets_cb =
      std::bind(&LocalFilePointer::fgets, &lfp, _1, _2);
  bool success = load_via_fgets(fgets_cb, skip_fragments);
  return success;
}

bool UNICHARSET::load_from_file(tesseract::TFile *file, bool skip_fragments) {
  using namespace std::placeholders; // for _1, _2
  std::function<char *(char *, int)> fgets_cb =
      std::bind(&tesseract::TFile::FGets, file, _1, _2);
  bool success = load_via_fgets(fgets_cb, skip_fragments);
  return success;
}

bool UNICHARSET::load_via_fgets(
    const std::function<char *(char *, int)> &fgets_cb, bool skip_fragments) {
  int unicharset_size;
  char buffer[256];

  this->clear();
  if (fgets_cb(buffer, sizeof(buffer)) == nullptr ||
      sscanf(buffer, "%d", &unicharset_size) != 1) {
    return false;
  }
  for (UNICHAR_ID id = 0; id < unicharset_size; ++id) {
    char unichar[256];
    unsigned int properties;
    char script[64];

    strncpy(script, null_script, sizeof(script) - 1);
    int min_bottom = 0;
    int max_bottom = UINT8_MAX;
    int min_top = 0;
    int max_top = UINT8_MAX;
    float width = 0.0f;
    float width_sd = 0.0f;
    float bearing = 0.0f;
    float bearing_sd = 0.0f;
    float advance = 0.0f;
    float advance_sd = 0.0f;
    // TODO(eger): check that this default it ok
    // after enabling BiDi iterator for Arabic.
    int direction = UNICHARSET::U_LEFT_TO_RIGHT;
    UNICHAR_ID other_case = unicharset_size;
    UNICHAR_ID mirror = unicharset_size;
    if (fgets_cb(buffer, sizeof(buffer)) == nullptr) {
      return false;
    }
    char normed[64];
    normed[0] = '\0';
    std::istringstream stream(buffer);
    stream.imbue(std::locale::classic());
    // 标 1 0,255,0,255,0,0,0,0,0,0 Han 68 0 68 标  # 标 [6807 ]x
    // stream.flags(std::ios::hex);
    stream >> std::setw(255) >> unichar >> std::hex >> properties >> std::dec;
    // stream.flags(std::ios::dec);
    if (stream.fail()) {
      fprintf(stderr, "%s:%d failed\n", __FILE__, __LINE__);
      return false;
    }
    auto position = stream.tellg();
    stream.seekg(position);
    char c1, c2, c3, c4, c5, c6, c7, c8, c9;
    stream >> min_bottom >> c1 >> max_bottom >> c2 >> min_top >> c3 >>
        max_top >> c4 >> width >> c5 >> width_sd >> c6 >> bearing >> c7 >>
        bearing_sd >> c8 >> advance >> c9 >> advance_sd >> std::setw(63) >>
        script >> other_case >> direction >> mirror >> std::setw(63) >> normed;
    if (stream.fail() || c1 != ',' || c2 != ',' || c3 != ',' || c4 != ',' ||
        c5 != ',' || c6 != ',' || c7 != ',' || c8 != ',' || c9 != ',') {
      stream.clear();
      stream.seekg(position);
      stream >> min_bottom >> c1 >> max_bottom >> c2 >> min_top >> c3 >>
          max_top >> c4 >> width >> c5 >> width_sd >> c6 >> bearing >> c7 >>
          bearing_sd >> c8 >> advance >> c9 >> advance_sd >> std::setw(63) >>
          script >> other_case >> direction >> mirror;
      if (stream.fail() || c1 != ',' || c2 != ',' || c3 != ',' || c4 != ',' ||
          c5 != ',' || c6 != ',' || c7 != ',' || c8 != ',' || c9 != ',') {
        stream.clear();
        stream.seekg(position);
        stream >> min_bottom >> c1 >> max_bottom >> c2 >> min_top >> c3 >>
            max_top >> std::setw(63) >> script >> other_case >> direction >>
            mirror;
        if (stream.fail() || c1 != ',' || c2 != ',' || c3 != ',') {
          stream.clear();
          stream.seekg(position);
          stream >> min_bottom >> c1 >> max_bottom >> c2 >> min_top >> c3 >>
              max_top >> std::setw(63) >> script >> other_case;
          if (stream.fail() || c1 != ',' || c2 != ',' || c3 != ',') {
            stream.clear();
            stream.seekg(position);
            stream >> std::setw(63) >> script >> other_case;
            if (stream.fail()) {
              stream.clear();
              stream.seekg(position);
              stream >> std::setw(63) >> script;
            }
          }
        }
      }
    }

    // Skip fragments if needed.
    CHAR_FRAGMENT *frag = nullptr;
    if (skip_fragments && (frag = CHAR_FRAGMENT::parse_from_string(unichar))) {
      int num_pieces = frag->get_total();
      delete frag;
      // Skip multi-element fragments, but keep singles like UNICHAR_BROKEN in.
      if (num_pieces > 1) {
        continue;
      }
    }
    // Insert unichar into unicharset and set its properties.
    if (strcmp(unichar, "NULL") == 0) {
      this->unichar_insert(" ");
    } else {
      this->unichar_insert_backwards_compatible(unichar);
    }

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
    this->set_other_case(id, (other_case < unicharset_size) ? other_case : id);
    this->set_mirror(id, (mirror < unicharset_size) ? mirror : id);
    this->set_normed(id, normed[0] != '\0' ? normed : unichar);
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
  for (unsigned id = 0; id < unichars.size(); ++id) {
    int min_bottom = 0;
    int max_bottom = UINT8_MAX;
    int min_top = 0;
    int max_top = UINT8_MAX;
    get_top_bottom(id, &min_bottom, &max_bottom, &min_top, &max_top);
    if (min_top > 0) {
      top_bottom_set_ = true;
    }
    if (get_isalpha(id)) {
      if (get_islower(id) || get_isupper(id)) {
        ++net_case_alphas;
      } else {
        --net_case_alphas;
      }
      if (min_top < kMeanlineThreshold && max_top < kMeanlineThreshold) {
        ++x_height_alphas;
      } else if (min_top > kMeanlineThreshold && max_top > kMeanlineThreshold) {
        ++cap_height_alphas;
      }
    }
    set_normed_ids(id);
  }

  script_has_upper_lower_ = net_case_alphas > 0;
  script_has_xheight_ =
      script_has_upper_lower_ ||
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
  thai_sid_ = get_script_id_from_name("Thai");
  hangul_sid_ = get_script_id_from_name("Hangul");

  // Compute default script. Use the highest-counting alpha script, that is
  // not the common script, as that still contains some "alphas".
  int *script_counts = new int[script_table_size_used];
  memset(script_counts, 0, sizeof(*script_counts) * script_table_size_used);
  for (unsigned id = 0; id < unichars.size(); ++id) {
    if (get_isalpha(id)) {
      ++script_counts[get_script(id)];
    }
  }
  default_sid_ = 0;
  for (int s = 1; s < script_table_size_used; ++s) {
    if (script_counts[s] > script_counts[default_sid_] && s != common_sid_) {
      default_sid_ = s;
    }
  }
  delete[] script_counts;
}

// Returns true if right_to_left scripts are significant in the unicharset,
// but without being so sensitive that "universal" unicharsets containing
// characters from many scripts, like orientation and script detection,
// look like they are right_to_left.
bool UNICHARSET::major_right_to_left() const {
  int ltr_count = 0;
  int rtl_count = 0;
  for (unsigned id = 0; id < unichars.size(); ++id) {
    int dir = get_direction(id);
    if (dir == UNICHARSET::U_LEFT_TO_RIGHT) {
      ltr_count++;
    }
    if (dir == UNICHARSET::U_RIGHT_TO_LEFT ||
        dir == UNICHARSET::U_RIGHT_TO_LEFT_ARABIC ||
        dir == UNICHARSET::U_ARABIC_NUMBER) {
      rtl_count++;
    }
  }
  return rtl_count > ltr_count;
}

// Set a whitelist and/or blacklist of characters to recognize.
// An empty or nullptr whitelist enables everything (minus any blacklist).
// An empty or nullptr blacklist disables nothing.
// An empty or nullptr unblacklist has no effect.
void UNICHARSET::set_black_and_whitelist(const char *blacklist,
                                         const char *whitelist,
                                         const char *unblacklist) {
  bool def_enabled = whitelist == nullptr || whitelist[0] == '\0';
  // Set everything to default
  for (auto &uc : unichars) {
    uc.properties.enabled = def_enabled;
  }
  if (!def_enabled) {
    // Enable the whitelist.
    std::vector<UNICHAR_ID> encoding;
    encode_string(whitelist, false, &encoding, nullptr, nullptr);
    for (auto it : encoding) {
      if (it != INVALID_UNICHAR_ID) {
        unichars[it].properties.enabled = true;
      }
    }
  }
  if (blacklist != nullptr && blacklist[0] != '\0') {
    // Disable the blacklist.
    std::vector<UNICHAR_ID> encoding;
    encode_string(blacklist, false, &encoding, nullptr, nullptr);
    for (auto it : encoding) {
      if (it != INVALID_UNICHAR_ID) {
        unichars[it].properties.enabled = false;
      }
    }
  }
  if (unblacklist != nullptr && unblacklist[0] != '\0') {
    // Re-enable the unblacklist.
    std::vector<UNICHAR_ID> encoding;
    encode_string(unblacklist, false, &encoding, nullptr, nullptr);
    for (auto it : encoding) {
      if (it != INVALID_UNICHAR_ID) {
        unichars[it].properties.enabled = true;
      }
    }
  }
}

// Returns true if there are any repeated unicodes in the normalized
// text of any unichar-id in the unicharset.
bool UNICHARSET::AnyRepeatedUnicodes() const {
  int start_id = 0;
  if (has_special_codes()) {
    start_id = SPECIAL_UNICHAR_CODES_COUNT;
  }
  for (unsigned id = start_id; id < unichars.size(); ++id) {
    // Convert to unicodes.
    std::vector<char32> unicodes = UNICHAR::UTF8ToUTF32(get_normed_unichar(id));
    for (size_t u = 1; u < unicodes.size(); ++u) {
      if (unicodes[u - 1] == unicodes[u]) {
        return true;
      }
    }
  }
  return false;
}

int UNICHARSET::add_script(const char *script) {
  for (int i = 0; i < script_table_size_used; ++i) {
    if (strcmp(script, script_table[i]) == 0) {
      return i;
    }
  }
  if (script_table_size_reserved == 0) {
    script_table_size_reserved = 8;
    script_table = new char *[script_table_size_reserved];
  } else if (script_table_size_used >= script_table_size_reserved) {
    assert(script_table_size_used == script_table_size_reserved);
    script_table_size_reserved += script_table_size_reserved;
    char **new_script_table = new char *[script_table_size_reserved];
    memcpy(new_script_table, script_table,
           script_table_size_used * sizeof(char *));
    delete[] script_table;
    script_table = new_script_table;
  }
  script_table[script_table_size_used] = new char[strlen(script) + 1];
  strcpy(script_table[script_table_size_used], script);
  return script_table_size_used++;
}

// Returns the string that represents a fragment
// with the given unichar, pos and total.
std::string CHAR_FRAGMENT::to_string(const char *unichar, int pos, int total,
                                     bool natural) {
  if (total == 1) {
    return std::string(unichar);
  }
  std::string result;
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
    return nullptr; // this string cannot represent a fragment
  }
  ptr++; // move to the next character
  int step = 0;
  while ((ptr + step) < (string + len) && *(ptr + step) != kSeparator) {
    step += UNICHAR::utf8_step(ptr + step);
  }
  if (step == 0 || step > UNICHAR_LEN) {
    return nullptr; // no character for unichar or the character is too long
  }
  char unichar[UNICHAR_LEN + 1];
  strncpy(unichar, ptr, step);
  unichar[step] = '\0'; // null terminate unichar
  ptr += step;          // move to the next fragment separator
  int pos = 0;
  int total = 0;
  bool natural = false;
  char *end_ptr = nullptr;
  for (int i = 0; i < 2; i++) {
    if (ptr > string + len || *ptr != kSeparator) {
      if (i == 1 && *ptr == kNaturalFlag) {
        natural = true;
      } else {
        return nullptr; // Failed to parse fragment representation.
      }
    }
    ptr++; // move to the next character
    i == 0 ? pos = static_cast<int>(strtol(ptr, &end_ptr, 10))
           : total = static_cast<int>(strtol(ptr, &end_ptr, 10));
    ptr = end_ptr;
  }
  if (ptr != string + len) {
    return nullptr; // malformed fragment representation
  }
  auto *fragment = new CHAR_FRAGMENT();
  fragment->set_all(unichar, pos, total, natural);
  return fragment;
}

int UNICHARSET::get_script_id_from_name(const char *script_name) const {
  for (int i = 0; i < script_table_size_used; ++i) {
    if (strcmp(script_name, script_table[i]) == 0) {
      return i;
    }
  }
  return 0; // 0 is always the null_script
}

// Removes/replaces content that belongs in rendered text, but not in the
// unicharset.
/* static */
std::string UNICHARSET::CleanupString(const char *utf8_str, size_t length) {
  std::string result;
  result.reserve(length);
  char ch;
  while ((ch = *utf8_str) != '\0' && length-- > 0) {
    int key_index = 0;
    const char *key;
    while ((key = kCleanupMaps[key_index][0]) != nullptr) {
      int match = 0;
      while (key[match] != '\0' && key[match] == utf8_str[match]) {
        ++match;
      }
      if (key[match] == '\0') {
        utf8_str += match;
        break;
      }
      ++key_index;
    }
    if (key == nullptr) {
      result.push_back(ch);
      ++utf8_str;
    } else {
      result.append(kCleanupMaps[key_index][1]);
    }
  }
  return result;
}

} // namespace tesseract
