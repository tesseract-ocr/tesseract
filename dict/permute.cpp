/* -*-C-*-
 ********************************************************************************
 *
 * File:        permute.c  (Formerly permute.c)
 * Description:  Choose OCR text given character-probability maps
 *               for sequences of glyph fragments and a dictionary provided as
 *               a Dual Acyclic Word Graph.
 *               In this file, "permute" should be read "combine."
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Sep 22 14:05:51 1989
 * Modified:     Thu Jan  3 16:38:46 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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
 *********************************************************************************/
/*----------------------------------------------------------------------
            I n c l u d e s
---------------------------------------------------------------------*/

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#pragma warning(disable:4800)  // int/bool warnings
#endif

#include <assert.h>
#include <math.h>

#include "const.h"

#include "permute.h"

#include "callcpp.h"
#include "ccutil.h"
#include "dict.h"
#include "freelist.h"
#include "helpers.h"
#include "image.h"
#include "globals.h"
#include "ndminx.h"
#include "ratngs.h"
#include "stopper.h"
#include "tprintf.h"
#include "trie.h"
#include "params.h"
#include "unicharset.h"


namespace tesseract {

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/

/**
 * get_best_delete_other
 *
 * Returns the best of two choices and deletes the other (worse) choice.
 * A choice is better if it has a non-empty string and has a lower
 * rating than the other choice. If the ratings are the same,
 * choice2 is preferred over choice1.
 */
WERD_CHOICE *get_best_delete_other(WERD_CHOICE *choice1,
                                   WERD_CHOICE *choice2) {
  if (!choice1) return choice2;
  if (!choice2) return choice1;
  if (choice1->rating() < choice2->rating() || choice2->length() == 0) {
    delete choice2;
    return choice1;
  } else {
    delete choice1;
    return choice2;
  }
}

/**
 * Returns the n-th choice in the given blob_list (top-K choices).
 * If n > K, the last choice is returned.
 */
BLOB_CHOICE* get_nth_choice(BLOB_CHOICE_LIST* blob_list, int n) {
  BLOB_CHOICE_IT c_it(blob_list);
  while (n-- > 0 && !c_it.at_last())
    c_it.forward();
  return c_it.data();
}

/** Returns the top choice char id.  A helper function to make code cleaner. */
UNICHAR_ID get_top_choice_uid(BLOB_CHOICE_LIST *blob_list) {
  if (!blob_list) return INVALID_UNICHAR_ID;
  BLOB_CHOICE_IT blob_choice_it(blob_list);
  return (blob_choice_it.data()) ? blob_choice_it.data()->unichar_id()
                                 : INVALID_UNICHAR_ID;
}

/**
 * Returns the rank (starting at 0) of a given unichar ID in the char
 * choice list, or -1 if not found.
 */
int find_choice_by_uid(BLOB_CHOICE_LIST *blob_list, UNICHAR_ID target_uid) {
  BLOB_CHOICE_IT c_it(blob_list);
  int pos = 0;
  while (1) {
    if (c_it.data()->unichar_id() == target_uid) return pos;
    if (c_it.at_last()) break;
    c_it.forward();
    pos++;
  }
  return -1;
}

/**
 * Returns a WERD formed by taking the specified position (nth choice) string
 * from char_choices starting at the given position.
 * For example, if start_pos=2, pos_str="0121" will form a word using the
 * 1st choice of char 3, 2nd choice of char 4, 3rd choice of char 5, 2nd choice
 * of char 6.  If n > number of choice, the closest (last) one is used.
 */
WERD_CHOICE* get_choice_from_posstr(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                                    int start_pos,
                                    const char* pos_str,
                                    float *certainties) {
  int pos_str_len = strlen(pos_str);
  WERD_CHOICE* wchoice = new WERD_CHOICE();
  if (start_pos + pos_str_len > char_choices.length()) {
    wchoice->make_bad();
    return wchoice;
  }
  for (int x = 0; x < pos_str_len; x++) {
    int pos = pos_str[x]-'0';
    if (pos < 0) pos = 0;   // use the top choice by default, eg. '.'
    if (pos >= 10)
      tprintf("PosStr[%d](%d)=%c  %d\n", x, pos_str_len, pos_str[x], pos);
    ASSERT_HOST(pos < 10);
    BLOB_CHOICE* blob_it = get_nth_choice(char_choices.get(start_pos+x), pos);
    wchoice->set_permuter(NO_PERM);
    wchoice->append_unichar_id(blob_it->unichar_id(), 1,
                               blob_it->rating(),
                               blob_it->certainty());
    if (certainties != NULL) certainties[x] = blob_it->certainty();
  }
  return wchoice;
}

/**
 * Given a WERD_CHOICE, find the corresponding position string from
 * char_choices.  Pos_str must have been allocated already.
 * This is the reverse of get_choice_from_posstr.
 */
void get_posstr_from_choice(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                            WERD_CHOICE* word_choice,
                            int start_pos,
                            char* pos_str) {
  for (int i = 0; i < word_choice->length(); i++) {
    UNICHAR_ID target_id = word_choice->unichar_id(i);
    BLOB_CHOICE_LIST* blob_choice_list = char_choices.get(start_pos + i);
    int pos = find_choice_by_uid(blob_choice_list, target_id);
    if (pos < 0) pos = 0;
    pos_str[i] = pos + '0';
  }
  pos_str[word_choice->length()] = '\0';
}

/**
 * Iterate through all the character choices (for a single blob) and
 * return the first that matches the given type, which is one of 'aA0px*',
 * for lower, upper, digit, punctuation, other, and 'any', respectively.
 * If not match is found, a NULL is returned.
 */
BLOB_CHOICE* find_choice_by_type(
    BLOB_CHOICE_LIST *blob_choices,
    char target_type,
    const UNICHARSET &unicharset) {
  BLOB_CHOICE_IT c_it(blob_choices);
  for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
    if (c_it.data() &&
        unicharset.get_chartype(c_it.data()->unichar_id()) == target_type)
      return c_it.data();
  }
  return NULL;
}

/**
 * Iterate through all the character choices (for a single blob) and
 * return the first that matches the target script ID.  If backup_sid
 * is not 0, then a match on either the target or backup sid is allowed.
 * Note that there is no preference between a target or backup sid.
 * To search for another sid only if no target_sid matched, use
 * secondary_sid.
 * So for example, to find first Han or Common char choice, do
 *   find_choice_by_script(cchoice, han_sid, common_sid, 0);
 * To find first Han choice, but allow Common if none is found, do
 *   find_choice_by_script(cchoice, han_sid, 0, common_sid);
 */
BLOB_CHOICE* find_choice_by_script(
    BLOB_CHOICE_LIST *blob_choices,
    int target_sid,
    int backup_sid,
    int secondary_sid) {
  BLOB_CHOICE_IT c_it(blob_choices);
  for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
    bool found = false;
    if (c_it.data()->script_id() == 0) continue;
    if (c_it.data()->script_id() == target_sid) found = true;
    if (backup_sid > 0 && c_it.data()->script_id() == backup_sid) found = true;
    if (found) return c_it.data();
  }
  if (secondary_sid > 0) {
    c_it.set_to_list(blob_choices);
    for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
      if (c_it.data()->script_id() == 0) continue;
      if (c_it.data()->script_id() == secondary_sid)
        return c_it.data();
    }
  }
  return NULL;
}


PermuterState::PermuterState() {
  char_choices_ = NULL;
  adjust_factor_ = 1.0f;
  allow_collision_ = false;
  word_length_ = 0;
  debug_ = false;
}

