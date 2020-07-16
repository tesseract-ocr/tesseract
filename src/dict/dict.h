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

#ifdef HAVE_CONFIG_H
#include "config_auto.h" // DISABLED_LEGACY_ENGINE
#endif

#ifndef DISABLED_LEGACY_ENGINE
#include "ambigs.h"
#endif
#include "dawg.h"
#include "dawg_cache.h"
#include "ratngs.h"
#include "stopper.h"
#include "trie.h"
#include "unicharset.h"
#ifndef DISABLED_LEGACY_ENGINE
#include "params_training_featdef.h"
#endif  // ndef DISABLED_LEGACY_ENGINE

class MATRIX;
class WERD_RES;

#define CHARS_PER_LINE 500
#define MAX_WERD_LENGTH        (int64_t) 128
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

using DawgVector = GenericVector<Dawg *>;

//
// Constants
//
static const int kRatingPad = 4;
static const int kDictMaxWildcards = 2;  // max wildcards for a word
// TODO(daria): If hyphens are different in different languages and can be
// inferred from training data we should load their values dynamically.
static const char kHyphenSymbol[] = "-";
static const char kSlashSymbol[] = "/";
static const char kQuestionSymbol[] = "?";
static const char kApostropheSymbol[] = "'";
static const float kSimCertaintyScale = -10.0;   // similarity matcher scaling
static const float kSimCertaintyOffset = -10.0;  // similarity matcher offset
static const float kSimilarityFloor = 100.0;  // worst E*L product to stop on
static const int kDocDictMaxRepChars = 4;

// Enum for describing whether the x-height for the word is consistent:
//  0 - everything is good.
//  1 - there are one or two secondary (but consistent) baselines
//      [think subscript and superscript], or there is an oversized
//      first character.
//  2 - the word is inconsistent.
enum XHeightConsistencyEnum {XH_GOOD, XH_SUBNORMAL, XH_INCONSISTENT};

struct DawgArgs {
  DawgArgs(DawgPositionVector *d, DawgPositionVector *up, PermuterType p)
      : active_dawgs(d), updated_dawgs(up), permuter(p), valid_end(false) {}

  DawgPositionVector *active_dawgs;
  DawgPositionVector *updated_dawgs;
  PermuterType permuter;
  // True if the current position is a valid word end.
  bool valid_end;
};

class Dict {
 public:
  Dict(CCUtil* image_ptr);
  ~Dict();
  const CCUtil* getCCUtil() const {
    return ccutil_;
  }
  CCUtil* getCCUtil() {
    return ccutil_;
  }
  const UNICHARSET& getUnicharset() const {
    return getCCUtil()->unicharset;
  }
  UNICHARSET& getUnicharset() {
    return getCCUtil()->unicharset;
  }
#ifndef DISABLED_LEGACY_ENGINE
  const UnicharAmbigs &getUnicharAmbigs() const {
    return getCCUtil()->unichar_ambigs;
  }
#endif
  // Returns true if unichar_id is a word compounding character like - or /.
  inline bool compound_marker(UNICHAR_ID unichar_id) {
    const UNICHARSET& unicharset = getUnicharset();
    ASSERT_HOST(unicharset.contains_unichar_id(unichar_id));
    const GenericVector<UNICHAR_ID>& normed_ids =
        unicharset.normed_ids(unichar_id);
    return normed_ids.size() == 1 &&
        (normed_ids[0] == hyphen_unichar_id_ ||
         normed_ids[0] == slash_unichar_id_);
  }
  // Returns true if unichar_id is an apostrophe-like character that may
  // separate prefix/suffix words from a main body word.
  inline bool is_apostrophe(UNICHAR_ID unichar_id) {
    const UNICHARSET& unicharset = getUnicharset();
    ASSERT_HOST(unicharset.contains_unichar_id(unichar_id));
    const GenericVector<UNICHAR_ID>& normed_ids =
        unicharset.normed_ids(unichar_id);
    return normed_ids.size() == 1 && normed_ids[0] == apostrophe_unichar_id_;
  }

