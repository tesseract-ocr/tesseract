/**********************************************************************
 * File:        pageres.cpp  (Formerly page_res.c)
 * Description: Hierarchy of results classes from PAGE_RES to WERD_RES
 *              and an iterator class to iterate over the words.
 * Main purposes:
 *              Easy way to iterate over the words without a 3-nested loop.
 *              Holds data used during word recognition.
 *              Holds information about alternative spacing paths.
 * Author:      Phil Cheatle
 * Created:     Tue Sep 22 08:42:49 BST 1992
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
#include          <stdlib.h>
#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "blamer.h"
#include          "pageres.h"
#include          "blobs.h"

ELISTIZE (BLOCK_RES)
CLISTIZE (BLOCK_RES) ELISTIZE (ROW_RES) ELISTIZE (WERD_RES)

// Gain factor for computing thresholds that determine the ambiguity of a word.
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
PAGE_RES::PAGE_RES(
    bool merge_similar_words,
    BLOCK_LIST *the_block_list,
    WERD_CHOICE **prev_word_best_choice_ptr) {
  Init();
  BLOCK_IT block_it(the_block_list);
  BLOCK_RES_IT block_res_it(&block_res_list);
  for (block_it.mark_cycle_pt();
       !block_it.cycled_list(); block_it.forward()) {
    block_res_it.add_to_end(new BLOCK_RES(merge_similar_words,
                                          block_it.data()));
  }
  prev_word_best_choice = prev_word_best_choice_ptr;
}

/*************************************************************************
 * BLOCK_RES::BLOCK_RES
 *
 * Constructor for BLOCK results
 *************************************************************************/

BLOCK_RES::BLOCK_RES(bool merge_similar_words, BLOCK *the_block) {
  ROW_IT row_it (the_block->row_list ());
  ROW_RES_IT row_res_it(&row_res_list);

  char_count = 0;
  rej_count = 0;
  font_class = -1;               //not assigned
  x_height = -1.0;
  font_assigned = FALSE;
  bold = FALSE;
  italic = FALSE;
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
  WERD_RES *combo = NULL;        // current combination of fuzzies
  WERD *copy_word;

  char_count = 0;
  rej_count = 0;
  whole_word_rej_count = 0;

  row = the_row;
  bool add_next_word = false;
  TBOX union_box;
  float line_height = the_row->x_height() + the_row->ascenders() -
      the_row->descenders();
  for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
    WERD_RES* word_res = new WERD_RES(word_it.data());
    word_res->x_height = the_row->x_height();
    if (add_next_word) {
      ASSERT_HOST(combo != NULL);
      // We are adding this word to the combination.
      word_res->part_of_combo = TRUE;
      combo->copy_on(word_res);
    } else if (merge_similar_words) {
      union_box = word_res->word->bounding_box();
      add_next_word = !word_res->word->flag(W_REP_CHAR) &&
          union_box.height() <= line_height * kMaxWordSizeRatio;
      word_res->odd_size = !add_next_word;
    }
    WERD* next_word = word_it.data_relative(1);
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
      if (combo == NULL) {
        copy_word = new WERD;
        *copy_word = *(word_it.data());  // deep copy
        combo = new WERD_RES(copy_word);
        combo->x_height = the_row->x_height();
        combo->combination = TRUE;
        word_res_it.add_to_end(combo);
      }
      word_res->part_of_combo = TRUE;
    } else {
      combo = NULL;
    }
    word_res_it.add_to_end(word_res);
  }
}


WERD_RES& WERD_RES::operator=(const WERD_RES & source) {
  this->ELIST_LINK::operator=(source);
  Clear();
  if (source.combination) {
    word = new WERD;
    *word = *(source.word);      // deep copy
  } else {
    word = source.word;          // pt to same word
  }
  if (source.bln_boxes != NULL)
    bln_boxes = new tesseract::BoxWord(*source.bln_boxes);
  if (source.chopped_word != NULL)
    chopped_word = new TWERD(*source.chopped_word);
  if (source.rebuild_word != NULL)
    rebuild_word = new TWERD(*source.rebuild_word);
  // TODO(rays) Do we ever need to copy the seam_array?
  blob_row = source.blob_row;
  denorm = source.denorm;
  if (source.box_word != NULL)
    box_word = new tesseract::BoxWord(*source.box_word);
  best_state = source.best_state;
  correct_text = source.correct_text;
  blob_widths = source.blob_widths;
  blob_gaps = source.blob_gaps;
  // None of the uses of operator= require the ratings matrix to be copied,
  // so don't as it would be really slow.

  // Copy the cooked choices.
  WERD_CHOICE_IT wc_it(const_cast<WERD_CHOICE_LIST*>(&source.best_choices));
  WERD_CHOICE_IT wc_dest_it(&best_choices);
  for (wc_it.mark_cycle_pt(); !wc_it.cycled_list(); wc_it.forward()) {
    const WERD_CHOICE *choice = wc_it.data();
    wc_dest_it.add_after_then_move(new WERD_CHOICE(*choice));
  }
  if (!wc_dest_it.empty()) {
    wc_dest_it.move_to_first();
    best_choice = wc_dest_it.data();
  } else {
    best_choice = NULL;
  }

  if (source.raw_choice != NULL) {
    raw_choice = new WERD_CHOICE(*source.raw_choice);
  } else {
    raw_choice = NULL;
  }
  if (source.ep_choice != NULL) {
    ep_choice = new WERD_CHOICE(*source.ep_choice);
  } else {
    ep_choice = NULL;
  }
  reject_map = source.reject_map;
  combination = source.combination;
  part_of_combo = source.part_of_combo;
  CopySimpleFields(source);
  if (source.blamer_bundle != NULL) {
    blamer_bundle =  new BlamerBundle(*(source.blamer_bundle));
  }
  return *this;
}

