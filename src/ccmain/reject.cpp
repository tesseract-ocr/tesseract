/**********************************************************************
 * File:        reject.cpp  (Formerly reject.c)
 * Description: Rejection functions used in tessedit
 * Author:    Phil Cheatle
 * Created:   Wed Sep 23 16:50:21 BST 1992
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifdef DISABLED_LEGACY_ENGINE

#include "tesseractclass.h"

namespace tesseract {

int16_t Tesseract::safe_dict_word(const WERD_RES *werd_res) {
  const WERD_CHOICE &word = *werd_res->best_choice;
  int dict_word_type = werd_res->tesseract->dict_word(word);
  return dict_word_type == DOC_DAWG_PERM ? 0 : dict_word_type;
}
}  // namespace tesseract

#else

#include "tessvars.h"
#include <cctype>
#include <cerrno>
#include <cstring>
#include "genericvector.h"
#include "reject.h"
#include "control.h"
#include "docqual.h"
#include "globaloc.h"  // For err_exit.
#include "globals.h"
#include "helpers.h"

#include "tesseractclass.h"


CLISTIZEH (STRING) CLISTIZE (STRING)

/*************************************************************************
 * set_done()
 *
 * Set the done flag based on the word acceptability criteria
 *************************************************************************/

namespace tesseract {
void Tesseract::set_done(WERD_RES *word, int16_t pass) {
  word->done = word->tess_accepted &&
      (strchr(word->best_choice->unichar_string().string(), ' ') == nullptr);
  bool word_is_ambig = word->best_choice->dangerous_ambig_found();
  bool word_from_dict = word->best_choice->permuter() == SYSTEM_DAWG_PERM ||
      word->best_choice->permuter() == FREQ_DAWG_PERM ||
      word->best_choice->permuter() == USER_DAWG_PERM;
  if (word->done && (pass == 1) && (!word_from_dict || word_is_ambig) &&
      one_ell_conflict(word, false)) {
    if (tessedit_rejection_debug) tprintf("one_ell_conflict detected\n");
    word->done = FALSE;
  }
  if (word->done && ((!word_from_dict &&
      word->best_choice->permuter() != NUMBER_PERM) || word_is_ambig)) {
    if (tessedit_rejection_debug) tprintf("non-dict or ambig word detected\n");
      word->done = FALSE;
  }
  if (tessedit_rejection_debug) {
    tprintf("set_done(): done=%d\n", word->done);
    word->best_choice->print("");
  }
}


/*************************************************************************
 * make_reject_map()
 *
 * Sets the done flag to indicate whether the resylt is acceptable.
 *
 * Sets a reject map for the word.
 *************************************************************************/
void Tesseract::make_reject_map(WERD_RES *word, ROW *row, int16_t pass) {
  int i;
  int offset;

  flip_0O(word);
  check_debug_pt(word, -1);     // For trap only
  set_done(word, pass);  // Set acceptance
  word->reject_map.initialise(word->best_choice->unichar_lengths().length());
  reject_blanks(word);
  /*
  0: Rays original heuristic - the baseline
  */
  if (tessedit_reject_mode == 0) {
    if (!word->done)
      reject_poor_matches(word);
  } else if (tessedit_reject_mode == 5) {
    /*
    5: Reject I/1/l from words where there is no strong contextual confirmation;
      the whole of any unacceptable words (incl PERM rej of dubious 1/I/ls);
      and the whole of any words which are very small
    */
    if (kBlnXHeight / word->denorm.y_scale() <= min_sane_x_ht_pixels) {
      word->reject_map.rej_word_small_xht();
    } else {
      one_ell_conflict(word, true);
      /*
        Originally the code here just used the done flag. Now I have duplicated
        and unpacked the conditions for setting the done flag so that each
        mechanism can be turned on or off independently. This works WITHOUT
        affecting the done flag setting.
      */
      if (rej_use_tess_accepted && !word->tess_accepted)
        word->reject_map.rej_word_not_tess_accepted ();

      if (rej_use_tess_blanks &&
        (strchr (word->best_choice->unichar_string().string (), ' ') != nullptr))
        word->reject_map.rej_word_contains_blanks ();

      WERD_CHOICE* best_choice = word->best_choice;
      if (rej_use_good_perm) {
        if ((best_choice->permuter() == SYSTEM_DAWG_PERM ||
             best_choice->permuter() == FREQ_DAWG_PERM ||
             best_choice->permuter() == USER_DAWG_PERM) &&
            (!rej_use_sensible_wd ||
             acceptable_word_string(*word->uch_set,
                                    best_choice->unichar_string().string(),
                                    best_choice->unichar_lengths().string()) !=
                                        AC_UNACCEPTABLE)) {
          // PASSED TEST
        } else if (best_choice->permuter() == NUMBER_PERM) {
          if (rej_alphas_in_number_perm) {
            for (i = 0, offset = 0;
                 best_choice->unichar_string()[offset] != '\0';
                 offset += best_choice->unichar_lengths()[i++]) {
              if (word->reject_map[i].accepted() &&
                  word->uch_set->get_isalpha(
                      best_choice->unichar_string().string() + offset,
                      best_choice->unichar_lengths()[i]))
                word->reject_map[i].setrej_bad_permuter();
              // rej alpha
            }
          }
        } else {
          word->reject_map.rej_word_bad_permuter();
        }
      }
      /* Ambig word rejection was here once !!*/
    }
  } else {
    tprintf("BAD tessedit_reject_mode\n");
    err_exit();
  }

  if (tessedit_image_border > -1)
    reject_edge_blobs(word);

  check_debug_pt (word, 10);
  if (tessedit_rejection_debug) {
    tprintf("Permuter Type = %d\n", word->best_choice->permuter ());
    tprintf("Certainty: %f     Rating: %f\n",
      word->best_choice->certainty (), word->best_choice->rating ());
    tprintf("Dict word: %d\n", dict_word(*(word->best_choice)));
  }

  flip_hyphens(word);
  check_debug_pt(word, 20);
}
}  // namespace tesseract


