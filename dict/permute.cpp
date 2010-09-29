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
#include "context.h"
#include "conversion.h"
#include "freelist.h"
#include "globals.h"
#include "ndminx.h"
#include "permdawg.h"
#include "permngram.h"
#include "ratngs.h"
#include "stopper.h"
#include "tordvars.h"
#include "tprintf.h"
#include "trie.h"
#include "varable.h"
#include "unicharset.h"
#include "dict.h"
#include "image.h"
#include "ccutil.h"

int permutation_count;           // Used in metrics.cpp.
/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
// TODO(tkielbus) Choose a value for the MAX_NUM_EDGES constant
// (or make it dynamic)
#define MAX_NUM_EDGES          2000000
#define MAX_DOC_EDGES          250000
#define MAX_USER_EDGES         50000
                                 /* Weights for adjustment */
#define NON_WERD               1.25
#define GARBAGE_STRING         1.5
#define MAX_PERM_LENGTH        128

// debugging flags
INT_VAR(fragments_debug, 0, "Debug character fragments");

BOOL_VAR(segment_debug, 0, "Debug the whole segmentation process");

BOOL_VAR(permute_debug, 0, "Debug char permutation process");


// control parameters
double_VAR(bestrate_pruning_factor, 2.0,
           "Multiplying factor of current best rate to prune other hypotheses");

BOOL_VAR(permute_script_word, 0,
         "Turn on word script consistency permuter");

BOOL_VAR(segment_segcost_rating, 0,
         "incorporate segmentation cost in word rating?");

double_VAR(segment_reward_script, 0.95,
           "Score multipler for script consistency within a word. "
           "Being a 'reward' factor, it should be <= 1. "
           "Smaller value implies bigger reward.");

double_VAR(segment_penalty_dict_nonword, NON_WERD,
           "Score multiplier for glyph fragment segmentations which do not "
           "match a dictionary word (lower is better).");

double_VAR(segment_penalty_garbage, GARBAGE_STRING,
           "Score multiplier for poorly cased strings that are not in the "
           "dictionary and generally look like garbage (lower is better).");

BOOL_VAR(save_doc_words, 0, "Save Document Words");

BOOL_VAR(doc_dict_enable, 1, "Enable Document Dictionary ");

BOOL_VAR(ngram_permuter_activated, FALSE,
         "Activate character-level n-gram-based permuter");

STRING_VAR(global_user_words_suffix, "user-words", "A list of user-provided words.");

// This is an ugly way to incorporate segmentation cost in word rating.
// See comments in incorporate_segcost.
float wordseg_rating_adjust_factor;

int permute_only_top = 0;

#define SIM_CERTAINTY_SCALE  -10.0   /*< Similarity matcher values */
#define SIM_CERTAINTY_OFFSET -10.0   /*< Similarity matcher values */
#define SIMILARITY_FLOOR     100.0   /*< Worst E*L product to stop on */

// TODO(daria): If hyphens are different in different languages and can be
// inferred from training data we should load their values dynamically.
static const char kHyphenSymbol[] = "-";

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
 * good_choice
 *
 * Return TRUE if a good answer is found for the unknown blob rating.
 */
int good_choice(const WERD_CHOICE &choice) {
  register float certainty;
  if (tord_similarity_enable) {
    if ((choice.rating() + 1) * choice.certainty() > SIMILARITY_FLOOR)
      return false;
    certainty =
      SIM_CERTAINTY_OFFSET + choice.rating() * SIM_CERTAINTY_SCALE;
  } else {
    certainty = choice.certainty();
  }

  return (certainty > tord_certainty_threshold) ? true : false;
}


