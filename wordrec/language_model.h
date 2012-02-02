///////////////////////////////////////////////////////////////////////
// File:        language_model.h
// Description: Functions that utilize the knowledge about the properties,
//              structure and statistics of the language to help recognition.
// Author:      Daria Antonova
// Created:     Mon Nov 11 11:26:43 PST 2009
//
// (C) Copyright 2009, Google Inc.
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

#ifndef TESSERACT_WORDREC_LANGUAGE_MODEL_H_
#define TESSERACT_WORDREC_LANGUAGE_MODEL_H_

#include "associate.h"
#include "dawg.h"
#include "dict.h"
#include "fontinfo.h"
#include "intproto.h"
#include "matrix.h"
#include "oldheap.h"
#include "params.h"
#include "pageres.h"

namespace tesseract {

// Used for expressing various language model flags.
typedef unsigned char LanguageModelFlagsType;

// Struct for keeping track of the consistency of the path.
struct LanguageModelConsistencyInfo {
  LanguageModelConsistencyInfo()
    : punc_ref(NO_EDGE), num_punc(0), invalid_punc(false),
      num_non_first_upper(0), num_lower(0),
      script_id(0), inconsistent_script(false),
      num_alphas(0), num_digits(0), num_other(0),
      num_inconsistent_spaces(0), inconsistent_font(false) {}
  inline int NumInconsistentPunc() const {
    return invalid_punc ? num_punc : 0;
  }
  inline int NumInconsistentCase() const {
    return (num_non_first_upper > num_lower) ? num_lower : num_non_first_upper;
  }
  inline int NumInconsistentChartype() const {
    return (NumInconsistentPunc() + num_other +
            ((num_alphas > num_digits) ? num_digits : num_alphas));
  }
  inline bool Consistent() const {
    return (NumInconsistentPunc() == 0 && NumInconsistentCase() == 0 &&
            NumInconsistentChartype() == 0 && !inconsistent_script);
  }
  inline int  NumInconsistentSpaces() const {
    return num_inconsistent_spaces;
  }

  EDGE_REF punc_ref;
  int num_punc;
  bool invalid_punc;
  int num_non_first_upper;
  int num_lower;
  int script_id;
  bool inconsistent_script;
  int num_alphas;
  int num_digits;
  int num_other;
  int num_inconsistent_spaces;
  bool inconsistent_font;
};


// The following structs are used for storing the state of the language model
// in the segmentation search graph. In this graph the nodes are BLOB_CHOICEs
// and the links are the replationships between the underlying blobs (see
// segsearch.h for a more detailed description).
// Each of the BLOB_CHOICEs contains LanguageModelState struct, which has
// a list of N best paths (list of ViterbiStateEntry) explored by the Viterbi
// search leading up to and including this BLOB_CHOICE.
// Each ViterbiStateEntry contains information from various components of the
// language model: dawgs in which the path is found, character ngram model
// probability of the path, script/chartype/font consistency info, state for
// language-specific heuristics (e.g. hyphenated and compund words, lower/upper
// case preferences, etc).
// Each ViterbiStateEntry also contains the parent pointer, so that the path
// that it represents (WERD_CHOICE) can be constructed by following these
// parent pointers.

// Struct for storing additional information used by Dawg language model
// component. It stores the set of active dawgs in which the sequence of
// letters on a path can be found and the constraints that have to be
// satisfied at the end of the word (e.g. beginning/ending punctuation).
struct LanguageModelDawgInfo {
  LanguageModelDawgInfo(DawgInfoVector *a, DawgInfoVector *c,
                        PermuterType pt) : permuter(pt) {
    active_dawgs = new DawgInfoVector(*a);
    constraints = new DawgInfoVector(*c);
  }
  ~LanguageModelDawgInfo() {
    delete active_dawgs;
    delete constraints;
  }
  DawgInfoVector *active_dawgs;
  DawgInfoVector *constraints;
  PermuterType permuter;
};

// Struct for storing additional information used by Ngram language model
// component.
struct LanguageModelNgramInfo {
  LanguageModelNgramInfo(const char *c, int l, bool p, float np, float nc)
    : context(c), context_unichar_step_len(l), pruned(p), ngram_prob(np),
      ngram_cost(nc) {}
  STRING context;  // context string
  // Length of the context measured by advancing using UNICHAR::utf8_step()
  // (should be at most the order of the character ngram model used).
  int context_unichar_step_len;
  // The paths with pruned set are pruned out from the perspective of the
  // character ngram model. They are explored further because they represent
  // a dictionary match or a top choice. Thus ngram_info is still computed
  // for them in order to calculate the combined cost.
  bool pruned;
  // -ln(P_ngram_model(path))
  float ngram_prob;
  // -[ ln(P_classifier(path)) + scale_factor * ln(P_ngram_model(path)) ]
  float ngram_cost;
};

// Struct for storing the information about a path in the segmentation graph
// explored by Viterbi search.
struct ViterbiStateEntry : public ELIST_LINK {
  ViterbiStateEntry(BLOB_CHOICE *pb, ViterbiStateEntry *pe,
                    BLOB_CHOICE *b, float c, float ol,
                    const LanguageModelConsistencyInfo &ci,
                    const AssociateStats &as,
                    LanguageModelFlagsType tcf,
                    LanguageModelDawgInfo *d, LanguageModelNgramInfo *n)
    : cost(c), parent_b(pb), parent_vse(pe), ratings_sum(b->rating()),
      min_certainty(b->certainty()), adapted(b->adapted()), length(1),
      outline_length(ol), consistency_info(ci), associate_stats(as),
      top_choice_flags(tcf), dawg_info(d), ngram_info(n), updated(true) {
    if (pe != NULL) {
      ratings_sum += pe->ratings_sum;
      if (pe->min_certainty < min_certainty) {
        min_certainty = pe->min_certainty;
      }
      adapted += pe->adapted;
      length += pe->length;
      outline_length += pe->outline_length;
    }
  }
  ~ViterbiStateEntry() {
    delete dawg_info;
    delete ngram_info;
  }
  // Comparator function for sorting ViterbiStateEntry_LISTs in
  // non-increasing order of costs.
  static int Compare(const void *e1, const void *e2) {
    const ViterbiStateEntry *ve1 =
      *reinterpret_cast<const ViterbiStateEntry * const *>(e1);
    const ViterbiStateEntry *ve2 =
      *reinterpret_cast<const ViterbiStateEntry * const *>(e2);
    return (ve1->cost < ve2->cost) ? -1 : 1;
  }
  inline bool Consistent() const {
    if (dawg_info != NULL && consistency_info.NumInconsistentCase() == 0) {
      return true;
    }
    return consistency_info.Consistent();
  }

