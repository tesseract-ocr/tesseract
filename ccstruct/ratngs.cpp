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

ELISTIZE (BLOB_CHOICE) CLISTIZE (BLOB_CHOICE_LIST) CLISTIZE (WERD_CHOICE);

const float WERD_CHOICE::kBadRating = 100000.0;

static const char kPermuterTypeNoPerm[] = "None";
static const char kPermuterTypePuncPerm[] = "Punctuation";
static const char kPermuterTypeTopPerm[] = "Top Choice";
static const char kPermuterTypeLowerPerm[] = "Top Lower Case";
static const char kPermuterTypeUpperPerm[] = "Top Upper Case";
static const char kPermuterTypeNgramPerm[] = "Ngram";
static const char kPermuterTypeNumberPerm[] = "Number";
static const char kPermuterTypeUserPatPerm[] = "User Pattern";
static const char kPermuterTypeSysDawgPerm[] = "System Dictionary";
static const char kPermuterTypeDocDawgPerm[] = "Document Dictionary";
static const char kPermuterTypeUserDawgPerm[] = "User Dictionary";
static const char kPermuterTypeFreqDawgPerm[] = "Frequent Words Dictionary";
static const char kPermuterTypeCompoundPerm[] = "Compound";

static const char * const kPermuterTypeNames[] = {
    kPermuterTypeNoPerm,        // 0
    kPermuterTypePuncPerm,      // 1
    kPermuterTypeTopPerm,       // 2
    kPermuterTypeLowerPerm,     // 3
    kPermuterTypeUpperPerm,     // 4
    kPermuterTypeNgramPerm,     // 5
    kPermuterTypeNumberPerm,    // 6
    kPermuterTypeUserPatPerm,   // 7
    kPermuterTypeSysDawgPerm,   // 8
    kPermuterTypeDocDawgPerm,   // 9
    kPermuterTypeUserDawgPerm,  // 10
    kPermuterTypeFreqDawgPerm,  // 11
    kPermuterTypeCompoundPerm   // 12
};

/**
 * BLOB_CHOICE::BLOB_CHOICE
 *
 * Constructor to build a BLOB_CHOICE from a char, rating and certainty.
 */
BLOB_CHOICE::BLOB_CHOICE(UNICHAR_ID src_unichar_id, // character id
                         float src_rating,         // rating
                         float src_cert,           // certainty
                         inT16 src_fontinfo_id,     // font
                         inT16 src_fontinfo_id2,    // 2nd choice font
                         int src_script_id,        // script
                         inT16 min_xheight,        // min xheight allowed
                         inT16 max_xheight,        // max xheight by this char
                         bool adapted              // adapted match or not
                        ) {
  unichar_id_ = src_unichar_id;
  rating_ = src_rating;
  certainty_ = src_cert;
  fontinfo_id_ = src_fontinfo_id;
  fontinfo_id2_ = src_fontinfo_id2;
  script_id_ = src_script_id;
  language_model_state_ = NULL;
  min_xheight_ = min_xheight;
  max_xheight_ = max_xheight;
  adapted_ = adapted;
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
  fontinfo_id_ = other.fontinfo_id();
  fontinfo_id2_ = other.fontinfo_id2();
  script_id_ = other.script_id();
  language_model_state_ = NULL;
  min_xheight_ = other.min_xheight_;
  max_xheight_ = other.max_xheight_;
  adapted_ = other.adapted_;
}

/**
 * WERD_CHOICE::WERD_CHOICE
 *
 * Constructor to build a WERD_CHOICE from the given string.
 * The function assumes that src_string is not NULL.
 */
