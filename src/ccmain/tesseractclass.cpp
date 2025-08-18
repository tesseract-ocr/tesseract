///////////////////////////////////////////////////////////////////////
// File:        tesseractclass.cpp
// Description: The Tesseract class. It holds/owns everything needed
//              to run Tesseract on a single language, and also a set of
//              sub-Tesseracts to run sub-languages. For thread safety, *every*
//              variable that was previously global or static (except for
//              constant data, and some visual debugging flags) has been moved
//              in here, directly, or indirectly.
//              This makes it safe to run multiple Tesseracts in different
//              threads in parallel, and keeps the different language
//              instances separate.
//              Some global functions remain, but they are isolated re-entrant
//              functions that operate on their arguments. Functions that work
//              on variable data have been moved to an appropriate class based
//              mostly on the directory hierarchy. For more information see
//              slide 6 of "2ArchitectureAndDataStructures" in
// https://drive.google.com/file/d/0B7l10Bj_LprhbUlIUFlCdGtDYkE/edit?usp=sharing
//              Some global data and related functions still exist in the
//              training-related code, but they don't interfere with normal
//              recognition operation.
// Author:      Ray Smith
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "tesseractclass.h"

#include <allheaders.h>
#include "edgblob.h"
#ifndef DISABLED_LEGACY_ENGINE
#  include "equationdetect.h"
#endif
#include "lstmrecognizer.h"
#include "thresholder.h" // for ThresholdMethod