void reject_blanks(WERD_RES *word) {
  int16_t i;
  int16_t offset;

  for (i = 0, offset = 0; word->best_choice->unichar_string()[offset] != '\0';
       offset += word->best_choice->unichar_lengths()[i], i += 1) {
    if (word->best_choice->unichar_string()[offset] == ' ')
                                 //rej unrecognised blobs
      word->reject_map[i].setrej_tess_failure ();
  }
}

namespace tesseract {
void Tesseract::reject_I_1_L(WERD_RES *word) {
  int16_t i;
  int16_t offset;

  for (i = 0, offset = 0; word->best_choice->unichar_string()[offset] != '\0';
       offset += word->best_choice->unichar_lengths()[i], i += 1) {
    if (STRING (conflict_set_I_l_1).
    contains (word->best_choice->unichar_string()[offset])) {
                                 //rej 1Il conflict
      word->reject_map[i].setrej_1Il_conflict ();
    }
  }
}
}  // namespace tesseract


void reject_poor_matches(WERD_RES *word) {
  float threshold = compute_reject_threshold(word->best_choice);
  for (int i = 0; i < word->best_choice->length(); ++i) {
    if (word->best_choice->unichar_id(i) == UNICHAR_SPACE)
      word->reject_map[i].setrej_tess_failure();
    else if (word->best_choice->certainty(i) < threshold)
      word->reject_map[i].setrej_poor_match();
  }
}


/**********************************************************************
 * compute_reject_threshold
 *
 * Set a rejection threshold for this word.
 * Initially this is a trivial function which looks for the largest
 * gap in the certainty value.
 **********************************************************************/

float compute_reject_threshold(WERD_CHOICE* word) {
  float threshold;               // rejection threshold
  float bestgap = 0.0f;          // biggest gap
  float gapstart;                // bottom of gap

  int blob_count = word->length();
  GenericVector<float> ratings;
  ratings.resize_no_init(blob_count);
  for (int i = 0; i < blob_count; ++i) {
    ratings[i] = word->certainty(i);
  }
  ratings.sort();
  gapstart = ratings[0] - 1;     // all reject if none better
  if (blob_count >= 3) {
    for (int index = 0; index < blob_count - 1; index++) {
      if (ratings[index + 1] - ratings[index] > bestgap) {
        bestgap = ratings[index + 1] - ratings[index];
        // find biggest
        gapstart = ratings[index];
      }
    }
  }
  threshold = gapstart + bestgap / 2;

  return threshold;
}


