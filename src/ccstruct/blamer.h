///////////////////////////////////////////////////////////////////////
// File:        blamer.h
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

#ifndef TESSERACT_CCSTRUCT_BLAMER_H_
#define TESSERACT_CCSTRUCT_BLAMER_H_

#ifdef HAVE_CONFIG_H
#  include "config_auto.h" // DISABLED_LEGACY_ENGINE
#endif
#include "boxword.h" // for BoxWord
#ifndef DISABLED_LEGACY_ENGINE
#  include "params_training_featdef.h" // for ParamsTrainingBundle, ParamsTra...
#endif                                 //  ndef DISABLED_LEGACY_ENGINE
#include "ratngs.h"                    // for BLOB_CHOICE_LIST (ptr only)
#include "rect.h"                      // for TBOX
#include "tprintf.h"                   // for tprintf

#include <tesseract/unichar.h> // for UNICHAR_ID

#include <cstdint> // for int16_t
#include <cstring> // for memcpy
#include <vector>  // for std::vector

namespace tesseract {

class DENORM;
class MATRIX;
class UNICHARSET;
class WERD_RES;

struct MATRIX_COORD;
struct TWERD;

class LMPainPoints;

static const int16_t kBlamerBoxTolerance = 5;

// Enum for expressing the source of error.
// Note: Please update kIncorrectResultReasonNames when modifying this enum.
enum IncorrectResultReason {
  // The text recorded in best choice == truth text
  IRR_CORRECT,
  // Either: Top choice is incorrect and is a dictionary word (language model
  // is unlikely to help correct such errors, so blame the classifier).
  // Or: the correct unichar was not included in shortlist produced by the
  // classifier at all.
  IRR_CLASSIFIER,
  // Chopper have not found one or more splits that correspond to the correct
  // character bounding boxes recorded in BlamerBundle::truth_word.
  IRR_CHOPPER,
  // Classifier did include correct unichars for each blob in the correct
  // segmentation, however its rating could have been too bad to allow the
  // language model to pull out the correct choice. On the other hand the
  // strength of the language model might have been too weak to favor the
  // correct answer, this we call this case a classifier-language model
  // tradeoff error.
  IRR_CLASS_LM_TRADEOFF,
  // Page layout failed to produce the correct bounding box. Blame page layout
  // if the truth was not found for the word, which implies that the bounding
  // box of the word was incorrect (no truth word had a similar bounding box).
  IRR_PAGE_LAYOUT,
  // SegSearch heuristic prevented one or more blobs from the correct
  // segmentation state to be classified (e.g. the blob was too wide).
  IRR_SEGSEARCH_HEUR,
  // The correct segmentaiton state was not explored because of poor SegSearch
  // pain point prioritization. We blame SegSearch pain point prioritization
  // if the best rating of a choice constructed from correct segmentation is
  // better than that of the best choice (i.e. if we got to explore the correct
  // segmentation state, language model would have picked the correct choice).
  IRR_SEGSEARCH_PP,
  // Same as IRR_CLASS_LM_TRADEOFF, but used when we only run chopper on a word,
  // and thus use the old language model (permuters).
  // TODO(antonova): integrate the new language mode with chopper
  IRR_CLASS_OLD_LM_TRADEOFF,
  // If there is an incorrect adaptive template match with a better score than
  // a correct one (either pre-trained or adapted), mark this as adaption error.
  IRR_ADAPTION,
  // split_and_recog_word() failed to find a suitable split in truth.
  IRR_NO_TRUTH_SPLIT,
  // Truth is not available for this word (e.g. when words in corrected content
  // file are turned into ~~~~ because an appropriate alignment was not found.
  IRR_NO_TRUTH,
  // The text recorded in best choice != truth text, but none of the above
  // reasons are set.
  IRR_UNKNOWN,

  IRR_NUM_REASONS
};

// Blamer-related information to determine the source of errors.
struct BlamerBundle {
  static const char *IncorrectReasonName(IncorrectResultReason irr);
  BlamerBundle()
      : truth_has_char_boxes_(false)
      , incorrect_result_reason_(IRR_CORRECT)
      , lattice_data_(nullptr) {
    ClearResults();
  }
  BlamerBundle(const BlamerBundle &other) {
    this->CopyTruth(other);
    this->CopyResults(other);
  }
  ~BlamerBundle() {
    delete[] lattice_data_;
  }

