/**********************************************************************
 * File:        pageres.cpp  (Formerly page_res.c)
 * Description: Hierarchy of results classes from PAGE_RES to WERD_RES
 *              and an iterator class to iterate over the words.
 * Main purposes:
 *              Easy way to iterate over the words without a 3-nested loop.
 *              Holds data used during word recognition.
 *              Holds information about alternative spacing paths.
 * Author:      Phil Cheatle
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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
 **********************************************************************/

#include "pageres.h"

#include "blamer.h"   // for BlamerBundle
#include "blob_bounds_calculator.h" // for BoxBoundariesCalculator
#include "blobs.h"    // for TWERD, TBLOB
#include "boxword.h"  // for BoxWord
#include "errcode.h"  // for ASSERT_HOST
#include "ocrblock.h" // for BLOCK_IT, BLOCK, BLOCK_LIST (ptr only)
#include "ocrrow.h"   // for ROW, ROW_IT
#include "pdblock.h"  // for PDBLK
#include "polyblk.h"  // for POLY_BLOCK
#include "seam.h"     // for SEAM, start_seam_list
#include "stepblob.h" // for C_BLOB_IT, C_BLOB, C_BLOB_LIST
#include "tprintf.h"  // for tprintf

#include <tesseract/publictypes.h> // for OcrEngineMode, OEM_LSTM_ONLY

#include <cassert> // for assert
#include <cstdint> // for INT32_MAX
#include <cstring> // for strlen

struct Pix;

namespace tesseract {

// Gain factor for computing thresholds that determine the ambiguity of a
// word.
static const double kStopperAmbiguityThresholdGain = 8.0;
// Constant offset for computing thresholds that determine the ambiguity of a
// word.
static const double kStopperAmbiguityThresholdOffset = 1.5;
// Max number of broken pieces to associate.
const int kWordrecMaxNumJoinChunks = 4;
// Max ratio of word box height to line size to allow it to be processed as
// a line with other words.
const double kMaxWordSizeRatio = 1.25;
// Max ratio of line box height to line size to allow a new word to be added.
const double kMaxLineSizeRatio = 1.25;
// Max ratio of word gap to line size to allow a new word to be added.
const double kMaxWordGapRatio = 2.0;

// Computes and returns a threshold of certainty difference used to determine
// which words to keep, based on the adjustment factors of the two words.
// TODO(rays) This is horrible. Replace with an enhance params training model.
static double StopperAmbigThreshold(double f1, double f2) {
  return (f2 - f1) * kStopperAmbiguityThresholdGain -
         kStopperAmbiguityThresholdOffset;
}

/*************************************************************************
 * PAGE_RES::PAGE_RES
 *
 * Constructor for page results
 *************************************************************************/
PAGE_RES::PAGE_RES(bool merge_similar_words, BLOCK_LIST *the_block_list,
                   WERD_CHOICE **prev_word_best_choice_ptr) {
  Init();
  BLOCK_IT block_it(the_block_list);
  BLOCK_RES_IT block_res_it(&block_res_list);
  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    block_res_it.add_to_end(
        new BLOCK_RES(merge_similar_words, block_it.data()));
  }
  prev_word_best_choice = prev_word_best_choice_ptr;
}

/*************************************************************************
 * BLOCK_RES::BLOCK_RES
 *
 * Constructor for BLOCK results
 *************************************************************************/

BLOCK_RES::BLOCK_RES(bool merge_similar_words, BLOCK *the_block) {
  ROW_IT row_it(the_block->row_list());
  ROW_RES_IT row_res_it(&row_res_list);

  char_count = 0;
  rej_count = 0;
  font_class = -1; // not assigned
  x_height = -1.0;
  font_assigned = false;
  row_count = 0;

  block = the_block;

  for (row_it.mark_cycle_pt(); !row_it.cycled_list(); row_it.forward()) {
    row_res_it.add_to_end(new ROW_RES(merge_similar_words, row_it.data()));
  }
}

/*************************************************************************
 * ROW_RES::ROW_RES
 *
 * Constructor for ROW results
 *************************************************************************/

ROW_RES::ROW_RES(bool merge_similar_words, ROW *the_row) {
  WERD_IT word_it(the_row->word_list());
  WERD_RES_IT word_res_it(&word_res_list);
  WERD_RES *combo = nullptr; // current combination of fuzzies
  WERD *copy_word;

  char_count = 0;
  rej_count = 0;
  whole_word_rej_count = 0;

  row = the_row;
  bool add_next_word = false;
  TBOX union_box;
  float line_height =
      the_row->x_height() + the_row->ascenders() - the_row->descenders();
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    auto *word_res = new WERD_RES(word_it.data());
    word_res->x_height = the_row->x_height();
    if (add_next_word) {
      ASSERT_HOST(combo != nullptr);
      // We are adding this word to the combination.
      word_res->part_of_combo = true;
      combo->copy_on(word_res);
    } else if (merge_similar_words) {
      union_box = word_res->word->bounding_box();
      add_next_word = !word_res->word->flag(W_REP_CHAR) &&
                      union_box.height() <= line_height * kMaxWordSizeRatio;
      word_res->odd_size = !add_next_word;
    }
    WERD *next_word = word_it.data_relative(1);
    if (merge_similar_words) {
      if (add_next_word && !next_word->flag(W_REP_CHAR)) {
        // Next word will be added on if all of the following are true:
        // Not a rep char.
        // Box height small enough.
        // Union box height small enough.
        // Horizontal gap small enough.
        TBOX next_box = next_word->bounding_box();
        int prev_right = union_box.right();
        union_box += next_box;
        if (next_box.height() > line_height * kMaxWordSizeRatio ||
            union_box.height() > line_height * kMaxLineSizeRatio ||
            next_box.left() > prev_right + line_height * kMaxWordGapRatio) {
          add_next_word = false;
        }
      }
      next_word->set_flag(W_FUZZY_NON, add_next_word);
    } else {
      add_next_word = next_word->flag(W_FUZZY_NON);
    }
    if (add_next_word) {
      if (combo == nullptr) {
        copy_word = new WERD;
        *copy_word = *(word_it.data()); // deep copy
        combo = new WERD_RES(copy_word);
        combo->x_height = the_row->x_height();
        combo->combination = true;
        word_res_it.add_to_end(combo);
      }
      word_res->part_of_combo = true;
    } else {
      combo = nullptr;
    }
    word_res_it.add_to_end(word_res);
  }
}

WERD_RES &WERD_RES::operator=(const WERD_RES &source) {
  this->ELIST<WERD_RES>::LINK::operator=(source);
  Clear();
  if (source.combination) {
    word = new WERD;
    *word = *(source.word); // deep copy
  } else {
    word = source.word; // pt to same word
  }
  if (source.bln_boxes != nullptr) {
    bln_boxes = new tesseract::BoxWord(*source.bln_boxes);
  }
  if (source.chopped_word != nullptr) {
    chopped_word = new TWERD(*source.chopped_word);
  }
  if (source.rebuild_word != nullptr) {
    rebuild_word = new TWERD(*source.rebuild_word);
  }
  // TODO(rays) Do we ever need to copy the seam_array?
  blob_row = source.blob_row;
  denorm = source.denorm;
  if (source.box_word != nullptr) {
    box_word = new tesseract::BoxWord(*source.box_word);
  }
  best_state = source.best_state;
  correct_text = source.correct_text;
  blob_widths = source.blob_widths;
  blob_gaps = source.blob_gaps;
  // None of the uses of operator= require the ratings matrix to be copied,
  // so don't as it would be really slow.

  // Copy the cooked choices.
  WERD_CHOICE_IT wc_it(const_cast<WERD_CHOICE_LIST *>(&source.best_choices));
  WERD_CHOICE_IT wc_dest_it(&best_choices);
  for (wc_it.mark_cycle_pt(); !wc_it.cycled_list(); wc_it.forward()) {
    const WERD_CHOICE *choice = wc_it.data();
    wc_dest_it.add_after_then_move(new WERD_CHOICE(*choice));
  }
  if (!wc_dest_it.empty()) {
    wc_dest_it.move_to_first();
    best_choice = wc_dest_it.data();
  } else {
    best_choice = nullptr;
  }

  if (source.raw_choice != nullptr) {
    raw_choice = new WERD_CHOICE(*source.raw_choice);
  } else {
    raw_choice = nullptr;
  }
  if (source.ep_choice != nullptr) {
    ep_choice = new WERD_CHOICE(*source.ep_choice);
  } else {
    ep_choice = nullptr;
  }
  reject_map = source.reject_map;
  combination = source.combination;
  part_of_combo = source.part_of_combo;
  CopySimpleFields(source);
  if (source.blamer_bundle != nullptr) {
    blamer_bundle = new BlamerBundle(*(source.blamer_bundle));
  }
  return *this;
}

