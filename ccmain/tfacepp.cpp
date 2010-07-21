/**********************************************************************
 * File:        tfacepp.cpp  (Formerly tface++.c)
 * Description: C++ side of the C/C++ Tess/Editor interface.
 * Author:                  Ray Smith
 * Created:                 Thu Apr 23 15:39:23 BST 1992
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
#pragma warning(disable:4305)  // int/float warnings
#pragma warning(disable:4800)  // int/bool warnings
#endif

#include "mfcpch.h"
#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "errcode.h"
#include          "ratngs.h"
#include          "reject.h"
#include          "werd.h"
#include          "tfacep.h"
#include          "tstruct.h"
#include          "tfacepp.h"
#include          "tessvars.h"
#include          "globals.h"
#include          "reject.h"
#include          "tesseractclass.h"

#define EXTERN

EXTERN BOOL_VAR (tessedit_override_permuter, TRUE, "According to dict_word");


#define MAX_UNDIVIDED_LENGTH 24



/**********************************************************************
 * recog_word
 *
 * Convert the word to tess form and pass it to the tess segmenter.
 * Convert the output back to editor form.
 **********************************************************************/
namespace tesseract {
WERD_CHOICE *Tesseract::recog_word(                     //recog one owrd
                                   WERD *word,          //word to do
                                   DENORM *denorm,      //de-normaliser
                                                        //matcher function
                                   POLY_MATCHER matcher,
                                   POLY_TESTER tester,  //tester function
                                   POLY_TESTER trainer, //trainer function
                                   BOOL8 testing,       //true if answer driven
                                                        //raw result
                                   WERD_CHOICE *&raw_choice,
                                                        //list of blob lists
                                   BLOB_CHOICE_LIST_CLIST *blob_choices,
                                   WERD *&outword       //bln word output
                                  ) {
  WERD_CHOICE *word_choice;
  uinT8 perm_type;
  uinT8 real_dict_perm_type;

  if (word->blob_list ()->empty ()) {
    word_choice = new WERD_CHOICE("", NULL, 10.0f, -1.0f,
                                  TOP_CHOICE_PERM, unicharset);
    raw_choice = new WERD_CHOICE("", NULL, 10.0f, -1.0f,
                                 TOP_CHOICE_PERM, unicharset);
    outword = word->poly_copy (denorm->row ()->x_height ());
  }
  else
    word_choice = recog_word_recursive (word, denorm, matcher, tester,
      trainer, testing, raw_choice,
      blob_choices, outword);
  if ((word_choice->length() != outword->blob_list()->length()) ||
      (word_choice->length() != blob_choices->length())) {
    tprintf
      ("recog_word ASSERT FAIL String:\"%s\"; Strlen=%d; #Blobs=%d; #Choices=%d\n",
      word_choice->debug_string(unicharset).string(),
      word_choice->length(), outword->blob_list()->length(),
      blob_choices->length());
  }
  ASSERT_HOST(word_choice->length() == outword->blob_list()->length());
  ASSERT_HOST(word_choice->length() == blob_choices->length());

  /* Copy any reject blobs into the outword */
  outword->rej_blob_list()->deep_copy(word->rej_blob_list(), &PBLOB::deep_copy);

  if (tessedit_override_permuter) {
    /* Override the permuter type if a straight dictionary check disagrees. */
    perm_type = word_choice->permuter();
    if ((perm_type != SYSTEM_DAWG_PERM) &&
        (perm_type != FREQ_DAWG_PERM) && (perm_type != USER_DAWG_PERM)) {
      real_dict_perm_type = dict_word(*word_choice);
      if (((real_dict_perm_type == SYSTEM_DAWG_PERM) ||
           (real_dict_perm_type == FREQ_DAWG_PERM) ||
           (real_dict_perm_type == USER_DAWG_PERM)) &&
          (alpha_count(word_choice->unichar_string().string(),
                      word_choice->unichar_lengths().string()) > 0)) {
        word_choice->set_permuter (real_dict_perm_type);  // use dict perm
      }
    }
    if (tessedit_rejection_debug && perm_type != word_choice->permuter ()) {
      tprintf ("Permuter Type Flipped from %d to %d\n",
        perm_type, word_choice->permuter ());
    }
  }
  assert ((word_choice == NULL) == (raw_choice == NULL));
  return word_choice;
}


/**********************************************************************
 * recog_word_recursive
 *
 * Convert the word to tess form and pass it to the tess segmenter.
 * Convert the output back to editor form.
 **********************************************************************/
WERD_CHOICE *
Tesseract::recog_word_recursive(
    WERD *word,                            // word to do
    DENORM *denorm,                        // de-normaliser
    POLY_MATCHER matcher,                  // matcher function
    POLY_TESTER tester,                    // tester function
    POLY_TESTER trainer,                   // trainer function
    BOOL8 testing,                         // true if answer driven
    WERD_CHOICE *&raw_choice,              // raw result
    BLOB_CHOICE_LIST_CLIST *blob_choices,  // list of blob lists
    WERD *&outword                         // bln word output
    ) {
  inT32 initial_blob_choice_len;
  inT32 word_length;                      // no of blobs
  STRING word_string;                     // converted from tess
  STRING word_string_lengths;
  BLOB_CHOICE_LIST_VECTOR *tess_ratings;  // tess results
  TWERD *tessword;                        // tess format
  BLOB_CHOICE_LIST_C_IT blob_choices_it;  // iterator

  tess_matcher = matcher;           // install matcher
  tess_tester = testing ? tester : NULL;
  tess_trainer = testing ? trainer : NULL;
  tess_denorm = denorm;
  tess_word = word;
  //      blob_matchers[1]=call_matcher;
  if (word->blob_list ()->length () > MAX_UNDIVIDED_LENGTH) {
    return split_and_recog_word (word, denorm, matcher, tester, trainer,
      testing, raw_choice, blob_choices,
      outword);
  } else {
    UNICHAR_ID space_id = unicharset.unichar_to_id(" ");
    WERD_CHOICE *best_choice = new WERD_CHOICE();
    raw_choice = new WERD_CHOICE();
    initial_blob_choice_len = blob_choices->length();
    tessword = make_tess_word (word, NULL);
    tess_ratings = cc_recog(tessword, best_choice, raw_choice,
                            testing && tester != NULL,
                            testing && trainer != NULL,
                            word->flag(W_EOL));

    outword = make_ed_word (tessword, word);  // convert word
    if (outword == NULL) {
      outword = word->poly_copy (denorm->row ()->x_height ());
    }
    delete_word(tessword);  // get rid of it
    word_length = outword->blob_list()->length();  // no of blobs

    // Put BLOB_CHOICE_LISTs from tess_ratings into blob_choices.
    blob_choices_it.set_to_list(blob_choices);
    for (int i = 0; i < tess_ratings->length(); ++i) {
      blob_choices_it.add_to_end(tess_ratings->get(i));
    }
    delete tess_ratings;

    // Pad raw_choice with spaces if needed.
    if (raw_choice->length() < word_length) {
      while (raw_choice->length() < word_length) {
        raw_choice->append_unichar_id(space_id, 1, 0.0,
                                      raw_choice->certainty());
      }
      raw_choice->populate_unichars(unicharset);
    }

    // Do sanity checks and minor fixes on best_choice.
    if (best_choice->length() > word_length) {
      tprintf("recog_word: Discarded long string \"%s\""
              " (%d characters vs %d blobs)\n",
              best_choice->unichar_string().string (),
              best_choice->length(), word_length);
      best_choice->make_bad();  // should never happen
      tprintf("Word is at (%g,%g)\n",
              denorm->origin(),
              denorm->y(word->bounding_box().bottom(), 0.0));
    }
    if (blob_choices->length() - initial_blob_choice_len != word_length) {
      best_choice->make_bad();  // force rejection
      tprintf ("recog_word: Choices list len:%d; blob lists len:%d\n",
        blob_choices->length(), word_length);
      blob_choices_it.set_to_list(blob_choices);  // list of lists
      while (blob_choices->length() - initial_blob_choice_len < word_length) {
        blob_choices_it.add_to_end(new BLOB_CHOICE_LIST());  // add a fake one
        tprintf("recog_word: Added dummy choice list\n");
      }
      while (blob_choices->length() - initial_blob_choice_len > word_length) {
        blob_choices_it.move_to_last(); // should never happen
        delete blob_choices_it.extract();
        tprintf("recog_word: Deleted choice list\n");
      }
    }
    if (best_choice->length() < word_length) {
      while (best_choice->length() < word_length) {
        best_choice->append_unichar_id(space_id, 1, 0.0,
                                       best_choice->certainty());
      }
      best_choice->populate_unichars(unicharset);
    }

    return best_choice;
  }
}


/**********************************************************************
 * split_and_recog_word
 *
 * Convert the word to tess form and pass it to the tess segmenter.
 * Convert the output back to editor form.
 **********************************************************************/

WERD_CHOICE *
Tesseract::split_and_recog_word(                        //recog one owrd
                                WERD *word,             //word to do
                                DENORM *denorm,         //de-normaliser
                                POLY_MATCHER matcher,   //matcher function
                                POLY_TESTER tester,     //tester function
                                POLY_TESTER trainer,    //trainer function
                                BOOL8 testing,          //true if answer driven
                                                        //raw result
                                WERD_CHOICE *&raw_choice,
                                                        //list of blob lists
                                BLOB_CHOICE_LIST_CLIST *blob_choices,
                                WERD *&outword          //bln word output
                               ) {
  //   inT32                                                      outword1_len;
  //   inT32                                                      outword2_len;
  WERD *first_word;              //poly copy of word
  WERD *second_word;             //fabricated word
  WERD *outword2;                //2nd output word
  PBLOB *blob;
  WERD_CHOICE *result;           //return value
  WERD_CHOICE *result2;          //output of 2nd word
  WERD_CHOICE *raw_choice2;      //raw version of 2nd
  float gap;                     //blob gap
  float bestgap;                 //biggest gap
  PBLOB_LIST new_blobs;          //list of gathered blobs
  PBLOB_IT blob_it;
                                 //iterator
  PBLOB_IT new_blob_it = &new_blobs;

  first_word = word->poly_copy (denorm->row ()->x_height ());
  blob_it.set_to_list (first_word->blob_list ());
  bestgap = (float) -MAX_INT32;
  while (!blob_it.at_last ()) {
    blob = blob_it.data ();
                                 //gap to next
    gap = (float) blob_it.data_relative(1)->bounding_box().left() -
        blob->bounding_box().right();
    blob_it.forward ();
    if (gap > bestgap) {
      bestgap = gap;             //find biggest
      new_blob_it = blob_it;     //save position
    }
  }
                                 //take 2nd half
  new_blobs.assign_to_sublist (&new_blob_it, &blob_it);
                                 //make it a word
  second_word = new WERD (&new_blobs, 1, NULL);
  ASSERT_HOST (word->blob_list ()->length () ==
    first_word->blob_list ()->length () +
    second_word->blob_list ()->length ());

  result = recog_word_recursive (first_word, denorm, matcher,
    tester, trainer, testing, raw_choice,
    blob_choices, outword);
  delete first_word;             //done that one
  result2 = recog_word_recursive (second_word, denorm, matcher,
    tester, trainer, testing, raw_choice2,
    blob_choices, outword2);
  delete second_word;            //done that too
  *result += *result2;           //combine ratings
  delete result2;
  *raw_choice += *raw_choice2;
  delete raw_choice2;            //finished with it
  //   outword1_len= outword->blob_list()->length();
  //   outword2_len= outword2->blob_list()->length();
  outword->join_on (outword2);   //join words
  delete outword2;
  //   if ( outword->blob_list()->length() != outword1_len + outword2_len )
  //      tprintf( "Split&Recog: part1len=%d; part2len=%d; combinedlen=%d\n",
  //                                outword1_len, outword2_len, outword->blob_list()->length() );
  //   ASSERT_HOST( outword->blob_list()->length() == outword1_len + outword2_len );
  return result;
}

}  // namespace tesseract