  // Accessors.
  std::string TruthString() const {
    std::string truth_str;
    for (auto &text : truth_text_) {
      truth_str += text;
    }
    return truth_str;
  }
  IncorrectResultReason incorrect_result_reason() const {
    return incorrect_result_reason_;
  }
  bool NoTruth() const {
    return incorrect_result_reason_ == IRR_NO_TRUTH || incorrect_result_reason_ == IRR_PAGE_LAYOUT;
  }
  bool HasDebugInfo() const {
    return debug_.length() > 0 || misadaption_debug_.length() > 0;
  }
  const std::string &debug() const {
    return debug_;
  }
  const std::string &misadaption_debug() const {
    return misadaption_debug_;
  }
  void UpdateBestRating(float rating) {
    if (rating < best_correctly_segmented_rating_) {
      best_correctly_segmented_rating_ = rating;
    }
  }
  int correct_segmentation_length() const {
    return correct_segmentation_cols_.size();
  }
  // Returns true if the given ratings matrix col,row position is included
  // in the correct segmentation path at the given index.
  bool MatrixPositionCorrect(int index, const MATRIX_COORD &coord) {
    return correct_segmentation_cols_[index] == coord.col &&
           correct_segmentation_rows_[index] == coord.row;
  }
  void set_best_choice_is_dict_and_top_choice(bool value) {
    best_choice_is_dict_and_top_choice_ = value;
  }
  const char *lattice_data() const {
    return lattice_data_;
  }
  int lattice_size() const {
    return lattice_size_; // size of lattice_data in bytes
  }
  void set_lattice_data(const char *data, int size) {
    lattice_size_ = size;
    delete[] lattice_data_;
    lattice_data_ = new char[lattice_size_];
    memcpy(lattice_data_, data, lattice_size_);
  }
#ifndef DISABLED_LEGACY_ENGINE
  const tesseract::ParamsTrainingBundle &params_training_bundle() const {
    return params_training_bundle_;
  }
  // Adds a new ParamsTrainingHypothesis to the current hypothesis list.
  void AddHypothesis(const tesseract::ParamsTrainingHypothesis &hypo) {
    params_training_bundle_.AddHypothesis(hypo);
  }
#endif // ndef DISABLED_LEGACY_ENGINE

  // Functions to setup the blamer.
  // Whole word string, whole word bounding box.
  void SetWordTruth(const UNICHARSET &unicharset, const char *truth_str, const TBOX &word_box);
  // Single "character" string, "character" bounding box.
  // May be called multiple times to indicate the characters in a word.
  void SetSymbolTruth(const UNICHARSET &unicharset, const char *char_str, const TBOX &char_box);
  // Marks that there is something wrong with the truth text, like it contains
  // reject characters.
  void SetRejectedTruth();

  // Returns true if the provided word_choice is correct.
  bool ChoiceIsCorrect(const WERD_CHOICE *word_choice) const;

  void ClearResults() {
    norm_truth_word_.DeleteAllBoxes();
    norm_box_tolerance_ = 0;
    if (!NoTruth()) {
      incorrect_result_reason_ = IRR_CORRECT;
    }
    debug_ = "";
    segsearch_is_looking_for_blame_ = false;
    best_correctly_segmented_rating_ = WERD_CHOICE::kBadRating;
    correct_segmentation_cols_.clear();
    correct_segmentation_rows_.clear();
    best_choice_is_dict_and_top_choice_ = false;
    delete[] lattice_data_;
    lattice_data_ = nullptr;
    lattice_size_ = 0;
  }
  void CopyTruth(const BlamerBundle &other) {
    truth_has_char_boxes_ = other.truth_has_char_boxes_;
    truth_word_ = other.truth_word_;
    truth_text_ = other.truth_text_;
    incorrect_result_reason_ = (other.NoTruth() ? other.incorrect_result_reason_ : IRR_CORRECT);
  }
  void CopyResults(const BlamerBundle &other) {
    norm_truth_word_ = other.norm_truth_word_;
    norm_box_tolerance_ = other.norm_box_tolerance_;
    incorrect_result_reason_ = other.incorrect_result_reason_;
    segsearch_is_looking_for_blame_ = other.segsearch_is_looking_for_blame_;
    best_correctly_segmented_rating_ = other.best_correctly_segmented_rating_;
    correct_segmentation_cols_ = other.correct_segmentation_cols_;
    correct_segmentation_rows_ = other.correct_segmentation_rows_;
    best_choice_is_dict_and_top_choice_ = other.best_choice_is_dict_and_top_choice_;
    if (other.lattice_data_ != nullptr) {
      lattice_data_ = new char[other.lattice_size_];
      memcpy(lattice_data_, other.lattice_data_, other.lattice_size_);
      lattice_size_ = other.lattice_size_;
    } else {
      lattice_data_ = nullptr;
    }
  }
  const char *IncorrectReason() const;

  // Appends choice and truth details to the given debug string.
  void FillDebugString(const std::string &msg, const WERD_CHOICE *choice, std::string &debug);

  // Sets up the norm_truth_word from truth_word using the given DENORM.
  void SetupNormTruthWord(const DENORM &denorm);

  // Splits *this into two pieces in bundle1 and bundle2 (preallocated, empty
  // bundles) where the right edge/ of the left-hand word is word1_right,
  // and the left edge of the right-hand word is word2_left.
  void SplitBundle(int word1_right, int word2_left, bool debug, BlamerBundle *bundle1,
                   BlamerBundle *bundle2) const;
  // "Joins" the blames from bundle1 and bundle2 into *this.
  void JoinBlames(const BlamerBundle &bundle1, const BlamerBundle &bundle2, bool debug);

