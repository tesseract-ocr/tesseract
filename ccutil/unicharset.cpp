
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

#include "unichar.h"
#include "unicharset.h"

static const int ISALPHA_MASK = 0x1;
static const int ISLOWER_MASK = 0x2;
static const int ISUPPER_MASK = 0x4;
static const int ISDIGIT_MASK = 0x8;

UNICHARSET::UNICHARSET() :
    unichars(NULL),
    ids(),
    size_used(0),
    size_reserved(0)
{
}

UNICHARSET::~UNICHARSET() {
  if (size_reserved > 0)
    delete[] unichars;
}

void UNICHARSET::reserve(int unichars_number) {
  if (unichars_number > size_reserved) {
    UNICHAR_SLOT* unichars_new = new UNICHAR_SLOT[unichars_number];
    for (int i = 0; i < size_used; ++i)
      memcpy(&unichars_new[i], &unichars[i], sizeof(UNICHAR_SLOT));
    delete[] unichars;
    unichars = unichars_new;
    size_reserved = unichars_number;
  }
}

const UNICHAR_ID
UNICHARSET::unichar_to_id(const char* const unichar_repr) const {
  assert(ids.contains(unichar_repr));
  return ids.unichar_to_id(unichar_repr);
}

const UNICHAR_ID UNICHARSET::unichar_to_id(const char* const unichar_repr,
                                           int length) const {
  assert(length > 0 && length <= UNICHAR_LEN);
  assert(ids.contains(unichar_repr, length));
  return ids.unichar_to_id(unichar_repr, length);
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
  assert(id < this->size());
  return unichars[id].representation;
}

void UNICHARSET::unichar_insert(const char* const unichar_repr) {
  if (!ids.contains(unichar_repr)) {
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
    this->unichars[size_used].properties.enabled = true;
    ids.insert(unichar_repr, size_used);
    ++size_used;
  }
}

bool UNICHARSET::contains_unichar(const char* const unichar_repr) {
  return ids.contains(unichar_repr);
}

bool UNICHARSET::contains_unichar(const char* const unichar_repr, int length) {
  return ids.contains(unichar_repr, length);
}

bool UNICHARSET::eq(UNICHAR_ID unichar_id, const char* const unichar_repr) {
  return strcmp(this->id_to_unichar(unichar_id), unichar_repr) == 0;
}

bool UNICHARSET::save_to_file(const char* filename) const {
  FILE* file = fopen(filename, "w+");

  if (file == NULL)
    return false;

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

    if (strcmp(this->id_to_unichar(id), " ") == 0)
      fprintf(file, "%s %x\n", "NULL", properties);
    else
      fprintf(file, "%s %x\n", this->id_to_unichar(id), properties);
  }
  fclose(file);
  return true;
}

bool UNICHARSET::load_from_file(const char* filename) {
  FILE* file = fopen(filename, "r");
  int unicharset_size;
  char buffer[256];

  if (file == NULL)
    return false;

  this->clear();
  if (fgets(buffer, sizeof (buffer), file) == NULL ||
      sscanf(buffer, "%d", &unicharset_size) != 1) {
    fclose(file);
    return false;
  }
  this->reserve(unicharset_size);
  for (UNICHAR_ID id = 0; id < unicharset_size; ++id) {
    char unichar[256];
    unsigned int properties;

    if (fgets(buffer, sizeof (buffer), file) == NULL ||
        sscanf(buffer, "%s %x", unichar, &properties) != 2) {
      fclose(file);
      return false;
    }
    if (strcmp(unichar, "NULL") == 0)
      this->unichar_insert(" ");
    else
      this->unichar_insert(unichar);

    this->set_isalpha(id, properties & ISALPHA_MASK);
    this->set_islower(id, properties & ISLOWER_MASK);
    this->set_isupper(id, properties & ISUPPER_MASK);
    this->set_isdigit(id, properties & ISDIGIT_MASK);
    this->unichars[id].properties.enabled = true;
  }
  fclose(file);
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

