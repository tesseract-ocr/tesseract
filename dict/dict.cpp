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

#include "dict.h"

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif
#include "tprintf.h"

namespace tesseract {

class Image;

Dict::Dict(Image* image_ptr)
    : letter_is_okay_(&tesseract::Dict::def_letter_is_okay),
      probability_in_context_(&tesseract::Dict::def_probability_in_context),
      image_ptr_(image_ptr),
      STRING_INIT_MEMBER(user_words_suffix, "",
                         "A list of user-provided words.",
                         getImage()->getCCUtil()->params()),
      STRING_INIT_MEMBER(user_patterns_suffix, "",
                         "A list of user-provided patterns.",
                         getImage()->getCCUtil()->params()),
      BOOL_INIT_MEMBER(load_system_dawg, true, "Load system word dawg.",
                       getImage()->getCCUtil()->params()),
      BOOL_INIT_MEMBER(load_freq_dawg, true, "Load frequent word dawg.",
                       getImage()->getCCUtil()->params()),
      BOOL_INIT_MEMBER(load_punc_dawg, true, "Load dawg with punctuation"
                       " patterns.", getImage()->getCCUtil()->params()),
      BOOL_INIT_MEMBER(load_number_dawg, true, "Load dawg with number"
                       " patterns.", getImage()->getCCUtil()->params()),
      BOOL_INIT_MEMBER(load_fixed_length_dawgs, true, "Load fixed length dawgs"
                       " (e.g. for non-space delimited languages)",
                       getImage()->getCCUtil()->params()),
      double_MEMBER(segment_penalty_dict_frequent_word, 1.0,
                    "Score multiplier for word matches which have good case and"
                    "are frequent in the given language (lower is better).",
                    getImage()->getCCUtil()->params()),
      double_MEMBER(segment_penalty_dict_case_ok, 1.1,
                    "Score multiplier for word matches that have good case "
                    "(lower is better).", getImage()->getCCUtil()->params()),
      double_MEMBER(segment_penalty_dict_case_bad, 1.3125,
                    "Default score multiplier for word matches, which may have "
                    "case issues (lower is better).",
                    getImage()->getCCUtil()->params()),
      double_MEMBER(segment_penalty_ngram_best_choice, 1.24,
                   "Multipler to for the best choice from the ngram model.",
                   getImage()->getCCUtil()->params()),
      double_MEMBER(segment_penalty_dict_nonword, 1.25,
                    "Score multiplier for glyph fragment segmentations which "
                    "do not match a dictionary word (lower is better).",
                    getImage()->getCCUtil()->params()),
      double_MEMBER(segment_penalty_garbage, 1.50,
                    "Score multiplier for poorly cased strings that are not in"
                    " the dictionary and generally look like garbage (lower is"
                    " better).", getImage()->getCCUtil()->params()),
      INT_MEMBER(dawg_debug_level, 0, "Set to 1 for general debug info"
                 ", to 2 for more details, to 3 to see all the debug messages",
                 getImage()->getCCUtil()->params()),
      INT_MEMBER(hyphen_debug_level, 0, "Debug level for hyphenated words.",
                 getImage()->getCCUtil()->params()),
      INT_MEMBER(max_viterbi_list_size, 10, "Maximum size of viterbi list.",
                 getImage()->getCCUtil()->params()),
      BOOL_MEMBER(use_only_first_uft8_step, false,
                  "Use only the first UTF8 step of the given string"
                  " when computing log probabilities.",
                  getImage()->getCCUtil()->params()),
      double_MEMBER(certainty_scale, 20.0, "Certainty scaling factor",
                    getImage()->getCCUtil()->params()),
      double_MEMBER(stopper_nondict_certainty_base, -2.50,
                    "Certainty threshold for non-dict words",
                    getImage()->getCCUtil()->params()),
      double_MEMBER(stopper_phase2_certainty_rejection_offset, 1.0,
                    "Reject certainty offset",
                    getImage()->getCCUtil()->params()),
      INT_MEMBER(stopper_smallword_size, 2,
                 "Size of dict word to be treated as non-dict word",
                 getImage()->getCCUtil()->params()),
      double_MEMBER(stopper_certainty_per_char, -0.50, "Certainty to add"
                    " for each dict char above small word size.",
                    getImage()->getCCUtil()->params()),
      double_MEMBER(stopper_allowable_character_badness, 3.0,
                    "Max certaintly variation allowed in a word (in sigma)",
                    getImage()->getCCUtil()->params()),
      INT_MEMBER(stopper_debug_level, 0, "Stopper debug level",
                 getImage()->getCCUtil()->params()),
      BOOL_MEMBER(stopper_no_acceptable_choices, false,
                  "Make AcceptableChoice() always return false. Useful"
                  " when there is a need to explore all segmentations",
                  getImage()->getCCUtil()->params()),
      BOOL_MEMBER(save_raw_choices, false, "Save all explored raw choices",
                  getImage()->getCCUtil()->params()),
      INT_MEMBER(tessedit_truncate_wordchoice_log, 10,
                 "Max words to keep in list",
                 getImage()->getCCUtil()->params()),
      STRING_MEMBER(word_to_debug, "", "Word for which stopper debug"
                    " information should be printed to stdout",
                    getImage()->getCCUtil()->params()),
      STRING_MEMBER(word_to_debug_lengths, "",
                    "Lengths of unichars in word_to_debug",
                    getImage()->getCCUtil()->params()),
      INT_MEMBER(fragments_debug, 0, "Debug character fragments",
                 getImage()->getCCUtil()->params()),
      INT_MEMBER(segment_debug, 0, "Debug the whole segmentation process",
                 getImage()->getCCUtil()->params()),
      BOOL_MEMBER(permute_debug, 0, "Debug char permutation process",
                  getImage()->getCCUtil()->params()),
      double_MEMBER(bestrate_pruning_factor, 2.0, "Multiplying factor of"
                    " current best rate to prune other hypotheses",
                    getImage()->getCCUtil()->params()),
      BOOL_MEMBER(permute_script_word, 0,
                  "Turn on word script consistency permuter",
                  getImage()->getCCUtil()->params()),
      BOOL_MEMBER(segment_segcost_rating, 0,
                  "incorporate segmentation cost in word rating?",
                  getImage()->getCCUtil()->params()),
      double_MEMBER(segment_reward_script, 0.95,
                    "Score multipler for script consistency within a word. "
                    "Being a 'reward' factor, it should be <= 1. "
                    "Smaller value implies bigger reward.",
                    getImage()->getCCUtil()->params()),
      BOOL_MEMBER(permute_fixed_length_dawg, 0,
                  "Turn on fixed-length phrasebook search permuter",
                  getImage()->getCCUtil()->params()),
      BOOL_MEMBER(permute_chartype_word, 0,
                  "Turn on character type (property) consistency permuter",
                  getImage()->getCCUtil()->params()),
      double_MEMBER(segment_reward_chartype, 0.97,
                    "Score multipler for char type consistency within a word. ",
                    getImage()->getCCUtil()->params()),
     double_MEMBER(segment_reward_ngram_best_choice, 0.99,
                   "Score multipler for ngram permuter's best choice"
                   " (only used in the Han script path).",
                   getImage()->getCCUtil()->params()),
      BOOL_MEMBER(save_doc_words, 0, "Save Document Words",
                  getImage()->getCCUtil()->params()),
      BOOL_MEMBER(doc_dict_enable, 1, "Enable Document Dictionary ",
                  getImage()->getCCUtil()->params()),
      double_MEMBER(doc_dict_pending_threshold, 0.0,
                    "Worst certainty for using pending dictionary",
                    getImage()->getCCUtil()->params()),
      double_MEMBER(doc_dict_certainty_threshold, -2.25,
                    "Worst certainty for words that can be inserted into the"
                    "document dictionary", getImage()->getCCUtil()->params()),
      BOOL_MEMBER(ngram_permuter_activated, false,
                  "Activate character-level n-gram-based permuter",
                  getImage()->getCCUtil()->params()),
      INT_MEMBER(max_permuter_attempts, 10000, "Maximum number of different"
                 " character choices to consider during permutation."
                 " This limit is especially useful when user patterns"
                 " are specified, since overly generic patterns can result in"
                 " dawg search exploring an overly large number of options.",
                 getImage()->getCCUtil()->params()),
      BOOL_MEMBER(permute_only_top, false, "Run only the top choice permuter",
                  getImage()->getCCUtil()->params()) {
  dang_ambigs_table_ = NULL;
  replace_ambigs_table_ = NULL;
  keep_word_choices_ = false;
  reject_offset_ = 0.0;
  best_raw_choice_ = NULL;
  best_choices_ = NIL_LIST;
  raw_choices_ = NIL_LIST;
  go_deeper_fxn_ = NULL;
  hyphen_word_ = NULL;
  last_word_on_line_ = false;
  hyphen_unichar_id_ = INVALID_UNICHAR_ID;
  document_words_ = NULL;
  pending_words_ = NULL;
  freq_dawg_ = NULL;
  punc_dawg_ = NULL;
  max_fixed_length_dawgs_wdlen_ = -1;
  wordseg_rating_adjust_factor_ = -1.0f;
}

Dict::~Dict() {
  if (hyphen_word_ != NULL) delete hyphen_word_;
}

void Dict::Load() {
  STRING name;
  STRING &lang = getImage()->getCCUtil()->lang;

  if (dawgs_.length() != 0) this->End();

  hyphen_unichar_id_ = getUnicharset().unichar_to_id(kHyphenSymbol);
  TessdataManager &tessdata_manager =
    getImage()->getCCUtil()->tessdata_manager;

  // Load dawgs_.
  if (load_punc_dawg && tessdata_manager.SeekToStart(TESSDATA_PUNC_DAWG)) {
    punc_dawg_ = new SquishedDawg(tessdata_manager.GetDataFilePtr(),
                                  DAWG_TYPE_PUNCTUATION, lang, PUNC_PERM,
                                  dawg_debug_level);
    dawgs_ += punc_dawg_;
  }
  if (load_system_dawg && tessdata_manager.SeekToStart(TESSDATA_SYSTEM_DAWG)) {
    dawgs_ += new SquishedDawg(tessdata_manager.GetDataFilePtr(),
                               DAWG_TYPE_WORD, lang, SYSTEM_DAWG_PERM,
                               dawg_debug_level);
  }
  if (load_number_dawg && tessdata_manager.SeekToStart(TESSDATA_NUMBER_DAWG)) {
    dawgs_ +=
      new SquishedDawg(tessdata_manager.GetDataFilePtr(),
                       DAWG_TYPE_NUMBER, lang, NUMBER_PERM, dawg_debug_level);
  }
  if (tessdata_manager.SeekToStart(TESSDATA_FREQ_DAWG)) {
    freq_dawg_ = new SquishedDawg(tessdata_manager.GetDataFilePtr(),
                                  DAWG_TYPE_WORD, lang, FREQ_DAWG_PERM,
                                  dawg_debug_level);
    dawgs_ += freq_dawg_;
  }

  if (((STRING &)user_words_suffix).length() > 0) {
    Trie *trie_ptr = new Trie(DAWG_TYPE_WORD, lang, USER_DAWG_PERM,
                              kMaxUserDawgEdges, getUnicharset().size(),
                              dawg_debug_level);
    name = getImage()->getCCUtil()->language_data_path_prefix;
    name += user_words_suffix;
    if (!trie_ptr->read_word_list(name.string(), getUnicharset())) {
      tprintf("Error: failed to load %s\n", name.string());
      exit(1);
    }
    dawgs_ += trie_ptr;
  }

  if (((STRING &)user_patterns_suffix).length() > 0) {
    Trie *trie_ptr = new Trie(DAWG_TYPE_PATTERN, lang, USER_PATTERN_PERM,
                              kMaxUserDawgEdges, getUnicharset().size(),
                              dawg_debug_level);
    trie_ptr->initialize_patterns(&(getUnicharset()));
    name = getImage()->getCCUtil()->language_data_path_prefix;
    name += user_patterns_suffix;
    if (!trie_ptr->read_pattern_list(name.string(), getUnicharset())) {
      tprintf("Error: failed to load %s\n", name.string());
      exit(1);
    }
    dawgs_ += trie_ptr;
  }

  document_words_ = new Trie(DAWG_TYPE_WORD, lang, DOC_DAWG_PERM,
                             kMaxDocDawgEdges, getUnicharset().size(),
                             dawg_debug_level);
  dawgs_ += document_words_;

  // This dawg is temporary and should not be searched by letter_is_ok.
  pending_words_ = new Trie(DAWG_TYPE_WORD, lang, NO_PERM,
                            kMaxDocDawgEdges, getUnicharset().size(),
                            dawg_debug_level);

  // Load fixed length dawgs if necessary (used for phrase search
  // for non-space delimited languages).
  if (load_fixed_length_dawgs &&
      tessdata_manager.SeekToStart(TESSDATA_FIXED_LENGTH_DAWGS)) {
    ReadFixedLengthDawgs(DAWG_TYPE_WORD, lang, SYSTEM_DAWG_PERM,
                         dawg_debug_level, tessdata_manager.GetDataFilePtr(),
                         &dawgs_, &max_fixed_length_dawgs_wdlen_);
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
      if (dawg != NULL && other != NULL &&
          (dawg->lang() == other->lang()) &&
          kDawgSuccessors[dawg->type()][other->type()]) *lst += j;
    }
    successors_ += lst;
  }
}

