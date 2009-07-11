///////////////////////////////////////////////////////////////////////
// File:        permngram.h
// Description: Character n-gram permuter
// Author:      Thomas Kielbus
// Created:     Wed Sep 12 11:26:42 PDT 2007
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

#ifndef PERMNGRAM_H
#define PERMNGRAM_H

#include "choicearr.h"
#include "dawg.h"

// Permute the given char_choices using a character level n-gram model and
// return the best word choice found. The given dawg is used to determine
// which choices are contained in the dictionary.
/*A_CHOICE *ngram_permute_and_select(CHOICES_LIST char_choices,
                                   float rating_limit,
                                   const Dawg *dawg);*/

#endif  // PERMNGRAM_H