  /* hyphen.cpp ************************************************************/

  /// Returns true if we've recorded the beginning of a hyphenated word.
  inline bool hyphenated() const { return
    !last_word_on_line_ && hyphen_word_;
  }
  /// Size of the base word (the part on the line before) of a hyphenated word.
  inline int hyphen_base_size() const {
    return this->hyphenated() ? hyphen_word_->length() : 0;
  }
  /// If this word is hyphenated copy the base word (the part on
  /// the line before) of a hyphenated word into the given word.
  /// This function assumes that word is not nullptr.
  inline void copy_hyphen_info(WERD_CHOICE *word) const {
    if (this->hyphenated()) {
      *word = *hyphen_word_;
      if (hyphen_debug_level) word->print("copy_hyphen_info: ");
    }
  }
  /// Check whether the word has a hyphen at the end.
  inline bool has_hyphen_end(const UNICHARSET* unicharset,
                             UNICHAR_ID unichar_id, bool first_pos) const {
    if (!last_word_on_line_ || first_pos)
      return false;
    ASSERT_HOST(unicharset->contains_unichar_id(unichar_id));
    const GenericVector<UNICHAR_ID>& normed_ids =
        unicharset->normed_ids(unichar_id);
    return normed_ids.size() == 1 && normed_ids[0] == hyphen_unichar_id_;
  }
  /// Same as above, but check the unichar at the end of the word.
  inline bool has_hyphen_end(const WERD_CHOICE &word) const {
    int word_index = word.length() - 1;
    return has_hyphen_end(word.unicharset(), word.unichar_id(word_index),
                          word_index == 0);
  }
  /// Unless the previous word was the last one on the line, and the current
  /// one is not (thus it is the first one on the line), erase hyphen_word_,
  /// clear hyphen_active_dawgs_, update last_word_on_line_.
  void reset_hyphen_vars(bool last_word_on_line);
  /// Update hyphen_word_, and copy the given DawgPositionVectors into
  /// hyphen_active_dawgs_ .
  void set_hyphen_word(const WERD_CHOICE &word,
                       const DawgPositionVector &active_dawgs);

  /* permdawg.cpp ************************************************************/
  // Note: Functions in permdawg.cpp are only used by NoDangerousAmbig().
  // When this function is refactored, permdawg.cpp can be removed.

  /// Copies word into best_choice if its rating is smaller
  /// than that of best_choice.
  inline void update_best_choice(const WERD_CHOICE &word,
                                 WERD_CHOICE *best_choice) {
    if (word.rating() < best_choice->rating()) {
      *best_choice = word;
    }
  }
  /// Fill the given active_dawgs vector with dawgs that could contain the
  /// beginning of the word. If hyphenated() returns true, copy the entries
  /// from hyphen_active_dawgs_ instead.
  void init_active_dawgs(DawgPositionVector *active_dawgs,
                         bool ambigs_mode) const;
  // Fill the given vector with the default collection of any-length dawgs
  void default_dawgs(DawgPositionVector *anylength_dawgs,
                               bool suppress_patterns) const;


  /// Recursively explore all the possible character combinations in
  /// the given char_choices. Use go_deeper_dawg_fxn() to explore all the
  /// dawgs in the dawgs_ vector in parallel and discard invalid words.
  ///
  /// Allocate and return a WERD_CHOICE with the best valid word found.
  WERD_CHOICE *dawg_permute_and_select(
      const BLOB_CHOICE_LIST_VECTOR &char_choices, float rating_limit);
  /// If the choice being composed so far could be a dictionary word
  /// and we have not reached the end of the word keep exploring the
  /// char_choices further.
  void go_deeper_dawg_fxn(
      const char *debug, const BLOB_CHOICE_LIST_VECTOR &char_choices,
      int char_choice_index, const CHAR_FRAGMENT_INFO *prev_char_frag_info,
      bool word_ending, WERD_CHOICE *word, float certainties[],
      float *limit, WERD_CHOICE *best_choice, int *attempts_left,
      void *void_more_args);

