///////////////////////////////////////////////////////////////////////
// File:        tesseractclass.cpp
// Description: An instance of Tesseract. For thread safety, *every*
//              global variable goes in here, directly, or indirectly.
// Author:      Ray Smith
// Created:     Fri Mar 07 08:17:01 PST 2008
//
// (C) Copyright 2008, Google Inc.
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

#include "tesseractclass.h"
#include "cube_reco_context.h"
#include "tesseract_cube_combiner.h"
#include "globals.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifdef HAVE_LIBLEPT
// Include leptonica library only if autoconf (or makefile etc) tell us to.
#include "allheaders.h"
#endif

namespace tesseract {

Tesseract::Tesseract()
  : BOOL_MEMBER(tessedit_resegment_from_boxes, false,
                "Take segmentation and labeling from box file",
                this->params()),
    BOOL_MEMBER(tessedit_resegment_from_line_boxes, false,
                "Conversion of word/line box file to char box file",
                this->params()),
    BOOL_MEMBER(tessedit_train_from_boxes, false,
                "Generate training data from boxed chars", this->params()),
    BOOL_MEMBER(tessedit_make_boxes_from_boxes, false,
                "Generate more boxes from boxed chars", this->params()),
    BOOL_MEMBER(tessedit_dump_pageseg_images, false,
               "Dump intermediate images made during page segmentation",
               this->params()),
    // The default for pageseg_mode is the old behaviour, so as not to
    // upset anything that relies on that.
    INT_MEMBER(tessedit_pageseg_mode, PSM_SINGLE_BLOCK,
               "Page seg mode: 0=osd only, 1=auto+osd, 2=auto, 3=col, 4=block,"
               " 5=line, 6=word, 7=char"
               " (Values from PageSegMode enum in publictypes.h)",
               this->params()),
    INT_INIT_MEMBER(tessedit_ocr_engine_mode, tesseract::OEM_TESSERACT_ONLY,
                    "Which OCR engine(s) to run (Tesseract, Cube, both)."
                    " Defaults to loading and running only Tesseract"
                    " (no Cube,no combiner)."
                    " Values from OcrEngineMode enum in tesseractclass.h)",
               this->params()),
    STRING_MEMBER(tessedit_char_blacklist, "",
                  "Blacklist of chars not to recognize", this->params()),
    STRING_MEMBER(tessedit_char_whitelist, "",
                  "Whitelist of chars to recognize", this->params()),
    BOOL_INIT_MEMBER(tessedit_ambigs_training, false,
                "Perform training for ambiguities", this->params()),
    INT_MEMBER(pageseg_devanagari_split_strategy,
              tesseract::ShiroRekhaSplitter::NO_SPLIT,
              "Whether to use the top-line splitting process for Devanagari "
              "documents while performing page-segmentation.", this->params()),
    INT_MEMBER(ocr_devanagari_split_strategy,
              tesseract::ShiroRekhaSplitter::NO_SPLIT,
              "Whether to use the top-line splitting process for Devanagari "
              "documents while performing ocr.", this->params()),
    STRING_MEMBER(tessedit_write_params_to_file, "",
                  "Write all parameters to the given file.", this->params()),
    BOOL_MEMBER(tessedit_adapt_to_char_fragments, true,
                "Adapt to words that contain "
                " a character composed form fragments", this->params()),
    BOOL_MEMBER(tessedit_adaption_debug, false, "Generate and print debug"
                " information for adaption", this->params()),
    INT_MEMBER(applybox_debug, 1, "Debug level", this->params()),
    INT_MEMBER(applybox_page, 0,
               "Page number to apply boxes from", this->params()),
    STRING_MEMBER(applybox_exposure_pattern, ".exp", "Exposure value follows"
                  " this pattern in the image filename. The name of the image"
                  " files are expected to be in the form"
                  " [lang].[fontname].exp[num].tif", this->params()),
    BOOL_MEMBER(applybox_learn_chars_and_char_frags_mode, false,
               "Learn both character fragments (as is done in the"
               " special low exposure mode) as well as unfragmented"
               " characters.", this->params()),
    BOOL_MEMBER(applybox_learn_ngrams_mode, false, "Each bounding box"
                " is assumed to contain ngrams. Only learn the ngrams"
                " whose outlines overlap horizontally.", this->params()),
    BOOL_MEMBER(tessedit_draw_outwords, false,
                "Draw output words", this->params()),
    BOOL_MEMBER(tessedit_training_tess, false,
                "Call Tess to learn blobs", this->params()),
    BOOL_MEMBER(tessedit_dump_choices, false,
                "Dump char choices", this->params()),
    BOOL_MEMBER(tessedit_fix_fuzzy_spaces, true,
                "Try to improve fuzzy spaces", this->params()),
    BOOL_MEMBER(tessedit_unrej_any_wd, false,
                "Dont bother with word plausibility", this->params()),
    BOOL_MEMBER(tessedit_fix_hyphens, true,
                "Crunch double hyphens?", this->params()),
    BOOL_MEMBER(tessedit_redo_xheight, true,
                "Check/Correct x-height", this->params()),
    BOOL_MEMBER(tessedit_enable_doc_dict, true,
                "Add words to the document dictionary", this->params()),
    BOOL_MEMBER(tessedit_debug_fonts, false,
                "Output font info per char", this->params()),
    BOOL_MEMBER(tessedit_debug_block_rejection, false,
                "Block and Row stats", this->params()),
    INT_MEMBER(debug_x_ht_level, 0, "Reestimate debug", this->params()),
    BOOL_MEMBER(debug_acceptable_wds, false,
                "Dump word pass/fail chk", this->params()),
    STRING_MEMBER(chs_leading_punct, "('`\"",
                  "Leading punctuation", this->params()),
    STRING_MEMBER(chs_trailing_punct1, ").,;:?!",
                  "1st Trailing punctuation", this->params()),
    STRING_MEMBER(chs_trailing_punct2, ")'`\"",
                  "2nd Trailing punctuation", this->params()),
    double_MEMBER(quality_rej_pc, 0.08,
                  "good_quality_doc lte rejection limit", this->params()),
    double_MEMBER(quality_blob_pc, 0.0,
                  "good_quality_doc gte good blobs limit", this->params()),
    double_MEMBER(quality_outline_pc, 1.0,
                  "good_quality_doc lte outline error limit", this->params()),
    double_MEMBER(quality_char_pc, 0.95,
                  "good_quality_doc gte good char limit", this->params()),
    INT_MEMBER(quality_min_initial_alphas_reqd, 2,
               "alphas in a good word", this->params()),
    BOOL_MEMBER(tessedit_tess_adapt_to_rejmap, false,
                "Use reject map to control Tesseract adaption", this->params()),
    INT_MEMBER(tessedit_tess_adaption_mode, 0x27,
               "Adaptation decision algorithm for tess", this->params()),
    BOOL_MEMBER(tessedit_minimal_rej_pass1, false,
                "Do minimal rejection on pass 1 output", this->params()),
    BOOL_MEMBER(tessedit_test_adaption, false,
                "Test adaption criteria", this->params()),
    BOOL_MEMBER(tessedit_matcher_log, false,
                "Log matcher activity", this->params()),
    INT_MEMBER(tessedit_test_adaption_mode, 3,
               "Adaptation decision algorithm for tess", this->params()),
    BOOL_MEMBER(save_best_choices, false,
                "Save the results of the recognition step (blob_choices)"
                " within the corresponding WERD_CHOICE", this->params()),
    BOOL_MEMBER(test_pt, false, "Test for point", this->params()),
    double_MEMBER(test_pt_x, 99999.99, "xcoord", this->params()),
    double_MEMBER(test_pt_y, 99999.99, "ycoord", this->params()),
    INT_MEMBER(cube_debug_level, 1, "Print cube debug info.", this->params()),
    STRING_MEMBER(outlines_odd, "%| ", "Non standard number of outlines",
                  this->params()),
    STRING_MEMBER(outlines_2, "ij!?%\":;",
                  "Non standard number of outlines", this->params()),
    BOOL_MEMBER(docqual_excuse_outline_errs, false,
                "Allow outline errs in unrejection?", this->params()),
    BOOL_MEMBER(tessedit_good_quality_unrej, true,
                "Reduce rejection on good docs", this->params()),
    BOOL_MEMBER(tessedit_use_reject_spaces, true,
                "Reject spaces?", this->params()),
    double_MEMBER(tessedit_reject_doc_percent, 65.00,
                  "%rej allowed before rej whole doc", this->params()),
    double_MEMBER(tessedit_reject_block_percent, 45.00,
                  "%rej allowed before rej whole block", this->params()),
    double_MEMBER(tessedit_reject_row_percent, 40.00,
                "%rej allowed before rej whole row", this->params()),
    double_MEMBER(tessedit_whole_wd_rej_row_percent, 70.00,
                  "Number of row rejects in whole word rejects"
                  "which prevents whole row rejection", this->params()),
    BOOL_MEMBER(tessedit_preserve_blk_rej_perfect_wds, true,
                "Only rej partially rejected words in block rejection",
                this->params()),
    BOOL_MEMBER(tessedit_preserve_row_rej_perfect_wds, true,
                "Only rej partially rejected words in row rejection",
                this->params()),
    BOOL_MEMBER(tessedit_dont_blkrej_good_wds, false,
                "Use word segmentation quality metric", this->params()),
    BOOL_MEMBER(tessedit_dont_rowrej_good_wds, false,
                "Use word segmentation quality metric", this->params()),
    INT_MEMBER(tessedit_preserve_min_wd_len, 2,
               "Only preserve wds longer than this", this->params()),
    BOOL_MEMBER(tessedit_row_rej_good_docs, true,
                "Apply row rejection to good docs", this->params()),
    double_MEMBER(tessedit_good_doc_still_rowrej_wd, 1.1,
                  "rej good doc wd if more than this fraction rejected",
                  this->params()),
    BOOL_MEMBER(tessedit_reject_bad_qual_wds, true,
                "Reject all bad quality wds", this->params()),
    BOOL_MEMBER(tessedit_debug_doc_rejection, false,
                "Page stats", this->params()),
    BOOL_MEMBER(tessedit_debug_quality_metrics, false,
                "Output data to debug file", this->params()),
    BOOL_MEMBER(bland_unrej, false,
                "unrej potential with no chekcs", this->params()),
    double_MEMBER(quality_rowrej_pc, 1.1,
                  "good_quality_doc gte good char limit", this->params()),
    BOOL_MEMBER(unlv_tilde_crunching, true,
                "Mark v.bad words for tilde crunch", this->params()),
    BOOL_MEMBER(crunch_early_merge_tess_fails, true,
                "Before word crunch?", this->params()),
    BOOL_MEMBER(crunch_early_convert_bad_unlv_chs, false,
                "Take out ~^ early?", this->params()),
    double_MEMBER(crunch_terrible_rating, 80.0,
                  "crunch rating lt this", this->params()),
    BOOL_MEMBER(crunch_terrible_garbage, true, "As it says", this->params()),
    double_MEMBER(crunch_poor_garbage_cert, -9.0,
                  "crunch garbage cert lt this", this->params()),
    double_MEMBER(crunch_poor_garbage_rate, 60,
                  "crunch garbage rating lt this", this->params()),
    double_MEMBER(crunch_pot_poor_rate, 40,
                  "POTENTIAL crunch rating lt this", this->params()),
    double_MEMBER(crunch_pot_poor_cert, -8.0,
                  "POTENTIAL crunch cert lt this", this->params()),
    BOOL_MEMBER(crunch_pot_garbage, true,
                "POTENTIAL crunch garbage", this->params()),
    double_MEMBER(crunch_del_rating, 60,
                  "POTENTIAL crunch rating lt this", this->params()),
    double_MEMBER(crunch_del_cert, -10.0,
                  "POTENTIAL crunch cert lt this", this->params()),
    double_MEMBER(crunch_del_min_ht, 0.7,
                  "Del if word ht lt xht x this", this->params()),
    double_MEMBER(crunch_del_max_ht, 3.0,
                  "Del if word ht gt xht x this", this->params()),
    double_MEMBER(crunch_del_min_width, 3.0,
                  "Del if word width lt xht x this", this->params()),
    double_MEMBER(crunch_del_high_word, 1.5,
                  "Del if word gt xht x this above bl", this->params()),
    double_MEMBER(crunch_del_low_word, 0.5,
                  "Del if word gt xht x this below bl", this->params()),
    double_MEMBER(crunch_small_outlines_size, 0.6,
                  "Small if lt xht x this", this->params()),
    INT_MEMBER(crunch_rating_max, 10,
               "For adj length in rating per ch", this->params()),
    INT_MEMBER(crunch_pot_indicators, 1,
               "How many potential indicators needed", this->params()),
    BOOL_MEMBER(crunch_leave_ok_strings, true,
                "Dont touch sensible strings", this->params()),
    BOOL_MEMBER(crunch_accept_ok, true,
                "Use acceptability in okstring", this->params()),
    BOOL_MEMBER(crunch_leave_accept_strings, false,
                "Dont pot crunch sensible strings", this->params()),
    BOOL_MEMBER(crunch_include_numerals, false,
                "Fiddle alpha figures", this->params()),
    INT_MEMBER(crunch_leave_lc_strings, 4,
               "Dont crunch words with long lower case strings",
               this->params()),
    INT_MEMBER(crunch_leave_uc_strings, 4,
               "Dont crunch words with long lower case strings",
               this->params()),
    INT_MEMBER(crunch_long_repetitions, 3,
               "Crunch words with long repetitions", this->params()),
    INT_MEMBER(crunch_debug, 0, "As it says", this->params()),
    INT_MEMBER(fixsp_non_noise_limit, 1,
               "How many non-noise blbs either side?", this->params()),
    double_MEMBER(fixsp_small_outlines_size, 0.28,
                  "Small if lt xht x this", this->params()),
    BOOL_MEMBER(tessedit_prefer_joined_punct, false,
                "Reward punctation joins", this->params()),
    INT_MEMBER(fixsp_done_mode, 1,
               "What constitues done for spacing", this->params()),
    INT_MEMBER(debug_fix_space_level, 0,
               "Contextual fixspace debug", this->params()),
    STRING_MEMBER(numeric_punctuation, ".,",
                  "Punct. chs expected WITHIN numbers", this->params()),
    INT_MEMBER(x_ht_acceptance_tolerance, 8,
               "Max allowed deviation of blob top outside of font data",
               this->params()),
    INT_MEMBER(x_ht_min_change, 8,
               "Min change in xht before actually trying it", this->params()),
    BOOL_MEMBER(tessedit_write_block_separators, false,
                "Write block separators in output", this->params()),
    BOOL_MEMBER(tessedit_write_rep_codes, false,
                "Write repetition char code", this->params()),
    BOOL_MEMBER(tessedit_write_unlv, false,
                "Write .unlv output file", this->params()),
    BOOL_MEMBER(tessedit_create_hocr, false,
                "Write .html hOCR output file", this->params()),
    STRING_MEMBER(unrecognised_char, "|",
                  "Output char for unidentified blobs", this->params()),
    INT_MEMBER(suspect_level, 99, "Suspect marker level", this->params()),
    INT_MEMBER(suspect_space_level, 100,
               "Min suspect level for rejecting spaces", this->params()),
    INT_MEMBER(suspect_short_words, 2,
               "Dont Suspect dict wds longer than this", this->params()),
    BOOL_MEMBER(suspect_constrain_1Il, false,
                "UNLV keep 1Il chars rejected", this->params()),
    double_MEMBER(suspect_rating_per_ch, 999.9,
                  "Dont touch bad rating limit", this->params()),
    double_MEMBER(suspect_accept_rating, -999.9,
                  "Accept good rating limit", this->params()),
    BOOL_MEMBER(tessedit_minimal_rejection, false,
                "Only reject tess failures", this->params()),
    BOOL_MEMBER(tessedit_zero_rejection, false,
                "Dont reject ANYTHING", this->params()),
    BOOL_MEMBER(tessedit_word_for_word, false,
                "Make output have exactly one word per WERD", this->params()),
    BOOL_MEMBER(tessedit_zero_kelvin_rejection, false,
                "Dont reject ANYTHING AT ALL", this->params()),
    BOOL_MEMBER(tessedit_consistent_reps, true,
                "Force all rep chars the same", this->params()),
    INT_MEMBER(tessedit_reject_mode, 0, "Rejection algorithm", this->params()),
    INT_MEMBER(tessedit_ok_mode, 5,
               "Acceptance decision algorithm", this->params()),
    BOOL_MEMBER(tessedit_rejection_debug, false,
                "Adaption debug", this->params()),
    BOOL_MEMBER(tessedit_flip_0O, true,
                "Contextual 0O O0 flips", this->params()),
    double_MEMBER(tessedit_lower_flip_hyphen, 1.5,
                  "Aspect ratio dot/hyphen test", this->params()),
    double_MEMBER(tessedit_upper_flip_hyphen, 1.8,
                  "Aspect ratio dot/hyphen test", this->params()),
    BOOL_MEMBER(rej_trust_doc_dawg, false,
                "Use DOC dawg in 11l conf. detector", this->params()),
    BOOL_MEMBER(rej_1Il_use_dict_word, false,
                "Use dictword test", this->params()),
    BOOL_MEMBER(rej_1Il_trust_permuter_type, true,
                "Dont double check", this->params()),
    BOOL_MEMBER(rej_use_tess_accepted, true,
                "Individual rejection control", this->params()),
    BOOL_MEMBER(rej_use_tess_blanks, true,
                "Individual rejection control", this->params()),
    BOOL_MEMBER(rej_use_good_perm, true,
                "Individual rejection control", this->params()),
    BOOL_MEMBER(rej_use_sensible_wd, false,
                "Extend permuter check", this->params()),
    BOOL_MEMBER(rej_alphas_in_number_perm, false,
                "Extend permuter check", this->params()),
    double_MEMBER(rej_whole_of_mostly_reject_word_fract, 0.85,
                  "if >this fract", this->params()),
    INT_MEMBER(tessedit_image_border, 2,
               "Rej blbs near image edge limit", this->params()),
    STRING_MEMBER(ok_repeated_ch_non_alphanum_wds, "-?*\075",
                  "Allow NN to unrej", this->params()),
    STRING_MEMBER(conflict_set_I_l_1, "Il1[]",
                  "Il1 conflict set", this->params()),
    INT_MEMBER(min_sane_x_ht_pixels, 8,
               "Reject any x-ht lt or eq than this", this->params()),
    BOOL_MEMBER(tessedit_create_boxfile, false,
                "Output text with boxes", this->params()),
    INT_MEMBER(tessedit_page_number, -1, "-1 -> All pages"
               " , else specifc page to process", this->params()),
    BOOL_MEMBER(tessedit_write_images, false,
                "Capture the image from the IPE", this->params()),
    BOOL_MEMBER(interactive_mode, false, "Run interactively?", this->params()),
    STRING_MEMBER(file_type, ".tif", "Filename extension", this->params()),
    BOOL_MEMBER(tessedit_override_permuter, true,
                "According to dict_word", this->params()),
    INT_INIT_MEMBER(tessdata_manager_debug_level, 0, "Debug level for"
                    " TessdataManager functions.", this->params()),
    double_MEMBER(min_orientation_margin, 12.0,
                  "Min acceptable orientation margin", this->params()),
    backup_config_file_(NULL),
    pix_binary_(NULL),
    pix_grey_(NULL),
    orig_image_changed_(false),
    textord_(this),
    right_to_left_(false),
    deskew_(1.0f, 0.0f),
    reskew_(1.0f, 0.0f),
    cube_cntxt_(NULL),
    tess_cube_combiner_(NULL) {
}

Tesseract::~Tesseract() {
  Clear();
  // Delete cube objects.
  if (cube_cntxt_ != NULL) {
    delete cube_cntxt_;
    cube_cntxt_ = NULL;
  }
  if (tess_cube_combiner_ != NULL) {
    delete tess_cube_combiner_;
    tess_cube_combiner_ = NULL;
  }
}

void Tesseract::Clear() {
#ifdef HAVE_LIBLEPT
  if (pix_binary_ != NULL)
    pixDestroy(&pix_binary_);
  if (pix_grey_ != NULL)
    pixDestroy(&pix_grey_);
#endif
  deskew_ = FCOORD(1.0f, 0.0f);
  reskew_ = FCOORD(1.0f, 0.0f);
  orig_image_changed_ = false;
  splitter_.Clear();
}

void Tesseract::SetBlackAndWhitelist() {
  // Set the white and blacklists (if any)
  unicharset.set_black_and_whitelist(tessedit_char_blacklist.string(),
                                     tessedit_char_whitelist.string());
}

// Perform steps to prepare underlying binary image/other data structures for
// page segmentation.
void Tesseract::PrepareForPageseg() {
  // Perform shiro-rekha (top-line) splitting and replace the current image by
  // the newly splitted image.
  splitter_.set_orig_pix(pix_binary());
  splitter_.set_pageseg_split_strategy(
      (ShiroRekhaSplitter::SplitStrategy)
      ((inT32)pageseg_devanagari_split_strategy));
  if (splitter_.Split(true)) {
    ASSERT_HOST(splitter_.splitted_image());
    splitter_.CopySplittedImageTo(NULL, &pix_binary_);
    orig_image_changed_ = true;
  }
}

// Perform steps to prepare underlying binary image/other data structures for
// OCR. The current segmentation is required by this method.
void Tesseract::PrepareForTessOCR(BLOCK_LIST* block_list,
                                  Tesseract* osd_tess, OSResults* osr) {
  // Creating blobs to OCR.
  // Utilize the segmentation information available.
  splitter_.set_segmentation_block_list(block_list);
  splitter_.set_ocr_split_strategy(
      (ShiroRekhaSplitter::SplitStrategy)
      ((inT32)ocr_devanagari_split_strategy));
  if (splitter_.Split(false)) {
    ASSERT_HOST(splitter_.splitted_image());
    splitter_.CopySplittedImageTo(NULL, &pix_binary_);
    orig_image_changed_ = true;
    // If the split strategies used before pageseg and ocr are the same, the
    // segmentation obtained from the second round can be used going forward.
    // Otherwise, the page-segmentation (& importantly, the word segmentation)
    // of first round is used.
    if (splitter_.HasDifferentSplitStrategies()) {
      // Refresh the segmentation with new blobs.
      BLOCK_LIST new_segmentation;
      SegmentPage(NULL, &new_segmentation, osd_tess, osr);
      C_BLOB_LIST new_blobs;
      ExtractBlobsFromSegmentation(&new_segmentation, &new_blobs);
      splitter_.RefreshSegmentationWithNewBlobs(&new_blobs);
    } else {
      block_list->clear();
      SegmentPage(NULL, block_list, osd_tess, osr);
    }
  }
}

// Perform steps to prepare underlying binary image/other data structures for
// Cube OCR.
void Tesseract::PrepareForCubeOCR() {
  if (orig_image_changed_) {
    // Revert to the original image as Cube likes them more.
    splitter_.CopyOriginalImageTo(NULL, &pix_binary_);
    orig_image_changed_ = false;
  }
}
}  // namespace tesseract
