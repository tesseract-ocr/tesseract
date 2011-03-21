/**********************************************************************
 * File:        pageres.h  (Formerly page_res.h)
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
#ifndef           PAGERES_H
#define           PAGERES_H

#include "blobs.h"
#include "boxword.h"
#include "elst.h"
#include "genericvector.h"
#include "normalis.h"
#include "ocrblock.h"
#include "ocrrow.h"
#include "ratngs.h"
#include "rejctmap.h"
#include "seam.h"
#include "werd.h"

/* Forward declarations */

class BLOCK_RES;

ELISTIZEH (BLOCK_RES) CLISTIZEH (BLOCK_RES)
class
ROW_RES;

ELISTIZEH (ROW_RES)
class WERD_RES;

ELISTIZEH (WERD_RES)

/*************************************************************************
 * PAGE_RES - Page results
 *************************************************************************/
class PAGE_RES {                 // page result
 public:
  inT32 char_count;
  inT32 rej_count;
  BLOCK_RES_LIST block_res_list;
  BOOL8 rejected;
  // Updated every time PAGE_RES_IT iterating on this PAGE_RES moves to
  // the next word. This pointer is not owned by PAGE_RES class.
  WERD_CHOICE **prev_word_best_choice;

  PAGE_RES() {
  }                            // empty constructor

  PAGE_RES(BLOCK_LIST *block_list,   // real blocks
           WERD_CHOICE **prev_word_best_choice_ptr);

  ~PAGE_RES () {               // destructor
  }
};

/*************************************************************************
 * BLOCK_RES - Block results
 *************************************************************************/

class BLOCK_RES:public ELIST_LINK {
 public:
  BLOCK * block;               // real block
  inT32 char_count;            // chars in block
  inT32 rej_count;             // rejected chars
  inT16 font_class;            //
  inT16 row_count;
  float x_height;
  BOOL8 font_assigned;         // block already
  //      processed
  BOOL8 bold;                  // all bold
  BOOL8 italic;                // all italic

  ROW_RES_LIST row_res_list;

  BLOCK_RES() {
  }                            // empty constructor

  BLOCK_RES(BLOCK *the_block);  // real block

  ~BLOCK_RES () {              // destructor
  }
};

/*************************************************************************
 * ROW_RES - Row results
 *************************************************************************/

class ROW_RES:public ELIST_LINK {
 public:
  ROW * row;                   // real row
  inT32 char_count;            // chars in block
  inT32 rej_count;             // rejected chars
  inT32 whole_word_rej_count;  // rejs in total rej wds
  WERD_RES_LIST word_res_list;

  ROW_RES() {
  }                            // empty constructor

  ROW_RES(bool right_to_left, ROW *the_row);  // real row

  ~ROW_RES() {                // destructor
  }
};

/*************************************************************************
 * WERD_RES - Word results
 *************************************************************************/
enum CRUNCH_MODE
{
  CR_NONE,
  CR_KEEP_SPACE,
  CR_LOOSE_SPACE,
  CR_DELETE
};