  /// Pointer to go_deeper function.
  void (Dict::*go_deeper_fxn_)(const char *debug,
                               const BLOB_CHOICE_LIST_VECTOR &char_choices,
                               int char_choice_index,
                               const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                               bool word_ending, WERD_CHOICE *word,
                               float certainties[], float *limit,
                               WERD_CHOICE *best_choice, int *attempts_left,
                               void *void_more_args);
  //
  // Helper functions for dawg_permute_and_select().
  //
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

    bool fragment_state_okay(UNICHAR_ID curr_unichar_id,
                             float curr_rating, float curr_certainty,
                             const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                             const char *debug, int word_ending,
                             CHAR_FRAGMENT_INFO *char_frag_info);

  /* stopper.cpp *************************************************************/
#if !defined(DISABLED_LEGACY_ENGINE)
  bool NoDangerousAmbig(WERD_CHOICE *BestChoice,
                        DANGERR *fixpt,
                        bool fix_replaceable,
                        MATRIX* ratings);
#endif  // !defined(DISABLED_LEGACY_ENGINE)
  // Replaces the corresponding wrong ngram in werd_choice with the correct
  // one. The whole correct n-gram is inserted into the ratings matrix and
  // the werd_choice: no more fragments!. Rating and certainty of new entries
  // in matrix and werd_choice are the sum and mean of the wrong ngram
  // respectively.
  // E.g. for werd_choice mystring'' and ambiguity ''->": werd_choice becomes
  // mystring", with a new entry in the ratings matrix for ".
  void ReplaceAmbig(int wrong_ngram_begin_index, int wrong_ngram_size,
                    UNICHAR_ID correct_ngram_id, WERD_CHOICE *werd_choice,
                    MATRIX *ratings);

  /// Returns the length of the shortest alpha run in WordChoice.
  int LengthOfShortestAlphaRun(const WERD_CHOICE &WordChoice) const;
  /// Returns true if the certainty of the BestChoice word is within a
  /// reasonable range of the average certainties for the best choices for
  /// each character in the segmentation.  This test is used to catch words
  /// in which one character is much worse than the other characters in the
  /// word (i.e. false will be returned in that case). The algorithm computes
  /// the mean and std deviation of the certainties in the word with the worst
  /// certainty thrown out.
  int UniformCertainties(const WERD_CHOICE& word);
  /// Returns true if the given best_choice is good enough to stop.
  bool AcceptableChoice(const WERD_CHOICE& best_choice,
                        XHeightConsistencyEnum xheight_consistency);
  /// Returns false if the best choice for the current word is questionable
  /// and should be tried again on the second pass or should be flagged to
  /// the user.
  bool AcceptableResult(WERD_RES *word) const;
#if !defined(DISABLED_LEGACY_ENGINE)
  void EndDangerousAmbigs();
#endif   // !defined(DISABLED_LEGACY_ENGINE)
  /// Prints the current choices for this word to stdout.
  void DebugWordChoices();
  /// Sets up stopper variables in preparation for the first pass.
  void SettupStopperPass1();
  /// Sets up stopper variables in preparation for the second pass.
  void SettupStopperPass2();
  /* context.cpp *************************************************************/
  /// Check a string to see if it matches a set of lexical rules.
  int case_ok(const WERD_CHOICE& word) const;
  /// Returns true if the word looks like an absolute garbage
  /// (e.g. image mistakenly recognized as text).
  bool absolute_garbage(const WERD_CHOICE &word, const UNICHARSET &unicharset);

  /* dict.cpp ****************************************************************/