WERD_CHOICE::WERD_CHOICE(const char *src_string,
                         const UNICHARSET &unicharset)
    : unicharset_(&unicharset){
  STRING src_lengths;
  const char *ptr = src_string;
  const char *end = src_string + strlen(src_string);
  int step = unicharset.step(ptr);
  for (; ptr < end && step > 0;
       step = unicharset.step(ptr), src_lengths += step, ptr += step);
  if (step != 0 && ptr == end) {
    this->init(src_string, src_lengths.string(),
               0.0, 0.0, NO_PERM);
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
                       uinT8 src_permuter) {
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
          unicharset_->unichar_to_id(src_string+offset, unichar_length);
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

const char *WERD_CHOICE::permuter_name() const {
  return kPermuterTypeNames[permuter_];
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
 * reverse_and_mirror_unichar_ids
 *
 * Reverses and mirrors unichars in unichar_ids.
 */
void WERD_CHOICE::reverse_and_mirror_unichar_ids() {
  for (int i = 0; i < length_/2; ++i) {
    UNICHAR_ID tmp_id = unichar_ids_[i];
    unichar_ids_[i] = unicharset_->get_mirror(unichar_ids_[length_-1-i]);
    unichar_ids_[length_-1-i] = unicharset_->get_mirror(tmp_id);
  }
  if (length_ % 2 != 0) {
    unichar_ids_[length_/2] = unicharset_->get_mirror(unichar_ids_[length_/2]);
  }
}

/**
 * punct_stripped
 *
 * Returns the half-open interval of unichar_id indices [start, end) which
 * enclose the core portion of this word -- the part after stripping
 * punctuation from the left and right.
 */
void WERD_CHOICE::punct_stripped(int *start, int *end) const {
  *start = 0;
  *end = length() - 1;
  while (*start < length() &&
         unicharset()->get_ispunctuation(unichar_id(*start))) {
    (*start)++;
  }
  while (*end > -1 &&
         unicharset()->get_ispunctuation(unichar_id(*end))) {
    (*end)--;
  }
  (*end)++;
}

WERD_CHOICE WERD_CHOICE::shallow_copy(int start, int end) const {
  ASSERT_HOST(start >= 0 && start <= length_);
  ASSERT_HOST(end >= 0 && end <= length_);
  if (end < start) { end = start; }
  WERD_CHOICE retval(unicharset_, end - start);
  for (int i = start; i < end; i++) {
    retval.append_unichar_id_space_allocated(
        unichar_ids_[i], fragment_lengths_[i], 0.0f, 0.0f);
  }
  return retval;
}

/**
 * has_rtl_unichar_id
 *
 * Returns true if unichar_ids contain at least one "strongly" RTL unichar.
 */
bool WERD_CHOICE::has_rtl_unichar_id() const {
  int i;
  for (i = 0; i < length_; ++i) {
    UNICHARSET::Direction dir = unicharset_->get_direction(unichar_ids_[i]);
    if (dir == UNICHARSET::U_RIGHT_TO_LEFT ||
        dir == UNICHARSET::U_RIGHT_TO_LEFT_ARABIC) {
      return true;
    }
  }
  return false;
}

/**
 * string_and_lengths
 *
 * Populates the given word_str with unichars from unichar_ids and
 * and word_lengths_str with the corresponding unichar lengths.
 */
void WERD_CHOICE::string_and_lengths(STRING *word_str,
                                     STRING *word_lengths_str) const {
  *word_str = "";
  if (word_lengths_str != NULL) *word_lengths_str = "";
  for (int i = 0; i < length_; ++i) {
    const char *ch = unicharset_->id_to_unichar_ext(unichar_ids_[i]);
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
  ASSERT_HOST(unicharset_ == second.unicharset_);
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

  unicharset_ = source.unicharset_;
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

bool EqualIgnoringCaseAndTerminalPunct(const WERD_CHOICE &word1,
                                       const WERD_CHOICE &word2) {
  const UNICHARSET *uchset = word1.unicharset();
  if (word2.unicharset() != uchset) return false;
  int w1start, w1end;
  word1.punct_stripped(&w1start, &w1end);
  int w2start, w2end;
  word2.punct_stripped(&w2start, &w2end);
  if (w1end - w1start != w2end - w2start) return false;
  for (int i = 0; i < w1end - w1start; i++) {
    if (uchset->to_lower(word1.unichar_id(w1start + i)) !=
        uchset->to_lower(word2.unichar_id(w2start + i))) {
        return false;
    }
  }
  return true;
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
    c_it.data()->print(&current_unicharset);
    if (!c_it.at_last()) tprintf("\n");
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
    c_it.data()->print(NULL);
    if (!c_it.at_last()) tprintf("\n");
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
  if (first_char != NULL && (*first_char == '\0' || *first_char == ' '))
    first_char = NULL;
  if (sec_char != NULL && (*sec_char == '\0' || *sec_char == ' '))
    sec_char = NULL;
  tprintf(" " INT32FORMAT " %s %g %g %s %g %g\n",
          ratings->length(),
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
    tprintf("\nchar[%d]: %s\n", x,
            current_unicharset.debug_str( c_it.data()->unichar_id()).string());
    if (detailed)
      print_ratings_list("", char_choices.get(x), current_unicharset);
  }
}

/**
 * print_word_alternates_list
 */
void print_word_alternates_list(
    WERD_CHOICE *word,
    GenericVector<WERD_CHOICE *> *alternates) {
  if (!word || !alternates) return;

  STRING alternates_str;
  for (int i = 0; i < alternates->size(); i++) {
    if (i > 0) alternates_str += "\", \"";
    alternates_str += alternates->get(i)->unichar_string();
  }
  tprintf("Alternates for \"%s\": {\"%s\"}\n",
          word->unichar_string().string(), alternates_str.string());
}