namespace tesseract {

Tesseract::Tesseract()
    : BOOL_MEMBER(tessedit_resegment_from_boxes, false,
                  "Take segmentation and labeling from box file", this->params())
    , BOOL_MEMBER(tessedit_resegment_from_line_boxes, false,
                  "Conversion of word/line box file to char box file", this->params())
    , BOOL_MEMBER(tessedit_train_from_boxes, false, "Generate training data from boxed chars",
                  this->params())
    , BOOL_MEMBER(tessedit_make_boxes_from_boxes, false, "Generate more boxes from boxed chars",
                  this->params())
    , BOOL_MEMBER(tessedit_train_line_recognizer, false,
                  "Break input into lines and remap boxes if present", this->params())
    , BOOL_MEMBER(tessedit_dump_pageseg_images, false,
                  "Dump intermediate images made during page segmentation", this->params())
    // TODO: remove deprecated tessedit_do_invert in release 6.
    , BOOL_MEMBER(tessedit_do_invert, true,
                  "Try inverted line image if necessary (deprecated, will be "
                  "removed in release 6, use the 'invert_threshold' parameter instead)",
                  this->params())
    , double_MEMBER(invert_threshold, 0.7,
                    "For lines with a mean confidence below this value, OCR is also tried with an inverted image",
                    this->params())
    ,
    // The default for pageseg_mode is the old behaviour, so as not to
    // upset anything that relies on that.
    INT_MEMBER(tessedit_pageseg_mode, PSM_SINGLE_BLOCK,
               "Page seg mode: 0=osd only, 1=auto+osd, 2=auto_only, 3=auto, "
               "4=column,"
               " 5=block_vert, 6=block, 7=line, 8=word, 9=word_circle, 10=char,"
               "11=sparse_text, 12=sparse_text+osd, 13=raw_line"
               " (Values from PageSegMode enum in tesseract/publictypes.h)",
               this->params())
    , INT_MEMBER(thresholding_method,
                 static_cast<int>(ThresholdMethod::Otsu),
                 "Thresholding method: 0 = Otsu, 1 = LeptonicaOtsu, 2 = "
                 "Sauvola",
                 this->params())
    , BOOL_MEMBER(thresholding_debug, false,
                  "Debug the thresholding process",
                  this->params())
    , double_MEMBER(thresholding_window_size, 0.33,
                    "Window size for measuring local statistics (to be "
                    "multiplied by image DPI). "
                    "This parameter is used by the Sauvola thresholding method",
                    this->params())
    , double_MEMBER(thresholding_kfactor, 0.34,
                    "Factor for reducing threshold due to variance. "
                    "This parameter is used by the Sauvola thresholding method."
                    " Normal range: 0.2-0.5",
                    this->params())
    , double_MEMBER(thresholding_tile_size, 0.33,
                    "Desired tile size (to be multiplied by image DPI). "
                    "This parameter is used by the LeptonicaOtsu thresholding "
                    "method",
                    this->params())
    , double_MEMBER(thresholding_smooth_kernel_size, 0.0,
                    "Size of convolution kernel applied to threshold array "
                    "(to be multiplied by image DPI). Use 0 for no smoothing. "
                    "This parameter is used by the LeptonicaOtsu thresholding "
                    "method",
                    this->params())
    , double_MEMBER(thresholding_score_fraction, 0.1,
                    "Fraction of the max Otsu score. "
                    "This parameter is used by the LeptonicaOtsu thresholding "
                    "method. "
                    "For standard Otsu use 0.0, otherwise 0.1 is recommended",
                    this->params())
    , INT_INIT_MEMBER(tessedit_ocr_engine_mode, tesseract::OEM_DEFAULT,
                      "Which OCR engine(s) to run (Tesseract, LSTM, both)."
                      " Defaults to loading and running the most accurate"
                      " available.",
                      this->params())
    , STRING_MEMBER(tessedit_char_blacklist, "", "Blacklist of chars not to recognize",
                    this->params())
    , STRING_MEMBER(tessedit_char_whitelist, "", "Whitelist of chars to recognize", this->params())
    , STRING_MEMBER(tessedit_char_unblacklist, "",
                    "List of chars to override tessedit_char_blacklist", this->params())
    , BOOL_MEMBER(tessedit_ambigs_training, false, "Perform training for ambiguities",
                  this->params())
    , INT_MEMBER(pageseg_devanagari_split_strategy, tesseract::ShiroRekhaSplitter::NO_SPLIT,
                 "Whether to use the top-line splitting process for Devanagari "
                 "documents while performing page-segmentation.",
                 this->params())
    , INT_MEMBER(ocr_devanagari_split_strategy, tesseract::ShiroRekhaSplitter::NO_SPLIT,
                 "Whether to use the top-line splitting process for Devanagari "
                 "documents while performing ocr.",
                 this->params())
    , STRING_MEMBER(tessedit_write_params_to_file, "", "Write all parameters to the given file.",
                    this->params())
    , BOOL_MEMBER(tessedit_adaption_debug, false,
                  "Generate and print debug"
                  " information for adaption",
                  this->params())
    , INT_MEMBER(bidi_debug, 0, "Debug level for BiDi", this->params())
    , INT_MEMBER(applybox_debug, 1, "Debug level", this->params())
    , INT_MEMBER(applybox_page, 0, "Page number to apply boxes from", this->params())
    , STRING_MEMBER(applybox_exposure_pattern, ".exp",
                    "Exposure value follows"
                    " this pattern in the image filename. The name of the image"
                    " files are expected to be in the form"
                    " [lang].[fontname].exp[num].tif",
                    this->params())
    , BOOL_MEMBER(applybox_learn_chars_and_char_frags_mode, false,
                  "Learn both character fragments (as is done in the"
                  " special low exposure mode) as well as unfragmented"
                  " characters.",
                  this->params())
    , BOOL_MEMBER(applybox_learn_ngrams_mode, false,
                  "Each bounding box"
                  " is assumed to contain ngrams. Only learn the ngrams"
                  " whose outlines overlap horizontally.",
                  this->params())
    , BOOL_MEMBER(tessedit_display_outwords, false, "Draw output words", this->params())
    , BOOL_MEMBER(tessedit_dump_choices, false, "Dump char choices", this->params())
    , BOOL_MEMBER(tessedit_timing_debug, false, "Print timing stats", this->params())
    , BOOL_MEMBER(tessedit_fix_fuzzy_spaces, true, "Try to improve fuzzy spaces", this->params())
    , BOOL_MEMBER(tessedit_unrej_any_wd, false, "Don't bother with word plausibility",
                  this->params())
    , BOOL_MEMBER(tessedit_fix_hyphens, true, "Crunch double hyphens?", this->params())
    , BOOL_MEMBER(tessedit_enable_doc_dict, true, "Add words to the document dictionary",
                  this->params())
    , BOOL_MEMBER(tessedit_debug_fonts, false, "Output font info per char", this->params())
    , INT_MEMBER(tessedit_font_id, 0, "Font ID to use or zero", this->params())
    , BOOL_MEMBER(tessedit_debug_block_rejection, false, "Block and Row stats", this->params())
    , BOOL_MEMBER(tessedit_enable_bigram_correction, true,
                  "Enable correction based on the word bigram dictionary.", this->params())
    , BOOL_MEMBER(tessedit_enable_dict_correction, false,
                  "Enable single word correction based on the dictionary.", this->params())
    , INT_MEMBER(tessedit_bigram_debug, 0, "Amount of debug output for bigram correction.",
                 this->params())
    , BOOL_MEMBER(enable_noise_removal, true,
                  "Remove and conditionally reassign small outlines when they"
                  " confuse layout analysis, determining diacritics vs noise",
                  this->params())
    , INT_MEMBER(debug_noise_removal, 0, "Debug reassignment of small outlines", this->params())
    ,
    // Worst (min) certainty, for which a diacritic is allowed to make the
    // base
    // character worse and still be included.
    double_MEMBER(noise_cert_basechar, -8.0, "Hingepoint for base char certainty", this->params())
    ,
    // Worst (min) certainty, for which a non-overlapping diacritic is allowed
    // to make the base character worse and still be included.
    double_MEMBER(noise_cert_disjoint, -1.0, "Hingepoint for disjoint certainty", this->params())
    ,
    // Worst (min) certainty, for which a diacritic is allowed to make a new
    // stand-alone blob.
    double_MEMBER(noise_cert_punc, -3.0, "Threshold for new punc char certainty", this->params())
    ,
    // Factor of certainty margin for adding diacritics to not count as worse.
    double_MEMBER(noise_cert_factor, 0.375, "Scaling on certainty diff from Hingepoint",
                  this->params())
    , INT_MEMBER(noise_maxperblob, 8, "Max diacritics to apply to a blob", this->params())
    , INT_MEMBER(noise_maxperword, 16, "Max diacritics to apply to a word", this->params())
    , INT_MEMBER(debug_x_ht_level, 0, "Reestimate debug", this->params())
    , STRING_MEMBER(chs_leading_punct, "('`\"", "Leading punctuation", this->params())
    , STRING_MEMBER(chs_trailing_punct1, ").,;:?!", "1st Trailing punctuation", this->params())
    , STRING_MEMBER(chs_trailing_punct2, ")'`\"", "2nd Trailing punctuation", this->params())
    , double_MEMBER(quality_rej_pc, 0.08, "good_quality_doc lte rejection limit", this->params())
    , double_MEMBER(quality_blob_pc, 0.0, "good_quality_doc gte good blobs limit", this->params())
    , double_MEMBER(quality_outline_pc, 1.0, "good_quality_doc lte outline error limit",
                    this->params())
    , double_MEMBER(quality_char_pc, 0.95, "good_quality_doc gte good char limit", this->params())
    , INT_MEMBER(quality_min_initial_alphas_reqd, 2, "alphas in a good word", this->params())
    , INT_MEMBER(tessedit_tess_adaption_mode, 0x27, "Adaptation decision algorithm for tess",
                 this->params())
    , BOOL_MEMBER(tessedit_minimal_rej_pass1, false, "Do minimal rejection on pass 1 output",
                  this->params())
    , BOOL_MEMBER(tessedit_test_adaption, false, "Test adaption criteria", this->params())
    , BOOL_MEMBER(test_pt, false, "Test for point", this->params())
    , double_MEMBER(test_pt_x, 99999.99, "xcoord", this->params())
    , double_MEMBER(test_pt_y, 99999.99, "ycoord", this->params())
    , INT_MEMBER(multilang_debug_level, 0, "Print multilang debug info.", this->params())
    , INT_MEMBER(paragraph_debug_level, 0, "Print paragraph debug info.", this->params())
    , BOOL_MEMBER(paragraph_text_based, true,
                  "Run paragraph detection on the post-text-recognition "
                  "(more accurate)",
                  this->params())
    , BOOL_MEMBER(lstm_use_matrix, 1, "Use ratings matrix/beam search with lstm", this->params())
    , STRING_MEMBER(outlines_odd, "%| ", "Non standard number of outlines", this->params())
    , STRING_MEMBER(outlines_2, "ij!?%\":;", "Non standard number of outlines", this->params())
    , BOOL_MEMBER(tessedit_good_quality_unrej, true, "Reduce rejection on good docs",
                  this->params())
    , BOOL_MEMBER(tessedit_use_reject_spaces, true, "Reject spaces?", this->params())
    , double_MEMBER(tessedit_reject_doc_percent, 65.00, "%rej allowed before rej whole doc",
                    this->params())
    , double_MEMBER(tessedit_reject_block_percent, 45.00, "%rej allowed before rej whole block",
                    this->params())
    , double_MEMBER(tessedit_reject_row_percent, 40.00, "%rej allowed before rej whole row",
                    this->params())
    , double_MEMBER(tessedit_whole_wd_rej_row_percent, 70.00,
                    "Number of row rejects in whole word rejects"
                    " which prevents whole row rejection",
                    this->params())
    , BOOL_MEMBER(tessedit_preserve_blk_rej_perfect_wds, true,
                  "Only rej partially rejected words in block rejection", this->params())
    , BOOL_MEMBER(tessedit_preserve_row_rej_perfect_wds, true,
                  "Only rej partially rejected words in row rejection", this->params())
    , BOOL_MEMBER(tessedit_dont_blkrej_good_wds, false, "Use word segmentation quality metric",
                  this->params())
    , BOOL_MEMBER(tessedit_dont_rowrej_good_wds, false, "Use word segmentation quality metric",
                  this->params())
    , INT_MEMBER(tessedit_preserve_min_wd_len, 2, "Only preserve wds longer than this",
                 this->params())
    , BOOL_MEMBER(tessedit_row_rej_good_docs, true, "Apply row rejection to good docs",
                  this->params())
    , double_MEMBER(tessedit_good_doc_still_rowrej_wd, 1.1,
                    "rej good doc wd if more than this fraction rejected", this->params())
    , BOOL_MEMBER(tessedit_reject_bad_qual_wds, true, "Reject all bad quality wds", this->params())
    , BOOL_MEMBER(tessedit_debug_doc_rejection, false, "Page stats", this->params())
    , BOOL_MEMBER(tessedit_debug_quality_metrics, false, "Output data to debug file",
                  this->params())
    , BOOL_MEMBER(bland_unrej, false, "unrej potential with no checks", this->params())
    , double_MEMBER(quality_rowrej_pc, 1.1, "good_quality_doc gte good char limit", this->params())
    , BOOL_MEMBER(unlv_tilde_crunching, false, "Mark v.bad words for tilde crunch", this->params())
    , BOOL_MEMBER(hocr_font_info, false, "Add font info to hocr output", this->params())
    , BOOL_MEMBER(hocr_char_boxes, false, "Add coordinates for each character to hocr output",
                  this->params())
    , BOOL_MEMBER(crunch_early_merge_tess_fails, true, "Before word crunch?", this->params())
    , BOOL_MEMBER(crunch_early_convert_bad_unlv_chs, false, "Take out ~^ early?", this->params())
    , double_MEMBER(crunch_terrible_rating, 80.0, "crunch rating lt this", this->params())
    , BOOL_MEMBER(crunch_terrible_garbage, true, "As it says", this->params())
    , double_MEMBER(crunch_poor_garbage_cert, -9.0, "crunch garbage cert lt this", this->params())
    , double_MEMBER(crunch_poor_garbage_rate, 60, "crunch garbage rating lt this", this->params())
    , double_MEMBER(crunch_pot_poor_rate, 40, "POTENTIAL crunch rating lt this", this->params())
    , double_MEMBER(crunch_pot_poor_cert, -8.0, "POTENTIAL crunch cert lt this", this->params())
    , double_MEMBER(crunch_del_rating, 60, "POTENTIAL crunch rating lt this", this->params())
    , double_MEMBER(crunch_del_cert, -10.0, "POTENTIAL crunch cert lt this", this->params())
    , double_MEMBER(crunch_del_min_ht, 0.7, "Del if word ht lt xht x this", this->params())
    , double_MEMBER(crunch_del_max_ht, 3.0, "Del if word ht gt xht x this", this->params())
    , double_MEMBER(crunch_del_min_width, 3.0, "Del if word width lt xht x this", this->params())
    , double_MEMBER(crunch_del_high_word, 1.5, "Del if word gt xht x this above bl", this->params())
    , double_MEMBER(crunch_del_low_word, 0.5, "Del if word gt xht x this below bl", this->params())
    , double_MEMBER(crunch_small_outlines_size, 0.6, "Small if lt xht x this", this->params())
    , INT_MEMBER(crunch_rating_max, 10, "For adj length in rating per ch", this->params())
    , INT_MEMBER(crunch_pot_indicators, 1, "How many potential indicators needed", this->params())
    , BOOL_MEMBER(crunch_leave_ok_strings, true, "Don't touch sensible strings", this->params())
    , BOOL_MEMBER(crunch_accept_ok, true, "Use acceptability in okstring", this->params())
    , BOOL_MEMBER(crunch_leave_accept_strings, false, "Don't pot crunch sensible strings",
                  this->params())
    , BOOL_MEMBER(crunch_include_numerals, false, "Fiddle alpha figures", this->params())
    , INT_MEMBER(crunch_leave_lc_strings, 4, "Don't crunch words with long lower case strings",
                 this->params())
    , INT_MEMBER(crunch_leave_uc_strings, 4, "Don't crunch words with long lower case strings",
                 this->params())
    , INT_MEMBER(crunch_long_repetitions, 3, "Crunch words with long repetitions", this->params())
    , INT_MEMBER(crunch_debug, 0, "As it says", this->params())
    , INT_MEMBER(fixsp_non_noise_limit, 1, "How many non-noise blbs either side?", this->params())
    , double_MEMBER(fixsp_small_outlines_size, 0.28, "Small if lt xht x this", this->params())
    , BOOL_MEMBER(tessedit_prefer_joined_punct, false, "Reward punctuation joins", this->params())
    , INT_MEMBER(fixsp_done_mode, 1, "What constitutes done for spacing", this->params())
    , INT_MEMBER(debug_fix_space_level, 0, "Contextual fixspace debug", this->params())
    , STRING_MEMBER(numeric_punctuation, ".,", "Punct. chs expected WITHIN numbers", this->params())
    , INT_MEMBER(x_ht_acceptance_tolerance, 8,
                 "Max allowed deviation of blob top outside of font data", this->params())
    , INT_MEMBER(x_ht_min_change, 8, "Min change in xht before actually trying it", this->params())
    , INT_MEMBER(superscript_debug, 0, "Debug level for sub & superscript fixer", this->params())
    , double_MEMBER(superscript_worse_certainty, 2.0,
                    "How many times worse "
                    "certainty does a superscript position glyph need to be for "
                    "us to try classifying it as a char with a different "
                    "baseline?",
                    this->params())
    , double_MEMBER(superscript_bettered_certainty, 0.97,
                    "What reduction in "
                    "badness do we think sufficient to choose a superscript "
                    "over what we'd thought.  For example, a value of 0.6 means "
                    "we want to reduce badness of certainty by at least 40%",
                    this->params())
    , double_MEMBER(superscript_scaledown_ratio, 0.4,
                    "A superscript scaled down more than this is unbelievably "
                    "small.  For example, 0.3 means we expect the font size to "
                    "be no smaller than 30% of the text line font size.",
                    this->params())
    , double_MEMBER(subscript_max_y_top, 0.5,
                    "Maximum top of a character measured as a multiple of "
                    "x-height above the baseline for us to reconsider whether "
                    "it's a subscript.",
                    this->params())
    , double_MEMBER(superscript_min_y_bottom, 0.3,
                    "Minimum bottom of a character measured as a multiple of "
                    "x-height above the baseline for us to reconsider whether "
                    "it's a superscript.",
                    this->params())
    , BOOL_MEMBER(tessedit_write_block_separators, false, "Write block separators in output",
                  this->params())
    , BOOL_MEMBER(tessedit_write_rep_codes, false, "Write repetition char code", this->params())
    , BOOL_MEMBER(tessedit_write_unlv, false, "Write .unlv output file", this->params())
    , BOOL_MEMBER(tessedit_create_txt, false, "Write .txt output file", this->params())
    , BOOL_MEMBER(tessedit_create_hocr, false, "Write .html hOCR output file", this->params())
    , BOOL_MEMBER(tessedit_create_alto, false, "Write .xml ALTO file", this->params())
    , BOOL_MEMBER(tessedit_create_page_xml, false, "Write .page.xml PAGE file", this->params())
    , BOOL_MEMBER(page_xml_polygon, true, "Create the PAGE file with polygons instead of box values", this->params())
    , INT_MEMBER(page_xml_level, 0, "Create the PAGE file on 0=line or 1=word level.", this->params())
    , BOOL_MEMBER(tessedit_create_lstmbox, false, "Write .box file for LSTM training",
                  this->params())
    , BOOL_MEMBER(tessedit_create_tsv, false, "Write .tsv output file", this->params())
    , BOOL_MEMBER(tessedit_create_wordstrbox, false, "Write WordStr format .box output file",
                  this->params())
    , BOOL_MEMBER(tessedit_create_pdf, false, "Write .pdf output file", this->params())
    , BOOL_MEMBER(textonly_pdf, false, "Create PDF with only one invisible text layer",
                  this->params())
    , INT_MEMBER(jpg_quality, 85, "Set JPEG quality level", this->params())
    , INT_MEMBER(user_defined_dpi, 0, "Specify DPI for input image", this->params())
    , INT_MEMBER(min_characters_to_try, 50, "Specify minimum characters to try during OSD",
                 this->params())
    , STRING_MEMBER(unrecognised_char, "|", "Output char for unidentified blobs", this->params())
    , INT_MEMBER(suspect_level, 99, "Suspect marker level", this->params())
    , INT_MEMBER(suspect_short_words, 2, "Don't suspect dict wds longer than this", this->params())
    , BOOL_MEMBER(suspect_constrain_1Il, false, "UNLV keep 1Il chars rejected", this->params())
    , double_MEMBER(suspect_rating_per_ch, 999.9, "Don't touch bad rating limit", this->params())
    , double_MEMBER(suspect_accept_rating, -999.9, "Accept good rating limit", this->params())
    , BOOL_MEMBER(tessedit_minimal_rejection, false, "Only reject tess failures", this->params())
    , BOOL_MEMBER(tessedit_zero_rejection, false, "Don't reject ANYTHING", this->params())
    , BOOL_MEMBER(tessedit_word_for_word, false, "Make output have exactly one word per WERD",
                  this->params())
    , BOOL_MEMBER(tessedit_zero_kelvin_rejection, false, "Don't reject ANYTHING AT ALL",
                  this->params())
    , INT_MEMBER(tessedit_reject_mode, 0, "Rejection algorithm", this->params())
    , BOOL_MEMBER(tessedit_rejection_debug, false, "Adaption debug", this->params())
    , BOOL_MEMBER(tessedit_flip_0O, true, "Contextual 0O O0 flips", this->params())
    , double_MEMBER(tessedit_lower_flip_hyphen, 1.5, "Aspect ratio dot/hyphen test", this->params())
    , double_MEMBER(tessedit_upper_flip_hyphen, 1.8, "Aspect ratio dot/hyphen test", this->params())
    , BOOL_MEMBER(rej_trust_doc_dawg, false, "Use DOC dawg in 11l conf. detector", this->params())
    , BOOL_MEMBER(rej_1Il_use_dict_word, false, "Use dictword test", this->params())
    , BOOL_MEMBER(rej_1Il_trust_permuter_type, true, "Don't double check", this->params())
    , BOOL_MEMBER(rej_use_tess_accepted, true, "Individual rejection control", this->params())
    , BOOL_MEMBER(rej_use_tess_blanks, true, "Individual rejection control", this->params())
    , BOOL_MEMBER(rej_use_good_perm, true, "Individual rejection control", this->params())
    , BOOL_MEMBER(rej_use_sensible_wd, false, "Extend permuter check", this->params())
    , BOOL_MEMBER(rej_alphas_in_number_perm, false, "Extend permuter check", this->params())
    , double_MEMBER(rej_whole_of_mostly_reject_word_fract, 0.85, "if >this fract", this->params())
    , INT_MEMBER(tessedit_image_border, 2, "Rej blbs near image edge limit", this->params())
    , STRING_MEMBER(ok_repeated_ch_non_alphanum_wds, "-?*\075", "Allow NN to unrej", this->params())
    , STRING_MEMBER(conflict_set_I_l_1, "Il1[]", "Il1 conflict set", this->params())
    , INT_MEMBER(min_sane_x_ht_pixels, 8, "Reject any x-ht lt or eq than this", this->params())
    , BOOL_MEMBER(tessedit_create_boxfile, false, "Output text with boxes", this->params())
    , INT_MEMBER(tessedit_page_number, -1, "-1 -> All pages, else specific page to process",
                 this->params())
    , BOOL_MEMBER(tessedit_write_images, false, "Capture the image from the IPE", this->params())
    , BOOL_MEMBER(interactive_display_mode, false, "Run interactively?", this->params())
    , STRING_MEMBER(file_type, ".tif", "Filename extension", this->params())
    , BOOL_MEMBER(tessedit_override_permuter, true, "According to dict_word", this->params())
    , STRING_MEMBER(tessedit_load_sublangs, "", "List of languages to load with this one",
                    this->params())
    , BOOL_MEMBER(tessedit_use_primary_params_model, false,
                  "In multilingual mode use params model of the"
                  " primary language",
                  this->params())
    , double_MEMBER(min_orientation_margin, 7.0, "Min acceptable orientation margin",
                    this->params())
    , BOOL_MEMBER(textord_tabfind_show_vlines, false, "Debug line finding", this->params())
    , BOOL_MEMBER(textord_use_cjk_fp_model, false, "Use CJK fixed pitch model", this->params())
    , BOOL_MEMBER(tsv_lang_info, false, "Include language info in the  .tsv output file", this->params())
    , BOOL_MEMBER(poly_allow_detailed_fx, false,
                  "Allow feature extractors to see the original outline", this->params())
    , BOOL_INIT_MEMBER(tessedit_init_config_only, false,
                       "Only initialize with the config file. Useful if the "
                       "instance is not going to be used for OCR but say only "
                       "for layout analysis.",
                       this->params())
#ifndef DISABLED_LEGACY_ENGINE
    , BOOL_MEMBER(textord_equation_detect, false, "Turn on equation detector", this->params())
#endif // ndef DISABLED_LEGACY_ENGINE
    , BOOL_MEMBER(textord_tabfind_vertical_text, true, "Enable vertical detection", this->params())
    , BOOL_MEMBER(textord_tabfind_force_vertical_text, false, "Force using vertical text page mode",
                  this->params())
    , double_MEMBER(textord_tabfind_vertical_text_ratio, 0.5,
                    "Fraction of textlines deemed vertical to use vertical page "
                    "mode",
                    this->params())
    , double_MEMBER(textord_tabfind_aligned_gap_fraction, 0.75,
                    "Fraction of height used as a minimum gap for aligned blobs.", this->params())
    , INT_MEMBER(tessedit_parallelize, 0, "Run in parallel where possible", this->params())
    , BOOL_MEMBER(preserve_interword_spaces, false, "Preserve multiple interword spaces",
                  this->params())
    , STRING_MEMBER(page_separator, "\f", "Page separator (default is form feed control character)",
                    this->params())
    , INT_MEMBER(lstm_choice_mode, 0,
                 "Allows to include alternative symbols choices in the hOCR output. "
                 "Valid input values are 0, 1 and 2. 0 is the default value. "
                 "With 1 the alternative symbol choices per timestep are included. "
                 "With 2 alternative symbol choices are extracted from the CTC "
                 "process instead of the lattice. The choices are mapped per "
                 "character.",
                 this->params())
    , INT_MEMBER(lstm_choice_iterations, 5,
                 "Sets the number of cascading iterations for the Beamsearch in "
                 "lstm_choice_mode. Note that lstm_choice_mode must be set to a "
                 "value greater than 0 to produce results.",
                 this->params())
    , double_MEMBER(lstm_rating_coefficient, 5,
                    "Sets the rating coefficient for the lstm choices. The smaller the "
                    "coefficient, the better are the ratings for each choice and less "
                    "information is lost due to the cut off at 0. The standard value is "
                    "5",
                    this->params())
    , BOOL_MEMBER(pageseg_apply_music_mask, false,
                  "Detect music staff and remove intersecting components", this->params())
    ,