void Dict::End() {
  if (dawgs_.length() == 0)
    return;  // Not safe to call twice.
  dawgs_.delete_data_pointers();
  successors_.delete_data_pointers();
  dawgs_.clear();
  successors_.clear();
  document_words_ = NULL;
  max_fixed_length_dawgs_wdlen_ = -1;
  if (pending_words_ != NULL) {
    delete pending_words_;
    pending_words_ = NULL;
  }
}

// Returns true if in light of the current state unichar_id is allowed
// according to at least one of the dawgs in the dawgs_ vector.
// See more extensive comments in dict.h where this function is declared.
int Dict::def_letter_is_okay(void* void_dawg_args,
                             UNICHAR_ID unichar_id,
                             bool word_end) {
  DawgArgs *dawg_args = reinterpret_cast<DawgArgs*>(void_dawg_args);

  if (dawg_debug_level >= 3) {
    tprintf("def_letter_is_okay: current unichar=%s word_end=%d"
            " num active dawgs=%d num constraints=%d\n",
            getUnicharset().debug_str(unichar_id).string(), word_end,
            dawg_args->active_dawgs->length(),
            dawg_args->constraints->length());
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
  dawg_args->updated_active_dawgs->clear();
  const DawgInfoVector &constraints = *(dawg_args->constraints);
  *dawg_args->updated_constraints = constraints;

  // Go over the active_dawgs vector and insert DawgInfo records with the
  // updated ref (an edge with the corresponding unichar id) into
  // dawg_args->updated_active_dawgs.
  for (int a = 0; a < dawg_args->active_dawgs->length(); ++a) {
    const DawgInfo &info = (*dawg_args->active_dawgs)[a];
    const Dawg *dawg = dawgs_[info.dawg_index];
    // dawg_unichar_id will contain the literal unichar_id to be found in the
    // dawgs (e.g. didgit pattern if unichar_id is a digit and dawg contains
    // number patterns, word pattern if dawg is a puncutation dawg and we
    // reached an end of beginning puntuation pattern, etc).
    UNICHAR_ID dawg_unichar_id = unichar_id;

    // If we are dealing with the pattern dawg, look up all the
    // possible edges, not only for the exact unichar_id, but also
    // for all its character classes (alpha, digit, etc).
    if (dawg->type() == DAWG_TYPE_PATTERN) {
      ProcessPatternEdges(dawg, info, dawg_unichar_id, word_end,
                          dawg_args, &curr_perm);
      // There can't be any successors to dawg that is of type
      // DAWG_TYPE_PATTERN, so we are done examining this DawgInfo.
      continue;
    }

    // The number dawg generalizes all digits to be kPatternUnicharID,
    // so try to match kPatternUnicharID if the current unichar is a digit.
    if (dawg->type() == DAWG_TYPE_NUMBER &&
        getUnicharset().get_isdigit(dawg_unichar_id)) {
      dawg_unichar_id = Dawg::kPatternUnicharID;
    }

    // Find the edge out of the node for the dawg_unichar_id.
    NODE_REF node = GetStartingNode(dawg, info.ref);
    EDGE_REF edge = (node != NO_EDGE) ?
      dawg->edge_char_of(node, dawg_unichar_id, word_end) : NO_EDGE;

    if (dawg_debug_level >= 3) {
      tprintf("Active dawg: [%d, " REFFORMAT "] edge=" REFFORMAT "\n",
              info.dawg_index, node, edge);
    }

    if (edge != NO_EDGE) {  // the unichar was found in the current dawg
      if (ConstraintsOk(*(dawg_args->updated_constraints),
                        word_end, dawg->type())) {
        if (dawg_debug_level >=3) {
          tprintf("Letter found in dawg %d\n", info.dawg_index);
        }
        if (dawg->permuter() > curr_perm) curr_perm = dawg->permuter();
        dawg_args->updated_active_dawgs->add_unique(
            DawgInfo(info.dawg_index, edge), dawg_debug_level > 0,
            "Append current dawg to updated active dawgs: ");
      }
    } else if (dawg_args->sought_word_length == kAnyWordLength) {
      // The unichar was not found in the current dawg.
      // Explore the successor dawgs (but only if we are not
      // just searching one dawg with a fixed word length).

      // Handle leading/trailing punctuation dawgs that denote a word pattern
      // as an edge with kPatternUnicharID. If such an edge is found we add a
      // constraint denoting the state of the dawg before the word pattern.
      // This constraint will be applied later when this dawg is found among
      // successor dawgs as well potentially at the end of the word.
      if (dawg->type() == DAWG_TYPE_PUNCTUATION) {
        edge = dawg->edge_char_of(node, Dawg::kPatternUnicharID, word_end);
        if (edge != NO_EDGE) {
          dawg_args->updated_constraints->add_unique(
              DawgInfo(info.dawg_index, edge), dawg_debug_level > 0,
              "Recording constraint: ");
        } else {
          // Do not explore successors of this dawg, since this
          // must be invalid leading or trailing punctuation.
          if (dawg_debug_level >= 3) {
            tprintf("Invalid punctuation from dawg %d\n", info.dawg_index);
          }
          continue;
        }
      }

      if (info.ref == NO_EDGE) {
        if (dawg_debug_level >= 3) {
          tprintf("No letters matched in dawg %d\n", info.dawg_index);
        }
        continue;
      }

      // Discard the dawg if the pattern can not end at previous letter.
      if (edge == NO_EDGE &&  // previous part is not leading punctuation
          !dawg->end_of_word(info.ref)) {
        if (dawg_debug_level >= 3) {
          tprintf("No valid pattern end in dawg %d\n", info.dawg_index);
        }
        continue;
      }

      // Look for the unichar in each of this dawg's successors
      // and append those in which it is found to active_dawgs.
      const SuccessorList &slist = *(successors_[info.dawg_index]);
      for (int s = 0; s < slist.length(); ++s) {
        int sdawg_index = slist[s];
        const Dawg *sdawg = dawgs_[sdawg_index];
        NODE_REF snode = 0;
        // Apply constraints to the successor dawg.
        for (int c = 0; c < constraints.length(); ++c) {
          // If the successor dawg is described in the constraints change
          // the start ref from 0 to the one recorded as the constraint.
          const DawgInfo &cinfo = constraints[c];
          if (cinfo.dawg_index == sdawg_index) {
            snode = sdawg->next_node(cinfo.ref);
            // Make sure we do not search the successor dawg if after
            // applying the saved constraint we are at the end of the word.
            if (snode == 0) snode = NO_EDGE;
            if (dawg_debug_level >= 3) {
               tprintf("Applying constraint [%d, " REFFORMAT "]\n",
                       sdawg_index, snode);
            }
          }
        }
        // Look for the letter in this successor dawg.
        EDGE_REF sedge = sdawg->edge_char_of(snode, unichar_id, word_end);
        // If we found the letter append sdawg to the active_dawgs list.
        if (sedge != NO_EDGE &&
            ConstraintsOk(*(dawg_args->updated_constraints), word_end,
                          dawgs_[sdawg_index]->type())) {
          if (dawg_debug_level >= 3) {
            tprintf("Letter found in the successor dawg %d\n", sdawg_index);
          }
          if (sdawg->permuter() > curr_perm) curr_perm = sdawg->permuter();
          if (sdawg->next_node(sedge) != 0) {  // if not word end
            dawg_args->updated_active_dawgs->add_unique(
              DawgInfo(sdawg_index, sedge), dawg_debug_level > 0,
              "Append successor to updated active dawgs: ");
          }
        }
      }  // end successors loop
    }  // end if/else
  }  // end for
  // Update dawg_args->permuter if it used to be NO_PERM or became NO_PERM
  // or if we found the current letter in a non-punctuation dawg. This
  // allows preserving information on which dawg the "core" word came from.
  // Keep the old value of dawg_args->permuter if it is COMPOUND_PERM.
  if (dawg_args->permuter == NO_PERM || curr_perm == NO_PERM ||
      (curr_perm != PUNC_PERM && dawg_args->permuter != COMPOUND_PERM)) {
    dawg_args->permuter = curr_perm;
  }
  return dawg_args->permuter;
}

void Dict::ProcessPatternEdges(const Dawg *dawg, const DawgInfo &info,
                               UNICHAR_ID unichar_id, bool word_end,
                               DawgArgs *dawg_args,
                               PermuterType *curr_perm) {
  NODE_REF node = GetStartingNode(dawg, info.ref);
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
    EDGE_REF edge = (k == 0) ?
      dawg->edge_char_of(node, unichar_id_patterns[i], word_end)
      : dawg->pattern_loop_edge(info.ref, unichar_id_patterns[i], word_end);
      if (edge != NO_EDGE) {
        if (dawg_debug_level >= 3) {
          tprintf("Pattern dawg: [%d, " REFFORMAT "] edge=" REFFORMAT "\n",
                  info.dawg_index, node, edge);
        }
        if (ConstraintsOk(*(dawg_args->updated_constraints),
                          word_end, dawg->type())) {
          if (dawg_debug_level >=3) {
            tprintf("Letter found in pattern dawg %d\n", info.dawg_index);
          }
          if (dawg->permuter() > *curr_perm) *curr_perm = dawg->permuter();
          dawg_args->updated_active_dawgs->add_unique(
              DawgInfo(info.dawg_index, edge), dawg_debug_level > 0,
              "Append current dawg to updated active dawgs: ");
        }
      }
    }
  }
}