// Copies basic fields that don't involve pointers that might be useful
// to copy when making one WERD_RES from another.
void WERD_RES::CopySimpleFields(const WERD_RES &source) {
  tess_failed = source.tess_failed;
  tess_accepted = source.tess_accepted;
  tess_would_adapt = source.tess_would_adapt;
  done = source.done;
  unlv_crunch_mode = source.unlv_crunch_mode;
  small_caps = source.small_caps;
  odd_size = source.odd_size;
  fontinfo = source.fontinfo;
  fontinfo2 = source.fontinfo2;
  fontinfo_id_count = source.fontinfo_id_count;
  fontinfo_id2_count = source.fontinfo_id2_count;
  x_height = source.x_height;
  caps_height = source.caps_height;
  baseline_shift = source.baseline_shift;
  guessed_x_ht = source.guessed_x_ht;
  guessed_caps_ht = source.guessed_caps_ht;
  reject_spaces = source.reject_spaces;
  uch_set = source.uch_set;
  tesseract = source.tesseract;
}

// Initializes a blank (default constructed) WERD_RES from one that has
// already been recognized.
// Use SetupFor*Recognition afterwards to complete the setup and make
// it ready for a retry recognition.
void WERD_RES::InitForRetryRecognition(const WERD_RES &source) {
  word = source.word;
  CopySimpleFields(source);
  if (source.blamer_bundle != nullptr) {
    blamer_bundle = new BlamerBundle();
    blamer_bundle->CopyTruth(*source.blamer_bundle);
  }
}

// Sets up the members used in recognition: bln_boxes, chopped_word,
// seam_array, denorm.  Returns false if
// the word is empty and sets up fake results.  If use_body_size is
// true and row->body_size is set, then body_size will be used for
// blob normalization instead of xheight + ascrise. This flag is for
// those languages that are using CJK pitch model and thus it has to
// be true if and only if tesseract->textord_use_cjk_fp_model is
// true.
// If allow_detailed_fx is true, the feature extractor will receive fine
// precision outline information, allowing smoother features and better
// features on low resolution images.
// The norm_mode_hint sets the default mode for normalization in absence
// of any of the above flags.
// norm_box is used to override the word bounding box to determine the
// normalization scale and offset.
// Returns false if the word is empty and sets up fake results.
bool WERD_RES::SetupForRecognition(const UNICHARSET &unicharset_in,
                                   tesseract::Tesseract *tess, Image pix,
                                   int norm_mode, const TBOX *norm_box,
                                   bool numeric_mode, bool use_body_size,
                                   bool allow_detailed_fx, ROW *row,
                                   const BLOCK *block) {
  auto norm_mode_hint = static_cast<tesseract::OcrEngineMode>(norm_mode);
  tesseract = tess;
  POLY_BLOCK *pb = block != nullptr ? block->pdblk.poly_block() : nullptr;
  if ((norm_mode_hint != tesseract::OEM_LSTM_ONLY &&
       word->cblob_list()->empty()) ||
      (pb != nullptr && !pb->IsText())) {
    // Empty words occur when all the blobs have been moved to the rej_blobs
    // list, which seems to occur frequently in junk.
    SetupFake(unicharset_in);
    word->set_flag(W_REP_CHAR, false);
    return false;
  }
  ClearResults();
  SetupWordScript(unicharset_in);
  chopped_word = TWERD::PolygonalCopy(allow_detailed_fx, word);
  float word_xheight =
      use_body_size && row != nullptr && row->body_size() > 0.0f
          ? row->body_size()
          : x_height;
  chopped_word->BLNormalize(block, row, pix, word->flag(W_INVERSE),
                            word_xheight, baseline_shift, numeric_mode,
                            norm_mode_hint, norm_box, &denorm);
  blob_row = row;
  SetupBasicsFromChoppedWord(unicharset_in);
  SetupBlamerBundle();
  int num_blobs = chopped_word->NumBlobs();
  ratings = new MATRIX(num_blobs, kWordrecMaxNumJoinChunks);
  tess_failed = false;
  return true;
}

// Set up the seam array, bln_boxes, best_choice, and raw_choice to empty
// accumulators from a made chopped word.  We presume the fields are already
// empty.
void WERD_RES::SetupBasicsFromChoppedWord(const UNICHARSET &unicharset_in) {
  bln_boxes = tesseract::BoxWord::CopyFromNormalized(chopped_word);
  start_seam_list(chopped_word, &seam_array);
  SetupBlobWidthsAndGaps();
  ClearWordChoices();
}

// Sets up the members used in recognition for an empty recognition result:
// bln_boxes, chopped_word, seam_array, denorm, best_choice, raw_choice.
void WERD_RES::SetupFake(const UNICHARSET &unicharset_in) {
  ClearResults();
  SetupWordScript(unicharset_in);
  chopped_word = new TWERD;
  rebuild_word = new TWERD;
  bln_boxes = new tesseract::BoxWord;
  box_word = new tesseract::BoxWord;
  int blob_count = word->cblob_list()->length();
  if (blob_count > 0) {
    auto **fake_choices = new BLOB_CHOICE *[blob_count];
    // For non-text blocks, just pass any blobs through to the box_word
    // and call the word failed with a fake classification.
    C_BLOB_IT b_it(word->cblob_list());
    int blob_id = 0;
    for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
      TBOX box = b_it.data()->bounding_box();
      box_word->InsertBox(box_word->length(), box);
      fake_choices[blob_id++] = new BLOB_CHOICE;
    }
    FakeClassifyWord(blob_count, fake_choices);
    delete[] fake_choices;
  } else {
    auto *word = new WERD_CHOICE(&unicharset_in);
    word->make_bad();
    LogNewRawChoice(word);
    // Ownership of word is taken by *this WERD_RES in LogNewCookedChoice.
    LogNewCookedChoice(1, false, word);
  }
  tess_failed = true;
  done = true;
}

void WERD_RES::SetupWordScript(const UNICHARSET &uch) {
  uch_set = &uch;
  int script = uch.default_sid();
  word->set_script_id(script);
  word->set_flag(W_SCRIPT_HAS_XHEIGHT, uch.script_has_xheight());
  word->set_flag(W_SCRIPT_IS_LATIN, script == uch.latin_sid());
}

// Sets up the blamer_bundle if it is not null, using the initialized denorm.
void WERD_RES::SetupBlamerBundle() {
  if (blamer_bundle != nullptr) {
    blamer_bundle->SetupNormTruthWord(denorm);
  }
}

// Computes the blob_widths and blob_gaps from the chopped_word.
void WERD_RES::SetupBlobWidthsAndGaps() {
  blob_widths.clear();
  blob_gaps.clear();
  int num_blobs = chopped_word->NumBlobs();
  for (int b = 0; b < num_blobs; ++b) {
    TBLOB *blob = chopped_word->blobs[b];
    TBOX box = blob->bounding_box();
    blob_widths.push_back(box.width());
    if (b + 1 < num_blobs) {
      blob_gaps.push_back(chopped_word->blobs[b + 1]->bounding_box().left() -
                          box.right());
    }
  }
}

// Updates internal data to account for a new SEAM (chop) at the given
// blob_number. Fixes the ratings matrix and states in the choices, as well
// as the blob widths and gaps.
void WERD_RES::InsertSeam(int blob_number, SEAM *seam) {
  // Insert the seam into the SEAMS array.
  seam->PrepareToInsertSeam(seam_array, chopped_word->blobs, blob_number, true);
  seam_array.insert(seam_array.begin() + blob_number, seam);
  if (ratings != nullptr) {
    // Expand the ratings matrix.
    ratings = ratings->ConsumeAndMakeBigger(blob_number);
    // Fix all the segmentation states.
    if (raw_choice != nullptr) {
      raw_choice->UpdateStateForSplit(blob_number);
    }
    WERD_CHOICE_IT wc_it(&best_choices);
    for (wc_it.mark_cycle_pt(); !wc_it.cycled_list(); wc_it.forward()) {
      WERD_CHOICE *choice = wc_it.data();
      choice->UpdateStateForSplit(blob_number);
    }
    SetupBlobWidthsAndGaps();
  }
}

