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
#include "dawg.h"
#include "host.h"
#include "image.h"
#include "oldlist.h"
#include "ratngs.h"
#include "stopper.h"
#include "trie.h"
#include "unicharset.h"
#include "permute.h"

#define MAX_WERD_LENGTH        (inT64) 128
#define NO_RATING               -1

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

//
// Constants
//
static const int kAnyWordLength = -1;
static const int kRatingPad = 4;
// TODO(daria): If hyphens are different in different languages and can be
// inferred from training data we should load their values dynamically.
static const char kHyphenSymbol[] = "-";
static const int kMaxNumDawgEdgees = 2000000;
static const int kMaxDocDawgEdges = 250000;
static const int kMaxUserDawgEdges = 50000;
static const float kSimCertaintyScale = -10.0;   // similarity matcher scaling
static const float kSimCertaintyOffset = -10.0;  // similarity matcher offset
static const float kSimilarityFloor = 100.0;  // worst E*L product to stop on
static const int kDocDictMaxRepChars = 4;

struct DawgArgs {
  DawgArgs(DawgInfoVector *d, DawgInfoVector *c, DawgInfoVector *ud,
           DawgInfoVector *uc, float r, PermuterType p, int len, int e) :
    active_dawgs(d), constraints(c), updated_active_dawgs(ud),
    updated_constraints(uc), rating_margin(r) {
    for (int i = 0; i < MAX_WERD_LENGTH; ++i) {
      rating_array[i] = NO_RATING;
    }
    permuter = p;
    sought_word_length = len;
    end_char_choice_index = e;
  }
  DawgInfoVector *active_dawgs;
  DawgInfoVector *constraints;
  DawgInfoVector *updated_active_dawgs;
  DawgInfoVector *updated_constraints;
  PermuterType permuter;
  int sought_word_length;

  // TODO(daria): remove these fields when permdawg is deprecated.
  float rating_margin;  /**< pruning margin ratio */
  float rating_array[MAX_WERD_LENGTH];
  int end_char_choice_index;
};

class Dict {
 public:
  // Gain factor for ambiguity threshold.
  static const float kStopperAmbiguityThresholdGain;
  // Certainty offset for ambiguity threshold.
  static const float kStopperAmbiguityThresholdOffset;

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

  inline bool compound_marker(UNICHAR_ID unichar_id) {
    return (unichar_id == getUnicharset().unichar_to_id("-") ||
            unichar_id == getUnicharset().unichar_to_id("/"));
  }

  /* hyphen.cpp ************************************************************/

