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
 * tess_segment_pass1
 *
 * Segment a word using the pass1 conditions of the tess segmenter.
 * @param word bln word to do
 * @param denorm de-normaliser
 * @param matcher matcher function
 * @param raw_choice raw result
 * @param blob_choices list of blob lists
 * @param outword bln word output
 */

namespace tesseract {
WERD_CHOICE *Tesseract::tess_segment_pass1(WERD *word,
                                           DENORM *denorm,
                                           POLY_MATCHER matcher,
                                           WERD_CHOICE *&raw_choice,
                                           BLOB_CHOICE_LIST_CLIST *blob_choices,
                                           WERD *&outword) {
  WERD_CHOICE *result;           //return value
  int saved_enable_assoc = 0;
  int saved_chop_enable = 0;

  if (word->flag (W_DONT_CHOP)) {
    saved_enable_assoc = wordrec_enable_assoc;
    saved_chop_enable = chop_enable;
    wordrec_enable_assoc.set_value(0);
    chop_enable.set_value(0);
    if (word->flag (W_REP_CHAR))
      permute_only_top = 1;
  }
  set_pass1();
  //      tprintf("pass1 chop on=%d, seg=%d, onlytop=%d",chop_enable,enable_assoc,permute_only_top);
  result = recog_word (word, denorm, matcher, NULL, NULL, FALSE,
    raw_choice, blob_choices, outword);
  if (word->flag (W_DONT_CHOP)) {
    wordrec_enable_assoc.set_value(saved_enable_assoc);
    chop_enable.set_value(saved_chop_enable);
    permute_only_top = 0;
  }
  return result;
}


/**
 * tess_segment_pass2
 *
 * Segment a word using the pass2 conditions of the tess segmenter.
 * @param word bln word to do
 * @param denorm de-normaliser
 * @param matcher matcher function
 * @param raw_choice raw result
 * @param blob_choices list of blob lists
 * @param outword bln word output
 */

WERD_CHOICE *Tesseract::tess_segment_pass2(WERD *word,
                                           DENORM *denorm,
                                           POLY_MATCHER matcher,
                                           WERD_CHOICE *&raw_choice,
                                           BLOB_CHOICE_LIST_CLIST *blob_choices,
                                           WERD *&outword) {
  WERD_CHOICE *result;           //return value
  int saved_enable_assoc = 0;
  int saved_chop_enable = 0;

  if (word->flag (W_DONT_CHOP)) {
    saved_enable_assoc = wordrec_enable_assoc;
    saved_chop_enable = chop_enable;
    wordrec_enable_assoc.set_value(0);
    chop_enable.set_value(0);
    if (word->flag (W_REP_CHAR))
      permute_only_top = 1;
  }
  set_pass2();
  result = recog_word (word, denorm, matcher, NULL, NULL, FALSE,
    raw_choice, blob_choices, outword);
  if (word->flag (W_DONT_CHOP)) {
    wordrec_enable_assoc.set_value(saved_enable_assoc);
    chop_enable.set_value(saved_chop_enable);
    permute_only_top = 0;
  }
  return result;
}


/**
 * correct_segment_pass2
 *
 * Segment a word correctly using the pass2 conditions of the tess segmenter.
 * Then call the tester with all the correctly segmented blobs.
 * If the correct segmentation cannot be found, the tester is called
 * with the segmentation found by tess and all the correct flags set to
 * false and all strings are NULL.
 * @param word bln word to do
 * @param denorm de-normaliser
 * @param matcher matcher function
 * @param tester tester function
 * @param raw_choice raw result
 * @param blob_choices list of blob lists
 * @param outword bln word output
 */

WERD_CHOICE *Tesseract::correct_segment_pass2(WERD *word,
                                              DENORM *denorm,
                                              POLY_MATCHER matcher,
                                              POLY_TESTER tester,
                                              WERD_CHOICE *&raw_choice,
                                              BLOB_CHOICE_LIST_CLIST *blob_choices,
                                              WERD *&outword) {
  set_pass2();
  return recog_word (word, denorm, matcher, NULL, tester, TRUE,
    raw_choice, blob_choices, outword);
}


/**
 * test_segment_pass2
 *
 * Segment a word correctly using the pass2 conditions of the tess segmenter.
 * Then call the tester on all words used by tess in its search.
 * Do this only on words where the correct segmentation could be found.
 * @param word bln word to do
 * @param denorm de-normaliser
 * @param matcher matcher function
 * @param tester tester function
 * @param raw_choice raw result
 * @param blob_choices list of blob lists
 * @param outword bln word output
 */
WERD_CHOICE *Tesseract::test_segment_pass2(WERD *word,
                                           DENORM *denorm,
                                           POLY_MATCHER matcher,
                                           POLY_TESTER tester,
                                           WERD_CHOICE *&raw_choice,
                                           BLOB_CHOICE_LIST_CLIST *blob_choices,
                                           WERD *&outword) {
  set_pass2();
  return recog_word (word, denorm, matcher, tester, NULL, TRUE,
    raw_choice, blob_choices, outword);
}


/**
 * tess_acceptable_word
 *
 * Return true if the word is regarded as "good enough".
 */
BOOL8 Tesseract::tess_acceptable_word(
    WERD_CHOICE *word_choice,  // after context
    WERD_CHOICE *raw_choice) {  // before context
  return getDict().AcceptableResult(*word_choice, *raw_choice);
}


/**
 * tess_adaptable_word
 *
 * Return true if the word is regarded as "good enough".
 */
BOOL8 Tesseract::tess_adaptable_word(  //< test adaptability
    WERD *word,                        //< word to test
    WERD_CHOICE *best_choice,          //< after context
    WERD_CHOICE *raw_choice            //< before context
                                     ) {
  TWERD *tessword = make_tess_word(word, NULL);
  int result = (tessword && best_choice && raw_choice &&
                AdaptableWord(tessword, *best_choice, *raw_choice));
  delete_word(tessword);
  return result != 0;
}


/**
 * tess_cn_matcher
 *
 * Match a blob using the Tess Char Normalized (non-adaptive) matcher
 * only.
 */

void Tesseract::tess_cn_matcher(                           //call tess
                                PBLOB *pblob,              //previous blob
                                PBLOB *blob,               //blob to match
                                PBLOB *nblob,              //next blob
                                WERD *word,                //word it came from
                                DENORM *denorm,            //de-normaliser
                                BLOB_CHOICE_LIST *ratings,  //list of results
                                CLASS_PRUNER_RESULTS cpresults  // may be null.
                    ) {
  TBLOB *tessblob;               //converted blob
  TEXTROW tessrow;               //dummy row

  tess_cn_matching.set_value(true);       //turn it on
  tess_bn_matching.set_value(false);
                                 //convert blob
  tessblob = make_rotated_tess_blob(denorm, blob, true);
                                 //make dummy row
  make_tess_row(denorm, &tessrow);
                                 //classify
  AdaptiveClassifier(tessblob, NULL, &tessrow, ratings, cpresults);
  free_blob(tessblob);
}


/**
 * tess_bn_matcher
 *
 * Match a blob using the Tess Baseline Normalized (adaptive) matcher
 * only.
 */

void Tesseract::tess_bn_matcher(                           //call tess
                                PBLOB *pblob,              //previous blob
                                PBLOB *blob,               //blob to match
                                PBLOB *nblob,              //next blob
                                WERD *word,                //word it came from
                                DENORM *denorm,            //de-normaliser
                                BLOB_CHOICE_LIST *ratings  //list of results
                               ) {
  TBLOB *tessblob;               //converted blob
  TEXTROW tessrow;               //dummy row

  tess_bn_matching.set_value(true);       //turn it on
  tess_cn_matching.set_value(false);
                                 //convert blob
  tessblob = make_rotated_tess_blob(denorm, blob, true);
                                 //make dummy row
  make_tess_row(denorm, &tessrow);
                                 //classify
  AdaptiveClassifier(tessblob, NULL, &tessrow, ratings, NULL);
  free_blob(tessblob);
}


/**
 * tess_default_matcher
 *
 * Match a blob using the default functionality of the Tess matcher.
 */

void Tesseract::tess_default_matcher(                       //call tess
                                     PBLOB *pblob,          //previous blob
                                     PBLOB *blob,           //blob to match
                                     PBLOB *nblob,          //next blob
                                     WERD *word,            //word it came from
                                     DENORM *denorm,        //de-normaliser
                                                            //list of results
                                     BLOB_CHOICE_LIST *ratings,
                                     const char* script
                                    ) {
  assert(ratings != NULL);
  TBLOB *tessblob;               //converted blob
  TEXTROW tessrow;               //dummy row

  tess_bn_matching.set_value(false);      //turn it off
  tess_cn_matching.set_value(false);
                                 //convert blob
  tessblob = make_rotated_tess_blob(denorm, blob, true);
                                 //make dummy row
  make_tess_row(denorm, &tessrow);
                                 //classify
  AdaptiveClassifier (tessblob, NULL, &tessrow, ratings, NULL);
  free_blob(tessblob);
}
}  // namespace tesseract


