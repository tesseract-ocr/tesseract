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

#include "mfcpch.h"
#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "errcode.h"
#include          "tessarray.h"
//#include                                                      "fxtop.h"
#include          "werd.h"
#include          "tfacep.h"
#include          "tstruct.h"
#include          "tfacepp.h"
#include          "tessvars.h"
#include          "globals.h"
#include          "reject.h"

#define EXTERN

EXTERN BOOL_VAR (tessedit_override_permuter, TRUE, "According to dict_word");

static POLY_MATCHER tess_matcher;//current matcher
static POLY_TESTER tess_tester;  //current tester
static POLY_TESTER tess_trainer; //current trainer
static DENORM *tess_denorm;      //current denorm
static WERD *tess_word;          //current word

#define MAX_UNDIVIDED_LENGTH 24

const int kReallyBadCertainty = -20;

/**********************************************************************
 * recog_word
 *
 * Convert the word to tess form and pass it to the tess segmenter.
 * Convert the output back to editor form.
 **********************************************************************/
WERD_CHOICE *recog_word(                           //recog one owrd
                        WERD *word,                //word to do
                        DENORM *denorm,            //de-normaliser
                        POLY_MATCHER matcher,      //matcher function
                        POLY_TESTER tester,        //tester function
                        POLY_TESTER trainer,       //trainer function
                        BOOL8 testing,             //true if answer driven
                        WERD_CHOICE *&raw_choice,  //raw result //list of blob lists
                        BLOB_CHOICE_LIST_CLIST *blob_choices,
                        WERD *&outword             //bln word output
                       ) {
  WERD_CHOICE *word_choice;
  uinT8 perm_type;
  uinT8 real_dict_perm_type;

  if (word->blob_list ()->empty ()) {
    char empty_lengths[] = {0};
    word_choice = new WERD_CHOICE ("", empty_lengths,
                                   10.0f, -1.0f, TOP_CHOICE_PERM);
    raw_choice = new WERD_CHOICE ("", empty_lengths,
                                  10.0f, -1.0f, TOP_CHOICE_PERM);
    outword = word->poly_copy (denorm->row ()->x_height ());
  }
  else
    word_choice = recog_word_recursive (word, denorm, matcher, tester,
      trainer, testing, raw_choice,
      blob_choices, outword);
  if ((word_choice->lengths ().length () !=
    outword->blob_list ()->length ()) ||
  (word_choice->lengths ().length () != blob_choices->length ())) {
    tprintf
      ("recog_word ASSERT FAIL String:\"%s\"; Strlen=%d; #Blobs=%d; #Choices=%d\n",
      word_choice->string ().string (), word_choice->lengths ().length (),
      outword->blob_list ()->length (), blob_choices->length ());
  }
  ASSERT_HOST (word_choice->lengths ().length () ==
    outword->blob_list ()->length ());
  ASSERT_HOST (word_choice->lengths ().length () == blob_choices->length ());

  /* Copy any reject blobs into the outword */
  outword->rej_blob_list()->deep_copy(word->rej_blob_list(), &PBLOB::deep_copy);

  if (tessedit_override_permuter) {
    /* Override the permuter type if a straight dictionary check disagrees. */
    perm_type = word_choice->permuter ();
    if ((perm_type != SYSTEM_DAWG_PERM) &&
    (perm_type != FREQ_DAWG_PERM) && (perm_type != USER_DAWG_PERM)) {
      real_dict_perm_type = dict_word (word_choice->string ().string ());
      if (((real_dict_perm_type == SYSTEM_DAWG_PERM) ||
        (real_dict_perm_type == FREQ_DAWG_PERM) ||
        (real_dict_perm_type == USER_DAWG_PERM)) &&
        (alpha_count (word_choice->string ().string (),
                      word_choice->lengths ().string ()) > 0))
        word_choice->set_permuter (real_dict_perm_type);
      //Use dict perm
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

WERD_CHOICE *recog_word_recursive(                           //recog one owrd
                                  WERD *word,                //word to do
                                  DENORM *denorm,            //de-normaliser
                                  POLY_MATCHER matcher,      //matcher function
                                  POLY_TESTER tester,        //tester function
                                  POLY_TESTER trainer,       //trainer function
                                  BOOL8 testing,             //true if answer driven
                                  WERD_CHOICE *&raw_choice,  //raw result //list of blob lists
                                  BLOB_CHOICE_LIST_CLIST *blob_choices,
                                  WERD *&outword             //bln word output
                                 ) {
  inT32 initial_blob_choice_len;
  inT32 word_length;             //no of blobs
  STRING word_string;            //converted from tess
  STRING word_string_lengths;
  ARRAY tess_ratings;            //tess results
  A_CHOICE tess_choice;          //best word
  A_CHOICE tess_raw;             //raw result
  TWERD *tessword;               //tess format
  BLOB_CHOICE_LIST *choice_list; //fake list
                                 //iterator
  BLOB_CHOICE_LIST_C_IT choice_it;

  tess_matcher = matcher;        //install matcher
  tess_tester = testing ? tester : NULL;
  tess_trainer = testing ? trainer : NULL;
  tess_denorm = denorm;
  tess_word = word;
  //      blob_matchers[1]=call_matcher;
  if (word->blob_list ()->length () > MAX_UNDIVIDED_LENGTH) {
    return split_and_recog_word (word, denorm, matcher, tester, trainer,
      testing, raw_choice, blob_choices,
      outword);
  }
  else {
    if (word->flag (W_EOL))
      last_word_on_line = TRUE;
    else
      last_word_on_line = FALSE;
    initial_blob_choice_len = blob_choices->length ();
    tessword = make_tess_word (word, NULL);
    tess_ratings = cc_recog (tessword, &tess_choice, &tess_raw,
      testing
      && tester != NULL /* ? call_tester : NULL */ ,
      testing
      && trainer !=
      NULL /* ? call_train_tester : NULL */ );
                                 //convert word
    outword = make_ed_word (tessword, word);
    if (outword == NULL) {
      outword = word->poly_copy (denorm->row ()->x_height ());
    }
    delete_word(tessword);  //get rid of it
                                 //no of blobs
    word_length = outword->blob_list ()->length ();
                                 //convert all ratings
    convert_choice_lists(tess_ratings, blob_choices);
                                 //copy string
    word_string = tess_raw.string;
    word_string_lengths = tess_raw.lengths;
    while (word_string_lengths.length () < word_length) {
      word_string += " ";        //pad with blanks
      word_string_lengths += 1;
    }
    raw_choice = new WERD_CHOICE (word_string.string (),
                                  word_string_lengths.string (),
                                  tess_raw.rating, tess_raw.certainty,
                                  tess_raw.permuter);
    word_string = tess_choice.string;
    word_string_lengths = tess_choice.lengths;
    if (word_string_lengths.length () > word_length) {
      tprintf ("recog_word: Discarded long string \"%s\""
               " (%d characters vs %d blobs)\n",
        word_string.string (), word_string_lengths.length(), word_length);
      word_string = NULL;        //should never happen
      word_string_lengths = NULL;
      tprintf("Word is at (%g,%g)\n",
              denorm->origin(),
              denorm->y(word->bounding_box().bottom(), 0.0));
    }
    if (blob_choices->length () - initial_blob_choice_len != word_length) {
      word_string = NULL;        //force rejection
      word_string_lengths = NULL;
      tprintf ("recog_word: Choices list len:%d; blob lists len:%d\n",
        blob_choices->length (), word_length);
                                 //list of lists
      choice_it.set_to_list (blob_choices);
      while (blob_choices->length () - initial_blob_choice_len <
      word_length) {
                                 //get fake one
        choice_list = new BLOB_CHOICE_LIST;
                                 //add to list
        choice_it.add_to_end (choice_list);
        tprintf ("recog_word: Added dummy choice list\n");
      }
      while (blob_choices->length () - initial_blob_choice_len >
      word_length) {
        choice_it.move_to_last ();
                                 //should never happen
        delete choice_it.extract ();
        tprintf ("recog_word: Deleted choice list\n");
      }
    }
    while (word_string_lengths.length () < word_length) {
      word_string += " ";        //pad with blanks
      word_string_lengths += 1;
    }

    assert (raw_choice != NULL);
    if (tess_choice.string) {
      strfree(tess_choice.string);
      strfree(tess_choice.lengths);
    }
    if (tess_raw.string) {
      strfree(tess_raw.string);
      strfree(tess_raw.lengths);
    }
    return new WERD_CHOICE (word_string.string (),
                            word_string_lengths.string (),
                            tess_choice.rating, tess_choice.certainty,
                            tess_choice.permuter);
  }
}


/**********************************************************************
 * split_and_recog_word
 *
 * Convert the word to tess form and pass it to the tess segmenter.
 * Convert the output back to editor form.
 **********************************************************************/

WERD_CHOICE *split_and_recog_word(                           //recog one owrd
                                  WERD *word,                //word to do
                                  DENORM *denorm,            //de-normaliser
                                  POLY_MATCHER matcher,      //matcher function
                                  POLY_TESTER tester,        //tester function
                                  POLY_TESTER trainer,       //trainer function
                                  BOOL8 testing,             //true if answer driven
                                  WERD_CHOICE *&raw_choice,  //raw result //list of blob lists
                                  BLOB_CHOICE_LIST_CLIST *blob_choices,
                                  WERD *&outword             //bln word output
                                 ) {
  //   inT32                                                      outword1_len;
  //   inT32                                                      outword2_len;
  WERD *first_word;              //poly copy of word
  WERD *second_word;             //fabricated word
  WERD *outword2;                //2nd output word
  PBLOB *blob;
  WERD_CHOICE *result;           //resturn value
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
  bestgap = -MAX_INT32;
  while (!blob_it.at_last ()) {
    blob = blob_it.data ();
                                 //gap to next
    gap = blob_it.data_relative (1)->bounding_box ().left () - blob->bounding_box ().right ();
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


/**********************************************************************
 * call_matcher
 *
 * Called from Tess with a blob in tess form.
 * Convert the blob to editor form.
 * Call the matcher setup by the segmenter in tess_matcher.
 * Convert the output choices back to tess form.
 **********************************************************************/

LIST call_matcher(                  //call a matcher
                  TBLOB *ptblob,    //previous
                  TBLOB *tessblob,  //blob to match
                  TBLOB *ntblob,    //next
                  void *,           //unused parameter
                  TEXTROW *         //always null anyway
                 ) {
  PBLOB *pblob;                  //converted blob
  PBLOB *blob;                   //converted blob
  PBLOB *nblob;                  //converted blob
  LIST result;                   //tess output
  BLOB_CHOICE *choice;           //current choice
  BLOB_CHOICE_LIST ratings;      //matcher result
  BLOB_CHOICE_IT it;             //iterator
  char choice_lengths[2] = {0, 0};

  blob = make_ed_blob (tessblob);//convert blob
  if (blob == NULL) {
    // Since it is actually possible to get a NULL blob here, due to invalid
    // segmentations, fake a really bad classification.
    choice_lengths[0] = strlen(unicharset.id_to_unichar(1));
    return append_choice(NULL, unicharset.id_to_unichar(1), choice_lengths,
                         static_cast<float>(MAX_NUM_INT_FEATURES),
                         static_cast<float>(kReallyBadCertainty), 0);
  }
  pblob = ptblob != NULL ? make_ed_blob (ptblob) : NULL;
  nblob = ntblob != NULL ? make_ed_blob (ntblob) : NULL;
  (*tess_matcher) (pblob, blob, nblob, tess_word, tess_denorm, ratings);
  //match it
  delete blob;                   //don't need that now
  if (pblob != NULL)
    delete pblob;
  if (nblob != NULL)
    delete nblob;
  it.set_to_list (&ratings);     //get list
  result = NULL;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    choice = it.data ();
    choice_lengths[0] = strlen(choice->unichar ());
    result = append_choice (result, choice->unichar (),
                            choice_lengths, choice->rating (),
                            choice->certainty (), choice->config ());
  }
  return result;                 //converted list
}


/**********************************************************************
 * call_tester
 *
 * Called from Tess with a blob in tess form.
 * Convert the blob to editor form.
 * Call the tester setup by the segmenter in tess_tester.
 **********************************************************************/

void call_tester(                     //call a tester
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
    (*tess_tester) (blob, tess_denorm, correct_blob, text, count, &ratings);
  delete blob;                   //don't need that now
}


/**********************************************************************
 * call_train_tester
 *
 * Called from Tess with a blob in tess form.
 * Convert the blob to editor form.
 * Call the trainer setup by the segmenter in tess_trainer.
 **********************************************************************/

void call_train_tester(                     //call a tester
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
    (*tess_trainer) (blob, tess_denorm, correct_blob, text, count, &ratings);
  delete blob;                   //don't need that now
}