  /// Returns true if we've recorded the beginning of a hyphenated word.
  inline bool hyphenated() { return
    !last_word_on_line_ && hyphen_word_ && GetMaxFixedLengthDawgIndex() < 0;
  }
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
  inline bool has_hyphen_end(UNICHAR_ID unichar_id, bool first_pos) {
    return (last_word_on_line_ && !first_pos &&
            unichar_id == hyphen_unichar_id_);
  }
  /// Same as above, but check the unichar at the end of the word.
  inline bool has_hyphen_end(const WERD_CHOICE &word) {
    int word_index = word.length() - 1;
    return has_hyphen_end(word.unichar_id(word_index), word_index == 0);
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
  /// Copies word into best_choice if its rating is smaller
  /// than that of best_choice.
  inline void update_best_choice(const WERD_CHOICE &word,
                                 WERD_CHOICE *best_choice) {
    if (word.rating() < best_choice->rating()) *best_choice = word;
  }
  /// Fill the given active_dawgs vector with dawgs that could contain the
  /// beginning of the word. If hyphenated() returns true, copy the entries
  /// from hyphen_active_dawgs_ instead.
  void init_active_dawgs(int sought_word_length,
                         DawgInfoVector *active_dawgs,
                         bool ambigs_mode);
  /// If hyphenated() returns true, copy the entries from hyphen_constraints_
  /// into the given constraints vector.
  void init_constraints(DawgInfoVector *constraints);
  /// Returns true if we are operating in ambigs mode.
  inline bool ambigs_mode(float rating_limit) { return rating_limit <= 0.0; }
  /// Recursively explore all the possible character combinations in
  /// the given char_choices. Use go_deeper_dawg_fxn() to explore all the
  /// dawgs in the dawgs_ vector in parallel and discard invalid words.
  ///
  /// Allocate and return a WERD_CHOICE with the best valid word found.
  WERD_CHOICE *dawg_permute_and_select(
      const BLOB_CHOICE_LIST_VECTOR &char_choices, float rating_limit,
      int sought_word_length, int end_char_choice_index);
  WERD_CHOICE *dawg_permute_and_select(
      const BLOB_CHOICE_LIST_VECTOR &char_choices, float rating_limit) {
    return dawg_permute_and_select(char_choices, rating_limit,
                                   kAnyWordLength, 0);
  }
  /// If the choice being composed so far could be a dictionary word
  /// and we have not reached the end of the word keep exploring the
  /// char_choices further.
  /// Also:
  /// -- sets hyphen word if needed
  /// -- if word_ending is true and the word is better than best_choice,
  ///    copies word to best_choice and logs new word choice
  void go_deeper_dawg_fxn(
      const char *debug, const BLOB_CHOICE_LIST_VECTOR &char_choices,
      int char_choice_index, const CHAR_FRAGMENT_INFO *prev_char_frag_info,
      bool word_ending, WERD_CHOICE *word, float certainties[],
      float *limit, WERD_CHOICE *best_choice, int *attempts_left,
      void *void_more_args);

  /* permute.cpp *************************************************************/
  WERD_CHOICE *get_top_choice_word(
      const BLOB_CHOICE_LIST_VECTOR &char_choices);
  WERD_CHOICE *permute_top_choice(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    float* rating_limit,
    WERD_CHOICE *raw_choice,
    BOOL8 *any_alpha);
  const char* choose_il1(const char *first_char,       //first choice
                         const char *second_char,      //second choice
                         const char *third_char,       //third choice
                         const char *prev_char,        //prev in word
                         const char *next_char,        //next in word
                         const char *next_next_char);  //after next next in word
  WERD_CHOICE *permute_all(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                           const WERD_CHOICE *best_choice,
                           WERD_CHOICE *raw_choice);
  void end_permute();
  void permute_subword(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                       float rating_limit,
                       int start,
                       int end,
                       WERD_CHOICE *current_word);
  bool permute_characters(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                          WERD_CHOICE *best_choice,
                          WERD_CHOICE *raw_choice);
  WERD_CHOICE *permute_compound_words(
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      float rating_limit);
  /// Find permutations matching a list of fixed-char-length dawgs
  /// The bestchoice based on this permuter alone is returned. Alternatively,
  /// non-conflicting changes can be combined through permuter_state.
  WERD_CHOICE *permute_fixed_length_words(
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      PermuterState *permuter_state);
  /// Incoporate segmentation cost into word rating
  void incorporate_segcost(WERD_CHOICE* word);
  /// Checks for script-consistent permutations.  Similar to fixed-length
  /// permuter, the best choice is returned by the function, but the combined
  /// changes are also recorded into permuter_state.
  WERD_CHOICE *permute_script_words(
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      PermuterState *permuter_state);
  /// checks for consistency in character property (eg. alpah, digit, punct)
  WERD_CHOICE *permute_chartype_words(
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      PermuterState *permuter_state);

  /// Look up the main chartype for each character position and store it in
  /// the given array.  Also returns the dominant type from unambiguous top
  /// choices.
  char top_word_chartype(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                         char* pos_chartypes);

  WERD_CHOICE *top_fragments_permute_and_select(
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      float rating_limit);
  /// While the choice being composed so far could be better
  /// than best_choice keeps exploring char_choices.
  /// If the end of the word is reached and the word is better than
  /// best_choice, copies word to best_choice and logs the new word choice.
  void go_deeper_top_fragments_fxn(
      const char *debug, const BLOB_CHOICE_LIST_VECTOR &char_choices,
      int char_choice_index, const CHAR_FRAGMENT_INFO *prev_char_frag_info,
      bool word_ending, WERD_CHOICE *word, float certainties[], float *limit,
      WERD_CHOICE *best_choice, int *attempts_left, void *more_args);

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
      int *attempts_left,
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
      int *attempts_left,
      void *more_args);
  /// Pointer to go_deeper function that will be modified by various permuters.
  void (Dict::*go_deeper_fxn_)(const char *debug,
                               const BLOB_CHOICE_LIST_VECTOR &char_choices,
                               int char_choice_index,
                               const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                               bool word_ending, WERD_CHOICE *word,
                               float certainties[], float *limit,
                               WERD_CHOICE *best_choice, int *attempts_left,
                               void *void_more_args);
  /* stopper.cpp *************************************************************/
  bool NoDangerousAmbig(WERD_CHOICE *BestChoice,
                        DANGERR *fixpt,
                        bool fix_replaceable,
                        BLOB_CHOICE_LIST_VECTOR *Choices,
                        bool *modified_blobs);
  /// Replaces the corresponding wrong ngram in werd_choice with the correct
  /// one. We indicate that this newly inserted ngram unichar is composed from
  /// several fragments and modify the corresponding entries in blob_choices to
  /// contain fragments of the correct ngram unichar instead of the original
  /// unichars. Ratings and certainties of entries in blob_choices and
  /// werd_choice are unichaged. E.g. for werd_choice mystring'' and ambiguity
  /// ''->": werd_choice becomes mystring", first ' in blob_choices becomes
  /// |"|0|2, second one is set to |"|1|2.
  void ReplaceAmbig(int wrong_ngram_begin_index, int wrong_ngram_size,
                    UNICHAR_ID correct_ngram_id, WERD_CHOICE *werd_choice,
                    BLOB_CHOICE_LIST_VECTOR *blob_choices,
                    bool *modified_blobs);

