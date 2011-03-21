/**********************************************************************
 * File:        pageres.cpp  (Formerly page_res.c)
 * Description: Results classes used by control.c
 * Author:		Phil Cheatle
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
#include "mfcpch.h"
#include          <stdlib.h>
#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "pageres.h"
#include          "blobs.h"

ELISTIZE (BLOCK_RES)
CLISTIZE (BLOCK_RES) ELISTIZE (ROW_RES) ELISTIZE (WERD_RES)
/*************************************************************************
 * PAGE_RES::PAGE_RES
 *
 * Constructor for page results
 *************************************************************************/
PAGE_RES::PAGE_RES(
    BLOCK_LIST *the_block_list,
    WERD_CHOICE **prev_word_best_choice_ptr) {
  BLOCK_IT block_it(the_block_list);
  BLOCK_RES_IT block_res_it(&block_res_list);

  char_count = 0;
  rej_count = 0;
  rejected = FALSE;

  for (block_it.mark_cycle_pt();
       !block_it.cycled_list(); block_it.forward()) {
    block_res_it.add_to_end(new BLOCK_RES(block_it.data()));
  }

  prev_word_best_choice = prev_word_best_choice_ptr;
}


/*************************************************************************
 * BLOCK_RES::BLOCK_RES
 *
 * Constructor for BLOCK results
 *************************************************************************/

BLOCK_RES::BLOCK_RES(BLOCK *the_block) {
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
    row_res_it.add_to_end(new ROW_RES(the_block->right_to_left(),
                                      row_it.data()));
  }
}


/*************************************************************************
 * ROW_RES::ROW_RES
 *
 * Constructor for ROW results
 *************************************************************************/

