///////////////////////////////////////////////////////////////////////
// File:        dict.cpp
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

#include <stdio.h>

#include "dict.h"
#include "unicodes.h"

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif
#include "tprintf.h"

namespace tesseract {

class Image;

Dict::Dict(CCUtil *ccutil)
    : letter_is_okay_(&tesseract::Dict::def_letter_is_okay),
      probability_in_context_(&tesseract::Dict::def_probability_in_context),
      params_model_classify_(NULL),
      ccutil_(ccutil),
      STRING_MEMBER(user_words_file, "", "A filename of user-provided words.",
                    getCCUtil()->params()),
      STRING_INIT_MEMBER(user_words_suffix, "",
                         "A suffix of user-provided words located in tessdata.",
                         getCCUtil()->params()),
      STRING_MEMBER(user_patterns_file, "",
                    "A filename of user-provided patterns.",
                    getCCUtil()->params()),
      STRING_INIT_MEMBER(user_patterns_suffix, "",
                         "A suffix of user-provided patterns located in "
                         "tessdata.",
                         getCCUtil()->params()),
      BOOL_INIT_MEMBER(load_system_dawg, true, "Load system word dawg.",
                       getCCUtil()->params()),
      BOOL_INIT_MEMBER(load_freq_dawg, true, "Load frequent word dawg.",
                       getCCUtil()->params()),
      BOOL_INIT_MEMBER(load_unambig_dawg, true, "Load unambiguous word dawg.",
                       getCCUtil()->params()),
      BOOL_INIT_MEMBER(load_punc_dawg, true,
                       "Load dawg with punctuation"
                       " patterns.",
                       getCCUtil()->params()),
      BOOL_INIT_MEMBER(load_number_dawg, true,
                       "Load dawg with number"
                       " patterns.",
                       getCCUtil()->params()),
      BOOL_INIT_MEMBER(load_bigram_dawg, true,
                       "Load dawg with special word "
                       "bigrams.",
                       getCCUtil()->params()),
      double_MEMBER(xheight_penalty_subscripts, 0.125,
                    "Score penalty (0.1 = 10%) added if there are subscripts "
                    "or superscripts in a word, but it is otherwise OK.",
                    getCCUtil()->params()),
      double_MEMBER(xheight_penalty_inconsistent, 0.25,
                    "Score penalty (0.1 = 10%) added if an xheight is "
                    "inconsistent.",
                    getCCUtil()->params()),
      double_MEMBER(segment_penalty_dict_frequent_word, 1.0,
                    "Score multiplier for word matches which have good case and"
                    "are frequent in the given language (lower is better).",
                    getCCUtil()->params()),
      double_MEMBER(segment_penalty_dict_case_ok, 1.1,
                    "Score multiplier for word matches that have good case "
                    "(lower is better).",
                    getCCUtil()->params()),
      double_MEMBER(segment_penalty_dict_case_bad, 1.3125,
                    "Default score multiplier for word matches, which may have "
                    "case issues (lower is better).",
                    getCCUtil()->params()),
      double_MEMBER(segment_penalty_ngram_best_choice, 1.24,
                    "Multipler to for the best choice from the ngram model.",
                    getCCUtil()->params()),
      double_MEMBER(segment_penalty_dict_nonword, 1.25,
                    "Score multiplier for glyph fragment segmentations which "
                    "do not match a dictionary word (lower is better).",
                    getCCUtil()->params()),
      double_MEMBER(segment_penalty_garbage, 1.50,
                    "Score multiplier for poorly cased strings that are not in"
                    " the dictionary and generally look like garbage (lower is"
                    " better).",
                    getCCUtil()->params()),
      STRING_MEMBER(output_ambig_words_file, "",
                    "Output file for ambiguities found in the dictionary",
                    getCCUtil()->params()),
      INT_MEMBER(dawg_debug_level, 0,
                 "Set to 1 for general debug info"
                 ", to 2 for more details, to 3 to see all the debug messages",
                 getCCUtil()->params()),
      INT_MEMBER(hyphen_debug_level, 0, "Debug level for hyphenated words.",
                 getCCUtil()->params()),
      INT_MEMBER(max_viterbi_list_size, 10, "Maximum size of viterbi list.",
                 getCCUtil()->params()),
      BOOL_MEMBER(use_only_first_uft8_step, false,
                  "Use only the first UTF8 step of the given string"
                  " when computing log probabilities.",
                  getCCUtil()->params()),
      double_MEMBER(certainty_scale, 20.0, "Certainty scaling factor",
                    getCCUtil()->params()),
      double_MEMBER(stopper_nondict_certainty_base, -2.50,
                    "Certainty threshold for non-dict words",
                    getCCUtil()->params()),
      double_MEMBER(stopper_phase2_certainty_rejection_offset, 1.0,
                    "Reject certainty offset", getCCUtil()->params()),
      INT_MEMBER(stopper_smallword_size, 2,
                 "Size of dict word to be treated as non-dict word",
                 getCCUtil()->params()),
      double_MEMBER(stopper_certainty_per_char, -0.50,
                    "Certainty to add"
                    " for each dict char above small word size.",
                    getCCUtil()->params()),
      double_MEMBER(stopper_allowable_character_badness, 3.0,
                    "Max certaintly variation allowed in a word (in sigma)",
                    getCCUtil()->params()),
      INT_MEMBER(stopper_debug_level, 0, "Stopper debug level",
                 getCCUtil()->params()),
      BOOL_MEMBER(stopper_no_acceptable_choices, false,
                  "Make AcceptableChoice() always return false. Useful"
                  " when there is a need to explore all segmentations",
                  getCCUtil()->params()),
      BOOL_MEMBER(save_raw_choices, false,
                  "Deprecated- backward compatibility only",
                  getCCUtil()->params()),
      INT_MEMBER(tessedit_truncate_wordchoice_log, 10,
                 "Max words to keep in list", getCCUtil()->params()),
      STRING_MEMBER(word_to_debug, "",
                    "Word for which stopper debug"
                    " information should be printed to stdout",
                    getCCUtil()->params()),
      STRING_MEMBER(word_to_debug_lengths, "",
                    "Lengths of unichars in word_to_debug",
                    getCCUtil()->params()),
      INT_MEMBER(fragments_debug, 0, "Debug character fragments",
                 getCCUtil()->params()),
      BOOL_MEMBER(segment_nonalphabetic_script, false,
                  "Don't use any alphabetic-specific tricks."
                  "Set to true in the traineddata config file for"
                  " scripts that are cursive or inherently fixed-pitch",
                  getCCUtil()->params()),
      BOOL_MEMBER(save_doc_words, 0, "Save Document Words",
                  getCCUtil()->params()),
      double_MEMBER(doc_dict_pending_threshold, 0.0,
                    "Worst certainty for using pending dictionary",
                    getCCUtil()->params()),
      double_MEMBER(doc_dict_certainty_threshold, -2.25,
                    "Worst certainty for words that can be inserted into the"
                    "document dictionary",
                    getCCUtil()->params()),
      INT_MEMBER(max_permuter_attempts, 10000,
                 "Maximum number of different"
                 " character choices to consider during permutation."
                 " This limit is especially useful when user patterns"
                 " are specified, since overly generic patterns can result in"
                 " dawg search exploring an overly large number of options.",
                 getCCUtil()->params()) {
  dang_ambigs_table_ = NULL;
  replace_ambigs_table_ = NULL;
  reject_offset_ = 0.0;
  go_deeper_fxn_ = NULL;
  hyphen_word_ = NULL;
  last_word_on_line_ = false;
  hyphen_unichar_id_ = INVALID_UNICHAR_ID;
  document_words_ = NULL;
  dawg_cache_ = NULL;
  dawg_cache_is_ours_ = false;
  pending_words_ = NULL;
  bigram_dawg_ = NULL;
  freq_dawg_ = NULL;
  punc_dawg_ = NULL;
  unambig_dawg_ = NULL;
  wordseg_rating_adjust_factor_ = -1.0f;
  output_ambig_words_file_ = NULL;
}

Dict::~Dict() {
  End();
  delete hyphen_word_;
  if (output_ambig_words_file_ != NULL) fclose(output_ambig_words_file_);
}

DawgCache *Dict::GlobalDawgCache() {
  // This global cache (a singleton) will outlive every Tesseract instance
  // (even those that someone else might declare as global statics).
  static DawgCache cache;
  return &cache;
}

// Sets up ready for a Load.
void Dict::SetupForLoad(DawgCache *dawg_cache) {
  if (dawgs_.length() != 0) this->End();

  apostrophe_unichar_id_ = getUnicharset().unichar_to_id(kApostropheSymbol);
  question_unichar_id_ = getUnicharset().unichar_to_id(kQuestionSymbol);
  slash_unichar_id_ = getUnicharset().unichar_to_id(kSlashSymbol);
  hyphen_unichar_id_ = getUnicharset().unichar_to_id(kHyphenSymbol);

  if (dawg_cache != NULL) {
    dawg_cache_ = dawg_cache;
    dawg_cache_is_ours_ = false;
  } else {
    dawg_cache_ = new DawgCache();
    dawg_cache_is_ours_ = true;
  }
}

// Loads the dawgs needed by Tesseract. Call FinishLoad() after.
void Dict::Load(const char *data_file_name, const STRING &lang) {
  // Load dawgs_.
  if (load_punc_dawg) {
    punc_dawg_ = dawg_cache_->GetSquishedDawg(
        lang, data_file_name, TESSDATA_PUNC_DAWG, dawg_debug_level);
    if (punc_dawg_) dawgs_ += punc_dawg_;
  }
  if (load_system_dawg) {
    Dawg *system_dawg = dawg_cache_->GetSquishedDawg(
        lang, data_file_name, TESSDATA_SYSTEM_DAWG, dawg_debug_level);
    if (system_dawg) dawgs_ += system_dawg;
  }
  if (load_number_dawg) {
    Dawg *number_dawg = dawg_cache_->GetSquishedDawg(
        lang, data_file_name, TESSDATA_NUMBER_DAWG, dawg_debug_level);
    if (number_dawg) dawgs_ += number_dawg;
  }
  if (load_bigram_dawg) {
    bigram_dawg_ = dawg_cache_->GetSquishedDawg(
        lang, data_file_name, TESSDATA_BIGRAM_DAWG, dawg_debug_level);
    if (bigram_dawg_) dawgs_ += bigram_dawg_;
  }
  if (load_freq_dawg) {
    freq_dawg_ = dawg_cache_->GetSquishedDawg(
        lang, data_file_name, TESSDATA_FREQ_DAWG, dawg_debug_level);
    if (freq_dawg_) dawgs_ += freq_dawg_;
  }
  if (load_unambig_dawg) {
    unambig_dawg_ = dawg_cache_->GetSquishedDawg(
        lang, data_file_name, TESSDATA_UNAMBIG_DAWG, dawg_debug_level);
    if (unambig_dawg_) dawgs_ += unambig_dawg_;
  }

  STRING name;
  if (((STRING &)user_words_suffix).length() > 0 ||
      ((STRING &)user_words_file).length() > 0) {
    Trie *trie_ptr = new Trie(DAWG_TYPE_WORD, lang, USER_DAWG_PERM,
                              getUnicharset().size(), dawg_debug_level);
    if (((STRING &)user_words_file).length() > 0) {
        name = user_words_file;
    } else {
        name = getCCUtil()->language_data_path_prefix;
        name += user_words_suffix;
    }
    if (!trie_ptr->read_and_add_word_list(name.string(), getUnicharset(),
                                          Trie::RRP_REVERSE_IF_HAS_RTL)) {
      tprintf("Error: failed to load %s\n", name.string());
      delete trie_ptr;
    } else {
      dawgs_ += trie_ptr;
    }
  }

  if (((STRING &)user_patterns_suffix).length() > 0 ||
      ((STRING &)user_patterns_file).length() > 0) {
    Trie *trie_ptr = new Trie(DAWG_TYPE_PATTERN, lang, USER_PATTERN_PERM,
                              getUnicharset().size(), dawg_debug_level);
    trie_ptr->initialize_patterns(&(getUnicharset()));
    if (((STRING &)user_patterns_file).length() > 0) {
        name = user_patterns_file;
    } else {
        name = getCCUtil()->language_data_path_prefix;
        name += user_patterns_suffix;
    }
    if (!trie_ptr->read_pattern_list(name.string(), getUnicharset())) {
      tprintf("Error: failed to load %s\n", name.string());
      delete trie_ptr;
    } else {
      dawgs_ += trie_ptr;
    }
  }

  document_words_ = new Trie(DAWG_TYPE_WORD, lang, DOC_DAWG_PERM,
                             getUnicharset().size(), dawg_debug_level);
  dawgs_ += document_words_;

  // This dawg is temporary and should not be searched by letter_is_ok.
  pending_words_ = new Trie(DAWG_TYPE_WORD, lang, NO_PERM,
                            getUnicharset().size(), dawg_debug_level);
}

// Completes the loading process after Load().
// Returns false if no dictionaries were loaded.
bool Dict::FinishLoad() {
  if (dawgs_.empty()) return false;
  // Construct a list of corresponding successors for each dawg. Each entry, i,
  // in the successors_ vector is a vector of integers that represent the
  // indices into the dawgs_ vector of the successors for dawg i.
  successors_.reserve(dawgs_.length());
  for (int i = 0; i < dawgs_.length(); ++i) {
    const Dawg *dawg = dawgs_[i];
    SuccessorList *lst = new SuccessorList();
    for (int j = 0; j < dawgs_.length(); ++j) {
      const Dawg *other = dawgs_[j];
      if (dawg != NULL && other != NULL &&
          (dawg->lang() == other->lang()) &&
          kDawgSuccessors[dawg->type()][other->type()]) *lst += j;
    }
    successors_ += lst;
  }
  return true;
}

void Dict::End() {
  if (dawgs_.length() == 0)
    return;  // Not safe to call twice.
  for (int i = 0; i < dawgs_.size(); i++) {
    if (!dawg_cache_->FreeDawg(dawgs_[i])) {
      delete dawgs_[i];
    }
  }
  if (dawg_cache_is_ours_) {
    delete dawg_cache_;
    dawg_cache_ = NULL;
  }
  successors_.delete_data_pointers();
  dawgs_.clear();
  successors_.clear();
  document_words_ = NULL;
  delete pending_words_;
  pending_words_ = NULL;
}

// Returns true if in light of the current state unichar_id is allowed
// according to at least one of the dawgs in the dawgs_ vector.
// See more extensive comments in dict.h where this function is declared.
int Dict::def_letter_is_okay(void* void_dawg_args,
                             UNICHAR_ID unichar_id,
                             bool word_end) const {
  DawgArgs *dawg_args = reinterpret_cast<DawgArgs*>(void_dawg_args);

  if (dawg_debug_level >= 3) {
    tprintf("def_letter_is_okay: current unichar=%s word_end=%d"
            " num active dawgs=%d\n",
            getUnicharset().debug_str(unichar_id).string(), word_end,
            dawg_args->active_dawgs->length());
  }

  // Do not accept words that contain kPatternUnicharID.
  // (otherwise pattern dawgs would not function correctly).
  // Do not accept words containing INVALID_UNICHAR_IDs.
  if (unichar_id == Dawg::kPatternUnicharID ||
      unichar_id == INVALID_UNICHAR_ID) {
    dawg_args->permuter = NO_PERM;
    return NO_PERM;
  }

  // Initialization.
  PermuterType curr_perm = NO_PERM;
  dawg_args->updated_dawgs->clear();
  dawg_args->valid_end = false;

  // Go over the active_dawgs vector and insert DawgPosition records
  // with the updated ref (an edge with the corresponding unichar id) into
  // dawg_args->updated_pos.
  for (int a = 0; a < dawg_args->active_dawgs->length(); ++a) {
    const DawgPosition &pos = (*dawg_args->active_dawgs)[a];
    const Dawg *punc_dawg = pos.punc_index >= 0 ? dawgs_[pos.punc_index] : NULL;
    const Dawg *dawg = pos.dawg_index >= 0 ? dawgs_[pos.dawg_index] : NULL;

    if (!dawg && !punc_dawg) {
      // shouldn't happen.
      tprintf("Received DawgPosition with no dawg or punc_dawg.  wth?\n");
      continue;
    }
    if (!dawg) {
      // We're in the punctuation dawg.  A core dawg has not been chosen.
      NODE_REF punc_node = GetStartingNode(punc_dawg, pos.punc_ref);
      EDGE_REF punc_transition_edge = punc_dawg->edge_char_of(
          punc_node, Dawg::kPatternUnicharID, word_end);
      if (punc_transition_edge != NO_EDGE) {
        // Find all successors, and see which can transition.
        const SuccessorList &slist = *(successors_[pos.punc_index]);
        for (int s = 0; s < slist.length(); ++s) {
          int sdawg_index = slist[s];
          const Dawg *sdawg = dawgs_[sdawg_index];
          UNICHAR_ID ch = char_for_dawg(unichar_id, sdawg);
          EDGE_REF dawg_edge = sdawg->edge_char_of(0, ch, word_end);
          if (dawg_edge != NO_EDGE) {
            if (dawg_debug_level >=3) {
              tprintf("Letter found in dawg %d\n", sdawg_index);
            }
            dawg_args->updated_dawgs->add_unique(
                DawgPosition(sdawg_index, dawg_edge,
                             pos.punc_index, punc_transition_edge, false),
                dawg_debug_level > 0,
                "Append transition from punc dawg to current dawgs: ");
            if (sdawg->permuter() > curr_perm) curr_perm = sdawg->permuter();
            if (sdawg->end_of_word(dawg_edge) &&
                punc_dawg->end_of_word(punc_transition_edge))
              dawg_args->valid_end = true;
          }
        }
      }
      EDGE_REF punc_edge = punc_dawg->edge_char_of(punc_node, unichar_id,
                                                   word_end);
      if (punc_edge != NO_EDGE) {
        if (dawg_debug_level >=3) {
          tprintf("Letter found in punctuation dawg\n");
        }
        dawg_args->updated_dawgs->add_unique(
            DawgPosition(-1, NO_EDGE, pos.punc_index, punc_edge, false),
            dawg_debug_level > 0,
            "Extend punctuation dawg: ");
        if (PUNC_PERM > curr_perm) curr_perm = PUNC_PERM;
        if (punc_dawg->end_of_word(punc_edge)) dawg_args->valid_end = true;
      }
      continue;
    }

    if (punc_dawg && dawg->end_of_word(pos.dawg_ref)) {
      // We can end the main word here.
      //  If we can continue on the punc ref, add that possibility.
      NODE_REF punc_node = GetStartingNode(punc_dawg, pos.punc_ref);
      EDGE_REF punc_edge = punc_node == NO_EDGE ? NO_EDGE
          : punc_dawg->edge_char_of(punc_node, unichar_id, word_end);
      if (punc_edge != NO_EDGE) {
        dawg_args->updated_dawgs->add_unique(
            DawgPosition(pos.dawg_index, pos.dawg_ref,
                         pos.punc_index, punc_edge, true),
            dawg_debug_level > 0,
            "Return to punctuation dawg: ");
        if (dawg->permuter() > curr_perm) curr_perm = dawg->permuter();
        if (punc_dawg->end_of_word(punc_edge)) dawg_args->valid_end = true;
      }
    }

    if (pos.back_to_punc) continue;

    // If we are dealing with the pattern dawg, look up all the
    // possible edges, not only for the exact unichar_id, but also
    // for all its character classes (alpha, digit, etc).
    if (dawg->type() == DAWG_TYPE_PATTERN) {
      ProcessPatternEdges(dawg, pos, unichar_id, word_end, dawg_args,
                          &curr_perm);
      // There can't be any successors to dawg that is of type
      // DAWG_TYPE_PATTERN, so we are done examining this DawgPosition.
      continue;
    }

    // Find the edge out of the node for the unichar_id.
    NODE_REF node = GetStartingNode(dawg, pos.dawg_ref);
    EDGE_REF edge = (node == NO_EDGE) ? NO_EDGE
        : dawg->edge_char_of(node, char_for_dawg(unichar_id, dawg), word_end);

    if (dawg_debug_level >= 3) {
      tprintf("Active dawg: [%d, " REFFORMAT "] edge=" REFFORMAT "\n",
              pos.dawg_index, node, edge);
    }

    if (edge != NO_EDGE) {  // the unichar was found in the current dawg
      if (dawg_debug_level >=3) {
        tprintf("Letter found in dawg %d\n", pos.dawg_index);
      }
      if (word_end && punc_dawg && !punc_dawg->end_of_word(pos.punc_ref)) {
        if (dawg_debug_level >= 3) {
          tprintf("Punctuation constraint not satisfied at end of word.\n");
        }
        continue;
      }
      if (dawg->permuter() > curr_perm) curr_perm = dawg->permuter();
      if (dawg->end_of_word(edge) &&
          (punc_dawg == NULL || punc_dawg->end_of_word(pos.punc_ref)))
        dawg_args->valid_end = true;
      dawg_args->updated_dawgs->add_unique(
          DawgPosition(pos.dawg_index, edge, pos.punc_index, pos.punc_ref,
                       false),
          dawg_debug_level > 0,
          "Append current dawg to updated active dawgs: ");
    }
  }  // end for
  // Update dawg_args->permuter if it used to be NO_PERM or became NO_PERM
  // or if we found the current letter in a non-punctuation dawg. This
  // allows preserving information on which dawg the "core" word came from.
  // Keep the old value of dawg_args->permuter if it is COMPOUND_PERM.
  if (dawg_args->permuter == NO_PERM || curr_perm == NO_PERM ||
      (curr_perm != PUNC_PERM && dawg_args->permuter != COMPOUND_PERM)) {
    dawg_args->permuter = curr_perm;
  }
  if (dawg_debug_level >= 2) {
    tprintf("Returning %d for permuter code for this character.\n",
            dawg_args->permuter);
  }
  return dawg_args->permuter;
}

void Dict::ProcessPatternEdges(const Dawg *dawg, const DawgPosition &pos,
                               UNICHAR_ID unichar_id, bool word_end,
                               DawgArgs *dawg_args,
                               PermuterType *curr_perm) const {
  NODE_REF node = GetStartingNode(dawg, pos.dawg_ref);
  // Try to find the edge corresponding to the exact unichar_id and to all the
  // edges corresponding to the character class of unichar_id.
  GenericVector<UNICHAR_ID> unichar_id_patterns;
  unichar_id_patterns.push_back(unichar_id);
  dawg->unichar_id_to_patterns(unichar_id, getUnicharset(),
                               &unichar_id_patterns);
  for (int i = 0; i < unichar_id_patterns.size(); ++i) {
    // On the first iteration check all the outgoing edges.
    // On the second iteration check all self-loops.
    for (int k = 0; k < 2; ++k) {
      EDGE_REF edge = (k == 0)
      ? dawg->edge_char_of(node, unichar_id_patterns[i], word_end)
      : dawg->pattern_loop_edge(pos.dawg_ref, unichar_id_patterns[i], word_end);
      if (edge == NO_EDGE) continue;
      if (dawg_debug_level >= 3) {
        tprintf("Pattern dawg: [%d, " REFFORMAT "] edge=" REFFORMAT "\n",
                pos.dawg_index, node, edge);
        tprintf("Letter found in pattern dawg %d\n", pos.dawg_index);
      }
      if (dawg->permuter() > *curr_perm) *curr_perm = dawg->permuter();
      if (dawg->end_of_word(edge)) dawg_args->valid_end = true;
      dawg_args->updated_dawgs->add_unique(
          DawgPosition(pos.dawg_index, edge, pos.punc_index, pos.punc_ref,
                       pos.back_to_punc),
          dawg_debug_level > 0,
          "Append current dawg to updated active dawgs: ");
    }
  }
}

// Fill the given active_dawgs vector with dawgs that could contain the
// beginning of the word. If hyphenated() returns true, copy the entries
// from hyphen_active_dawgs_ instead.
void Dict::init_active_dawgs(DawgPositionVector *active_dawgs,
                             bool ambigs_mode) const {
  int i;
  if (hyphenated()) {
    *active_dawgs = hyphen_active_dawgs_;
    if (dawg_debug_level >= 3) {
      for (i = 0; i < hyphen_active_dawgs_.size(); ++i) {
        tprintf("Adding hyphen beginning dawg [%d, " REFFORMAT "]\n",
                hyphen_active_dawgs_[i].dawg_index,
                hyphen_active_dawgs_[i].dawg_ref);
      }
    }
  } else {
    default_dawgs(active_dawgs, ambigs_mode);
  }
}

void Dict::default_dawgs(DawgPositionVector *dawg_pos_vec,
                         bool suppress_patterns) const {
  bool punc_dawg_available =
    (punc_dawg_ != NULL) &&
    punc_dawg_->edge_char_of(0, Dawg::kPatternUnicharID, true) != NO_EDGE;

  for (int i = 0; i < dawgs_.length(); i++) {
    if (dawgs_[i] != NULL &&
        !(suppress_patterns && (dawgs_[i])->type() == DAWG_TYPE_PATTERN)) {
      int dawg_ty = dawgs_[i]->type();
      bool subsumed_by_punc = kDawgSuccessors[DAWG_TYPE_PUNCTUATION][dawg_ty];
      if (dawg_ty == DAWG_TYPE_PUNCTUATION) {
        *dawg_pos_vec += DawgPosition(-1, NO_EDGE, i, NO_EDGE, false);
        if (dawg_debug_level >= 3) {
          tprintf("Adding beginning punc dawg [%d, " REFFORMAT "]\n", i,
                  NO_EDGE);
        }
      } else if (!punc_dawg_available || !subsumed_by_punc) {
        *dawg_pos_vec += DawgPosition(i, NO_EDGE, -1, NO_EDGE, false);
        if (dawg_debug_level >= 3) {
          tprintf("Adding beginning dawg [%d, " REFFORMAT "]\n", i, NO_EDGE);
        }
      }
    }
  }
}

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