  inline void DisableChoiceAccum() { keep_word_choices_ = false; }
  inline void EnableChoiceAccum() { keep_word_choices_ = true; }
  inline bool ChoiceAccumEnabled() { return keep_word_choices_; }

  /// Returns the length of the shortest alpha run in WordChoice.
  int LengthOfShortestAlphaRun(const WERD_CHOICE &WordChoice);
  /// Allocates a new viable choice data structure, copies WordChoice,
  /// Certainties, and current_segmentation_ into it, returns a pointer to
  /// the newly created VIABLE_CHOICE.
  /// WordChoice is a choice to be converted to a viable choice.
  /// AdjustFactor is a factor used to adjust ratings for WordChoice.
  /// Certainties contain certainty for each character in WordChoice.
  VIABLE_CHOICE NewViableChoice(const WERD_CHOICE &WordChoice,
                                FLOAT32 AdjustFactor,
                                const float Certainties[]);
  /// Dumps a text representation of the specified Choice to File.
  void PrintViableChoice(FILE *File, const char *Label, VIABLE_CHOICE Choice);
  /// Compares unichar ids in word_choice to those in viable_choice,
  /// returns true if they are the same.
  bool StringSameAs(const WERD_CHOICE &WordChoice,
                    VIABLE_CHOICE ViableChoice);
  /// Compares String to ViableChoice and returns true if they are the same.
  bool StringSameAs(const char *String,
                    const char *String_lengths,
                    VIABLE_CHOICE ViableChoice);
  /// Returns true if the certainty of the BestChoice word is within a
  /// reasonable range of the average certainties for the best choices for
  /// each character in the segmentation.  This test is used to catch words
  /// in which one character is much worse than the other characters in the
  /// word (i.e. false will be returned in that case). The algorithm computes
  /// the mean and std deviation of the certainties in the word with the worst
  /// certainty thrown out.
  int UniformCertainties(const BLOB_CHOICE_LIST_VECTOR &Choices,
                         const WERD_CHOICE &BestChoice);
  /// Returns true if the given best_choice is good enough to stop.
  bool AcceptableChoice(BLOB_CHOICE_LIST_VECTOR *Choices,
                        WERD_CHOICE *BestChoice,
                        DANGERR *fixpt,
                        ACCEPTABLE_CHOICE_CALLER caller,
                        bool *modified_blobs);
  /// Returns false if the best choice for the current word is questionable
  /// and should be tried again on the second pass or should be flagged to
  /// the user.
  bool AcceptableResult(const WERD_CHOICE &BestChoice);
  /// Compares the corresponding strings of WordChoice and ViableChoice and
  /// returns true if they are the same.
  int ChoiceSameAs(const WERD_CHOICE &WordChoice, VIABLE_CHOICE ViableChoice);
  /// Adds Choice to ChoicesList if the adjusted certainty for Choice is within
  /// a reasonable range of the best choice in ChoicesList. The ChoicesList list
  /// is kept in sorted order by rating. Duplicates are removed.
  /// WordChoice is the new choice for current word.
  /// AdjustFactor is an adjustment factor which was applied to choice.
  /// Certainties are certainties for each char in new choice.
  /// raw_choice indicates whether WordChoice is a raw or best choice.
  void LogNewChoice(FLOAT32 AdjustFactor, const float Certainties[],
                    bool raw_choice, WERD_CHOICE *WordChoice);
  void EndDangerousAmbigs();
  /// Returns true if WordChoice is the same as the current best choice.
  bool CurrentBestChoiceIs(const WERD_CHOICE &WordChoice);
  /// Returns the adjustment factor for the best choice for the current word.
  FLOAT32 CurrentBestChoiceAdjustFactor();
  /// Returns true if there are multiple good choices for the current word.
  bool CurrentWordAmbig();
  /// Prints the current choices for this word to stdout.
  void DebugWordChoices();
  /// Print all the choices in raw_choices_ list for non 1-1 ambiguities.
  void PrintAmbigAlternatives(FILE *file, const char *label,
                              int label_num_unichars);
  /// Fill ViableChoice with information from WordChoice, AChoice, AdjustFactor,
  /// and Certainties.
  void FillViableChoice(const WERD_CHOICE &WordChoice,
                        FLOAT32 AdjustFactor, const float Certainties[],
                        bool SameString, VIABLE_CHOICE ViableChoice);
  /// Returns true if there are no alternative choices for the current word
  /// or if all alternatives have an adjust factor worse than Threshold.
  bool AlternativeChoicesWorseThan(FLOAT32 Threshold);
  /// Removes from best_choices_ all choices which are not within a reasonable
  /// range of the best choice.
  void FilterWordChoices();
  /// Compares the best choice for the current word to the best raw choice
  /// to determine which characters were classified incorrectly by the
  /// classifier. Then places a separate threshold into Thresholds for each
  /// character in the word. If the classifier was correct, MaxRating is placed
  /// into Thresholds. If the classifier was incorrect, the avg. match rating
  /// (error percentage) of the classifier's incorrect choice minus some margin
  /// is placed into thresholds.This can then be used by the caller to try to
  /// create a new template for the desired class that will classify the
  /// character with a rating better than the threshold value. The match rating
  /// placed into Thresholds is never allowed to be below MinRating in order to
  /// prevent trying to make overly tight templates.
  /// MinRating limits how tight to make a template.
  /// MaxRating limits how loose to make a template.
  /// RatingMargin denotes the amount of margin to put in template.
  void FindClassifierErrors(FLOAT32 MinRating,
                            FLOAT32 MaxRating,
                            FLOAT32 RatingMargin,
                            FLOAT32 Thresholds[]);
  /// Initializes the data structures used to keep track the good word choices
  /// found for a word.
  void InitChoiceAccum();
  /// Clears best_choices_ list accumulated by the stopper.
  void ClearBestChoiceAccum();
  /// Updates the blob widths in current_segmentation_ to be the same as
  /// provided in BlobWidth. BlobWidth[] contains the number of chunks in each
  /// blob in the current segmentation.
  void LogNewSegmentation(PIECES_STATE BlobWidth);
  /// Given Blob (the index of the blob that was split), adds 1 chunk to the
  /// specified blob for each choice in best_choices_ and for best_raw_choice_.
  void LogNewSplit(int Blob);
  /// Increments the chunk count of the character in Choice which corresponds
  /// to Blob (index of the blob being split).
  void AddNewChunk(VIABLE_CHOICE Choice, int Blob);
  /// Sets up stopper variables in preparation for the first pass.
  void SettupStopperPass1();
  /// Sets up stopper variables in preparation for the second pass.
  void SettupStopperPass2();
  /* context.cpp *************************************************************/
  /// Check a string to see if it matches a set of lexical rules.
  int case_ok(const WERD_CHOICE &word, const UNICHARSET &unicharset);
  /// Returns true if the word looks like an absolute garbage
  /// (e.g. image mistakenly recognized as text).
  bool absolute_garbage(const WERD_CHOICE &word, const UNICHARSET &unicharset);