// Returns true if all the word choices except the first have adjust_factors
// worse than the given threshold.
bool WERD_RES::AlternativeChoiceAdjustmentsWorseThan(float threshold) const {
  // The choices are not changed by this iteration.
  WERD_CHOICE_IT wc_it(const_cast<WERD_CHOICE_LIST *>(&best_choices));
  for (wc_it.forward(); !wc_it.at_first(); wc_it.forward()) {
    WERD_CHOICE *choice = wc_it.data();
    if (choice->adjust_factor() <= threshold) {
      return false;
    }
  }
  return true;
}

// Returns true if the current word is ambiguous (by number of answers or
// by dangerous ambigs.)
bool WERD_RES::IsAmbiguous() {
  return !best_choices.singleton() || best_choice->dangerous_ambig_found();
}

// Returns true if the ratings matrix size matches the sum of each of the
// segmentation states.
bool WERD_RES::StatesAllValid() {
  unsigned ratings_dim = ratings->dimension();
  if (raw_choice->TotalOfStates() != ratings_dim) {
    tprintf("raw_choice has total of states = %u vs ratings dim of %u\n",
            raw_choice->TotalOfStates(), ratings_dim);
    return false;
  }
  WERD_CHOICE_IT it(&best_choices);
  unsigned index = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward(), ++index) {
    WERD_CHOICE *choice = it.data();
    if (choice->TotalOfStates() != ratings_dim) {
      tprintf("Cooked #%u has total of states = %u vs ratings dim of %u\n",
              index, choice->TotalOfStates(), ratings_dim);
      return false;
    }
  }
  return true;
}

// Prints a list of words found if debug is true or the word result matches
// the word_to_debug.
void WERD_RES::DebugWordChoices(bool debug, const char *word_to_debug) {
  if (debug || (word_to_debug != nullptr && *word_to_debug != '\0' &&
                best_choice != nullptr &&
                best_choice->unichar_string() == std::string(word_to_debug))) {
    if (raw_choice != nullptr) {
      raw_choice->print("\nBest Raw Choice");
    }

    WERD_CHOICE_IT it(&best_choices);
    int index = 0;
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward(), ++index) {
      WERD_CHOICE *choice = it.data();
      std::string label;
      label += "\nCooked Choice #" + std::to_string(index);
      choice->print(label.c_str());
    }
  }
}

// Prints the top choice along with the accepted/done flags.
void WERD_RES::DebugTopChoice(const char *msg) const {
  tprintf("Best choice: accepted=%d, adaptable=%d, done=%d : ", tess_accepted,
          tess_would_adapt, done);
  if (best_choice == nullptr) {
    tprintf("<Null choice>\n");
  } else {
    best_choice->print(msg);
  }
}

// Removes from best_choices all choices which are not within a reasonable
// range of the best choice.
// TODO(rays) incorporate the information used here into the params training
// re-ranker, in place of this heuristic that is based on the previous
// adjustment factor.
void WERD_RES::FilterWordChoices(int debug_level) {
  if (best_choice == nullptr || best_choices.singleton()) {
    return;
  }

  if (debug_level >= 2) {
    best_choice->print("\nFiltering against best choice");
  }
  WERD_CHOICE_IT it(&best_choices);
  int index = 0;
  for (it.forward(); !it.at_first(); it.forward(), ++index) {
    WERD_CHOICE *choice = it.data();
    float threshold = StopperAmbigThreshold(best_choice->adjust_factor(),
                                            choice->adjust_factor());
    // i, j index the blob choice in choice, best_choice.
    // chunk is an index into the chopped_word blobs (AKA chunks).
    // Since the two words may use different segmentations of the chunks, we
    // iterate over the chunks to find out whether a comparable blob
    // classification is much worse than the best result.
    unsigned i = 0, j = 0, chunk = 0;
    // Each iteration of the while deals with 1 chunk. On entry choice_chunk
    // and best_chunk are the indices of the first chunk in the NEXT blob,
    // i.e. we don't have to increment i, j while chunk < choice_chunk and
    // best_chunk respectively.
    auto choice_chunk = choice->state(0), best_chunk = best_choice->state(0);
    while (i < choice->length() && j < best_choice->length()) {
      if (choice->unichar_id(i) != best_choice->unichar_id(j) &&
          choice->certainty(i) - best_choice->certainty(j) < threshold) {
        if (debug_level >= 2) {
          choice->print("WorstCertaintyDiffWorseThan");
          tprintf(
              "i %u j %u Choice->Blob[i].Certainty %.4g"
              " WorstOtherChoiceCertainty %g Threshold %g\n",
              i, j, choice->certainty(i), best_choice->certainty(j), threshold);
          tprintf("Discarding bad choice #%d\n", index);
        }
        delete it.extract();
        break;
      }
      ++chunk;
      // If needed, advance choice_chunk to keep up with chunk.
      while (choice_chunk < chunk && ++i < choice->length()) {
        choice_chunk += choice->state(i);
      }
      // If needed, advance best_chunk to keep up with chunk.
      while (best_chunk < chunk && ++j < best_choice->length()) {
        best_chunk += best_choice->state(j);
      }
    }
  }
}

void WERD_RES::ComputeAdaptionThresholds(float certainty_scale,
                                         float min_rating, float max_rating,
                                         float rating_margin,
                                         float *thresholds) {
  int chunk = 0;
  int end_chunk = best_choice->state(0);
  int end_raw_chunk = raw_choice->state(0);
  int raw_blob = 0;
  for (unsigned i = 0; i < best_choice->length(); i++, thresholds++) {
    float avg_rating = 0.0f;
    int num_error_chunks = 0;

    // For each chunk in best choice blob i, count non-matching raw results.
    while (chunk < end_chunk) {
      if (chunk >= end_raw_chunk) {
        ++raw_blob;
        end_raw_chunk += raw_choice->state(raw_blob);
      }
      if (best_choice->unichar_id(i) != raw_choice->unichar_id(raw_blob)) {
        avg_rating += raw_choice->certainty(raw_blob);
        ++num_error_chunks;
      }
      ++chunk;
    }

    if (num_error_chunks > 0) {
      avg_rating /= num_error_chunks;
      *thresholds = (avg_rating / -certainty_scale) * (1.0 - rating_margin);
    } else {
      *thresholds = max_rating;
    }

    if (*thresholds > max_rating) {
      *thresholds = max_rating;
    }
    if (*thresholds < min_rating) {
      *thresholds = min_rating;
    }
  }
}

// Saves a copy of the word_choice if it has the best unadjusted rating.
// Returns true if the word_choice was the new best.
bool WERD_RES::LogNewRawChoice(WERD_CHOICE *word_choice) {
  if (raw_choice == nullptr || word_choice->rating() < raw_choice->rating()) {
    delete raw_choice;
    raw_choice = new WERD_CHOICE(*word_choice);
    raw_choice->set_permuter(TOP_CHOICE_PERM);
    return true;
  }
  return false;
}

// Consumes word_choice by adding it to best_choices, (taking ownership) if
// the certainty for word_choice is some distance of the best choice in
// best_choices, or by deleting the word_choice and returning false.
// The best_choices list is kept in sorted order by rating. Duplicates are
// removed, and the list is kept no longer than max_num_choices in length.
// Returns true if the word_choice is still a valid pointer.
bool WERD_RES::LogNewCookedChoice(int max_num_choices, bool debug,
                                  WERD_CHOICE *word_choice) {
  if (best_choice != nullptr) {
    // Throw out obviously bad choices to save some work.
    // TODO(rays) Get rid of this! This piece of code produces different
    // results according to the order in which words are found, which is an
    // undesirable behavior. It would be better to keep all the choices and
    // prune them later when more information is available.
    float max_certainty_delta = StopperAmbigThreshold(
        best_choice->adjust_factor(), word_choice->adjust_factor());
    if (max_certainty_delta > -kStopperAmbiguityThresholdOffset) {
      max_certainty_delta = -kStopperAmbiguityThresholdOffset;
    }
    if (word_choice->certainty() - best_choice->certainty() <
        max_certainty_delta) {
      if (debug) {
        std::string bad_string;
        word_choice->string_and_lengths(&bad_string, nullptr);
        tprintf(
            "Discarding choice \"%s\" with an overly low certainty"
            " %.3f vs best choice certainty %.3f (Threshold: %.3f)\n",
            bad_string.c_str(), word_choice->certainty(),
            best_choice->certainty(),
            max_certainty_delta + best_choice->certainty());
      }
      delete word_choice;
      return false;
    }
  }

  // Insert in the list in order of increasing rating, but knock out worse
  // string duplicates.
  WERD_CHOICE_IT it(&best_choices);
  const std::string &new_str = word_choice->unichar_string();
  bool inserted = false;
  int num_choices = 0;
  if (!it.empty()) {
    do {
      WERD_CHOICE *choice = it.data();
      if (choice->rating() > word_choice->rating() && !inserted) {
        // Time to insert.
        it.add_before_stay_put(word_choice);
        inserted = true;
        if (num_choices == 0) {
          best_choice = word_choice; // This is the new best.
        }
        ++num_choices;
      }
      if (choice->unichar_string() == new_str) {
        if (inserted) {
          // New is better.
          delete it.extract();
        } else {
          // Old is better.
          if (debug) {
            tprintf("Discarding duplicate choice \"%s\", rating %g vs %g\n",
                    new_str.c_str(), word_choice->rating(), choice->rating());
          }
          delete word_choice;
          return false;
        }
      } else {
        ++num_choices;
        if (num_choices > max_num_choices) {
          delete it.extract();
        }
      }
      it.forward();
    } while (!it.at_first());
  }
  if (!inserted && num_choices < max_num_choices) {
    it.add_to_end(word_choice);
    inserted = true;
    if (num_choices == 0) {
      best_choice = word_choice; // This is the new best.
    }
  }
  if (debug) {
    if (inserted) {
      tprintf("New %s", best_choice == word_choice ? "Best" : "Secondary");
    } else {
      tprintf("Poor");
    }
    word_choice->print(" Word Choice");
  }
  if (!inserted) {
    delete word_choice;
    return false;
  }
  return true;
}