  // The cost is an adjusted ratings sum, that is adjusted by all the language
  // model components that use Viterbi search.
  float cost;

  // Pointers to parent BLOB_CHOICE and ViterbiStateEntry (not owned by this).
  BLOB_CHOICE *parent_b;
  ViterbiStateEntry *parent_vse;

  // Various information about the characters on the path represented
  // by this ViterbiStateEntry.
  float ratings_sum;  // sum of ratings of character on the path
  float min_certainty;  // minimum certainty on the path
  int adapted;  // number of BLOB_CHOICES from adapted templates
  int length;  // number of characters on the path
  float outline_length;  // length of the outline so far
  LanguageModelConsistencyInfo consistency_info;  // path consistency info
  AssociateStats associate_stats;  // character widths/gaps/seams

  // Flags for marking the entry as a top choice path with
  // the smallest rating or lower/upper case letters).
  LanguageModelFlagsType top_choice_flags;

  // Extra information maintained by Dawg laguage model component
  // (owned by ViterbiStateEntry).
  LanguageModelDawgInfo *dawg_info;

  // Extra information maintained by Ngram laguage model component
  // (owned by ViterbiStateEntry).
  LanguageModelNgramInfo *ngram_info;

  bool updated;  // set to true if the entry has just been created/updated
};

ELISTIZEH(ViterbiStateEntry);

// Struct to store information maintained by various language model components.
struct LanguageModelState {
  LanguageModelState(int col, int row) : contained_in_col(col),
      contained_in_row(row), viterbi_state_entries_prunable_length(0),
      viterbi_state_entries_length(0),
      viterbi_state_entries_prunable_max_cost(MAX_FLOAT32) {}
  ~LanguageModelState() {}

  // Ratings matrix cell that holds this LanguageModelState
  // (needed to construct best STATE for rebuild_current_state()
  // and best BLOB_CHOICE_LIST_VECTOR for AcceptableChoice()).
  int contained_in_col;
  int contained_in_row;

  // Storage for the Viterbi state.
  ViterbiStateEntry_LIST viterbi_state_entries;
  // Number and max cost of prunable paths in viterbi_state_entries.
  int viterbi_state_entries_prunable_length;
  // Total number of entries in viterbi_state_entries.
  int viterbi_state_entries_length;
  float viterbi_state_entries_prunable_max_cost;

  // TODO(daria): add font consistency checking.
};

// Bundle together all the things pertaining to the best choice/state.
struct BestChoiceBundle {
  BestChoiceBundle(STATE *s, WERD_CHOICE *bc, WERD_CHOICE *rc,
                   BLOB_CHOICE_LIST_VECTOR *bcc)
    : best_state(s), best_choice(bc), raw_choice(rc),
      best_char_choices(bcc), updated(false), best_vse(NULL), best_b(NULL) {}