  /* dict.cpp ****************************************************************/

  /// Initialize Dict class - load dawgs from [lang].traineddata and
  /// user-specified wordlist and parttern list.
  void Load();
  void End();

  // Resets the document dictionary analogous to ResetAdaptiveClassifier.
  void ResetDocumentDictionary() {
    if (pending_words_ != NULL)
      pending_words_->clear();
    if (document_words_ != NULL)
      document_words_->clear();
  }

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

  //
  int def_letter_is_okay(void* void_dawg_args,
                         UNICHAR_ID unichar_id, bool word_end);

  int (Dict::*letter_is_okay_)(void* void_dawg_args,
                               UNICHAR_ID unichar_id, bool word_end);
  /// Calls letter_is_okay_ member function.
  int LetterIsOkay(void* void_dawg_args,
                   UNICHAR_ID unichar_id, bool word_end) {
    return (this->*letter_is_okay_)(void_dawg_args, unichar_id, word_end);
  }


  /// Probability in context function used by the ngram permuter.
  double (Dict::*probability_in_context_)(const char* lang,
                                          const char* context,
                                          int context_bytes,
                                          const char* character,
                                          int character_bytes);
  /// Calls probability_in_context_ member function.
  double ProbabilityInContext(const char* context,
                              int context_bytes,
                              const char* character,
                              int character_bytes) {
    return (this->*probability_in_context_)(
        getImage()->getCCUtil()->lang.string(),
        context, context_bytes,
        character, character_bytes);
  }

