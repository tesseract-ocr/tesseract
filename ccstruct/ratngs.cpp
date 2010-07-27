/**********************************************************************
 * File: ratngs.cpp  (Formerly ratings.c)
 * Description: Code to manipulate the BLOB_CHOICE and WERD_CHOICE classes.
 * Author: Ray Smith
 * Created: Thu Apr 23 13:23:29 BST 1992
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

#include "ratngs.h"
#include "callcpp.h"
#include "genericvector.h"
#include "unicharset.h"

extern FILE *matcher_fp;

ELISTIZE (BLOB_CHOICE) CLISTIZE (BLOB_CHOICE_LIST) CLISTIZE (WERD_CHOICE)
//extern FILE* matcher_fp;

/**
 * BLOB_CHOICE::BLOB_CHOICE
 *
 * Constructor to build a BLOB_CHOICE from a char, rating and certainty.
 */
BLOB_CHOICE::BLOB_CHOICE(UNICHAR_ID src_unichar_id, //< character id
                         float src_rating,          //< rating
                         float src_cert,            //< certainty
                         inT8 src_config,           //< config (font)
                         int src_script_id          //< script
                        ) {
  unichar_id_ = src_unichar_id;
  rating_ = src_rating;
  certainty_ = src_cert;
  config_ = src_config;
  script_id_ = src_script_id;
}

/**
 * BLOB_CHOICE::BLOB_CHOICE
 *
 * Constructor to build a BLOB_CHOICE from another BLOB_CHOICE.
 */
BLOB_CHOICE::BLOB_CHOICE(const BLOB_CHOICE &other) {
  unichar_id_ = other.unichar_id();
  rating_ = other.rating();
  certainty_ = other.certainty();
  config_ = other.config();
  script_id_ = other.script_id();
}

/**
 * WERD_CHOICE::WERD_CHOICE
 *
 * Constructor to build a WERD_CHOICE from the given string.
 * The function assumes that src_string is not NULL.
 */
WERD_CHOICE::WERD_CHOICE(const char *src_string,
                         const UNICHARSET &unicharset) {
  STRING src_lengths;
  int len = strlen(src_string);
  const char *ptr = src_string;
  int step = unicharset.step(ptr);
  for (; ptr < src_string + len && step > 0;
       step = unicharset.step(ptr), src_lengths += step, ptr += step);
  if (step != 0 && ptr == src_string + len) {
    this->init(src_string, src_lengths.string(),
               0.0, 0.0, NO_PERM, unicharset);
  } else {  // there must have been an invalid unichar in the string
    this->init(8);
    this->make_bad();
  }
}

/**
 * WERD_CHOICE::init
 *
 * Helper function to build a WERD_CHOICE from the given string,
 * fragment lengths, rating, certainty and permuter.
 *
 * The function assumes that src_string is not NULL.
 * src_lengths argument could be NULL, in which case the unichars
 * in src_string are assumed to all be of length 1.
 */
void WERD_CHOICE::init(const char *src_string,
                       const char *src_lengths,
                       float src_rating,
                       float src_certainty,
                       uinT8 src_permuter,
                       const UNICHARSET &unicharset) {
  int src_string_len = strlen(src_string);
  if (src_string_len == 0) {
    this->init(8);
  } else {
    this->init(src_lengths ? strlen(src_lengths): src_string_len);
    length_ = reserved_;
    int offset = 0;
    for (int i = 0; i < length_; ++i) {
      int unichar_length = src_lengths ? src_lengths[i] : 1;
      unichar_ids_[i] =
          unicharset.unichar_to_id(src_string+offset, unichar_length);
      fragment_lengths_[i] = 1;
      offset += unichar_length;
    }
  }
  rating_ = src_rating;
  certainty_ = src_certainty;
  permuter_ = src_permuter;
}

/**
 * WERD_CHOICE::~WERD_CHOICE
 */
WERD_CHOICE::~WERD_CHOICE() {
  delete[] unichar_ids_;
  delete[] fragment_lengths_;
  delete_blob_choices();
}


/**
 * WERD_CHOICE::set_blob_choices
 *
 * Delete current blob_choices. Set the blob_choices to the given new
 * list.
 */
void WERD_CHOICE::set_blob_choices(BLOB_CHOICE_LIST_CLIST *blob_choices) {
  if (blob_choices_ != blob_choices) {
    delete_blob_choices();
    blob_choices_ = blob_choices;
  }
}