  STATE *best_state;
  WERD_CHOICE *best_choice;
  WERD_CHOICE *raw_choice;
  BLOB_CHOICE_LIST_VECTOR *best_char_choices;
  bool updated;
  DANGERR fixpt;
  ViterbiStateEntry *best_vse; // best ViterbiStateEntry and BLOB_CHOICE
  BLOB_CHOICE *best_b;         // at the end of the best choice path
};

struct BestPathByColumn {
  float avg_cost;
  ViterbiStateEntry *best_vse;
  BLOB_CHOICE *best_b;
};

// This class that contains the data structures and functions necessary
// to represent and use the knowledge about the language.
class LanguageModel {
 public:
  // Adjustments to pain point priority.
  static const float kInitialPainPointPriorityAdjustment;
  static const float kDefaultPainPointPriorityAdjustment;
  static const float kBestChoicePainPointPriorityAdjustment;
  static const float kCriticalPainPointPriorityAdjustment;

  // Denominator for normalizing per-letter ngram cost when deriving
  // penalty adjustments.
  static const float kMaxAvgNgramCost;
  // Minimum word length for fixed length dawgs.
  // TODO(daria): check in the new chi/jpn.traineddata without the
  // fixed length dawg of length 1 and delete this variable.
  static const int kMinFixedLengthDawgLength;
  // If there is a significant drop in character ngram probability or a
  // dangerous ambiguity make the thresholds on what blob combinations
  // can be classified looser.
  static const float kLooseMaxCharWhRatio;

  // Masks for interpreting which language model components
  // were changed by the call to UpdateState().
  static const LanguageModelFlagsType kSmallestRatingFlag = 0x1;
  static const LanguageModelFlagsType kLowerCaseFlag = 0x2;
  static const LanguageModelFlagsType kUpperCaseFlag = 0x4;
  static const LanguageModelFlagsType kConsistentFlag = 0x8;
  static const LanguageModelFlagsType kDawgFlag = 0x10;
  static const LanguageModelFlagsType kNgramFlag = 0x20;
  static const LanguageModelFlagsType kJustClassifiedFlag = 0x80;
  static const LanguageModelFlagsType kAllChangedFlag = 0xff;

  LanguageModel(const UnicityTable<FontInfo> *fontinfo_table, Dict *dict);
  ~LanguageModel();

  // Updates data structures that are used for the duration of the segmentation
  // search on the current word;
  void InitForWord(const WERD_CHOICE *prev_word,
                   bool fixed_pitch, float best_choice_cert,
                   float max_char_wh_ratio, float rating_cert_scale,
                   HEAP *pain_points, CHUNKS_RECORD *chunks_record,
                   BlamerBundle *blamer_bundle, bool debug_blamer);
  // Resets all the "updated" flags used by the Viterbi search that were
  // "registered" during the update of the ratings matrix.
  void CleanUp();
  // Deletes and sets to NULL language model states of each of the
  // BLOB_CHOICEs in the given BLOB_CHOICE_LIST.
  void DeleteState(BLOB_CHOICE_LIST *choices);

  // Updates language model state of the given BLOB_CHOICE_LIST (from
  // the ratings matrix) a its parent. Updates pain_points if new
  // problematic points are found in the segmentation graph.
  //
  // At most language_model_viterbi_list_size are kept in each
  // LanguageModelState.viterbi_state_entries list.
  // At most language_model_viterbi_list_max_num_prunable of those are prunable
  // (non-dictionary) paths.
  // The entries that represent dictionary word paths are kept at the front
  // of the list.
  // The list ordered by cost that is computed collectively by several
  // language model components (currently dawg and ngram components).
  //
  // best_path_by_column records the lowest cost path found so far for each
  // column of the chunks_record->ratings matrix over all the rows. This
  // array is updated if a lower cost ViterbiStateEntry is created in curr_col.
  LanguageModelFlagsType UpdateState(
      LanguageModelFlagsType changed,
      int curr_col, int curr_row,
      BLOB_CHOICE_LIST *curr_list,
      BLOB_CHOICE_LIST *parent_list,
      HEAP *pain_points,
      BestPathByColumn *best_path_by_column[],
      CHUNKS_RECORD *chunks_record,
      BestChoiceBundle *best_choice_bundle,
      BlamerBundle *blamer_bundle);