void PermuterState::Init(const BLOB_CHOICE_LIST_VECTOR& char_choices,
                         const UNICHARSET& unicharset,
                         float default_bias,
                         bool debug) {
  ASSERT_HOST(char_choices.length() < MAX_PERM_LENGTH);
  char_choices_ = &char_choices;
  word_length_ = char_choices.length();
  for (int i = 0; i < word_length_; ++i)
    perm_state_[i] = kPosFree;
  perm_state_[word_length_] = '\0';
  // Skip fragment choice and the mark positions so they remain unchanged.
  for (int i = 0; i < word_length_; ++i) {
    UNICHAR_ID unichar_id = get_top_choice_uid(char_choices.get(i));
    if (unicharset.get_fragment(unichar_id) != NULL)
      perm_state_[i] = '1';
  }
  adjust_factor_ = default_bias;
  allow_collision_ = false;
  debug_ = debug;
}

// Promote char positions specified in pos_str with given weight.
// For example, AddPreference(5, "234", 0.95f)
// would promote the 3rd, 4th and 5th choice for character 5, 6, 7
// (starting at 0) in the word with a rating adjustment of 0.95.
void PermuterState::AddPreference(int start_pos, char* pos_str, float weight) {
  ASSERT_HOST(char_choices_ != NULL);
  ASSERT_HOST(start_pos + strlen(pos_str) - 1 < word_length_);
  if (debug_) {
    tprintf("Copy over %s -> %s @ %d ", pos_str, perm_state_, start_pos);
  }
  // copy over preferred position without the terminating null
  if (!allow_collision_) {
    int len = strlen(pos_str);
    for (int i = 0; i < len; ++i)
      if (position_marked(start_pos + i)) return;
  }
  strncpy(&perm_state_[start_pos], pos_str, strlen(pos_str));
  adjust_factor_ *= weight;
  if (debug_) tprintf("==> %s %f\n", perm_state_, adjust_factor_);
}

// Promote the input blob_choice at specified position with given weight.
void PermuterState::AddPreference(int char_pos, BLOB_CHOICE* blob_choice,
                                  float weight) {
  ASSERT_HOST(char_choices_ != NULL);
  ASSERT_HOST(char_pos < word_length_);
  // avoid collision (but this doesn't work if the first choice is favored!
  if (!allow_collision_ && position_marked(char_pos)) return;

  if (debug_) {
    tprintf("Set UID %d -> %s @ %d ",
            blob_choice->unichar_id(), perm_state_, char_pos);
  }
  int pos = find_choice_by_uid(char_choices_->get(char_pos),
                               blob_choice->unichar_id());
  perm_state_[char_pos] = pos + '0';
  adjust_factor_ *= weight;
  if (debug_) tprintf("==> %s %f\n", perm_state_, adjust_factor_);
}

// Get the best word permutation so far.  Caller should destroy WERD_CHOICE.
WERD_CHOICE* PermuterState::GetPermutedWord(float *certainties,
                                            float *adjust_factor) {
  ASSERT_HOST(char_choices_ != NULL);
  WERD_CHOICE *word_choice = get_choice_from_posstr(*char_choices_,
                                                    0, perm_state_,
                                                    certainties);
  float rating = word_choice->rating() * adjust_factor_;
  word_choice->set_rating(rating);
  *adjust_factor = adjust_factor_;
  return word_choice;
}


/**********************************************************************
 * permute_all
 *
 * Permute all the characters together using all of the different types
 * of permuters/selectors available.  Each of the characters must have
 * a non-NULL choice list.
 *
 * Note: order of applying permuters does matter, since the latter
 * permuter will be recorded if the resulting word ratings are the same.
 *
 * Note: the function assumes that best_choice and raw_choice are not NULL.
 *
 * Note: Since permuter_all maybe called recursively (through permuter_
 * compound_words), there must be a separate instance of permuter_state
 * for each invocation.
 **********************************************************************/
WERD_CHOICE *Dict::permute_all(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                               const WERD_CHOICE *best_choice,
                               WERD_CHOICE *raw_choice) {
  WERD_CHOICE *result1 = NULL;
  WERD_CHOICE *result2 = NULL;
  BOOL8 any_alpha;
  float top_choice_rating_limit = best_choice->rating();
  int word_script_id = get_top_word_script(char_choices, getUnicharset());

  PermuterState permuter_state;
  if (getUnicharset().han_sid() != getUnicharset().null_sid() &&
      word_script_id == getUnicharset().han_sid()) {
    permuter_state.Init(char_choices, getUnicharset(), 1.0f, permute_debug);

    result1 = get_top_choice_word(char_choices);

    // Note that we no longer need the returned word from these permuters,
    // except to delete the memory.  The word choice from all permutations
    // is returned by permuter_state.GetpermutedWord() at the end.
    if (permute_fixed_length_dawg) {
      result2 = permute_fixed_length_words(char_choices, &permuter_state);
      delete result2;
    }
    if (permute_chartype_word) {
      result2 = permute_chartype_words(char_choices, &permuter_state);
      delete result2;
    }
    if (permute_script_word) {
      result2 = permute_script_words(char_choices, &permuter_state);
      delete result2;
    }

    float certainties[MAX_PERM_LENGTH];
    float adjust_factor;
    result2 = permuter_state.GetPermutedWord(certainties, &adjust_factor);
    LogNewChoice(adjust_factor, certainties, false, result2);
    result1 = get_best_delete_other(result1, result2);

    if (segment_segcost_rating) incorporate_segcost(result1);
  } else {
    result1 = permute_top_choice(char_choices, &top_choice_rating_limit,
                                 raw_choice, &any_alpha);
    if (result1 == NULL)
      return (NULL);
    if (permute_only_top)
      return result1;

    if (permute_chartype_word) {
      permuter_state.Init(char_choices, getUnicharset(),
                          segment_penalty_garbage, permute_debug);
      result2 = permute_chartype_words(char_choices, &permuter_state);
      result1 = get_best_delete_other(result1, result2);
    }

    // Permute character fragments if necessary.
    if (result1 == NULL || result1->fragment_mark()) {
      result2 = top_fragments_permute_and_select(char_choices,
                                                 top_choice_rating_limit);
      result1 = get_best_delete_other(result1, result2);
    }

    result2 = dawg_permute_and_select(char_choices, best_choice->rating());
    result1 = get_best_delete_other(result1, result2);

    result2 = permute_compound_words(char_choices, best_choice->rating());
    result1 = get_best_delete_other(result1, result2);
  }
  return result1;
}

/**
 * Incorporate segmentation cost into the word rating.  This is done
 * through a multiplier wordseg_rating_adjust_factor_ which is determined
 * in bestfirst.cpp during state evaluation.  This is not the cleanest
 * way to do this.  It would be better to reorganize the SEARCH_STATE
 * to keep track of associated states, or do the rating adjustment
 * outside the permuter in evalaute_state.
 */
void Dict::incorporate_segcost(WERD_CHOICE *word) {
  if (!word || wordseg_rating_adjust_factor_ <= 0) return;

  float old_rating = word->rating();
  float new_rating = old_rating * wordseg_rating_adjust_factor_;
  word->set_rating(new_rating);
  if (permute_debug)
    tprintf("Permute segadjust %f * %f --> %f\n",
            old_rating, wordseg_rating_adjust_factor_, new_rating);
}