/**
 * contains_unichar_id
 *
 * Returns true if unichar_ids_ contain the given unichar_id, false otherwise.
 */
bool WERD_CHOICE::contains_unichar_id(UNICHAR_ID unichar_id) const {
  for (int i = 0; i < length_; ++i) {
    if (unichar_ids_[i] == unichar_id) {
      return true;
    }
  }
  return false;
}

/**
 * remove_unichar_ids
 *
 * Removes num unichar ids starting from index start from unichar_ids_
 * and updates length_ and fragment_lengths_ to reflect this change.
 * Note: this function does not modify rating_ and certainty_.
 */
void WERD_CHOICE::remove_unichar_ids(int start, int num) {
  ASSERT_HOST(start >= 0 && start + num <= length_);
  for (int i = start; i+num < length_; ++i) {
    unichar_ids_[i] = unichar_ids_[i+num];
    fragment_lengths_[i] = fragment_lengths_[i+num];
  }
  length_ -= num;
}

/**
 * string_and_lengths
 *
 * Populates the given word_str with unichars from unichar_ids and
 * and word_lengths_str with the corresponding unichar lengths.
 * Uses current_unicharset to make unichar id -> unichar conversions.
 */
void WERD_CHOICE::string_and_lengths(const UNICHARSET &current_unicharset,
                                     STRING *word_str,
                                     STRING *word_lengths_str) const {
  *word_str = "";
  if (word_lengths_str != NULL) *word_lengths_str = "";
  for (int i = 0; i < length_; ++i) {
    const char *ch = current_unicharset.id_to_unichar(unichar_ids_[i]);
    *word_str += ch;
    if (word_lengths_str != NULL) {
      *word_lengths_str += strlen(ch);
    }
  }
}

/**
 * append_unichar_id
 *
 * Make sure there is enough space in the word for the new unichar id
 * and call append_unichar_id_space_allocated().
 */
void WERD_CHOICE::append_unichar_id(
    UNICHAR_ID unichar_id, char fragment_length,
    float rating, float certainty) {
  if (length_ == reserved_) {
    this->double_the_size();
  }
  this->append_unichar_id_space_allocated(unichar_id, fragment_length,
                                          rating, certainty);
}

/**
 * WERD_CHOICE::operator+=
 *
 * Cat a second word rating on the end of this current one.
 * The ratings are added and the confidence is the min.
 * If the permuters are NOT the same the permuter is set to COMPOUND_PERM
 */
WERD_CHOICE & WERD_CHOICE::operator+= (const WERD_CHOICE & second) {
  // TODO(daria): find out why the choice was cleared this way if any
  // of the pieces are empty. Add the description of this behavior
  // to the comments.
  // if (word_string.length () == 0 || second.word_string.length () == 0) {
  //   word_string = NULL;          //make it empty
  //   word_lengths = NULL;
  //   delete_blob_choices();
  // } else {
  while (reserved_ < length_ + second.length()) {
    this->double_the_size();
  }
  const UNICHAR_ID *other_unichar_ids = second.unichar_ids();
  const char *other_fragment_lengths = second.fragment_lengths();
  for (int i = 0; i < second.length(); ++i) {
    unichar_ids_[length_ + i] = other_unichar_ids[i];
    fragment_lengths_[length_ + i] = other_fragment_lengths[i];
  }
  length_ += second.length();
  rating_ += second.rating();  // add ratings
  if (second.certainty() < certainty_) // take min
    certainty_ = second.certainty();
  if (permuter_ == NO_PERM) {
    permuter_ = second.permuter();
  } else if (second.permuter() != NO_PERM &&
             second.permuter() != permuter_) {
    permuter_ = COMPOUND_PERM;
  }
  unichar_string_ += second.unichar_string();
  unichar_lengths_ += second.unichar_lengths();

  // Append a deep copy of second blob_choices if it exists.
  if (second.blob_choices_ != NULL) {
    if (this->blob_choices_ == NULL)
      this->blob_choices_ = new BLOB_CHOICE_LIST_CLIST;

    BLOB_CHOICE_LIST_C_IT this_blob_choices_it;
    BLOB_CHOICE_LIST_C_IT second_blob_choices_it;

    this_blob_choices_it.set_to_list(this->blob_choices_);
    this_blob_choices_it.move_to_last();

    second_blob_choices_it.set_to_list(second.blob_choices_);

    for (second_blob_choices_it.mark_cycle_pt();
         !second_blob_choices_it.cycled_list();
         second_blob_choices_it.forward()) {

      BLOB_CHOICE_LIST* blob_choices_copy = new BLOB_CHOICE_LIST();
      blob_choices_copy->deep_copy(second_blob_choices_it.data(),
                                   &BLOB_CHOICE::deep_copy);

      this_blob_choices_it.add_after_then_move(blob_choices_copy);
    }
  }
  return *this;
}