  /// Default (no-op) implementation of probability in context function.
  double def_probability_in_context(
      const char* lang, const char* context, int context_bytes,
      const char* character, int character_bytes) {
    (void) context;
    (void) context_bytes;
    (void) character;
    (void) character_bytes;
    return 0.0;
  }
  double ngram_probability_in_context(const char* lang,
                                      const char* context,
                                      int context_bytes,
                                      const char* character,
                                      int character_bytes);

  /// Return the number of dawgs in the dawgs_ vector.
  inline const int NumDawgs() const { return dawgs_.size(); }
  /// Return i-th dawg pointer recorded in the dawgs_ vector.
  inline const Dawg *GetDawg(int index) const { return dawgs_[index]; }
  /// Return the points to the punctuation dawg.
  inline const Dawg *GetPuncDawg() const { return punc_dawg_; }
  /// Return the pointer to the Dawg that contains words of length word_length.
  inline const Dawg *GetFixedLengthDawg(int word_length) const {
    if (word_length > max_fixed_length_dawgs_wdlen_) return NULL;
    assert(dawgs_.size() > word_length);
    return dawgs_[word_length];
  }
  inline const int GetMaxFixedLengthDawgIndex() const {
    return max_fixed_length_dawgs_wdlen_;
  }
  /// Returns the appropriate next node given the EDGE_REF.
  static inline NODE_REF GetStartingNode(const Dawg *dawg, EDGE_REF edge_ref) {
    if (edge_ref == NO_EDGE) return 0;  // beginning to explore the dawg
    NODE_REF node = dawg->next_node(edge_ref);
    if (node == 0) node = NO_EDGE;  // end of word
    return node;
  }
  /// At word ending make sure all the recorded constraints are satisfied.
  /// Each constraint signifies that we found a beginning pattern in a
  /// pattern dawg. Check that this pattern can end here (e.g. if some
  /// leading punctuation is found this would ensure that we are not
  /// expecting any particular trailing punctuation after the word).
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