/**
 * tess_training_tester
 *
 * Matcher tester function which actually trains tess.
 */

void tess_training_tester(                           //call tess
                          const STRING& filename,    //filename to output
                          PBLOB *blob,               //blob to match
                          DENORM *denorm,            //de-normaliser
                          BOOL8 correct,             //ly segmented
                          char *text,                //correct text
                          inT32 count,               //chars in text
                          BLOB_CHOICE_LIST *ratings  //list of results
                         ) {
  TBLOB *tessblob;               //converted blob
  TEXTROW tessrow;               //dummy row

  if (correct) {
    classify_norm_method.set_value(character); // force char norm spc 30/11/93
    tess_bn_matching.set_value(false);    //turn it off
    tess_cn_matching.set_value(false);
                                 //convert blob
    tessblob = make_tess_blob (blob, TRUE);
                                 //make dummy row
    make_tess_row(denorm, &tessrow);
                                 //learn it
    LearnBlob(filename, tessblob, &tessrow, text);
    free_blob(tessblob);
  }
}


/**
 * tess_adapter
 *
 * Adapt to the word using the Tesseract mechanism.
 */

namespace tesseract {
void Tesseract::tess_adapter(                         //adapt to word
                             WERD *word,              //bln word
                             DENORM *denorm,          //de-normalise
                             const WERD_CHOICE& choice,      //string for word
                             const WERD_CHOICE& raw_choice,  //before context
                             const char *rejmap       //reject map
                            ) {
  TWERD *tessword;               //converted word
  static TEXTROW tessrow;        //dummy row

                                 //make dummy row
  make_tess_row(denorm, &tessrow);
                                 //make a word
  tessword = make_tess_word (word, &tessrow);
  AdaptToWord(tessword, &tessrow, choice, raw_choice, rejmap);
  //adapt to it
  delete_word(tessword);  //free it
}


/**
 * tess_add_doc_word
 *
 * Add the given word to the document dictionary
 */
void Tesseract::tess_add_doc_word(WERD_CHOICE *word_choice) {
  getDict().add_document_word(*word_choice);
}
}  // namespace tesseract
