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

const char * const kIncorrectResultReasonNames[] = {
    kBlameCorrect,
    kBlameClassifier,
    kBlameChopper,
    kBlameClassLMTradeoff,
    kBlamePageLayout,
    kBlameSegsearchHeur,
    kBlameSegsearchPP,
    kBlameClassOldLMTradeoff,
    kBlameAdaption,
    kBlameNoTruthSplit,
    kBlameNoTruth,
    kBlameUnknown
};

const char *BlamerBundle::IncorrectReasonName(IncorrectResultReason irr) {
  return kIncorrectResultReasonNames[irr];
}

const char *BlamerBundle::IncorrectReason() const {
  return kIncorrectResultReasonNames[incorrect_result_reason];
}

void BlamerBundle::FillDebugString(const STRING &msg,
                                   const WERD_CHOICE *choice,
                                   STRING *debug) {
  (*debug) += "Truth ";
  for (int i = 0; i < this->truth_text.length(); ++i) {
    (*debug) += this->truth_text[i];
  }
  if (!this->truth_has_char_boxes) (*debug) += " (no char boxes)";
  if (choice != NULL) {
    (*debug) += " Choice ";
    STRING choice_str;
    choice->string_and_lengths(&choice_str, NULL);
    (*debug) += choice_str;
  }
  if (msg.length() > 0) {
    (*debug) += "\n";
    (*debug) += msg;
  }
  (*debug) += "\n";
}

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
  Init();
  BLOCK_IT block_it(the_block_list);
  BLOCK_RES_IT block_res_it(&block_res_list);
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
    row_res_it.add_to_end(new ROW_RES(row_it.data()));
  }
}


/*************************************************************************
 * ROW_RES::ROW_RES
 *
 * Constructor for ROW results
 *************************************************************************/

ROW_RES::ROW_RES(ROW *the_row) {
  WERD_IT word_it(the_row->word_list());
  WERD_RES_IT word_res_it(&word_res_list);
  WERD_RES *combo = NULL;        // current combination of fuzzies
  WERD_RES *word_res;            // current word
  WERD *copy_word;

  char_count = 0;
  rej_count = 0;
  whole_word_rej_count = 0;

  row = the_row;
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
    best_choice = new WERD_CHOICE(*source.best_choice);
    raw_choice = new WERD_CHOICE(*source.raw_choice);
    best_choice_fontinfo_ids = source.best_choice_fontinfo_ids;
  }
  else {
    best_choice = NULL;
    raw_choice = NULL;
    if (!best_choice_fontinfo_ids.empty()) {
      best_choice_fontinfo_ids.clear();
    }
  }
  for (int i = 0; i < source.alt_choices.length(); ++i) {
    const WERD_CHOICE *choice = source.alt_choices[i];
    ASSERT_HOST(choice != NULL);
    alt_choices.push_back(new WERD_CHOICE(*choice));
  }
  alt_states = source.alt_states;
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
  italic = source.italic;
  bold = source.bold;
  fontinfo = source.fontinfo;
  fontinfo2 = source.fontinfo2;
  fontinfo_id_count = source.fontinfo_id_count;
  fontinfo_id2_count = source.fontinfo_id2_count;
  x_height = source.x_height;
  caps_height = source.caps_height;
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

// Sets up the members used in recognition:
// bln_boxes, chopped_word, seam_array, denorm, best_choice, raw_choice.
// Returns false if the word is empty and sets up fake results.
bool WERD_RES::SetupForTessRecognition(const UNICHARSET& unicharset_in,
                                   tesseract::Tesseract* tess, Pix* pix,
                                   bool numeric_mode,
                                   bool use_body_size,
                                   ROW *row, BLOCK* block) {
  tesseract = tess;
  POLY_BLOCK* pb = block != NULL ? block->poly_block() : NULL;
  if (word->cblob_list()->empty() || (pb != NULL && !pb->IsText())) {
    // Empty words occur when all the blobs have been moved to the rej_blobs
    // list, which seems to occur frequently in junk.
    SetupFake(unicharset_in);
    word->set_flag(W_REP_CHAR, false);
    return false;
  }
  ClearResults();
  SetupWordScript(unicharset_in);
  chopped_word = TWERD::PolygonalCopy(word);
  if (use_body_size && row->body_size() > 0.0f) {
    chopped_word->SetupBLNormalize(block, row, row->body_size(),
                                   numeric_mode, &denorm);
  } else {
    chopped_word->SetupBLNormalize(block, row, x_height, numeric_mode, &denorm);
  }
  // The image will be 8-bit grey if the input was grey or color. Note that in
  // a grey image 0 is black and 255 is white. If the input was binary, then
  // the pix will be binary and 0 is white, with 1 being black.
  // To tell the difference pixGetDepth() will return 8 or 1.
  denorm.set_pix(pix);
  // The inverse flag will be true iff the word has been determined to be white
  // on black, and is independent of whether the pix is 8 bit or 1 bit.
  denorm.set_inverse(word->flag(W_INVERSE));
  chopped_word->Normalize(denorm);
  bln_boxes = tesseract::BoxWord::CopyFromNormalized(NULL, chopped_word);
  seam_array = start_seam_list(chopped_word->blobs);
  best_choice = new WERD_CHOICE(&unicharset_in);
  best_choice->make_bad();
  raw_choice = new WERD_CHOICE(&unicharset_in);
  raw_choice->make_bad();
  SetupBlamerBundle();
  return true;
}