void Dict::ReadFixedLengthDawgs(DawgType type, const STRING &lang,
                                PermuterType perm, int debug_level,
                                FILE *file, DawgVector *dawg_vec,
                                int *max_wdlen) {
  int i;
  DawgVector dawg_vec_copy;
  dawg_vec_copy.move(dawg_vec); // save the input dawg_vec.
  inT32 num_dawgs;
  fread(&num_dawgs, sizeof(inT32), 1, file);
  bool swap = (num_dawgs > MAX_WERD_LENGTH);
  if (swap) num_dawgs = reverse32(num_dawgs);
  inT32 word_length;
  int max_word_length = 0;
  // Read and record pointers to fixed-length dawgs such that:
  // dawg_vec[word_length] = pointer to dawg with word length of word_length,
  //                         NULL if such fixed-length dawg does not exist.
  for (i = 0; i < num_dawgs; ++i) {
    fread(&word_length, sizeof(inT32), 1, file);
    if (swap) word_length = reverse32(word_length);
    ASSERT_HOST(word_length >  0 && word_length <= MAX_WERD_LENGTH);
    while (word_length >= dawg_vec->size()) dawg_vec->push_back(NULL);
    (*dawg_vec)[word_length] =
      new SquishedDawg(file, type, lang, perm, debug_level);
    if (word_length > max_word_length) max_word_length = word_length;
  }
  *max_wdlen = max_word_length;
  // Entries dawg_vec[0] to dawg_vec[max_word_length] now hold pointers
  // to fixed-length dawgs. The rest of the vector will contain the dawg
  // pointers from the original input dawg_vec.
  for (i = 0; i < dawg_vec_copy.size(); ++i) {
    dawg_vec->push_back(dawg_vec_copy[i]);
  }
}