ROW_RES::ROW_RES(bool right_to_left,
                 ROW *the_row) {
  WERD_IT word_it(the_row->word_list());
  WERD_RES_IT word_res_it(&word_res_list);
  WERD_RES *combo = NULL;        // current combination of fuzzies
  WERD_RES *word_res;            // current word
  WERD *copy_word;

  char_count = 0;
  rej_count = 0;
  whole_word_rej_count = 0;

  row = the_row;
  if (right_to_left) {
    word_it.move_to_last();
    for (word_it.mark_cycle_pt(); !word_it.cycled_list();  word_it.backward()) {
      word_res = new WERD_RES(word_it.data());
      word_res->x_height = the_row->x_height();
      // A FUZZY_NON marks the beginning of a combo if we are not in one.
      if (combo == NULL && word_res->word->flag(W_FUZZY_NON)) {
        copy_word = new WERD;
                                 //deep copy
        *copy_word = *(word_it.data());
        combo = new WERD_RES(copy_word);
        combo->x_height = the_row->x_height();
        combo->combination = TRUE;
        word_res_it.add_to_end(combo);
        word_res->part_of_combo = TRUE;
      } else if (combo != NULL) {
        word_res->part_of_combo = TRUE;
        combo->copy_on(word_res);
        // The first non FUZZY_NON is the last word in the combo.
        if (!word_res->word->flag(W_FUZZY_NON))
          combo = NULL;
      }
      word_res_it.add_to_end(word_res);
    }
  } else {
    for (word_it.mark_cycle_pt(); !word_it.cycled_list(); word_it.forward()) {
      word_res = new WERD_RES(word_it.data());
      word_res->x_height = the_row->x_height();

      if (word_res->word->flag(W_FUZZY_NON)) {
        ASSERT_HOST(combo != NULL);
        word_res->part_of_combo = TRUE;
        combo->copy_on(word_res);
      }
      if (word_it.data_relative(1)->flag(W_FUZZY_NON)) {
        if (combo == NULL) {
          copy_word = new WERD;
                                   //deep copy
          *copy_word = *(word_it.data());
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
  denorm = source.denorm;
  if (source.box_word != NULL)
    box_word = new tesseract::BoxWord(*source.box_word);
  best_state = source.best_state;
  correct_text = source.correct_text;

  if (source.best_choice != NULL) {
    best_choice = new WERD_CHOICE;
    *best_choice = *(source.best_choice);
    raw_choice = new WERD_CHOICE;
    *raw_choice = *(source.raw_choice);
    best_choice_fontinfo_ids = source.best_choice_fontinfo_ids;
  }
  else {
    best_choice = NULL;
    raw_choice = NULL;
    if (!best_choice_fontinfo_ids.empty()) {
      best_choice_fontinfo_ids.clear();
    }
  }
  if (source.ep_choice != NULL) {
    ep_choice = new WERD_CHOICE;
    *ep_choice = *(source.ep_choice);
  }
  else
    ep_choice = NULL;
  reject_map = source.reject_map;
  combination = source.combination;
  part_of_combo = source.part_of_combo;
  CopySimpleFields(source);
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
  italic = source.italic;
  bold = source.bold;
  fontinfo_id = source.fontinfo_id;
  fontinfo_id_count = source.fontinfo_id_count;
  fontinfo_id2 = source.fontinfo_id2;
  fontinfo_id2_count = source.fontinfo_id2_count;
  x_height = source.x_height;
  caps_height = source.caps_height;
  guessed_x_ht = source.guessed_x_ht;
  guessed_caps_ht = source.guessed_caps_ht;
  reject_spaces = source.reject_spaces;
}

// Sets up the members used in recognition:
// bln_boxes, chopped_word, seam_array, denorm, best_choice, raw_choice.
// Returns false if the word is empty and sets up fake results.
bool WERD_RES::SetupForRecognition(const UNICHARSET& unicharset,
                                   bool numeric_mode, ROW *row, BLOCK* block) {
  ClearResults();
  if (word->cblob_list()->empty()) {
    // Empty words occur when all the blobs have been moved to the rej_blobs
    // list, which seems to occur frequently in junk.
    chopped_word = new TWERD;
    rebuild_word = new TWERD;
    bln_boxes = new tesseract::BoxWord;
    box_word = new tesseract::BoxWord;
    best_choice = new WERD_CHOICE("", NULL, 10.0f, -1.0f,
                                        TOP_CHOICE_PERM, unicharset);
    raw_choice = new WERD_CHOICE("", NULL, 10.0f, -1.0f,
                                       TOP_CHOICE_PERM, unicharset);
    tess_failed = true;
    return false;
  }
  chopped_word = TWERD::PolygonalCopy(word);
  chopped_word->SetupBLNormalize(block, row, x_height, numeric_mode, &denorm);
  chopped_word->Normalize(denorm);
  bln_boxes = tesseract::BoxWord::CopyFromNormalized(NULL, chopped_word);
  seam_array = start_seam_list(chopped_word->blobs);
  best_choice = new WERD_CHOICE;
  best_choice->make_bad();
  raw_choice = new WERD_CHOICE;
  raw_choice->make_bad();
  return true;
}

// Builds the rebuild_word from the chopped_word and the best_state.
void WERD_RES::RebuildBestState() {
  if (rebuild_word != NULL)
    delete rebuild_word;
  rebuild_word = new TWERD;
  TBLOB* prev_blob = NULL;
  int start = 0;
  for (int i = 0; i < best_state.size(); ++i) {
    int length = best_state[i];
    join_pieces(chopped_word->blobs, seam_array, start, start + length - 1);
    TBLOB* blob = chopped_word->blobs;
    for (int i = 0; i < start; ++i)
      blob = blob->next;
    TBLOB* copy_blob = new TBLOB(*blob);
    if (prev_blob == NULL)
      rebuild_word->blobs = copy_blob;
    else
      prev_blob->next = copy_blob;
    prev_blob = copy_blob;
    break_pieces(blob, seam_array, start, start + length - 1);
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
  box_word = tesseract::BoxWord::CopyFromNormalized(&denorm, rebuild_word);
  box_word->ClipToOriginalWord(denorm.block(), word);
}

// Sets up the script positions in the output boxword using the best_choice
// to get the unichars, and the unicharset to get the target positions.
void WERD_RES::SetScriptPositions(const UNICHARSET& unicharset) {
  box_word->SetScriptPositions(unicharset, small_caps, rebuild_word,
                               best_choice);
}

// Classifies the word with some already-calculated BLOB_CHOICEs.
// The choices are an array of blob_count pointers to BLOB_CHOICE,
// providing a single classifier result for each blob.
// The BLOB_CHOICEs are consumed and the word takes ownership.
// The number of blobs in the outword must match blob_count.
void WERD_RES::FakeClassifyWord(const UNICHARSET& unicharset, int blob_count,
                                BLOB_CHOICE** choices) {
  // Setup the WERD_RES.
  ASSERT_HOST(box_word != NULL);
  ASSERT_HOST(blob_count == box_word->length());
  ASSERT_HOST(best_choice != NULL);
  BLOB_CHOICE_LIST_CLIST* word_choices = new BLOB_CHOICE_LIST_CLIST;
  BLOB_CHOICE_LIST_C_IT bc_it(word_choices);
  for (int c = 0; c < blob_count; ++c) {
    best_choice->append_unichar_id(
        choices[c]->unichar_id(), 1,
        choices[c]->rating(), choices[c]->certainty());
    BLOB_CHOICE_LIST* choice_list = new BLOB_CHOICE_LIST;
    BLOB_CHOICE_IT choice_it(choice_list);
    choice_it.add_after_then_move(choices[c]);
    bc_it.add_after_then_move(choice_list);
  }
  best_choice->set_blob_choices(word_choices);
  best_choice->populate_unichars(unicharset);
  delete raw_choice;
  raw_choice = new WERD_CHOICE(*best_choice);
  reject_map.initialise(blob_count);
}

// Copies the best_choice strings to the correct_text for adaption/training.
void WERD_RES::BestChoiceToCorrectText(const UNICHARSET& unicharset) {
  correct_text.clear();
  ASSERT_HOST(best_choice != NULL);
  for (int i = 0; i < best_choice->length(); ++i) {
    UNICHAR_ID choice_id = best_choice->unichar_id(i);
    const char* blob_choice = unicharset.id_to_unichar(choice_id);
    correct_text.push_back(STRING(blob_choice));
  }
}

// Merges 2 adjacent blobs in the result if the permanent callback
// class_cb returns other than INVALID_UNICHAR_ID, AND the permanent
// callback box_cb is NULL or returns true, setting the merged blob
// result to the class returned from class_cb.
// Returns true if anything was merged.
bool WERD_RES::ConditionalBlobMerge(
    const UNICHARSET& unicharset,
    TessResultCallback2<UNICHAR_ID, UNICHAR_ID, UNICHAR_ID>* class_cb,
    TessResultCallback2<bool, const TBOX&, const TBOX&>* box_cb,

    BLOB_CHOICE_LIST_CLIST *blob_choices) {
  bool modified = false;
  for (int i = 0; i + 1 < best_choice->length(); ++i) {
    UNICHAR_ID new_id = class_cb->Run(best_choice->unichar_id(i),
                                      best_choice->unichar_id(i+1));
    if (new_id != INVALID_UNICHAR_ID &&
        (box_cb == NULL || box_cb->Run(box_word->BlobBox(i),
                                       box_word->BlobBox(i + 1)))) {
      if (reject_map.length() == best_choice->length())
        reject_map.remove_pos(i);
      best_choice->set_unichar_id(new_id, i);
      best_choice->remove_unichar_id(i + 1);
      raw_choice->set_unichar_id(new_id, i);
      raw_choice->remove_unichar_id(i + 1);
      modified = true;
      rebuild_word->MergeBlobs(i, i + 2);
      box_word->MergeBoxes(i, i + 2);
      if (i + 1 < best_state.length()) {
        best_state[i] += best_state[i + 1];
        best_state.remove(i + 1);
      }

      BLOB_CHOICE_LIST_C_IT blob_choices_it(blob_choices);
      for (int j = 0; j < i; ++j)
        blob_choices_it.forward();
      BLOB_CHOICE_IT it1(blob_choices_it.data());            // first choices
      BLOB_CHOICE_LIST* target_choices = blob_choices_it.data_relative(1);
      BLOB_CHOICE_IT it2(target_choices);  // second choices
      float certainty = it2.data()->certainty();
      float rating = it2.data()->rating();
      if (it1.data()->certainty() < certainty) {
        certainty = it1.data()->certainty();
        rating = it1.data()->rating();
        target_choices = blob_choices_it.data();
        blob_choices_it.forward();
      }
      delete blob_choices_it.extract();  // get rid of spare
      // TODO(rays) Fix the choices so they contain the desired result.
      // Do we really need to ? Only needed for fix_quotes, which should be
      // going away.
    }
  }
  delete class_cb;
  delete box_cb;
  if (modified) {
    best_choice->populate_unichars(unicharset);
    raw_choice->populate_unichars(unicharset);
  }
  return modified;
}


WERD_RES::~WERD_RES () {
  Clear();
}

void WERD_RES::InitPointers() {
  word = NULL;
  bln_boxes = NULL;
  chopped_word = NULL;
  rebuild_word = NULL;
  box_word = NULL;
  seam_array = NULL;
  best_choice = NULL;
  raw_choice = NULL;
  ep_choice = NULL;
}

void WERD_RES::Clear() {
  if (word != NULL && combination)
    delete word;
  word = NULL;
  ClearResults();
}

void WERD_RES::ClearResults() {
  done = false;
  if (bln_boxes != NULL) {
    delete bln_boxes;
    bln_boxes = NULL;
  }
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
  if (seam_array != NULL) {
    free_seam_list(seam_array);
    seam_array = NULL;
  }
  if (best_choice != NULL) {
    delete best_choice;
    delete raw_choice;
    best_choice = NULL;
    raw_choice = NULL;
  }
  if (ep_choice != NULL) {
    delete ep_choice;
    ep_choice = NULL;
  }
}


// Inserts the new_word and a corresponding WERD_RES before the current
// position. The simple fields of the WERD_RES are copied from clone_res and
// the resulting WERD_RES is returned for further setup with best_choice etc.
WERD_RES* PAGE_RES_IT::InsertCloneWord(const WERD_RES& clone_res,
                                       WERD* new_word) {
  // Insert new_word into the ROW.
  WERD_IT w_it(row()->row->word_list());
  for (w_it.mark_cycle_pt(); !w_it.cycled_list(); w_it.forward()) {
    WERD* word = w_it.data();
    if (word == word_res->word)
      break;
  }
  ASSERT_HOST(!w_it.cycled_list());
  w_it.add_before_then_move(new_word);
  // Make a WERD_RES for the new_word.
  WERD_RES* new_res = new WERD_RES(new_word);
  new_res->CopySimpleFields(clone_res);
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
// anything, since operations on the current word will not have disturbed
// the word_res_it.
void PAGE_RES_IT::ResetWordIterator() {
  if (row_res == next_row_res) {
    // Reset the member iterator so it can move forward and detect the
    // cycled_list state correctly.
    word_res_it.move_to_first();
    word_res_it.mark_cycle_pt();
    while (!word_res_it.cycled_list() && word_res_it.data() != next_word_res)
      word_res_it.forward();
    ASSERT_HOST(!word_res_it.cycled_list());
    word_res_it.forward();
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