// Copies basic fields that don't involve pointers that might be useful
// to copy when making one WERD_RES from another.
void WERD_RES::CopySimpleFields(const WERD_RES& source) {
  tess_failed = source.tess_failed;
  tess_accepted = source.tess_accepted;
  tess_would_adapt = source.tess_would_adapt;
  done = source.done;
  unlv_crunch_mode = source.unlv_crunch_mode;
  small_caps = source.small_caps;
  odd_size = source.odd_size;
  italic = source.italic;
  bold = source.bold;
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
void WERD_RES::InitForRetryRecognition(const WERD_RES& source) {
  word = source.word;
  CopySimpleFields(source);
  if (source.blamer_bundle != NULL) {
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
bool WERD_RES::SetupForRecognition(const UNICHARSET& unicharset_in,
                                   tesseract::Tesseract* tess, Pix* pix,
                                   int norm_mode,
                                   const TBOX* norm_box,
                                   bool numeric_mode,
                                   bool use_body_size,
                                   bool allow_detailed_fx,
                                   ROW *row, const BLOCK* block) {
  tesseract::OcrEngineMode norm_mode_hint =
      static_cast<tesseract::OcrEngineMode>(norm_mode);
  tesseract = tess;
  POLY_BLOCK* pb = block != NULL ? block->poly_block() : NULL;
  if ((norm_mode_hint != tesseract::OEM_CUBE_ONLY &&
       word->cblob_list()->empty()) || (pb != NULL && !pb->IsText())) {
    // Empty words occur when all the blobs have been moved to the rej_blobs
    // list, which seems to occur frequently in junk.
    SetupFake(unicharset_in);
    word->set_flag(W_REP_CHAR, false);
    return false;
  }
  ClearResults();
  SetupWordScript(unicharset_in);
  chopped_word = TWERD::PolygonalCopy(allow_detailed_fx, word);
  float word_xheight = use_body_size && row != NULL && row->body_size() > 0.0f
                     ? row->body_size() : x_height;
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
void WERD_RES::SetupFake(const UNICHARSET& unicharset_in) {
  ClearResults();
  SetupWordScript(unicharset_in);
  chopped_word = new TWERD;
  rebuild_word = new TWERD;
  bln_boxes = new tesseract::BoxWord;
  box_word = new tesseract::BoxWord;
  int blob_count = word->cblob_list()->length();
  if (blob_count > 0) {
    BLOB_CHOICE** fake_choices = new BLOB_CHOICE*[blob_count];
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
    delete [] fake_choices;
  } else {
    WERD_CHOICE* word = new WERD_CHOICE(&unicharset_in);
    word->make_bad();
    LogNewRawChoice(word);
    // Ownership of word is taken by *this WERD_RES in LogNewCookedChoice.
    LogNewCookedChoice(1, false, word);
  }
  tess_failed = true;
  done = true;
}

void WERD_RES::SetupWordScript(const UNICHARSET& uch) {
  uch_set = &uch;
  int script = uch.default_sid();
  word->set_script_id(script);
  word->set_flag(W_SCRIPT_HAS_XHEIGHT, uch.script_has_xheight());
  word->set_flag(W_SCRIPT_IS_LATIN, script == uch.latin_sid());
}

// Sets up the blamer_bundle if it is not null, using the initialized denorm.
void WERD_RES::SetupBlamerBundle() {
  if (blamer_bundle != NULL) {
    blamer_bundle->SetupNormTruthWord(denorm);
  }
}

// Computes the blob_widths and blob_gaps from the chopped_word.
void WERD_RES::SetupBlobWidthsAndGaps() {
  blob_widths.truncate(0);
  blob_gaps.truncate(0);
  int num_blobs = chopped_word->NumBlobs();
  for (int b = 0; b < num_blobs; ++b) {
    TBLOB *blob = chopped_word->blobs[b];
    TBOX box = blob->bounding_box();
    blob_widths.push_back(box.width());
    if (b + 1 < num_blobs) {
      blob_gaps.push_back(
          chopped_word->blobs[b + 1]->bounding_box().left() - box.right());
    }
  }
}

// Updates internal data to account for a new SEAM (chop) at the given
// blob_number. Fixes the ratings matrix and states in the choices, as well
// as the blob widths and gaps.
void WERD_RES::InsertSeam(int blob_number, SEAM* seam) {
  // Insert the seam into the SEAMS array.
  seam->PrepareToInsertSeam(seam_array, chopped_word->blobs, blob_number, true);
  seam_array.insert(seam, blob_number);
  if (ratings != NULL) {
    // Expand the ratings matrix.
    ratings = ratings->ConsumeAndMakeBigger(blob_number);
    // Fix all the segmentation states.
    if (raw_choice != NULL)
      raw_choice->UpdateStateForSplit(blob_number);
    WERD_CHOICE_IT wc_it(&best_choices);
    for (wc_it.mark_cycle_pt(); !wc_it.cycled_list(); wc_it.forward()) {
      WERD_CHOICE* choice = wc_it.data();
      choice->UpdateStateForSplit(blob_number);
    }
    SetupBlobWidthsAndGaps();
  }
}

// Returns true if all the word choices except the first have adjust_factors
// worse than the given threshold.
bool WERD_RES::AlternativeChoiceAdjustmentsWorseThan(float threshold) const {
  // The choices are not changed by this iteration.
  WERD_CHOICE_IT wc_it(const_cast<WERD_CHOICE_LIST*>(&best_choices));
  for (wc_it.forward(); !wc_it.at_first(); wc_it.forward()) {
    WERD_CHOICE* choice = wc_it.data();
    if (choice->adjust_factor() <= threshold)
      return false;
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
  int ratings_dim = ratings->dimension();
  if (raw_choice->TotalOfStates() != ratings_dim) {
    tprintf("raw_choice has total of states = %d vs ratings dim of %d\n",
            raw_choice->TotalOfStates(), ratings_dim);
    return false;
  }
  WERD_CHOICE_IT it(&best_choices);
  int index = 0;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward(), ++index) {
    WERD_CHOICE* choice = it.data();
    if (choice->TotalOfStates() != ratings_dim) {
      tprintf("Cooked #%d has total of states = %d vs ratings dim of %d\n",
              index, choice->TotalOfStates(), ratings_dim);
      return false;
    }
  }
  return true;
}

// Prints a list of words found if debug is true or the word result matches
// the word_to_debug.
void WERD_RES::DebugWordChoices(bool debug, const char* word_to_debug) {
  if (debug ||
      (word_to_debug != NULL && *word_to_debug != '\0' && best_choice != NULL &&
       best_choice->unichar_string() == STRING(word_to_debug))) {
    if (raw_choice != NULL)
      raw_choice->print("\nBest Raw Choice");

    WERD_CHOICE_IT it(&best_choices);
    int index = 0;
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward(), ++index) {
      WERD_CHOICE* choice = it.data();
      STRING label;
      label.add_str_int("\nCooked Choice #", index);
      choice->print(label.string());
    }
  }
}

// Prints the top choice along with the accepted/done flags.
void WERD_RES::DebugTopChoice(const char* msg) const {
  tprintf("Best choice: accepted=%d, adaptable=%d, done=%d : ",
          tess_accepted, tess_would_adapt, done);
  if (best_choice == NULL)
    tprintf("<Null choice>\n");
  else
    best_choice->print(msg);
}

// Removes from best_choices all choices which are not within a reasonable
// range of the best choice.
// TODO(rays) incorporate the information used here into the params training
// re-ranker, in place of this heuristic that is based on the previous
// adjustment factor.
void WERD_RES::FilterWordChoices(int debug_level) {
  if (best_choice == NULL || best_choices.singleton())
    return;

  if (debug_level >= 2)
    best_choice->print("\nFiltering against best choice");
  WERD_CHOICE_IT it(&best_choices);
  int index = 0;
  for (it.forward(); !it.at_first(); it.forward(), ++index) {
    WERD_CHOICE* choice = it.data();
    float threshold = StopperAmbigThreshold(best_choice->adjust_factor(),
                                            choice->adjust_factor());
    // i, j index the blob choice in choice, best_choice.
    // chunk is an index into the chopped_word blobs (AKA chunks).
    // Since the two words may use different segmentations of the chunks, we
    // iterate over the chunks to find out whether a comparable blob
    // classification is much worse than the best result.
    int i = 0, j = 0, chunk = 0;
    // Each iteration of the while deals with 1 chunk. On entry choice_chunk
    // and best_chunk are the indices of the first chunk in the NEXT blob,
    // i.e. we don't have to increment i, j while chunk < choice_chunk and
    // best_chunk respectively.
    int choice_chunk = choice->state(0), best_chunk = best_choice->state(0);
    while (i < choice->length() && j < best_choice->length()) {
      if (choice->unichar_id(i) != best_choice->unichar_id(j) &&
          choice->certainty(i) - best_choice->certainty(j) < threshold) {
        if (debug_level >= 2) {
          STRING label;
          label.add_str_int("\nDiscarding bad choice #", index);
          choice->print(label.string());
          tprintf("i %d j %d Chunk %d Choice->Blob[i].Certainty %.4g"
              " BestChoice->ChunkCertainty[Chunk] %g Threshold %g\n",
              i, j, chunk, choice->certainty(i),
              best_choice->certainty(j), threshold);
        }
        delete it.extract();
        break;
      }
      ++chunk;
      // If needed, advance choice_chunk to keep up with chunk.
      while (choice_chunk < chunk && ++i < choice->length())
        choice_chunk += choice->state(i);
      // If needed, advance best_chunk to keep up with chunk.
      while (best_chunk < chunk && ++j < best_choice->length())
        best_chunk += best_choice->state(j);
    }
  }
}

void WERD_RES::ComputeAdaptionThresholds(float certainty_scale,
                                         float min_rating,
                                         float max_rating,
                                         float rating_margin,
                                         float* thresholds) {
  int chunk = 0;
  int end_chunk = best_choice->state(0);
  int end_raw_chunk = raw_choice->state(0);
  int raw_blob = 0;
  for (int i = 0; i < best_choice->length(); i++, thresholds++) {
    float avg_rating = 0.0f;
    int num_error_chunks = 0;

    // For each chunk in best choice blob i, count non-matching raw results.
    while (chunk < end_chunk) {
      if (chunk >= end_raw_chunk) {
        ++raw_blob;
        end_raw_chunk += raw_choice->state(raw_blob);
      }
      if (best_choice->unichar_id(i) !=
          raw_choice->unichar_id(raw_blob)) {
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

    if (*thresholds > max_rating)
      *thresholds = max_rating;
    if (*thresholds < min_rating)
      *thresholds = min_rating;
  }
}

// Saves a copy of the word_choice if it has the best unadjusted rating.
// Returns true if the word_choice was the new best.
bool WERD_RES::LogNewRawChoice(WERD_CHOICE* word_choice) {
  if (raw_choice == NULL || word_choice->rating() < raw_choice->rating()) {
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
                                  WERD_CHOICE* word_choice) {
  if (best_choice != NULL) {
    // Throw out obviously bad choices to save some work.
    // TODO(rays) Get rid of this! This piece of code produces different
    // results according to the order in which words are found, which is an
    // undesirable behavior. It would be better to keep all the choices and
    // prune them later when more information is available.
    float max_certainty_delta =
        StopperAmbigThreshold(best_choice->adjust_factor(),
                              word_choice->adjust_factor());
    if (max_certainty_delta > -kStopperAmbiguityThresholdOffset)
      max_certainty_delta = -kStopperAmbiguityThresholdOffset;
    if (word_choice->certainty() - best_choice->certainty() <
        max_certainty_delta) {
      if (debug) {
        STRING bad_string;
        word_choice->string_and_lengths(&bad_string, NULL);
        tprintf("Discarding choice \"%s\" with an overly low certainty"
                " %.3f vs best choice certainty %.3f (Threshold: %.3f)\n",
                bad_string.string(), word_choice->certainty(),
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
  const STRING& new_str = word_choice->unichar_string();
  bool inserted = false;
  int num_choices = 0;
  if (!it.empty()) {
    do {
      WERD_CHOICE* choice = it.data();
      if (choice->rating() > word_choice->rating() && !inserted) {
        // Time to insert.
        it.add_before_stay_put(word_choice);
        inserted = true;
        if (num_choices == 0)
          best_choice = word_choice;  // This is the new best.
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
                    new_str.string(), word_choice->rating(), choice->rating());
          }
          delete word_choice;
          return false;
        }
      } else {
        ++num_choices;
        if (num_choices > max_num_choices)
          delete it.extract();
      }
      it.forward();
    } while (!it.at_first());
  }
  if (!inserted && num_choices < max_num_choices) {
    it.add_to_end(word_choice);
    inserted = true;
    if (num_choices == 0)
      best_choice = word_choice;  // This is the new best.
  }
  if (debug) {
    if (inserted)
      tprintf("New %s", best_choice == word_choice ? "Best" : "Secondary");
    else
      tprintf("Poor");
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
template<class T> static void MovePointerData(T** dest, T**src) {
  delete *dest;
  *dest = *src;
  *src = NULL;
}

// Prints a brief list of all the best choices.
void WERD_RES::PrintBestChoices() const {
  STRING alternates_str;
  WERD_CHOICE_IT it(const_cast<WERD_CHOICE_LIST*>(&best_choices));
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    if (!it.at_first()) alternates_str += "\", \"";
    alternates_str += it.data()->unichar_string();
  }
  tprintf("Alternates for \"%s\": {\"%s\"}\n",
          best_choice->unichar_string().string(), alternates_str.string());
}

// Returns the sum of the widths of the blob between start_blob and last_blob
// inclusive.
int WERD_RES::GetBlobsWidth(int start_blob, int last_blob) {
  int result = 0;
  for (int b = start_blob; b <= last_blob; ++b) {
    result += blob_widths[b];
    if (b < last_blob)
      result += blob_gaps[b];
  }
  return result;
}
// Returns the width of a gap between the specified blob and the next one.
int WERD_RES::GetBlobsGap(int blob_index) {
  if (blob_index < 0 || blob_index >= blob_gaps.size())
    return 0;
  return blob_gaps[blob_index];
}

// Returns the BLOB_CHOICE corresponding to the given index in the
// best choice word taken from the appropriate cell in the ratings MATRIX.
// Borrowed pointer, so do not delete. May return NULL if there is no
// BLOB_CHOICE matching the unichar_id at the given index.
BLOB_CHOICE* WERD_RES::GetBlobChoice(int index) const {
  if (index < 0 || index >= best_choice->length()) return NULL;
  BLOB_CHOICE_LIST* choices = GetBlobChoices(index);
  return FindMatchingChoice(best_choice->unichar_id(index), choices);
}

// Returns the BLOB_CHOICE_LIST corresponding to the given index in the
// best choice word taken from the appropriate cell in the ratings MATRIX.
// Borrowed pointer, so do not delete.
BLOB_CHOICE_LIST* WERD_RES::GetBlobChoices(int index) const {
  return best_choice->blob_choices(index, ratings);
}

// Moves the results fields from word to this. This takes ownership of all
// the data, so src can be destructed.
void WERD_RES::ConsumeWordResults(WERD_RES* word) {
  denorm = word->denorm;
  blob_row = word->blob_row;
  MovePointerData(&chopped_word, &word->chopped_word);
  MovePointerData(&rebuild_word, &word->rebuild_word);
  MovePointerData(&box_word, &word->box_word);
  seam_array.delete_data_pointers();
  seam_array = word->seam_array;
  word->seam_array.clear();
  best_state.move(&word->best_state);
  correct_text.move(&word->correct_text);
  blob_widths.move(&word->blob_widths);
  blob_gaps.move(&word->blob_gaps);
  if (ratings != NULL) ratings->delete_matrix_pointers();
  MovePointerData(&ratings, &word->ratings);
  best_choice = word->best_choice;
  MovePointerData(&raw_choice, &word->raw_choice);
  best_choices.clear();
  WERD_CHOICE_IT wc_it(&best_choices);
  wc_it.add_list_after(&word->best_choices);
  reject_map = word->reject_map;
  if (word->blamer_bundle != NULL) {
    assert(blamer_bundle != NULL);
    blamer_bundle->CopyResults(*(word->blamer_bundle));
  }
  CopySimpleFields(*word);
}

// Replace the best choice and rebuild box word.
// choice must be from the current best_choices list.
void WERD_RES::ReplaceBestChoice(WERD_CHOICE* choice) {
  best_choice = choice;
  RebuildBestState();
  SetupBoxWord();
  // Make up a fake reject map of the right length to keep the
  // rejection pass happy.
  reject_map.initialise(best_state.length());
  done = tess_accepted = tess_would_adapt = true;
  SetScriptPositions();
}

// Builds the rebuild_word and sets the best_state from the chopped_word and
// the best_choice->state.
void WERD_RES::RebuildBestState() {
  ASSERT_HOST(best_choice != NULL);
  if (rebuild_word != NULL)
    delete rebuild_word;
  rebuild_word = new TWERD;
  if (seam_array.empty())
    start_seam_list(chopped_word, &seam_array);
  best_state.truncate(0);
  int start = 0;
  for (int i = 0; i < best_choice->length(); ++i) {
    int length = best_choice->state(i);
    best_state.push_back(length);
    if (length > 1) {
      SEAM::JoinPieces(seam_array, chopped_word->blobs, start,
                       start + length - 1);
    }
    TBLOB* blob = chopped_word->blobs[start];
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
  if (rebuild_word != NULL)
    delete rebuild_word;
  rebuild_word = new TWERD(*chopped_word);
  SetupBoxWord();
  int word_len = box_word->length();
  best_state.reserve(word_len);
  correct_text.reserve(word_len);
  for (int i = 0; i < word_len; ++i) {
    best_state.push_back(1);
    correct_text.push_back(STRING(""));
  }
}

// Sets/replaces the box_word with one made from the rebuild_word.
void WERD_RES::SetupBoxWord() {
  if (box_word != NULL)
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
  for (wc_it.mark_cycle_pt(); !wc_it.cycled_list(); wc_it.forward())
    wc_it.data()->SetAllScriptPositions(position);
}

// Classifies the word with some already-calculated BLOB_CHOICEs.
// The choices are an array of blob_count pointers to BLOB_CHOICE,
// providing a single classifier result for each blob.
// The BLOB_CHOICEs are consumed and the word takes ownership.
// The number of blobs in the box_word must match blob_count.
void WERD_RES::FakeClassifyWord(int blob_count, BLOB_CHOICE** choices) {
  // Setup the WERD_RES.
  ASSERT_HOST(box_word != NULL);
  ASSERT_HOST(blob_count == box_word->length());
  ClearWordChoices();
  ClearRatings();
  ratings = new MATRIX(blob_count, 1);
  for (int c = 0; c < blob_count; ++c) {
    BLOB_CHOICE_LIST* choice_list = new BLOB_CHOICE_LIST;
    BLOB_CHOICE_IT choice_it(choice_list);
    choice_it.add_after_then_move(choices[c]);
    ratings->put(c, c, choice_list);
  }
  FakeWordFromRatings(TOP_CHOICE_PERM);
  reject_map.initialise(blob_count);
  done = true;
}

// Creates a WERD_CHOICE for the word using the top choices from the leading
// diagonal of the ratings matrix.
void WERD_RES::FakeWordFromRatings(PermuterType permuter) {
  int num_blobs = ratings->dimension();
  WERD_CHOICE* word_choice = new WERD_CHOICE(uch_set, num_blobs);
  word_choice->set_permuter(permuter);
  for (int b = 0; b < num_blobs; ++b) {
    UNICHAR_ID unichar_id = UNICHAR_SPACE;
    float rating = MAX_INT32;
    float certainty = -MAX_INT32;
    BLOB_CHOICE_LIST* choices = ratings->get(b, b);
    if (choices != NULL && !choices->empty()) {
      BLOB_CHOICE_IT bc_it(choices);
      BLOB_CHOICE* choice = bc_it.data();
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
  ASSERT_HOST(best_choice != NULL);
  for (int i = 0; i < best_choice->length(); ++i) {
    UNICHAR_ID choice_id = best_choice->unichar_id(i);
    const char* blob_choice = uch_set->id_to_unichar(choice_id);
    correct_text.push_back(STRING(blob_choice));
  }
}

// Merges 2 adjacent blobs in the result if the permanent callback
// class_cb returns other than INVALID_UNICHAR_ID, AND the permanent
// callback box_cb is NULL or returns true, setting the merged blob
// result to the class returned from class_cb.
// Returns true if anything was merged.
bool WERD_RES::ConditionalBlobMerge(
    TessResultCallback2<UNICHAR_ID, UNICHAR_ID, UNICHAR_ID>* class_cb,
    TessResultCallback2<bool, const TBOX&, const TBOX&>* box_cb) {
  ASSERT_HOST(best_choice->length() == 0 || ratings != NULL);
  bool modified = false;
  for (int i = 0; i + 1 < best_choice->length(); ++i) {
    UNICHAR_ID new_id = class_cb->Run(best_choice->unichar_id(i),
                                      best_choice->unichar_id(i+1));
    if (new_id != INVALID_UNICHAR_ID &&
        (box_cb == NULL || box_cb->Run(box_word->BlobBox(i),
                                       box_word->BlobBox(i + 1)))) {
      // Raw choice should not be fixed.
      best_choice->set_unichar_id(new_id, i);
      modified = true;
      MergeAdjacentBlobs(i);
      const MATRIX_COORD& coord = best_choice->MatrixCoord(i);
      if (!coord.Valid(*ratings)) {
        ratings->IncreaseBandSize(coord.row + 1 - coord.col);
      }
      BLOB_CHOICE_LIST* blob_choices = GetBlobChoices(i);
      if (FindMatchingChoice(new_id, blob_choices) == NULL) {
        // Insert a fake result.
        BLOB_CHOICE* blob_choice = new BLOB_CHOICE;
        blob_choice->set_unichar_id(new_id);
        BLOB_CHOICE_IT bc_it(blob_choices);
        bc_it.add_before_then_move(blob_choice);
      }
    }
  }
  delete class_cb;
  delete box_cb;
  return modified;
}

// Merges 2 adjacent blobs in the result (index and index+1) and corrects
// all the data to account for the change.
void WERD_RES::MergeAdjacentBlobs(int index) {
  if (reject_map.length() == best_choice->length())
    reject_map.remove_pos(index);
  best_choice->remove_unichar_id(index + 1);
  rebuild_word->MergeBlobs(index, index + 2);
  box_word->MergeBoxes(index, index + 2);
  if (index + 1 < best_state.length()) {
    best_state[index] += best_state[index + 1];
    best_state.remove(index + 1);
  }
}

// TODO(tkielbus) Decide between keeping this behavior here or modifying the
// training data.

// Utility function for fix_quotes
// Return true if the next character in the string (given the UTF8 length in
// bytes) is a quote character.
static int is_simple_quote(const char* signed_str, int length) {
  const unsigned char* str =
      reinterpret_cast<const unsigned char*>(signed_str);
  // Standard 1 byte quotes.
  return (length == 1 && (*str == '\'' || *str == '`')) ||
      // UTF-8 3 bytes curved quotes.
      (length == 3 && ((*str == 0xe2 &&
                        *(str + 1) == 0x80 &&
                        *(str + 2) == 0x98) ||
                       (*str == 0xe2 &&
                        *(str + 1) == 0x80 &&
                        *(str + 2) == 0x99)));
}

// Callback helper for fix_quotes returns a double quote if both
// arguments are quote, otherwise INVALID_UNICHAR_ID.
UNICHAR_ID WERD_RES::BothQuotes(UNICHAR_ID id1, UNICHAR_ID id2) {
  const char *ch = uch_set->id_to_unichar(id1);
  const char *next_ch = uch_set->id_to_unichar(id2);
  if (is_simple_quote(ch, strlen(ch)) &&
      is_simple_quote(next_ch, strlen(next_ch)))
    return uch_set->unichar_to_id("\"");
  return INVALID_UNICHAR_ID;
}

// Change pairs of quotes to double quotes.
void WERD_RES::fix_quotes() {
  if (!uch_set->contains_unichar("\"") ||
      !uch_set->get_enabled(uch_set->unichar_to_id("\"")))
    return;  // Don't create it if it is disallowed.

  ConditionalBlobMerge(
      NewPermanentTessCallback(this, &WERD_RES::BothQuotes),
      NULL);
}

// Callback helper for fix_hyphens returns UNICHAR_ID of - if both
// arguments are hyphen, otherwise INVALID_UNICHAR_ID.
UNICHAR_ID WERD_RES::BothHyphens(UNICHAR_ID id1, UNICHAR_ID id2) {
  const char *ch = uch_set->id_to_unichar(id1);
  const char *next_ch = uch_set->id_to_unichar(id2);
  if (strlen(ch) == 1 && strlen(next_ch) == 1 &&
      (*ch == '-' || *ch == '~') && (*next_ch == '-' || *next_ch == '~'))
    return uch_set->unichar_to_id("-");
  return INVALID_UNICHAR_ID;
}

// Callback helper for fix_hyphens returns true if box1 and box2 overlap
// (assuming both on the same textline, are in order and a chopped em dash.)
bool WERD_RES::HyphenBoxesOverlap(const TBOX& box1, const TBOX& box2) {
  return box1.right() >= box2.left();
}

// Change pairs of hyphens to a single hyphen if the bounding boxes touch
// Typically a long dash which has been segmented.
void WERD_RES::fix_hyphens() {
  if (!uch_set->contains_unichar("-") ||
      !uch_set->get_enabled(uch_set->unichar_to_id("-")))
    return;  // Don't create it if it is disallowed.

  ConditionalBlobMerge(
      NewPermanentTessCallback(this, &WERD_RES::BothHyphens),
      NewPermanentTessCallback(this, &WERD_RES::HyphenBoxesOverlap));
}

// Callback helper for merge_tess_fails returns a space if both
// arguments are space, otherwise INVALID_UNICHAR_ID.
UNICHAR_ID WERD_RES::BothSpaces(UNICHAR_ID id1, UNICHAR_ID id2) {
  if (id1 == id2 && id1 == uch_set->unichar_to_id(" "))
    return id1;
  else
    return INVALID_UNICHAR_ID;
}

// Change pairs of tess failures to a single one
void WERD_RES::merge_tess_fails() {
  if (ConditionalBlobMerge(
      NewPermanentTessCallback(this, &WERD_RES::BothSpaces), NULL)) {
    int len = best_choice->length();
    ASSERT_HOST(reject_map.length() == len);
    ASSERT_HOST(box_word->length() == len);
  }
}

// Returns true if the collection of count pieces, starting at start, are all
// natural connected components, ie there are no real chops involved.
bool WERD_RES::PiecesAllNatural(int start, int count) const {
  // all seams must have no splits.
  for (int index = start; index < start + count - 1; ++index) {
    if (index >= 0 && index < seam_array.size()) {
      SEAM* seam = seam_array[index];
      if (seam != NULL && seam->HasAnySplits()) return false;
    }
  }
  return true;
}


WERD_RES::~WERD_RES () {
  Clear();
}

void WERD_RES::InitNonPointers() {
  tess_failed = FALSE;
  tess_accepted = FALSE;
  tess_would_adapt = FALSE;
  done = FALSE;
  unlv_crunch_mode = CR_NONE;
  small_caps = false;
  odd_size = false;
  italic = FALSE;
  bold = FALSE;
  // The fontinfos and tesseract count as non-pointers as they point to
  // data owned elsewhere.
  fontinfo = NULL;
  fontinfo2 = NULL;
  tesseract = NULL;
  fontinfo_id_count = 0;
  fontinfo_id2_count = 0;
  x_height = 0.0;
  caps_height = 0.0;
  baseline_shift = 0.0f;
  space_certainty = 0.0f;
  guessed_x_ht = TRUE;
  guessed_caps_ht = TRUE;
  combination = FALSE;
  part_of_combo = FALSE;
  reject_spaces = FALSE;
}

void WERD_RES::InitPointers() {
  word = NULL;
  bln_boxes = NULL;
  blob_row = NULL;
  uch_set = NULL;
  chopped_word = NULL;
  rebuild_word = NULL;
  box_word = NULL;
  ratings = NULL;
  best_choice = NULL;
  raw_choice = NULL;
  ep_choice = NULL;
  blamer_bundle = NULL;
}

void WERD_RES::Clear() {
  if (word != NULL && combination) {
    delete word;
  }
  word = NULL;
  delete blamer_bundle;
  blamer_bundle = NULL;
  ClearResults();
}

void WERD_RES::ClearResults() {
  done = false;
  fontinfo = NULL;
  fontinfo2 = NULL;
  fontinfo_id_count = 0;
  fontinfo_id2_count = 0;
  if (bln_boxes != NULL) {
    delete bln_boxes;
    bln_boxes = NULL;
  }
  blob_row = NULL;
  if (chopped_word != NULL) {
    delete chopped_word;
    chopped_word = NULL;
  }
  if (rebuild_word != NULL) {
    delete rebuild_word;
    rebuild_word = NULL;
  }
  if (box_word != NULL) {
    delete box_word;
    box_word = NULL;
  }
  best_state.clear();
  correct_text.clear();
  seam_array.delete_data_pointers();
  seam_array.clear();
  blob_widths.clear();
  blob_gaps.clear();
  ClearRatings();
  ClearWordChoices();
  if (blamer_bundle != NULL) blamer_bundle->ClearResults();
}
void WERD_RES::ClearWordChoices() {
  best_choice = NULL;
  if (raw_choice != NULL) {
    delete raw_choice;
    raw_choice = NULL;
  }
  best_choices.clear();
  if (ep_choice != NULL) {
    delete ep_choice;
    ep_choice = NULL;
  }
}
void WERD_RES::ClearRatings() {
  if (ratings != NULL) {
    ratings->delete_matrix_pointers();
    delete ratings;
    ratings = NULL;
  }
}


bool PAGE_RES_IT::operator ==(const PAGE_RES_IT &other) const {
  return word_res == other.word_res &&
      row_res == other.row_res &&
      block_res == other.block_res;
}

int PAGE_RES_IT::cmp(const PAGE_RES_IT &other) const {
  ASSERT_HOST(page_res == other.page_res);
  if (other.block_res == NULL) {
    // other points to the end of the page.
    if (block_res == NULL)
      return 0;
    return -1;
  }
  if (block_res == NULL) {
    return 1; // we point to the end of the page.
  }
  if (block_res == other.block_res) {
    if (other.row_res == NULL || row_res == NULL) {
      // this should only happen if we hit an image block.
      return 0;
    }
    if (row_res == other.row_res) {
      // we point to the same block and row.
      ASSERT_HOST(other.word_res != NULL && word_res != NULL);
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
      ASSERT_HOST("Error: Incomparable PAGE_RES_ITs" == NULL);
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
    ASSERT_HOST("Error: Incomparable PAGE_RES_ITs" == NULL);
  }

  // We point to different blocks.
  BLOCK_RES_IT block_res_it(&page_res->block_res_list);
  for (block_res_it.mark_cycle_pt();
       !block_res_it.cycled_list(); block_res_it.forward()) {
    if (block_res_it.data() == block_res) {
      return -1;
    } else if (block_res_it.data() == other.block_res) {
      return 1;
    }
  }
  // Shouldn't happen...
  ASSERT_HOST("Error: Incomparable PAGE_RES_ITs" == NULL);
  return 0;
}

// Inserts the new_word as a combination owned by a corresponding WERD_RES
// before the current position. The simple fields of the WERD_RES are copied
// from clone_res and the resulting WERD_RES is returned for further setup
// with best_choice etc.
WERD_RES* PAGE_RES_IT::InsertSimpleCloneWord(const WERD_RES& clone_res,
                                             WERD* new_word) {
  // Make a WERD_RES for the new_word.
  WERD_RES* new_res = new WERD_RES(new_word);
  new_res->CopySimpleFields(clone_res);
  new_res->combination = true;
  // Insert into the appropriate place in the ROW_RES.
  WERD_RES_IT wr_it(&row()->word_res_list);
  for (wr_it.mark_cycle_pt(); !wr_it.cycled_list(); wr_it.forward()) {
    WERD_RES* word = wr_it.data();
    if (word == word_res)
      break;
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

// Helper computes the boundaries between blobs in the word. The blob bounds
// are likely very poor, if they come from LSTM, where it only outputs the
// character at one pixel within it, so we find the midpoints between them.
static void ComputeBlobEnds(const WERD_RES& word, C_BLOB_LIST* next_word_blobs,
                            GenericVector<int>* blob_ends) {
  C_BLOB_IT blob_it(word.word->cblob_list());
  for (int i = 0; i < word.best_state.size(); ++i) {
    int length = word.best_state[i];
    // Get the bounding box of the fake blobs
    TBOX blob_box = blob_it.data()->bounding_box();
    blob_it.forward();
    for (int b = 1; b < length; ++b) {
      blob_box += blob_it.data()->bounding_box();
      blob_it.forward();
    }
    // This blob_box is crap, so for now we are only looking for the
    // boundaries between them.
    int blob_end = MAX_INT32;
    if (!blob_it.at_first() || next_word_blobs != NULL) {
      if (blob_it.at_first())
        blob_it.set_to_list(next_word_blobs);
      blob_end = (blob_box.right() + blob_it.data()->bounding_box().left()) / 2;
    }
    blob_ends->push_back(blob_end);
  }
}

// Replaces the current WERD/WERD_RES with the given words. The given words
// contain fake blobs that indicate the position of the characters. These are
// replaced with real blobs from the current word as much as possible.
void PAGE_RES_IT::ReplaceCurrentWord(
    tesseract::PointerVector<WERD_RES>* words) {
  if (words->empty()) {
    DeleteCurrentWord();
    return;
  }
  WERD_RES* input_word = word();
  // Set the BOL/EOL flags on the words from the input word.
  if (input_word->word->flag(W_BOL)) {
    (*words)[0]->word->set_flag(W_BOL, true);
  } else {
    (*words)[0]->word->set_blanks(1);
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
      WERD* word = w_it.data();
      if (word == input_word->word)
        break;
    }
    // w_it is now set to the input_word's word.
    ASSERT_HOST(!w_it.cycled_list());
  }
  // Insert into the appropriate place in the ROW_RES.
  WERD_RES_IT wr_it(&row()->word_res_list);
  for (wr_it.mark_cycle_pt(); !wr_it.cycled_list(); wr_it.forward()) {
    WERD_RES* word = wr_it.data();
    if (word == input_word)
      break;
  }
  ASSERT_HOST(!wr_it.cycled_list());
  // Since we only have an estimate of the bounds between blobs, use the blob
  // x-middle as the determiner of where to put the blobs
  C_BLOB_IT src_b_it(input_word->word->cblob_list());
  src_b_it.sort(&C_BLOB::SortByXMiddle);
  C_BLOB_IT rej_b_it(input_word->word->rej_cblob_list());
  rej_b_it.sort(&C_BLOB::SortByXMiddle);
  for (int w = 0; w < words->size(); ++w) {
    WERD_RES* word_w = (*words)[w];
    // Compute blob boundaries.
    GenericVector<int> blob_ends;
    C_BLOB_LIST* next_word_blobs =
        w + 1 < words->size() ? (*words)[w + 1]->word->cblob_list() : NULL;
    ComputeBlobEnds(*word_w, next_word_blobs, &blob_ends);
    // Delete the fake blobs on the current word.
    word_w->word->cblob_list()->clear();
    C_BLOB_IT dest_it(word_w->word->cblob_list());
    // Build the box word as we move the blobs.
    tesseract::BoxWord* box_word = new tesseract::BoxWord;
    for (int i = 0; i < blob_ends.size(); ++i) {
      int end_x = blob_ends[i];
      TBOX blob_box;
      // Add the blobs up to end_x.
      while (!src_b_it.empty() &&
             src_b_it.data()->bounding_box().x_middle() < end_x) {
        blob_box += src_b_it.data()->bounding_box();
        dest_it.add_after_then_move(src_b_it.extract());
        src_b_it.forward();
      }
      while (!rej_b_it.empty() &&
             rej_b_it.data()->bounding_box().x_middle() < end_x) {
        blob_box += rej_b_it.data()->bounding_box();
        dest_it.add_after_then_move(rej_b_it.extract());
        rej_b_it.forward();
      }
      // Clip to the previously computed bounds. Although imperfectly accurate,
      // it is good enough, and much more complicated to determine where else
      // to clip.
      if (i > 0 && blob_box.left() < blob_ends[i - 1])
        blob_box.set_left(blob_ends[i - 1]);
      if (blob_box.right() > end_x)
        blob_box.set_right(end_x);
      box_word->InsertBox(i, blob_box);
    }
    // Fix empty boxes. If a very joined blob sits over multiple characters,
    // then we will have some empty boxes from using the middle, so look for
    // overlaps.
    for (int i = 0; i < box_word->length(); ++i) {
      TBOX box = box_word->BlobBox(i);
      if (box.null_box()) {
        // Nothing has its middle in the bounds of this blob, so use anything
        // that overlaps.
        for (dest_it.mark_cycle_pt(); !dest_it.cycled_list();
             dest_it.forward()) {
          TBOX blob_box = dest_it.data()->bounding_box();
          if (blob_box.left() < blob_ends[i] &&
              (i == 0 || blob_box.right() >= blob_ends[i - 1])) {
            if (i > 0 && blob_box.left() < blob_ends[i - 1])
              blob_box.set_left(blob_ends[i - 1]);
            if (blob_box.right() > blob_ends[i])
              blob_box.set_right(blob_ends[i]);
            box_word->ChangeBox(i, blob_box);
            break;
          }
        }
      }
    }
    delete word_w->box_word;
    word_w->box_word = box_word;
    if (!input_word->combination) {
      // Insert word_w->word into the ROW. It doesn't own its word, so the
      // ROW needs to own it.
      w_it.add_before_stay_put(word_w->word);
      word_w->combination = false;
    }
    (*words)[w] = NULL;  // We are taking ownership.
    wr_it.add_before_stay_put(word_w);
  }
  // We have taken ownership of the words.
  words->clear();
  // Delete the current word, which has been replaced. We could just call
  // DeleteCurrentWord, but that would iterate both lists again, and we know
  // we are already in the right place.
  if (!input_word->combination)
    delete w_it.extract();
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
      word_res = NULL;
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
  WERD* real_word = word_res->word;
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
  prev_block_res = NULL;
  prev_row_res = NULL;
  prev_word_res = NULL;
  block_res = NULL;
  row_res = NULL;
  word_res = NULL;
  next_block_res = NULL;
  next_row_res = NULL;
  next_word_res = NULL;
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
        if (prev_row_res == row_res) prev_word_res = word_res;
        word_res = word_res_it.data();
      }
    }
    ASSERT_HOST(!word_res_it.cycled_list());
    word_res_it.forward();
  } else {
    // word_res_it is OK, but reset word_res and prev_word_res if needed.
    WERD_RES_IT wr_it(&row_res->word_res_list);
    for (wr_it.mark_cycle_pt(); !wr_it.cycled_list(); wr_it.forward()) {
      if (!wr_it.data()->part_of_combo) {
        if (prev_row_res == row_res) prev_word_res = word_res;
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
 * imaginary word in a single imaginary row. (word() and row() both return NULL
 * in such a block and the return value is NULL.)
 * If empty_ok is false, the old behaviour is maintained. Each real word
 * is visited and empty and non-text blocks and rows are skipped.
 * new_block is used to initialize the iterators for a new block.
 * The iterator maintains pointers to block, row and word for the previous,
 * current and next words.  These are correct, regardless of block/row
 * boundaries. NULL values denote start and end of the page.
 *************************************************************************/

WERD_RES *PAGE_RES_IT::internal_forward(bool new_block, bool empty_ok) {
  bool new_row = false;

  prev_block_res = block_res;
  prev_row_res = row_res;
  prev_word_res = word_res;
  block_res = next_block_res;
  row_res = next_row_res;
  word_res = next_word_res;
  next_block_res = NULL;
  next_row_res = NULL;
  next_word_res = NULL;

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
      while (!word_res_it.cycled_list() && word_res_it.data()->part_of_combo)
        word_res_it.forward();
      if (!word_res_it.cycled_list()) {
        next_block_res = block_res_it.data();
        next_row_res = row_res_it.data();
        next_word_res = word_res_it.data();
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
  if (page_res != NULL && page_res->prev_word_best_choice != NULL) {
    *page_res->prev_word_best_choice =
      (new_block || prev_word_res == NULL) ? NULL : prev_word_res->best_choice;
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
  if (!row) return NULL;
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
         (next_row_res != NULL && next_row_res->row != NULL &&
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
  inT16 chars_in_word;
  inT16 rejects_in_word = 0;

  chars_in_word = word_res->reject_map.length ();
  page_res->char_count += chars_in_word;
  block_res->char_count += chars_in_word;
  row_res->char_count += chars_in_word;

  rejects_in_word = word_res->reject_map.reject_count ();

  page_res->rej_count += rejects_in_word;
  block_res->rej_count += rejects_in_word;
  row_res->rej_count += rejects_in_word;
  if (chars_in_word == rejects_in_word)
    row_res->whole_word_rej_count += rejects_in_word;
}