    backup_config_file_(nullptr)
    , pix_binary_(nullptr)
    , pix_grey_(nullptr)
    , pix_original_(nullptr)
    , pix_thresholds_(nullptr)
    , source_resolution_(0)
    , textord_(this)
    , right_to_left_(false)
    , scaled_color_(nullptr)
    , scaled_factor_(-1)
    , deskew_(1.0f, 0.0f)
    , reskew_(1.0f, 0.0f)
    , gradient_(0.0f)
    , most_recently_used_(this)
    , font_table_size_(0)
#ifndef DISABLED_LEGACY_ENGINE
    , equ_detect_(nullptr)
#endif // ndef DISABLED_LEGACY_ENGINE
    , lstm_recognizer_(nullptr)
    , train_line_page_num_(0) {}

Tesseract::~Tesseract() {
  Clear();
  pix_original_.destroy();
  end_tesseract();
  for (auto *lang : sub_langs_) {
    delete lang;
  }
  delete lstm_recognizer_;
  lstm_recognizer_ = nullptr;
}

Dict &Tesseract::getDict() {
  if (0 == Classify::getDict().NumDawgs() && AnyLSTMLang()) {
    if (lstm_recognizer_ && lstm_recognizer_->GetDict()) {
      return *lstm_recognizer_->GetDict();
    }
  }
  return Classify::getDict();
}

void Tesseract::Clear() {
  std::string debug_name = imagebasename + "_debug.pdf";
  pixa_debug_.WritePDF(debug_name.c_str());
  pix_binary_.destroy();
  pix_grey_.destroy();
  pix_thresholds_.destroy();
  scaled_color_.destroy();
  deskew_ = FCOORD(1.0f, 0.0f);
  reskew_ = FCOORD(1.0f, 0.0f);
  gradient_ = 0.0f;
  splitter_.Clear();
  scaled_factor_ = -1;
  for (auto &sub_lang : sub_langs_) {
    sub_lang->Clear();
  }
}

#ifndef DISABLED_LEGACY_ENGINE

void Tesseract::SetEquationDetect(EquationDetect *detector) {
  equ_detect_ = detector;
  equ_detect_->SetLangTesseract(this);
}

// Clear all memory of adaption for this and all subclassifiers.
void Tesseract::ResetAdaptiveClassifier() {
  ResetAdaptiveClassifierInternal();
  for (auto &sub_lang : sub_langs_) {
    sub_lang->ResetAdaptiveClassifierInternal();
  }
}

#endif // ndef DISABLED_LEGACY_ENGINE

// Clear the document dictionary for this and all subclassifiers.
void Tesseract::ResetDocumentDictionary() {
  getDict().ResetDocumentDictionary();
  for (auto &sub_lang : sub_langs_) {
    sub_lang->getDict().ResetDocumentDictionary();
  }
}

void Tesseract::SetBlackAndWhitelist() {
  // Set the white and blacklists (if any)
  unicharset.set_black_and_whitelist(tessedit_char_blacklist.c_str(),
                                     tessedit_char_whitelist.c_str(),
                                     tessedit_char_unblacklist.c_str());
  if (lstm_recognizer_) {
    UNICHARSET &lstm_unicharset = lstm_recognizer_->GetUnicharset();
    lstm_unicharset.set_black_and_whitelist(tessedit_char_blacklist.c_str(),
                                            tessedit_char_whitelist.c_str(),
                                            tessedit_char_unblacklist.c_str());
  }
  // Black and white lists should apply to all loaded classifiers.
  for (auto &sub_lang : sub_langs_) {
    sub_lang->unicharset.set_black_and_whitelist(tessedit_char_blacklist.c_str(),
                                                 tessedit_char_whitelist.c_str(),
                                                 tessedit_char_unblacklist.c_str());
    if (sub_lang->lstm_recognizer_) {
      UNICHARSET &lstm_unicharset = sub_lang->lstm_recognizer_->GetUnicharset();
      lstm_unicharset.set_black_and_whitelist(tessedit_char_blacklist.c_str(),
                                              tessedit_char_whitelist.c_str(),
                                              tessedit_char_unblacklist.c_str());
    }
  }
}

// Perform steps to prepare underlying binary image/other data structures for
// page segmentation.
void Tesseract::PrepareForPageseg() {
  textord_.set_use_cjk_fp_model(textord_use_cjk_fp_model);
  // Find the max splitter strategy over all langs.
  auto max_pageseg_strategy = static_cast<ShiroRekhaSplitter::SplitStrategy>(
      static_cast<int32_t>(pageseg_devanagari_split_strategy));
  for (auto &sub_lang : sub_langs_) {
    auto pageseg_strategy = static_cast<ShiroRekhaSplitter::SplitStrategy>(
        static_cast<int32_t>(sub_lang->pageseg_devanagari_split_strategy));
    if (pageseg_strategy > max_pageseg_strategy) {
      max_pageseg_strategy = pageseg_strategy;
    }
    sub_lang->pix_binary_.destroy();
    sub_lang->pix_binary_ = pix_binary().clone();
  }
  // Perform shiro-rekha (top-line) splitting and replace the current image by
  // the newly split image.
  splitter_.set_orig_pix(pix_binary());
  splitter_.set_pageseg_split_strategy(max_pageseg_strategy);
  if (splitter_.Split(true, &pixa_debug_)) {
    ASSERT_HOST(splitter_.splitted_image());
    pix_binary_.destroy();
    pix_binary_ = splitter_.splitted_image().clone();
  }
}

// Perform steps to prepare underlying binary image/other data structures for
// OCR. The current segmentation is required by this method.
// Note that this method resets pix_binary_ to the original binarized image,
// which may be different from the image actually used for OCR depending on the
// value of devanagari_ocr_split_strategy.
void Tesseract::PrepareForTessOCR(BLOCK_LIST *block_list, Tesseract *osd_tess, OSResults *osr) {
  // Find the max splitter strategy over all langs.
  auto max_ocr_strategy = static_cast<ShiroRekhaSplitter::SplitStrategy>(
      static_cast<int32_t>(ocr_devanagari_split_strategy));
  for (auto &sub_lang : sub_langs_) {
    auto ocr_strategy = static_cast<ShiroRekhaSplitter::SplitStrategy>(
        static_cast<int32_t>(sub_lang->ocr_devanagari_split_strategy));
    if (ocr_strategy > max_ocr_strategy) {
      max_ocr_strategy = ocr_strategy;
    }
  }
  // Utilize the segmentation information available.
  splitter_.set_segmentation_block_list(block_list);
  splitter_.set_ocr_split_strategy(max_ocr_strategy);
  // Run the splitter for OCR
  bool split_for_ocr = splitter_.Split(false, &pixa_debug_);
  // Restore pix_binary to the binarized original pix for future reference.
  ASSERT_HOST(splitter_.orig_pix());
  pix_binary_.destroy();
  pix_binary_ = splitter_.orig_pix().clone();
  // If the pageseg and ocr strategies are different, refresh the block list
  // (from the last SegmentImage call) with blobs from the real image to be used
  // for OCR.
  if (splitter_.HasDifferentSplitStrategies()) {
    BLOCK block("", true, 0, 0, 0, 0, pixGetWidth(pix_binary_), pixGetHeight(pix_binary_));
    Image pix_for_ocr = split_for_ocr ? splitter_.splitted_image() : splitter_.orig_pix();
    extract_edges(pix_for_ocr, &block);
    splitter_.RefreshSegmentationWithNewBlobs(block.blob_list());
  }
  // The splitter isn't needed any more after this, so save memory by clearing.
  splitter_.Clear();
}

} // namespace tesseract
