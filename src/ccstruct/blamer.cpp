///////////////////////////////////////////////////////////////////////
// File:        blamer.cpp
// Description: Module allowing precise error causes to be allocated.
// Author:      Rike Antonova
// Refactored:  Ray Smith
//
// (C) Copyright 2013, Google Inc.
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

#include "blamer.h"

#include "blobs.h"   // for TPOINT, TWERD, TBLOB
#include "errcode.h" // for ASSERT_HOST
#if !defined(DISABLED_LEGACY_ENGINE)
#  include "lm_pain_points.h" // for LMPainPoints
#endif
#include "matrix.h"     // for MATRIX
#include "normalis.h"   // for DENORM
#include "pageres.h"    // for WERD_RES
#include "unicharset.h" // for UNICHARSET

#include <cmath>   // for abs
#include <cstdlib> // for abs

namespace tesseract {

// Names for each value of IncorrectResultReason enum. Keep in sync.
const char kBlameCorrect[] = "corr";
const char kBlameClassifier[] = "cl";
const char kBlameChopper[] = "chop";
const char kBlameClassLMTradeoff[] = "cl/LM";
const char kBlamePageLayout[] = "pglt";
const char kBlameSegsearchHeur[] = "ss_heur";
const char kBlameSegsearchPP[] = "ss_pp";
const char kBlameClassOldLMTradeoff[] = "cl/old_LM";
const char kBlameAdaption[] = "adapt";
const char kBlameNoTruthSplit[] = "no_tr_spl";
const char kBlameNoTruth[] = "no_tr";
const char kBlameUnknown[] = "unkn";

const char *const kIncorrectResultReasonNames[] = {
    kBlameCorrect,    kBlameClassifier,    kBlameChopper,     kBlameClassLMTradeoff,
    kBlamePageLayout, kBlameSegsearchHeur, kBlameSegsearchPP, kBlameClassOldLMTradeoff,
    kBlameAdaption,   kBlameNoTruthSplit,  kBlameNoTruth,     kBlameUnknown};

const char *BlamerBundle::IncorrectReasonName(IncorrectResultReason irr) {
  return kIncorrectResultReasonNames[irr];
}

const char *BlamerBundle::IncorrectReason() const {
  return kIncorrectResultReasonNames[incorrect_result_reason_];
}

// Functions to setup the blamer.
// Whole word string, whole word bounding box.
void BlamerBundle::SetWordTruth(const UNICHARSET &unicharset, const char *truth_str,
                                const TBOX &word_box) {
  truth_word_.InsertBox(0, word_box);
  truth_has_char_boxes_ = false;
  // Encode the string as UNICHAR_IDs.
  std::vector<UNICHAR_ID> encoding;
  std::vector<char> lengths;
  unicharset.encode_string(truth_str, false, &encoding, &lengths, nullptr);
  int total_length = 0;
  for (size_t i = 0; i < encoding.size(); total_length += lengths[i++]) {
    std::string uch(truth_str + total_length);
    uch.resize(lengths[i] - total_length);
    UNICHAR_ID id = encoding[i];
    if (id != INVALID_UNICHAR_ID) {
      uch = unicharset.get_normed_unichar(id);
    }
    truth_text_.push_back(uch);
  }
}

// Single "character" string, "character" bounding box.
// May be called multiple times to indicate the characters in a word.
void BlamerBundle::SetSymbolTruth(const UNICHARSET &unicharset, const char *char_str,
                                  const TBOX &char_box) {
  std::string symbol_str(char_str);
  UNICHAR_ID id = unicharset.unichar_to_id(char_str);
  if (id != INVALID_UNICHAR_ID) {
    std::string normed_uch(unicharset.get_normed_unichar(id));
    if (normed_uch.length() > 0) {
      symbol_str = normed_uch;
    }
  }
  int length = truth_word_.length();
  truth_text_.push_back(symbol_str);
  truth_word_.InsertBox(length, char_box);
  if (length == 0) {
    truth_has_char_boxes_ = true;
  } else if (truth_word_.BlobBox(length - 1) == char_box) {
    truth_has_char_boxes_ = false;
  }
}

// Marks that there is something wrong with the truth text, like it contains
// reject characters.
void BlamerBundle::SetRejectedTruth() {
  incorrect_result_reason_ = IRR_NO_TRUTH;
  truth_has_char_boxes_ = false;
}

// Returns true if the provided word_choice is correct.
bool BlamerBundle::ChoiceIsCorrect(const WERD_CHOICE *word_choice) const {
  if (word_choice == nullptr) {
    return false;
  }
  const UNICHARSET *uni_set = word_choice->unicharset();
  std::string normed_choice_str;
  for (unsigned i = 0; i < word_choice->length(); ++i) {
    normed_choice_str += uni_set->get_normed_unichar(word_choice->unichar_id(i));
  }
  std::string truth_str = TruthString();
  return truth_str == normed_choice_str;
}

void BlamerBundle::FillDebugString(const std::string &msg, const WERD_CHOICE *choice, std::string &debug) {
  debug += "Truth ";
  for (auto &text : this->truth_text_) {
    debug += text;
  }
  if (!this->truth_has_char_boxes_) {
    debug += " (no char boxes)";
  }
  if (choice != nullptr) {
    debug += " Choice ";
    std::string choice_str;
    choice->string_and_lengths(&choice_str, nullptr);
    debug += choice_str;
  }
  if (msg.length() > 0) {
    debug += "\n";
    debug += msg;
  }
  debug += "\n";
}

// Sets up the norm_truth_word from truth_word using the given DENORM.
void BlamerBundle::SetupNormTruthWord(const DENORM &denorm) {
  // TODO(rays) Is this the last use of denorm in WERD_RES and can it go?
  norm_box_tolerance_ = kBlamerBoxTolerance * denorm.x_scale();
  TPOINT topleft;
  TPOINT botright;
  TPOINT norm_topleft;
  TPOINT norm_botright;
  for (unsigned b = 0; b < truth_word_.length(); ++b) {
    const TBOX &box = truth_word_.BlobBox(b);
    topleft.x = box.left();
    topleft.y = box.top();
    botright.x = box.right();
    botright.y = box.bottom();
    denorm.NormTransform(nullptr, topleft, &norm_topleft);
    denorm.NormTransform(nullptr, botright, &norm_botright);
    TBOX norm_box(norm_topleft.x, norm_botright.y, norm_botright.x, norm_topleft.y);
    norm_truth_word_.InsertBox(b, norm_box);
  }
}

// Splits *this into two pieces in bundle1 and bundle2 (preallocated, empty
// bundles) where the right edge/ of the left-hand word is word1_right,
// and the left edge of the right-hand word is word2_left.
void BlamerBundle::SplitBundle(int word1_right, int word2_left, bool debug, BlamerBundle *bundle1,
                               BlamerBundle *bundle2) const {
  std::string debug_str;
  // Find truth boxes that correspond to the split in the blobs.
  unsigned begin2_truth_index = 0;
  if (incorrect_result_reason_ != IRR_NO_TRUTH && truth_has_char_boxes_) {
    debug_str = "Looking for truth split at";
    debug_str += " end1_x " + std::to_string(word1_right);
    debug_str += " begin2_x " + std::to_string(word2_left);
    debug_str += "\nnorm_truth_word boxes:\n";
    if (norm_truth_word_.length() > 1) {
      norm_truth_word_.BlobBox(0).print_to_str(debug_str);
      for (unsigned b = 1; b < norm_truth_word_.length(); ++b) {
        norm_truth_word_.BlobBox(b).print_to_str(debug_str);
        if ((abs(word1_right - norm_truth_word_.BlobBox(b - 1).right()) < norm_box_tolerance_) &&
            (abs(word2_left - norm_truth_word_.BlobBox(b).left()) < norm_box_tolerance_)) {
          begin2_truth_index = b;
          debug_str += "Split found";
          break;
        }
      }
      debug_str += '\n';
    }
  }
  // Populate truth information in word and word2 with the first and second
  // part of the original truth.
  if (begin2_truth_index > 0) {
    bundle1->truth_has_char_boxes_ = true;
    bundle1->norm_box_tolerance_ = norm_box_tolerance_;
    bundle2->truth_has_char_boxes_ = true;
    bundle2->norm_box_tolerance_ = norm_box_tolerance_;
    BlamerBundle *curr_bb = bundle1;
    for (unsigned b = 0; b < norm_truth_word_.length(); ++b) {
      if (b == begin2_truth_index) {
        curr_bb = bundle2;
      }
      curr_bb->norm_truth_word_.InsertBox(b, norm_truth_word_.BlobBox(b));
      curr_bb->truth_word_.InsertBox(b, truth_word_.BlobBox(b));
      curr_bb->truth_text_.push_back(truth_text_[b]);
    }
  } else if (incorrect_result_reason_ == IRR_NO_TRUTH) {
    bundle1->incorrect_result_reason_ = IRR_NO_TRUTH;
    bundle2->incorrect_result_reason_ = IRR_NO_TRUTH;
  } else {
    debug_str += "Truth split not found";
    debug_str += truth_has_char_boxes_ ? "\n" : " (no truth char boxes)\n";
    bundle1->SetBlame(IRR_NO_TRUTH_SPLIT, debug_str, nullptr, debug);
    bundle2->SetBlame(IRR_NO_TRUTH_SPLIT, debug_str, nullptr, debug);
  }
}

// "Joins" the blames from bundle1 and bundle2 into *this.
void BlamerBundle::JoinBlames(const BlamerBundle &bundle1, const BlamerBundle &bundle2,
                              bool debug) {
  std::string debug_str;
  IncorrectResultReason irr = incorrect_result_reason_;
  if (irr != IRR_NO_TRUTH_SPLIT) {
    debug_str = "";
  }
  if (bundle1.incorrect_result_reason_ != IRR_CORRECT &&
      bundle1.incorrect_result_reason_ != IRR_NO_TRUTH &&
      bundle1.incorrect_result_reason_ != IRR_NO_TRUTH_SPLIT) {
    debug_str += "Blame from part 1: ";
    debug_str += bundle1.debug_;
    irr = bundle1.incorrect_result_reason_;
  }
  if (bundle2.incorrect_result_reason_ != IRR_CORRECT &&
      bundle2.incorrect_result_reason_ != IRR_NO_TRUTH &&
      bundle2.incorrect_result_reason_ != IRR_NO_TRUTH_SPLIT) {
    debug_str += "Blame from part 2: ";
    debug_str += bundle2.debug_;
    if (irr == IRR_CORRECT) {
      irr = bundle2.incorrect_result_reason_;
    } else if (irr != bundle2.incorrect_result_reason_) {
      irr = IRR_UNKNOWN;
    }
  }
  incorrect_result_reason_ = irr;
  if (irr != IRR_CORRECT && irr != IRR_NO_TRUTH) {
    SetBlame(irr, debug_str, nullptr, debug);
  }
}

// If a blob with the same bounding box as one of the truth character
// bounding boxes is not classified as the corresponding truth character
// blames character classifier for incorrect answer.
void BlamerBundle::BlameClassifier(const UNICHARSET &unicharset, const TBOX &blob_box,
                                   const BLOB_CHOICE_LIST &choices, bool debug) {
  if (!truth_has_char_boxes_ || incorrect_result_reason_ != IRR_CORRECT) {
    return; // Nothing to do here.
  }

  for (unsigned b = 0; b < norm_truth_word_.length(); ++b) {
    const TBOX &truth_box = norm_truth_word_.BlobBox(b);
    // Note that we are more strict on the bounding box boundaries here
    // than in other places (chopper, segmentation search), since we do
    // not have the ability to check the previous and next bounding box.
    if (blob_box.x_almost_equal(truth_box, norm_box_tolerance_ / 2)) {
      bool found = false;
      bool incorrect_adapted = false;
      UNICHAR_ID incorrect_adapted_id = INVALID_UNICHAR_ID;
      const char *truth_str = truth_text_[b].c_str();
      // We promise not to modify the list or its contents, using a
      // const BLOB_CHOICE* below.
      BLOB_CHOICE_IT choices_it(const_cast<BLOB_CHOICE_LIST *>(&choices));
      for (choices_it.mark_cycle_pt(); !choices_it.cycled_list(); choices_it.forward()) {
        const BLOB_CHOICE *choice = choices_it.data();
        if (strcmp(truth_str, unicharset.get_normed_unichar(choice->unichar_id())) == 0) {
          found = true;
          break;
        } else if (choice->IsAdapted()) {
          incorrect_adapted = true;
          incorrect_adapted_id = choice->unichar_id();
        }
      } // end choices_it for loop
      if (!found) {
        std::string debug_str = "unichar ";
        debug_str += truth_str;
        debug_str += " not found in classification list";
        SetBlame(IRR_CLASSIFIER, debug_str, nullptr, debug);
      } else if (incorrect_adapted) {
        std::string debug_str = "better rating for adapted ";
        debug_str += unicharset.id_to_unichar(incorrect_adapted_id);
        debug_str += " than for correct ";
        debug_str += truth_str;
        SetBlame(IRR_ADAPTION, debug_str, nullptr, debug);
      }
      break;
    }
  } // end iterating over blamer_bundle->norm_truth_word
}

// Checks whether chops were made at all the character bounding box
// boundaries in word->truth_word. If not - blames the chopper for an
// incorrect answer.
void BlamerBundle::SetChopperBlame(const WERD_RES *word, bool debug) {
  if (NoTruth() || !truth_has_char_boxes_ || word->chopped_word->blobs.empty()) {
    return;
  }
  bool missing_chop = false;
  int num_blobs = word->chopped_word->blobs.size();
  unsigned box_index = 0;
  int blob_index = 0;
  int16_t truth_x = -1;
  while (box_index < truth_word_.length() && blob_index < num_blobs) {
    truth_x = norm_truth_word_.BlobBox(box_index).right();
    TBLOB *curr_blob = word->chopped_word->blobs[blob_index];
    if (curr_blob->bounding_box().right() < truth_x - norm_box_tolerance_) {
      ++blob_index;
      continue; // encountered an extra chop, keep looking
    } else if (curr_blob->bounding_box().right() > truth_x + norm_box_tolerance_) {
      missing_chop = true;
      break;
    } else {
      ++blob_index;
    }
  }
  if (missing_chop || box_index < norm_truth_word_.length()) {
    std::string debug_str;
    if (missing_chop) {
      debug_str += "Detected missing chop (tolerance=" + std::to_string(norm_box_tolerance_);
      debug_str += ") at Bounding Box=";
      TBLOB *curr_blob = word->chopped_word->blobs[blob_index];
      curr_blob->bounding_box().print_to_str(debug_str);
      debug_str += "\nNo chop for truth at x=" + std::to_string(truth_x);
    } else {
      debug_str += "Missing chops for last " + std::to_string(norm_truth_word_.length() - box_index);
      debug_str += " truth box(es)";
    }
    debug_str += "\nMaximally chopped word boxes:\n";
    for (blob_index = 0; blob_index < num_blobs; ++blob_index) {
      TBLOB *curr_blob = word->chopped_word->blobs[blob_index];
      curr_blob->bounding_box().print_to_str(debug_str);
      debug_str += '\n';
    }
    debug_str += "Truth  bounding  boxes:\n";
    for (box_index = 0; box_index < norm_truth_word_.length(); ++box_index) {
      norm_truth_word_.BlobBox(box_index).print_to_str(debug_str);
      debug_str += '\n';
    }
    SetBlame(IRR_CHOPPER, debug_str, word->best_choice, debug);
  }
}

// Blames the classifier or the language model if, after running only the
// chopper, best_choice is incorrect and no blame has been yet set.
// Blames the classifier if best_choice is classifier's top choice and is a
// dictionary word (i.e. language model could not have helped).
// Otherwise, blames the language model (formerly permuter word adjustment).
void BlamerBundle::BlameClassifierOrLangModel(const WERD_RES *word, const UNICHARSET &unicharset,
                                              bool valid_permuter, bool debug) {
  if (valid_permuter) {
    // Find out whether best choice is a top choice.
    best_choice_is_dict_and_top_choice_ = true;
    for (unsigned i = 0; i < word->best_choice->length(); ++i) {
      BLOB_CHOICE_IT blob_choice_it(word->GetBlobChoices(i));
      ASSERT_HOST(!blob_choice_it.empty());
      BLOB_CHOICE *first_choice = nullptr;
      for (blob_choice_it.mark_cycle_pt(); !blob_choice_it.cycled_list();
           blob_choice_it.forward()) { // find first non-fragment choice
        if (!(unicharset.get_fragment(blob_choice_it.data()->unichar_id()))) {
          first_choice = blob_choice_it.data();
          break;
        }
      }
      ASSERT_HOST(first_choice != nullptr);
      if (first_choice->unichar_id() != word->best_choice->unichar_id(i)) {
        best_choice_is_dict_and_top_choice_ = false;
        break;
      }
    }
  }
  std::string debug_str;
  if (best_choice_is_dict_and_top_choice_) {
    debug_str = "Best choice is: incorrect, top choice, dictionary word";
    debug_str += " with permuter ";
    debug_str += word->best_choice->permuter_name();
  } else {
    debug_str = "Classifier/Old LM tradeoff is to blame";
  }
  SetBlame(best_choice_is_dict_and_top_choice_ ? IRR_CLASSIFIER : IRR_CLASS_OLD_LM_TRADEOFF,
           debug_str, word->best_choice, debug);
}

// Sets up the correct_segmentation_* to mark the correct bounding boxes.
void BlamerBundle::SetupCorrectSegmentation(const TWERD *word, bool debug) {
#ifndef DISABLED_LEGACY_ENGINE
  params_training_bundle_.StartHypothesisList();
#endif //  ndef DISABLED_LEGACY_ENGINE
  if (incorrect_result_reason_ != IRR_CORRECT || !truth_has_char_boxes_) {
    return; // Nothing to do here.
  }

  std::string debug_str = "Blamer computing correct_segmentation_cols\n";
  int curr_box_col = 0;
  int next_box_col = 0;
  int num_blobs = word->NumBlobs();
  if (num_blobs == 0) {
    return; // No blobs to play with.
  }
  int blob_index = 0;
  int16_t next_box_x = word->blobs[blob_index]->bounding_box().right();
  for (unsigned truth_idx = 0; blob_index < num_blobs && truth_idx < norm_truth_word_.length();
       ++blob_index) {
    ++next_box_col;
    int16_t curr_box_x = next_box_x;
    if (blob_index + 1 < num_blobs) {
      next_box_x = word->blobs[blob_index + 1]->bounding_box().right();
    }
    int16_t truth_x = norm_truth_word_.BlobBox(truth_idx).right();
    debug_str += "Box x coord vs. truth: " + std::to_string(curr_box_x);
    debug_str += " " + std::to_string(truth_x);
    debug_str += "\n";
    if (curr_box_x > (truth_x + norm_box_tolerance_)) {
      break;                                                  // failed to find a matching box
    } else if (curr_box_x >= truth_x - norm_box_tolerance_ && // matched
               (blob_index + 1 >= num_blobs ||                // next box can't be included
                next_box_x > truth_x + norm_box_tolerance_)) {
      correct_segmentation_cols_.push_back(curr_box_col);
      correct_segmentation_rows_.push_back(next_box_col - 1);
      ++truth_idx;
      debug_str += "col=" + std::to_string(curr_box_col);
      debug_str += " row=" + std::to_string(next_box_col - 1);
      debug_str += "\n";
      curr_box_col = next_box_col;
    }
  }
  if (blob_index < num_blobs || // trailing blobs
      correct_segmentation_cols_.size() != norm_truth_word_.length()) {
    debug_str +=
        "Blamer failed to find correct segmentation"
        " (tolerance=" +
        std::to_string(norm_box_tolerance_);
    if (blob_index >= num_blobs) {
      debug_str += " blob == nullptr";
    }
    debug_str += ")\n";
    debug_str += " path length " + std::to_string(correct_segmentation_cols_.size());
    debug_str += " vs. truth " + std::to_string(norm_truth_word_.length());
    debug_str += "\n";
    SetBlame(IRR_UNKNOWN, debug_str, nullptr, debug);
    correct_segmentation_cols_.clear();
    correct_segmentation_rows_.clear();
  }
}

// Returns true if a guided segmentation search is needed.
bool BlamerBundle::GuidedSegsearchNeeded(const WERD_CHOICE *best_choice) const {
  return incorrect_result_reason_ == IRR_CORRECT && !segsearch_is_looking_for_blame_ &&
         truth_has_char_boxes_ && !ChoiceIsCorrect(best_choice);
}

#if !defined(DISABLED_LEGACY_ENGINE)
// Setup ready to guide the segmentation search to the correct segmentation.
void BlamerBundle::InitForSegSearch(const WERD_CHOICE *best_choice, MATRIX *ratings,
                                    UNICHAR_ID wildcard_id, bool debug, std::string &debug_str,
                                    tesseract::LMPainPoints *pain_points, double max_char_wh_ratio,
                                    WERD_RES *word_res) {
  segsearch_is_looking_for_blame_ = true;
  if (debug) {
    tprintf("segsearch starting to look for blame\n");
  }
  // Fill pain points for any unclassifed blob corresponding to the
  // correct segmentation state.
  debug_str += "Correct segmentation:\n";
  for (unsigned idx = 0; idx < correct_segmentation_cols_.size(); ++idx) {
    debug_str += "col=" + std::to_string(correct_segmentation_cols_[idx]);
    debug_str += " row=" + std::to_string(correct_segmentation_rows_[idx]);
    debug_str += "\n";
    if (!ratings->Classified(correct_segmentation_cols_[idx], correct_segmentation_rows_[idx],
                             wildcard_id) &&
        !pain_points->GeneratePainPoint(
            correct_segmentation_cols_[idx], correct_segmentation_rows_[idx],
            tesseract::LM_PPTYPE_BLAMER, 0.0, false, max_char_wh_ratio, word_res)) {
      segsearch_is_looking_for_blame_ = false;
      debug_str += "\nFailed to insert pain point\n";
      SetBlame(IRR_SEGSEARCH_HEUR, debug_str, best_choice, debug);
      break;
    }
  } // end for blamer_bundle->correct_segmentation_cols/rows
}
#endif // !defined(DISABLED_LEGACY_ENGINE)

// Returns true if the guided segsearch is in progress.
bool BlamerBundle::GuidedSegsearchStillGoing() const {
  return segsearch_is_looking_for_blame_;
}

// The segmentation search has ended. Sets the blame appropriately.
void BlamerBundle::FinishSegSearch(const WERD_CHOICE *best_choice, bool debug, std::string &debug_str) {
  // If we are still looking for blame (i.e. best_choice is incorrect, but a
  // path representing the correct segmentation could be constructed), we can
  // blame segmentation search pain point prioritization if the rating of the
  // path corresponding to the correct segmentation is better than that of
  // best_choice (i.e. language model would have done the correct thing, but
  // because of poor pain point prioritization the correct segmentation was
  // never explored). Otherwise we blame the tradeoff between the language model
  // and the classifier, since even after exploring the path corresponding to
  // the correct segmentation incorrect best_choice would have been chosen.
  // One special case when we blame the classifier instead is when best choice
  // is incorrect, but it is a dictionary word and it classifier's top choice.
  if (segsearch_is_looking_for_blame_) {
    segsearch_is_looking_for_blame_ = false;
    if (best_choice_is_dict_and_top_choice_) {
      debug_str = "Best choice is: incorrect, top choice, dictionary word";
      debug_str += " with permuter ";
      debug_str += best_choice->permuter_name();
      SetBlame(IRR_CLASSIFIER, debug_str, best_choice, debug);
    } else if (best_correctly_segmented_rating_ < best_choice->rating()) {
      debug_str += "Correct segmentation state was not explored";
      SetBlame(IRR_SEGSEARCH_PP, debug_str, best_choice, debug);
    } else {
      if (best_correctly_segmented_rating_ >= WERD_CHOICE::kBadRating) {
        debug_str += "Correct segmentation paths were pruned by LM\n";
      } else {
        debug_str += "Best correct segmentation rating " +
                                  std::to_string(best_correctly_segmented_rating_);
        debug_str += " vs. best choice rating " + std::to_string(best_choice->rating());
      }
      SetBlame(IRR_CLASS_LM_TRADEOFF, debug_str, best_choice, debug);
    }
  }
}

// If the bundle is null or still does not indicate the correct result,
// fix it and use some backup reason for the blame.
void BlamerBundle::LastChanceBlame(bool debug, WERD_RES *word) {
  if (word->blamer_bundle == nullptr) {
    word->blamer_bundle = new BlamerBundle();
    word->blamer_bundle->SetBlame(IRR_PAGE_LAYOUT, "LastChanceBlame", word->best_choice, debug);
  } else if (word->blamer_bundle->incorrect_result_reason_ == IRR_NO_TRUTH) {
    word->blamer_bundle->SetBlame(IRR_NO_TRUTH, "Rejected truth", word->best_choice, debug);
  } else {
    bool correct = word->blamer_bundle->ChoiceIsCorrect(word->best_choice);
    IncorrectResultReason irr = word->blamer_bundle->incorrect_result_reason_;
    if (irr == IRR_CORRECT && !correct) {
      std::string debug_str = "Choice is incorrect after recognition";
      word->blamer_bundle->SetBlame(IRR_UNKNOWN, debug_str, word->best_choice, debug);
    } else if (irr != IRR_CORRECT && correct) {
      if (debug) {
        tprintf("Corrected %s\n", word->blamer_bundle->debug_.c_str());
      }
      word->blamer_bundle->incorrect_result_reason_ = IRR_CORRECT;
      word->blamer_bundle->debug_ = "";
    }
  }
}

// Sets the misadaption debug if this word is incorrect, as this word is
// being adapted to.
void BlamerBundle::SetMisAdaptionDebug(const WERD_CHOICE *best_choice, bool debug) {
  if (incorrect_result_reason_ != IRR_NO_TRUTH && !ChoiceIsCorrect(best_choice)) {
    misadaption_debug_ = "misadapt to word (";
    misadaption_debug_ += best_choice->permuter_name();
    misadaption_debug_ += "): ";
    FillDebugString("", best_choice, misadaption_debug_);
    if (debug) {
      tprintf("%s\n", misadaption_debug_.c_str());
    }
  }
}

} // namespace tesseract
