/**********************************************************************
 * File:			ratngs.cpp  (Formerly ratings.c)
 * Description:	Code to manipulate the BLOB_CHOICE and WERD_CHOICE classes.
 * Author:		Ray Smith
 * Created:		Thu Apr 23 13:23:29 BST 1992
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

#include          "mfcpch.h"
//#include                                      "ipeerr.h"
#include          "callcpp.h"
#include          "ratngs.h"
//#include "tordvars.h"

extern FILE *matcher_fp;

ELISTIZE (BLOB_CHOICE) CLISTIZE (BLOB_CHOICE_LIST) CLISTIZE (WERD_CHOICE)
//extern FILE* matcher_fp;
/**********************************************************************
 * BLOB_CHOICE::BLOB_CHOICE
 *
 * Constructor to build a BLOB_CHOICE from a char, rating and certainty.
 **********************************************************************/
BLOB_CHOICE::BLOB_CHOICE(                   //constructor
                         char *src_unichar, //character
                         float src_rating,  //rating
                         float src_cert,    //certainty
                         inT8 src_config,   //config (font)
                         const char* src_script  //script
                        ) {
  strcpy(blob_unichar, src_unichar);
  blob_rating = src_rating;
  blob_certainty = src_cert;     //just copy them
  blob_config = src_config;
  blob_script = src_script;
}


/**********************************************************************
 * WERD_CHOICE::WERD_CHOICE
 *
 * Constructor to build a WERD_CHOICE from a char, rating and certainty.
 **********************************************************************/

WERD_CHOICE::WERD_CHOICE (
//constructor
const char *src_string,          //word string
const char *src_lengths,         //unichar lengths
float src_rating,                //rating
float src_cert,                  //certainty
uinT8 src_permuter               //permuter code
):
word_string(src_string),
word_lengths(src_lengths) {
  word_rating = src_rating;
  word_certainty = src_cert;
  word_permuter = src_permuter;  //just copy them
  word_blob_choices = NULL;
}


/**********************************************************************
 * WERD_CHOICE::~WERD_CHOICE
 **********************************************************************/

WERD_CHOICE::~WERD_CHOICE() {
  delete_blob_choices();
}


/**********************************************************************
 * WERD_CHOICE::set_blob_choices
 *
 * Delete current blob_choices. Set the blob_choices to the given new
 * list.
 **********************************************************************/

void WERD_CHOICE::set_blob_choices(BLOB_CHOICE_LIST_CLIST *blob_choices) {
  if (word_blob_choices != blob_choices) {
    delete_blob_choices();
    word_blob_choices = blob_choices;
  }
}

/**********************************************************************
 * WERD_CHOICE::operator+=
 *
 * Cat a second word rating on the end of this current one.
 * The ratings are added and the confidence is the min.
 * If the permuters are NOT the same the permuter is set to COMPOUND_PERM
 **********************************************************************/

                                 //add one on
WERD_CHOICE & WERD_CHOICE::operator+= (
const WERD_CHOICE & second       //second word
) {
  if (word_string.length () == 0 || second.word_string.length () == 0) {
    word_string = NULL;          //make it empty
    word_lengths = NULL;
    delete_blob_choices();
  }
  else {
                                 //add ratings
    word_rating += second.word_rating;
    if (second.word_certainty < word_certainty)
                                 //take min
      word_certainty = second.word_certainty;
                                 //cat strings
    word_string += second.word_string;
    word_lengths += second.word_lengths;
    if (second.word_permuter != word_permuter)
      word_permuter = COMPOUND_PERM;

    // Append a deep copy of second blob_choices if it exists
    if (second.word_blob_choices != NULL) {

      if (this->word_blob_choices == NULL)
        this->word_blob_choices = new BLOB_CHOICE_LIST_CLIST;

      BLOB_CHOICE_LIST_C_IT this_word_blob_choices_it;
      BLOB_CHOICE_LIST_C_IT second_word_blob_choices_it;

      this_word_blob_choices_it.set_to_list(this->word_blob_choices);
      this_word_blob_choices_it.move_to_last();

      second_word_blob_choices_it.set_to_list(second.word_blob_choices);

      for (second_word_blob_choices_it.mark_cycle_pt();
           !second_word_blob_choices_it.cycled_list();
           second_word_blob_choices_it.forward()) {

      BLOB_CHOICE_LIST* blob_choices_copy = new BLOB_CHOICE_LIST();
      blob_choices_copy->deep_copy(second_word_blob_choices_it.data(),
                                   &BLOB_CHOICE::deep_copy);

        this_word_blob_choices_it.add_after_then_move(blob_choices_copy);
      }
    }
  }

  return *this;
}