// WERD_RES is a collection of publicly accessible members that gathers
// information about a word result.
class WERD_RES : public ELIST_LINK {
 public:
  // Which word is which?
  // There are 3 coordinate spaces in use here: a possibly rotated pixel space,
  // the original image coordinate space, and the BLN space in which the
  // baseline of a word is at kBlnBaselineOffset, the xheight is kBlnXHeight,
  // and the x-middle of the word is at 0.
  // In the rotated pixel space, coordinates correspond to the input image,
  // but may be rotated about the origin by a multiple of 90 degrees,
  // and may therefore be negative.
  // In any case a rotation by denorm.block()->re_rotation() will take them
  // back to the original image.
  // The other differences between words all represent different stages of
  // processing.
  //
  // The word is the input C_BLOBs in the rotated pixel space.
  // word is NOT owned by the WERD_RES unless combination is true.
  // All the other word pointers ARE owned by the WERD_RES.
  WERD* word;                     // Input C_BLOB word.
  // The bln_boxes contains the bounding boxes (only) of the input word, in the
  // BLN space. The lengths of word and bln_boxes
  // match as they are both before any chopping.
  // TODO(rays) determine if docqual does anything useful and delete bln_boxes
  // if it doesn't.
  tesseract::BoxWord* bln_boxes;  // BLN input bounding boxes.
  // The chopped_word is also in BLN space, and represents the fully chopped
  // character fragments that make up the word.
  // The length of chopped_word matches length of seam_array + 1 (if set).
  TWERD* chopped_word;            // BLN chopped fragments output.
  SEAMS seam_array;               // Seams matching chopped_word.
  // The rebuild_word is also in BLN space, but represents the final best
  // segmentation of the word. Its length is therefore the same as box_word.
  TWERD* rebuild_word;            // BLN best segmented word.
  // The denorm provides the transformation to get back to the rotated image
  // coords from the chopped_word/rebuild_word BLN coords.
  DENORM denorm;                  // For use on chopped_word.
  // The box_word is in the original image coordinate space. It is the
  // bounding boxes of the rebuild_word, after denormalization.
  // The length of box_word matches rebuild_word, best_state (if set) and
  // correct_text (if set), as well as best_choice and represents the
  // number of classified units in the output.
  tesseract::BoxWord* box_word;   // Denormalized output boxes.
  // The best_state stores the relationship between chopped_word and
  // rebuild_word. Each blob[i] in rebuild_word is composed of best_state[i]
  // adjacent blobs in chopped_word. The seams in seam_array are hidden
  // within a rebuild_word blob and revealed between them.
  GenericVector<int> best_state;  // Number of blobs in each best blob.
  // The correct_text is used during training and adaption to carry the
  // text to the training system without the need for a unicharset. There
  // is one entry in the vector for each blob in rebuild_word and box_word.
  GenericVector<STRING> correct_text;
  // The truth_* fields below are used by the blamer to determine the source
  // of errors.
  // The truth_word (in the original image coordinate space) contains ground
  // truth bounding boxes for this WERD_RES.
  tesseract::BoxWord* truth_word;
  // The truth_text contains ground truth unichar for each
  // of the bounding boxes in truth_word.
  GenericVector<STRING> truth_text;
  WERD_CHOICE *best_choice;    // tess output
  WERD_CHOICE *raw_choice;     // top choice permuter
  WERD_CHOICE *ep_choice;      // ep text TODO(rays) delete this.
  REJMAP reject_map;           // best_choice rejects
  BOOL8 tess_failed;
  /*
    If tess_failed is TRUE, one of the following tests failed when Tess
    returned:
    - The outword blob list was not the same length as the best_choice string;
    - The best_choice string contained ALL blanks;
    - The best_choice string was zero length
  */
  BOOL8 tess_accepted;          // Tess thinks its ok?
  BOOL8 tess_would_adapt;       // Tess would adapt?
  BOOL8 done;                   // ready for output?
  bool small_caps;             // word appears to be small caps
  inT8 italic;
  inT8 bold;
  inT16 fontinfo_id;            // primary font id (should be at least inT16)
  inT8 fontinfo_id_count;       // number of votes
  inT16 fontinfo_id2;           // secondary font id (should be at least inT16)
  inT8 fontinfo_id2_count;      // number of votes
  CRUNCH_MODE unlv_crunch_mode;
  float x_height;              // post match estimate
  float caps_height;           // post match estimate
  BOOL8 guessed_x_ht;
  BOOL8 guessed_caps_ht;
  /*
    To deal with fuzzy spaces we need to be able to combine "words" to form
    combinations when we suspect that the gap is a non-space. The (new) text
    ord code generates separate words for EVERY fuzzy gap - flags in the word
    indicate whether the gap is below the threshold (fuzzy kern) and is thus
    NOT a real word break by default, or above the threshold (fuzzy space) and
    this is a real word break by default.

    The WERD_RES list contains all these words PLUS "combination" words built
    out of (copies of) the words split by fuzzy kerns. The separate parts have
    their "part_of_combo" flag set true and should be IGNORED on a default
    reading of the list.

    Combination words are FOLLOWED by the sequence of part_of_combo words
    which they combine.
  */
  BOOL8 combination;           //of two fuzzy gap wds
  BOOL8 part_of_combo;         //part of a combo
  BOOL8 reject_spaces;         //Reject spacing?
  // FontInfo ids for each unichar in best_choice.
  GenericVector<inT8> best_choice_fontinfo_ids;