void Dict::WriteFixedLengthDawgs(
    const GenericVector<SquishedDawg *> &dawg_vec,
    int num_dawgs, int debug_level, FILE *output_file) {
  fwrite(&num_dawgs, sizeof(inT32), 1, output_file);
  if (debug_level) tprintf("Writing %d split length dawgs\n", num_dawgs);
  for (int i = 1; i < dawg_vec.size(); ++i) {
    if ((dawg_vec)[i] != NULL) {
      fwrite(&i, sizeof(inT32), 1, output_file);
      dawg_vec[i]->write_squished_dawg(output_file);
      if (debug_level) tprintf("Wrote Dawg with word length %d\n", i);
    }
  }
}

// Fill the given active_dawgs vector with dawgs that could contain the
// beginning of the word. If hyphenated() returns true, copy the entries
// from hyphen_active_dawgs_ instead.
void Dict::init_active_dawgs(int sought_word_length,
                             DawgInfoVector *active_dawgs,
                             bool ambigs_mode) {
  int i;
  if (sought_word_length != kAnyWordLength) {
    // Only search one fixed word length dawg.
    if (sought_word_length <= max_fixed_length_dawgs_wdlen_ &&
        dawgs_[sought_word_length] != NULL) {
      *active_dawgs += DawgInfo(sought_word_length, NO_EDGE);
    }
  } else if (hyphenated()) {
    *active_dawgs = hyphen_active_dawgs_;
    if (dawg_debug_level >= 3) {
      for (i = 0; i < hyphen_active_dawgs_.size(); ++i) {
        tprintf("Adding hyphen beginning dawg [%d, " REFFORMAT "]\n",
                hyphen_active_dawgs_[i].dawg_index,
                hyphen_active_dawgs_[i].ref);
      }
    }
  } else {
    for (i = 0; i < dawgs_.length(); ++i) {
      if (dawgs_[i] != NULL && kBeginningDawgsType[(dawgs_[i])->type()] &&
          !(ambigs_mode && (dawgs_[i])->type() == DAWG_TYPE_PATTERN)) {
        *active_dawgs += DawgInfo(i, NO_EDGE);
        if (dawg_debug_level >= 3) {
          tprintf("Adding beginning dawg [%d, " REFFORMAT "]\n", i, NO_EDGE);
        }
      }
    }
  }
}