  /// For each of the character classes of the given unichar_id (and the
  /// unichar_id itself) finds the corresponding outgoing node or self-loop
  /// in the given dawg and (after checking that it is valid) records it in
  /// dawg_args->updated_ative_dawgs. Updates current_permuter if any valid
  /// edges were found.
  void ProcessPatternEdges(const Dawg *dawg, const DawgInfo &info,
                           UNICHAR_ID unichar_id, bool word_end,
                           DawgArgs *dawg_args, PermuterType *current_permuter);

  /// Read/Write/Access special purpose dawgs which contain words
  /// only of a certain length (used for phrase search for
  /// non-space-delimited languages).

  /// Reads a sequence of dawgs from the given file.
  /// Appends the constructed dawgs to the given dawg_vec.
  /// Fills the given table with indices of the dawgs in the
  /// dawg_vec corresponding to the dawgs with words
  /// of a particular length.
  static void ReadFixedLengthDawgs(DawgType type, const STRING &lang,
                                   PermuterType perm, int debug_level,
                                   FILE *file, DawgVector *dawg_vec,
                                   int *max_wdlen);
  /// Writes the dawgs in the dawgs_vec to a file. Updates the given table with
  /// the indices of dawgs in the dawg_vec for the corresponding word lengths.
  static void WriteFixedLengthDawgs(
      const GenericVector<SquishedDawg *> &dawg_vec,
      int num_dawgs, int debug_level, FILE *output_file);