  if (valid_word(best_choice) || stringlen < 2)
    return;

  // Discard words that contain >= kDocDictMaxRepChars repeating unichars.
  if (best_choice.length() >= kDocDictMaxRepChars) {
    int num_rep_chars = 1;
    UNICHAR_ID uch_id = best_choice.unichar_id(0);
    for (int i = 1; i < best_choice.length(); ++i) {
      if (best_choice.unichar_id(i) != uch_id) {
        num_rep_chars = 1;
        uch_id = best_choice.unichar_id(i);
      } else {
        ++num_rep_chars;
        if (num_rep_chars == kDocDictMaxRepChars) return;
      }
    }
  }

  if (best_choice.certainty() < doc_dict_certainty_threshold ||
      stringlen == 2) {
    if (best_choice.certainty() < doc_dict_pending_threshold)
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
    strcpy(filename, getCCUtil()->imagefile.string());
    strcat(filename, ".doc");
    doc_word_file = open_file (filename, "a");
    fprintf(doc_word_file, "%s\n",
            best_choice.debug_string().string());
    fclose(doc_word_file);
  }
  document_words_->add_word_to_dawg(best_choice);
}

void Dict::adjust_word(WERD_CHOICE *word,
                       bool nonword,
                       XHeightConsistencyEnum xheight_consistency,
                       float additional_adjust,
                       bool modify_rating,
                       bool debug) {
  bool is_han = (getUnicharset().han_sid() != getUnicharset().null_sid() &&
                 word->GetTopScriptID() == getUnicharset().han_sid());
  bool case_is_ok = (is_han || case_ok(*word, getUnicharset()));
  bool punc_is_ok = (is_han || !nonword || valid_punctuation(*word));

  float adjust_factor = additional_adjust;
  float new_rating = word->rating();
  new_rating += kRatingPad;
  const char *xheight_triggered = "";
  if (word->length() > 1) {
    // Calculate x-height and y-offset consistency penalties.
    switch (xheight_consistency) {
      case XH_INCONSISTENT:
        adjust_factor += xheight_penalty_inconsistent;
        xheight_triggered = ", xhtBAD";
        break;
      case XH_SUBNORMAL:
        adjust_factor += xheight_penalty_subscripts;
        xheight_triggered = ", xhtSUB";
        break;
      case XH_GOOD:
        // leave the factor alone - all good!
        break;
    }
    // TODO(eger): if nonword is true, but there is a "core" thats' a dict
    // word, negate nonword status.
  } else {
    if (debug) {
      tprintf("Consistency could not be calculated.\n");
    }
  }
  if (debug) {
    tprintf("%sWord: %s %4.2f%s", nonword ? "Non-" : "",
            word->unichar_string().string(), word->rating(),
            xheight_triggered);
  }

  if (nonword) {  // non-dictionary word
    if (case_is_ok && punc_is_ok) {
      adjust_factor += segment_penalty_dict_nonword;
      new_rating *= adjust_factor;
      if (debug) tprintf(", W");
    } else {
      adjust_factor += segment_penalty_garbage;
      new_rating *= adjust_factor;
      if (debug) {
        if (!case_is_ok) tprintf(", C");
        if (!punc_is_ok) tprintf(", P");
      }
    }
  } else {  // dictionary word
    if (case_is_ok) {
      if (!is_han && freq_dawg_ != NULL && freq_dawg_->word_in_dawg(*word)) {
        word->set_permuter(FREQ_DAWG_PERM);
        adjust_factor += segment_penalty_dict_frequent_word;
        new_rating *= adjust_factor;
        if (debug) tprintf(", F");
      } else {
        adjust_factor += segment_penalty_dict_case_ok;
        new_rating *= adjust_factor;
        if (debug) tprintf(", ");
      }
    } else {
      adjust_factor += segment_penalty_dict_case_bad;
      new_rating *= adjust_factor;
      if (debug) tprintf(", C");
    }
  }
  new_rating -= kRatingPad;
  if (modify_rating) word->set_rating(new_rating);
  if (debug) tprintf(" %4.2f --> %4.2f\n", adjust_factor, new_rating);
  word->set_adjust_factor(adjust_factor);
}