/**
 * WERD_CHOICE::operator=
 *
 * Allocate enough memory to hold a copy of source and copy over
 * all the information from source to this WERD_CHOICE.
 */
WERD_CHOICE& WERD_CHOICE::operator=(const WERD_CHOICE& source) {
  while (reserved_ < source.length()) {
    this->double_the_size();
  }

  const UNICHAR_ID *other_unichar_ids = source.unichar_ids();
  const char *other_fragment_lengths = source.fragment_lengths();
  for (int i = 0; i < source.length(); ++i) {
    unichar_ids_[i] = other_unichar_ids[i];
    fragment_lengths_[i] = other_fragment_lengths[i];
  }
  length_ = source.length();
  rating_ = source.rating();
  certainty_ = source.certainty();
  permuter_ = source.permuter();
  fragment_mark_ = source.fragment_mark();
  unichar_string_ = source.unichar_string();
  unichar_lengths_ = source.unichar_lengths();

  // Delete existing blob_choices
  this->delete_blob_choices();

  // Deep copy blob_choices of source
  if (source.blob_choices_ != NULL) {
    BLOB_CHOICE_LIST_C_IT this_blob_choices_it;
    BLOB_CHOICE_LIST_C_IT source_blob_choices_it;

    this->blob_choices_ = new BLOB_CHOICE_LIST_CLIST();

    this_blob_choices_it.set_to_list(this->blob_choices_);
    source_blob_choices_it.set_to_list(source.blob_choices_);

    for (source_blob_choices_it.mark_cycle_pt();
         !source_blob_choices_it.cycled_list();
         source_blob_choices_it.forward()) {

      BLOB_CHOICE_LIST* blob_choices_copy = new BLOB_CHOICE_LIST();
      blob_choices_copy->deep_copy(source_blob_choices_it.data(),
                                   &BLOB_CHOICE::deep_copy);

      this_blob_choices_it.add_after_then_move(blob_choices_copy);
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
  if (blob_choices_ != NULL) {
    blob_choices_->deep_clear();
    delete blob_choices_;
    blob_choices_ = NULL;
  }
}

/**
 * WERD_CHOICE::print
 *
 * Print WERD_CHOICE to stdout.
 */
const void WERD_CHOICE::print(const char *msg) const {
  tprintf("%s WERD_CHOICE:\n", msg);
  tprintf("length_ %d reserved_ %d permuter_ %d\n",
         length_, reserved_, permuter_);
  tprintf("rating_ %.4f certainty_ %.4f", rating_, certainty_);
  if (fragment_mark_) {
    tprintf(" fragment_mark_ true");
  }
  tprintf("\n");
  if (unichar_string_.length() > 0) {
    tprintf("unichar_string_ %s unichar_lengths_ %s\n",
            unichar_string_.string(), unichar_lengths_.string());
  }
  tprintf("unichar_ids: ");
  int i;
  for (i = 0; i < length_; ++i) {
    tprintf("%d ", unichar_ids_[i]);
  }
  tprintf("\nfragment_lengths_: ");
  for (i = 0; i < length_; ++i) {
    tprintf("%d ", fragment_lengths_[i]);
  }
  tprintf("\n");
  fflush(stdout);
}

/**
 * print_ratings_list
 *
 * Send all the ratings out to the logfile.
 *
 * @param msg intro message
 * @param ratings list of ratings
 * @param current_unicharset unicharset that can be used
 * for id-to-unichar conversion
 */
void print_ratings_list(const char *msg,
                        BLOB_CHOICE_LIST *ratings,
                        const UNICHARSET &current_unicharset) {
  if (ratings->length() == 0) {
    tprintf("%s:<none>\n", msg);
    return;
  }
  if (*msg != '\0') {
    tprintf("%s\n", msg);
  }
  BLOB_CHOICE_IT c_it;
  c_it.set_to_list(ratings);
  for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
    tprintf("r%.2f c%.2f : %d %s",
            c_it.data()->rating(), c_it.data()->certainty(),
            c_it.data()->unichar_id(),
            current_unicharset.debug_str(c_it.data()->unichar_id()).string());
    if (!c_it.at_last()) {
      tprintf("\n");
    }
  }
  tprintf("\n");
  fflush(stdout);
}

/**
 * print_ratings_list
 *
 * Print ratings list (unichar ids only).
 */
void print_ratings_list(const char *msg, BLOB_CHOICE_LIST *ratings) {
  if (ratings->length() == 0) {
    tprintf("%s:<none>\n", msg);
    return;
  }
  if (*msg != '\0') {
    tprintf("%s\n", msg);
  }
  BLOB_CHOICE_IT c_it;
  c_it.set_to_list(ratings);
  for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
    tprintf("r%.2f c%.2f : %d", c_it.data()->rating(),
            c_it.data()->certainty(), c_it.data()->unichar_id());
    if (!c_it.at_last()) {
      tprintf("\n");
    }
  }
  tprintf("\n");
  fflush(stdout);
}

