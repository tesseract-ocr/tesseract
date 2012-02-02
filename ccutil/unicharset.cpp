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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "tesscallback.h"
#include "tprintf.h"
#include "unichar.h"
#include "unicharset.h"
#include "params.h"

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
  min_width = 0;
  max_width = MAX_INT16;
  min_bearing = 0;
  max_bearing = MAX_INT16;
  min_advance = 0;
  max_advance = MAX_INT16;
}

// Sets all ranges to empty. Used before expanding with font-based data.
void UNICHARSET::UNICHAR_PROPERTIES::SetRangesEmpty() {
  min_bottom = MAX_UINT8;
  max_bottom = 0;
  min_top = MAX_UINT8;
  max_top = 0;
  min_width = MAX_INT16;
  max_width = 0;
  min_bearing = MAX_INT16;
  max_bearing = 0;
  min_advance = MAX_INT16;
  max_advance = 0;
}

// Returns true if any of the top/bottom/width/bearing/advance ranges is
// emtpy.
bool UNICHARSET::UNICHAR_PROPERTIES::AnyRangeEmpty() const {
  return min_bottom > max_bottom || min_top > max_top ||
      min_width > max_width || min_bearing > max_bearing ||
      min_advance > max_advance;
}

// Expands the ranges with the ranges from the src properties.
void UNICHARSET::UNICHAR_PROPERTIES::ExpandRangesFrom(
    const UNICHAR_PROPERTIES& src) {
  UpdateRange(src.min_bottom, &min_bottom, &max_bottom);
  UpdateRange(src.max_bottom, &min_bottom, &max_bottom);
  UpdateRange(src.min_top, &min_top, &max_top);
  UpdateRange(src.max_top, &min_top, &max_top);
  UpdateRange(src.min_width, &min_width, &max_width);
  UpdateRange(src.max_width, &min_width, &max_width);
  UpdateRange(src.min_bearing, &min_bearing, &max_bearing);
  UpdateRange(src.max_bearing, &min_bearing, &max_bearing);
  UpdateRange(src.min_advance, &min_advance, &max_advance);
  UpdateRange(src.max_advance, &min_advance, &max_advance);
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

const UNICHAR_ID
UNICHARSET::unichar_to_id(const char* const unichar_repr) const {
  return ids.contains(unichar_repr) ?
    ids.unichar_to_id(unichar_repr) : INVALID_UNICHAR_ID;
}

const UNICHAR_ID UNICHARSET::unichar_to_id(const char* const unichar_repr,
                                           int length) const {
  assert(length > 0 && length <= UNICHAR_LEN);
  return ids.contains(unichar_repr, length) ?
    ids.unichar_to_id(unichar_repr, length) : INVALID_UNICHAR_ID;
}

// Return the minimum number of bytes that matches a legal UNICHAR_ID,
// while leaving a legal UNICHAR_ID afterwards. In other words, if there
// is both a short and a long match to the string, return the length that
// ensures there is a legal match after it.
int UNICHARSET::step(const char* str) const {
  // Find the length of the first matching unicharset member.
  int minlength = ids.minmatch(str);
  if (minlength == 0)
    return 0;  // Empty string or illegal char.

  int goodlength = minlength;
  while (goodlength <= UNICHAR_LEN) {
    if (str[goodlength] == '\0' || ids.minmatch(str + goodlength) > 0)
      return goodlength;  // This length works!

    // The next char is illegal so find the next usable length.
    do {
      ++goodlength;
    } while (str[goodlength] != '\0' && goodlength <= UNICHAR_LEN &&
             !ids.contains(str, goodlength));
    if (goodlength > UNICHAR_LEN || !ids.contains(str, goodlength)) {
      // This does not constitute a good length!
      return minlength;
    }
  }
  // Search to find a subsequent legal char failed so return the minlength.
  return minlength;
}

// Return whether the given UTF-8 string is encodable with this UNICHARSET.
// If not encodable, write the first byte offset which cannot be converted
// into the second (return) argument.
bool UNICHARSET::encodable_string(const char *str,
                                  int *first_bad_position) const {
  for (int i = 0, len = strlen(str); i < len; ) {
    int increment = step(str + i);
    if (increment == 0) {
      if (first_bad_position) *first_bad_position = i;
      return false;
    }
    i += increment;
  }
  return true;
}

const char* const UNICHARSET::id_to_unichar(UNICHAR_ID id) const {
  if (id == INVALID_UNICHAR_ID) {
    return INVALID_UNICHAR;
  }
  ASSERT_HOST(id < this->size());
  return unichars[id].representation;
}

const char* const UNICHARSET::id_to_unichar_ext(UNICHAR_ID id) const {
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
void UNICHARSET::SetPropertiesFromOther(const UNICHARSET& src) {
  for (int ch = 0; ch < size_used; ++ch) {
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

// For each id in src, if it does not occur in this, add it, as in
// SetPropertiesFromOther, otherwise expand the ranges, as in
// ExpandRangesFromOther.
void UNICHARSET::AppendOtherUnicharset(const UNICHARSET& src) {
  for (int ch = 0; ch < src.size_used; ++ch) {
    const UNICHAR_PROPERTIES& src_props = src.unichars[ch].properties;
    const char* utf8 = src.id_to_unichar(ch);
    if (strcmp(utf8, " ") != 0 && src_props.AnyRangeEmpty()) {
      // Only use fully valid entries.
      tprintf("Bad properties for char %s: %d,%d %d,%d %d,%d %d,%d %d,%d\n",
              utf8, src_props.min_bottom, src_props.max_bottom,
              src_props.min_top, src_props.max_top,
              src_props.min_width, src_props.max_width,
              src_props.min_bearing, src_props.max_bearing,
              src_props.min_advance, src_props.max_advance);
      continue;
    }
    int id = size_used;
    if (contains_unichar(utf8)) {
      id = unichar_to_id(utf8);
    } else {
      unichar_insert(utf8);
      unichars[id].properties.SetRangesEmpty();
    }
    if (!unichars[id].properties.AnyRangeEmpty()) {
      // Just expand current ranges.
      unichars[id].properties.ExpandRangesFrom(src_props);
    } else {
      // Copy properties from src_props.
      unichars[id].properties.CopyFrom(src_props);
      // Setup the script_id, other_case and mirror properly.
      const char* script = src.get_script_from_script_id(src_props.script_id);
      unichars[id].properties.script_id = add_script(script);
      const char* other_case = src.id_to_unichar(src_props.other_case);
      if (!contains_unichar(other_case)) {
        unichar_insert(other_case);
        unichars[size_used - 1].properties.SetRangesEmpty();
        // Other_case will have its ranges set later as it is contained in src.
      }
      unichars[id].properties.other_case = unichar_to_id(other_case);
      const char* mirror_str = src.id_to_unichar(src_props.mirror);
      if (!contains_unichar(mirror_str)) {
        unichar_insert(mirror_str);
        unichars[size_used - 1].properties.SetRangesEmpty();
        // Mirror will have its ranges set later as it is contained in src.
      }
      unichars[id].properties.mirror = unichar_to_id(mirror_str);
    }
  }
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
  props->min_advance = 0;
  props->max_advance = 0;
  int utf8_step = 0;
  int total_unicodes = 0;
  for (int offset = 0; utf8_str[offset] != '\0'; offset += utf8_step) {
    utf8_step = step(utf8_str + offset);
    if (utf8_step == 0) return false;
    int id = unichar_to_id(utf8_str + offset, utf8_step);
    if (id < 0) return false;
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
    int bearing = props->min_advance + src_props.min_bearing;
    if (total_unicodes == 0 || bearing < props->min_bearing)
      props->min_bearing = bearing;
    bearing = props->max_advance + src_props.max_bearing;
    if (total_unicodes == 0 || bearing < props->max_bearing)
      props->max_bearing = bearing;
    props->min_advance += src_props.min_advance;
    props->max_advance += src_props.max_advance;
    // With a single width, just use the widths stored in the unicharset.
    props->min_width = src_props.min_width;
    props->max_width = src_props.max_width;
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
    props->min_width = props->min_advance - props->max_bearing;
    props->max_width = props->max_advance - props->min_bearing;
  }
  return total_unicodes > 0;
}

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

bool UNICHARSET::save_to_file(FILE *file) const {
  fprintf(file, "%d\n", this->size());
  for (UNICHAR_ID id = 0; id < this->size(); ++id) {
    int min_bottom, max_bottom, min_top, max_top;
    get_top_bottom(id, &min_bottom, &max_bottom, &min_top, &max_top);
    int min_width, max_width;
    get_width_range(id, &min_width, &max_width);
    int min_bearing, max_bearing;
    get_bearing_range(id, &min_bearing, &max_bearing);
    int min_advance, max_advance;
    get_advance_range(id, &min_advance, &max_advance);
    unsigned int properties = this->get_properties(id);
    if (strcmp(this->id_to_unichar(id), " ") == 0) {
      fprintf(file, "%s %x %s %d\n", "NULL", properties,
              this->get_script_from_script_id(this->get_script(id)),
              this->get_other_case(id));
    } else {
      fprintf(file,
              "%s %x %d,%d,%d,%d,%d,%d,%d,%d,%d,%d %s %d %d %d %s\t# %s\n",
              this->id_to_unichar(id), properties,
              min_bottom, max_bottom, min_top, max_top, min_width, max_width,
              min_bearing, max_bearing, min_advance, max_advance,
              this->get_script_from_script_id(this->get_script(id)),
              this->get_other_case(id), this->get_direction(id),
              this->get_mirror(id), this->get_normed_unichar(id),
              this->debug_str(id).string());
    }
  }
  return true;
}

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
    int min_width = 0;
    int max_width = MAX_INT16;
    int min_bearing = 0;
    int max_bearing = MAX_INT16;
    int min_advance = 0;
    int max_advance = MAX_INT16;
    // TODO(eger): check that this default it ok
    // after enabling BiDi iterator for Arabic+Cube.
    int direction = UNICHARSET::U_LEFT_TO_RIGHT;
    UNICHAR_ID other_case = id;
    UNICHAR_ID mirror = id;
    char normed[64];
    int v = -1;
    if (fgets_cb->Run(buffer, sizeof (buffer)) == NULL ||
        ((v = sscanf(buffer,
                     "%s %x %d,%d,%d,%d,%d,%d,%d,%d,%d,%d %63s %d %d %d %63s",
                     unichar, &properties,
                     &min_bottom, &max_bottom, &min_top, &max_top,
                     &min_width, &max_width, &min_bearing, &max_bearing,
                     &min_advance, &max_advance, script, &other_case,
                     &direction, &mirror, normed)) != 17 &&
         (v = sscanf(buffer,
                     "%s %x %d,%d,%d,%d,%d,%d,%d,%d,%d,%d %63s %d %d %d",
                     unichar, &properties,
                     &min_bottom, &max_bottom, &min_top, &max_top,
                     &min_width, &max_width, &min_bearing, &max_bearing,
                     &min_advance, &max_advance,
                     script, &other_case, &direction, &mirror)) != 16 &&
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
          (v = sscanf(buffer, "%s %x", unichar, &properties) != 2))) {
      return false;
    }

    // Skip fragments if needed.
    CHAR_FRAGMENT *frag = NULL;
    if (skip_fragments && (frag = CHAR_FRAGMENT::parse_from_string(unichar))) {
      delete frag;
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
    this->set_width_range(id, min_width, max_width);
    this->set_bearing_range(id, min_bearing, max_bearing);
    this->set_advance_range(id, min_advance, max_advance);
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
void UNICHARSET::set_black_and_whitelist(const char* blacklist,
                                         const char* whitelist) {
  bool def_enabled = whitelist == NULL || whitelist[0] == '\0';
  // Set everything to default
  for (int ch = 0; ch < size_used; ++ch)
    unichars[ch].properties.enabled = def_enabled;
  int ch_step;
  if (!def_enabled) {
    // Enable the whitelist.
    for (int w_ind = 0; whitelist[w_ind] != '\0'; w_ind += ch_step) {
      ch_step = step(whitelist + w_ind);
      if (ch_step > 0) {
        UNICHAR_ID u_id = unichar_to_id(whitelist + w_ind, ch_step);
        if (u_id != INVALID_UNICHAR_ID) {
          unichars[u_id].properties.enabled = true;
        }
      } else {
        ch_step = 1;
      }
    }
  }
  if (blacklist != NULL && blacklist[0] != '\0') {
    // Disable the blacklist.
    for (int b_ind = 0; blacklist[b_ind] != '\0'; b_ind += ch_step) {
      ch_step = step(blacklist + b_ind);
      if (ch_step > 0) {
        UNICHAR_ID u_id = unichar_to_id(blacklist + b_ind, ch_step);
        if (u_id != INVALID_UNICHAR_ID) {
          unichars[u_id].properties.enabled = false;
        }
      } else {
        ch_step = 1;
      }
    }
  }
}

int UNICHARSET::add_script(const char* script) {
  for (int i = 0; i < script_table_size_used; ++i) {
    if (strcmp(script, script_table[i]) == 0)
      return i;
  }
  if (script_table_size_reserved == 0) {
    script_table_size_reserved = 8;
    script_table = new char*[script_table_size_reserved];
  }
  if (script_table_size_used + 1 >= script_table_size_reserved) {
    char** new_script_table = new char*[script_table_size_reserved * 2];
    memcpy(new_script_table, script_table, script_table_size_reserved * sizeof(char*));
    delete[] script_table;
    script_table = new_script_table;
      script_table_size_reserved = 2 * script_table_size_reserved;
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
