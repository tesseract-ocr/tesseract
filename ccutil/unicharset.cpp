
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
  if (unichars_number > size_reserved)
  {
    UNICHAR_SLOT* unichars_new = new UNICHAR_SLOT[unichars_number];
    for (int i = 0; i < size_used; ++i)
      memcpy(&unichars_new[i], &unichars[i], sizeof (UNICHAR_SLOT));
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

const char* const UNICHARSET::id_to_unichar(UNICHAR_ID id) const {
  assert(id < this->size());
  return unichars[id].representation;
}

void UNICHARSET::unichar_insert(const char* const unichar_repr) {
  if (!ids.contains(unichar_repr)) {
    if (size_used == size_reserved)
    {
      if (size_used == 0)
        reserve(8);
      else
        reserve(2 * size_used);
    }

    strcpy(unichars[size_used].representation, unichar_repr);
    ids.insert(unichar_repr, size_used);
    ++size_used;
  }
}

bool UNICHARSET::contains_unichar(const char* const unichar_repr) {
  return ids.contains(unichar_repr);
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
      sscanf(buffer, "%d", &unicharset_size) != 1)
  {
    fclose(file);
    return false;
  }
  this->reserve(unicharset_size);
  for (UNICHAR_ID id = 0; id < unicharset_size; ++id) {
    char unichar[256];
    unsigned int properties;

    if (fgets(buffer, sizeof (buffer), file) == NULL ||
        sscanf(buffer, "%s %x", unichar, &properties) != 2)
    {
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
  }
  fclose(file);
  return true;
}
