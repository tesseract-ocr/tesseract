///////////////////////////////////////////////////////////////////////
// File:        dict.h
// Description: dict class.
// Author:      Samuel Charron
//
// (C) Copyright 2006, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_DICT_DICT_H_
#define TESSERACT_DICT_DICT_H_

#include "ambigs.h"
#include "choices.h"
#include "choicearr.h"
#include "dawg.h"
#include "image.h"
#include "ratngs.h"
#include "stopper.h"
#include "trie.h"
#include "unicharset.h"

extern STRING_VAR_H(global_user_words_suffix, "user-words",
                    "A list of user-provided words.");
extern INT_VAR_H(hyphen_debug_level, 0, "Debug level for hyphenated words.");

#define MAX_WERD_LENGTH        (inT64) 40
#define NO_RATING               -1
#define FREQ_WERD               1.0
#define GOOD_WERD               1.1
#define OK_WERD                 1.3125

/** Struct used to hold temporary information about fragments. */
struct CHAR_FRAGMENT_INFO {
  UNICHAR_ID unichar_id;
  const CHAR_FRAGMENT *fragment;
  int num_fragments;
  float rating;
  float certainty;
};

namespace tesseract {

typedef GenericVector<Dawg *> DawgVector;

struct DawgArgs {
  DawgArgs(DawgInfoVector *d, DawgInfoVector *c,
           DawgInfoVector *ud, DawgInfoVector *uc, float r) :
    active_dawgs(d), constraints(c), updated_active_dawgs(ud),
    updated_constraints(uc), rating_margin(r) {
    for (int i = 0; i < MAX_WERD_LENGTH; ++i) {
      rating_array[i] = NO_RATING;
    }
    permuter = NO_PERM;
  }
  DawgInfoVector *active_dawgs;
  DawgInfoVector *constraints;
  DawgInfoVector *updated_active_dawgs;
  DawgInfoVector *updated_constraints;
  PermuterType permuter;
  float rating_margin;  /**< pruning margin ratio */
  float rating_array[MAX_WERD_LENGTH];
};

class Dict {
 public:
  Dict(Image* image_ptr);
  ~Dict();
  Image* getImage() {
    return image_ptr_;
  }
  UNICHARSET& getUnicharset() {
    return getImage()->getCCUtil()->unicharset;
  }
  const UnicharAmbigs &getUnicharAmbigs() {
    return getImage()->getCCUtil()->unichar_ambigs;
  }

  /* hyphen.cpp ************************************************************/

  /// Returns true if we've recorded the beginning of a hyphenated word.
  inline bool hyphenated() { return !last_word_on_line_ && hyphen_word_; }
  /// Size of the base word (the part on the line before) of a hyphenated word.
  inline int hyphen_base_size() {
    return this->hyphenated() ? hyphen_word_->length() : 0;
  }
  /// If this word is hyphenated copy the base word (the part on
  /// the line before) of a hyphenated word into the given word.
  /// This function assumes that word is not NULL.
  inline void copy_hyphen_info(WERD_CHOICE *word) {
    if (this->hyphenated()) {
      *word = *hyphen_word_;
      if (hyphen_debug_level) word->print("copy_hyphen_info: ");
    }
  }
  /// Erase the unichar ids corresponding to the portion of the word
  /// from the previous line. The word is not changed if it is not
  /// split between lines and hyphenated.
  inline void remove_hyphen_head(WERD_CHOICE *word) {
    if (this->hyphenated()) {
      word->remove_unichar_ids(0, hyphen_word_->length());
      if (hyphen_debug_level) hyphen_word_->print("remove_hyphen_head: ");
    }
  }
  /// Check whether the word has a hyphen at the end.
  inline bool has_hyphen_end(const WERD_CHOICE &word) {
    int word_index = word.length() - 1;
    return (last_word_on_line_ && word_index > 0 &&
            word.unichar_id(word_index) == hyphen_unichar_id_);
  }
  /// Unless the previous word was the last one on the line, and the current
  /// one is not (thus it is the first one on the line), erase hyphen_word_,
  /// clear hyphen_active_dawgs_, hyphen_constraints_ update last_word_on_line_.
  void reset_hyphen_vars(bool last_word_on_line);
  /// Update hyphen_word_, and copy the given DawgInfoVectors into
  /// hyphen_active_dawgs_ and hyphen_constraints_.
  void set_hyphen_word(const WERD_CHOICE &word,
                       const DawgInfoVector &active_dawgs,
                       const DawgInfoVector &constraints);