  /// Initialize Dict class - load dawgs from [lang].traineddata and
  /// user-specified wordlist and parttern list.
  static TESS_API DawgCache *GlobalDawgCache();
  // Sets up ready for a Load or LoadLSTM.
  void SetupForLoad(DawgCache *dawg_cache);
  // Loads the dawgs needed by Tesseract. Call FinishLoad() after.
  void Load(const STRING &lang, TessdataManager *data_file);
  // Loads the dawgs needed by the LSTM model. Call FinishLoad() after.
  void LoadLSTM(const STRING &lang, TessdataManager *data_file);
  // Completes the loading process after Load() and/or LoadLSTM().
  // Returns false if no dictionaries were loaded.
  bool FinishLoad();
  void End();

  // Resets the document dictionary analogous to ResetAdaptiveClassifier.
  void ResetDocumentDictionary() {
    if (pending_words_ != nullptr)
      pending_words_->clear();
    if (document_words_ != nullptr)
      document_words_->clear();
  }

  /**
   * Returns the maximal permuter code (from ccstruct/ratngs.h) if in light
   * of the current state the letter at word_index in the given word
   * is allowed according to at least one of the dawgs in dawgs_,
   * otherwise returns NO_PERM.
   *
   * The state is described by void_dawg_args, which are interpreted as
   * DawgArgs and contain relevant active dawg positions.
   * Each entry in the active_dawgs vector contains an index
   * into the dawgs_ vector and an EDGE_REF that indicates the last edge
   * followed in the dawg.  It also may contain a position in the punctuation
   * dawg which describes surrounding punctuation (see struct DawgPosition).
   *
   * Input:
   * At word_index 0 dawg_args->active_dawgs should contain an entry for each
   * dawg that may start at the beginning of a word, with punc_ref and edge_ref
   * initialized to NO_EDGE.  Since the punctuation dawg includes the empty
   * pattern " " (meaning anything without surrounding punctuation), having a
   * single entry for the punctuation dawg will cover all dawgs reachable
   * therefrom -- that includes all number and word dawgs. The only dawg
   * non-reachable from the punctuation_dawg is the pattern dawg.
   * If hyphen state needs to be applied, initial dawg_args->active_dawgs can
   * be copied from the saved hyphen state (maintained by Dict).
   * For word_index > 0 the corresponding state (active_dawgs and punc position)
   * can be obtained from dawg_args->updated_dawgs passed to
   * def_letter_is_okay for word_index-1.
   * Note: the function assumes that active_dawgs, and updated_dawgs
   * member variables of dawg_args are not nullptr.
   *
   * Output:
   * The function fills in dawg_args->updated_dawgs vector with the
   * entries for dawgs that contain the word up to the letter at word_index.
   *
   */

  //
  int def_letter_is_okay(void* void_dawg_args, const UNICHARSET& unicharset,
                         UNICHAR_ID unichar_id, bool word_end) const;