// Sets up the members used in recognition:
// bln_boxes, chopped_word, seam_array, denorm, best_choice, raw_choice.
// Returns false if the word is empty and sets up fake results.
bool WERD_RES::SetupForCubeRecognition(const UNICHARSET& unicharset_in,
                                       tesseract::Tesseract* tess,
                                       const BLOCK* block) {
  tesseract = tess;
  POLY_BLOCK* pb = block != NULL ? block->poly_block() : NULL;
  if (pb != NULL && !pb->IsText()) {
    // Ignore words in graphic regions.
    SetupFake(unicharset_in);
    word->set_flag(W_REP_CHAR, false);
    return false;
  }
  ClearResults();
  SetupWordScript(unicharset_in);
  TBOX word_box = word->bounding_box();
  denorm.SetupNormalization(block, NULL, NULL, NULL, NULL, 0,
                            word_box.left(), word_box.bottom(),
                            1.0f, 1.0f, 0.0f, 0.0f);
  SetupBlamerBundle();
  return true;
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
  best_choice = new WERD_CHOICE("", NULL, 10.0f, -1.0f,
                                TOP_CHOICE_PERM, unicharset_in);
  raw_choice = new WERD_CHOICE("", NULL, 10.0f, -1.0f,
                               TOP_CHOICE_PERM, unicharset_in);
  if (blob_count > 0) {
    BLOB_CHOICE** fake_choices = new BLOB_CHOICE*[blob_count];
    // For non-text blocks, just pass any blobs through to the box_word
    // and call the word failed with a fake classification.
    C_BLOB_IT b_it(word->cblob_list());
    int blob_id = 0;
    for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
      TBOX box = b_it.data()->bounding_box();
      box_word->InsertBox(box_word->length(), box);
      fake_choices[blob_id++] = new BLOB_CHOICE(0, 10.0f, -1.0f,
                                                -1, -1, -1, 0, 0, false);
    }
    FakeClassifyWord(blob_count, fake_choices);
    delete [] fake_choices;
  }
  tess_failed = true;
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
    blamer_bundle->norm_box_tolerance = kBlamerBoxTolerance * denorm.x_scale();
    TPOINT topleft;
    TPOINT botright;
    TPOINT norm_topleft;
    TPOINT norm_botright;
    for (int b = 0; b < blamer_bundle->truth_word.length(); ++b) {
      const TBOX &box = blamer_bundle->truth_word.BlobBox(b);
      topleft.x = box.left();
      topleft.y = box.top();
      botright.x = box.right();
      botright.y = box.bottom();
      denorm.NormTransform(topleft, &norm_topleft);
      denorm.NormTransform(botright, &norm_botright);
      TBOX norm_box(norm_topleft.x, norm_botright.y,
                    norm_botright.x, norm_topleft.y);
      blamer_bundle->norm_truth_word.InsertBox(b, norm_box);
    }
  }
}

// Simple helper moves the ownership of the pointer data from src to dest,
// first deleting anything in dest, and nulling out src afterwards.
template<class T> static void MovePointerData(T** dest, T**src) {
  delete *dest;
  *dest = *src;
  *src = NULL;
}

// Moves the results fields from word to this. This takes ownership of all
// the data, so src can be destructed.
void WERD_RES::ConsumeWordResults(WERD_RES* word) {
  denorm = word->denorm;
  MovePointerData(&chopped_word, &word->chopped_word);
  MovePointerData(&rebuild_word, &word->rebuild_word);
  MovePointerData(&box_word, &word->box_word);
  if (seam_array != NULL)
    free_seam_list(seam_array);
  seam_array = word->seam_array;
  word->seam_array = NULL;
  best_state.move(&word->best_state);
  correct_text.move(&word->correct_text);
  MovePointerData(&best_choice, &word->best_choice);
  MovePointerData(&raw_choice, &word->raw_choice);
  alt_choices.delete_data_pointers();
  alt_choices.move(&word->alt_choices);
  alt_states.move(&word->alt_states);
  reject_map = word->reject_map;
  if (word->blamer_bundle != NULL) {
    assert(blamer_bundle != NULL);
    blamer_bundle->CopyResults(*(word->blamer_bundle));
  }
  CopySimpleFields(*word);
}

// Replace the best choice and rebuild box word.
void WERD_RES::ReplaceBestChoice(
    const WERD_CHOICE& choice,
    const GenericVector<int>& segmentation_state) {
  delete best_choice;
  best_choice = new WERD_CHOICE(choice);
  best_state = segmentation_state;
  RebuildBestState();
  SetupBoxWord();
  // Make up a fake reject map of the right length to keep the
  // rejection pass happy.
  reject_map.initialise(segmentation_state.length());
  done = tess_accepted = tess_would_adapt = true;
  SetScriptPositions();
}

