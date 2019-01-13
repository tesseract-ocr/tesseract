/**********************************************************************
 * File:        pagewalk.cpp  (Formerly walkers.c)
 * Description: Block list processors
 * Author:      Phil Cheatle
 * Created:     Thu Oct 10 16:25:24 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#include "pageres.h"
#include "tesseractclass.h"

namespace tesseract {
/**
 * @name process_selected_words()
 *
 * Walk the current block list applying the specified word processor function
 * to each word that overlaps the selection_box.
 */
void Tesseract::process_selected_words(
        PAGE_RES* page_res, // blocks to check
        TBOX& selection_box,
        bool (tesseract::Tesseract::* word_processor)(PAGE_RES_IT* pr_it)) {
  for (PAGE_RES_IT page_res_it(page_res); page_res_it.word() != nullptr;
       page_res_it.forward()) {
    WERD* word = page_res_it.word()->word;
    if (word->bounding_box().overlap(selection_box)) {
      if (!(this->*word_processor)(&page_res_it))
        return;
    }
  }
}
}  // namespace tesseract
