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

#include "blamer.h"
#include "blobs.h"
#include "boxword.h"
#include "elst.h"
#include "genericvector.h"
#include "normalis.h"
#include "ocrblock.h"
#include "ocrrow.h"
#include "params_training_featdef.h"
#include "ratngs.h"
#include "rejctmap.h"
#include "seam.h"
#include "werd.h"

namespace tesseract {
struct FontInfo;
class Tesseract;
}
using tesseract::FontInfo;

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
  // Sums of blame reasons computed by the blamer.
  GenericVector<int> blame_reasons;
  // Debug information about all the misadaptions on this page.
  // Each BlamerBundle contains an index into this vector, so that words that
  // caused misadaption could be marked. However, since words could be
  // deleted/split/merged, the log is stored on the PAGE_RES level.
  GenericVector<STRING> misadaption_log;

  inline void Init() {
    char_count = 0;
    rej_count = 0;
    rejected = FALSE;
    prev_word_best_choice = NULL;
    blame_reasons.init_to_size(IRR_NUM_REASONS, 0);
  }

  PAGE_RES() { Init(); }  // empty constructor

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

  ROW_RES(ROW *the_row);  // real row

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
  // processing during recognition.

  // ---------------------------INPUT-------------------------------------

  // The word is the input C_BLOBs in the rotated pixel space.
  // word is NOT owned by the WERD_RES unless combination is true.
  // All the other word pointers ARE owned by the WERD_RES.
  WERD* word;                     // Input C_BLOB word.

  // -------------SETUP BY SetupFor*Recognition---READONLY-INPUT------------

  // The bln_boxes contains the bounding boxes (only) of the input word, in the
  // BLN space. The lengths of word and bln_boxes
  // match as they are both before any chopping.
  // TODO(rays) determine if docqual does anything useful and delete bln_boxes
  // if it doesn't.
  tesseract::BoxWord* bln_boxes;  // BLN input bounding boxes.
  // The ROW that this word sits in. NOT owned by the WERD_RES.
  ROW* blob_row;
  // The denorm provides the transformation to get back to the rotated image
  // coords from the chopped_word/rebuild_word BLN coords, but each blob also
  // has its own denorm.
  DENORM denorm;                  // For use on chopped_word.
  // Unicharset used by the classifier output in best_choice and raw_choice.
  const UNICHARSET* uch_set;  // For converting back to utf8.

  // ----Initialized by SetupFor*Recognition---BUT OUTPUT FROM RECOGNITION----
  // ----Setup to a (different!) state expected by the various classifiers----
  // TODO(rays) Tidy and make more consistent.

  // The chopped_word is also in BLN space, and represents the fully chopped
  // character fragments that make up the word.
  // The length of chopped_word matches length of seam_array + 1 (if set).
  TWERD* chopped_word;            // BLN chopped fragments output.
  // Vector of SEAM* holding chopping points matching chopped_word.
  GenericVector<SEAM*> seam_array;
  // Widths of blobs in chopped_word.
  GenericVector<int> blob_widths;
  // Gaps between blobs in chopped_word. blob_gaps[i] is the gap between
  // blob i and blob i+1.
  GenericVector<int> blob_gaps;
  // Ratings matrix contains classifier choices for each classified combination
  // of blobs. The dimension is the same as the number of blobs in chopped_word
  // and the leading diagonal corresponds to classifier results of the blobs
  // in chopped_word. The state_ members of best_choice, raw_choice and
  // best_choices all correspond to this ratings matrix and allow extraction
  // of the blob choices for any given WERD_CHOICE.
  MATRIX* ratings;                // Owned pointer.
  // Pointer to the first WERD_CHOICE in best_choices. This is the result that
  // will be output from Tesseract. Note that this is now a borrowed pointer
  // and should NOT be deleted.
  WERD_CHOICE* best_choice;       // Borrowed pointer.
  // The best raw_choice found during segmentation search. Differs from the
  // best_choice by being the best result according to just the character
  // classifier, not taking any language model information into account.
  // Unlike best_choice, the pointer IS owned by this WERD_RES.
  WERD_CHOICE* raw_choice;        // Owned pointer.
  // Alternative results found during chopping/segmentation search stages.
  // Note that being an ELIST, best_choices owns the WERD_CHOICEs.
  WERD_CHOICE_LIST best_choices;

  // Truth bounding boxes, text and incorrect choice reason.
  BlamerBundle *blamer_bundle;

  // --------------OUTPUT FROM RECOGNITION-------------------------------
  // --------------Not all fields are necessarily set.-------------------
  // ---best_choice, raw_choice *must* end up set, with a box_word-------
  // ---In complete output, the number of blobs in rebuild_word matches---
  // ---the number of boxes in box_word, the number of unichar_ids in---
  // ---best_choice, the number of ints in best_state, and the number---
  // ---of strings in correct_text--------------------------------------
  // ---SetupFake Sets everything to appropriate values if the word is---
  // ---known to be bad before recognition.------------------------------

  // The rebuild_word is also in BLN space, but represents the final best
  // segmentation of the word. Its length is therefore the same as box_word.
  TWERD* rebuild_word;            // BLN best segmented word.
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
  // The Tesseract that was used to recognize this word. Just a borrowed
  // pointer. Note: Tesseract's class definition is in a higher-level library.
  // We avoid introducing a cyclic dependency by not using the Tesseract
  // within WERD_RES. We are just storing it to provide access to it
  // for the top-level multi-language controller, and maybe for output of
  // the recognized language.
  tesseract::Tesseract* tesseract;

  // Less-well documented members.
  // TODO(rays) Add more documentation here.
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
  // The fontinfos are pointers to data owned by the classifier.
  const FontInfo* fontinfo;
  const FontInfo* fontinfo2;
  inT8 fontinfo_id_count;       // number of votes
  inT8 fontinfo_id2_count;      // number of votes
  BOOL8 guessed_x_ht;
  BOOL8 guessed_caps_ht;
  CRUNCH_MODE unlv_crunch_mode;
  float x_height;              // post match estimate
  float caps_height;           // post match estimate

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
    InitNonPointers();
    InitPointers();
  }
  WERD_RES(WERD *the_word) {
    InitNonPointers();
    InitPointers();
    word = the_word;
  }
  // Deep copies everything except the ratings MATRIX.
  // To get that use deep_copy below.
  WERD_RES(const WERD_RES &source) {
    InitPointers();
    *this = source;            // see operator=
  }

  ~WERD_RES();

  // Returns the UTF-8 string for the given blob index in the best_choice word,
  // given that we know whether we are in a right-to-left reading context.
  // This matters for mirrorable characters such as parentheses.  We recognize
  // characters purely based on their shape on the page, and by default produce
  // the corresponding unicode for a left-to-right context.
  const char* const BestUTF8(int blob_index, bool in_rtl_context) const {
    if (blob_index < 0 || best_choice == NULL ||
        blob_index >= best_choice->length())
      return NULL;
    UNICHAR_ID id = best_choice->unichar_id(blob_index);
    if (id < 0 || id >= uch_set->size() || id == INVALID_UNICHAR_ID)
      return NULL;
    UNICHAR_ID mirrored = uch_set->get_mirror(id);
    if (in_rtl_context && mirrored > 0 && mirrored != INVALID_UNICHAR_ID)
      id = mirrored;
    return uch_set->id_to_unichar_ext(id);
  }
  // Returns the UTF-8 string for the given blob index in the raw_choice word.
  const char* const RawUTF8(int blob_index) const {
    if (blob_index < 0 || blob_index >= raw_choice->length())
      return NULL;
    UNICHAR_ID id = raw_choice->unichar_id(blob_index);
    if (id < 0 || id >= uch_set->size() || id == INVALID_UNICHAR_ID)
      return NULL;
    return uch_set->id_to_unichar(id);
  }

  UNICHARSET::Direction SymbolDirection(int blob_index) const {
    if (best_choice == NULL ||
        blob_index >= best_choice->length() ||
        blob_index < 0)
      return UNICHARSET::U_OTHER_NEUTRAL;
    return uch_set->get_direction(best_choice->unichar_id(blob_index));
  }

  bool AnyRtlCharsInWord() const {
    if (uch_set == NULL || best_choice == NULL || best_choice->length() < 1)
      return false;
    for (int id = 0; id < best_choice->length(); id++) {
      int unichar_id = best_choice->unichar_id(id);
      if (unichar_id < 0 || unichar_id >= uch_set->size())
        continue;  // Ignore illegal chars.
      UNICHARSET::Direction dir =
          uch_set->get_direction(unichar_id);
      if (dir == UNICHARSET::U_RIGHT_TO_LEFT ||
          dir == UNICHARSET::U_RIGHT_TO_LEFT_ARABIC ||
          dir == UNICHARSET::U_ARABIC_NUMBER)
        return true;
    }
    return false;
  }

  bool AnyLtrCharsInWord() const {
    if (uch_set == NULL || best_choice == NULL || best_choice->length() < 1)
      return false;
    for (int id = 0; id < best_choice->length(); id++) {
      int unichar_id = best_choice->unichar_id(id);
      if (unichar_id < 0 || unichar_id >= uch_set->size())
        continue;  // Ignore illegal chars.
      UNICHARSET::Direction dir = uch_set->get_direction(unichar_id);
      if (dir == UNICHARSET::U_LEFT_TO_RIGHT)
        return true;
    }
    return false;
  }

  // Return whether the blobs in this WERD_RES 0, 1,... come from an engine
  // that gave us the unichars in reading order (as opposed to strict left
  // to right).
  bool UnicharsInReadingOrder() const {
    return best_choice->unichars_in_script_order();
  }

  void InitNonPointers();
  void InitPointers();
  void Clear();
  void ClearResults();
  void ClearWordChoices();
  void ClearRatings();

  // Deep copies everything except the ratings MATRIX.
  // To get that use deep_copy below.
  WERD_RES& operator=(const WERD_RES& source);  //from this

  void CopySimpleFields(const WERD_RES& source);

  // Initializes a blank (default constructed) WERD_RES from one that has
  // already been recognized.
  // Use SetupFor*Recognition afterwards to complete the setup and make
  // it ready for a retry recognition.
  void InitForRetryRecognition(const WERD_RES& source);

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
  // The norm_mode sets the default mode for normalization in absence
  // of any of the above flags. It should really be a tesseract::OcrEngineMode
  // but is declared as int for ease of use with tessedit_ocr_engine_mode.
  // Returns false if the word is empty and sets up fake results.
  bool SetupForRecognition(const UNICHARSET& unicharset_in,
                           tesseract::Tesseract* tesseract, Pix* pix,
                           int norm_mode,
                           const TBOX* norm_box, bool numeric_mode,
                           bool use_body_size, bool allow_detailed_fx,
                           ROW *row, const BLOCK* block);

  // Set up the seam array, bln_boxes, best_choice, and raw_choice to empty
  // accumulators from a made chopped word.  We presume the fields are already
  // empty.
  void SetupBasicsFromChoppedWord(const UNICHARSET &unicharset_in);

  // Sets up the members used in recognition for an empty recognition result:
  // bln_boxes, chopped_word, seam_array, denorm, best_choice, raw_choice.
  void SetupFake(const UNICHARSET& uch);

  // Set the word as having the script of the input unicharset.
  void SetupWordScript(const UNICHARSET& unicharset_in);

  // Sets up the blamer_bundle if it is not null, using the initialized denorm.
  void SetupBlamerBundle();

  // Computes the blob_widths and blob_gaps from the chopped_word.
  void SetupBlobWidthsAndGaps();

  // Updates internal data to account for a new SEAM (chop) at the given
  // blob_number. Fixes the ratings matrix and states in the choices, as well
  // as the blob widths and gaps.
  void InsertSeam(int blob_number, SEAM* seam);

  // Returns true if all the word choices except the first have adjust_factors
  // worse than the given threshold.
  bool AlternativeChoiceAdjustmentsWorseThan(float threshold) const;

  // Returns true if the current word is ambiguous (by number of answers or
  // by dangerous ambigs.)
  bool IsAmbiguous();

  // Returns true if the ratings matrix size matches the sum of each of the
  // segmentation states.
  bool StatesAllValid();

  // Prints a list of words found if debug is true or the word result matches
  // the word_to_debug.
  void DebugWordChoices(bool debug, const char* word_to_debug);

  // Removes from best_choices all choices which are not within a reasonable
  // range of the best choice.
  void FilterWordChoices(int debug_level);

  // Computes a set of distance thresholds used to control adaption.
  // Compares the best choice for the current word to the best raw choice
  // to determine which characters were classified incorrectly by the
  // classifier. Then places a separate threshold into thresholds for each
  // character in the word. If the classifier was correct, max_rating is placed
  // into thresholds. If the classifier was incorrect, the mean match rating
  // (error percentage) of the classifier's incorrect choice minus some margin
  // is placed into thresholds. This can then be used by the caller to try to
  // create a new template for the desired class that will classify the
  // character with a rating better than the threshold value. The match rating
  // placed into thresholds is never allowed to be below min_rating in order to
  // prevent trying to make overly tight templates.
  // min_rating limits how tight to make a template.
  // max_rating limits how loose to make a template.
  // rating_margin denotes the amount of margin to put in template.
  void ComputeAdaptionThresholds(float certainty_scale,
                                 float min_rating,
                                 float max_rating,
                                 float rating_margin,
                                 float* thresholds);

  // Saves a copy of the word_choice if it has the best unadjusted rating.
  // Returns true if the word_choice was the new best.
  bool LogNewRawChoice(WERD_CHOICE* word_choice);
  // Consumes word_choice by adding it to best_choices, (taking ownership) if
  // the certainty for word_choice is some distance of the best choice in
  // best_choices, or by deleting the word_choice and returning false.
  // The best_choices list is kept in sorted order by rating. Duplicates are
  // removed, and the list is kept no longer than max_num_choices in length.
  // Returns true if the word_choice is still a valid pointer.
  bool LogNewCookedChoice(int max_num_choices, bool debug,
                          WERD_CHOICE* word_choice);

  // Prints a brief list of all the best choices.
  void PrintBestChoices() const;

  // Returns the sum of the widths of the blob between start_blob and last_blob
  // inclusive.
  int GetBlobsWidth(int start_blob, int last_blob);
  // Returns the width of a gap between the specified blob and the next one.
  int GetBlobsGap(int blob_index);

  // Returns the BLOB_CHOICE corresponding to the given index in the
  // best choice word taken from the appropriate cell in the ratings MATRIX.
  // Borrowed pointer, so do not delete. May return NULL if there is no
  // BLOB_CHOICE matching the unichar_id at the given index.
  BLOB_CHOICE* GetBlobChoice(int index) const;

  // Returns the BLOB_CHOICE_LIST corresponding to the given index in the
  // best choice word taken from the appropriate cell in the ratings MATRIX.
  // Borrowed pointer, so do not delete.
  BLOB_CHOICE_LIST* GetBlobChoices(int index) const;

  // Moves the results fields from word to this. This takes ownership of all
  // the data, so src can be destructed.
  // word1.ConsumeWordResult(word);
  // delete word;
  // is simpler and faster than:
  // word1 = *word;
  // delete word;
  // as it doesn't need to copy and reallocate anything.
  void ConsumeWordResults(WERD_RES* word);

  // Replace the best choice and rebuild box word.
  // choice must be from the current best_choices list.
  void ReplaceBestChoice(WERD_CHOICE* choice);

  // Builds the rebuild_word and sets the best_state from the chopped_word and
  // the best_choice->state.
  void RebuildBestState();

  // Copies the chopped_word to the rebuild_word, faking a best_state as well.
  // Also sets up the output box_word.
  void CloneChoppedToRebuild();

  // Sets/replaces the box_word with one made from the rebuild_word.
  void SetupBoxWord();

  // Sets up the script positions in the best_choice using the best_choice
  // to get the unichars, and the unicharset to get the target positions.
  void SetScriptPositions();
  // Sets all the blobs in all the words (best choice and alternates) to be
  // the given position. (When a sub/superscript is recognized as a separate
  // word, it falls victim to the rule that a whole word cannot be sub or
  // superscript, so this function overrides that problem.)
  void SetAllScriptPositions(tesseract::ScriptPos position);

  // Classifies the word with some already-calculated BLOB_CHOICEs.
  // The choices are an array of blob_count pointers to BLOB_CHOICE,
  // providing a single classifier result for each blob.
  // The BLOB_CHOICEs are consumed and the word takes ownership.
  // The number of blobs in the box_word must match blob_count.
  void FakeClassifyWord(int blob_count, BLOB_CHOICE** choices);

  // Creates a WERD_CHOICE for the word using the top choices from the leading
  // diagonal of the ratings matrix.
  void FakeWordFromRatings();

  // Copies the best_choice strings to the correct_text for adaption/training.
  void BestChoiceToCorrectText();

  // Merges 2 adjacent blobs in the result if the permanent callback
  // class_cb returns other than INVALID_UNICHAR_ID, AND the permanent
  // callback box_cb is NULL or returns true, setting the merged blob
  // result to the class returned from class_cb.
  // Returns true if anything was merged.
  bool ConditionalBlobMerge(
      TessResultCallback2<UNICHAR_ID, UNICHAR_ID, UNICHAR_ID>* class_cb,
      TessResultCallback2<bool, const TBOX&, const TBOX&>* box_cb);

  // Merges 2 adjacent blobs in the result (index and index+1) and corrects
  // all the data to account for the change.
  void MergeAdjacentBlobs(int index);

  // Callback helper for fix_quotes returns a double quote if both
  // arguments are quote, otherwise INVALID_UNICHAR_ID.
  UNICHAR_ID BothQuotes(UNICHAR_ID id1, UNICHAR_ID id2);
  void fix_quotes();

  // Callback helper for fix_hyphens returns UNICHAR_ID of - if both
  // arguments are hyphen, otherwise INVALID_UNICHAR_ID.
  UNICHAR_ID BothHyphens(UNICHAR_ID id1, UNICHAR_ID id2);
  // Callback helper for fix_hyphens returns true if box1 and box2 overlap
  // (assuming both on the same textline, are in order and a chopped em dash.)
  bool HyphenBoxesOverlap(const TBOX& box1, const TBOX& box2);
  void fix_hyphens();

  // Callback helper for merge_tess_fails returns a space if both
  // arguments are space, otherwise INVALID_UNICHAR_ID.
  UNICHAR_ID BothSpaces(UNICHAR_ID id1, UNICHAR_ID id2);
  void merge_tess_fails();

  // Returns a really deep copy of *src, including the ratings MATRIX.
  static WERD_RES* deep_copy(const WERD_RES* src) {
    WERD_RES* result = new WERD_RES(*src);
    // That didn't copy the ratings, but we want a copy if there is one to
    // begin width.
    if (src->ratings != NULL)
      result->ratings = src->ratings->DeepCopy();
    return result;
  }

  // Copy blobs from word_res onto this word (eliminating spaces between).
  // Since this may be called bidirectionally OR both the BOL and EOL flags.
  void copy_on(WERD_RES *word_res) {  //from this word
    word->set_flag(W_BOL, word->flag(W_BOL) || word_res->word->flag(W_BOL));
    word->set_flag(W_EOL, word->flag(W_EOL) || word_res->word->flag(W_EOL));
    word->copy_on(word_res->word);
  }

  // Returns true if the collection of count pieces, starting at start, are all
  // natural connected components, ie there are no real chops involved.
  bool PiecesAllNatural(int start, int count) const;
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

  // Do two PAGE_RES_ITs point at the same word?
  // This is much cheaper than cmp().
  bool operator ==(const PAGE_RES_IT &other) const;

  bool operator !=(const PAGE_RES_IT &other) const {return !(*this == other); }

  // Given another PAGE_RES_IT to the same page,
  //  this before other:     -1
  //  this equal to other:    0
  //  this later than other:  1
  int cmp(const PAGE_RES_IT &other) const;

  WERD_RES *restart_page() {
    return start_page(false);  // Skip empty blocks.
  }
  WERD_RES *restart_page_with_empties() {
    return start_page(true);  // Allow empty blocks.
  }
  WERD_RES *start_page(bool empty_ok);

  WERD_RES *restart_row();

  // ============ Methods that mutate the underling structures ===========
  // Note that these methods will potentially invalidate other PAGE_RES_ITs
  // and are intended to be used only while a single PAGE_RES_IT is  active.
  // This problem needs to be taken into account if these mutation operators
  // are ever provided to PageIterator or its subclasses.

  // Inserts the new_word and a corresponding WERD_RES before the current
  // position. The simple fields of the WERD_RES are copied from clone_res and
  // the resulting WERD_RES is returned for further setup with best_choice etc.
  WERD_RES* InsertSimpleCloneWord(const WERD_RES& clone_res, WERD* new_word);

  // Deletes the current WERD_RES and its underlying WERD.
  void DeleteCurrentWord();

  WERD_RES *forward() {  // Get next word.
    return internal_forward(false, false);
  }
  // Move forward, but allow empty blocks to show as single NULL words.
  WERD_RES *forward_with_empties() {
    return internal_forward(false, true);
  }

  WERD_RES *forward_paragraph();  // get first word in next non-empty paragraph
  WERD_RES *forward_block();  // get first word in next non-empty block

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