  /* permdawg.cpp ************************************************************/
  /// If new_rating < best_choice->rating(), copy word int best_choice
  /// and update rating and permuter of best_choice to the new given values.
  inline void update_best_choice(
      const WERD_CHOICE &word, WERD_CHOICE *best_choice) {
    if (word.rating() < best_choice->rating()) {
      *best_choice = word;
    }
  }
  /// Fill the given active_dawgs vector with dawgs that could contain the
  /// beginning of the word. If hyphenated() returns true, copy the entries
  /// from hyphen_active_dawgs_ instead.
  void init_active_dawgs(DawgInfoVector *active_dawgs);
  /// If hyphenated() returns true, copy the entries from hyphen_constraints_
  /// into the given constraints vector.
  void init_constraints(DawgInfoVector *constraints);
  /// Recursively explore all the possible character combinations in
  /// the given char_choices. Use go_deeper_dawg_fxn() to explore all the
  /// dawgs in the dawgs_ vector in parallel and discard invalid words.
  ///
  /// Allocate and return a WERD_CHOICE with the best valid word found.
  WERD_CHOICE *dawg_permute_and_select(
      const BLOB_CHOICE_LIST_VECTOR &char_choices, float rating_limit);
  void adjust_word(WERD_CHOICE *best_choice,
                   float *certainty_array);
  /// If the choice being composed so far could be a dictionary word
  /// and we have not reached the end of the word keep exploring the
  /// char_choices further.
  ///
  /// Also:
  /// - set hyphen word if needed
  /// - if word_ending is true and word is better than best_choice
  ///   copy word to best_choice log new word choice
  void go_deeper_dawg_fxn(
      const char *debug, const BLOB_CHOICE_LIST_VECTOR &char_choices,
      int char_choice_index, const CHAR_FRAGMENT_INFO *prev_char_frag_info,
      bool word_ending, WERD_CHOICE *word, float certainties[],
      float *limit, WERD_CHOICE *best_choice, void *void_more_args);

  /* permute.cpp *************************************************************/
  void add_document_word(const WERD_CHOICE &best_choice);
  void init_permute();
  WERD_CHOICE *permute_top_choice(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    float* rating_limit,
    WERD_CHOICE *raw_choice,
    BOOL8 *any_alpha);
  const char* choose_il1(const char *first_char,       //< first choice
                         const char *second_char,      //< second choice
                         const char *third_char,       //< third choice
                         const char *prev_char,        //< prev in word
                         const char *next_char,        //< next in word
                         const char *next_next_char);  //< after next next in word
  /// @return NO_PERM for words with digits
  int valid_word(const WERD_CHOICE &word) {
    return valid_word(word, false);  
  }
  /// @return NUMBER_PERM for valid numbers
  int valid_word_or_number(const WERD_CHOICE &word) {
    return valid_word(word, true);
  }
  int valid_word(const WERD_CHOICE &word, bool numbers_ok);
  bool valid_punctuation(const WERD_CHOICE &word);
  WERD_CHOICE *permute_all(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                           float rating_limit,
                           WERD_CHOICE *raw_choice);
  void end_permute();
  void adjust_non_word(WERD_CHOICE *word, float *adjust_factor);
  void permute_subword(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                       float rating_limit,
                       int start,
                       int end,
                       WERD_CHOICE *current_word);
  void permute_characters(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                          float limit,
                          WERD_CHOICE *best_choice,
                          WERD_CHOICE *raw_choice);
  WERD_CHOICE *permute_compound_words(
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      float rating_limit);
  /// checks if the dominant word script, if there is one, is same as target.
  bool word_script_eq(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                      int target_script_id);
  /// Incoporate segmentation cost into word rating
  void incorporate_segcost(WERD_CHOICE* word);
  /// checks for script-consistent permutations
  WERD_CHOICE *permute_script_words(
      const BLOB_CHOICE_LIST_VECTOR &char_choices);