  int (Dict::*letter_is_okay_)(void* void_dawg_args,
                               const UNICHARSET& unicharset,
                               UNICHAR_ID unichar_id, bool word_end) const;
  /// Calls letter_is_okay_ member function.
  int LetterIsOkay(void* void_dawg_args, const UNICHARSET& unicharset,
                   UNICHAR_ID unichar_id, bool word_end) const {
    return (this->*letter_is_okay_)(void_dawg_args,
                                    unicharset, unichar_id, word_end);
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
        getCCUtil()->lang.c_str(),
        context, context_bytes,
        character, character_bytes);
  }

  /// Default (no-op) implementation of probability in context function.
  double def_probability_in_context(
      const char* lang, const char* context, int context_bytes,
      const char* character, int character_bytes) {
    (void)lang;
    (void)context;
    (void)context_bytes;
    (void)character;
    (void)character_bytes;
    return 0.0;
  }
  double ngram_probability_in_context(const char* lang,
                                      const char* context,
                                      int context_bytes,
                                      const char* character,
                                      int character_bytes);

  // Interface with params model.
  float (Dict::*params_model_classify_)(const char *lang, void *path);
  float ParamsModelClassify(const char *lang, void *path);
  // Call params_model_classify_ member function.
  float CallParamsModelClassify(void *path) {
    ASSERT_HOST(params_model_classify_ != nullptr);  // ASSERT_HOST -> assert
    return (this->*params_model_classify_)(
        getCCUtil()->lang.c_str(), path);
  }

  inline void SetWildcardID(UNICHAR_ID id) { wildcard_unichar_id_ = id; }
  inline UNICHAR_ID WildcardID() const { return wildcard_unichar_id_; }
  /// Return the number of dawgs in the dawgs_ vector.
  inline int NumDawgs() const { return dawgs_.size(); }
  /// Return i-th dawg pointer recorded in the dawgs_ vector.
  inline const Dawg *GetDawg(int index) const { return dawgs_[index]; }
  /// Return the points to the punctuation dawg.
  inline const Dawg *GetPuncDawg() const { return punc_dawg_; }
  /// Return the points to the unambiguous words dawg.
  inline const Dawg *GetUnambigDawg() const { return unambig_dawg_; }
  /// Returns the appropriate next node given the EDGE_REF.
  static inline NODE_REF GetStartingNode(const Dawg *dawg, EDGE_REF edge_ref) {
    if (edge_ref == NO_EDGE) return 0;  // beginning to explore the dawg
    NODE_REF node = dawg->next_node(edge_ref);
    if (node == 0) node = NO_EDGE;  // end of word
    return node;
  }

  // Given a unichar from a string and a given dawg, return the unichar
  // we should use to match in that dawg type.  (for example, in the number
  // dawg, all numbers are transformed to kPatternUnicharId).
  UNICHAR_ID char_for_dawg(const UNICHARSET& unicharset, UNICHAR_ID ch,
                           const Dawg *dawg) const {
    if (!dawg) return ch;
    switch (dawg->type()) {
      case DAWG_TYPE_NUMBER:
        return unicharset.get_isdigit(ch) ? Dawg::kPatternUnicharID : ch;
      default:
        return ch;
    }
  }

  /// For each of the character classes of the given unichar_id (and the
  /// unichar_id itself) finds the corresponding outgoing node or self-loop
  /// in the given dawg and (after checking that it is valid) records it in
  /// dawg_args->updated_ative_dawgs. Updates current_permuter if any valid
  /// edges were found.
  void ProcessPatternEdges(const Dawg *dawg, const DawgPosition &info,
                           UNICHAR_ID unichar_id, bool word_end,
                           DawgArgs *dawg_args,
                           PermuterType *current_permuter) const;

  /// Read/Write/Access special purpose dawgs which contain words
  /// only of a certain length (used for phrase search for
  /// non-space-delimited languages).

  /// Check all the DAWGs to see if this word is in any of them.
  inline static bool valid_word_permuter(uint8_t perm, bool numbers_ok) {
    return (perm == SYSTEM_DAWG_PERM || perm == FREQ_DAWG_PERM ||
            perm == DOC_DAWG_PERM || perm == USER_DAWG_PERM ||
            perm == USER_PATTERN_PERM || perm == COMPOUND_PERM ||
            (numbers_ok && perm == NUMBER_PERM));
  }
  int valid_word(const WERD_CHOICE &word, bool numbers_ok) const;
  int valid_word(const WERD_CHOICE &word) const {
    return valid_word(word, false);  // return NO_PERM for words with digits
  }
  int valid_word_or_number(const WERD_CHOICE &word) const {
    return valid_word(word, true);  // return NUMBER_PERM for valid numbers
  }
  /// This function is used by api/tesseract_cube_combiner.cpp
  int valid_word(const char *string) const {
    WERD_CHOICE word(string, getUnicharset());
    return valid_word(word);
  }
  // Do the two WERD_CHOICEs form a meaningful bigram?
  bool valid_bigram(const WERD_CHOICE &word1, const WERD_CHOICE &word2) const;
  /// Returns true if the word contains a valid punctuation pattern.
  /// Note: Since the domains of punctuation symbols and symblos
  /// used in numbers are not disjoint, a valid number might contain
  /// an invalid punctuation pattern (e.g. .99).
  bool valid_punctuation(const WERD_CHOICE &word);
  /// Returns true if a good answer is found for the unknown blob rating.
  int good_choice(const WERD_CHOICE &choice);
  /// Adds a word found on this document to the document specific dictionary.
  void add_document_word(const WERD_CHOICE &best_choice);
  /// Adjusts the rating of the given word.
  void adjust_word(WERD_CHOICE *word,
                   bool nonword, XHeightConsistencyEnum xheight_consistency,
                   float additional_adjust,
                   bool modify_rating,
                   bool debug);
  /// Set wordseg_rating_adjust_factor_ to the given value.
  inline void SetWordsegRatingAdjustFactor(float f) {
    wordseg_rating_adjust_factor_ = f;
  }
  /// Returns true if the language is space-delimited (not CJ, or T).
  bool IsSpaceDelimitedLang() const;

 private:
  /** Private member variables. */
  CCUtil* ccutil_;
  /**
   * Table that stores ambiguities computed during training
   * (loaded when NoDangerousAmbigs() is called for the first time).
   * Each entry i in the table stores a set of amibiguities whose
   * wrong ngram starts with unichar id i.
   */
