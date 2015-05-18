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

#include "mfoutline.h"
#include "tessbox.h"
#include "tesseractclass.h"

#define EXTERN

/**
 * @name tess_segment_pass_n
 *
 * Segment a word using the pass_n conditions of the tess segmenter.
 * @param pass_n pass number
 * @param word word to do
 */

namespace tesseract {
void Tesseract::tess_segment_pass_n(int pass_n, WERD_RES *word) {
  int saved_enable_assoc = 0;
  int saved_chop_enable = 0;

  if (word->word->flag(W_DONT_CHOP)) {
    saved_enable_assoc = wordrec_enable_assoc;
    saved_chop_enable = chop_enable;
    wordrec_enable_assoc.set_value(0);
    chop_enable.set_value(0);
  }
  if (pass_n == 1)
    set_pass1();
  else
    set_pass2();
  recog_word(word);
  if (word->best_choice == NULL)
    word->SetupFake(*word->uch_set);
  if (word->word->flag(W_DONT_CHOP)) {
    wordrec_enable_assoc.set_value(saved_enable_assoc);
    chop_enable.set_value(saved_chop_enable);
  }
}

/**
 * @name tess_acceptable_word
 *
 * @return true if the word is regarded as "good enough".
 * @param word_choice after context
 * @param raw_choice before context
 */
bool Tesseract::tess_acceptable_word(WERD_RES* word) {
  return getDict().AcceptableResult(word);
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