namespace tesseract {
/**
 * add_document_word
 *
 * Add a word found on this document to the document specific
 * dictionary.
 */
void Dict::add_document_word(const WERD_CHOICE &best_choice) {
  // Do not add hyphenated word parts to the document dawg.
  // hyphen_word_ will be non-NULL after the set_hyphen_word() is
  // called when the first part of the hyphenated word is
  // discovered and while the second part of the word is recognized.
  // hyphen_word_ is cleared in cc_recg() before the next word on
  // the line is recognized.
  if (hyphen_word_) return;

  char filename[CHARS_PER_LINE];
  FILE *doc_word_file;
  int stringlen = best_choice.length();

  if (!doc_dict_enable || valid_word(best_choice) ||
      CurrentWordAmbig() || stringlen < 2)
    return;

  if (!good_choice(best_choice) || stringlen == 2) {
    if (best_choice.certainty() < permuter_pending_threshold)
      return;

    if (!pending_words_->word_in_dawg(best_choice)) {
      if (stringlen > 2 ||
          (stringlen == 2 &&
           getUnicharset().get_isupper(best_choice.unichar_id(0)) &&
           getUnicharset().get_isupper(best_choice.unichar_id(1)))) {
        pending_words_->add_word_to_dawg(best_choice);
      }
      return;
    }
  }

  if (save_doc_words) {
    strcpy(filename, getImage()->getCCUtil()->imagefile.string());
    strcat (filename, ".doc");
    doc_word_file = open_file (filename, "a");
    fprintf (doc_word_file, "%s\n",
             best_choice.debug_string(getUnicharset()).string());
    fclose(doc_word_file);
  }
  document_words_->add_word_to_dawg(best_choice);
}


/**
 * adjust_non_word
 *
 * Assign an adjusted value to a string that is a non-word.  The value
 * that this word choice has is based on case and punctuation rules.
 * The adjustment value applied is stored in adjust_factor upon return.
 */
void Dict::adjust_non_word(WERD_CHOICE *word, float *adjust_factor) {
  float new_rating;
  if (permute_debug)
    cprintf("Non-word: %s %4.2f ",
            word->debug_string(getUnicharset()).string(), word->rating());

  new_rating = word->rating() + RATING_PAD;
  if (Context::case_ok(*word, getUnicharset()) && valid_punctuation(*word)) {
    new_rating *= segment_penalty_dict_nonword;
    *adjust_factor = segment_penalty_dict_nonword;
    if (permute_debug) tprintf(", W");
  } else {
    new_rating *= segment_penalty_garbage;
    *adjust_factor = segment_penalty_garbage;
    if (permute_debug) {
      if (!Context::case_ok(*word, getUnicharset())) tprintf(", C");
      if (!valid_punctuation(*word)) tprintf(", P");
    }
  }
  new_rating -= RATING_PAD;
  word->set_rating(new_rating);
  if (permute_debug)
    cprintf (" %4.2f --> %4.2f\n", *adjust_factor, new_rating);
}


/**
 * init_permute
 *
 * Initialize anything that needs to be set up for the permute
 * functions.
 */
void Dict::init_permute() {
  STRING name;
  STRING &lang = getImage()->getCCUtil()->lang;

  if (dawgs_.length() != 0) end_permute();

  hyphen_unichar_id_ = getUnicharset().unichar_to_id(kHyphenSymbol);
  TessdataManager &tessdata_manager =
    getImage()->getCCUtil()->tessdata_manager;

  // Load dawgs_.
  if (global_load_punc_dawg &&
      tessdata_manager.SeekToStart(TESSDATA_PUNC_DAWG)) {
    dawgs_ += new SquishedDawg(tessdata_manager.GetDataFilePtr(),
                               DAWG_TYPE_PUNCTUATION, lang, PUNC_PERM);
  }
  if (global_load_system_dawg &&
      tessdata_manager.SeekToStart(TESSDATA_SYSTEM_DAWG)) {
    dawgs_ += new SquishedDawg(tessdata_manager.GetDataFilePtr(),
                               DAWG_TYPE_WORD, lang, SYSTEM_DAWG_PERM);
  }
  if (global_load_number_dawg &&
      tessdata_manager.SeekToStart(TESSDATA_NUMBER_DAWG)) {
    dawgs_ +=
      new SquishedDawg(tessdata_manager.GetDataFilePtr(),
                       DAWG_TYPE_NUMBER, lang, NUMBER_PERM);
  }
  if (((STRING &)global_user_words_suffix).length() > 0) {
    name = getImage()->getCCUtil()->language_data_path_prefix;
    name += global_user_words_suffix;
    if (exists_file(name.string())) {
      Trie *trie_ptr = new Trie(DAWG_TYPE_WORD, lang, USER_DAWG_PERM,
                                MAX_USER_EDGES, getUnicharset().size());
      if (!trie_ptr->read_word_list(name.string(), getUnicharset())) {
        tprintf("Error: failed to load %s\n", name.string());
        exit(1);
      }
      dawgs_ += trie_ptr;
    }
  }
  document_words_ = new Trie(DAWG_TYPE_WORD, lang, DOC_DAWG_PERM,
                             MAX_DOC_EDGES, getUnicharset().size());
  dawgs_ += document_words_;

  // This dawg is temporary and should not be searched by letter_is_ok.
  pending_words_ = new Trie(DAWG_TYPE_WORD, lang, NO_PERM,
                            MAX_DOC_EDGES, getUnicharset().size());

  // The frequent words dawg is only searched when a word
  // is found in any of the other dawgs.
  if (tessdata_manager.SeekToStart(TESSDATA_FREQ_DAWG)) {
    freq_dawg_ = new SquishedDawg(tessdata_manager.GetDataFilePtr(),
                                  DAWG_TYPE_WORD, lang, FREQ_DAWG_PERM);
  }

  // Construct a list of corresponding successors for each dawg. Each entry i
  // in the successors_ vector is a vector of integers that represent the
  // indices into the dawgs_ vector of the successors for dawg i.
  successors_.reserve(dawgs_.length());
  for (int i = 0; i < dawgs_.length(); ++i) {
    const Dawg *dawg = dawgs_[i];
    SuccessorList *lst = new SuccessorList();
    for (int j = 0; j < dawgs_.length(); ++j) {
      const Dawg *other = dawgs_[j];
      if (dawg->lang() == other->lang() &&
          kDawgSuccessors[dawg->type()][other->type()]) *lst += j;
    }
    successors_ += lst;
  }
}

void Dict::end_permute() {
  if (dawgs_.length() == 0)
    return;  // Not safe to call twice.
  dawgs_.delete_data_pointers();
  successors_.delete_data_pointers();
  dawgs_.clear();
  successors_.clear();
  document_words_ = NULL;
  if (pending_words_ != NULL) delete pending_words_;
  pending_words_ = NULL;
  if (freq_dawg_ != NULL) delete freq_dawg_;
  freq_dawg_ = NULL;
}


/**
 * permute_all
 *
 * Permute all the characters together using all of the different types
 * of permuters/selectors available.  Each of the characters must have
 * a non-NULL choice list.
 *
 * Note: order of applying permuters does matter, since the latter
 * permuter will be recorded if the resulting word ratings are the same.
 */
WERD_CHOICE *Dict::permute_all(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                               float rating_limit,
                               WERD_CHOICE *raw_choice) {
  WERD_CHOICE *result1;
  WERD_CHOICE *result2 = NULL;
  BOOL8 any_alpha;
  float top_choice_rating_limit = rating_limit;

  // Initialize result1 from the result of permute_top_choice.
  result1 = permute_top_choice(char_choices, &top_choice_rating_limit,
                               raw_choice, &any_alpha);

  // Enforce script consistency within a word on some scripts
  if (permute_script_word &&
      !word_script_eq(char_choices, getUnicharset().common_sid()) &&
      !word_script_eq(char_choices, getUnicharset().latin_sid())) {
    result2 = permute_script_words(char_choices);
    // TODO(dsl): incorporate segmentation cost into word rating.
    // This should only be turned on for scripts that we have a segmentation
    // cost model for, such as CJK.
    if (segment_segcost_rating)
      incorporate_segcost(result2);
    result1 = get_best_delete_other(result1, result2);
  }

  // Permute character fragments if necessary.
  if (result1 == NULL || result1->fragment_mark()) {
    result2 = top_fragments_permute_and_select(char_choices,
                                               top_choice_rating_limit);
    result1 = get_best_delete_other(result1, result2);
  }

  // TODO(daria): update ngram permuter code.
  if (ngram_permuter_activated) {
    tprintf("Error: ngram permuter functionality is not available\n");
    exit(1);
    // A_CHOICE *ngram_choice =
    //  ngram_permute_and_select(old_char_choices, rating_limit, word_dawg_);
    // return ngram_choice;
  }

  if (result1 == NULL)
    return (NULL);
  if (permute_only_top)
    return result1;

  result2 = dawg_permute_and_select(char_choices, rating_limit);
  result1 = get_best_delete_other(result1, result2);

  result2 = permute_compound_words(char_choices, rating_limit);
  result1 = get_best_delete_other(result1, result2);

  return (result1);
}

/** Returns the top choice char id.  A helper function to make code cleaner. */
UNICHAR_ID get_top_choice_uid(BLOB_CHOICE_LIST *blob_list) {
  BLOB_CHOICE_IT blob_choice_it;
  blob_choice_it.set_to_list(blob_list);
  return (blob_choice_it.data()) ? blob_choice_it.data()->unichar_id()
                                 : INVALID_UNICHAR_ID;
}

/**
 * Return the "dominant" script ID for the word.  By "dominant", the script
 * must account for at least half the characters.  Otherwise, it returns 0.
 */
int get_top_word_script(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                        const UNICHARSET &unicharset) {
  int max_script = unicharset.get_script_table_size();
  int *sid = new int[max_script];
  int x;
  for (x = 0; x < max_script; x++) sid[x] = 0;
  for (x = 0; x < char_choices.length(); ++x) {
    BLOB_CHOICE_IT blob_choice_it;
    blob_choice_it.set_to_list(char_choices.get(x));
    sid[blob_choice_it.data()->script_id()]++;
  }
  // Note that high script ID overrides lower one on a tie, thus biasing
  // towards non-Common script (if sorted that way in unicharset file).
  int max_sid = 0;
  for (x = 1; x < max_script; x++)
    if (sid[x] >= sid[max_sid]) max_sid = x;
  if (sid[max_sid] < char_choices.length() / 2)
    max_sid = unicharset.null_sid();
  delete[] sid;
  return max_sid;
}

/**
 * Checks whether the dominant word script, if there is one, matches
 * the given target script ID.
 */
bool Dict::word_script_eq(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                          int target_sid) {
  int max_sid = get_top_word_script(char_choices, getUnicharset());
  // If "Latin" is not a loaded script, then latin_sid() would return 0.
  // max_sid could also be 0 if there is no dominant script.
  // This is faster than
  // strcmp(getUnicharset().get_script_from_script_id(max_sid), "Latin")
  return (max_sid > 0 && max_sid == target_sid);
}

/**
 * Iterate through all the character choices (for a single blob) and
 * return the first that matches the given type, which is one of 'aA0px*',
 * for lower, upper, digit, punctuation, other, and 'any', respectively.
 * If not match is found, a NULL is returned.
 */
BLOB_CHOICE* find_choice_by_type(
    BLOB_CHOICE_LIST *char_choices,
    char target_type,
    const UNICHARSET &unicharset) {
  BLOB_CHOICE_IT c_it;
  c_it.set_to_list(char_choices);
  for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
    bool found = false;
    UNICHAR_ID unichar_id = c_it.data()->unichar_id();
    switch (target_type) {
      case '*': found = true;  break;
      case 'A': found = unicharset.get_isupper(unichar_id); break;
      case 'a': found = unicharset.get_islower(unichar_id); break;
      case '0': found = unicharset.get_isdigit(unichar_id); break;
      case 'p': found = unicharset.get_ispunctuation(unichar_id); break;
      case 'x': found = !unicharset.get_isupper(unichar_id) &&
                        !unicharset.get_islower(unichar_id) &&
                        !unicharset.get_isdigit(unichar_id) &&
                        !unicharset.get_ispunctuation(unichar_id);
                break;
    }
    if (found) return c_it.data();
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
    BLOB_CHOICE_LIST *char_choices,
    int target_sid,
    int backup_sid,
    int secondary_sid) {
  BLOB_CHOICE_IT c_it;
  c_it.set_to_list(char_choices);
  for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
    bool found = false;
    if (c_it.data()->script_id() == 0) continue;
    if (c_it.data()->script_id() == target_sid) found = true;
    if (backup_sid > 0 && c_it.data()->script_id() == backup_sid) found = true;
    if (found) return c_it.data();
  }
  if (secondary_sid > 0) {
    c_it.set_to_list(char_choices);
    for (c_it.mark_cycle_pt(); !c_it.cycled_list(); c_it.forward()) {
      if (c_it.data()->script_id() == 0) continue;
      if (c_it.data()->script_id() == secondary_sid)
        return c_it.data();
    }
  }
  return NULL;
}