/**
 * Perform search on fixed-length dictionaries within a word.
 * This is used for non-space delimited languages like CJK when a "word"
 * corresponds to a "phrase" consisted of multiple short words.
 * It iterates over every character position looking for longest matches
 * against a set of fixed-length dawgs.  Each dictionary hit is rewarded
 * with a rating bonus.
 * Note: this is very slow as it is performed on every segmentation state.
 */
WERD_CHOICE* Dict::permute_fixed_length_words(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    PermuterState *permuter_state) {
  if (permute_debug)
    print_char_choices_list("\n\nPermute FixedLength Word",
                            char_choices, getUnicharset(), false);
  WERD_CHOICE* best_choice = new WERD_CHOICE(char_choices.length());
  const int max_dict_len = max_fixed_length_dawgs_wdlen_;
  const int min_dict_len = 2;
  char posstr[256];
  int match_score = 0;
  int anchor_pos = 0;
  while (anchor_pos < char_choices.length()) {
     // search from longest phrase to shortest, stop when we find a match
     WERD_CHOICE* part_choice = NULL;
     int step = max_dict_len;
     while (step >= min_dict_len) {
       int end_pos = anchor_pos + step - 1;
       if (end_pos < char_choices.length()) {
         part_choice = dawg_permute_and_select(char_choices,
                                               200.0,   // rate limit
                                               step,
                                               anchor_pos);
         if (part_choice->length() == step) {
           if (permute_debug)
             tprintf("match found at pos=%d len=%d\n%s\n", anchor_pos, step,
                     part_choice->unichar_string().string());
           break;
         }
         delete part_choice;
         part_choice = NULL;
       }
       step--;
     }

     if (part_choice && step > 1) {   // found lexicon match
       part_choice->populate_unichars(getUnicharset());
       get_posstr_from_choice(char_choices, part_choice, anchor_pos, posstr);
       float adjust_factor = pow(0.95, 1.0 + step*2.0/char_choices.length());
       if (permuter_state)
         permuter_state->AddPreference(anchor_pos, posstr, adjust_factor);
       match_score += step - 1;   // single chars don't count
       if (permute_debug)
         tprintf("Promote word rating %d-len%d\n%s\n", anchor_pos, step,
                 part_choice->unichar_string().string());
     } else {     // no lexicon match
       step = 1;
       part_choice =
         get_choice_from_posstr(char_choices, anchor_pos, "0", NULL);
       if (permute_debug)
         tprintf("Single char %d %s\n", anchor_pos,
                 part_choice->unichar_string().string());
     }
     if (part_choice && part_choice->length() > 0)
       (*best_choice) += (*part_choice);
     if (part_choice) delete part_choice;
     anchor_pos += step;
  }

  if (match_score > 0) {
    float adjust_factor = pow(0.8,    // 1.0/segment_penalty_dict_nonword,
                              match_score * 2.0 / char_choices.length());
    float adjusted_score = best_choice->rating() * adjust_factor;
    if (permute_debug)
      tprintf("Adjusting score %f @ %d -> %f\n",
              best_choice->rating(), match_score, adjusted_score);
    best_choice->set_rating(adjusted_score);
  }
  best_choice->populate_unichars(getUnicharset());
  if (permute_debug)
    tprintf("Found Best CJK word %f: %s\n",
            best_choice->rating(), best_choice->unichar_string().string());
  return best_choice;
}


/**********************************************************************
 * Returns the dominant chartype for the word.  Only the "main" chartype
 * of each character is used, and a consistent chartype is defined by
 * the majority chartype from non-ambiguous character positions.
 * If pos_chartypes is not NULL, then the "main" chartype at each pos
 * is also returned.  The caller must allocate and deallocate the space.
 **********************************************************************/
char Dict::top_word_chartype(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                             char* pos_chartypes) {
  const UNICHARSET &unicharset = getUnicharset();
  const int hist_size = 128;   // to contain 'A','a','0','x','p'
  int chprop[hist_size];
  int x;
  for (x = 0; x < hist_size; x++) chprop[x] = 0;
  for (x = 0; x < char_choices.length(); ++x) {
    UNICHAR_ID unichar_id = get_top_choice_uid(char_choices.get(x));
    char ctype = unicharset.get_chartype(unichar_id);
    if (pos_chartypes) pos_chartypes[x] = ctype;
    if (ctype == 0 || ctype == 'p') continue;
    if (getUnicharAmbigs().OneToOneDefiniteAmbigs(unichar_id) != NULL) continue;
    chprop[ctype]++;
    if (x == 0 && ctype == 'A')  // first-cap also counts as lower
      chprop['a']++;
  }
  int max_prop = 0;
  for (x = 1; x < hist_size; x++)
    if (chprop[x] >= chprop[max_prop]) max_prop = x;
  return (chprop[max_prop] > 0) ? max_prop : 0;
}

/**********************************************************************
 * Promote consistent character type based on neighboring characters.
 * For each character position, if the top choice property is inconsistent
 * with that of the word or previous character, then its likely
 * subsitutions, as defined by DangAmbigs, will be examined and the one
 * with a matching property will be selected.
 **********************************************************************/