/**********************************************************************
 * call_tester
 *
 * Called from Tess with a blob in tess form.
 * Convert the blob to editor form.
 * Call the tester setup by the segmenter in tess_tester.
 **********************************************************************/
#if 0  // dead code
void call_tester(                     //call a tester
                 const STRING& filename,
                 TBLOB *tessblob,     //blob to test
                 BOOL8 correct_blob,  //true if good
                 char *text,          //source text
                 inT32 count,         //chars in text
                 LIST result          //output of matcher
                ) {
  PBLOB *blob;                   //converted blob
  BLOB_CHOICE_LIST ratings;      //matcher result

  blob = make_ed_blob (tessblob);//convert blob
  if (blob == NULL)
    return;
                                 //make it right type
  convert_choice_list(result, ratings);
  if (tess_tester != NULL)
    (*tess_tester) (filename, blob, tess_denorm, correct_blob, text, count, &ratings);
  delete blob;                   //don't need that now
}
#endif

/**********************************************************************
 * call_train_tester
 *
 * Called from Tess with a blob in tess form.
 * Convert the blob to editor form.
 * Call the trainer setup by the segmenter in tess_trainer.
 **********************************************************************/
#if 0  // dead code
void call_train_tester(                     //call a tester
                       const STRING& filename,
                       TBLOB *tessblob,     //blob to test
                       BOOL8 correct_blob,  //true if good
                       char *text,          //source text
                       inT32 count,         //chars in text
                       LIST result          //output of matcher
                      ) {
  PBLOB *blob;                   //converted blob
  BLOB_CHOICE_LIST ratings;      //matcher result

  blob = make_ed_blob (tessblob);//convert blob
  if (blob == NULL)
    return;
                                 //make it right type
  convert_choice_list(result, ratings);
  if (tess_trainer != NULL)
    (*tess_trainer) (filename, blob, tess_denorm, correct_blob, text, count, &ratings);
  delete blob;                   //don't need that now
}
#endif