/**
 * Incorporate segmentation cost into the word rating.  This is done
 * through a mutliplier wordseg_rating_adjust_factor which is determined
 * in bestfirst.cpp during state evaluation.  This is not the cleanest
 * way to do this.  It would be better to reorganize the SEARCH_STATE
 * to keep track of associated states, or do the rating adjustment
 * outside the permuter in evalaute_state.
 */
void Dict::incorporate_segcost(WERD_CHOICE *word) {
  if (!word || wordseg_rating_adjust_factor <= 0) return;

  float old_rating = word->rating();
  float new_rating = old_rating * wordseg_rating_adjust_factor;
  word->set_rating(new_rating);
  if (permute_debug)
    tprintf("Permute segadjust %f * %f --> %f\n",
            old_rating, wordseg_rating_adjust_factor, new_rating);
}

/**
 * Try flipping characters in a word to get better script consistency.
 * Similar to how upper/lower case checking is done in top_choice_permuter,
 * this permuter tries to suggest a more script-consistent choice AND
 * modifieds the rating.  So it combines both the case_ok check and
 * adjust_non_word functionality.  However, instead of penalizing an
 * inconsistent word with a > 1 multiplier, we reward the script-consistent
 * choice with a < 1 multiplier.
 */
WERD_CHOICE* Dict::permute_script_words(
    const BLOB_CHOICE_LIST_VECTOR &char_choices) {
  if (char_choices.length() > MAX_WERD_LENGTH)
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

  WERD_CHOICE *current_word = new WERD_CHOICE(MAX_WERD_LENGTH);
  BLOB_CHOICE_IT blob_choice_it;
  bool replaced = false;
  bool prev_is_consistent = false;
  for (int x = 0; x < char_choices.length(); ++x) {
    blob_choice_it.set_to_list(char_choices.get(x));
    BLOB_CHOICE *first_choice = blob_choice_it.data();
    if (!first_choice) return NULL;
    UNICHAR_ID unichar_id = first_choice->unichar_id();
    bool sid_consistent = (first_choice->script_id() == word_sid);
    bool this_is_punct = getUnicharset().get_ispunctuation(unichar_id);

    if (!sid_consistent && !this_is_punct && prev_is_consistent) {
      // If the previous char is CJK, we prefer a cjk over non-cjk char
      if (permute_debug) {
        tprintf("Checking %s r%g\n", getUnicharset().id_to_unichar(unichar_id),
                                     first_choice->rating());
        print_ratings_list("\t", char_choices.get(x), getUnicharset());
      }
      // prefer a script consistent choice
      BLOB_CHOICE* c_it = find_choice_by_script(char_choices.get(x),
                                                word_sid, 0, 0);
      // make this a separate check
      // otherwise, prefer a punctuation
      if (c_it == NULL)
        c_it = find_choice_by_type(char_choices.get(x), 'p', getUnicharset());

      if (c_it != NULL) {
        if (permute_debug)
          tprintf("Replacing %d r%g ==> %d r%g\n",
                  first_choice->unichar_id(), first_choice->rating(),
                  c_it->unichar_id(), c_it->rating());
        first_choice = c_it;
        replaced = true;
      }
    }
    current_word->append_unichar_id_space_allocated(
      first_choice->unichar_id(), 1,
      first_choice->rating(), first_choice->certainty());
    prev_is_consistent = sid_consistent;
  }
  if (replaced) {
    // When we replace a word choice (usually top choice) with
    // another for the sake of script consistency, we need to improve its
    // rating so that it will replace the best choice.  How much we modify
    // the rating determines how strong is the script consistency constraint.
    // We need a more consistent solution for all contextual constraints
    // like case, punct pattern, script, etc.  Right now, this does the same
    // thing as adjust_non_words for case and punctuation rules.
    float rating = current_word->rating();
    rating *= segment_reward_script;
    current_word->set_rating(rating);
  }
  current_word->populate_unichars(getUnicharset());
  if (permute_debug && replaced)
    current_word->print("<== permute_script_word **");
  return current_word;
}