  WERD_RES() {
    InitPointers();
  }
  WERD_RES(                   //simple constructor
           WERD *the_word) {  //real word
    InitPointers();
    word = the_word;
    tess_failed = FALSE;
    tess_accepted = FALSE;
    tess_would_adapt = FALSE;
    done = FALSE;
    unlv_crunch_mode = CR_NONE;
    small_caps = false;
    italic = FALSE;
    bold = FALSE;
    fontinfo_id = -1;
    fontinfo_id_count = 0;
    fontinfo_id2 = -1;
    fontinfo_id2_count = 0;
    x_height = 0.0;
    caps_height = 0.0;
    guessed_x_ht = TRUE;
    guessed_caps_ht = TRUE;
    combination = FALSE;
    part_of_combo = FALSE;
    reject_spaces = FALSE;
  }
  WERD_RES(const WERD_RES &source) {
    InitPointers();
    *this = source;            // see operator=
  }

  ~WERD_RES();
  void InitPointers();
  void Clear();
  void ClearResults();

  WERD_RES& operator=(const WERD_RES& source);  //from this

  void CopySimpleFields(const WERD_RES& source);

  // Sets up the members used in recognition:
  // bln_boxes, chopped_word, seam_array, denorm, best_choice, raw_choice.
  // Returns false if the word is empty and sets up fake results.
  bool SetupForRecognition(const UNICHARSET& unicharset,
                           bool numeric_mode, ROW *row, BLOCK* block);

  // Builds the rebuild_word from the chopped_word and the best_state.
  void RebuildBestState();

  // Copies the chopped_word to the rebuild_word, faking a best_state as well.
  // Also sets up the output box_word.
  void CloneChoppedToRebuild();

  // Sets/replaces the box_word with one made from the rebuild_word.
  void SetupBoxWord();

  // Sets up the script positions in the output boxword using the best_choice
  // to get the unichars, and the unicharset to get the target positions.
  void SetScriptPositions(const UNICHARSET& unicharset);

  // Classifies the word with some already-calculated BLOB_CHOICEs.
  // The choices are an array of blob_count pointers to BLOB_CHOICE,
  // providing a single classifier result for each blob.
  // The BLOB_CHOICEs are consumed and the word takes ownership.
  // The number of blobs in the outword must match blob_count.
  void FakeClassifyWord(const UNICHARSET& unicharset, int blob_count,
                        BLOB_CHOICE** choices);

  // Copies the best_choice strings to the correct_text for adaption/training.
  void BestChoiceToCorrectText(const UNICHARSET& unicharset);

  // Merges 2 adjacent blobs in the result if the permanent callback
  // class_cb returns other than INVALID_UNICHAR_ID, AND the permanent
  // callback box_cb is NULL or returns true, setting the merged blob
  // result to the class returned from class_cb.
  // Returns true if anything was merged.
  bool ConditionalBlobMerge(
      const UNICHARSET& unicharset,
      TessResultCallback2<UNICHAR_ID, UNICHAR_ID, UNICHAR_ID>* class_cb,
      TessResultCallback2<bool, const TBOX&, const TBOX&>* box_cb,
      BLOB_CHOICE_LIST_CLIST *blob_choices);