WERD_CHOICE* Dict::permute_chartype_words(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    PermuterState *permuter_state) {

  if (char_choices.length() >= MAX_PERM_LENGTH)
    return NULL;
  // Store main character property of top choice at every position
  char pos_chartypes[MAX_PERM_LENGTH];
  char word_type = top_word_chartype(char_choices, pos_chartypes);
  if (word_type == 0 || word_type == 'p')
    return NULL;     // skip if word type is punctuation or unknown
  if (permute_debug) {
    tprintf("\n\nPermuteCharType[%c]\n", word_type);
    print_char_choices_list("", char_choices, getUnicharset(), true);
  }

  WERD_CHOICE *current_word = new WERD_CHOICE();
  BLOB_CHOICE_IT blob_choice_it;
  const UNICHARSET& unicharset = getUnicharset();
  bool replaced = false;        // has any character choice been replaced
  int prev_unambig_type = 0;    // the last chartype of an unambiguous char
  for (int x = 0; x < char_choices.length(); ++x) {
    BLOB_CHOICE_LIST* pos_choice = char_choices.get(x);
    UNICHAR_ID unichar_id = get_top_choice_uid(pos_choice);
    if (unichar_id == 0) {
      delete current_word;
      return NULL;
    }
    blob_choice_it.set_to_list(pos_choice);
    BLOB_CHOICE *first_choice = blob_choice_it.data();
    ASSERT_HOST(first_choice != NULL);

    const UnicharIdVector* ambig_uids =
        getUnicharAmbigs().OneToOneDefiniteAmbigs(unichar_id);
    bool is_ambiguous = (ambig_uids != NULL);
    bool is_punct = unicharset.get_ispunctuation(unichar_id);
    bool is_consistent = is_punct ||
        unicharset.get_chartype(unichar_id) == prev_unambig_type ||
        unicharset.get_chartype(unichar_id) == word_type;
    bool is_fragment = getUnicharset().get_fragment(unichar_id) != NULL;
    if (permute_debug)
      tprintf("char[%d]:%s is_ambig %c   is_punct %c  is_consistent %c\n",
              x, unicharset.id_to_unichar(unichar_id),
              is_ambiguous?'T':'F', is_punct?'T':'F', is_consistent?'T':'F');

    if (is_fragment) {
      // Ignore any fragmented characters by skipping them to next choice
      // (originally first choice).
      first_choice = get_nth_choice(pos_choice, 1);
      ASSERT_HOST(first_choice != NULL);
    } else if (is_ambiguous && !is_consistent) {
      // Check every confusable blob choice where the top choice is inconsistent
      // with the character type of the previous unambiguous character.
      if (permute_debug) {
        tprintf("Checking %s r%g  PrevCharType %c\n",
                unicharset.id_to_unichar(unichar_id),
                first_choice->rating(), prev_unambig_type);
        print_ratings_list("\t", pos_choice, getUnicharset());
      }
      BLOB_CHOICE* c_it = NULL;
      if (c_it == NULL) {
        c_it = find_choice_by_type(pos_choice, word_type, unicharset);
      }

      // Prefer a character choice whose type is the same as the previous
      // unambiguous character and the confusion appears in the ambig list.
      if (c_it == NULL && prev_unambig_type > 0) {
        c_it = find_choice_by_type(pos_choice, prev_unambig_type, unicharset);
        if (c_it &&
            UnicharIdArrayUtils::find_in(*ambig_uids, c_it->unichar_id()) < 0)
          c_it = NULL;
      }

      // Otherwise, perfer a punctuation
      if (c_it == NULL) {
        c_it = find_choice_by_type(pos_choice, 'p', unicharset);
        if (c_it &&
            UnicharIdArrayUtils::find_in(*ambig_uids, c_it->unichar_id()) < 0)
          c_it = NULL;
      }

      // save any preference other than the top choice
      if (c_it != NULL) {
        if (permute_debug) {
          tprintf("Replacing %s r%g ==> %s r%g\n",
                  unicharset.id_to_unichar(unichar_id), first_choice->rating(),
                  unicharset.id_to_unichar(c_it->unichar_id()), c_it->rating());
          tprintf("\n\nPermuteCharType[%c]\n", word_type);
          print_char_choices_list("", char_choices, getUnicharset(), false);
        }
        if (permuter_state)
          permuter_state->AddPreference(x, c_it, segment_reward_chartype);
        first_choice = c_it;
        replaced = true;
      }
    } else if (!is_ambiguous && !is_punct) {
      // keep the last unambiguous character type
      prev_unambig_type = pos_chartypes[x];
    }
    current_word->append_unichar_id(first_choice->unichar_id(), 1,
                                    first_choice->rating(),
                                    first_choice->certainty());
  }
  // All permuter choices should go through adjust_non_word so the choice
  // rating would be adjusted on the same scale.
  float certainties[MAX_PERM_LENGTH + 1];
  adjust_non_word(current_word, certainties, permute_debug);
  current_word->populate_unichars(unicharset);
  if (replaced) {
    // Apply a reward multiplier on rating if an chartype permutation is made.
    float rating = current_word->rating();
    current_word->set_rating(rating * segment_reward_chartype);
    if (permute_debug)
      current_word->print("<== permute_chartype_word **");
  }
  return current_word;
}

/**
 * Try flipping characters in a word to get better script consistency.
 * Similar to how upper/lower case checking is done in top_choice_permuter,
 * this permuter tries to suggest a more script-consistent choice AND
 * modifies the rating.  So it combines both the case_ok check and
 * adjust_non_word functionality.  However, instead of penalizing an
 * inconsistent word with a > 1 multiplier, we reward the script-consistent
 * choice with a < 1 multiplier.
 */
WERD_CHOICE* Dict::permute_script_words(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    PermuterState *permuter_state) {
  if (char_choices.length() >= MAX_WERD_LENGTH)
    return NULL;

  int word_sid = get_top_word_script(char_choices, getUnicharset());
  if (word_sid == getUnicharset().null_sid())
    return NULL;

  if (permute_debug) {
    tprintf("\n\nPermuteScript %s\n",
            getUnicharset().get_script_from_script_id(word_sid));
    print_char_choices_list("", char_choices, getUnicharset(),
                            permute_debug > 1);
  }

  WERD_CHOICE *current_word = new WERD_CHOICE();
  BLOB_CHOICE_IT blob_choice_it;
  bool replaced = false;
  bool prev_is_consistent = false;
  for (int x = 0; x < char_choices.length(); ++x) {
    blob_choice_it.set_to_list(char_choices.get(x));
    BLOB_CHOICE *first_choice = blob_choice_it.data();
    if (!first_choice) {
      delete current_word;
      return NULL;
    }
    UNICHAR_ID unichar_id = first_choice->unichar_id();
    if (unichar_id == 0) {
      delete current_word;
      return NULL;
    }
    bool sid_consistent = (getUnicharset().get_script(unichar_id) == word_sid);
    bool this_is_punct = getUnicharset().get_chartype(unichar_id) == 'p';
    bool is_fragment = getUnicharset().get_fragment(unichar_id) != NULL;

    if (is_fragment) {
      // Ignore any fragmented characters by skipping them to next choice
      // (originally first choice).
      first_choice = get_nth_choice(char_choices.get(x), 1);
      ASSERT_HOST(first_choice != NULL);
    } else if (!sid_consistent && !this_is_punct && prev_is_consistent) {
      // If the previous char is CJK, we prefer a cjk over non-cjk char
      if (permute_debug) {
        tprintf("Checking %s r%g\n", getUnicharset().id_to_unichar(unichar_id),
                                     first_choice->rating());
        print_ratings_list("\t", char_choices.get(x), getUnicharset());
      }
      // prefer a script consistent choice
      BLOB_CHOICE* c_it = find_choice_by_script(char_choices.get(x),
                                                word_sid, 0, 0);
      // otherwise, prefer a punctuation
      if (c_it == NULL)
        c_it = find_choice_by_type(char_choices.get(x), 'p', getUnicharset());

      if (c_it != NULL) {
        if (permute_debug)
          tprintf("Replacing %s r%g ==> %s r%g\n",
                  getUnicharset().id_to_unichar(unichar_id),
                  first_choice->rating(),
                  getUnicharset().id_to_unichar(c_it->unichar_id()),
                  c_it->rating());
        if (permuter_state)
          permuter_state->AddPreference(x, c_it, segment_reward_script);
        first_choice = c_it;
        replaced = true;
      }
    }
    current_word->append_unichar_id(first_choice->unichar_id(), 1,
                                    first_choice->rating(),
                                    first_choice->certainty());
    prev_is_consistent = sid_consistent;
  }
  // All permuter choices should go through adjust_non_word so the choice
  // rating would be adjusted on the same scale.
  float certainties[MAX_PERM_LENGTH + 1];
  adjust_non_word(current_word, certainties, permute_debug);
  current_word->populate_unichars(getUnicharset());
  if (replaced) {
    // Apply a reward multiplier on rating if an script permutation is made.
    float rating = current_word->rating();
    current_word->set_rating(rating * segment_reward_script);
    if (permute_debug)
      current_word->print("<== permute_script_word **");
  }
  return current_word;
}

/**
 * permute_characters
 *
 * Permute these characters together according to each of the different
 * permuters that are enabled.
 * Returns true if best_choice was updated.
 */
