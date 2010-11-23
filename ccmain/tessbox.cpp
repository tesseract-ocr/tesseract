/**********************************************************************
 * File:        tessbox.cpp  (Formerly tessbox.c)
 * Description: Black boxed Tess for developing a resaljet.
 * Author:					Ray Smith
 * Created:					Thu Apr 23 11:03:36 BST 1992
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "mfcpch.h"
#include          "tfacep.h"
#include          "tfacepp.h"
#include          "tessbox.h"
#include "mfoutline.h"
#include "tesseractclass.h"

#define EXTERN

/**
 * @name tess_segment_pass1
 *
 * Segment a word using the pass1 conditions of the tess segmenter.
 * @param word word to do
 * @param blob_choices list of blob lists
 */

namespace tesseract {
void Tesseract::tess_segment_pass1(WERD_RES *word,
                                   BLOB_CHOICE_LIST_CLIST *blob_choices) {
  int saved_enable_assoc = 0;
  int saved_chop_enable = 0;

  if (word->word->flag(W_DONT_CHOP)) {
    saved_enable_assoc = wordrec_enable_assoc;
    saved_chop_enable = chop_enable;
    wordrec_enable_assoc.set_value(0);
    chop_enable.set_value(0);
    if (word->word->flag(W_REP_CHAR))
      getDict().permute_only_top.set_value(true);
  }
  set_pass1();
  recog_word(word, blob_choices);
  if (word->word->flag(W_DONT_CHOP)) {
    wordrec_enable_assoc.set_value(saved_enable_assoc);
    chop_enable.set_value(saved_chop_enable);
    getDict().permute_only_top.set_value(false);
  }
}


/**
 * @name tess_segment_pass2
 *
 * Segment a word using the pass2 conditions of the tess segmenter.
 * @param word word to do
 * @param blob_choices list of blob lists
 */

void Tesseract::tess_segment_pass2(WERD_RES *word,
                                   BLOB_CHOICE_LIST_CLIST *blob_choices) {
  int saved_enable_assoc = 0;
  int saved_chop_enable = 0;

  if (word->word->flag(W_DONT_CHOP)) {
    saved_enable_assoc = wordrec_enable_assoc;
    saved_chop_enable = chop_enable;
    wordrec_enable_assoc.set_value(0);
    chop_enable.set_value(0);
    if (word->word->flag(W_REP_CHAR))
      getDict().permute_only_top.set_value(true);
  }
  set_pass2();
  recog_word(word, blob_choices);
  if (word->word->flag(W_DONT_CHOP)) {
    wordrec_enable_assoc.set_value(saved_enable_assoc);
    chop_enable.set_value(saved_chop_enable);
    getDict().permute_only_top.set_value(false);
  }
}

/**
 * @name tess_acceptable_word
 *
 * @return true if the word is regarded as "good enough".
 * @param word_choice after context
 * @param raw_choice before context
 */
BOOL8 Tesseract::tess_acceptable_word(
    WERD_CHOICE *word_choice,  // after context
    WERD_CHOICE *raw_choice) {  // before context
  return getDict().AcceptableResult(*word_choice);
}


/**
 * @name tess_add_doc_word
 *
 * Add the given word to the document dictionary
 */
void Tesseract::tess_add_doc_word(WERD_CHOICE *word_choice) {
  getDict().add_document_word(*word_choice);
}
}  // namespace tesseract