  // Generates pain points from the problematic top choice paths when the
  // segmentation search is guided by the character ngram model.
  // It is necessary to consider problematic the top choice paths instead of
  // the problematic lowest cost paths because the character ngram model
  // might assign a very high cost to very improbably paths. For example,
  // "liot" might have a much lower cost than "llot", and the character ngram
  // model might detect a dip in probability for p(t|lio) at the end of the
  // word, but not at the beginning (p(i|l) would be ok). However, looking at
  // the dips in character ngram probability of the top choices we would be
  // able to stop the problematic points (p(l| l) would be low).
  void GenerateNgramModelPainPointsFromColumn(int col, int row,
                                              HEAP *pain_points,
                                              CHUNKS_RECORD *chunks_record);

  // Generates pain points from the problematic lowest cost paths that are
  // "promising" (i.e. would have the cost lower than the one recorded in
  // best_path_by_column if the problematic ending of the path is removed
  // and after being combined with another blob the certainty of the last
  // blob is improved).
  void GenerateProblematicPathPainPointsFromColumn(
      int col, int row, float best_choice_cert,
      HEAP *pain_points, BestPathByColumn *best_path_by_column[],
      CHUNKS_RECORD *chunks_record);

  // This function can be called after processing column col of the
  // chunks_record->ratings matrix in order to find the promising paths
  // that were terminated or made inconsistent by the character choices
  // in column col. If such paths are identified, this function generates
  // pain points to combine the problematic cells of the matrix.
  void GeneratePainPointsFromColumn(
    int col,
    const GenericVector<int> &non_empty_rows,
    float best_choice_cert,
    HEAP *pain_points,
    BestPathByColumn *best_path_by_column[],
    CHUNKS_RECORD *chunks_record);

  // Generates a pain point for each problematic point on the best choice
  // path. Such problematic points could be a termination of a dicionary
  // word, dip in ngram probability, invalid punctuation, inconsistent
  // case/chartype/script or punctuation in the middle of a word.
  void GeneratePainPointsFromBestChoice(
      HEAP *pain_points,
      CHUNKS_RECORD *chunks_record,
      BestChoiceBundle *best_choice_bundle);

  // Adds a pain point to the given pain_points queue that will cause
  // the entry at chunks_record->ratings(col, row) to be classified.
  // The priority of the pain point is set to be:
  //
  // priority_adjustment * sqrt(avg_parent_cost)
  // ----------------------------------------------------
  // sqrt(dict_parent_path_length) * |worst_piece_cert|
  //
  // The priority is further lowered if fragmented is true.
  // Reurns true if a new pain point was added to pain_points.
  bool GeneratePainPoint(int col, int row, bool ok_to_extend,
                         float priority_adjustment,
                         float worst_piece_cert,
                         bool fragmented,
                         float best_choice_cert,
                         float max_char_wh_ratio,
                         BLOB_CHOICE *parent_b,
                         ViterbiStateEntry *parent_vse,
                         CHUNKS_RECORD *chunks_record,
                         HEAP *pain_points);

  // Returns true if an acceptable best choice was discovered.
  inline bool AcceptableChoiceFound() { return acceptable_choice_found_; }

  // Fills cert with the worst certainty of the top non-fragmented choice
  // of the left and right neighbor of the given col,row.
  // Sets fragmented if any of the neighbors have a fragmented character
  // as the top choice.
  inline void GetWorstPieceCertainty(int col, int row, MATRIX *ratings,
                                     float *cert, bool *fragmented) {
    *cert = 0.0f;
    *fragmented = false;
    if (row > 0) {
      GetPieceCertainty(ratings->get(col, row-1), cert, fragmented);
    }
    if (col+1 < ratings->dimension()) {
      GetPieceCertainty(ratings->get(col+1, row), cert, fragmented);
    }
    ASSERT_HOST(*cert < 0.0f);
  }

  // Returns outline length of the given blob is computed as:
  // rating_cert_scale * rating / certainty
  // Since from Wordrec::SegSearch() in segsearch.cpp
  // rating_cert_scale = -1.0 * getDict().certainty_scale / rating_scale
  // And from Classify::ConvertMatchesToChoices() in adaptmatch.cpp
  // Rating = Certainty = next.rating
  // Rating *= rating_scale * Results->BlobLength
  // Certainty *= -(getDict().certainty_scale)
  inline float ComputeOutlineLength(BLOB_CHOICE *b) {
    return rating_cert_scale_ * b->rating() / b->certainty();
  }

 protected:

  inline float CertaintyScore(float cert) {
    if (language_model_use_sigmoidal_certainty) {
      // cert is assumed to be between 0 and -dict_->certainty_scale.
      // If you enable language_model_use_sigmoidal_certainty, you
      // need to adjust language_model_ngram_nonmatch_score as well.
      cert = -cert / dict_->certainty_scale;
      return 1.0f / (1.0f + exp(10.0f * cert));
    } else {
      return (-1.0f / cert);
    }
  }