int Dict::valid_word(const WERD_CHOICE &word, bool numbers_ok) const {
  const WERD_CHOICE *word_ptr = &word;
  WERD_CHOICE temp_word(word.unicharset());
  if (hyphenated() && hyphen_word_->unicharset() == word.unicharset()) {
    copy_hyphen_info(&temp_word);
    temp_word += word;
    word_ptr = &temp_word;
  }
  if (word_ptr->length() == 0) return NO_PERM;
  // Allocate vectors for holding current and updated
  // active_dawgs and initialize them.
  DawgPositionVector *active_dawgs = new DawgPositionVector[2];
  init_active_dawgs(&(active_dawgs[0]), false);
  DawgArgs dawg_args(&(active_dawgs[0]), &(active_dawgs[1]), NO_PERM);
  int last_index = word_ptr->length() - 1;
  // Call leter_is_okay for each letter in the word.
  for (int i = hyphen_base_size(); i <= last_index; ++i) {
    if (!((this->*letter_is_okay_)(&dawg_args, word_ptr->unichar_id(i),
                                   i == last_index))) break;
    // Swap active_dawgs, constraints with the corresponding updated vector.
    if (dawg_args.updated_dawgs == &(active_dawgs[1])) {
      dawg_args.updated_dawgs = &(active_dawgs[0]);
      ++(dawg_args.active_dawgs);
    } else {
      ++(dawg_args.updated_dawgs);
      dawg_args.active_dawgs = &(active_dawgs[0]);
    }
  }
  delete[] active_dawgs;
  return valid_word_permuter(dawg_args.permuter, numbers_ok) ?
    dawg_args.permuter : NO_PERM;
}