// Builds the rebuild_word from the chopped_word and the best_state.
void WERD_RES::RebuildBestState() {
  if (rebuild_word != NULL)
    delete rebuild_word;
  rebuild_word = new TWERD;
  if (seam_array == NULL) {
    seam_array = start_seam_list(chopped_word->blobs);
  }
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
void WERD_RES::SetScriptPositions() {
  box_word->SetScriptPositions(*uch_set, small_caps, rebuild_word,
                               best_choice);
}

void WERD_RES::WithoutFootnoteSpan(int *pstart, int *pend) const {
  int end = best_choice->length();
  while (end > 0 &&
         uch_set->get_isdigit(best_choice->unichar_ids()[end - 1]) &&
         box_word->BlobPosition(end - 1) == tesseract::SP_SUPERSCRIPT) {
    end--;
  }
  int start = 0;
  while (start < end &&
         uch_set->get_isdigit(best_choice->unichar_ids()[start]) &&
         box_word->BlobPosition(start) == tesseract::SP_SUPERSCRIPT) {
    start++;
  }
  *pstart = start;
  *pend = end;
}

void WERD_RES::WithoutFootnoteSpan(
    const WERD_CHOICE &word, const GenericVector<int> &state,
    int *pstart, int *pend) const {
  int len = word.length();
  *pstart = 0;
  *pend = len;
  if (len < 2) return;
  if (!word.unicharset()->get_isdigit(word.unichar_ids()[len - 1]) &&
      !word.unicharset()->get_isdigit(word.unichar_ids()[0])) return;

  // ok, now that we know the word ends in digits, do the expensive bit of
  // figuring out if they're superscript.
  WERD_RES copy(*this);
  copy.ReplaceBestChoice(word, state);
  copy.WithoutFootnoteSpan(pstart, pend);
}

// Classifies the word with some already-calculated BLOB_CHOICEs.
// The choices are an array of blob_count pointers to BLOB_CHOICE,
// providing a single classifier result for each blob.
// The BLOB_CHOICEs are consumed and the word takes ownership.
// The number of blobs in the outword must match blob_count.
void WERD_RES::FakeClassifyWord(int blob_count, BLOB_CHOICE** choices) {
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
  delete raw_choice;
  raw_choice = new WERD_CHOICE(*best_choice);
  reject_map.initialise(blob_count);
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
  return modified;
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
void WERD_RES::fix_quotes(BLOB_CHOICE_LIST_CLIST* blob_choices) {
  if (!uch_set->contains_unichar("\"") ||
      !uch_set->get_enabled(uch_set->unichar_to_id("\"")))
    return;  // Don't create it if it is disallowed.

  ConditionalBlobMerge(
      NewPermanentTessCallback(this, &WERD_RES::BothQuotes),
      NULL,
      blob_choices);
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
void WERD_RES::fix_hyphens(BLOB_CHOICE_LIST_CLIST *blob_choices) {
  if (!uch_set->contains_unichar("-") ||
      !uch_set->get_enabled(uch_set->unichar_to_id("-")))
    return;  // Don't create it if it is disallowed.

  ConditionalBlobMerge(
      NewPermanentTessCallback(this, &WERD_RES::BothHyphens),
      NewPermanentTessCallback(this, &WERD_RES::HyphenBoxesOverlap),
      blob_choices);
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
      NewPermanentTessCallback(this, &WERD_RES::BothSpaces), NULL,
      best_choice->blob_choices())) {
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
    if (index >= 0 && index < array_count(seam_array)) {
      SEAM* seam = reinterpret_cast<SEAM *>(array_value(seam_array, index));
      if (seam != NULL && seam->split1 != NULL)
        return false;
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
  guessed_x_ht = TRUE;
  guessed_caps_ht = TRUE;
  combination = FALSE;
  part_of_combo = FALSE;
  reject_spaces = FALSE;
}

void WERD_RES::InitPointers() {
  word = NULL;
  bln_boxes = NULL;
  uch_set = NULL;
  chopped_word = NULL;
  rebuild_word = NULL;
  box_word = NULL;
  seam_array = NULL;
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
  if (!alt_choices.empty()) {
    alt_choices.delete_data_pointers();
    alt_choices.clear();
  }
  alt_states.clear();
  if (ep_choice != NULL) {
    delete ep_choice;
    ep_choice = NULL;
  }
  if (blamer_bundle != NULL) blamer_bundle->ClearResults();
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

// Inserts the new_word and a corresponding WERD_RES before the current
// position. The simple fields of the WERD_RES are copied from clone_res and
// the resulting WERD_RES is returned for further setup with best_choice etc.
WERD_RES* PAGE_RES_IT::InsertSimpleCloneWord(const WERD_RES& clone_res,
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