#ifndef DISABLED_LEGACY_ENGINE
  UnicharAmbigs* dang_ambigs_table_ = nullptr;
  /** Same as above, but for ambiguities with replace flag set. */
  UnicharAmbigs* replace_ambigs_table_ = nullptr;
#endif
  /** Additional certainty padding allowed before a word is rejected. */
  float reject_offset_;
  // Cached UNICHAR_IDs:
  UNICHAR_ID wildcard_unichar_id_;    // kDictWildcard.
  UNICHAR_ID apostrophe_unichar_id_;  // kApostropheSymbol.
  UNICHAR_ID question_unichar_id_;    // kQuestionSymbol.
  UNICHAR_ID slash_unichar_id_;       // kSlashSymbol.
  UNICHAR_ID hyphen_unichar_id_;      // kHyphenSymbol.
  // Hyphen-related variables.
  WERD_CHOICE *hyphen_word_;
  DawgPositionVector hyphen_active_dawgs_;
  bool last_word_on_line_;
  // List of lists of "equivalent" UNICHAR_IDs for the purposes of dictionary
  // matching.  The first member of each list is taken as canonical.  For
  // example, the first list contains hyphens and dashes with the first symbol
  // being the ASCII hyphen minus.
  GenericVector<GenericVectorEqEq<UNICHAR_ID> > equivalent_symbols_;
  // Dawg Cache reference - this is who we ask to allocate/deallocate dawgs.
  DawgCache *dawg_cache_;
  bool dawg_cache_is_ours_;  // we should delete our own dawg_cache_
  // Dawgs.
  DawgVector dawgs_;
  SuccessorListsVector successors_;
  Trie *pending_words_;
  /// The following pointers are only cached for convenience.
  /// The dawgs will be deleted when dawgs_ vector is destroyed.
  // bigram_dawg_ points to a dawg of two-word bigrams which always supersede if
  // any of them are present on the best choices list for a word pair.
  // the bigrams are stored as space-separated words where:
  // (1) leading and trailing punctuation has been removed from each word and
  // (2) any digits have been replaced with '?' marks.
  Dawg *bigram_dawg_;
  // TODO(daria): need to support multiple languages in the future,
  // so maybe will need to maintain a list of dawgs of each kind.
  Dawg *freq_dawg_;
  Dawg *unambig_dawg_;
  Dawg *punc_dawg_;
  Trie *document_words_;
  /// Current segmentation cost adjust factor for word rating.
  /// See comments in incorporate_segcost.
  float wordseg_rating_adjust_factor_;
  // File for recording ambiguities discovered during dictionary search.
  FILE *output_ambig_words_file_;

 public:
  /// Variable members.
  /// These have to be declared and initialized after image_ptr_, which contains
  /// the pointer to the params vector - the member of its base CCUtil class.
  STRING_VAR_H(user_words_file, "", "A filename of user-provided words.");
  STRING_VAR_H(user_words_suffix, "",
               "A suffix of user-provided words located in tessdata.");
  STRING_VAR_H(user_patterns_file, "",
               "A filename of user-provided patterns.");
  STRING_VAR_H(user_patterns_suffix, "",
               "A suffix of user-provided patterns located in tessdata.");
  BOOL_VAR_H(load_system_dawg, true, "Load system word dawg.");
  BOOL_VAR_H(load_freq_dawg, true, "Load frequent word dawg.");
  BOOL_VAR_H(load_unambig_dawg, true, "Load unambiguous word dawg.");
  BOOL_VAR_H(load_punc_dawg, true,
             "Load dawg with punctuation patterns.");
  BOOL_VAR_H(load_number_dawg, true, "Load dawg with number patterns.");
  BOOL_VAR_H(load_bigram_dawg, true,
             "Load dawg with special word bigrams.");
  double_VAR_H(xheight_penalty_subscripts, 0.125,
               "Score penalty (0.1 = 10%) added if there are subscripts "
               "or superscripts in a word, but it is otherwise OK.");
  double_VAR_H(xheight_penalty_inconsistent, 0.25,
               "Score penalty (0.1 = 10%) added if an xheight is "
               "inconsistent.");
  double_VAR_H(segment_penalty_dict_frequent_word, 1.0,
               "Score multiplier for word matches which have good case and"
               "are frequent in the given language (lower is better).");

  double_VAR_H(segment_penalty_dict_case_ok, 1.1,
               "Score multiplier for word matches that have good case "
               "(lower is better).");

  double_VAR_H(segment_penalty_dict_case_bad, 1.3125,
               "Default score multiplier for word matches, which may have "
               "case issues (lower is better).");

  double_VAR_H(segment_penalty_dict_nonword, 1.25,
               "Score multiplier for glyph fragment segmentations which "
               "do not match a dictionary word (lower is better).");

  double_VAR_H(segment_penalty_garbage, 1.50,
               "Score multiplier for poorly cased strings that are not in"
               " the dictionary and generally look like garbage (lower is"
               " better).");
  STRING_VAR_H(output_ambig_words_file, "",
               "Output file for ambiguities found in the dictionary");
  INT_VAR_H(dawg_debug_level, 0, "Set to 1 for general debug info"
            ", to 2 for more details, to 3 to see all the debug messages");
  INT_VAR_H(hyphen_debug_level, 0, "Debug level for hyphenated words.");
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
  INT_VAR_H(tessedit_truncate_wordchoice_log, 10, "Max words to keep in list");
  STRING_VAR_H(word_to_debug, "", "Word for which stopper debug information"
               " should be printed to stdout");
  BOOL_VAR_H(segment_nonalphabetic_script, false,
             "Don't use any alphabetic-specific tricks."
             "Set to true in the traineddata config file for"
             " scripts that are cursive or inherently fixed-pitch");
  BOOL_VAR_H(save_doc_words, 0, "Save Document Words");
  double_VAR_H(doc_dict_pending_threshold, 0.0,
               "Worst certainty for using pending dictionary");
  double_VAR_H(doc_dict_certainty_threshold, -2.25, "Worst certainty"
               " for words that can be inserted into the document dictionary");
  INT_VAR_H(max_permuter_attempts, 10000, "Maximum number of different"
              " character choices to consider during permutation."
              " This limit is especially useful when user patterns"
              " are specified, since overly generic patterns can result in"
              " dawg search exploring an overly large number of options.");
};
}  // namespace tesseract

#endif  // THIRD_PARTY_TESSERACT_DICT_DICT_H_