// Simple helper moves the ownership of the pointer data from src to dest,
// first deleting anything in dest, and nulling out src afterwards.
template <class T>
static void MovePointerData(T **dest, T **src) {
  delete *dest;
  *dest = *src;
  *src = nullptr;
}

// Prints a brief list of all the best choices.
void WERD_RES::PrintBestChoices() const {
  std::string alternates_str;
  WERD_CHOICE_IT it(const_cast<WERD_CHOICE_LIST *>(&best_choices));
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    if (!it.at_first()) {
      alternates_str += "\", \"";
    }
    alternates_str += it.data()->unichar_string();
  }
  tprintf("Alternates for \"%s\": {\"%s\"}\n",
          best_choice->unichar_string().c_str(), alternates_str.c_str());
}

// Returns the sum of the widths of the blob between start_blob and last_blob
// inclusive.
int WERD_RES::GetBlobsWidth(int start_blob, int last_blob) const {
  int result = 0;
  for (int b = start_blob; b <= last_blob; ++b) {
    result += blob_widths[b];
    if (b < last_blob) {
      result += blob_gaps[b];
    }
  }
  return result;
}
// Returns the width of a gap between the specified blob and the next one.
int WERD_RES::GetBlobsGap(unsigned blob_index) const {
  if (blob_index >= blob_gaps.size()) {
    return 0;
  }
  return blob_gaps[blob_index];
}

// Returns the BLOB_CHOICE corresponding to the given index in the
// best choice word taken from the appropriate cell in the ratings MATRIX.
// Borrowed pointer, so do not delete. May return nullptr if there is no
// BLOB_CHOICE matching the unichar_id at the given index.
BLOB_CHOICE *WERD_RES::GetBlobChoice(unsigned index) const {
  if (index >= best_choice->length()) {
    return nullptr;
  }
  BLOB_CHOICE_LIST *choices = GetBlobChoices(index);
  return FindMatchingChoice(best_choice->unichar_id(index), choices);
}

// Returns the BLOB_CHOICE_LIST corresponding to the given index in the
// best choice word taken from the appropriate cell in the ratings MATRIX.
// Borrowed pointer, so do not delete.
BLOB_CHOICE_LIST *WERD_RES::GetBlobChoices(int index) const {
  return best_choice->blob_choices(index, ratings);
}

// Moves the results fields from word to this. This takes ownership of all
// the data, so src can be destructed.
void WERD_RES::ConsumeWordResults(WERD_RES *word) {
  denorm = word->denorm;
  blob_row = word->blob_row;
  MovePointerData(&chopped_word, &word->chopped_word);
  MovePointerData(&rebuild_word, &word->rebuild_word);
  MovePointerData(&box_word, &word->box_word);
  for (auto data : seam_array) {
    delete data;
  }
  seam_array = word->seam_array;
  word->seam_array.clear();
  // TODO: optimize moves.
  best_state = word->best_state;
  word->best_state.clear();
  correct_text = word->correct_text;
  word->correct_text.clear();
  blob_widths = word->blob_widths;
  word->blob_widths.clear();
  blob_gaps = word->blob_gaps;
  word->blob_gaps.clear();
  if (ratings != nullptr) {
    ratings->delete_matrix_pointers();
  }
  MovePointerData(&ratings, &word->ratings);
  best_choice = word->best_choice;
  MovePointerData(&raw_choice, &word->raw_choice);
  best_choices.clear();
  WERD_CHOICE_IT wc_it(&best_choices);
  wc_it.add_list_after(&word->best_choices);
  reject_map = word->reject_map;
  if (word->blamer_bundle != nullptr) {
    assert(blamer_bundle != nullptr);
    blamer_bundle->CopyResults(*(word->blamer_bundle));
  }
  CopySimpleFields(*word);
}

// Replace the best choice and rebuild box word.
// choice must be from the current best_choices list.
void WERD_RES::ReplaceBestChoice(WERD_CHOICE *choice) {
  best_choice = choice;
  RebuildBestState();
  SetupBoxWord();
  // Make up a fake reject map of the right length to keep the
  // rejection pass happy.
  reject_map.initialise(best_state.size());
  done = tess_accepted = tess_would_adapt = true;
  SetScriptPositions();
}

// Builds the rebuild_word and sets the best_state from the chopped_word and
// the best_choice->state.
void WERD_RES::RebuildBestState() {
  ASSERT_HOST(best_choice != nullptr);
  delete rebuild_word;
  rebuild_word = new TWERD;
  if (seam_array.empty()) {
    start_seam_list(chopped_word, &seam_array);
  }
  best_state.clear();
  int start = 0;
  for (unsigned i = 0; i < best_choice->length(); ++i) {
    int length = best_choice->state(i);
    best_state.push_back(length);
    if (length > 1) {
      SEAM::JoinPieces(seam_array, chopped_word->blobs, start,
                       start + length - 1);
    }
    TBLOB *blob = chopped_word->blobs[start];
    rebuild_word->blobs.push_back(new TBLOB(*blob));
    if (length > 1) {
      SEAM::BreakPieces(seam_array, chopped_word->blobs, start,
                        start + length - 1);
    }
    start += length;
  }
}

// Copies the chopped_word to the rebuild_word, faking a best_state as well.
// Also sets up the output box_word.
void WERD_RES::CloneChoppedToRebuild() {
  delete rebuild_word;
  rebuild_word = new TWERD(*chopped_word);
  SetupBoxWord();
  auto word_len = box_word->length();
  best_state.reserve(word_len);
  correct_text.reserve(word_len);
  for (unsigned i = 0; i < word_len; ++i) {
    best_state.push_back(1);
    correct_text.emplace_back("");
  }
}

// Sets/replaces the box_word with one made from the rebuild_word.
void WERD_RES::SetupBoxWord() {
  delete box_word;
  rebuild_word->ComputeBoundingBoxes();
  box_word = tesseract::BoxWord::CopyFromNormalized(rebuild_word);
  box_word->ClipToOriginalWord(denorm.block(), word);
}

// Sets up the script positions in the output best_choice using the best_choice
// to get the unichars, and the unicharset to get the target positions.
void WERD_RES::SetScriptPositions() {
  best_choice->SetScriptPositions(small_caps, chopped_word);
}
// Sets all the blobs in all the words (raw choice and best choices) to be
// the given position. (When a sub/superscript is recognized as a separate
// word, it falls victim to the rule that a whole word cannot be sub or
// superscript, so this function overrides that problem.)
void WERD_RES::SetAllScriptPositions(tesseract::ScriptPos position) {
  raw_choice->SetAllScriptPositions(position);
  WERD_CHOICE_IT wc_it(&best_choices);
  for (wc_it.mark_cycle_pt(); !wc_it.cycled_list(); wc_it.forward()) {
    wc_it.data()->SetAllScriptPositions(position);
  }
}