  // If a blob with the same bounding box as one of the truth character
  // bounding boxes is not classified as the corresponding truth character
  // blames character classifier for incorrect answer.
  void BlameClassifier(const UNICHARSET &unicharset, const TBOX &blob_box,
                       const BLOB_CHOICE_LIST &choices, bool debug);

  // Checks whether chops were made at all the character bounding box
  // boundaries in word->truth_word. If not - blames the chopper for an
  // incorrect answer.
  void SetChopperBlame(const WERD_RES *word, bool debug);
  // Blames the classifier or the language model if, after running only the
  // chopper, best_choice is incorrect and no blame has been yet set.
  // Blames the classifier if best_choice is classifier's top choice and is a
  // dictionary word (i.e. language model could not have helped).
  // Otherwise, blames the language model (formerly permuter word adjustment).
  void BlameClassifierOrLangModel(const WERD_RES *word, const UNICHARSET &unicharset,
                                  bool valid_permuter, bool debug);
  // Sets up the correct_segmentation_* to mark the correct bounding boxes.
  void SetupCorrectSegmentation(const TWERD *word, bool debug);

  // Returns true if a guided segmentation search is needed.
  bool GuidedSegsearchNeeded(const WERD_CHOICE *best_choice) const;
  // Setup ready to guide the segmentation search to the correct segmentation.
  void InitForSegSearch(const WERD_CHOICE *best_choice, MATRIX *ratings, UNICHAR_ID wildcard_id,
                        bool debug, std::string &debug_str, tesseract::LMPainPoints *pain_points,
                        double max_char_wh_ratio, WERD_RES *word_res);
  // Returns true if the guided segsearch is in progress.
  bool GuidedSegsearchStillGoing() const;
  // The segmentation search has ended. Sets the blame appropriately.
  void FinishSegSearch(const WERD_CHOICE *best_choice, bool debug, std::string &debug_str);

  // If the bundle is null or still does not indicate the correct result,
  // fix it and use some backup reason for the blame.
  static void LastChanceBlame(bool debug, WERD_RES *word);

  // Sets the misadaption debug if this word is incorrect, as this word is
  // being adapted to.
  void SetMisAdaptionDebug(const WERD_CHOICE *best_choice, bool debug);

private:
  // Copy assignment operator (currently unused, therefore private).
  BlamerBundle &operator=(const BlamerBundle &other) = delete;
  void SetBlame(IncorrectResultReason irr, const std::string &msg, const WERD_CHOICE *choice,
                bool debug) {
    incorrect_result_reason_ = irr;
    debug_ = IncorrectReason();
    debug_ += " to blame: ";
    FillDebugString(msg, choice, debug_);
    if (debug) {
      tprintf("SetBlame(): %s", debug_.c_str());
    }
  }

private:
  // Set to true when bounding boxes for individual unichars are recorded.
  bool truth_has_char_boxes_;
  // Variables used by the segmentation search when looking for the blame.
  // Set to true while segmentation search is continued after the usual
  // termination condition in order to look for the blame.
  bool segsearch_is_looking_for_blame_;
  // Set to true if best choice is a dictionary word and
  // classifier's top choice.
  bool best_choice_is_dict_and_top_choice_;
  // Tolerance for bounding box comparisons in normalized space.
  int norm_box_tolerance_;
  // The true_word (in the original image coordinate space) contains ground
  // truth bounding boxes for this WERD_RES.
  tesseract::BoxWord truth_word_;
  // Same as above, but in normalized coordinates
  // (filled in by WERD_RES::SetupForRecognition()).
  tesseract::BoxWord norm_truth_word_;
  // Contains ground truth unichar for each of the bounding boxes in truth_word.
  std::vector<std::string> truth_text_;
  // The reason for incorrect OCR result.
  IncorrectResultReason incorrect_result_reason_;
  // Debug text associated with the blame.
  std::string debug_;
  // Misadaption debug information (filled in if this word was misadapted to).
  std::string misadaption_debug_;
  // Vectors populated by SegSearch to indicate column and row indices that
  // correspond to blobs with correct bounding boxes.
  std::vector<int> correct_segmentation_cols_;
  std::vector<int> correct_segmentation_rows_;
  // Best rating for correctly segmented path
  // (set and used by SegSearch when looking for blame).
  float best_correctly_segmented_rating_;
  int lattice_size_; // size of lattice_data in bytes
  // Serialized segmentation search lattice.
  char *lattice_data_;
  // Information about hypotheses (paths) explored by the segmentation search.
#ifndef DISABLED_LEGACY_ENGINE
  tesseract::ParamsTrainingBundle params_training_bundle_;
#endif // ndef DISABLED_LEGACY_ENGINE
};

} // namespace tesseract

#endif // TESSERACT_CCSTRUCT_BLAMER_H_