  inline bool NonAlphaOrDigitMiddle(int col, int row, int dimension,
                                    UNICHAR_ID unichar_id) {
    return (!dict_->getUnicharset().get_isalpha(unichar_id) &&
            !dict_->getUnicharset().get_isdigit(unichar_id) &&
            col > 0 && row+1 < dimension);
  }

  inline bool IsFragment(BLOB_CHOICE *b) {
    return dict_->getUnicharset().get_fragment(b->unichar_id());
  }

  inline bool IsHan(int script_id) {
    return ((dict_->getUnicharset().han_sid() !=
             dict_->getUnicharset().null_sid()) &&
            (script_id == dict_->getUnicharset().han_sid()));
  }

  // Finds the first non-fragmented character in the given BLOB_CHOICE_LIST
  // and updates cert if its certainty is less than the one recorded in cert.
  // Sets fragmented if the first choice in BLOB_CHOICE_LIST is a fragment.
  inline void GetPieceCertainty(BLOB_CHOICE_LIST *blist,
                                float *cert, bool *fragmented) {
    if (blist == NOT_CLASSIFIED || blist->empty()) return;
    BLOB_CHOICE_IT bit(blist);
    while (!bit.at_last() && IsFragment(bit.data())) {
      *fragmented = true;
      bit.forward();  // skip fragments
    }
    // Each classification must have at least one non-fragmented choice.
    ASSERT_HOST(!IsFragment(bit.data()));
    if (bit.data()->certainty() < *cert) *cert = bit.data()->certainty();
  }

  inline float ComputeAdjustment(int num_problems, float penalty) {
    if (num_problems == 0) return 0.0f;
    if (num_problems == 1) return penalty;
    return (penalty + (language_model_penalty_increment *
                       static_cast<float>(num_problems-1)));
  }

  // Computes the adjustment to the ratings sum based on the given
  // consistency_info. The paths with invalid punctuation, inconsistent
  // case and character type are penalized proportionally to the number
  // of inconsistencies on the path.
  inline float ComputeConsistencyAdjustment(
      const LanguageModelDawgInfo *dawg_info,
      const LanguageModelConsistencyInfo &consistency_info) {
    if (dawg_info != NULL) {
      return ComputeAdjustment(consistency_info.NumInconsistentCase(),
                               language_model_penalty_case);
    }
    return (ComputeAdjustment(consistency_info.NumInconsistentPunc(),
                              language_model_penalty_punc) +
            ComputeAdjustment(consistency_info.NumInconsistentCase(),
                              language_model_penalty_case) +
            ComputeAdjustment(consistency_info.NumInconsistentChartype(),
                              language_model_penalty_chartype) +
            ComputeAdjustment(consistency_info.NumInconsistentSpaces(),
                              language_model_penalty_spacing) +
            (consistency_info.inconsistent_script ?
             language_model_penalty_script : 0.0f) +
            (consistency_info.inconsistent_font ?
             language_model_penalty_font : 0.0f));
  }

  // Returns an adjusted ratings sum that includes inconsistency penalties.
  inline float ComputeConsistencyAdjustedRatingsSum(
      float ratings_sum,
      const LanguageModelDawgInfo *dawg_info,
      const LanguageModelConsistencyInfo &consistency_info) {
    return (ratings_sum * (1.0f + ComputeConsistencyAdjustment(
        dawg_info, consistency_info)));
  }

  // Returns an adjusted ratings sum that includes inconsistency penalties,
  // penalties for non-dictionary paths and paths with dips in ngram
  // probability.
  float ComputeAdjustedPathCost(
      float ratings_sum, int length, float dawg_score,
      const LanguageModelDawgInfo *dawg_info,
      const LanguageModelNgramInfo *ngram_info,
      const LanguageModelConsistencyInfo &consistency_info,
      const AssociateStats &associate_stats,
      ViterbiStateEntry *parent_vse);

  // Returns true if the given ViterbiStateEntry represents a problematic
  // path. A path is considered problematic if the last unichar makes it
  // inconsistent, introduces a dip in ngram probability or transforms a
  // dictionary path into a non-dictionary one.
  bool ProblematicPath(const ViterbiStateEntry &vse,
                       UNICHAR_ID unichar_id, bool word_end);

  // Finds the first lower and upper case character in curr_list.
  // If none found, chooses the first character in the list.
  void GetTopChoiceLowerUpper(LanguageModelFlagsType changed,
                              BLOB_CHOICE_LIST *curr_list,
                              BLOB_CHOICE **first_lower,
                              BLOB_CHOICE **first_upper);

