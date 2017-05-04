///////////////////////////////////////////////////////////////////////
// File:        lm_state.h
// Description: Structures and functionality for capturing the state of
//              segmentation search guided by the language model.
//
// Author:      Rika Antonova
// Created:     Mon Jun 20 11:26:43 PST 2012
//
// (C) Copyright 2012, Google Inc.
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

#ifndef TESSERACT_WORDREC_LANGUAGE_MODEL_DEFS_H_
#define TESSERACT_WORDREC_LANGUAGE_MODEL_DEFS_H_

#include "associate.h"
#include "elst.h"
#include "dawg.h"
#include "lm_consistency.h"
#include "matrix.h"
#include "ratngs.h"
#include "stopper.h"
#include "strngs.h"

namespace tesseract {

/// Used for expressing various language model flags.
typedef unsigned char LanguageModelFlagsType;

/// The following structs are used for storing the state of the language model
/// in the segmentation search graph. In this graph the nodes are BLOB_CHOICEs
/// and the links are the relationships between the underlying blobs (see
/// segsearch.h for a more detailed description).
///
/// Each of the BLOB_CHOICEs contains LanguageModelState struct, which has
/// a list of N best paths (list of ViterbiStateEntry) explored by the Viterbi
/// search leading up to and including this BLOB_CHOICE.
///
/// Each ViterbiStateEntry contains information from various components of the
/// language model: dawgs in which the path is found, character ngram model
/// probability of the path, script/chartype/font consistency info, state for
/// language-specific heuristics (e.g. hyphenated and compound words,
/// lower/upper case preferences, etc).
///
/// Each ViterbiStateEntry also contains the parent pointer, so that the path
/// that it represents (WERD_CHOICE) can be constructed by following these
/// parent pointers.

/// Struct for storing additional information used by Dawg language model
/// component. It stores the set of active dawgs in which the sequence of
/// letters on a path can be found.
struct LanguageModelDawgInfo {
  LanguageModelDawgInfo(const DawgPositionVector *a, PermuterType pt)
    : active_dawgs(*a), permuter(pt) {}
  DawgPositionVector active_dawgs;
  PermuterType permuter;
};

/// Struct for storing additional information used by Ngram language model
/// component.
struct LanguageModelNgramInfo {
  LanguageModelNgramInfo(const char *c, int l, bool p, float nc, float ncc)
    : context(c), context_unichar_step_len(l), pruned(p), ngram_cost(nc),
      ngram_and_classifier_cost(ncc) {}
  STRING context;  //< context string
  /// Length of the context measured by advancing using UNICHAR::utf8_step()
  /// (should be at most the order of the character ngram model used).
  int context_unichar_step_len;
  /// The paths with pruned set are pruned out from the perspective of the
  /// character ngram model. They are explored further because they represent
  /// a dictionary match or a top choice. Thus ngram_info is still computed
  /// for them in order to calculate the combined cost.
  bool pruned;
  /// -ln(P_ngram_model(path))
  float ngram_cost;
  /// -[ ln(P_classifier(path)) + scale_factor * ln(P_ngram_model(path)) ]
  float ngram_and_classifier_cost;
};

/// Struct for storing the information about a path in the segmentation graph
/// explored by Viterbi search.
struct ViterbiStateEntry : public ELIST_LINK {
  ViterbiStateEntry(ViterbiStateEntry *pe,
                    BLOB_CHOICE *b, float c, float ol,
                    const LMConsistencyInfo &ci,
                    const AssociateStats &as,
                    LanguageModelFlagsType tcf,
                    LanguageModelDawgInfo *d,
                    LanguageModelNgramInfo *n,
                    const char *debug_uch)
    : cost(c), curr_b(b), parent_vse(pe), competing_vse(NULL),
      ratings_sum(b->rating()),
      min_certainty(b->certainty()), adapted(b->IsAdapted()), length(1),
      outline_length(ol), consistency_info(ci), associate_stats(as),
      top_choice_flags(tcf), dawg_info(d), ngram_info(n),
      updated(true) {
    debug_str = (debug_uch == NULL) ? NULL : new STRING();
    if (pe != NULL) {
      ratings_sum += pe->ratings_sum;
      if (pe->min_certainty < min_certainty) {
        min_certainty = pe->min_certainty;
      }
      adapted += pe->adapted;
      length += pe->length;
      outline_length += pe->outline_length;
      if (debug_uch != NULL) *debug_str += *(pe->debug_str);
    }
    if (debug_str != NULL && debug_uch != NULL) *debug_str += debug_uch;
  }
  ~ViterbiStateEntry() {
    delete dawg_info;
    delete ngram_info;
    delete debug_str;
  }
  /// Comparator function for sorting ViterbiStateEntry_LISTs in
  /// non-increasing order of costs.
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
  /// Returns true if this VSE has an alphanumeric character as its classifier
  /// result.
  bool HasAlnumChoice(const UNICHARSET& unicharset) {
    if (curr_b == NULL) return false;
    UNICHAR_ID unichar_id =  curr_b->unichar_id();
    if (unicharset.get_isalpha(unichar_id) ||
        unicharset.get_isdigit(unichar_id))
      return true;
    return false;
  }
  void Print(const char *msg) const;