bool Dict::permute_characters(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                              WERD_CHOICE *best_choice,
                              WERD_CHOICE *raw_choice) {
  float old_raw_choice_rating = raw_choice->rating();
  if (permute_debug) {
    tprintf("\n\n\n##### Permute_Characters #######\n");
    print_char_choices_list("\n==> Input CharChoices", char_choices,
                            getUnicharset(), segment_debug > 1);
    tprintf("\n");
  }

  if (char_choices.length() == 1 &&
      get_top_choice_uid(char_choices.get(0)) == 0) return false;
  WERD_CHOICE *this_choice = permute_all(char_choices, best_choice, raw_choice);

  if (raw_choice->rating() < old_raw_choice_rating) {
    // Populate unichars_ and unichar_lengths_ of raw_choice. This is
    // needed for various components that still work with unichars rather
    // than unichar ids (e.g. LearnWord).
    raw_choice->populate_unichars(getUnicharset());
  }
  if (this_choice && this_choice->rating() < best_choice->rating()) {
    *best_choice = *this_choice;
    // Populate unichars_ and unichar_lengths_ of best_choice. This is
    // needed for various components that still work with unichars rather
    // than unichar ids (dawg, *_ok functions, various hard-coded hacks).
    best_choice->populate_unichars(getUnicharset());

    if (permute_debug) {
      best_choice->print("\n**** Populate BestChoice");
      cprintf("populate best_choice\n\t%s\n",
              best_choice->debug_string(getUnicharset()).string());
    }
    delete this_choice;
    return true;
  }
  delete this_choice;
  return false;
}

/**
 * permute_compound_words
 *
 * Return the top choice for each character as the choice for the word.
 */
WERD_CHOICE *Dict::permute_compound_words(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    float rating_limit) {
  BLOB_CHOICE *first_choice;
  WERD_CHOICE *best_choice = NULL;
  WERD_CHOICE current_word(MAX_WERD_LENGTH);
  int first_index = 0;
  int x;
  BLOB_CHOICE_IT blob_choice_it;

  if (char_choices.length() > MAX_WERD_LENGTH) {
    WERD_CHOICE *bad_word_choice = new WERD_CHOICE();
    bad_word_choice->make_bad();
    return bad_word_choice;
  }

  UNICHAR_ID slash = getUnicharset().unichar_to_id("/");
  UNICHAR_ID dash = getUnicharset().unichar_to_id("-");
  for (x = 0; x < char_choices.length(); ++x) {
    blob_choice_it.set_to_list(char_choices.get(x));
    first_choice = blob_choice_it.data();
    if (first_choice->unichar_id() == slash ||
        first_choice->unichar_id() == dash) {
      if (x > first_index) {
        if (segment_debug)
          cprintf ("Hyphenated word found\n");
        permute_subword(char_choices, rating_limit, first_index,
                        x - 1, &current_word);
        if (current_word.rating() > rating_limit)
          break;
      }
      // Append hyphen/slash separator to current_word.
      current_word.append_unichar_id_space_allocated(
          first_choice->unichar_id(), 1,
          first_choice->rating(), first_choice->certainty());

      first_index = x + 1;  // update first_index
    }
  }

  if (first_index > 0 && first_index < x &&
      current_word.rating() <= rating_limit) {
    permute_subword(char_choices, rating_limit, first_index,
                    x - 1, &current_word);
    best_choice = new WERD_CHOICE(current_word);
    best_choice->set_permuter(COMPOUND_PERM);
  }
  return (best_choice);
}


/**
 * permute_subword
 *
 * Permute a part of a compound word this subword is bounded by hyphens
 * and the start and end of the word.  Call the standard word permute
 * function on a set of choices covering only part of the original
 * word.  When it is done reclaim the memory that was used in the
 * exercise.
 */
void Dict::permute_subword(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                           float rating_limit,
                           int start,
                           int end,
                           WERD_CHOICE *current_word) {
  int x;
  BLOB_CHOICE_LIST_VECTOR subchoices;
  WERD_CHOICE *best_choice = NULL;
  WERD_CHOICE raw_choice;
  raw_choice.make_bad();

  DisableChoiceAccum();

  for (x = start; x <= end; x++) {
    if (char_choices.get(x) != NULL) {
      subchoices += char_choices.get(x);
    }
  }

  if (!subchoices.empty()) {
    WERD_CHOICE initial_choice;
    initial_choice.make_bad();
    initial_choice.set_rating(rating_limit);

    best_choice = permute_all(subchoices, &initial_choice, &raw_choice);

    if (best_choice && best_choice->length() > 0) {
      *current_word += *best_choice;
    } else {
      current_word->set_rating(MAX_FLOAT32);
    }
  } else {
    current_word->set_rating(MAX_FLOAT32);
  }

  if (best_choice)
    delete best_choice;

  if (segment_debug && current_word->rating() < MAX_FLOAT32) {
    cprintf ("Subword permuted = %s, %5.2f, %5.2f\n\n",
             current_word->debug_string(getUnicharset()).string(),
             current_word->rating(), current_word->certainty());
  }
  current_word->populate_unichars(getUnicharset());

  EnableChoiceAccum();
}

/**
 * Return the top choice for each character as the choice for the word.
 */
WERD_CHOICE *Dict::get_top_choice_word(
    const BLOB_CHOICE_LIST_VECTOR &char_choices) {
  WERD_CHOICE *top_word = new WERD_CHOICE(MAX_PERM_LENGTH);
  float certainties[MAX_PERM_LENGTH];
  top_word->set_permuter(TOP_CHOICE_PERM);
  for (int x = 0; x < char_choices.length(); x++) {
    BLOB_CHOICE_IT blob_choice_it;
    blob_choice_it.set_to_list(char_choices.get(x));
    BLOB_CHOICE *top_choice = blob_choice_it.data();
    top_word->append_unichar_id_space_allocated(top_choice->unichar_id(), 1,
                                                top_choice->rating(),
                                                top_choice->certainty());
    certainties[x] = top_choice->certainty();
  }
  LogNewChoice(1.0, certainties, true, top_word);
  return top_word;
}

/**
 * permute_top_choice
 *
 * Return the top choice for each character as the choice for the word.
 * In addition a choice is created for the best lower and upper case
 * non-words.  In each character position the best lower (or upper) case
 * character is substituted for the best overall character.
 */