  WERD_CHOICE *top_fragments_permute_and_select(
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      float rating_limit);
  /// If the choice being composed so far could be better
  /// than best_choice keep exploring char_choices.
  /// If we have reached the end of the word and word is better than
  /// best_choice, copy word to best_choice and log a new word choice.
  void go_deeper_top_fragments_fxn(
      const char *debug, const BLOB_CHOICE_LIST_VECTOR &char_choices,
      int char_choice_index, const CHAR_FRAGMENT_INFO *prev_char_frag_info,
      bool word_ending, WERD_CHOICE *word, float certainties[],
      float *limit, WERD_CHOICE *best_choice, void *more_args);

  /// Semi-generic functions used by multiple permuters.
  bool fragment_state_okay(UNICHAR_ID curr_unichar_id,
                           float curr_rating, float curr_certainty,
                           const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                           const char *debug, int word_ending,
                           CHAR_FRAGMENT_INFO *char_frag_info);
  void permute_choices(
      const char *debug,
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      int char_choice_index,
      const CHAR_FRAGMENT_INFO *prev_char_frag_info,
      WERD_CHOICE *word,
      float certainties[],
      float *limit,
      WERD_CHOICE *best_choice,
      void *more_args);

  void append_choices(
      const char *debug,
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      const BLOB_CHOICE &blob_choice,
      int char_choice_index,
      const CHAR_FRAGMENT_INFO *prev_char_frag_info,
      WERD_CHOICE *word,
      float certainties[],
      float *limit,
      WERD_CHOICE *best_choice,
      void *more_args);
  /// Pointer to go_deeper function that will be modified by various permuters.
  void (Dict::*go_deeper_fxn_)(const char *debug,
                               const BLOB_CHOICE_LIST_VECTOR &char_choices,
                               int char_choice_index,
                               const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                               bool word_ending, WERD_CHOICE *word,
                               float certainties[], float *limit,
                               WERD_CHOICE *best_choice, void *void_more_args);
  /* stopper.cpp *************************************************************/
  int NoDangerousAmbig(WERD_CHOICE *BestChoice,
                       DANGERR *fixpt,
                       bool fix_replaceable,
                       BLOB_CHOICE_LIST_VECTOR *Choices,
                       bool *modified_blobs);
  void ReplaceAmbig(int wrong_ngram_begin_index, int wrong_ngram_size,
                    UNICHAR_ID correct_ngram_id, WERD_CHOICE *werd_choice,
                    BLOB_CHOICE_LIST_VECTOR *blob_choices,
                    bool *modified_blobs);

  inline void DisableChoiceAccum() { keep_word_choices_ = FALSE; }
  inline void EnableChoiceAccum() { keep_word_choices_ = TRUE; }

  int LengthOfShortestAlphaRun(const WERD_CHOICE &WordChoice);
  VIABLE_CHOICE NewViableChoice(const WERD_CHOICE &WordChoice,
                                FLOAT32 AdjustFactor,
                                const float Certainties[]);
  void PrintViableChoice(FILE *File, const char *Label, VIABLE_CHOICE Choice);
  int StringSameAs(const char *String,
                   const char *String_lengths,
                   VIABLE_CHOICE ViableChoice);
  bool StringSameAs(const WERD_CHOICE &WordChoice,
                    VIABLE_CHOICE ViableChoice);
  int AcceptableChoice(BLOB_CHOICE_LIST_VECTOR *Choices,
                       WERD_CHOICE *BestChoice,
                       const WERD_CHOICE &RawChoice,
                       DANGERR *fixpt,
                       ACCEPTABLE_CHOICE_CALLER caller,
                       bool *modified_blobs);
  int AcceptableResult(const WERD_CHOICE &BestChoice,
                       const WERD_CHOICE &RawChoice);
  int ChoiceSameAs(const WERD_CHOICE &WordChoice, VIABLE_CHOICE ViableChoice);
  void LogNewChoice(const WERD_CHOICE &WordChoice, FLOAT32 AdjustFactor,
                    const float Certainties[], bool raw_choice);
  void EndDangerousAmbigs();
  int CurrentBestChoiceIs(const WERD_CHOICE &WordChoice);
  FLOAT32 CurrentBestChoiceAdjustFactor();
  int CurrentWordAmbig();
  void DebugWordChoices();
  void PrintAmbigAlternatives(FILE *file, const char *label,
                              int label_num_unichars);
  void FillViableChoice(const WERD_CHOICE &WordChoice,
                        FLOAT32 AdjustFactor, const float Certainties[],
                        bool SameString, VIABLE_CHOICE ViableChoice);
  int AlternativeChoicesWorseThan(FLOAT32 Threshold);
  void FilterWordChoices();
  void FindClassifierErrors(FLOAT32 MinRating,
                          FLOAT32 MaxRating,
                          FLOAT32 RatingMargin,
                          FLOAT32 Thresholds[]);
  void InitChoiceAccum();
  void LogNewSegmentation(PIECES_STATE BlobWidth);
  void LogNewSplit(int Blob);
  void SettupStopperPass1();
  void SettupStopperPass2();
  /* choices.cpp *************************************************************/
  void print_word_string(const char* str);
  void print_word_choice(const char *label, A_CHOICE* choice);
  void print_choices(const char *label,
                     CHOICES rating);   // List of (A_CHOICE*).
  /* permngram.cpp ***********************************************************/
  A_CHOICE *ngram_permute_and_select(CHOICES_LIST char_choices,
                                     float rating_limit,
                                     const Dawg *dawg);
  /* dawg.cpp ****************************************************************/