// If hyphenated() returns true, copy the entries from hyphen_constraints_
// into the given constraints vector.
void Dict::init_constraints(DawgInfoVector *constraints) {
  if (hyphenated()) {
    *constraints = hyphen_constraints_;
    if (dawg_debug_level >= 3) {
      for (int i = 0; i < hyphen_constraints_.size(); ++i) {
        tprintf("Adding hyphen constraint [%d, " REFFORMAT "]\n",
                hyphen_constraints_[i].dawg_index,
                hyphen_constraints_[i].ref);
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

  if (!doc_dict_enable || valid_word(best_choice) ||
      CurrentWordAmbig() || stringlen < 2)
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
    strcpy(filename, getImage()->getCCUtil()->imagefile.string());
    strcat(filename, ".doc");
    doc_word_file = open_file (filename, "a");
    fprintf(doc_word_file, "%s\n",
            best_choice.debug_string(getUnicharset()).string());
    fclose(doc_word_file);
  }
  document_words_->add_word_to_dawg(best_choice);
}

void Dict::adjust_word(WERD_CHOICE *word,
                       float *certainty_array,
                       const BLOB_CHOICE_LIST_VECTOR *char_choices,
                       bool nonword,
                       float additional_adjust,
                       bool debug) {
  bool is_han = (char_choices != NULL &&
                 getUnicharset().han_sid() != getUnicharset().null_sid() &&
                 get_top_word_script(*char_choices, getUnicharset()) ==
                 getUnicharset().han_sid());
  bool case_is_ok = (is_han || case_ok(*word, getUnicharset()));
  bool punc_is_ok = (is_han || !nonword || valid_punctuation(*word));

  float adjust_factor = additional_adjust;
  float new_rating = word->rating();
  if (debug) {
    tprintf("%sWord: %s %4.2f ", nonword ? "Non-" : "",
            word->debug_string(getUnicharset()).string(), word->rating());
  }
  new_rating += kRatingPad;
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
  word->set_rating(new_rating);
  if (debug) tprintf(" %4.2f --> %4.2f\n", adjust_factor, new_rating);
  LogNewChoice(adjust_factor, certainty_array, false, word);
}

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
  init_active_dawgs(kAnyWordLength, &(active_dawgs[0]), false);
  init_constraints(&(constraints[0]));
  DawgArgs dawg_args(&(active_dawgs[0]), &(constraints[0]),
                     &(active_dawgs[1]), &(constraints[1]),
                     0.0, NO_PERM, kAnyWordLength, 0);
  int last_index = word_ptr->length() - 1;
  // Call leter_is_okay for each letter in the word.
  for (int i = hyphen_base_size(); i <= last_index; ++i) {
    if (!((this->*letter_is_okay_)(&dawg_args, word_ptr->unichar_id(i),
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
  return valid_word_permuter(dawg_args.permuter, numbers_ok) ?
    dawg_args.permuter : NO_PERM;
}

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
    if (dawgs_[i] != NULL &&
        dawgs_[i]->type() == DAWG_TYPE_PUNCTUATION &&
        dawgs_[i]->word_in_dawg(new_word)) return true;
  }
  return false;
}

// Returns the "dominant" script ID for the word.  By "dominant", the script
// must account for at least half the characters.  Otherwise, it returns 0.
// Note that for Japanese, Hiragana and Katakana are simply treated as Han.
int Dict::get_top_word_script(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                              const UNICHARSET &unicharset) {
  int max_script = unicharset.get_script_table_size();
  int *sid = new int[max_script];
  int x;
  for (x = 0; x < max_script; x++) sid[x] = 0;
  for (x = 0; x < char_choices.length(); ++x) {
    BLOB_CHOICE_IT blob_choice_it(char_choices.get(x));
    sid[blob_choice_it.data()->script_id()]++;
  }
  if (unicharset.han_sid() != unicharset.null_sid()) {
    // Add the Hiragana & Katakana counts to Han and zero them out.
    if (unicharset.hiragana_sid() != unicharset.null_sid()) {
      sid[unicharset.han_sid()] += sid[unicharset.hiragana_sid()];
      sid[unicharset.hiragana_sid()] = 0;
    }
    if (unicharset.katakana_sid() != unicharset.null_sid()) {
      sid[unicharset.han_sid()] += sid[unicharset.katakana_sid()];
      sid[unicharset.katakana_sid()] = 0;
    }
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

}  // namespace tesseract