WERD_CHOICE *Dict::permute_top_choice(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    float* rating_limit,
    WERD_CHOICE *raw_choice,
    BOOL8 *any_alpha) {
  BLOB_CHOICE *first_choice;
  const char *first_char;             //first choice
  const char *second_char;            //second choice
  const char *third_char;             //third choice
  char prev_char[UNICHAR_LEN + 1];    //prev in word
  const char *next_char = "";         //next in word
  const char *next_next_char = "";    //after next next in word

  WERD_CHOICE word(MAX_PERM_LENGTH);
  word.set_permuter(TOP_CHOICE_PERM);
  WERD_CHOICE capital_word(MAX_PERM_LENGTH);
  capital_word.set_permuter(UPPER_CASE_PERM);
  WERD_CHOICE lower_word(MAX_PERM_LENGTH);
  lower_word.set_permuter(LOWER_CASE_PERM);

  int x;
  BOOL8 char_alpha;
  float first_rating = 0;

  float certainties[MAX_PERM_LENGTH + 1];
  float lower_certainties[MAX_PERM_LENGTH + 1];
  float upper_certainties[MAX_PERM_LENGTH + 1];

  BLOB_CHOICE_IT blob_choice_it;
  UNICHAR_ID temp_id;
  UNICHAR_ID unichar_id;
  UNICHAR_ID space = getUnicharset().unichar_to_id(" ");
  register const char* ch;
  register inT8 lower_done;
  register inT8 upper_done;

  prev_char[0] = '\0';

  if (any_alpha != NULL)
    *any_alpha = FALSE;

  if (char_choices.length() > MAX_PERM_LENGTH) {
    return (NULL);
  }

  for (x = 0; x < char_choices.length(); ++x) {
    if (x + 1 < char_choices.length()) {
      unichar_id = get_top_choice_uid(char_choices.get(x+1));
      next_char = unichar_id != INVALID_UNICHAR_ID ?
        getUnicharset().id_to_unichar(unichar_id) : "";
    } else {
      next_char = "";
    }

    if (x + 2 < char_choices.length()) {
      unichar_id = get_top_choice_uid(char_choices.get(x+2));
      next_next_char = unichar_id != INVALID_UNICHAR_ID ?
        getUnicharset().id_to_unichar(unichar_id) : "";
    } else {
      next_next_char = "";
    }

    blob_choice_it.set_to_list(char_choices.get(x));
    ASSERT_HOST(!blob_choice_it.empty());
    first_choice = NULL;
    for (blob_choice_it.mark_cycle_pt(); !blob_choice_it.cycled_list();
         blob_choice_it.forward()) {  // find the best non-fragment char choice
      temp_id = blob_choice_it.data()->unichar_id();
      if (!(getUnicharset().get_fragment(temp_id))) {
        first_choice = blob_choice_it.data();
        break;
      } else if (char_choices.length() > 1) {
        word.set_fragment_mark(true);
        capital_word.set_fragment_mark(true);
        lower_word.set_fragment_mark(true);
      }
    }
    if (first_choice == NULL) {
      cprintf("Permuter found only fragments for"
              " character at position %d; word=%s\n",
              x, word.debug_string(getUnicharset()).string());
    }
    ASSERT_HOST(first_choice != NULL);

    unichar_id = first_choice->unichar_id() != INVALID_UNICHAR_ID ?
      first_choice->unichar_id() : space;
    first_char = getUnicharset().id_to_unichar(unichar_id);
    first_rating = first_choice->rating();
    word.append_unichar_id_space_allocated(
        unichar_id, 1, first_choice->rating(), first_choice->certainty());
    capital_word.append_unichar_id_space_allocated(
        unichar_id, 1, first_choice->rating(), first_choice->certainty());
    lower_word.append_unichar_id_space_allocated(
        unichar_id, 1, first_choice->rating(), first_choice->certainty());

    certainties[x] = first_choice->certainty();
    lower_certainties[x] = first_choice->certainty();
    upper_certainties[x] = first_choice->certainty();

    lower_done = FALSE;
    upper_done = FALSE;
    char_alpha = FALSE;
    second_char = "";
    third_char = "";
    for (; !blob_choice_it.cycled_list(); blob_choice_it.forward()) {
      unichar_id = blob_choice_it.data()->unichar_id();
      if (getUnicharset().eq(unichar_id, "l") && !blob_choice_it.at_last() &&
          blob_choice_it.data_relative(1)->rating() == first_rating) {
        temp_id = blob_choice_it.data_relative(1)->unichar_id();
        if (getUnicharset().eq(temp_id, "1") ||
            getUnicharset().eq(temp_id, "I")) {
          second_char = getUnicharset().id_to_unichar(temp_id);
          blob_choice_it.forward();
          if (!blob_choice_it.at_last() &&
              blob_choice_it.data_relative(1)->rating() == first_rating) {
            temp_id = blob_choice_it.data_relative(1)->unichar_id();
            if (getUnicharset().eq(temp_id, "1") ||
                getUnicharset().eq(temp_id, "I")) {
              third_char = getUnicharset().id_to_unichar(temp_id);
              blob_choice_it.forward();
            }
          }
          ch = choose_il1 (first_char, second_char, third_char,
            prev_char, next_char, next_next_char);
          unichar_id = (ch != NULL && *ch != '\0') ?
            getUnicharset().unichar_to_id(ch) : INVALID_UNICHAR_ID;
          if (strcmp(ch, "l") != 0 &&
              getUnicharset().eq(word.unichar_id(x), "l")) {
            word.set_unichar_id(unichar_id, x);
            lower_word.set_unichar_id(unichar_id, x);
            capital_word.set_unichar_id(unichar_id, x);
          }
        }
      }
      if (unichar_id != INVALID_UNICHAR_ID) {
        /* Find lower case */
        if (!lower_done &&
            (getUnicharset().get_islower(unichar_id) ||
             (getUnicharset().get_isupper(unichar_id) && x == 0))) {
          lower_word.set_unichar_id(unichar_id, x);
          lower_word.set_rating(lower_word.rating() -
            first_choice->rating() + blob_choice_it.data()->rating());
          if (blob_choice_it.data()->certainty() < lower_word.certainty()) {
            lower_word.set_certainty(blob_choice_it.data()->certainty());
          }
          lower_certainties[x] = blob_choice_it.data()->certainty();
          lower_done = TRUE;
        }
        /* Find upper case */
        if (!upper_done && getUnicharset().get_isupper(unichar_id)) {
          capital_word.set_unichar_id(unichar_id, x);
          capital_word.set_rating(capital_word.rating() -
            first_choice->rating() + blob_choice_it.data()->rating());
          if (blob_choice_it.data()->certainty() < capital_word.certainty()) {
            capital_word.set_certainty(blob_choice_it.data()->certainty());
          }
          upper_certainties[x] = blob_choice_it.data()->certainty();
          upper_done = TRUE;
        }
        if (!char_alpha) {
          const CHAR_FRAGMENT *fragment =
            getUnicharset().get_fragment(unichar_id);
          temp_id = !fragment ? unichar_id :
            getUnicharset().unichar_to_id(fragment->get_unichar());
          if (getUnicharset().get_isalpha(temp_id)) {
            char_alpha = TRUE;
          }
        }
        if (lower_done && upper_done)
          break;
      }
    }
    if (char_alpha && any_alpha != NULL)
      *any_alpha = TRUE;

    if (word.rating() > bestrate_pruning_factor * *rating_limit) {
      if (permute_debug)
        tprintf("\n***** Aborting high-cost word: %g > limit %g\n",
                word.rating(), bestrate_pruning_factor * *rating_limit);
      return (NULL);
    }

    *prev_char = '\0';
    temp_id = word.unichar_id(word.length()-1);
    if (temp_id != INVALID_UNICHAR_ID) {
      strcpy(prev_char, getUnicharset().id_to_unichar(temp_id));
    }
  }

  if (word.rating() < raw_choice->rating()) {
    *raw_choice = word;
    LogNewChoice(1.0, certainties, true, raw_choice);
  }
  float rating = word.rating();
  adjust_non_word(&word, certainties, permute_debug);

  float lower_rating = lower_word.rating();
  adjust_non_word(&lower_word, lower_certainties, permute_debug);

  float upper_rating = capital_word.rating();
  adjust_non_word(&capital_word, upper_certainties, permute_debug);

  WERD_CHOICE *best_choice = &word;
  *rating_limit = rating;
  if (lower_word.rating() < best_choice->rating()) {
    best_choice = &lower_word;
    *rating_limit = lower_rating;
  }
  if (capital_word.rating() < best_choice->rating()) {
    best_choice = &capital_word;
    *rating_limit = upper_rating;
  }
  return new WERD_CHOICE(*best_choice);
}


/**
 * @name choose_il1
 *
 * Choose between the candidate il1 chars.
 * @param first_char first choice
 * @param second_char second choice
 * @param third_char third choice
 * @param prev_char prev in word
 * @param next_char next in word
 * @param next_next_char after next next in word
 */
