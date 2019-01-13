/**********************************************************************
 * File:        werdit.cpp  (Formerly wordit.c)
 * Description: An iterator for passing over all the words in a document.
 * Author:      Ray Smith
 * Created:     Mon Apr 27 08:51:22 BST 1992
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

#include "werdit.h"
#include "errcode.h"   // for ASSERT_HOST
#include "pageres.h"   // for PAGE_RES_IT, PAGE_RES (ptr only), WERD_RES
#include "stepblob.h"  // for C_BLOB_IT, C_BLOB, C_BLOB_LIST
#include "werd.h"      // for WERD

/**********************************************************************
 * make_pseudo_word
 *
 * Make all the blobs inside a selection into a single word.
 * The returned PAGE_RES_IT* it points to the new word. After use, call
 * it->DeleteCurrentWord() to delete the fake word, and then
 * delete it to get rid of the iterator itself.
 **********************************************************************/

PAGE_RES_IT* make_pseudo_word(PAGE_RES* page_res, const TBOX& selection_box) {
  PAGE_RES_IT pr_it(page_res);
  C_BLOB_LIST new_blobs;               // list of gathered blobs
  C_BLOB_IT new_blob_it = &new_blobs;  // iterator

  for (WERD_RES* word_res = pr_it.word(); word_res != nullptr;
       word_res = pr_it.forward()) {
    WERD* word = word_res->word;
    if (word->bounding_box().overlap(selection_box)) {
      C_BLOB_IT blob_it(word->cblob_list());
      for (blob_it.mark_cycle_pt();
           !blob_it.cycled_list(); blob_it.forward()) {
        C_BLOB* blob = blob_it.data();
        if (blob->bounding_box().overlap(selection_box)) {
          new_blob_it.add_after_then_move(C_BLOB::deep_copy(blob));
        }
      }
      if (!new_blobs.empty()) {
        WERD* pseudo_word = new WERD(&new_blobs, 1, nullptr);
        word_res = pr_it.InsertSimpleCloneWord(*word_res, pseudo_word);
        PAGE_RES_IT* it = new PAGE_RES_IT(page_res);
        while (it->word() != word_res && it->word() != nullptr) it->forward();
        ASSERT_HOST(it->word() == word_res);
        return it;
      }
    }
  }
  return nullptr;
}