// Classifies the word with some already-calculated BLOB_CHOICEs.
// The choices are an array of blob_count pointers to BLOB_CHOICE,
// providing a single classifier result for each blob.
// The BLOB_CHOICEs are consumed and the word takes ownership.
// The number of blobs in the box_word must match blob_count.
void WERD_RES::FakeClassifyWord(unsigned blob_count, BLOB_CHOICE **choices) {
  // Setup the WERD_RES.
  ASSERT_HOST(box_word != nullptr);
  ASSERT_HOST(blob_count == box_word->length());
  ClearWordChoices();
  ClearRatings();
  ratings = new MATRIX(blob_count, 1);
  for (unsigned c = 0; c < blob_count; ++c) {
    auto *choice_list = new BLOB_CHOICE_LIST;
    BLOB_CHOICE_IT choice_it(choice_list);
    choice_it.add_after_then_move(choices[c]);
    ratings->put(c, c, choice_list);
  }
  FakeWordFromRatings(TOP_CHOICE_PERM);
  reject_map.initialise(blob_count);
  best_state.clear();
  best_state.resize(blob_count, 1);
  done = true;
}

// Creates a WERD_CHOICE for the word using the top choices from the leading
// diagonal of the ratings matrix.
void WERD_RES::FakeWordFromRatings(PermuterType permuter) {
  int num_blobs = ratings->dimension();
  auto *word_choice = new WERD_CHOICE(uch_set, num_blobs);
  word_choice->set_permuter(permuter);
  for (int b = 0; b < num_blobs; ++b) {
    UNICHAR_ID unichar_id = UNICHAR_SPACE;
    // Initialize rating and certainty like in WERD_CHOICE::make_bad().
    float rating = WERD_CHOICE::kBadRating;
    float certainty = -FLT_MAX;
    BLOB_CHOICE_LIST *choices = ratings->get(b, b);
    if (choices != nullptr && !choices->empty()) {
      BLOB_CHOICE_IT bc_it(choices);
      BLOB_CHOICE *choice = bc_it.data();
      unichar_id = choice->unichar_id();
      rating = choice->rating();
      certainty = choice->certainty();
    }
    word_choice->append_unichar_id_space_allocated(unichar_id, 1, rating,
                                                   certainty);
  }
  LogNewRawChoice(word_choice);
  // Ownership of word_choice taken by word here.
  LogNewCookedChoice(1, false, word_choice);
}

// Copies the best_choice strings to the correct_text for adaption/training.
void WERD_RES::BestChoiceToCorrectText() {
  correct_text.clear();
  ASSERT_HOST(best_choice != nullptr);
  for (unsigned i = 0; i < best_choice->length(); ++i) {
    UNICHAR_ID choice_id = best_choice->unichar_id(i);
    const char *blob_choice = uch_set->id_to_unichar(choice_id);
    correct_text.emplace_back(blob_choice);
  }
}

// Merges 2 adjacent blobs in the result if the permanent callback
// class_cb returns other than INVALID_UNICHAR_ID, AND the permanent
// callback box_cb is nullptr or returns true, setting the merged blob
// result to the class returned from class_cb.
// Returns true if anything was merged.
bool WERD_RES::ConditionalBlobMerge(
    const std::function<UNICHAR_ID(UNICHAR_ID, UNICHAR_ID)> &class_cb,
    const std::function<bool(const TBOX &, const TBOX &)> &box_cb) {
  ASSERT_HOST(best_choice->empty() || ratings != nullptr);
  bool modified = false;
  for (unsigned i = 0; i + 1 < best_choice->length(); ++i) {
    UNICHAR_ID new_id =
        class_cb(best_choice->unichar_id(i), best_choice->unichar_id(i + 1));
    if (new_id != INVALID_UNICHAR_ID &&
        (box_cb == nullptr ||
         box_cb(box_word->BlobBox(i), box_word->BlobBox(i + 1)))) {
      // Raw choice should not be fixed.
      best_choice->set_unichar_id(new_id, i);
      modified = true;
      MergeAdjacentBlobs(i);
      const MATRIX_COORD &coord = best_choice->MatrixCoord(i);
      if (!coord.Valid(*ratings)) {
        ratings->IncreaseBandSize(coord.row + 1 - coord.col);
      }
      BLOB_CHOICE_LIST *blob_choices = GetBlobChoices(i);
      if (FindMatchingChoice(new_id, blob_choices) == nullptr) {
        // Insert a fake result.
        auto *blob_choice = new BLOB_CHOICE;
        blob_choice->set_unichar_id(new_id);
        BLOB_CHOICE_IT bc_it(blob_choices);
        bc_it.add_before_then_move(blob_choice);
      }
    }
  }
  return modified;
}

// Merges 2 adjacent blobs in the result (index and index+1) and corrects
// all the data to account for the change.
void WERD_RES::MergeAdjacentBlobs(unsigned index) {
  if (reject_map.length() == best_choice->length()) {
    reject_map.remove_pos(index);
  }
  best_choice->remove_unichar_id(index + 1);
  rebuild_word->MergeBlobs(index, index + 2);
  box_word->MergeBoxes(index, index + 2);
  if (index + 1 < best_state.size()) {
    best_state[index] += best_state[index + 1];
    best_state.erase(best_state.begin() + index + 1);
  }
}

// TODO(tkielbus) Decide between keeping this behavior here or modifying the
// training data.

// Utility function for fix_quotes
// Return true if the next character in the string (given the UTF8 length in
// bytes) is a quote character.
static int is_simple_quote(const char *signed_str, int length) {
  const auto *str = reinterpret_cast<const unsigned char *>(signed_str);
  // Standard 1 byte quotes.
  return (length == 1 && (*str == '\'' || *str == '`')) ||
         // UTF-8 3 bytes curved quotes.
         (length == 3 &&
          ((*str == 0xe2 && *(str + 1) == 0x80 && *(str + 2) == 0x98) ||
           (*str == 0xe2 && *(str + 1) == 0x80 && *(str + 2) == 0x99)));
}

// Callback helper for fix_quotes returns a double quote if both
// arguments are quote, otherwise INVALID_UNICHAR_ID.
UNICHAR_ID WERD_RES::BothQuotes(UNICHAR_ID id1, UNICHAR_ID id2) {
  const char *ch = uch_set->id_to_unichar(id1);
  const char *next_ch = uch_set->id_to_unichar(id2);
  if (is_simple_quote(ch, strlen(ch)) &&
      is_simple_quote(next_ch, strlen(next_ch))) {
    return uch_set->unichar_to_id("\"");
  }
  return INVALID_UNICHAR_ID;
}

// Change pairs of quotes to double quotes.
void WERD_RES::fix_quotes() {
  if (!uch_set->contains_unichar("\"") ||
      !uch_set->get_enabled(uch_set->unichar_to_id("\""))) {
    return; // Don't create it if it is disallowed.
  }

  using namespace std::placeholders; // for _1, _2
  ConditionalBlobMerge(std::bind(&WERD_RES::BothQuotes, this, _1, _2), nullptr);
}

// Callback helper for fix_hyphens returns UNICHAR_ID of - if both
// arguments are hyphen, otherwise INVALID_UNICHAR_ID.
UNICHAR_ID WERD_RES::BothHyphens(UNICHAR_ID id1, UNICHAR_ID id2) {
  const char *ch = uch_set->id_to_unichar(id1);
  const char *next_ch = uch_set->id_to_unichar(id2);
  if (strlen(ch) == 1 && strlen(next_ch) == 1 && (*ch == '-' || *ch == '~') &&
      (*next_ch == '-' || *next_ch == '~')) {
    return uch_set->unichar_to_id("-");
  }
  return INVALID_UNICHAR_ID;
}

// Callback helper for fix_hyphens returns true if box1 and box2 overlap
// (assuming both on the same textline, are in order and a chopped em dash.)
bool WERD_RES::HyphenBoxesOverlap(const TBOX &box1, const TBOX &box2) {
  return box1.right() >= box2.left();
}

// Change pairs of hyphens to a single hyphen if the bounding boxes touch
// Typically a long dash which has been segmented.
void WERD_RES::fix_hyphens() {
  if (!uch_set->contains_unichar("-") ||
      !uch_set->get_enabled(uch_set->unichar_to_id("-"))) {
    return; // Don't create it if it is disallowed.
  }

  using namespace std::placeholders; // for _1, _2
  ConditionalBlobMerge(std::bind(&WERD_RES::BothHyphens, this, _1, _2),
                       std::bind(&WERD_RES::HyphenBoxesOverlap, this, _1, _2));
}

// Callback helper for merge_tess_fails returns a space if both
// arguments are space, otherwise INVALID_UNICHAR_ID.
UNICHAR_ID WERD_RES::BothSpaces(UNICHAR_ID id1, UNICHAR_ID id2) {
  if (id1 == id2 && id1 == uch_set->unichar_to_id(" ")) {
    return id1;
  } else {
    return INVALID_UNICHAR_ID;
  }
}