const char* Dict::choose_il1(const char *first_char,
                             const char *second_char,
                             const char *third_char,
                             const char *prev_char,
                             const char *next_char,
                             const char *next_next_char) {
  inT32 type1;                   //1/I/l type of first choice
  inT32 type2;                   //1/I/l type of second choice
  inT32 type3;                   //1/I/l type of third choice

  int first_char_length = strlen(first_char);
  int prev_char_length = strlen(prev_char);
  int next_char_length = strlen(next_char);
  int next_next_char_length = strlen(next_next_char);

  if (*first_char == 'l' && *second_char != '\0') {
    if (*second_char == 'I'
        && (((prev_char_length != 0 &&
            getUnicharset().get_isupper (prev_char, prev_char_length)) &&
            (next_char_length == 0 ||
             !getUnicharset().get_islower (next_char, next_char_length)) &&
            (next_char_length == 0 ||
             !getUnicharset().get_isdigit (next_char, next_char_length))) ||
            ((next_char_length != 0 &&
             getUnicharset().get_isupper (next_char, next_char_length)) &&
            (prev_char_length == 0 ||
             !getUnicharset().get_islower (prev_char, prev_char_length)) &&
            (prev_char_length == 0 ||
             !getUnicharset().get_isdigit (prev_char, prev_char_length)))))
      first_char = second_char;  //override
    else if (*second_char == '1' || *third_char == '1') {
      if ((next_char_length != 0 &&
           getUnicharset().get_isdigit (next_char, next_char_length)) ||
          (prev_char_length != 0 &&
           getUnicharset().get_isdigit (prev_char, prev_char_length))
          || (*next_char == 'l' &&
          (next_next_char_length != 0 &&
           getUnicharset().get_isdigit (next_next_char,
                                        next_next_char_length)))) {
        first_char = "1";
        first_char_length = 1;
      }
      else if ((prev_char_length == 0 ||
                !getUnicharset().get_islower (prev_char, prev_char_length)) &&
               ((next_char_length == 0 ||
                 !getUnicharset().get_islower (next_char, next_char_length)) ||
                (*next_char == 's' &&
                *next_next_char == 't'))) {
        if (((*prev_char != '\'' && *prev_char != '`') || *next_char != '\0')
            && ((*next_char != '\'' && *next_char != '`')
                || *prev_char != '\0')) {
          first_char = "1";
          first_char_length = 1;
        }
      }
    }
    if (*first_char == 'l' && *next_char != '\0' &&
        (prev_char_length == 0 ||
         !getUnicharset().get_isalpha (prev_char, prev_char_length))) {
      type1 = 2;

      if (*second_char == '1')
        type2 = 0;
      else if (*second_char == 'I')
        type2 = 1;
      else if (*second_char == 'l')
        type2 = 2;
      else
        type2 = type1;

      if (*third_char == '1')
        type3 = 0;
      else if (*third_char == 'I')
        type3 = 1;
      else if (*third_char == 'l')
        type3 = 2;
      else
        type3 = type1;

#if 0
      if (bigram_counts[*next_char][type2] >
      bigram_counts[*next_char][type1]) {
        first_char = second_char;
        type1 = type2;
      }
      if (bigram_counts[*next_char][type3] >
      bigram_counts[*next_char][type1]) {
        first_char = third_char;
      }
#endif
    }
  }
  return first_char;
}

/**
 * @name fragment_state
 *
 * Given the current char choice and information about previously seen
 * fragments, determines whether adjacent character fragments are
 * present and whether they can be concatenated.
 *
 * The given prev_char_frag_info contains:
 * - fragment: if not NULL contains information about immediately
 *   preceeding fragmented character choice
 * - num_fragments: number of fragments that have been used so far
 *   to construct a character
 * - certainty: certainty of the current choice or minimum
 *   certainty of all fragments concatenated so far
 * - rating: rating of the current choice or sum of fragment
 *   ratings concatenated so far
 *
 * The output char_frag_info is filled in as follows:
 * - character: is set to be NULL if the choice is a non-matching
 *   or non-ending fragment piece; is set to unichar of the given choice
 *   if it represents a regular character or a matching ending fragment
 * - fragment,num_fragments,certainty,rating are set as described above
 *
 * @returns false if a non-matching fragment is discovered, true otherwise.
 */
bool Dict::fragment_state_okay(UNICHAR_ID curr_unichar_id,
                               float curr_rating, float curr_certainty,
                               const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                               const char *debug, int word_ending,
                               CHAR_FRAGMENT_INFO *char_frag_info) {
  const CHAR_FRAGMENT *this_fragment =
    getUnicharset().get_fragment(curr_unichar_id);
  const CHAR_FRAGMENT *prev_fragment =
    prev_char_frag_info != NULL ? prev_char_frag_info->fragment : NULL;

  // Print debug info for fragments.
  if (debug && (prev_fragment || this_fragment)) {
    cprintf("%s check fragments: choice=%s word_ending=%d\n", debug,
            getUnicharset().debug_str(curr_unichar_id).string(),
            word_ending);
    if (prev_fragment) {
      cprintf("prev_fragment %s\n", prev_fragment->to_string().string());
    }
    if (this_fragment) {
      cprintf("this_fragment %s\n", this_fragment->to_string().string());
    }
  }

  char_frag_info->unichar_id = curr_unichar_id;
  char_frag_info->fragment = this_fragment;
  char_frag_info->rating = curr_rating;
  char_frag_info->certainty = curr_certainty;
  char_frag_info->num_fragments = 1;
  if (prev_fragment && !this_fragment) {
    if (debug) tprintf("Skip choice with incomplete fragment\n");
    return false;
  }
  if (this_fragment) {
    // We are dealing with a fragment.
    char_frag_info->unichar_id = INVALID_UNICHAR_ID;
    if (prev_fragment) {
      if (!this_fragment->is_continuation_of(prev_fragment)) {
        if (debug) tprintf("Non-matching fragment piece\n");
        return false;
      }
      if (this_fragment->is_ending()) {
        char_frag_info->unichar_id =
          getUnicharset().unichar_to_id(this_fragment->get_unichar());
        char_frag_info->fragment = NULL;
        if (debug) {
          tprintf("Built character %s from fragments\n",
                  getUnicharset().debug_str(
                      char_frag_info->unichar_id).string());
        }
      } else {
        if (debug) tprintf("Record fragment continuation\n");
        char_frag_info->fragment = this_fragment;
      }
      // Update certainty and rating.
      char_frag_info->rating =
        prev_char_frag_info->rating + curr_rating;
      char_frag_info->num_fragments = prev_char_frag_info->num_fragments + 1;
      char_frag_info->certainty =
        MIN(curr_certainty, prev_char_frag_info->certainty);
    } else {
      if (this_fragment->is_beginning()) {
        if (debug) cprintf("Record fragment beginning\n");
      } else {
        if (debug) {
          tprintf("Non-starting fragment piece with no prev_fragment\n");
        }
        return false;
      }
    }
  }
  if (word_ending && char_frag_info->fragment) {
    if (debug) tprintf("Word can not end with a fragment\n");
    return false;
  }
  return true;
}
/**
 * top_fragments_permute_and_select
 *
 * Creates a copy of character choices list that contain only fragments
 * and the best non-fragmented character choice.
 * Permutes character in this shortened list, builds characters from
 * fragments if possible and returns a better choice if found.
 */