/**********************************************************************
 * WERD_CHOICE::operator=
 **********************************************************************/
WERD_CHOICE& WERD_CHOICE::operator= (const WERD_CHOICE& source) {
  if (&source != this) {
    this->word_string = source.word_string;
    this->word_lengths = source.word_lengths;
    this->word_rating = source.word_rating;
    this->word_certainty = source.word_certainty;
    this->word_permuter = source.word_permuter;

    // Delete existing blob_choices
    this->delete_blob_choices();

    // Deep copy blob_choices of source
    if (source.word_blob_choices != NULL) {
      BLOB_CHOICE_LIST_C_IT this_word_blob_choices_it;
      BLOB_CHOICE_LIST_C_IT source_word_blob_choices_it;

      this->word_blob_choices = new BLOB_CHOICE_LIST_CLIST();

      this_word_blob_choices_it.set_to_list(this->word_blob_choices);
      source_word_blob_choices_it.set_to_list(source.word_blob_choices);

      for (source_word_blob_choices_it.mark_cycle_pt();
           !source_word_blob_choices_it.cycled_list();
           source_word_blob_choices_it.forward()) {

      BLOB_CHOICE_LIST* blob_choices_copy = new BLOB_CHOICE_LIST();
      blob_choices_copy->deep_copy(source_word_blob_choices_it.data(),
                                   &BLOB_CHOICE::deep_copy);

        this_word_blob_choices_it.add_after_then_move(blob_choices_copy);
      }
    }
  }
  return *this;
}

/**********************************************************************
 * WERD_CHOICE::delete_blob_choices
 *
 * Clear the blob_choices list, delete it and set it to NULL.
 **********************************************************************/

void WERD_CHOICE::delete_blob_choices() {
  if (word_blob_choices != NULL) {
    word_blob_choices->deep_clear();
    delete word_blob_choices;
    word_blob_choices = NULL;
  }
}

/**********************************************************************
 * print_ratings_list
 *
 * Send all the ratings out to the logfile.
 **********************************************************************/