// Change pairs of tess failures to a single one
void WERD_RES::merge_tess_fails() {
  using namespace std::placeholders; // for _1, _2
  if (ConditionalBlobMerge(std::bind(&WERD_RES::BothSpaces, this, _1, _2),
                           nullptr)) {
    unsigned len = best_choice->length();
    ASSERT_HOST(reject_map.length() == len);
    ASSERT_HOST(box_word->length() == len);
  }
}

// Returns true if the collection of count pieces, starting at start, are all
// natural connected components, ie there are no real chops involved.
bool WERD_RES::PiecesAllNatural(int start, int count) const {
  // all seams must have no splits.
  for (int index = start; index < start + count - 1; ++index) {
    if (index >= 0 && static_cast<size_t>(index) < seam_array.size()) {
      SEAM *seam = seam_array[index];
      if (seam != nullptr && seam->HasAnySplits()) {
        return false;
      }
    }
  }
  return true;
}

WERD_RES::~WERD_RES() {
  Clear();
}

void WERD_RES::Clear() {
  if (combination) {
    delete word;
  }
  word = nullptr;
  delete blamer_bundle;
  blamer_bundle = nullptr;
  ClearResults();
}

void WERD_RES::ClearResults() {
  done = false;
  fontinfo = nullptr;
  fontinfo2 = nullptr;
  fontinfo_id_count = 0;
  fontinfo_id2_count = 0;
  delete bln_boxes;
  bln_boxes = nullptr;
  blob_row = nullptr;
  delete chopped_word;
  chopped_word = nullptr;
  delete rebuild_word;
  rebuild_word = nullptr;
  delete box_word;
  box_word = nullptr;
  best_state.clear();
  correct_text.clear();
  for (auto data : seam_array) {
    delete data;
  }
  seam_array.clear();
  blob_widths.clear();
  blob_gaps.clear();
  ClearRatings();
  ClearWordChoices();
  if (blamer_bundle != nullptr) {
    blamer_bundle->ClearResults();
  }
}
void WERD_RES::ClearWordChoices() {
  best_choice = nullptr;
  delete raw_choice;
  raw_choice = nullptr;
  best_choices.clear();
  delete ep_choice;
  ep_choice = nullptr;
}
void WERD_RES::ClearRatings() {
  if (ratings != nullptr) {
    ratings->delete_matrix_pointers();
    delete ratings;
    ratings = nullptr;
  }
}

int PAGE_RES_IT::cmp(const PAGE_RES_IT &other) const {
  ASSERT_HOST(page_res == other.page_res);
  if (other.block_res == nullptr) {
    // other points to the end of the page.
    if (block_res == nullptr) {
      return 0;
    }
    return -1;
  }
  if (block_res == nullptr) {
    return 1; // we point to the end of the page.
  }
  if (block_res == other.block_res) {
    if (other.row_res == nullptr || row_res == nullptr) {
      // this should only happen if we hit an image block.
      return 0;
    }
    if (row_res == other.row_res) {
      // we point to the same block and row.
      ASSERT_HOST(other.word_res != nullptr && word_res != nullptr);
      if (word_res == other.word_res) {
        // we point to the same word!
        return 0;
      }

      WERD_RES_IT word_res_it(&row_res->word_res_list);
      for (word_res_it.mark_cycle_pt(); !word_res_it.cycled_list();
           word_res_it.forward()) {
        if (word_res_it.data() == word_res) {
          return -1;
        } else if (word_res_it.data() == other.word_res) {
          return 1;
        }
      }
      ASSERT_HOST("Error: Incomparable PAGE_RES_ITs" == nullptr);
    }

    // we both point to the same block, but different rows.
    ROW_RES_IT row_res_it(&block_res->row_res_list);
    for (row_res_it.mark_cycle_pt(); !row_res_it.cycled_list();
         row_res_it.forward()) {
      if (row_res_it.data() == row_res) {
        return -1;
      } else if (row_res_it.data() == other.row_res) {
        return 1;
      }
    }
    ASSERT_HOST("Error: Incomparable PAGE_RES_ITs" == nullptr);
  }

  // We point to different blocks.
  BLOCK_RES_IT block_res_it(&page_res->block_res_list);
  for (block_res_it.mark_cycle_pt(); !block_res_it.cycled_list();
       block_res_it.forward()) {
    if (block_res_it.data() == block_res) {
      return -1;
    } else if (block_res_it.data() == other.block_res) {
      return 1;
    }
  }
  // Shouldn't happen...
  ASSERT_HOST("Error: Incomparable PAGE_RES_ITs" == nullptr);
  return 0;
}

// Inserts the new_word as a combination owned by a corresponding WERD_RES
// before the current position. The simple fields of the WERD_RES are copied
// from clone_res and the resulting WERD_RES is returned for further setup
// with best_choice etc.
WERD_RES *PAGE_RES_IT::InsertSimpleCloneWord(const WERD_RES &clone_res,
                                             WERD *new_word) {
  // Make a WERD_RES for the new_word.
  auto *new_res = new WERD_RES(new_word);
  new_res->CopySimpleFields(clone_res);
  new_res->combination = true;
  // Insert into the appropriate place in the ROW_RES.
  WERD_RES_IT wr_it(&row()->word_res_list);
  for (wr_it.mark_cycle_pt(); !wr_it.cycled_list(); wr_it.forward()) {
    WERD_RES *word = wr_it.data();
    if (word == word_res) {
      break;
    }
  }
  ASSERT_HOST(!wr_it.cycled_list());
  wr_it.add_before_then_move(new_res);
  if (wr_it.at_first()) {
    // This is the new first word, so reset the member iterator so it
    // detects the cycled_list state correctly.
    ResetWordIterator();
  }
  return new_res;
}

// Helper computes the bounds of a word by restricting it to existing words
// that significantly overlap.
static TBOX ComputeWordBounds(const tesseract::PointerVector<WERD_RES> &words,
                              int w_index, TBOX prev_box, WERD_RES_IT w_it) {
  constexpr int kSignificantOverlapFraction = 4;
  TBOX clipped_box;
  TBOX current_box = words[w_index]->word->bounding_box();
  TBOX next_box;
  if (static_cast<size_t>(w_index + 1) < words.size() &&
      words[w_index + 1] != nullptr && words[w_index + 1]->word != nullptr) {
    next_box = words[w_index + 1]->word->bounding_box();
  }
  for (w_it.forward(); !w_it.at_first() && w_it.data()->part_of_combo;
       w_it.forward()) {
    if (w_it.data() == nullptr || w_it.data()->word == nullptr) {
      continue;
    }
    TBOX w_box = w_it.data()->word->bounding_box();
    int height_limit = std::min<int>(w_box.height(), w_box.width() / 2);
    int width_limit = w_box.width() / kSignificantOverlapFraction;
    int min_significant_overlap = std::max(height_limit, width_limit);
    int overlap = w_box.intersection(current_box).width();
    int prev_overlap = w_box.intersection(prev_box).width();
    int next_overlap = w_box.intersection(next_box).width();
    if (overlap > min_significant_overlap) {
      if (prev_overlap > min_significant_overlap) {
        // We have no choice but to use the LSTM word edge.
        clipped_box.set_left(current_box.left());
      } else if (next_overlap > min_significant_overlap) {
        // We have no choice but to use the LSTM word edge.
        clipped_box.set_right(current_box.right());
      } else {
        clipped_box += w_box;
      }
    }
  }
  if (clipped_box.height() <= 0) {
    clipped_box.set_top(current_box.top());
    clipped_box.set_bottom(current_box.bottom());
  }
  if (clipped_box.width() <= 0) {
    clipped_box = current_box;
  }
  return clipped_box;
}

// Helper to compute input for BoxBoundariesCalculator
static std::vector<BoxBoundaries> ComputeFakeWordBlobXBounds(
    const PointerVector<WERD_RES> &words) {

  std::vector<BoxBoundaries> result;

  for (size_t w = 0; w < words.size(); ++w) {
    WERD_RES *word_w = words[w];

    C_BLOB_IT blob_it(word_w->word->cblob_list());
    for (int length : word_w->best_state) {
      TBOX blob_box = blob_it.data()->bounding_box();
      blob_it.forward();
      for (int b = 1; b < length; ++b) {
        blob_box += blob_it.data()->bounding_box();
        blob_it.forward();
      }
      result.push_back({blob_box.left(), blob_box.right()});
    }
  }
  return result;
}

