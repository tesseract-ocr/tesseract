
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

#include "tprintf.h"
#include "unichar.h"
#include "unicharset.h"
#include "varable.h"

static const int ISALPHA_MASK = 0x1;
static const int ISLOWER_MASK = 0x2;
static const int ISUPPER_MASK = 0x4;
static const int ISDIGIT_MASK = 0x8;
static const int ISPUNCTUATION_MASK = 0x10;

UNICHARSET::UNICHARSET() :
    unichars(NULL),
    ids(),
    size_used(0),
    size_reserved(0),
    script_table(0),
    script_table_size_used(0),
    script_table_size_reserved(0),
    null_script("NULL"),
    null_sid_(0),
    common_sid_(0),
    latin_sid_(0),
    cyrillic_sid_(0),
    greek_sid_(0),
    han_sid_(0) {}

UNICHARSET::~UNICHARSET() {
  if (size_reserved > 0) {
    for (int i = 0; i < script_table_size_used; ++i)
      delete[] script_table[i];
    delete[] script_table;
    delete_pointers_in_unichars();
    delete[] unichars;
  }
}

void UNICHARSET::reserve(int unichars_number) {
  if (unichars_number > size_reserved) {
    UNICHAR_SLOT* unichars_new = new UNICHAR_SLOT[unichars_number];
    for (int i = 0; i < size_used; ++i)
      memcpy(&unichars_new[i], &unichars[i], sizeof(UNICHAR_SLOT));
    for (int j = size_used; j < unichars_number; ++j) {
      unichars_new[j].properties.script_id = add_script(null_script);
      unichars_new[j].properties.fragment = NULL;
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
  }
  // Search to find a subsequent legal char failed so return the minlength.
  return minlength;
}

const char* const UNICHARSET::id_to_unichar(UNICHAR_ID id) const {
  if (id == INVALID_UNICHAR_ID) {
    return INVALID_UNICHAR;
  }
  assert(id < this->size());
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
  const CHAR_FRAGMENT *fragment = this->get_fragment(id);
  if (fragment) {
    STRING base = debug_str(fragment->get_unichar());
    return CHAR_FRAGMENT::to_string(base.string(), fragment->get_pos(),
                                    fragment->get_total());
  }
  const char* str = id_to_unichar(id);
  if (id == INVALID_UNICHAR_ID) return STRING(str);
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
    this->set_isalpha(size_used, false);
    this->set_islower(size_used, false);
    this->set_isupper(size_used, false);
    this->set_isdigit(size_used, false);
    this->set_ispunctuation(size_used, false);
    this->set_isngram(size_used, false);
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

    if (strcmp(this->id_to_unichar(id), " ") == 0)
      fprintf(file, "%s %x %s %d\n", "NULL", properties,
              this->get_script_from_script_id(this->get_script(id)),
              this->get_other_case(id));
    else
      fprintf(file, "%s %x %s %d\n", this->id_to_unichar(id), properties,
              this->get_script_from_script_id(this->get_script(id)),
              this->get_other_case(id));
  }
  return true;
}

bool UNICHARSET::load_from_file(FILE *file) {
  int unicharset_size;
  char buffer[256];

  this->clear();
  if (fgets(buffer, sizeof (buffer), file) == NULL ||
      sscanf(buffer, "%d", &unicharset_size) != 1) {
    return false;
  }
  this->reserve(unicharset_size);
  for (UNICHAR_ID id = 0; id < unicharset_size; ++id) {
    char unichar[256];
    unsigned int properties;
    char script[64];

    strcpy(script, null_script);
    this->unichars[id].properties.other_case = id;
    if (fgets(buffer, sizeof (buffer), file) == NULL ||
        (sscanf(buffer, "%s %x %63s %d", unichar, &properties,
                script, &(this->unichars[id].properties.other_case)) != 4 &&
         sscanf(buffer, "%s %x %63s", unichar, &properties, script) != 3 &&
         sscanf(buffer, "%s %x", unichar, &properties) != 2)) {
      return false;
    }
    if (strcmp(unichar, "NULL") == 0)
      this->unichar_insert(" ");
    else
      this->unichar_insert(unichar);

    this->set_isalpha(id, (properties & ISALPHA_MASK) != 0);
    this->set_islower(id, (properties & ISLOWER_MASK) != 0);
    this->set_isupper(id, (properties & ISUPPER_MASK) != 0);
    this->set_isdigit(id, (properties & ISDIGIT_MASK) != 0);
    this->set_ispunctuation(id, (properties & ISPUNCTUATION_MASK) != 0);
    this->set_isngram(id, false);
    this->set_script(id, script);
    this->unichars[id].properties.enabled = true;
  }

  null_sid_ = get_script_id_from_name(null_script);
  ASSERT_HOST(null_sid_ == 0);
  common_sid_ = get_script_id_from_name("Common");
  latin_sid_ = get_script_id_from_name("Latin");
  cyrillic_sid_ = get_script_id_from_name("Cyrillic");
  greek_sid_ = get_script_id_from_name("Greek");
  han_sid_ = get_script_id_from_name("Han");
  return true;
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
        unichars[u_id].properties.enabled = true;
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
        unichars[u_id].properties.enabled = false;
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
  char *end_ptr = NULL;
  for (int i = 0; i < 2; i++) {
    if (ptr > string + len || *ptr != kSeparator) {
      return NULL;  // failed to parse fragment representation
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
  fragment->set_all(unichar, pos, total);
  return fragment;
}

int UNICHARSET::get_script_id_from_name(const char* script_name) const {
  for (int i = 0; i < script_table_size_used; ++i) {
    if (strcmp(script_name, script_table[i]) == 0)
      return i;
  }
  return 0;  // 0 is always the null_script
}