  /// Check all the DAWGs to see if this word is in any of them.
  inline bool valid_word_permuter(uinT8 perm, bool numbers_ok) {
    return (perm == SYSTEM_DAWG_PERM || perm == FREQ_DAWG_PERM ||
            perm == DOC_DAWG_PERM || perm == USER_DAWG_PERM ||
            perm == USER_PATTERN_PERM || (numbers_ok && perm == NUMBER_PERM));
  }
  int valid_word(const WERD_CHOICE &word, bool numbers_ok);
  int valid_word(const WERD_CHOICE &word) {
    return valid_word(word, false);  // return NO_PERM for words with digits
  }
  int valid_word_or_number(const WERD_CHOICE &word) {
    return valid_word(word, true);  // return NUMBER_PERM for valid numbers
  }
  /// This function is used by api/tesseract_cube_combiner.cpp
  int valid_word(const char *string) {
    WERD_CHOICE word(string, getUnicharset());
    return valid_word(word);
  }
  /// Returns true if the word contains a valid punctuation pattern.
  /// Note: Since the domains of punctuation symbols and symblos
  /// used in numbers are not disjoint, a valid number might contain
  /// an invalid punctuation pattern (e.g. .99).
  bool valid_punctuation(const WERD_CHOICE &word);
  /// Returns true if a good answer is found for the unknown blob rating.
  int good_choice(const WERD_CHOICE &choice);
  /// Adds a word found on this document to the document specific dictionary.
  void add_document_word(const WERD_CHOICE &best_choice);
  int get_top_word_script(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                          const UNICHARSET &unicharset);
  /// Adjusts the rating of the given word.
  void adjust_word(WERD_CHOICE *word, float *certainty_array,
                   const BLOB_CHOICE_LIST_VECTOR *char_choices,
                   bool nonword, float additional_adjust, bool debug);
  void adjust_word(WERD_CHOICE *word, float *certainty_array, bool debug) {
    adjust_word(word, certainty_array, NULL, false, 0.0f, debug);
  }
  void adjust_non_word(WERD_CHOICE *word, float *certainty_array, bool debug) {
    adjust_word(word, certainty_array, NULL, true, 0.0f, debug);
  }
  /// Set wordseg_rating_adjust_factor_ to the given value.
  inline void SetWordsegRatingAdjustFactor(float f) {
    wordseg_rating_adjust_factor_ = f;
  }

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
  bool keep_word_choices_;
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
  Trie *pending_words_;
  /// The following pointers are only cached for convenience.
  /// The dawgs will be deleted when dawgs_ vector is destroyed.
  // TODO(daria): need to support multiple languages in the future,
  // so maybe will need to maintain a list of dawgs of each kind.
  Dawg *freq_dawg_;
  Dawg *punc_dawg_;
  Trie *document_words_;
  /// Maximum word length of fixed-length word dawgs.
  /// A value < 1 indicates that no fixed-length dawgs are loaded.
  int max_fixed_length_dawgs_wdlen_;
  /// Current segmentation cost adjust factor for word rating.
  /// See comments in incorporate_segcost.
  float wordseg_rating_adjust_factor_;

 public:
  /// Variable members.
  /// These have to be declared and initialized after image_ptr_, which contains
  /// the pointer to the params vector - the member of its base CCUtil class.
  STRING_VAR_H(user_words_suffix, "", "A list of user-provided words.");
  STRING_VAR_H(user_patterns_suffix, "",
               "A list of user-provided patterns.");
  BOOL_VAR_H(load_system_dawg, true, "Load system word dawg.");
  BOOL_VAR_H(load_freq_dawg, true, "Load frequent word dawg.");
  BOOL_VAR_H(load_punc_dawg, true,
             "Load dawg with punctuation patterns.");
  BOOL_VAR_H(load_number_dawg, true, "Load dawg with number patterns.");
  BOOL_VAR_H(load_fixed_length_dawgs, true,  "Load fixed length"
             " dawgs (e.g. for non-space delimited languages)");
  double_VAR_H(segment_penalty_dict_frequent_word, 1.0,
               "Score multiplier for word matches which have good case and"
               "are frequent in the given language (lower is better).");

  double_VAR_H(segment_penalty_dict_case_ok, 1.1,
               "Score multiplier for word matches that have good case "
               "(lower is better).");

  double_VAR_H(segment_penalty_dict_case_bad, 1.3125,
               "Default score multiplier for word matches, which may have "
               "case issues (lower is better).");

  // TODO(daria): remove this param when ngram permuter is deprecated.
  double_VAR_H(segment_penalty_ngram_best_choice, 1.24,
               "Multipler to for the best choice from the ngram model.");

  double_VAR_H(segment_penalty_dict_nonword, 1.25,
               "Score multiplier for glyph fragment segmentations which "
               "do not match a dictionary word (lower is better).");