  // Helper function that computes the cost of the path composed of the
  // path in the given parent ViterbiStateEntry and the given BLOB_CHOICE.
  // Adds a new ViterbiStateEntry to the list of viterbi entries
  // in the given BLOB_CHOICE if the new path looks good enough.
  // Returns LanguageModelFlagsType that indicates which language
  // model components were involved in creating the new entry.
  LanguageModelFlagsType AddViterbiStateEntry(
      LanguageModelFlagsType top_choice_flags,
      float denom,
      bool word_end,
      int curr_col, int curr_row,
      BLOB_CHOICE *b,
      BLOB_CHOICE *parent_b,
      ViterbiStateEntry *parent_vse,
      HEAP *pain_points,
      BestPathByColumn *best_path_by_column[],
      CHUNKS_RECORD *chunks_record,
      BestChoiceBundle *best_choice_bundle,
      BlamerBundle *blamer_bundle);

  // Pretty print information in the given ViterbiStateEntry.
  void PrintViterbiStateEntry(const char *msg,
                              ViterbiStateEntry *vse,
                              BLOB_CHOICE *b,
                              CHUNKS_RECORD *chunks_record);

  // Determines whether a potential entry is a true top choice and
  // updates changed accordingly.
  //
  // Note: The function assumes that b, top_choice_flags and changed
  // are not NULL.
  void GenerateTopChoiceInfo(
      float ratings_sum,
      const LanguageModelDawgInfo *dawg_info,
      const LanguageModelConsistencyInfo &consistency_info,
      const ViterbiStateEntry *parent_vse,
      BLOB_CHOICE *b,
      LanguageModelFlagsType *top_choice_flags,
      LanguageModelFlagsType *changed);

  // Calls dict_->LetterIsOk() with DawgArgs initialized from parent_vse and
  // unichar from b.unichar_id(). Constructs and returns LanguageModelDawgInfo
  // with updated active dawgs, constraints and permuter.
  //
  // Note: the caller is responsible for deleting the returned pointer.
  LanguageModelDawgInfo *GenerateDawgInfo(bool word_end, int script_id,
                                          int curr_col, int curr_row,
                                          const BLOB_CHOICE &b,
                                          const ViterbiStateEntry *parent_vse,
                                          LanguageModelFlagsType *changed);

  // Computes p(unichar | parent context) and records it in ngram_cost.
  // If b.unichar_id() is an unlikely continuation of the parent context
  // sets found_small_prob to true and returns NULL.
  // Otherwise creates a new LanguageModelNgramInfo entry containing the
  // updated context (that includes b.unichar_id() at the end) and returns it.
  //
  // Note: the caller is responsible for deleting the returned pointer.
  LanguageModelNgramInfo *GenerateNgramInfo(const char *unichar,
                                            float certainty, float denom,
                                            int curr_col, int curr_row,
                                            const ViterbiStateEntry *parent_vse,
                                            BLOB_CHOICE *parent_b,
                                            LanguageModelFlagsType *changed);

  // Computes -(log(prob(classifier)) + log(prob(ngram model)))
  // for the given unichar in the given context. If there are multiple
  // unichars at one position - takes the average of their probabilities.
  // UNICHAR::utf8_step() is used to separate out individual UTF8 characters,
  // since probability_in_context() can only handle one at a time (while
  // unicharset might contain ngrams and glyphs composed from multiple UTF8
  // characters).
  float ComputeNgramCost(const char *unichar, float certainty, float denom,
                         const char *context, int *unichar_step_len,
                         bool *found_small_prob, float *ngram_prob);

  // Computes the normalization factors for the classifier confidences
  // (used by ComputeNgramCost()).
  float ComputeDenom(BLOB_CHOICE_LIST *curr_list);

  // Fills the given consistenty_info based on parent_vse.consistency_info
  // and on the consistency of the given unichar_id with parent_vse.
  void FillConsistencyInfo(
      int curr_col, bool word_end, BLOB_CHOICE *b,
      ViterbiStateEntry *parent_vse, BLOB_CHOICE *parent_b,
      CHUNKS_RECORD *chunks_record,
      LanguageModelConsistencyInfo *consistency_info);

  // Constructs WERD_CHOICE by recording unichar_ids of the BLOB_CHOICEs
  // on the path represented by the given BLOB_CHOICE and language model
  // state entries (lmse, dse). The path is re-constructed by following
  // the parent pointers in the the lang model state entries). If the
  // constructed WERD_CHOICE is better than the best/raw choice recorded
  // in the best_choice_bundle, this function updates the corresponding
  // fields and sets best_choice_bunldle->updated to true.
  void UpdateBestChoice(BLOB_CHOICE *b,
                        ViterbiStateEntry *vse,
                        HEAP *pain_points,
                        CHUNKS_RECORD *chunks_record,
                        BestChoiceBundle *best_choice_bundle,
                        BlamerBundle *blamer_bundle);