bool Dict::valid_bigram(const WERD_CHOICE &word1,
                        const WERD_CHOICE &word2) const {
  if (bigram_dawg_ == NULL) return false;

  // Extract the core word from the middle of each word with any digits
  //         replaced with question marks.
  int w1start, w1end, w2start, w2end;
  word1.punct_stripped(&w1start, &w1end);
  word2.punct_stripped(&w2start, &w2end);

  // We don't want to penalize a single guillemet, hyphen, etc.
  // But our bigram list doesn't have any information about punctuation.
  if (w1start >= w1end) return word1.length() < 3;
  if (w2start >= w2end) return word2.length() < 3;

  const UNICHARSET& uchset = getUnicharset();
  GenericVector<UNICHAR_ID> bigram_string;
  bigram_string.reserve(w1end + w2end + 1);
  for (int i = w1start; i < w1end; i++) {
    const GenericVector<UNICHAR_ID>& normed_ids =
        getUnicharset().normed_ids(word1.unichar_id(i));
    if (normed_ids.size() == 1 && uchset.get_isdigit(normed_ids[0]))
      bigram_string.push_back(question_unichar_id_);
    else
      bigram_string += normed_ids;
  }
  bigram_string.push_back(UNICHAR_SPACE);
  for (int i = w2start; i < w2end; i++) {
    const GenericVector<UNICHAR_ID>& normed_ids =
        getUnicharset().normed_ids(word2.unichar_id(i));
    if (normed_ids.size() == 1 && uchset.get_isdigit(normed_ids[0]))
      bigram_string.push_back(question_unichar_id_);
    else
      bigram_string += normed_ids;
  }
  WERD_CHOICE normalized_word(&uchset, bigram_string.size());
  for (int i = 0; i < bigram_string.size(); ++i) {
    normalized_word.append_unichar_id_space_allocated(bigram_string[i], 1,
                                                      0.0f, 0.0f);
  }
  return bigram_dawg_->word_in_dawg(normalized_word);
}

bool Dict::valid_punctuation(const WERD_CHOICE &word) {
  if (word.length() == 0) return NO_PERM;
  int i;
  WERD_CHOICE new_word(word.unicharset());
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
    if (dawgs_[i] != NULL &&
        dawgs_[i]->type() == DAWG_TYPE_PUNCTUATION &&
        dawgs_[i]->word_in_dawg(new_word)) return true;
  }
  return false;
}

/// Returns true if the language is space-delimited (not CJ, or T).
bool Dict::IsSpaceDelimitedLang() const {
  const UNICHARSET &u_set = getUnicharset();
  if (u_set.han_sid() > 0) return false;
  if (u_set.katakana_sid() > 0) return false;
  return true;
}

}  // namespace tesseract