  double_VAR_H(segment_penalty_garbage, 1.50,
               "Score multiplier for poorly cased strings that are not in"
               " the dictionary and generally look like garbage (lower is"
               " better).");
  INT_VAR_H(dawg_debug_level, 0, "Set to 1 for general debug info"
            ", to 2 for more details, to 3 to see all the debug messages");
  INT_VAR_H(hyphen_debug_level, 0, "Debug level for hyphenated words.");
  INT_VAR_H(max_viterbi_list_size, 10, "Maximum size of viterbi list.");
  BOOL_VAR_H(use_only_first_uft8_step, false,
             "Use only the first UTF8 step of the given string"
             " when computing log probabilities.");
  double_VAR_H(certainty_scale, 20.0, "Certainty scaling factor");
  double_VAR_H(stopper_nondict_certainty_base, -2.50,
               "Certainty threshold for non-dict words");
  double_VAR_H(stopper_phase2_certainty_rejection_offset, 1.0,
               "Reject certainty offset");
  INT_VAR_H(stopper_smallword_size, 2,
            "Size of dict word to be treated as non-dict word");
  double_VAR_H(stopper_certainty_per_char, -0.50,
               "Certainty to add for each dict char above small word size.");
  double_VAR_H(stopper_allowable_character_badness, 3.0,
               "Max certaintly variation allowed in a word (in sigma)");
  INT_VAR_H(stopper_debug_level, 0, "Stopper debug level");
  BOOL_VAR_H(stopper_no_acceptable_choices, false,
             "Make AcceptableChoice() always return false. Useful"
             " when there is a need to explore all segmentations");
  BOOL_VAR_H(save_raw_choices, false, "Save all explored raw choices");
  INT_VAR_H(tessedit_truncate_wordchoice_log, 10, "Max words to keep in list");
  STRING_VAR_H(word_to_debug, "", "Word for which stopper debug information"
               " should be printed to stdout");
  STRING_VAR_H(word_to_debug_lengths, "",
               "Lengths of unichars in word_to_debug");
  INT_VAR_H(fragments_debug, 0, "Debug character fragments");
  INT_VAR_H(segment_debug, 0, "Debug the whole segmentation process");
  BOOL_VAR_H(permute_debug, 0, "Debug char permutation process");
  double_VAR_H(bestrate_pruning_factor, 2.0, "Multiplying factor of"
               " current best rate to prune other hypotheses");
  BOOL_VAR_H(permute_script_word, 0,
             "Turn on word script consistency permuter");
  BOOL_VAR_H(segment_segcost_rating, 0,
             "incorporate segmentation cost in word rating?");
  double_VAR_H(segment_reward_script, 0.95,
               "Score multipler for script consistency within a word. "
               "Being a 'reward' factor, it should be <= 1. "
               "Smaller value implies bigger reward.");
  BOOL_VAR_H(permute_fixed_length_dawg, 0,
             "Turn on fixed-length phrasebook search permuter");
  BOOL_VAR_H(permute_chartype_word, 0,
             "Turn on character type (property) consistency permuter");
  double_VAR_H(segment_reward_chartype, 0.97,
               "Score multipler for char type consistency within a word. ");
  // TODO(daria): remove this param when ngram permuter is deprecated.
  double_VAR_H(segment_reward_ngram_best_choice, 0.99,
               "Score multipler for ngram permuter's best choice"
               " (only used in the Han script path).");
  BOOL_VAR_H(save_doc_words, 0, "Save Document Words");
  BOOL_VAR_H(doc_dict_enable, 1, "Enable Document Dictionary ");
  double_VAR_H(doc_dict_pending_threshold, 0.0,
               "Worst certainty for using pending dictionary");
  double_VAR_H(doc_dict_certainty_threshold, -2.25, "Worst certainty"
               " for words that can be inserted into the document dictionary");
  BOOL_VAR_H(ngram_permuter_activated, false,
             "Activate character-level n-gram-based permuter");
  INT_VAR_H(max_permuter_attempts, 10000, "Maximum number of different"
            " character choices to consider during permutation."
            " This limit is especially useful when user patterns"
            " are specified, since overly generic patterns can result in"
            " dawg search exploring an overly large number of options.");
  BOOL_VAR_H(permute_only_top, false, "Run only the top choice permuter");
};
}  // namespace tesseract

#endif  // THIRD_PARTY_TESSERACT_DICT_DICT_H_
