///////////////////////////////////////////////////////////////////////
// File:        conversion.h
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

#ifndef TESSERACT_DICT_CONVERSION_H_
#define TESSERACT_DICT_CONVERSION_H_

#include "choicearr.h"
#include "choices.h"
#include "ratngs.h"
#include "unicharset.h"

void convert_to_word_choice(const A_CHOICE *a_choice,
                            const UNICHARSET &current_unicharset,
                            WERD_CHOICE *word_choice);

WERD_CHOICE *get_best_delete_other(
    const UNICHARSET &current_unicharset,
    WERD_CHOICE *choice,
    A_CHOICE *a_choice);

CHOICES_LIST convert_to_choices_list(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    const UNICHARSET &current_unicharset);

#endif  // TESSERACT_DICT_CONVERSION_H_