/**
 * permute_characters
 *
 * Permute these characters together according to each of the different
 * permuters that are enabled.
 */
void Dict::permute_characters(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                              float limit,
                              WERD_CHOICE *best_choice,
                              WERD_CHOICE *raw_choice) {
  float old_raw_choice_rating = raw_choice->rating();
  permutation_count++;           /* Global counter */
  if (tord_display_ratings > 1) {
    cprintf("\nchar_choices in permute_characters:\n");
    print_char_choices_list("\n==> Input CharChoices", char_choices,
                            getUnicharset(), true);
  }

  if (char_choices.length() == 1 &&
      get_top_choice_uid(char_choices.get(0)) == 0)
    return;
  WERD_CHOICE *this_choice = permute_all(char_choices, limit, raw_choice);

  if (raw_choice->rating() < old_raw_choice_rating) {
    // Populate unichars_ and unichar_lengths_ of raw_choice. This is
    // needed for various components that still work with unichars rather
    // than unichar ids (e.g. AdaptToWord).
    raw_choice->populate_unichars(getUnicharset());
  }
  if (this_choice && this_choice->rating() < best_choice->rating()) {
    *best_choice = *this_choice;
    // Populate unichars_ and unichar_lengths_ of best_choice. This is
    // needed for various components that still work with unichars rather
    // than unichar ids (dawg, *_ok functions, various hard-coded hacks).
    best_choice->populate_unichars(getUnicharset());

    if (tord_display_ratings) {
      cprintf("permute_characters: %s\n",
              best_choice->debug_string(getUnicharset()).string());
    }
  }
  delete this_choice;
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
    current_word.populate_unichars(getUnicharset());
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
 * excercise.
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
    bool old_segment_dawg_debug = segment_dawg_debug;
    if (segment_debug) segment_dawg_debug.set_value(true);
    best_choice = permute_all(subchoices, rating_limit, &raw_choice);

    if (segment_debug) {
      segment_dawg_debug.set_value(old_segment_dawg_debug);
    }
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

  EnableChoiceAccum();
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
  float adjust_factor;

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
        tprintf("\n***** Aborting high-cost word: %g > limit %g \n",
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
    LogNewChoice(*raw_choice, 1.0, certainties, true);
  }

  if (ngram_permuter_activated)
    return NULL;

  float rating = word.rating();
  adjust_non_word(&word, &adjust_factor);
  LogNewChoice(word, adjust_factor, certainties, false);

  float lower_rating = lower_word.rating();
  adjust_non_word(&lower_word, &adjust_factor);
  LogNewChoice(lower_word, adjust_factor, lower_certainties, false);

  float upper_rating = capital_word.rating();
  adjust_non_word(&capital_word, &adjust_factor);
  LogNewChoice(capital_word, adjust_factor, upper_certainties, false);

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
 * Check all the DAWGs to see if this word is in any of them.
 */
int Dict::valid_word(const WERD_CHOICE &word, bool numbers_ok) {
  const WERD_CHOICE *word_ptr = &word;
  WERD_CHOICE temp_word;
  if (hyphenated()) {
    copy_hyphen_info(&temp_word);
    temp_word += word;
    word_ptr = &temp_word;
  }
  if (word_ptr->length() == 0) return NO_PERM;
  // Allocate vectors for holding current and updated
  // active_dawgs and constraints and initialize them.
  DawgInfoVector *active_dawgs = new DawgInfoVector[2];
  DawgInfoVector *constraints = new DawgInfoVector[2];
  init_active_dawgs(&(active_dawgs[0]));
  init_constraints(&(constraints[0]));
  DawgArgs dawg_args(&(active_dawgs[0]), &(constraints[0]),
                     &(active_dawgs[1]), &(constraints[1]), 0.0);
  int last_index = word_ptr->length() - 1;
  // Call leter_is_okay for each letter in the word.
  for (int i = hyphen_base_size(); i <= last_index; ++i) {
    if (!((this->*letter_is_okay_)(&dawg_args, i, word_ptr,
                                   i == last_index))) break;
    // Swap active_dawgs, constraints with the corresponding updated vector.
    if (dawg_args.updated_active_dawgs == &(active_dawgs[1])) {
      dawg_args.updated_active_dawgs = &(active_dawgs[0]);
      dawg_args.updated_constraints = &(constraints[0]);
      ++(dawg_args.active_dawgs);
      ++(dawg_args.constraints);
    } else {
      ++(dawg_args.updated_active_dawgs);
      ++(dawg_args.updated_constraints);
      dawg_args.active_dawgs = &(active_dawgs[0]);
      dawg_args.constraints = &(constraints[0]);
    }
  }
  delete[] active_dawgs;
  delete[] constraints;
  if (dawg_args.permuter == SYSTEM_DAWG_PERM ||
      dawg_args.permuter == DOC_DAWG_PERM ||
      dawg_args.permuter == USER_DAWG_PERM ||
      (numbers_ok && dawg_args.permuter == NUMBER_PERM)){
    return dawg_args.permuter;
  } else {
    return NO_PERM;
  }
}

/**
 * @return true if the word contains a valid punctuation pattern.
 *
 * @note Since the domains of punctuation symbols and symblos
 * used in numbers are not disjoint, a valid number might contain
 * an invalid punctuation pattern (e.g. .99).
 */
bool Dict::valid_punctuation(const WERD_CHOICE &word) {
  if (word.length() == 0) return NO_PERM;
  int i;
  WERD_CHOICE new_word;
  int last_index = word.length() - 1;
  int new_len = 0;
  for (i = 0; i <= last_index; ++i) {
    UNICHAR_ID unichar_id = (word.unichar_id(i));
    if (getUnicharset().get_ispunctuation(unichar_id)) {
      new_word.append_unichar_id(unichar_id, 1, 0.0, 0.0);
    } else if (!getUnicharset().get_isalpha(unichar_id) &&
               !getUnicharset().get_isdigit(unichar_id)) {
      return false;  // neither punc, nor alpha, nor digit
    } else if ((new_len = new_word.length()) == 0 ||
               new_word.unichar_id(new_len-1) != Dawg::kPatternUnicharID) {
      new_word.append_unichar_id(Dawg::kPatternUnicharID, 1, 0.0, 0.0);
    }
  }
  for (i = 0; i < dawgs_.size(); ++i) {
    if (dawgs_[i]->type() == DAWG_TYPE_PUNCTUATION &&
        dawgs_[i]->word_in_dawg(new_word)) return true;
  }
  return false;
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
  permute_choices((fragments_debug > 1) ? "fragments_debug" : NULL,
                  frag_char_choices, 0, NULL, &word, certainties,
                  &rating_limit, best_choice, NULL);

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
    void *more_args) {
  if (debug) {
    tprintf("%s permute_choices: char_choice_index=%d"
            " limit=%4.2f rating=%4.2f, certainty=%4.2f word=%s\n",
            debug, char_choice_index, *limit, word->rating(),
            word->certainty(), word->debug_string(getUnicharset()).string());
  }
  if (char_choice_index < char_choices.length()) {
    BLOB_CHOICE_IT blob_choice_it;
    blob_choice_it.set_to_list(char_choices.get(char_choice_index));
    for (blob_choice_it.mark_cycle_pt(); !blob_choice_it.cycled_list();
         blob_choice_it.forward()) {
      append_choices(debug, char_choices, *(blob_choice_it.data()),
                     char_choice_index, prev_char_frag_info, word,
                     certainties, limit, best_choice, more_args);

    }
  }
}

/**
 * append_choices
 *
 * Check to see whether or not the next choice is worth appending to
 * the word being generated. If so then keep going deeper into the word.
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
                    best_choice, more_args);
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
                          limit, best_choice, more_args);

  // Remove the unichar we added to explore other choices in it's place.
  word->remove_last_unichar_id();
  word->set_rating(old_rating);
  word->set_certainty(old_certainty);
  word->set_permuter(old_permuter);
}

/**
 * go_deeper_top_fragments_fxn
 *
 * If the choice being composed so far could be better
 * than best_choice keep exploring choices.
 */
void Dict::go_deeper_top_fragments_fxn(
    const char *debug, const BLOB_CHOICE_LIST_VECTOR &char_choices,
    int char_choice_index,
    const CHAR_FRAGMENT_INFO *prev_char_frag_info,
    bool word_ending, WERD_CHOICE *word, float certainties[],
    float *limit, WERD_CHOICE *best_choice, void *more_args) {
  if (word->rating() < *limit) {
    if (word_ending) {
      if (fragments_debug > 1) {
        tprintf("fragments_debug new choice = %s\n",
                word->debug_string(getUnicharset()).string());
      }
      *limit = word->rating();

      float adjust_factor;
      adjust_non_word(word, &adjust_factor);
      LogNewChoice(*word, adjust_factor, certainties, false);

      if (word->rating() < best_choice->rating()) {
        *best_choice = *word;
      }
    } else {  // search the next letter
      permute_choices(debug, char_choices, char_choice_index + 1,
                      prev_char_frag_info, word, certainties, limit,
                      best_choice, more_args);
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
