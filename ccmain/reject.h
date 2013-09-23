/**********************************************************************
 * File:        reject.h  (Formerly reject.h)
 * Description: Rejection functions used in tessedit
 * Author:		Phil Cheatle
 * Created:		Wed Sep 23 16:50:21 BST 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifndef           REJECT_H
#define           REJECT_H

#include          "params.h"
#include          "pageres.h"

void reject_blanks(WERD_RES *word);
void reject_poor_matches(WERD_RES *word);
float compute_reject_threshold(WERD_CHOICE* word);
BOOL8 word_contains_non_1_digit(const char *word, const char *word_lengths);
void dont_allow_1Il(WERD_RES *word);
void flip_hyphens(WERD_RES *word);
void flip_0O(WERD_RES *word);
BOOL8 non_0_digit(const char* str, int length);
#endif