WERD_CHOICE *Dict::top_fragments_permute_and_select(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    float rating_limit) {
  if (char_choices.length() <= 1 ||
      char_choices.length() > MAX_PERM_LENGTH) {
    return NULL;
  }
  // See it would be possible to benefit from permuting fragments.
  int x;
  float min_rating = 0.0;
  BLOB_CHOICE_IT blob_choice_it;
  for (x = 0; x < char_choices.length(); ++x) {
    blob_choice_it.set_to_list(char_choices.get(x));
    if (blob_choice_it.data()) {
      min_rating += blob_choice_it.data()->rating();
    }
    if (min_rating >= rating_limit) {
      return NULL;
    }
  }
  if (fragments_debug > 1) {
    tprintf("A choice with fragment beats top choice\n");
    tprintf("Running fragment permuter...\n");
  }

  // Construct a modified choices list that contains (for each position):
  // the best choice, all fragments and at least one choice for
  // a non-fragmented character.
  BLOB_CHOICE_LIST_VECTOR frag_char_choices(char_choices.length());
  for (x = 0; x < char_choices.length(); ++x) {
    bool need_nonfrag_char = true;
    BLOB_CHOICE_LIST *frag_choices = new BLOB_CHOICE_LIST();
    BLOB_CHOICE_IT frag_choices_it;
    frag_choices_it.set_to_list(frag_choices);
    blob_choice_it.set_to_list(char_choices.get(x));
    for (blob_choice_it.mark_cycle_pt(); !blob_choice_it.cycled_list();
         blob_choice_it.forward()) {
      if (getUnicharset().get_fragment(blob_choice_it.data()->unichar_id())) {
        frag_choices_it.add_after_then_move(
            new BLOB_CHOICE(*(blob_choice_it.data())));
      } else if (need_nonfrag_char) {
        frag_choices_it.add_after_then_move(
            new BLOB_CHOICE(*(blob_choice_it.data())));
        need_nonfrag_char = false;
      }
    }
    frag_char_choices += frag_choices;
  }

  WERD_CHOICE *best_choice = new WERD_CHOICE();
  best_choice->make_bad();
  WERD_CHOICE word(MAX_PERM_LENGTH);
  word.set_permuter(TOP_CHOICE_PERM);
  float certainties[MAX_PERM_LENGTH];
  this->go_deeper_fxn_ = &tesseract::Dict::go_deeper_top_fragments_fxn;
  int attempts_left = max_permuter_attempts;
  permute_choices((fragments_debug > 1) ? "fragments_debug" : NULL,
                  frag_char_choices, 0, NULL, &word, certainties,
                  &rating_limit, best_choice, &attempts_left, NULL);

  frag_char_choices.delete_data_pointers();
  return best_choice;
}

/**
 * permute_choices
 *
 * Call append_choices() for each BLOB_CHOICE in BLOB_CHOICE_LIST
 * with the given char_choice_index in char_choices.
 */
void Dict::permute_choices(
    const char *debug,
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    int char_choice_index,
    const CHAR_FRAGMENT_INFO *prev_char_frag_info,
    WERD_CHOICE *word,
    float certainties[],
    float *limit,
    WERD_CHOICE *best_choice,
    int *attempts_left,
    void *more_args) {
  if (debug) {
    tprintf("%s permute_choices: char_choice_index=%d"
            " limit=%g rating=%g, certainty=%g word=%s\n",
            debug, char_choice_index, *limit, word->rating(),
            word->certainty(), word->debug_string(getUnicharset()).string());
  }
  if (char_choice_index < char_choices.length()) {
    BLOB_CHOICE_IT blob_choice_it;
    blob_choice_it.set_to_list(char_choices.get(char_choice_index));
    for (blob_choice_it.mark_cycle_pt(); !blob_choice_it.cycled_list();
         blob_choice_it.forward()) {
      (*attempts_left)--;
      append_choices(debug, char_choices, *(blob_choice_it.data()),
                     char_choice_index, prev_char_frag_info, word,
                     certainties, limit, best_choice, attempts_left, more_args);
      if (*attempts_left <= 0) {
        if (debug) tprintf("permute_choices(): attempts_left is 0\n");
        break;
      }
    }
  }
}

/**
 * append_choices
 *
 * Checks to see whether or not the next choice is worth appending to
 * the word being generated. If so then keeps going deeper into the word.
 *
 * This function assumes that Dict::go_deeper_fxn_ is set.
 */
void Dict::append_choices(
    const char *debug,
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    const BLOB_CHOICE &blob_choice,
    int char_choice_index,
    const CHAR_FRAGMENT_INFO *prev_char_frag_info,
    WERD_CHOICE *word,
    float certainties[],
    float *limit,
    WERD_CHOICE *best_choice,
    int *attempts_left,
    void *more_args) {
  int word_ending =
    (char_choice_index == char_choices.length() - 1) ? true : false;

  // Deal with fragments.
  CHAR_FRAGMENT_INFO char_frag_info;
  if (!fragment_state_okay(blob_choice.unichar_id(), blob_choice.rating(),
                           blob_choice.certainty(), prev_char_frag_info, debug,
                           word_ending, &char_frag_info)) {
    return;  // blob_choice must be an invalid fragment
  }
  // Search the next letter if this character is a fragment.
  if (char_frag_info.unichar_id == INVALID_UNICHAR_ID) {
    permute_choices(debug, char_choices, char_choice_index + 1,
                    &char_frag_info, word, certainties, limit,
                    best_choice, attempts_left, more_args);
    return;
  }

  // Add the next unichar.
  float old_rating = word->rating();
  float old_certainty = word->certainty();
  uinT8 old_permuter = word->permuter();
  certainties[word->length()] = char_frag_info.certainty;
  word->append_unichar_id_space_allocated(
      char_frag_info.unichar_id, char_frag_info.num_fragments,
      char_frag_info.rating, char_frag_info.certainty);

  // Explore the next unichar.
  (this->*go_deeper_fxn_)(debug, char_choices, char_choice_index,
                          &char_frag_info, word_ending, word, certainties,
                          limit, best_choice, attempts_left, more_args);

  // Remove the unichar we added to explore other choices in it's place.
  word->remove_last_unichar_id();
  word->set_rating(old_rating);
  word->set_certainty(old_certainty);
  word->set_permuter(old_permuter);
}

/**
 * go_deeper_top_fragments_fxn
 *
 * While the choice being composed so far could be better
 * than best_choice keeps exploring char_choices.
 * If the end of the word is reached and the word is better than
 * best_choice, copies word to best_choice and logs the new word choice.
 */
void Dict::go_deeper_top_fragments_fxn(
    const char *debug, const BLOB_CHOICE_LIST_VECTOR &char_choices,
    int char_choice_index, const CHAR_FRAGMENT_INFO *prev_char_frag_info,
    bool word_ending, WERD_CHOICE *word, float certainties[], float *limit,
    WERD_CHOICE *best_choice, int *attempts_left, void *more_args) {
  if (word->rating() < *limit) {
    if (word_ending) {
      if (fragments_debug > 1) {
        tprintf("fragments_debug new choice = %s\n",
                word->debug_string(getUnicharset()).string());
      }
      *limit = word->rating();
      adjust_non_word(word, certainties, permute_debug);
      update_best_choice(*word, best_choice);
    } else {  // search the next letter
      permute_choices(debug, char_choices, char_choice_index + 1,
                      prev_char_frag_info, word, certainties, limit,
                      best_choice, attempts_left, more_args);
    }
  } else {
    if (fragments_debug > 1) {
      tprintf("fragments_debug pruned word (%s, rating=%4.2f, limit=%4.2f)\n",
              word->debug_string(getUnicharset()).string(),
              word->rating(), *limit);
    }
  }
}

}  // namespace tesseract