/*************************************************************************
 * reject_edge_blobs()
 *
 * If the word is perilously close to the edge of the image, reject those blobs
 * in the word which are too close to the edge as they could be clipped.
 *************************************************************************/
namespace tesseract {
void Tesseract::reject_edge_blobs(WERD_RES *word) {
  TBOX word_box = word->word->bounding_box();
  // Use the box_word as it is already denormed back to image coordinates.
  int blobcount = word->box_word->length();

  if (word_box.left() < tessedit_image_border ||
      word_box.bottom() < tessedit_image_border ||
      word_box.right() + tessedit_image_border > ImageWidth() - 1 ||
      word_box.top() + tessedit_image_border > ImageHeight() - 1) {
    ASSERT_HOST(word->reject_map.length() == blobcount);
    for (int blobindex = 0; blobindex < blobcount; blobindex++) {
      TBOX blob_box = word->box_word->BlobBox(blobindex);
      if (blob_box.left() < tessedit_image_border ||
          blob_box.bottom() < tessedit_image_border ||
          blob_box.right() + tessedit_image_border > ImageWidth() - 1 ||
          blob_box.top() + tessedit_image_border > ImageHeight() - 1) {
        word->reject_map[blobindex].setrej_edge_char();
        // Close to edge
      }
    }
  }
}

/**********************************************************************
 * one_ell_conflict()
 *
 * Identify words where there is a potential I/l/1 error.
 * - A bundle of contextual heuristics!
 **********************************************************************/
bool Tesseract::one_ell_conflict(WERD_RES* word_res, bool update_map) {
  const char *word;
  const char *lengths;
  int16_t word_len;                //its length
  int16_t first_alphanum_index_;
  int16_t first_alphanum_offset_;
  int16_t i;
  int16_t offset;
  bool non_conflict_set_char;   //non conf set a/n?
  bool conflict = false;
  bool allow_1s;
  ACCEPTABLE_WERD_TYPE word_type;
  bool dict_perm_type;
  bool dict_word_ok;
  int dict_word_type;

  word = word_res->best_choice->unichar_string().string ();
  lengths = word_res->best_choice->unichar_lengths().string();
  word_len = strlen(lengths);
  /*
    If there are no occurrences of the conflict set characters then the word
    is OK.
  */
  if (strpbrk(word, conflict_set_I_l_1.string ()) == nullptr)
    return false;

  /*
    There is a conflict if there are NO other (confirmed) alphanumerics apart
    from those in the conflict set.
  */

  for (i = 0, offset = 0, non_conflict_set_char = false;
       (i < word_len) && !non_conflict_set_char; offset += lengths[i++])
    non_conflict_set_char =
        (word_res->uch_set->get_isalpha(word + offset, lengths[i]) ||
            word_res->uch_set->get_isdigit(word + offset, lengths[i])) &&
        !STRING (conflict_set_I_l_1).contains (word[offset]);
  if (!non_conflict_set_char) {
    if (update_map)
      reject_I_1_L(word_res);
    return true;
  }

  /*
    If the word is accepted by a dawg permuter, and the first alpha character
    is "I" or "l", check to see if the alternative is also a dawg word. If it
    is, then there is a potential error otherwise the word is ok.
  */

  dict_perm_type = (word_res->best_choice->permuter () == SYSTEM_DAWG_PERM) ||
    (word_res->best_choice->permuter () == USER_DAWG_PERM) ||
    (rej_trust_doc_dawg &&
    (word_res->best_choice->permuter () == DOC_DAWG_PERM)) ||
    (word_res->best_choice->permuter () == FREQ_DAWG_PERM);
  dict_word_type = dict_word(*(word_res->best_choice));
  dict_word_ok = (dict_word_type > 0) &&
    (rej_trust_doc_dawg || (dict_word_type != DOC_DAWG_PERM));

  if ((rej_1Il_use_dict_word && dict_word_ok) ||
    (rej_1Il_trust_permuter_type && dict_perm_type) ||
  (dict_perm_type && dict_word_ok)) {
    first_alphanum_index_ = first_alphanum_index (word, lengths);
    first_alphanum_offset_ = first_alphanum_offset (word, lengths);
    if (lengths[first_alphanum_index_] == 1 &&
        word[first_alphanum_offset_] == 'I') {
      word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'l';
      if (safe_dict_word(word_res) > 0) {
        word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'I';
        if (update_map)
          word_res->reject_map[first_alphanum_index_].
            setrej_1Il_conflict();
        return true;
      }
      else {
        word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'I';
        return false;
      }
    }

    if (lengths[first_alphanum_index_] == 1 &&
        word[first_alphanum_offset_] == 'l') {
      word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'I';
      if (safe_dict_word(word_res) > 0) {
        word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'l';
        if (update_map)
          word_res->reject_map[first_alphanum_index_].
            setrej_1Il_conflict();
        return true;
      }
      else {
        word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'l';
        return false;
      }
    }
    return false;
  }

  /*
    NEW 1Il code. The old code relied on permuter types too much. In fact,
    tess will use TOP_CHOICE permute for good things like "palette".
    In this code the string is examined independently to see if it looks like
    a well formed word.
  */

  /*
    REGARDLESS OF PERMUTER, see if flipping a leading I/l generates a
    dictionary word.
  */
  first_alphanum_index_ = first_alphanum_index (word, lengths);
  first_alphanum_offset_ = first_alphanum_offset (word, lengths);
  if (lengths[first_alphanum_index_] == 1 &&
      word[first_alphanum_offset_] == 'l') {
    word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'I';
    if (safe_dict_word(word_res) > 0)
      return false;
    else
      word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'l';
  }
  else if (lengths[first_alphanum_index_] == 1 &&
           word[first_alphanum_offset_] == 'I') {
    word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'l';
    if (safe_dict_word(word_res) > 0)
      return false;
    else
      word_res->best_choice->unichar_string()[first_alphanum_offset_] = 'I';
  }
  /*
    For strings containing digits:
      If there are no alphas OR the numeric permuter liked the word,
        reject any non 1 conflict chs
      Else reject all conflict chs
  */
  if (word_contains_non_1_digit (word, lengths)) {
    allow_1s = (alpha_count (word, lengths) == 0) ||
      (word_res->best_choice->permuter () == NUMBER_PERM);

    int16_t offset;
    conflict = false;
    for (i = 0, offset = 0; word[offset] != '\0';
         offset += word_res->best_choice->unichar_lengths()[i++]) {
      if ((!allow_1s || (word[offset] != '1')) &&
      STRING (conflict_set_I_l_1).contains (word[offset])) {
        if (update_map)
          word_res->reject_map[i].setrej_1Il_conflict ();
        conflict = true;
      }
    }
    return conflict;
  }
  /*
    For anything else. See if it conforms to an acceptable word type. If so,
    treat accordingly.
  */
  word_type = acceptable_word_string(*word_res->uch_set, word, lengths);
  if ((word_type == AC_LOWER_CASE) || (word_type == AC_INITIAL_CAP)) {
    first_alphanum_index_ = first_alphanum_index (word, lengths);
    first_alphanum_offset_ = first_alphanum_offset (word, lengths);
    if (STRING (conflict_set_I_l_1).contains (word[first_alphanum_offset_])) {
      if (update_map)
        word_res->reject_map[first_alphanum_index_].
            setrej_1Il_conflict ();
      return true;
    }
    else
      return false;
  }
  else if (word_type == AC_UPPER_CASE) {
    return false;
  }
  else {
    if (update_map)
      reject_I_1_L(word_res);
    return true;
  }
}


int16_t Tesseract::first_alphanum_index(const char *word,
                                      const char *word_lengths) {
  int16_t i;
  int16_t offset;

  for (i = 0, offset = 0; word[offset] != '\0'; offset += word_lengths[i++]) {
    if (unicharset.get_isalpha(word + offset, word_lengths[i]) ||
        unicharset.get_isdigit(word + offset, word_lengths[i]))
      return i;
  }
  return -1;
}

int16_t Tesseract::first_alphanum_offset(const char *word,
                                       const char *word_lengths) {
  int16_t i;
  int16_t offset;

  for (i = 0, offset = 0; word[offset] != '\0'; offset += word_lengths[i++]) {
    if (unicharset.get_isalpha(word + offset, word_lengths[i]) ||
        unicharset.get_isdigit(word + offset, word_lengths[i]))
      return offset;
  }
  return -1;
}

int16_t Tesseract::alpha_count(const char *word,
                             const char *word_lengths) {
  int16_t i;
  int16_t offset;
  int16_t count = 0;

  for (i = 0, offset = 0; word[offset] != '\0'; offset += word_lengths[i++]) {
    if (unicharset.get_isalpha (word + offset, word_lengths[i]))
      count++;
  }
  return count;
}


bool Tesseract::word_contains_non_1_digit(const char* word,
                                          const char* word_lengths) {
  int16_t i;
  int16_t offset;

  for (i = 0, offset = 0; word[offset] != '\0'; offset += word_lengths[i++]) {
    if (unicharset.get_isdigit (word + offset, word_lengths[i]) &&
        (word_lengths[i] != 1 || word[offset] != '1'))
      return true;
  }
  return false;
}

/*************************************************************************
 * dont_allow_1Il()
 * Don't unreject LONE accepted 1Il conflict set chars
 *************************************************************************/
void Tesseract::dont_allow_1Il(WERD_RES *word) {
  int i = 0;
  int offset;
  int word_len = word->reject_map.length();
  const char *s = word->best_choice->unichar_string().string();
  const char *lengths = word->best_choice->unichar_lengths().string();
  bool accepted_1Il = false;

  for (i = 0, offset = 0; i < word_len;
       offset += word->best_choice->unichar_lengths()[i++]) {
    if (word->reject_map[i].accepted()) {
      if (STRING(conflict_set_I_l_1).contains(s[offset])) {
        accepted_1Il = true;
      } else {
        if (word->uch_set->get_isalpha(s + offset, lengths[i]) ||
            word->uch_set->get_isdigit(s + offset, lengths[i]))
          return;                // >=1 non 1Il ch accepted
      }
    }
  }
  if (!accepted_1Il)
    return;                      //Nothing to worry about

  for (i = 0, offset = 0; i < word_len;
       offset += word->best_choice->unichar_lengths()[i++]) {
    if (STRING(conflict_set_I_l_1).contains(s[offset]) &&
      word->reject_map[i].accepted())
      word->reject_map[i].setrej_postNN_1Il();
  }
}


int16_t Tesseract::count_alphanums(WERD_RES *word_res) {
  int count = 0;
  const WERD_CHOICE *best_choice = word_res->best_choice;
  for (int i = 0; i < word_res->reject_map.length(); ++i) {
    if ((word_res->reject_map[i].accepted()) &&
        (word_res->uch_set->get_isalpha(best_choice->unichar_id(i)) ||
            word_res->uch_set->get_isdigit(best_choice->unichar_id(i)))) {
      count++;
    }
  }
  return count;
}


// reject all if most rejected.
void Tesseract::reject_mostly_rejects(WERD_RES *word) {
  /* Reject the whole of the word if the fraction of rejects exceeds a limit */

  if ((float) word->reject_map.reject_count() / word->reject_map.length() >=
    rej_whole_of_mostly_reject_word_fract)
    word->reject_map.rej_word_mostly_rej();
}


bool Tesseract::repeated_nonalphanum_wd(WERD_RES* word, ROW* row) {
  int16_t char_quality;
  int16_t accepted_char_quality;

  if (word->best_choice->unichar_lengths().length() <= 1)
    return false;

  if (!STRING(ok_repeated_ch_non_alphanum_wds).
    contains(word->best_choice->unichar_string()[0]))
    return false;

  UNICHAR_ID uch_id = word->best_choice->unichar_id(0);
  for (int i = 1; i < word->best_choice->length(); ++i) {
    if (word->best_choice->unichar_id(i) != uch_id) return false;
  }

  word_char_quality(word, row, &char_quality, &accepted_char_quality);

  if ((word->best_choice->unichar_lengths().length () == char_quality) &&
    (char_quality == accepted_char_quality))
    return true;
  else
    return false;
}

int16_t Tesseract::safe_dict_word(const WERD_RES *werd_res) {
  const WERD_CHOICE &word = *werd_res->best_choice;
  int dict_word_type = werd_res->tesseract->dict_word(word);
  return dict_word_type == DOC_DAWG_PERM ? 0 : dict_word_type;
}

// Note: After running this function word_res->ratings
// might not contain the right BLOB_CHOICE corresponding to each character
// in word_res->best_choice.
void Tesseract::flip_hyphens(WERD_RES *word_res) {
  WERD_CHOICE *best_choice = word_res->best_choice;
  int i;
  int prev_right = -9999;
  int next_left;
  TBOX out_box;
  float aspect_ratio;

  if (tessedit_lower_flip_hyphen <= 1)
    return;

  int num_blobs = word_res->rebuild_word->NumBlobs();
  UNICHAR_ID unichar_dash = word_res->uch_set->unichar_to_id("-");
  for (i = 0; i < best_choice->length() && i < num_blobs; ++i) {
    TBLOB* blob = word_res->rebuild_word->blobs[i];
    out_box = blob->bounding_box();
    if (i + 1 == num_blobs)
      next_left = 9999;
    else
      next_left = word_res->rebuild_word->blobs[i + 1]->bounding_box().left();
    // Don't touch small or touching blobs - it is too dangerous.
    if ((out_box.width() > 8 * word_res->denorm.x_scale()) &&
        (out_box.left() > prev_right) && (out_box.right() < next_left)) {
      aspect_ratio = out_box.width() / (float) out_box.height();
      if (word_res->uch_set->eq(best_choice->unichar_id(i), ".")) {
        if (aspect_ratio >= tessedit_upper_flip_hyphen &&
            word_res->uch_set->contains_unichar_id(unichar_dash) &&
            word_res->uch_set->get_enabled(unichar_dash)) {
          /* Certain HYPHEN */
          best_choice->set_unichar_id(unichar_dash, i);
          if (word_res->reject_map[i].rejected())
            word_res->reject_map[i].setrej_hyphen_accept();
        }
        if ((aspect_ratio > tessedit_lower_flip_hyphen) &&
          word_res->reject_map[i].accepted())
                                 //Suspected HYPHEN
          word_res->reject_map[i].setrej_hyphen ();
      }
      else if (best_choice->unichar_id(i) == unichar_dash) {
        if ((aspect_ratio >= tessedit_upper_flip_hyphen) &&
          (word_res->reject_map[i].rejected()))
          word_res->reject_map[i].setrej_hyphen_accept();
        //Certain HYPHEN

        if ((aspect_ratio <= tessedit_lower_flip_hyphen) &&
          (word_res->reject_map[i].accepted()))
                                 //Suspected HYPHEN
          word_res->reject_map[i].setrej_hyphen();
      }
    }
    prev_right = out_box.right();
  }
}

// Note: After running this function word_res->ratings
// might not contain the right BLOB_CHOICE corresponding to each character
// in word_res->best_choice.
void Tesseract::flip_0O(WERD_RES *word_res) {
  WERD_CHOICE *best_choice = word_res->best_choice;
  int i;
  TBOX out_box;

  if (!tessedit_flip_0O)
    return;

  int num_blobs = word_res->rebuild_word->NumBlobs();
  for (i = 0; i < best_choice->length() && i < num_blobs; ++i) {
    TBLOB* blob = word_res->rebuild_word->blobs[i];
    if (word_res->uch_set->get_isupper(best_choice->unichar_id(i)) ||
        word_res->uch_set->get_isdigit(best_choice->unichar_id(i))) {
      out_box = blob->bounding_box();
      if ((out_box.top() < kBlnBaselineOffset + kBlnXHeight) ||
        (out_box.bottom() > kBlnBaselineOffset + kBlnXHeight / 4))
        return;                  //Beware words with sub/superscripts
    }
  }
  UNICHAR_ID unichar_0 = word_res->uch_set->unichar_to_id("0");
  UNICHAR_ID unichar_O = word_res->uch_set->unichar_to_id("O");
  if (unichar_0 == INVALID_UNICHAR_ID ||
      !word_res->uch_set->get_enabled(unichar_0) ||
      unichar_O == INVALID_UNICHAR_ID ||
      !word_res->uch_set->get_enabled(unichar_O)) {
    return;  // 0 or O are not present/enabled in unicharset
  }
  for (i = 1; i < best_choice->length(); ++i) {
    if (best_choice->unichar_id(i) == unichar_0 ||
        best_choice->unichar_id(i) == unichar_O) {
      /* A0A */
      if ((i+1) < best_choice->length() &&
          non_O_upper(*word_res->uch_set, best_choice->unichar_id(i-1)) &&
          non_O_upper(*word_res->uch_set, best_choice->unichar_id(i+1))) {
        best_choice->set_unichar_id(unichar_O, i);
      }
      /* A00A */
      if (non_O_upper(*word_res->uch_set, best_choice->unichar_id(i-1)) &&
          (i+1) < best_choice->length() &&
          (best_choice->unichar_id(i+1) == unichar_0 ||
           best_choice->unichar_id(i+1) == unichar_O) &&
          (i+2) < best_choice->length() &&
          non_O_upper(*word_res->uch_set, best_choice->unichar_id(i+2))) {
        best_choice->set_unichar_id(unichar_O, i);
        i++;
      }
      /* AA0<non digit or end of word> */
      if ((i > 1) &&
          non_O_upper(*word_res->uch_set, best_choice->unichar_id(i-2)) &&
          non_O_upper(*word_res->uch_set, best_choice->unichar_id(i-1)) &&
          (((i+1) < best_choice->length() &&
            !word_res->uch_set->get_isdigit(best_choice->unichar_id(i+1)) &&
            !word_res->uch_set->eq(best_choice->unichar_id(i+1), "l") &&
            !word_res->uch_set->eq(best_choice->unichar_id(i+1), "I")) ||
           (i == best_choice->length() - 1))) {
        best_choice->set_unichar_id(unichar_O, i);
      }
      /* 9O9 */
      if (non_0_digit(*word_res->uch_set, best_choice->unichar_id(i-1)) &&
          (i+1) < best_choice->length() &&
          non_0_digit(*word_res->uch_set, best_choice->unichar_id(i+1))) {
        best_choice->set_unichar_id(unichar_0, i);
      }
      /* 9OOO */
      if (non_0_digit(*word_res->uch_set, best_choice->unichar_id(i-1)) &&
          (i+2) < best_choice->length() &&
          (best_choice->unichar_id(i+1) == unichar_0 ||
           best_choice->unichar_id(i+1) == unichar_O) &&
          (best_choice->unichar_id(i+2) == unichar_0 ||
           best_choice->unichar_id(i+2) == unichar_O)) {
        best_choice->set_unichar_id(unichar_0, i);
        best_choice->set_unichar_id(unichar_0, i+1);
        best_choice->set_unichar_id(unichar_0, i+2);
        i += 2;
      }
      /* 9OO<non upper> */
      if (non_0_digit(*word_res->uch_set, best_choice->unichar_id(i-1)) &&
          (i+2) < best_choice->length() &&
          (best_choice->unichar_id(i+1) == unichar_0 ||
          best_choice->unichar_id(i+1) == unichar_O) &&
          !word_res->uch_set->get_isupper(best_choice->unichar_id(i+2))) {
        best_choice->set_unichar_id(unichar_0, i);
        best_choice->set_unichar_id(unichar_0, i+1);
        i++;
      }
      /* 9O<non upper> */
      if (non_0_digit(*word_res->uch_set, best_choice->unichar_id(i-1)) &&
          (i+1) < best_choice->length() &&
          !word_res->uch_set->get_isupper(best_choice->unichar_id(i+1))) {
        best_choice->set_unichar_id(unichar_0, i);
      }
      /* 9[.,]OOO.. */
      if ((i > 1) &&
          (word_res->uch_set->eq(best_choice->unichar_id(i-1), ".") ||
              word_res->uch_set->eq(best_choice->unichar_id(i-1), ",")) &&
          (word_res->uch_set->get_isdigit(best_choice->unichar_id(i-2)) ||
           best_choice->unichar_id(i-2) == unichar_O)) {
        if (best_choice->unichar_id(i-2) == unichar_O) {
          best_choice->set_unichar_id(unichar_0, i-2);
        }
        while (i < best_choice->length() &&
               (best_choice->unichar_id(i) == unichar_O ||
                best_choice->unichar_id(i) == unichar_0)) {
          best_choice->set_unichar_id(unichar_0, i);
          i++;
        }
        i--;
      }
    }
  }
}

bool Tesseract::non_O_upper(const UNICHARSET& ch_set, UNICHAR_ID unichar_id) {
  return ch_set.get_isupper(unichar_id) && !ch_set.eq(unichar_id, "O");
}

bool Tesseract::non_0_digit(const UNICHARSET& ch_set, UNICHAR_ID unichar_id) {
  return ch_set.get_isdigit(unichar_id) && !ch_set.eq(unichar_id, "0");
}
}  // namespace tesseract

#endif  // def DISABLED_LEGACY_ENGINE
