/**********************************************************************
 * File:			wordit.c
 * Description: An iterator for passing over all the words in a document.
 * Author:		Ray Smith
 * Created:		Mon Apr 27 08:51:22 BST 1992
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

#ifndef           WERDIT_H
#define           WERDIT_H

#include          "pageres.h"

PAGE_RES_IT* make_pseudo_word(PAGE_RES* page_res, const TBOX& selection_box);

#endif
