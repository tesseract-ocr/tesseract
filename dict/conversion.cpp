///////////////////////////////////////////////////////////////////////
// File:        conversion.cpp
// Description: Collection of utility functions for A_CHOICE conversions.
//              TODO(daria): delete this file when conversion to unichar_ids
//              is finished and all permuters are completely updated/replaced.
// Author:      Daria Antonova
// Created:     Mon Jun 23 11:26:43 PDT 2008
//
// (C) Copyright 2007, Google Inc.
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#pragma warning(disable:4018)  // signed/unsigned warnings
#endif

#include "conversion.h"

#include "callcpp.h"
#include "choicearr.h"
#include "choices.h"
#include "dict.h"
#include "ratngs.h"
#include "stopper.h"
#include "unicharset.h"

namespace tesseract {
int Dict::valid_word(const char *string) {
  WERD_CHOICE word(string, getUnicharset());
  return valid_word(word);
}

void Dict::LogNewWordChoice(A_CHOICE *a_choice,
                            FLOAT32 adjust_factor,
                            const float certainties[],
                            const UNICHARSET &unicharset) {
  WERD_CHOICE word_choice(strlen(a_choice->lengths));
  convert_to_word_choice(a_choice, unicharset, &word_choice);
  LogNewChoice(word_choice, adjust_factor, certainties, false);
}
}  // namespace tesseract

// Fills in the given WERD_CHOICE with information from the given A_CHOICE.
// Assumes that word_choice pointer is not NULL.
void convert_to_word_choice(const A_CHOICE *a_choice,
                            const UNICHARSET &current_unicharset,
                            WERD_CHOICE *word_choice) {
  if (a_choice == NULL) return;
  const char *string = a_choice->string;
  const char *lengths = a_choice->lengths;
  const char *fragment_lengths = a_choice->fragment_lengths;
  int offset = 0;
  for (int x = 0; x < strlen(a_choice->lengths); ++x) {
    UNICHAR_ID unichar_id =
      current_unicharset.unichar_to_id(string + offset, lengths[x]);
    word_choice->append_unichar_id(unichar_id, fragment_lengths[x], 0.0, 0.0);
    offset += lengths[x];
  }
  word_choice->set_rating(a_choice->rating);
  word_choice->set_certainty(a_choice->certainty);
  word_choice->set_permuter(a_choice->permuter);
  word_choice->set_fragment_mark(a_choice->fragment_mark);
}

// Returns the best of two choices and deletes the other (worse) choice.
// A choice is better if it has a non-empty string and has a lower
// rating than the other choice. If the ratings are the same,
// a_choice is preferred over choice.
// If the best choice is in the A_CHOICE form, copies it to a new
// WERD_CHOICE and deletes A_CHOICE.
WERD_CHOICE *get_best_delete_other(const UNICHARSET &current_unicharset,
                                   WERD_CHOICE *choice,
                                   A_CHOICE *a_choice) {
  if (!a_choice) return choice;
  if (choice != NULL &&
      (choice->rating() < a_choice->rating || a_choice->string == NULL)) {
    free_choice(a_choice);
    return choice;
  } else {
    delete choice;
    WERD_CHOICE *word_choice = new WERD_CHOICE();
    convert_to_word_choice(a_choice, current_unicharset, word_choice);
    free_choice(a_choice);
    return word_choice;
  }
}

// Convert BLOB_CHOICE_LIST_VECTOR to CHOICES_LIST.
// The caller is responsible for deleting the returned CHOICES_LIST.
CHOICES_LIST convert_to_choices_list(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    const UNICHARSET &current_unicharset) {
  CHOICES_LIST old_char_choices = new_choice_list();
  int x;
  BLOB_CHOICE_IT it;
  BLOB_CHOICE *blob_choice;
  char choice_lengths[2] = {0, 0};
  char unichar[UNICHAR_LEN + 1];
  for (x = 0; x < char_choices.length(); ++x) {
    it.set_to_list(char_choices.get(x));
    LIST result = NIL;
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      blob_choice = it.data();
      strcpy(unichar,
             current_unicharset.id_to_unichar(blob_choice->unichar_id()));
      choice_lengths[0] = strlen(unichar);
      result = append_char_choice(result, unichar, choice_lengths,
                                  blob_choice->rating(),
                                  blob_choice->certainty(),
                                  blob_choice->config(), NULL);
    }
    old_char_choices = array_push(old_char_choices, result);
  }
  return old_char_choices;
}