  /**
   * Returns the maximal permuter code (from ccstruct/ratngs.h) if in light
   * of the current state the letter at word_index in the given word
   * is allowed according to at least one of the dawgs in dawgs_,
   * otherwise returns NO_PERM.
   *
   * The state is described by void_dawg_args, which are interpreted as
   * DawgArgs and contain two relevant input vectors: active_dawgs and
   * constraints. Each entry in the active_dawgs vector contains an index
   * into the dawgs_ vector and an EDGE_REF that indicates the last edge
   * followed in the dawg. Each entry in the constraints vector contains
   * an index into the dawgs_ vector and an EDGE_REF that indicates an edge
   * in a pattern dawg followed to match a pattern. Currently constraints
   * are used to save the state of punctuation dawgs after leading
   * punctuation was found.
   *
   * Input:
   * At word_index 0 dawg_args->active_dawgs should contain an entry for each
   * dawg whose type has a bit set in kBeginningDawgsType,
   * dawg_args->constraints should be empty. EDGE_REFs in active_dawgs and
   * constraints vectors should be initialized to NO_EDGE. If hyphen state
   * needs to be applied, initial dawg_args->active_dawgs and
   * dawg_args->constrains can be copied from the saved hyphen state
   * (maintained by Dict).
   * For word_index > 0 the corresponding state (active_dawgs and constraints)
   * can be obtained from dawg_args->updated_* passed to def_letter_is_okay
   * for word_index-1.
   * Note: the function assumes that active_dags, constraints and updated_*
   * member variables of dawg_args are not NULL.
   *
   * Output:
   * The function fills in dawg_args->updated_active_dawgs vector with the
   * entries for dawgs that contain the word up to the letter at word_index.
   * The new constraints (if any) are added to dawg_args->updated_constraints,
   * the constraints from dawg_args->constraints are also copied into it.
   *
   * Detailed description:
   * In order to determine whether the word is still valid after considering
   * all the letters up to the one at word_index the following is done for
   * each entry in dawg_args->active_dawgs:
   *
   *  - next starting node is obtained from entry.ref and edge_char_of() is
   *    called to obtain the next edge
   *  - if a valid edge is found, the function returns the updated permuter
   *    code true and an entry [entry.dawg_index, edge] is inserted in
   *    dawg_args->updated_active_dawgs
   *    otherwise:
   *    - if we are dealing with dawg of type DAWG_TYPE_PUNCTUATION,
   *      edge_char_of() is called again, but now with kPatternUnicharID
   *      as unichar_id; if a valid edge is found it is recorded in
   *      dawg_args->updated_constraints
   *    - the function checks whether the word can end with the previous
   *      letter
   *    - each successor of the dawg (e.g. dawgs with type DAWG_TYPE_WORD
   *      could be successors to dawgs with type DAWG_TYPE_PUNCTUATION; the
   *      successors are defined by successors_ vector) is explored and if
   *      a letter is found in the successor dawg, a new entry is inserted
   *      into dawg_args->updated_active_dawgs with EDGE_REF being either
   *      NO_EDGE or an EDGE_REF recorded in constraints vector for the
   *      corresponding dawg index
   */