  /// The cost is an adjusted ratings sum, that is adjusted by all the language
  /// model components that use Viterbi search.
  float cost;

  /// Pointers to BLOB_CHOICE and parent ViterbiStateEntry (not owned by this).
  BLOB_CHOICE *curr_b;
  ViterbiStateEntry *parent_vse;
  /// Pointer to a case-competing ViterbiStateEntry in the same list that
  /// represents a path ending in the same letter of the opposite case.
  ViterbiStateEntry *competing_vse;

  /// Various information about the characters on the path represented
  /// by this ViterbiStateEntry.
  float ratings_sum;     //< sum of ratings of character on the path
  float min_certainty;   //< minimum certainty on the path
  int adapted;           //< number of BLOB_CHOICES from adapted templates
  int length;            //< number of characters on the path
  float outline_length;  //< length of the outline so far
  LMConsistencyInfo consistency_info;  //< path consistency info
  AssociateStats associate_stats;      //< character widths/gaps/seams

  /// Flags for marking the entry as a top choice path with
  /// the smallest rating or lower/upper case letters).
  LanguageModelFlagsType top_choice_flags;

  /// Extra information maintained by Dawg language model component
  /// (owned by ViterbiStateEntry).
  LanguageModelDawgInfo *dawg_info;

  /// Extra information maintained by Ngram language model component
  /// (owned by ViterbiStateEntry).
  LanguageModelNgramInfo *ngram_info;

  bool updated;  //< set to true if the entry has just been created/updated
  /// UTF8 string representing the path corresponding to this vse.
  /// Populated only in when language_model_debug_level > 0.
  STRING *debug_str;
};

ELISTIZEH(ViterbiStateEntry);

/// Struct to store information maintained by various language model components.
struct LanguageModelState {
  LanguageModelState() :
     viterbi_state_entries_prunable_length(0),
    viterbi_state_entries_prunable_max_cost(MAX_FLOAT32),
    viterbi_state_entries_length(0) {}
  ~LanguageModelState() {}

  /// Clears the viterbi search state back to its initial conditions.
  void Clear();

  void Print(const char *msg);

  /// Storage for the Viterbi state.
  ViterbiStateEntry_LIST viterbi_state_entries;
  /// Number and max cost of prunable paths in viterbi_state_entries.
  int viterbi_state_entries_prunable_length;
  float viterbi_state_entries_prunable_max_cost;
  /// Total number of entries in viterbi_state_entries.
  int viterbi_state_entries_length;
};

/// Bundle together all the things pertaining to the best choice/state.
struct BestChoiceBundle {
  explicit BestChoiceBundle(int matrix_dimension)
    : updated(false), best_vse(NULL) {
    beam.reserve(matrix_dimension);
    for (int i = 0; i < matrix_dimension; ++i)
      beam.push_back(new LanguageModelState);
  }
  ~BestChoiceBundle() {}

  /// Flag to indicate whether anything was changed.
  bool updated;
  /// Places to try to fix the word suggested by ambiguity checking.
  DANGERR fixpt;
  /// The beam. One LanguageModelState containing a list of ViterbiStateEntry
  /// per row in the ratings matrix containing all VSEs whose BLOB_CHOICE is
  /// somewhere in the corresponding row.
  PointerVector<LanguageModelState> beam;
  /// Best ViterbiStateEntry and BLOB_CHOICE.
  ViterbiStateEntry *best_vse;
};

}  // namespace tesseract

#endif  // TESSERACT_WORDREC_LANGUAGE_MODEL_DEFS_H_