// Helper to compute input for BoxBoundariesCalculator
static std::vector<BoxBoundaries> ComputeBlobXBoundsFromTBOX(
    const std::vector<TBOX> &boxes) {
  std::vector<BoxBoundaries> result;
  result.reserve(boxes.size());
  for (const auto& box : boxes) {
    result.push_back({box.left(), box.right()});
  }
  return result;
}

// Helper moves the src_blob to dest. If it isn't contained by clip_box,
// the blob is replaced by a fake that is contained. The helper takes ownership
// of the blob.
static TBOX ClipAndAddBlob(C_BLOB *src_blob, C_BLOB_IT *dest_it,
                           const TBOX &clip_box) {
  TBOX box = src_blob->bounding_box();
  if (!clip_box.contains(box)) {
    int left =
        ClipToRange<int>(box.left(), clip_box.left(), clip_box.right() - 1);
    int right =
        ClipToRange<int>(box.right(), clip_box.left() + 1, clip_box.right());
    int top =
        ClipToRange<int>(box.top(), clip_box.bottom() + 1, clip_box.top());
    int bottom =
        ClipToRange<int>(box.bottom(), clip_box.bottom(), clip_box.top() - 1);
    box = TBOX(left, bottom, right, top);
    delete src_blob;
    src_blob = C_BLOB::FakeBlob(box);
  }
  dest_it->add_after_then_move(src_blob);
  return box;
}

// Helper to clip a box only in X direction
static TBOX ClipBoxX(const TBOX &box, int left, int right) {
  int clip_left = ClipToRange<int>(box.left(), left, right - 1);
  int clip_right = ClipToRange<int>(box.right(), left + 1, right);
  return TBOX(clip_left, box.bottom(), clip_right, box.top());
}

// Replaces the current WERD/WERD_RES with the given words. The given words
// contain fake blobs that indicate the position of the characters. These are
// replaced with real blobs from the current word as much as possible.
void PAGE_RES_IT::ReplaceCurrentWord(
    tesseract::PointerVector<WERD_RES> *words) {
  if (words->empty()) {
    DeleteCurrentWord();
    return;
  }
  WERD_RES *input_word = word();
  // Set the BOL/EOL flags on the words from the input word.
  if (input_word->word->flag(W_BOL)) {
    (*words)[0]->word->set_flag(W_BOL, true);
  } else {
    (*words)[0]->word->set_blanks(input_word->word->space());
  }
  words->back()->word->set_flag(W_EOL, input_word->word->flag(W_EOL));

  // Move the blobs from the input word to the new set of words.
  // If the input word_res is a combination, then the replacements will also be
  // combinations, and will own their own words. If the input word_res is not a
  // combination, then the final replacements will not be either, (although it
  // is allowed for the input words to be combinations) and their words
  // will get put on the row list. This maintains the ownership rules.
  WERD_IT w_it(row()->row->word_list());
  if (!input_word->combination) {
    for (w_it.mark_cycle_pt(); !w_it.cycled_list(); w_it.forward()) {
      WERD *word = w_it.data();
      if (word == input_word->word) {
        break;
      }
    }
    // w_it is now set to the input_word's word.
    ASSERT_HOST(!w_it.cycled_list());
  }
  // Insert into the appropriate place in the ROW_RES.
  WERD_RES_IT wr_it(&row()->word_res_list);
  for (wr_it.mark_cycle_pt(); !wr_it.cycled_list(); wr_it.forward()) {
    WERD_RES *word = wr_it.data();
    if (word == input_word) {
      break;
    }
  }
  ASSERT_HOST(!wr_it.cycled_list());

  std::vector<TBOX> blob_boxes;

  C_BLOB_IT src_b_it(input_word->word->cblob_list());
  src_b_it.sort(&C_BLOB::SortByXMiddle);
  for (src_b_it.mark_cycle_pt(); !src_b_it.cycled_list(); src_b_it.forward()) {
    blob_boxes.push_back(src_b_it.data()->bounding_box());
  }
  src_b_it.move_to_first();

  C_BLOB_IT rej_b_it(input_word->word->rej_cblob_list());
  rej_b_it.sort(&C_BLOB::SortByXMiddle);

  auto fake_blob_bounds = ComputeFakeWordBlobXBounds(*words);
  BoxBoundariesCalculator calculator{ComputeBlobXBoundsFromTBOX(blob_boxes), {}};
  auto char_bounds = calculator.calculate_bounds(fake_blob_bounds);
  size_t char_bounds_i = 0;
  size_t box_bounds_i = 0;
  TBOX last_blob_box;

  TBOX clip_box;
  for (size_t w = 0; w < words->size(); ++w) {
    WERD_RES *word_w = (*words)[w];
    clip_box = ComputeWordBounds(*words, w, clip_box, wr_it_of_current_word);

    // Remove the fake blobs on the current word, but keep safe for back-up if
    // no blob can be found.
    C_BLOB_LIST fake_blobs;
    C_BLOB_IT fake_b_it(&fake_blobs);
    fake_b_it.add_list_after(word_w->word->cblob_list());
    fake_b_it.move_to_first();
    word_w->word->cblob_list()->clear();
    C_BLOB_IT dest_it(word_w->word->cblob_list());
    // Build the box word as we move the blobs.
    auto *box_word = new tesseract::BoxWord;

    for (size_t i = 0; i < word_w->best_state.size(); ++i) {
      const auto& char_bound = char_bounds[char_bounds_i++];

      TBOX blob_box;
      if (char_bound.begin_box_index != char_bound.end_box_index) {
        // The box indices in curr_char_bound will always be increasing, thus
        // we can iterate src_b_it in the same order.
        while (box_bounds_i < char_bound.begin_box_index) {
          box_bounds_i++;
          src_b_it.forward();
        }

        if (box_bounds_i > char_bound.begin_box_index) {
          // The blob was split across multiple characters and has already
          // been extracted for a previous character. We have the bounds
          // of the blob and can create a fake blob out of it.
          TBOX fake_box = ClipBoxX(last_blob_box,
                                   char_bound.begin_x, char_bound.end_x);
          blob_box += ClipAndAddBlob(C_BLOB::FakeBlob(fake_box),
                                     &dest_it, clip_box);
        }

        // Add all blobs that have not yet been assigned to any of the
        // characters.
        while (box_bounds_i < char_bound.end_box_index) {
          auto* src_blob = src_b_it.extract();
          last_blob_box = src_blob->bounding_box();
          TBOX inserted_box = ClipAndAddBlob(src_blob, &dest_it, clip_box);

          box_bounds_i++;
          src_b_it.forward();

          // Note that the blob may be split across multiple characters in
          // which case we want to clip the box to the part that was "assigned"
          // to the character.
          blob_box += ClipBoxX(inserted_box,
                               char_bound.begin_x, char_bound.end_x);
        }
      }

      // It's not clear where rejected blobs should be added because by
      // definition we don't have enough information about them. So we just
      // add them to whatever character follows.
      while (!rej_b_it.empty() &&
             rej_b_it.data()->bounding_box().x_middle() < char_bound.end_x) {
        blob_box += ClipAndAddBlob(rej_b_it.extract(), &dest_it, clip_box);
        rej_b_it.forward();
      }

      if (blob_box.null_box()) {
        // Use the original box as a back-up.
        blob_box = ClipAndAddBlob(fake_b_it.extract(), &dest_it, clip_box);
      }
      box_word->InsertBox(i, blob_box);
      fake_b_it.forward();
    }

    delete word_w->box_word;
    word_w->box_word = box_word;
    if (!input_word->combination) {
      // Insert word_w->word into the ROW. It doesn't own its word, so the
      // ROW needs to own it.
      w_it.add_before_stay_put(word_w->word);
      word_w->combination = false;
    }
    (*words)[w] = nullptr; // We are taking ownership.
    wr_it.add_before_stay_put(word_w);
  }
  // We have taken ownership of the words.
  words->clear();
  // Delete the current word, which has been replaced. We could just call
  // DeleteCurrentWord, but that would iterate both lists again, and we know
  // we are already in the right place.
  if (!input_word->combination) {
    delete w_it.extract();
  }
  delete wr_it.extract();
  ResetWordIterator();
}