  int def_letter_is_okay(void* void_dawg_args, int word_index,
                         const void* word, bool word_end);

  int new_letter_is_okay(void* void_dawg_args, int word_index,
                         const void* word, bool word_end);
  int (Dict::*letter_is_okay_)(void* void_dawg_args, int word_index,
                               const void *word, bool word_end);
  /** Return the number of dawgs in the dawgs_ vector. */
  inline const int NumDawgs() const { return dawgs_.size(); }
  /** Return i-th dawg pointer recorded in the dawgs_ vector. */
  inline const Dawg *GetDawg(int index) const { return dawgs_[index]; }
  /**
   * At word ending make sure all the recorded constraints are satisfied.
   * Each constraint signifies that we found a beginning pattern in a
   * pattern dawg. Check that this pattern can end here (e.g. if some
   * leading punctuation is found this would ensure that we are not
   * expecting any particular trailing punctuation after the word).
   */
  inline bool ConstraintsOk(const DawgInfoVector &constraints,
                            int word_end, DawgType current_dawg_type) {
    if (!word_end) return true;
    if (current_dawg_type == DAWG_TYPE_PUNCTUATION) return true;
    for (int c = 0; c < constraints.length(); ++c) {
      const DawgInfo &cinfo = constraints[c];
      Dawg *cdawg = dawgs_[cinfo.dawg_index];
      if (!cdawg->end_of_word(cinfo.ref)) {
        if (dawg_debug_level >= 3) {
          tprintf("Constraint [%d, " REFFORMAT "] is not satisfied\n",
                  cinfo.dawg_index, cinfo.ref);
        }
        return false;
      }
    }
    return true;
  }
  /// Record the maximum of the two permuters in permuter.
  static inline void UpdatePermuter(PermuterType new_permuter,
                                    PermuterType *permuter) {
    if (dawg_debug_level >= 3) tprintf("Letter found\n");
    if (new_permuter > *permuter) *permuter = new_permuter;
  }

  /* conversion.cpp **********************************************************/
  // TODO(daria): remove these function when conversion.cpp is deprecated
  // and all the code is converted to work with unichar ids.
  void LogNewWordChoice(A_CHOICE *a_choice,
                        FLOAT32 adjust_factor,
                        const float certainties[],
                        const UNICHARSET &unicharset);
  int valid_word(const char *string);

 private:
  /** Private member variables. */
  Image* image_ptr_;
  /**
   * Table that stores ambiguities computed during training
   * (loaded when NoDangerousAmbigs() is called for the first time).
   * Each entry i in the table stores a set of amibiguities whose
   * wrong ngram starts with unichar id i.
   */
  UnicharAmbigs *dang_ambigs_table_;
  /** Same as above, but for ambiguities with replace flag set. */
  UnicharAmbigs *replace_ambigs_table_;
  /**
   * Flag used to disable accumulation of word choices
   * during compound word permutation.
   */
  BOOL8 keep_word_choices_;
  /** Additional certainty padding allowed before a word is rejected. */
  FLOAT32 reject_offset_;
  /** Current word segmentation. */
  PIECES_STATE current_segmentation_;
  /** Variables to keep track of best/raw word choices. */
  VIABLE_CHOICE best_raw_choice_;
  LIST raw_choices_;
  LIST best_choices_;
  // Hyphen-related variables.
  UNICHAR_ID hyphen_unichar_id_;
  WERD_CHOICE *hyphen_word_;
  DawgInfoVector hyphen_active_dawgs_;
  DawgInfoVector hyphen_constraints_;
  bool last_word_on_line_;
  // Dawgs.
  DawgVector dawgs_;
  SuccessorListsVector successors_;
  Dawg *freq_dawg_;
  Trie *pending_words_;
  /// The following pointers are only cached for convenience.
  /// The dawgs will be deleted when dawgs_ vector is destroyed.
  // TODO(daria): need to support multiple languages in the future,
  // so maybe will need to maintain a list of dawgs of each kind.
  Trie *document_words_;
};
}  // namespace tesseract

#endif  // THIRD_PARTY_TESSERACT_DICT_DICT_H_