  static WERD_RES* deep_copy(const WERD_RES* src) {
    return new WERD_RES(*src);
  }

  // Copy blobs from word_res onto this word (eliminating spaces between).
  // Since this may be called bidirectionally OR both the BOL and EOL flags.
  void copy_on(WERD_RES *word_res) {  //from this word
    word->set_flag(W_BOL, word->flag(W_BOL) || word_res->word->flag(W_BOL));
    word->set_flag(W_EOL, word->flag(W_EOL) || word_res->word->flag(W_EOL));
    word->copy_on(word_res->word);
  }
};

/*************************************************************************
 * PAGE_RES_IT - Page results iterator
 *************************************************************************/

class PAGE_RES_IT {
 public:
  PAGE_RES * page_res;         // page being iterated

  PAGE_RES_IT() {
  }                            // empty contructor

  PAGE_RES_IT(PAGE_RES *the_page_res) {    // page result
    page_res = the_page_res;
    restart_page();  // ready to scan
  }

  WERD_RES *restart_page() {
    return start_page(false);  // Skip empty blocks.
  }
  WERD_RES *restart_page_with_empties() {
    return start_page(true);  // Allow empty blocks.
  }
  WERD_RES *start_page(bool empty_ok);

  // ============ Methods that mutate the underling structures ===========
  // Note that these methods will potentially invalidate other PAGE_RES_ITs
  // and are intended to be used only while a single PAGE_RES_IT is  active.
  // This problem needs to be taken into account if these mutation operators
  // are ever provided to PageIterator or its subclasses.

  // Inserts the new_word and a corresponding WERD_RES before the current
  // position. The simple fields of the WERD_RES are copied from clone_res and
  // the resulting WERD_RES is returned for further setup with best_choice etc.
  WERD_RES* InsertCloneWord(const WERD_RES& clone_res, WERD* new_word);

  // Deletes the current WERD_RES and its underlying WERD.
  void DeleteCurrentWord();

  WERD_RES *forward() {  // Get next word.
    return internal_forward(false, false);
  }
  // Move forward, but allow empty blocks to show as single NULL words.
  WERD_RES *forward_with_empties() {
    return internal_forward(false, true);
  }

  WERD_RES *forward_block();  // get first word in
  // next non-empty block
  WERD_RES *prev_word() const {  // previous word
    return prev_word_res;
  }
  ROW_RES *prev_row() const {  // row of prev word
    return prev_row_res;
  }
  BLOCK_RES *prev_block() const {  // block of prev word
    return prev_block_res;
  }
  WERD_RES *word() const {  // current word
    return word_res;
  }
  ROW_RES *row() const {  // row of current word
    return row_res;
  }
  BLOCK_RES *block() const {  // block of cur. word
    return block_res;
  }
  WERD_RES *next_word() const {  // next word
    return next_word_res;
  }
  ROW_RES *next_row() const {  // row of next word
    return next_row_res;
  }
  BLOCK_RES *next_block() const {  // block of next word
    return next_block_res;
  }
  void rej_stat_word();  // for page/block/row

 private:
  void ResetWordIterator();
  WERD_RES *internal_forward(bool new_block, bool empty_ok);

  WERD_RES * prev_word_res;    // previous word
  ROW_RES *prev_row_res;       // row of prev word
  BLOCK_RES *prev_block_res;   // block of prev word

  WERD_RES *word_res;          // current word
  ROW_RES *row_res;            // row of current word
  BLOCK_RES *block_res;        // block of cur. word

  WERD_RES *next_word_res;     // next word
  ROW_RES *next_row_res;       // row of next word
  BLOCK_RES *next_block_res;   // block of next word

  BLOCK_RES_IT block_res_it;   // iterators
  ROW_RES_IT row_res_it;
  WERD_RES_IT word_res_it;
};
#endif