// Deletes the current WERD_RES and its underlying WERD.
void PAGE_RES_IT::DeleteCurrentWord() {
  // Check that this word is as we expect. part_of_combos are NEVER iterated
  // by the normal iterator, so we should never be trying to delete them.
  ASSERT_HOST(!word_res->part_of_combo);
  if (!word_res->combination) {
    // Combinations own their own word, so we won't find the word on the
    // row's word_list, but it is legitimate to try to delete them.
    // Delete word from the ROW when not a combination.
    WERD_IT w_it(row()->row->word_list());
    for (w_it.mark_cycle_pt(); !w_it.cycled_list(); w_it.forward()) {
      if (w_it.data() == word_res->word) {
        break;
      }
    }
    ASSERT_HOST(!w_it.cycled_list());
    delete w_it.extract();
  }
  // Remove the WERD_RES for the new_word.
  // Remove the WORD_RES from the ROW_RES.
  WERD_RES_IT wr_it(&row()->word_res_list);
  for (wr_it.mark_cycle_pt(); !wr_it.cycled_list(); wr_it.forward()) {
    if (wr_it.data() == word_res) {
      word_res = nullptr;
      break;
    }
  }
  ASSERT_HOST(!wr_it.cycled_list());
  delete wr_it.extract();
  ResetWordIterator();
}

// Makes the current word a fuzzy space if not already fuzzy. Updates
// corresponding part of combo if required.
void PAGE_RES_IT::MakeCurrentWordFuzzy() {
  WERD *real_word = word_res->word;
  if (!real_word->flag(W_FUZZY_SP) && !real_word->flag(W_FUZZY_NON)) {
    real_word->set_flag(W_FUZZY_SP, true);
    if (word_res->combination) {
      // The next word should be the corresponding part of combo, but we have
      // already stepped past it, so find it by search.
      WERD_RES_IT wr_it(&row()->word_res_list);
      for (wr_it.mark_cycle_pt();
           !wr_it.cycled_list() && wr_it.data() != word_res; wr_it.forward()) {
      }
      wr_it.forward();
      ASSERT_HOST(wr_it.data()->part_of_combo);
      real_word = wr_it.data()->word;
      ASSERT_HOST(!real_word->flag(W_FUZZY_SP) &&
                  !real_word->flag(W_FUZZY_NON));
      real_word->set_flag(W_FUZZY_SP, true);
    }
  }
}

/*************************************************************************
 * PAGE_RES_IT::restart_page
 *
 * Set things up at the start of the page
 *************************************************************************/

WERD_RES *PAGE_RES_IT::start_page(bool empty_ok) {
  block_res_it.set_to_list(&page_res->block_res_list);
  block_res_it.mark_cycle_pt();
  prev_block_res = nullptr;
  prev_row_res = nullptr;
  prev_word_res = nullptr;
  block_res = nullptr;
  row_res = nullptr;
  word_res = nullptr;
  next_block_res = nullptr;
  next_row_res = nullptr;
  next_word_res = nullptr;
  internal_forward(true, empty_ok);
  return internal_forward(false, empty_ok);
}

// Recovers from operations on the current word, such as in InsertCloneWord
// and DeleteCurrentWord.
// Resets the word_res_it so that it is one past the next_word_res, as
// it should be after internal_forward. If next_row_res != row_res,
// then the next_word_res is in the next row, so there is no need to do
// anything to word_res_it, but it is still a good idea to reset the pointers
// word_res and prev_word_res, which are still in the current row.
void PAGE_RES_IT::ResetWordIterator() {
  if (row_res == next_row_res) {
    // Reset the member iterator so it can move forward and detect the
    // cycled_list state correctly.
    word_res_it.move_to_first();
    for (word_res_it.mark_cycle_pt();
         !word_res_it.cycled_list() && word_res_it.data() != next_word_res;
         word_res_it.forward()) {
      if (!word_res_it.data()->part_of_combo) {
        if (prev_row_res == row_res) {
          prev_word_res = word_res;
        }
        word_res = word_res_it.data();
      }
    }
    ASSERT_HOST(!word_res_it.cycled_list());
    wr_it_of_next_word = word_res_it;
    word_res_it.forward();
  } else {
    // word_res_it is OK, but reset word_res and prev_word_res if needed.
    WERD_RES_IT wr_it(&row_res->word_res_list);
    for (wr_it.mark_cycle_pt(); !wr_it.cycled_list(); wr_it.forward()) {
      if (!wr_it.data()->part_of_combo) {
        if (prev_row_res == row_res) {
          prev_word_res = word_res;
        }
        word_res = wr_it.data();
      }
    }
  }
}

/*************************************************************************
 * PAGE_RES_IT::internal_forward
 *
 * Find the next word on the page. If empty_ok is true, then non-text blocks
 * and text blocks with no text are visited as if they contain a single
 * imaginary word in a single imaginary row. (word() and row() both return
 *nullptr in such a block and the return value is nullptr.) If empty_ok is
 *false, the old behaviour is maintained. Each real word is visited and empty
 *and non-text blocks and rows are skipped. new_block is used to initialize the
 *iterators for a new block. The iterator maintains pointers to block, row and
 *word for the previous, current and next words.  These are correct, regardless
 *of block/row boundaries. nullptr values denote start and end of the page.
 *************************************************************************/

WERD_RES *PAGE_RES_IT::internal_forward(bool new_block, bool empty_ok) {
  bool new_row = false;

  prev_block_res = block_res;
  prev_row_res = row_res;
  prev_word_res = word_res;
  block_res = next_block_res;
  row_res = next_row_res;
  word_res = next_word_res;
  wr_it_of_current_word = wr_it_of_next_word;
  next_block_res = nullptr;
  next_row_res = nullptr;
  next_word_res = nullptr;

  while (!block_res_it.cycled_list()) {
    if (new_block) {
      new_block = false;
      row_res_it.set_to_list(&block_res_it.data()->row_res_list);
      row_res_it.mark_cycle_pt();
      if (row_res_it.empty() && empty_ok) {
        next_block_res = block_res_it.data();
        break;
      }
      new_row = true;
    }
    while (!row_res_it.cycled_list()) {
      if (new_row) {
        new_row = false;
        word_res_it.set_to_list(&row_res_it.data()->word_res_list);
        word_res_it.mark_cycle_pt();
      }
      // Skip any part_of_combo words.
      while (!word_res_it.cycled_list() && word_res_it.data()->part_of_combo) {
        word_res_it.forward();
      }
      if (!word_res_it.cycled_list()) {
        next_block_res = block_res_it.data();
        next_row_res = row_res_it.data();
        next_word_res = word_res_it.data();
        wr_it_of_next_word = word_res_it;
        word_res_it.forward();
        goto foundword;
      }
      // end of row reached
      row_res_it.forward();
      new_row = true;
    }
    // end of block reached
    block_res_it.forward();
    new_block = true;
  }
foundword:
  // Update prev_word_best_choice pointer.
  if (page_res != nullptr && page_res->prev_word_best_choice != nullptr) {
    *page_res->prev_word_best_choice = (new_block || prev_word_res == nullptr)
                                           ? nullptr
                                           : prev_word_res->best_choice;
  }
  return word_res;
}

/*************************************************************************
 * PAGE_RES_IT::restart_row()
 *
 * Move to the beginning (leftmost word) of the current row.
 *************************************************************************/
WERD_RES *PAGE_RES_IT::restart_row() {
  ROW_RES *row = this->row();
  if (!row) {
    return nullptr;
  }
  for (restart_page(); this->row() != row; forward()) {
    // pass
  }
  return word();
}

/*************************************************************************
 * PAGE_RES_IT::forward_paragraph
 *
 * Move to the beginning of the next paragraph, allowing empty blocks.
 *************************************************************************/

WERD_RES *PAGE_RES_IT::forward_paragraph() {
  while (block_res == next_block_res &&
         (next_row_res != nullptr && next_row_res->row != nullptr &&
          row_res->row->para() == next_row_res->row->para())) {
    internal_forward(false, true);
  }
  return internal_forward(false, true);
}

/*************************************************************************
 * PAGE_RES_IT::forward_block
 *
 * Move to the beginning of the next block, allowing empty blocks.
 *************************************************************************/

WERD_RES *PAGE_RES_IT::forward_block() {
  while (block_res == next_block_res) {
    internal_forward(false, true);
  }
  return internal_forward(false, true);
}

void PAGE_RES_IT::rej_stat_word() {
  int16_t chars_in_word;
  int16_t rejects_in_word = 0;

  chars_in_word = word_res->reject_map.length();
  page_res->char_count += chars_in_word;
  block_res->char_count += chars_in_word;
  row_res->char_count += chars_in_word;

  rejects_in_word = word_res->reject_map.reject_count();

  page_res->rej_count += rejects_in_word;
  block_res->rej_count += rejects_in_word;
  row_res->rej_count += rejects_in_word;
  if (chars_in_word == rejects_in_word) {
    row_res->whole_word_rej_count += rejects_in_word;
  }
}

} // namespace tesseract