  // Fills the given floats array with raw features extracted from the
  // path represented by the given ViterbiStateEntry.
  // See ccstruct/params_training_featdef.h for feature information.
  void ExtractRawFeaturesFromPath(const ViterbiStateEntry &vse,
                                  float *features);

  // Constructs a WERD_CHOICE by tracing parent pointers starting with
  // the given LanguageModelStateEntry. Returns the constructed word.
  // Updates best_char_choices, certainties and state if they are not
  // NULL (best_char_choices and certainties are assumed to have the
  // length equal to lmse->length).
  // The caller is resposible for freeing memory associated with the
  // returned WERD_CHOICE.
  WERD_CHOICE *ConstructWord(BLOB_CHOICE *b,
                             ViterbiStateEntry *vse,
                             CHUNKS_RECORD *chunks_record,
                             BLOB_CHOICE_LIST_VECTOR *best_char_choices,
                             float certainties[],
                             float *dawg_score,
                             STATE *state,
                             BlamerBundle *blamer_bundle,
                             bool *truth_path);

  // This function is used for non-space delimited languages when looking
  // for word endings recorded while trying to separate the path into words.
  //
  // The function increments covered if a valid word ending is found in
  // active_dawgs (if covered is incremented, skip is set to the number
  // of unichars that should be skipped because they are covered by the
  // word whose ending was just discovered).
  //
  // dawg_score and dawg_score_done are updated if:
  // -- at the end of the path we discover a valid word ending from a
  //    non-fixed length dawg (this means that the whole word is a
  //    valid word, so dawg_score is set to 1.0f
  // -- word_start is true (dawg_score is set to covered / word length)
  //
  // Note: this function assumes that skip, covered, dawg_score and
  // dawg_score_done are not NULL.
  void UpdateCoveredByFixedLengthDawgs(const DawgInfoVector &active_dawgs,
                                       int word_index, int word_length,
                                       int *skip, int *covered,
                                       float *dawg_score,
                                       bool *dawg_score_done);

  // Wrapper around AssociateUtils::ComputeStats().
  inline void ComputeAssociateStats(int col, int row,
                                    float max_char_wh_ratio,
                                    ViterbiStateEntry *parent_vse,
                                    CHUNKS_RECORD *chunks_record,
                                    AssociateStats *associate_stats) {
  AssociateUtils::ComputeStats(
      col, row,
      (parent_vse != NULL) ? &(parent_vse->associate_stats) : NULL,
      (parent_vse != NULL) ? parent_vse->length : 0,
      fixed_pitch_, max_char_wh_ratio,
      chunks_record->word_res != NULL ? &chunks_record->word_res->denorm : NULL,
      chunks_record, language_model_debug_level, associate_stats);
  }

  // Returns true if the path with such top_choice_flags and dawg_info
  // could be pruned out (i.e. is neither a system/user/frequent dictionary
  // nor a top choice path).
  // In non-space delimited languages all paths can be "somewhat" dictionary
  // words. In such languages we can not do dictionary-driven path prunning,
  // so paths with non-empty dawg_info are considered prunable.
  inline bool PrunablePath(LanguageModelFlagsType top_choice_flags,
                           const LanguageModelDawgInfo *dawg_info) {
    if (top_choice_flags) return false;
    if (dawg_info != NULL &&
        (dawg_info->permuter == SYSTEM_DAWG_PERM ||
         dawg_info->permuter == USER_DAWG_PERM ||
         dawg_info->permuter == FREQ_DAWG_PERM) &&
         dict_->GetMaxFixedLengthDawgIndex() < 0) return false;
    return true;
  }

  // Returns true if the given ViterbiStateEntry represents an acceptable path.
  inline bool AcceptablePath(const ViterbiStateEntry &vse) {
    return (vse.dawg_info != NULL || vse.Consistent() ||
            (vse.ngram_info != NULL && !vse.ngram_info->pruned));
  }

 public:
  // Parameters.
  INT_VAR_H(language_model_debug_level, 0, "Language model debug level");
  BOOL_VAR_H(language_model_ngram_on, false,
             "Turn on/off the use of character ngram model");
  INT_VAR_H(language_model_ngram_order, 8,
            "Maximum order of the character ngram model");
  INT_VAR_H(language_model_viterbi_list_max_num_prunable, 10,
            "Maximum number of prunable (those for which PrunablePath() is true)"
            "entries in each viterbi list recorded in BLOB_CHOICEs");
  INT_VAR_H(language_model_viterbi_list_max_size, 500,
            "Maximum size of viterbi lists recorded in BLOB_CHOICEs");
  double_VAR_H(language_model_ngram_small_prob, 0.000001,
               "To avoid overly small denominators use this as the floor"
               " of the probability returned by the ngram model");
  double_VAR_H(language_model_ngram_nonmatch_score, -40.0,
               "Average classifier score of a non-matching unichar");
  BOOL_VAR_H(language_model_ngram_use_only_first_uft8_step, false,
             "Use only the first UTF8 step of the given string"
             " when computing log probabilities");
  double_VAR_H(language_model_ngram_scale_factor, 0.03,
               "Strength of the character ngram model relative to the"
               " character classifier ");
  BOOL_VAR_H(language_model_ngram_space_delimited_language, true,
             "Words are delimited by space");