/**
 * print_ratings_info
 *
 * Send all the ratings out to the logfile.
 *
 * @param fp file to use
 * @param ratings list of results
 * @param current_unicharset unicharset that can be used
 * for id-to-unichar conversion
 */
void print_ratings_info(FILE *fp,
                        BLOB_CHOICE_LIST *ratings,
                        const UNICHARSET &current_unicharset) {
  inT32 index;                    // to list
  inT32 best_index;               // to list
  FLOAT32 best_rat;               // rating
  FLOAT32 best_cert;              // certainty
  const char* first_char = NULL;  // character
  FLOAT32 first_rat;              // rating
  FLOAT32 first_cert;             // certainty
  const char* sec_char = NULL;    // character
  FLOAT32 sec_rat = 0.0f;         // rating
  FLOAT32 sec_cert = 0.0f;        // certainty
  BLOB_CHOICE_IT c_it = ratings;  // iterator

  index = ratings->length();
  if (index > 0) {
    first_char = current_unicharset.id_to_unichar(c_it.data()->unichar_id());
    first_rat = c_it.data()->rating();
    first_cert = -c_it.data()->certainty();
    if (index > 1) {
      sec_char = current_unicharset.id_to_unichar(
          c_it.data_relative(1)->unichar_id());
      sec_rat = c_it.data_relative(1)->rating();
      sec_cert = -c_it.data_relative(1)->certainty();
    } else {
      sec_char = NULL;
      sec_rat = -1;
      sec_cert = -1;
    }
  } else {
    first_char = NULL;
    first_rat = -1;
    first_cert = -1;
  }
  best_index = -1;
  best_rat = -1;
  best_cert = -1;
  for (index = 0, c_it.mark_cycle_pt(); !c_it.cycled_list();
       c_it.forward(), index++) {
    if (strcmp(current_unicharset.id_to_unichar(c_it.data()->unichar_id()),
               blob_answer) == 0) {
      best_index = index;
      best_rat = c_it.data()->rating();
      best_cert = -c_it.data()->certainty();
    }
  }
  if (first_char != NULL && (*first_char == '\0' || *first_char == ' '))
    first_char = NULL;
  if (sec_char != NULL && (*sec_char == '\0' || *sec_char == ' '))
    sec_char = NULL;
  fprintf(matcher_fp,
          " " INT32FORMAT " " INT32FORMAT " %g %g %s %g %g %s %g %g\n",
          ratings->length(), best_index, best_rat, best_cert,
          first_char != NULL ? first_char : "~",
          first_rat, first_cert, sec_char != NULL ? sec_char : "~",
          sec_rat, sec_cert);
}

/**
 * print_char_choices_list
 */
void print_char_choices_list(const char *msg,
                             const BLOB_CHOICE_LIST_VECTOR &char_choices,
                             const UNICHARSET &current_unicharset,
                             BOOL8 detailed) {
  if (*msg != '\0') tprintf("%s\n", msg);
  for (int x = 0; x < char_choices.length(); ++x) {
    BLOB_CHOICE_IT c_it;
    c_it.set_to_list(char_choices.get(x));
    tprintf("char[%d]: %s\n", x,
            current_unicharset.debug_str( c_it.data()->unichar_id()).string());
    if (detailed)
      print_ratings_list("  ", char_choices.get(x), current_unicharset);
  }
}