void print_ratings_list(                           //print whole list
                        const char *msg,           //intro message
                        BLOB_CHOICE_LIST *ratings  //list of results
                       ) {
  BLOB_CHOICE_IT
    c_it = ratings;              //iterator

  switch (ratings->length ()) {
    case 0:
      tprintf ("%s:<none>", msg);
      break;
    case 1:
      tprintf ("%s:%s/%g/%g", msg,
        c_it.data ()->unichar (),
        c_it.data ()->rating (), c_it.data ()->certainty ());
      break;
    case 2:
      tprintf ("%s:%s/%g/%g %s/%g/%g", msg,
        c_it.data ()->unichar (),
        c_it.data ()->rating (),
        c_it.data ()->certainty (),
        c_it.data_relative (1)->unichar (),
        c_it.data_relative (1)->rating (),
        c_it.data_relative (1)->certainty ());
      break;
    case 3:
      tprintf ("%s:%s/%g/%g %s/%g/%g %s/%g/%g", msg,
        c_it.data ()->unichar (),
        c_it.data ()->rating (),
        c_it.data ()->certainty (),
        c_it.data_relative (1)->unichar (),
        c_it.data_relative (1)->rating (),
        c_it.data_relative (1)->certainty (),
        c_it.data_relative (2)->unichar (),
        c_it.data_relative (2)->rating (),
        c_it.data_relative (2)->certainty ());
      break;
    case 4:
      tprintf ("%s:%s/%g/%g %s/%g/%g %s/%g/%g %s/%g/%g", msg,
        c_it.data ()->unichar (),
        c_it.data ()->rating (),
        c_it.data ()->certainty (),
        c_it.data_relative (1)->unichar (),
        c_it.data_relative (1)->rating (),
        c_it.data_relative (1)->certainty (),
        c_it.data_relative (2)->unichar (),
        c_it.data_relative (2)->rating (),
        c_it.data_relative (2)->certainty (),
        c_it.data_relative (3)->unichar (),
        c_it.data_relative (3)->rating (),
        c_it.data_relative (3)->certainty ());
      break;
    default:
      tprintf ("%s:%s/%g/%g %s/%g/%g %s/%g/%g %s/%g/%g %s/%g/%g", msg,
        c_it.data ()->unichar (),
        c_it.data ()->rating (),
        c_it.data ()->certainty (),
        c_it.data_relative (1)->unichar (),
        c_it.data_relative (1)->rating (),
        c_it.data_relative (1)->certainty (),
        c_it.data_relative (2)->unichar (),
        c_it.data_relative (2)->rating (),
        c_it.data_relative (2)->certainty (),
        c_it.data_relative (3)->unichar (),
        c_it.data_relative (3)->rating (),
        c_it.data_relative (3)->certainty (),
        c_it.data_relative (4)->unichar (),
        c_it.data_relative (4)->rating (),
        c_it.data_relative (4)->certainty ());
      c_it.forward ();
      c_it.forward ();
      c_it.forward ();
      c_it.forward ();
      while (!c_it.at_last ()) {
        c_it.forward ();
        tprintf ("%s/%g/%g",
          c_it.data ()->unichar (),
          c_it.data ()->rating (), c_it.data ()->certainty ());
      }

      break;
  }
}


/**********************************************************************
 * print_ratings_info
 *
 * Send all the ratings out to the logfile.
 **********************************************************************/

void print_ratings_info(                           //print summary info
                        FILE *fp,                  //file to use
                        BLOB_CHOICE_LIST *ratings  //list of results
                       ) {
  inT32
    index;                       //to list
  inT32
    best_index;                  //to list
  FLOAT32
    best_rat;                    //rating
  FLOAT32
    best_cert;                   //certainty
  const char*
    first_char = NULL;           //character
  FLOAT32
    first_rat;                   //rating
  FLOAT32
    first_cert;                  //certainty
  const char*
    sec_char = NULL;             //character
  FLOAT32
    sec_rat = 0.0f;              //rating
  FLOAT32
    sec_cert = 0.0f;             //certainty
  BLOB_CHOICE_IT
    c_it = ratings;              //iterator

  index = ratings->length ();
  if (index > 0) {
    first_char = c_it.data ()->unichar ();
    first_rat = c_it.data ()->rating ();
    first_cert = -c_it.data ()->certainty ();
    if (index > 1) {
      sec_char = c_it.data_relative (1)->unichar ();
      sec_rat = c_it.data_relative (1)->rating ();
      sec_cert = -c_it.data_relative (1)->certainty ();
    }
    else {
      sec_char = NULL;
      sec_rat = -1;
      sec_cert = -1;
    }
  }
  else {
    first_char = NULL;
    first_rat = -1;
    first_cert = -1;
  }
  best_index = -1;
  best_rat = -1;
  best_cert = -1;
  for (index = 0, c_it.mark_cycle_pt (); !c_it.cycled_list ();
  c_it.forward (), index++) {
    if (strcmp(c_it.data ()->unichar (), blob_answer) == 0) {
      best_index = index;
      best_rat = c_it.data ()->rating ();
      best_cert = -c_it.data ()->certainty ();
    }
  }
  if (first_char != NULL && (*first_char == '\0' || *first_char == ' '))
    first_char = NULL;
  if (sec_char != NULL && (*sec_char == '\0' || *sec_char == ' '))
    sec_char = NULL;
  fprintf (matcher_fp,
    " " INT32FORMAT " " INT32FORMAT " %g %g %s %g %g %s %g %g\n",
    ratings->length (), best_index, best_rat, best_cert, first_char != NULL ? first_char : "~",
    first_rat, first_cert, sec_char != NULL ? sec_char : "~", sec_rat, sec_cert);
}