  INT_VAR_H(language_model_min_compound_length, 3,
            "Minimum length of compound words");
  INT_VAR_H(language_model_fixed_length_choices_depth, 3,
            "Depth of blob choice lists to explore"
            " when fixed length dawgs are on");
  // Penalties used for adjusting path costs and final word rating.
  double_VAR_H(language_model_penalty_non_freq_dict_word, 0.1,
               "Penalty for words not in the frequent word dictionary");
  double_VAR_H(language_model_penalty_non_dict_word, 0.15,
               "Penalty for non-dictionary words");
  double_VAR_H(language_model_penalty_punc, 0.2,
               "Penalty for inconsistent punctuation");
  double_VAR_H(language_model_penalty_case, 0.1,
               "Penalty for inconsistent case");
  double_VAR_H(language_model_penalty_script, 0.5,
               "Penalty for inconsistent script");
  double_VAR_H(language_model_penalty_chartype, 0.3,
               "Penalty for inconsistent character type");
  double_VAR_H(language_model_penalty_font, 0.00,
               "Penalty for inconsistent font");
  double_VAR_H(language_model_penalty_spacing, 0.05,
               "Penalty for inconsistent spacing");
  double_VAR_H(language_model_penalty_increment, 0.01, "Penalty increment");
  BOOL_VAR_H(language_model_use_sigmoidal_certainty, false,
             "Use sigmoidal score for certainty");

 protected:
  // Member Variables.

  // Temporary DawgArgs struct that is re-used across different words to
  // avoid dynamic memory re-allocation (should be cleared before each use).
  DawgArgs *dawg_args_;
  // List of pointers to updated flags used by Viterbi search to mark
  // recently updated ViterbiStateEntries.
  GenericVector<bool *> updated_flags_;
  // Scaling for recovering blob outline length from rating and certainty.
  float rating_cert_scale_;

  // The following variables are set at construction time.

  // Pointer to fontinfo table (not owned by LanguageModel).
  const UnicityTable<FontInfo> *fontinfo_table_;

  // Pointer to Dict class, that is used for querying the dictionaries
  // (the pointer is not owned by LanguageModel).
  Dict *dict_;

  // TODO(daria): the following variables should become LanguageModel params
  // when the old code in bestfirst.cpp and heuristic.cpp is deprecated.
  //
  // Set to true if we are dealing with fixed pitch text
  // (set to assume_fixed_pitch_char_segment).
  bool fixed_pitch_;
  // Max char width-to-height ratio allowed
  // (set to segsearch_max_char_wh_ratio).
  float max_char_wh_ratio_;

  // The following variables are initialized with InitForWord().

  // String representation of the classification of the previous word
  // (since this is only used by the character ngram model component,
  // only the last language_model_ngram_order of the word are stored).
  STRING prev_word_str_;
  int prev_word_unichar_step_len_;
  // Active dawg and constraints vector.
  DawgInfoVector *beginning_active_dawgs_;
  DawgInfoVector *beginning_constraints_;
  DawgInfoVector *fixed_length_beginning_active_dawgs_;
  DawgInfoVector *empty_dawg_info_vec_;
  // Maximum adjustment factor for character ngram choices.
  float max_penalty_adjust_;
  // Set to true if acceptable choice was discovered.
  // Note: it would be nice to use this to terminate the search once an
  // acceptable choices is found. However we do not do that and once an
  // acceptable choice is found we finish looking for alternative choices
  // in the current segmentation graph and then exit the search (no more
  // classifications are done after an acceptable choice is found).
  // This is needed in order to let the search find the words very close to
  // the best choice in rating (e.g. what/What, Cat/cat, etc) and log these
  // choices. This way the stopper will know that the best choice is not
  // ambiguous (i.e. there are best choices in the best choice list that have
  // ratings close to the very best one) and will be less likely to mis-adapt.
  bool acceptable_choice_found_;
  // Set to true if a choice representing correct segmentation was explored.
  bool correct_segmentation_explored_;

};

}  // namespace tesseract

#endif  // TESSERACT_WORDREC_LANGUAGE_MODEL_H_
