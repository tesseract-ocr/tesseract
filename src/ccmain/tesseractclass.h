///////////////////////////////////////////////////////////////////////
// File:        tesseractclass.h
// Description: The Tesseract class. It holds/owns everything needed
//              to run Tesseract on a single language, and also a set of
//              sub-Tesseracts to run sub-languages. For thread safety, *every*
//              global variable goes in here, directly, or indirectly.
//              This makes it safe to run multiple Tesseracts in different
//              threads in parallel, and keeps the different language
//              instances separate.
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

#ifndef TESSERACT_CCMAIN_TESSERACTCLASS_H_
#define TESSERACT_CCMAIN_TESSERACTCLASS_H_

#ifdef HAVE_CONFIG_H
#  include "config_auto.h" // DISABLED_LEGACY_ENGINE
#endif

#include "control.h"               // for ACCEPTABLE_WERD_TYPE
#include "debugpixa.h"             // for DebugPixa
#include "devanagari_processing.h" // for ShiroRekhaSplitter
#ifndef DISABLED_LEGACY_ENGINE
#  include "docqual.h" // for GARBAGE_LEVEL
#endif
#include "genericvector.h"   // for PointerVector
#include "pageres.h"         // for WERD_RES (ptr only), PAGE_RES (pt...
#include "params.h"          // for BOOL_VAR_H, BoolParam, DoubleParam
#include "points.h"          // for FCOORD
#include "ratngs.h"          // for ScriptPos, WERD_CHOICE (ptr only)
#include "tessdatamanager.h" // for TessdataManager
#include "textord.h"         // for Textord
#include "wordrec.h"         // for Wordrec

#include <tesseract/publictypes.h> // for OcrEngineMode, PageSegMode, OEM_L...
#include <tesseract/unichar.h>     // for UNICHAR_ID

#include <allheaders.h> // for pixDestroy, pixGetWidth, pixGetHe...

#include <cstdint> // for int16_t, int32_t, uint16_t
#include <cstdio>  // for FILE

namespace tesseract {

class BLOCK_LIST;
class ETEXT_DESC;
struct OSResults;
class PAGE_RES;
class PAGE_RES_IT;
class ROW;
class SVMenuNode;
class TBOX;
class TO_BLOCK_LIST;
class WERD;
class WERD_CHOICE;
class WERD_RES;

class ColumnFinder;
class DocumentData;
#ifndef DISABLED_LEGACY_ENGINE
class EquationDetect;
#endif // ndef DISABLED_LEGACY_ENGINE
class ImageData;
class LSTMRecognizer;
class Tesseract;

// Top-level class for all tesseract global instance data.
// This class either holds or points to all data used by an instance
// of Tesseract, including the memory allocator. When this is
// complete, Tesseract will be thread-safe. UNTIL THEN, IT IS NOT!
//
// NOTE to developers: Do not create cyclic dependencies through this class!
// The directory dependency tree must remain a tree! To keep this clean,
// lower-level code (eg in ccutil, the bottom level) must never need to
// know about the content of a higher-level directory.
// The following scheme will grant the easiest access to lower-level
// global members without creating a cyclic dependency:
//
// Class Hierarchy (^ = inheritance):
//
//             CCUtil (ccutil/ccutil.h)
//                         ^      Members include: UNICHARSET
//           CCStruct (ccstruct/ccstruct.h)
//                         ^       Members include: Image
//           Classify (classify/classify.h)
//                         ^       Members include: Dict
//             WordRec (wordrec/wordrec.h)
//                         ^       Members include: WERD*, DENORM*
//        Tesseract (ccmain/tesseractclass.h)
//                                 Members include: Pix*
//
// Other important classes:
//
//  TessBaseAPI (tesseract/baseapi.h)
//                                 Members include: BLOCK_LIST*, PAGE_RES*,
//                                 Tesseract*, ImageThresholder*
//  Dict (dict/dict.h)
//                                 Members include: Image* (private)
//
// NOTE: that each level contains members that correspond to global
// data that is defined (and used) at that level, not necessarily where
// the type is defined so for instance:
// BOOL_VAR_H(textord_show_blobs);
// goes inside the Textord class, not the cc_util class.

// A collection of various variables for statistics and debugging.
struct TesseractStats {
  TesseractStats()
      : adaption_word_number(0)
      , doc_blob_quality(0)
      , doc_outline_errs(0)
      , doc_char_quality(0)
      , good_char_count(0)
      , doc_good_char_quality(0)
      , word_count(0)
      , dict_words(0)
      , tilde_crunch_written(false)
      , last_char_was_newline(true)
      , last_char_was_tilde(false)
      , write_results_empty_block(true) {}

  int32_t adaption_word_number;
  int16_t doc_blob_quality;
  int16_t doc_outline_errs;
  int16_t doc_char_quality;
  int16_t good_char_count;
  int16_t doc_good_char_quality;
  int32_t word_count;    // count of word in the document
  int32_t dict_words;    // number of dicitionary words in the document
  std::string dump_words_str; // accumulator used by dump_words()
  // Flags used by write_results()
  bool tilde_crunch_written;
  bool last_char_was_newline;
  bool last_char_was_tilde;
  bool write_results_empty_block;
};

// Struct to hold all the pointers to relevant data for processing a word.
struct WordData {
  WordData() : word(nullptr), row(nullptr), block(nullptr), prev_word(nullptr) {}
  explicit WordData(const PAGE_RES_IT &page_res_it)
      : word(page_res_it.word())
      , row(page_res_it.row()->row)
      , block(page_res_it.block()->block)
      , prev_word(nullptr) {}
  WordData(BLOCK *block_in, ROW *row_in, WERD_RES *word_res)
      : word(word_res), row(row_in), block(block_in), prev_word(nullptr) {}

  WERD_RES *word;
  ROW *row;
  BLOCK *block;
  WordData *prev_word;
  PointerVector<WERD_RES> lang_words;
};

// Definition of a Tesseract WordRecognizer. The WordData provides the context
// of row/block, in_word holds an initialized, possibly pre-classified word,
// that the recognizer may or may not consume (but if so it sets
// *in_word=nullptr) and produces one or more output words in out_words, which
// may be the consumed in_word, or may be generated independently. This api
// allows both a conventional tesseract classifier to work, or a line-level
// classifier that generates multiple words from a merged input.
using WordRecognizer = void (Tesseract::*)(const WordData &, WERD_RES **,
                                           PointerVector<WERD_RES> *);

class TESS_API Tesseract : public Wordrec {
public:
  Tesseract();
  ~Tesseract() override;

  // Return appropriate dictionary
  Dict &getDict() override;

  // Clear as much used memory as possible without resetting the adaptive
  // classifier or losing any other classifier data.
  void Clear();
  // Clear all memory of adaption for this and all subclassifiers.
  void ResetAdaptiveClassifier();
  // Clear the document dictionary for this and all subclassifiers.
  void ResetDocumentDictionary();

#ifndef DISABLED_LEGACY_ENGINE
  // Set the equation detector.
  void SetEquationDetect(EquationDetect *detector);
#endif // ndef DISABLED_LEGACY_ENGINE

  // Simple accessors.
  const FCOORD &reskew() const {
    return reskew_;
  }
  // Destroy any existing pix and return a pointer to the pointer.
  Image *mutable_pix_binary() {
    pix_binary_.destroy();
    return &pix_binary_;
  }
  Image pix_binary() const {
    return pix_binary_;
  }
  Image pix_grey() const {
    return pix_grey_;
  }
  void set_pix_grey(Image grey_pix) {
    pix_grey_.destroy();
    pix_grey_ = grey_pix;
  }
  Image pix_original() const {
    return pix_original_;
  }
  // Takes ownership of the given original_pix.
  void set_pix_original(Image original_pix) {
    pix_original_.destroy();
    pix_original_ = original_pix;
    // Clone to sublangs as well.
    for (auto &lang : sub_langs_) {
      lang->set_pix_original(original_pix ? original_pix.clone() : nullptr);
    }
  }
  // Returns a pointer to a Pix representing the best available resolution image
  // of the page, with best available bit depth as second priority. Result can
  // be of any bit depth, but never color-mapped, as that has always been
  // removed. Note that in grey and color, 0 is black and 255 is
  // white. If the input was binary, then black is 1 and white is 0.
  // To tell the difference pixGetDepth() will return 32, 8 or 1.
  // In any case, the return value is a borrowed Pix, and should not be
  // deleted or pixDestroyed.
  Image BestPix() const {
    if (pixGetWidth(pix_original_) == ImageWidth()) {
      return pix_original_;
    } else if (pix_grey_ != nullptr) {
      return pix_grey_;
    } else {
      return pix_binary_;
    }
  }
  void set_pix_thresholds(Image thresholds) {
    pix_thresholds_.destroy();
    pix_thresholds_ = thresholds;
  }
  int source_resolution() const {
    return source_resolution_;
  }
  void set_source_resolution(int ppi) {
    source_resolution_ = ppi;
  }
  int ImageWidth() const {
    return pixGetWidth(pix_binary_);
  }
  int ImageHeight() const {
    return pixGetHeight(pix_binary_);
  }
  Image scaled_color() const {
    return scaled_color_;
  }
  int scaled_factor() const {
    return scaled_factor_;
  }
  void SetScaledColor(int factor, Image color) {
    scaled_factor_ = factor;
    scaled_color_ = color;
  }
  const Textord &textord() const {
    return textord_;
  }
  Textord *mutable_textord() {
    return &textord_;
  }

  bool right_to_left() const {
    return right_to_left_;
  }
  int num_sub_langs() const {
    return sub_langs_.size();
  }
  Tesseract *get_sub_lang(int index) const {
    return sub_langs_[index];
  }
  // Returns true if any language uses Tesseract (as opposed to LSTM).
  bool AnyTessLang() const {
    if (tessedit_ocr_engine_mode != OEM_LSTM_ONLY) {
      return true;
    }
    for (auto &lang : sub_langs_) {
      if (lang->tessedit_ocr_engine_mode != OEM_LSTM_ONLY) {
        return true;
      }
    }
    return false;
  }
  // Returns true if any language uses the LSTM.
  bool AnyLSTMLang() const {
    if (tessedit_ocr_engine_mode != OEM_TESSERACT_ONLY) {
      return true;
    }
    for (auto &lang : sub_langs_) {
      if (lang->tessedit_ocr_engine_mode != OEM_TESSERACT_ONLY) {
        return true;
      }
    }
    return false;
  }

  void SetBlackAndWhitelist();

  // Perform steps to prepare underlying binary image/other data structures for
  // page segmentation. Uses the strategy specified in the global variable
  // pageseg_devanagari_split_strategy for perform splitting while preparing for
  // page segmentation.
  void PrepareForPageseg();

  // Perform steps to prepare underlying binary image/other data structures for
  // Tesseract OCR. The current segmentation is required by this method.
  // Uses the strategy specified in the global variable
  // ocr_devanagari_split_strategy for performing splitting while preparing for
  // Tesseract ocr.
  void PrepareForTessOCR(BLOCK_LIST *block_list, Tesseract *osd_tess, OSResults *osr);

  int SegmentPage(const char *input_file, BLOCK_LIST *blocks, Tesseract *osd_tess, OSResults *osr);
  void SetupWordScripts(BLOCK_LIST *blocks);
  int AutoPageSeg(PageSegMode pageseg_mode, BLOCK_LIST *blocks, TO_BLOCK_LIST *to_blocks,
                  BLOBNBOX_LIST *diacritic_blobs, Tesseract *osd_tess, OSResults *osr);
  ColumnFinder *SetupPageSegAndDetectOrientation(PageSegMode pageseg_mode, BLOCK_LIST *blocks,
                                                 Tesseract *osd_tess, OSResults *osr,
                                                 TO_BLOCK_LIST *to_blocks, Image *photo_mask_pix,
                                                 Image *music_mask_pix);
  // par_control.cpp
  void PrerecAllWordsPar(const std::vector<WordData> &words);

  //// linerec.cpp
  // Generates training data for training a line recognizer, eg LSTM.
  // Breaks the page into lines, according to the boxes, and writes them to a
  // serialized DocumentData based on output_basename.
  // Return true if successful, false if an error occurred.
  bool TrainLineRecognizer(const char *input_imagename, const std::string &output_basename,
                           BLOCK_LIST *block_list);
  // Generates training data for training a line recognizer, eg LSTM.
  // Breaks the boxes into lines, normalizes them, converts to ImageData and
  // appends them to the given training_data.
  void TrainFromBoxes(const std::vector<TBOX> &boxes, const std::vector<std::string> &texts,
                      BLOCK_LIST *block_list, DocumentData *training_data);

  // Returns an Imagedata containing the image of the given textline,
  // and ground truth boxes/truth text if available in the input.
  // The image is not normalized in any way.
  ImageData *GetLineData(const TBOX &line_box, const std::vector<TBOX> &boxes,
                         const std::vector<std::string> &texts, int start_box, int end_box,
                         const BLOCK &block);
  // Helper gets the image of a rectangle, using the block.re_rotation() if
  // needed to get to the image, and rotating the result back to horizontal
  // layout. (CJK characters will be on their left sides) The vertical text flag
  // is set in the returned ImageData if the text was originally vertical, which
  // can be used to invoke a different CJK recognition engine. The revised_box
  // is also returned to enable calculation of output bounding boxes.
  ImageData *GetRectImage(const TBOX &box, const BLOCK &block, int padding,
                          TBOX *revised_box) const;
  // Recognizes a word or group of words, converting to WERD_RES in *words.
  // Analogous to classify_word_pass1, but can handle a group of words as well.
  void LSTMRecognizeWord(const BLOCK &block, ROW *row, WERD_RES *word,
                         PointerVector<WERD_RES> *words);
  // Apply segmentation search to the given set of words, within the constraints
  // of the existing ratings matrix. If there is already a best_choice on a word
  // leaves it untouched and just sets the done/accepted etc flags.
  void SearchWords(PointerVector<WERD_RES> *words);

  //// control.h /////////////////////////////////////////////////////////
  bool ProcessTargetWord(const TBOX &word_box, const TBOX &target_word_box, const char *word_config,
                         int pass);
  // Sets up the words ready for whichever engine is to be run
  void SetupAllWordsPassN(int pass_n, const TBOX *target_word_box, const char *word_config,
                          PAGE_RES *page_res, std::vector<WordData> *words);
  // Sets up the single word ready for whichever engine is to be run.
  void SetupWordPassN(int pass_n, WordData *word);
  // Runs word recognition on all the words.
  bool RecogAllWordsPassN(int pass_n, ETEXT_DESC *monitor, PAGE_RES_IT *pr_it,
                          std::vector<WordData> *words);
  bool recog_all_words(PAGE_RES *page_res, ETEXT_DESC *monitor, const TBOX *target_word_box,
                       const char *word_config, int dopasses);
  void rejection_passes(PAGE_RES *page_res, ETEXT_DESC *monitor, const TBOX *target_word_box,
                        const char *word_config);
  void bigram_correction_pass(PAGE_RES *page_res);
  void blamer_pass(PAGE_RES *page_res);
  // Sets script positions and detects smallcaps on all output words.
  void script_pos_pass(PAGE_RES *page_res);
  // Helper to recognize the word using the given (language-specific) tesseract.
  // Returns positive if this recognizer found more new best words than the
  // number kept from best_words.
  int RetryWithLanguage(const WordData &word_data, WordRecognizer recognizer, bool debug,
                        WERD_RES **in_word, PointerVector<WERD_RES> *best_words);
  // Moves good-looking "noise"/diacritics from the reject list to the main
  // blob list on the current word. Returns true if anything was done, and
  // sets make_next_word_fuzzy if blob(s) were added to the end of the word.
  bool ReassignDiacritics(int pass, PAGE_RES_IT *pr_it, bool *make_next_word_fuzzy);
  // Attempts to put noise/diacritic outlines into the blobs that they overlap.
  // Input: a set of noisy outlines that probably belong to the real_word.
  // Output: outlines that overlapped blobs are set to nullptr and put back into
  // the word, either in the blobs or in the reject list.
  void AssignDiacriticsToOverlappingBlobs(const std::vector<C_OUTLINE *> &outlines, int pass,
                                          WERD *real_word, PAGE_RES_IT *pr_it,
                                          std::vector<bool> *word_wanted,
                                          std::vector<bool> *overlapped_any_blob,
                                          std::vector<C_BLOB *> *target_blobs);
  // Attempts to assign non-overlapping outlines to their nearest blobs or
  // make new blobs out of them.
  void AssignDiacriticsToNewBlobs(const std::vector<C_OUTLINE *> &outlines, int pass,
                                  WERD *real_word, PAGE_RES_IT *pr_it,
                                  std::vector<bool> *word_wanted,
                                  std::vector<C_BLOB *> *target_blobs);
  // Starting with ok_outlines set to indicate which outlines overlap the blob,
  // chooses the optimal set (approximately) and returns true if any outlines
  // are desired, in which case ok_outlines indicates which ones.
  bool SelectGoodDiacriticOutlines(int pass, float certainty_threshold, PAGE_RES_IT *pr_it,
                                   C_BLOB *blob, const std::vector<C_OUTLINE *> &outlines,
                                   int num_outlines, std::vector<bool> *ok_outlines);
  // Classifies the given blob plus the outlines flagged by ok_outlines, undoes
  // the inclusion of the outlines, and returns the certainty of the raw choice.
  float ClassifyBlobPlusOutlines(const std::vector<bool> &ok_outlines,
                                 const std::vector<C_OUTLINE *> &outlines, int pass_n,
                                 PAGE_RES_IT *pr_it, C_BLOB *blob, std::string &best_str);
  // Classifies the given blob (part of word_data->word->word) as an individual
  // word, using languages, chopper etc, returning only the certainty of the
  // best raw choice, and undoing all the work done to fake out the word.
  float ClassifyBlobAsWord(int pass_n, PAGE_RES_IT *pr_it, C_BLOB *blob, std::string &best_str,
                           float *c2);
  void classify_word_and_language(int pass_n, PAGE_RES_IT *pr_it, WordData *word_data);
  void classify_word_pass1(const WordData &word_data, WERD_RES **in_word,
                           PointerVector<WERD_RES> *out_words);
  void recog_pseudo_word(PAGE_RES *page_res, // blocks to check
                         TBOX &selection_box);

  void fix_rep_char(PAGE_RES_IT *page_res_it);

  ACCEPTABLE_WERD_TYPE acceptable_word_string(const UNICHARSET &char_set, const char *s,
                                              const char *lengths);
  void match_word_pass_n(int pass_n, WERD_RES *word, ROW *row, BLOCK *block);
  void classify_word_pass2(const WordData &word_data, WERD_RES **in_word,
                           PointerVector<WERD_RES> *out_words);
  void ReportXhtFixResult(bool accept_new_word, float new_x_ht, WERD_RES *word, WERD_RES *new_word);
  bool RunOldFixXht(WERD_RES *word, BLOCK *block, ROW *row);
  bool TrainedXheightFix(WERD_RES *word, BLOCK *block, ROW *row);
  // Runs recognition with the test baseline shift and x-height and returns true
  // if there was an improvement in recognition result.
  bool TestNewNormalization(int original_misfits, float baseline_shift, float new_x_ht,
                            WERD_RES *word, BLOCK *block, ROW *row);
  bool recog_interactive(PAGE_RES_IT *pr_it);

  // Set fonts of this word.
  void set_word_fonts(WERD_RES *word);
  void font_recognition_pass(PAGE_RES *page_res);
  void dictionary_correction_pass(PAGE_RES *page_res);
  bool check_debug_pt(WERD_RES *word, int location);

  //// superscript.cpp ////////////////////////////////////////////////////
  bool SubAndSuperscriptFix(WERD_RES *word_res);
  void GetSubAndSuperscriptCandidates(const WERD_RES *word, int *num_rebuilt_leading,
                                      ScriptPos *leading_pos, float *leading_certainty,
                                      int *num_rebuilt_trailing, ScriptPos *trailing_pos,
                                      float *trailing_certainty, float *avg_certainty,
                                      float *unlikely_threshold);
  WERD_RES *TrySuperscriptSplits(int num_chopped_leading, float leading_certainty,
                                 ScriptPos leading_pos, int num_chopped_trailing,
                                 float trailing_certainty, ScriptPos trailing_pos, WERD_RES *word,
                                 bool *is_good, int *retry_leading, int *retry_trailing);
  bool BelievableSuperscript(bool debug, const WERD_RES &word, float certainty_threshold,
                             int *left_ok, int *right_ok) const;

  //// output.h //////////////////////////////////////////////////////////

  void output_pass(PAGE_RES_IT &page_res_it, const TBOX *target_word_box);
  void write_results(PAGE_RES_IT &page_res_it, // full info
                     char newline_type,        // type of newline
                     bool force_eol            // override tilde crunch?
  );
  void set_unlv_suspects(WERD_RES *word);
  UNICHAR_ID get_rep_char(WERD_RES *word); // what char is repeated?
  bool acceptable_number_string(const char *s, const char *lengths);
  int16_t count_alphanums(const WERD_CHOICE &word);
  int16_t count_alphas(const WERD_CHOICE &word);

  void read_config_file(const char *filename, SetParamConstraint constraint);
  // Initialize for potentially a set of languages defined by the language
  // string and recursively any additional languages required by any language
  // traineddata file (via tessedit_load_sublangs in its config) that is loaded.
  // See init_tesseract_internal for args.
  int init_tesseract(const std::string &arg0, const std::string &textbase,
                     const std::string &language, OcrEngineMode oem, char **configs,
                     int configs_size, const std::vector<std::string> *vars_vec,
                     const std::vector<std::string> *vars_values, bool set_only_non_debug_params,
                     TessdataManager *mgr);
  int init_tesseract(const std::string &datapath, const std::string &language, OcrEngineMode oem) {
    TessdataManager mgr;
    return init_tesseract(datapath, {}, language, oem, nullptr, 0, nullptr, nullptr, false, &mgr);
  }
  // Common initialization for a single language.
  // arg0 is the datapath for the tessdata directory, which could be the
  // path of the tessdata directory with no trailing /, or (if tessdata
  // lives in the same directory as the executable, the path of the executable,
  // hence the name arg0.
  // textbase is an optional output file basename (used only for training)
  // language is the language code to load.
  // oem controls which engine(s) will operate on the image
  // configs (argv) is an array of config filenames to load variables from.
  // May be nullptr.
  // configs_size (argc) is the number of elements in configs.
  // vars_vec is an optional vector of variables to set.
  // vars_values is an optional corresponding vector of values for the variables
  // in vars_vec.
  // If set_only_non_debug_params is true, only params that do not contain
  // "debug" in the name will be set.
  int init_tesseract_internal(const std::string &arg0, const std::string &textbase,
                              const std::string &language, OcrEngineMode oem, char **configs,
                              int configs_size, const std::vector<std::string> *vars_vec,
                              const std::vector<std::string> *vars_values,
                              bool set_only_non_debug_params, TessdataManager *mgr);

  // Set the universal_id member of each font to be unique among all
  // instances of the same font loaded.
  void SetupUniversalFontIds();

  void recognize_page(std::string &image_name);
  void end_tesseract();

  bool init_tesseract_lang_data(const std::string &arg0,
                                const std::string &language, OcrEngineMode oem, char **configs,
                                int configs_size, const std::vector<std::string> *vars_vec,
                                const std::vector<std::string> *vars_values,
                                bool set_only_non_debug_params, TessdataManager *mgr);

  void ParseLanguageString(const std::string &lang_str, std::vector<std::string> *to_load,
                           std::vector<std::string> *not_to_load);

  //// pgedit.h //////////////////////////////////////////////////////////
  SVMenuNode *build_menu_new();
#ifndef GRAPHICS_DISABLED
  void pgeditor_main(int width, int height, PAGE_RES *page_res);

  void process_image_event( // action in image win
      const SVEvent &event);
  bool process_cmd_win_event( // UI command semantics
      int32_t cmd_event,      // which menu item?
      char *new_value         // any prompt data
  );
#endif // !GRAPHICS_DISABLED
  void debug_word(PAGE_RES *page_res, const TBOX &selection_box);
  void do_re_display(bool (tesseract::Tesseract::*word_painter)(PAGE_RES_IT *pr_it));
  bool word_display(PAGE_RES_IT *pr_it);
  bool word_bln_display(PAGE_RES_IT *pr_it);
  bool word_blank_and_set_display(PAGE_RES_IT *pr_its);
  bool word_set_display(PAGE_RES_IT *pr_it);
  // #ifndef GRAPHICS_DISABLED
  bool word_dumper(PAGE_RES_IT *pr_it);
  // #endif // !GRAPHICS_DISABLED
  void blob_feature_display(PAGE_RES *page_res, const TBOX &selection_box);
  //// reject.h //////////////////////////////////////////////////////////
  // make rej map for word
  void make_reject_map(WERD_RES *word, ROW *row, int16_t pass);
  bool one_ell_conflict(WERD_RES *word_res, bool update_map);
  int16_t first_alphanum_index(const char *word, const char *word_lengths);
  int16_t first_alphanum_offset(const char *word, const char *word_lengths);
  int16_t alpha_count(const char *word, const char *word_lengths);
  bool word_contains_non_1_digit(const char *word, const char *word_lengths);
  void dont_allow_1Il(WERD_RES *word);
  int16_t count_alphanums( // how many alphanums
      WERD_RES *word);
  void flip_0O(WERD_RES *word);
  bool non_0_digit(const UNICHARSET &ch_set, UNICHAR_ID unichar_id);
  bool non_O_upper(const UNICHARSET &ch_set, UNICHAR_ID unichar_id);
  bool repeated_nonalphanum_wd(WERD_RES *word, ROW *row);
  void nn_match_word( // Match a word
      WERD_RES *word, ROW *row);
  void nn_recover_rejects(WERD_RES *word, ROW *row);
  void set_done( // set done flag
      WERD_RES *word, int16_t pass);
  int16_t safe_dict_word(const WERD_RES *werd_res); // is best_choice in dict?
  void flip_hyphens(WERD_RES *word);
  void reject_I_1_L(WERD_RES *word);
  void reject_edge_blobs(WERD_RES *word);
  void reject_mostly_rejects(WERD_RES *word);
  //// adaptions.h ///////////////////////////////////////////////////////
  bool word_adaptable( // should we adapt?
      WERD_RES *word, uint16_t mode);

  //// tfacepp.cpp ///////////////////////////////////////////////////////
  void recog_word_recursive(WERD_RES *word);
  void recog_word(WERD_RES *word);
  void split_and_recog_word(WERD_RES *word);
  void split_word(WERD_RES *word, unsigned split_pt, WERD_RES **right_piece,
                  BlamerBundle **orig_blamer_bundle) const;
  void join_words(WERD_RES *word, WERD_RES *word2, BlamerBundle *orig_bb) const;
  //// fixspace.cpp ///////////////////////////////////////////////////////
  bool digit_or_numeric_punct(WERD_RES *word, int char_position);
  int16_t eval_word_spacing(WERD_RES_LIST &word_res_list);
  void match_current_words(WERD_RES_LIST &words, ROW *row, BLOCK *block);
  int16_t fp_eval_word_spacing(WERD_RES_LIST &word_res_list);
  void fix_noisy_space_list(WERD_RES_LIST &best_perm, ROW *row, BLOCK *block);
  void fix_fuzzy_space_list(WERD_RES_LIST &best_perm, ROW *row, BLOCK *block);
  void fix_sp_fp_word(WERD_RES_IT &word_res_it, ROW *row, BLOCK *block);
  void fix_fuzzy_spaces(   // find fuzzy words
      ETEXT_DESC *monitor, // progress monitor
      int32_t word_count,  // count of words in doc
      PAGE_RES *page_res);
  void dump_words(WERD_RES_LIST &perm, int16_t score, int16_t mode, bool improved);
  bool fixspace_thinks_word_done(WERD_RES *word);
  int16_t worst_noise_blob(WERD_RES *word_res, float *worst_noise_score);
  float blob_noise_score(TBLOB *blob);
  void break_noisiest_blob_word(WERD_RES_LIST &words);
  //// docqual.cpp ////////////////////////////////////////////////////////
#ifndef DISABLED_LEGACY_ENGINE
  GARBAGE_LEVEL garbage_word(WERD_RES *word, bool ok_dict_word);
  bool potential_word_crunch(WERD_RES *word, GARBAGE_LEVEL garbage_level, bool ok_dict_word);
#endif
  void tilde_crunch(PAGE_RES_IT &page_res_it);
  void unrej_good_quality_words( // unreject potential
      PAGE_RES_IT &page_res_it);
  void doc_and_block_rejection( // reject big chunks
      PAGE_RES_IT &page_res_it, bool good_quality_doc);
  void quality_based_rejection(PAGE_RES_IT &page_res_it, bool good_quality_doc);
  void convert_bad_unlv_chs(WERD_RES *word_res);
  void tilde_delete(PAGE_RES_IT &page_res_it);
  int16_t word_blob_quality(WERD_RES *word);
  void word_char_quality(WERD_RES *word, int16_t *match_count, int16_t *accepted_match_count);
  void unrej_good_chs(WERD_RES *word);
  int16_t count_outline_errs(char c, int16_t outline_count);
  int16_t word_outline_errs(WERD_RES *word);
#ifndef DISABLED_LEGACY_ENGINE
  bool terrible_word_crunch(WERD_RES *word, GARBAGE_LEVEL garbage_level);
#endif
  CRUNCH_MODE word_deletable(WERD_RES *word, int16_t &delete_mode);
  int16_t failure_count(WERD_RES *word);
  bool noise_outlines(TWERD *word);
  //// pagewalk.cpp ///////////////////////////////////////////////////////
  void process_selected_words(PAGE_RES *page_res, // blocks to check
                                                  // function to call
                              TBOX &selection_box,
                              bool (tesseract::Tesseract::*word_processor)(PAGE_RES_IT *pr_it));
  //// tessbox.cpp ///////////////////////////////////////////////////////
  void tess_add_doc_word(      // test acceptability
      WERD_CHOICE *word_choice // after context
  );
  void tess_segment_pass_n(int pass_n, WERD_RES *word);
  bool tess_acceptable_word(WERD_RES *word);

  //// applybox.cpp //////////////////////////////////////////////////////
  // Applies the box file based on the image name filename, and resegments
  // the words in the block_list (page), with:
  // blob-mode: one blob per line in the box file, words as input.
  // word/line-mode: one blob per space-delimited unit after the #, and one word
  // per line in the box file. (See comment above for box file format.)
  // If find_segmentation is true, (word/line mode) then the classifier is used
  // to re-segment words/lines to match the space-delimited truth string for
  // each box. In this case, the input box may be for a word or even a whole
  // text line, and the output words will contain multiple blobs corresponding
  // to the space-delimited input string.
  // With find_segmentation false, no classifier is needed, but the chopper
  // can still be used to correctly segment touching characters with the help
  // of the input boxes.
  // In the returned PAGE_RES, the WERD_RES are setup as they would be returned
  // from normal classification, ie. with a word, chopped_word, rebuild_word,
  // seam_array, denorm, box_word, and best_state, but NO best_choice or
  // raw_choice, as they would require a UNICHARSET, which we aim to avoid.
  // Instead, the correct_text member of WERD_RES is set, and this may be later
  // converted to a best_choice using CorrectClassifyWords. CorrectClassifyWords
  // is not required before calling ApplyBoxTraining.
  PAGE_RES *ApplyBoxes(const char *filename, bool find_segmentation, BLOCK_LIST *block_list);

  // Any row xheight that is significantly different from the median is set
  // to the median.
  void PreenXHeights(BLOCK_LIST *block_list);

  // Builds a PAGE_RES from the block_list in the way required for ApplyBoxes:
  // All fuzzy spaces are removed, and all the words are maximally chopped.
  PAGE_RES *SetupApplyBoxes(const std::vector<TBOX> &boxes, BLOCK_LIST *block_list);
  // Tests the chopper by exhaustively running chop_one_blob.
  // The word_res will contain filled chopped_word, seam_array, denorm,
  // box_word and best_state for the maximally chopped word.
  void MaximallyChopWord(const std::vector<TBOX> &boxes, BLOCK *block, ROW *row,
                         WERD_RES *word_res);
  // Gather consecutive blobs that match the given box into the best_state
  // and corresponding correct_text.
  // Fights over which box owns which blobs are settled by pre-chopping and
  // applying the blobs to box or next_box with the least non-overlap.
  // Returns false if the box was in error, which can only be caused by
  // failing to find an appropriate blob for a box.
  // This means that occasionally, blobs may be incorrectly segmented if the
  // chopper fails to find a suitable chop point.
  bool ResegmentCharBox(PAGE_RES *page_res, const TBOX *prev_box, const TBOX &box,
                        const TBOX *next_box, const char *correct_text);
  // Consume all source blobs that strongly overlap the given box,
  // putting them into a new word, with the correct_text label.
  // Fights over which box owns which blobs are settled by
  // applying the blobs to box or next_box with the least non-overlap.
  // Returns false if the box was in error, which can only be caused by
  // failing to find an overlapping blob for a box.
  bool ResegmentWordBox(BLOCK_LIST *block_list, const TBOX &box, const TBOX *next_box,
                        const char *correct_text);
  // Resegments the words by running the classifier in an attempt to find the
  // correct segmentation that produces the required string.
  void ReSegmentByClassification(PAGE_RES *page_res);
  // Converts the space-delimited string of utf8 text to a vector of UNICHAR_ID.
  // Returns false if an invalid UNICHAR_ID is encountered.
  bool ConvertStringToUnichars(const char *utf8, std::vector<UNICHAR_ID> *class_ids);
  // Resegments the word to achieve the target_text from the classifier.
  // Returns false if the re-segmentation fails.
  // Uses brute-force combination of up to kMaxGroupSize adjacent blobs, and
  // applies a full search on the classifier results to find the best classified
  // segmentation. As a compromise to obtain better recall, 1-1 ambigiguity
  // substitutions ARE used.
  bool FindSegmentation(const std::vector<UNICHAR_ID> &target_text, WERD_RES *word_res);
  // Recursive helper to find a match to the target_text (from text_index
  // position) in the choices (from choices_pos position).
  // Choices is an array of vectors of length choices_length, with each
  // element representing a starting position in the word, and the
  // vector holding classification results for a sequence of consecutive
  // blobs, with index 0 being a single blob, index 1 being 2 blobs etc.
  void SearchForText(const std::vector<BLOB_CHOICE_LIST *> *choices, int choices_pos,
                     unsigned choices_length, const std::vector<UNICHAR_ID> &target_text,
                     unsigned text_index, float rating, std::vector<int> *segmentation,
                     float *best_rating, std::vector<int> *best_segmentation);
  // Counts up the labelled words and the blobs within.
  // Deletes all unused or emptied words, counting the unused ones.
  // Resets W_BOL and W_EOL flags correctly.
  // Builds the rebuild_word and rebuilds the box_word.
  void TidyUp(PAGE_RES *page_res);
  // Logs a bad box by line in the box file and box coords.
  void ReportFailedBox(int boxfile_lineno, TBOX box, const char *box_ch, const char *err_msg);
  // Creates a fake best_choice entry in each WERD_RES with the correct text.
  void CorrectClassifyWords(PAGE_RES *page_res);
  // Call LearnWord to extract features for labelled blobs within each word.
  // Features are stored in an internal buffer.
  void ApplyBoxTraining(const std::string &fontname, PAGE_RES *page_res);

  //// fixxht.cpp ///////////////////////////////////////////////////////
  // Returns the number of misfit blob tops in this word.
  int CountMisfitTops(WERD_RES *word_res);
  // Returns a new x-height in pixels (original image coords) that is
  // maximally compatible with the result in word_res.
  // Returns 0.0f if no x-height is found that is better than the current
  // estimate.
  float ComputeCompatibleXheight(WERD_RES *word_res, float *baseline_shift);
  //// Data members ///////////////////////////////////////////////////////
  // TODO(ocr-team): Find and remove obsolete parameters.
  BOOL_VAR_H(tessedit_resegment_from_boxes);
  BOOL_VAR_H(tessedit_resegment_from_line_boxes);
  BOOL_VAR_H(tessedit_train_from_boxes);
  BOOL_VAR_H(tessedit_make_boxes_from_boxes);
  BOOL_VAR_H(tessedit_train_line_recognizer);
  BOOL_VAR_H(tessedit_dump_pageseg_images);
  // TODO: remove deprecated tessedit_do_invert in release 6.
  BOOL_VAR_H(tessedit_do_invert);
  double_VAR_H(invert_threshold);
  INT_VAR_H(tessedit_pageseg_mode);
  INT_VAR_H(thresholding_method);
  BOOL_VAR_H(thresholding_debug);
  double_VAR_H(thresholding_window_size);
  double_VAR_H(thresholding_kfactor);
  double_VAR_H(thresholding_tile_size);
  double_VAR_H(thresholding_smooth_kernel_size);
  double_VAR_H(thresholding_score_fraction);
  INT_VAR_H(tessedit_ocr_engine_mode);
  STRING_VAR_H(tessedit_char_blacklist);
  STRING_VAR_H(tessedit_char_whitelist);
  STRING_VAR_H(tessedit_char_unblacklist);
  BOOL_VAR_H(tessedit_ambigs_training);
  INT_VAR_H(pageseg_devanagari_split_strategy);
  INT_VAR_H(ocr_devanagari_split_strategy);
  STRING_VAR_H(tessedit_write_params_to_file);
  BOOL_VAR_H(tessedit_adaption_debug);
  INT_VAR_H(bidi_debug);
  INT_VAR_H(applybox_debug);
  INT_VAR_H(applybox_page);
  STRING_VAR_H(applybox_exposure_pattern);
  BOOL_VAR_H(applybox_learn_chars_and_char_frags_mode);
  BOOL_VAR_H(applybox_learn_ngrams_mode);
  BOOL_VAR_H(tessedit_display_outwords);
  BOOL_VAR_H(tessedit_dump_choices);
  BOOL_VAR_H(tessedit_timing_debug);
  BOOL_VAR_H(tessedit_fix_fuzzy_spaces);
  BOOL_VAR_H(tessedit_unrej_any_wd);
  BOOL_VAR_H(tessedit_fix_hyphens);
  BOOL_VAR_H(tessedit_enable_doc_dict);
  BOOL_VAR_H(tessedit_debug_fonts);
  INT_VAR_H(tessedit_font_id);
  BOOL_VAR_H(tessedit_debug_block_rejection);
  BOOL_VAR_H(tessedit_enable_bigram_correction);
  BOOL_VAR_H(tessedit_enable_dict_correction);
  INT_VAR_H(tessedit_bigram_debug);
  BOOL_VAR_H(enable_noise_removal);
  INT_VAR_H(debug_noise_removal);
  // Worst (min) certainty, for which a diacritic is allowed to make the base
  // character worse and still be included.
  double_VAR_H(noise_cert_basechar);
  // Worst (min) certainty, for which a non-overlapping diacritic is allowed to
  // make the base character worse and still be included.
  double_VAR_H(noise_cert_disjoint);
  // Worst (min) certainty, for which a diacritic is allowed to make a new
  // stand-alone blob.
  double_VAR_H(noise_cert_punc);
  // Factor of certainty margin for adding diacritics to not count as worse.
  double_VAR_H(noise_cert_factor);
  INT_VAR_H(noise_maxperblob);
  INT_VAR_H(noise_maxperword);
  INT_VAR_H(debug_x_ht_level);
  STRING_VAR_H(chs_leading_punct);
  STRING_VAR_H(chs_trailing_punct1);
  STRING_VAR_H(chs_trailing_punct2);
  double_VAR_H(quality_rej_pc);
  double_VAR_H(quality_blob_pc);
  double_VAR_H(quality_outline_pc);
  double_VAR_H(quality_char_pc);
  INT_VAR_H(quality_min_initial_alphas_reqd);
  INT_VAR_H(tessedit_tess_adaption_mode);
  BOOL_VAR_H(tessedit_minimal_rej_pass1);
  BOOL_VAR_H(tessedit_test_adaption);
  BOOL_VAR_H(test_pt);
  double_VAR_H(test_pt_x);
  double_VAR_H(test_pt_y);
  INT_VAR_H(multilang_debug_level);
  INT_VAR_H(paragraph_debug_level);
  BOOL_VAR_H(paragraph_text_based);
  BOOL_VAR_H(lstm_use_matrix);
  STRING_VAR_H(outlines_odd);
  STRING_VAR_H(outlines_2);
  BOOL_VAR_H(tessedit_good_quality_unrej);
  BOOL_VAR_H(tessedit_use_reject_spaces);
  double_VAR_H(tessedit_reject_doc_percent);
  double_VAR_H(tessedit_reject_block_percent);
  double_VAR_H(tessedit_reject_row_percent);
  double_VAR_H(tessedit_whole_wd_rej_row_percent);
  BOOL_VAR_H(tessedit_preserve_blk_rej_perfect_wds);
  BOOL_VAR_H(tessedit_preserve_row_rej_perfect_wds);
  BOOL_VAR_H(tessedit_dont_blkrej_good_wds);
  BOOL_VAR_H(tessedit_dont_rowrej_good_wds);
  INT_VAR_H(tessedit_preserve_min_wd_len);
  BOOL_VAR_H(tessedit_row_rej_good_docs);
  double_VAR_H(tessedit_good_doc_still_rowrej_wd);
  BOOL_VAR_H(tessedit_reject_bad_qual_wds);
  BOOL_VAR_H(tessedit_debug_doc_rejection);
  BOOL_VAR_H(tessedit_debug_quality_metrics);
  BOOL_VAR_H(bland_unrej);
  double_VAR_H(quality_rowrej_pc);
  BOOL_VAR_H(unlv_tilde_crunching);
  BOOL_VAR_H(hocr_font_info);
  BOOL_VAR_H(hocr_char_boxes);
  BOOL_VAR_H(crunch_early_merge_tess_fails);
  BOOL_VAR_H(crunch_early_convert_bad_unlv_chs);
  double_VAR_H(crunch_terrible_rating);
  BOOL_VAR_H(crunch_terrible_garbage);
  double_VAR_H(crunch_poor_garbage_cert);
  double_VAR_H(crunch_poor_garbage_rate);
  double_VAR_H(crunch_pot_poor_rate);
  double_VAR_H(crunch_pot_poor_cert);
  double_VAR_H(crunch_del_rating);
  double_VAR_H(crunch_del_cert);
  double_VAR_H(crunch_del_min_ht);
  double_VAR_H(crunch_del_max_ht);
  double_VAR_H(crunch_del_min_width);
  double_VAR_H(crunch_del_high_word);
  double_VAR_H(crunch_del_low_word);
  double_VAR_H(crunch_small_outlines_size);
  INT_VAR_H(crunch_rating_max);
  INT_VAR_H(crunch_pot_indicators);
  BOOL_VAR_H(crunch_leave_ok_strings);
  BOOL_VAR_H(crunch_accept_ok);
  BOOL_VAR_H(crunch_leave_accept_strings);
  BOOL_VAR_H(crunch_include_numerals);
  INT_VAR_H(crunch_leave_lc_strings);
  INT_VAR_H(crunch_leave_uc_strings);
  INT_VAR_H(crunch_long_repetitions);
  INT_VAR_H(crunch_debug);
  INT_VAR_H(fixsp_non_noise_limit);
  double_VAR_H(fixsp_small_outlines_size);
  BOOL_VAR_H(tessedit_prefer_joined_punct);
  INT_VAR_H(fixsp_done_mode);
  INT_VAR_H(debug_fix_space_level);
  STRING_VAR_H(numeric_punctuation);
  INT_VAR_H(x_ht_acceptance_tolerance);
  INT_VAR_H(x_ht_min_change);
  INT_VAR_H(superscript_debug);
  double_VAR_H(superscript_worse_certainty);
  double_VAR_H(superscript_bettered_certainty);
  double_VAR_H(superscript_scaledown_ratio);
  double_VAR_H(subscript_max_y_top);
  double_VAR_H(superscript_min_y_bottom);
  BOOL_VAR_H(tessedit_write_block_separators);
  BOOL_VAR_H(tessedit_write_rep_codes);
  BOOL_VAR_H(tessedit_write_unlv);
  BOOL_VAR_H(tessedit_create_txt);
  BOOL_VAR_H(tessedit_create_hocr);
  BOOL_VAR_H(tessedit_create_alto);
  BOOL_VAR_H(tessedit_create_lstmbox);
  BOOL_VAR_H(tessedit_create_tsv);
  BOOL_VAR_H(tessedit_create_wordstrbox);
  BOOL_VAR_H(tessedit_create_pdf);
  BOOL_VAR_H(textonly_pdf);
  INT_VAR_H(jpg_quality);
  INT_VAR_H(user_defined_dpi);
  INT_VAR_H(min_characters_to_try);
  STRING_VAR_H(unrecognised_char);
  INT_VAR_H(suspect_level);
  INT_VAR_H(suspect_short_words);
  BOOL_VAR_H(suspect_constrain_1Il);
  double_VAR_H(suspect_rating_per_ch);
  double_VAR_H(suspect_accept_rating);
  BOOL_VAR_H(tessedit_minimal_rejection);
  BOOL_VAR_H(tessedit_zero_rejection);
  BOOL_VAR_H(tessedit_word_for_word);
  BOOL_VAR_H(tessedit_zero_kelvin_rejection);
  INT_VAR_H(tessedit_reject_mode);
  BOOL_VAR_H(tessedit_rejection_debug);
  BOOL_VAR_H(tessedit_flip_0O);
  double_VAR_H(tessedit_lower_flip_hyphen);
  double_VAR_H(tessedit_upper_flip_hyphen);
  BOOL_VAR_H(rej_trust_doc_dawg);
  BOOL_VAR_H(rej_1Il_use_dict_word);
  BOOL_VAR_H(rej_1Il_trust_permuter_type);
  BOOL_VAR_H(rej_use_tess_accepted);
  BOOL_VAR_H(rej_use_tess_blanks);
  BOOL_VAR_H(rej_use_good_perm);
  BOOL_VAR_H(rej_use_sensible_wd);
  BOOL_VAR_H(rej_alphas_in_number_perm);
  double_VAR_H(rej_whole_of_mostly_reject_word_fract);
  INT_VAR_H(tessedit_image_border);
  STRING_VAR_H(ok_repeated_ch_non_alphanum_wds);
  STRING_VAR_H(conflict_set_I_l_1);
  INT_VAR_H(min_sane_x_ht_pixels);
  BOOL_VAR_H(tessedit_create_boxfile);
  INT_VAR_H(tessedit_page_number);
  BOOL_VAR_H(tessedit_write_images);
  BOOL_VAR_H(interactive_display_mode);
  STRING_VAR_H(file_type);
  BOOL_VAR_H(tessedit_override_permuter);
  STRING_VAR_H(tessedit_load_sublangs);
  BOOL_VAR_H(tessedit_use_primary_params_model);
  // Min acceptable orientation margin (difference in scores between top and 2nd
  // choice in OSResults::orientations) to believe the page orientation.
  double_VAR_H(min_orientation_margin);
  BOOL_VAR_H(textord_tabfind_show_vlines);
  BOOL_VAR_H(textord_use_cjk_fp_model);
  BOOL_VAR_H(poly_allow_detailed_fx);
  BOOL_VAR_H(tessedit_init_config_only);
#ifndef DISABLED_LEGACY_ENGINE
  BOOL_VAR_H(textord_equation_detect);
#endif // ndef DISABLED_LEGACY_ENGINE
  BOOL_VAR_H(textord_tabfind_vertical_text);
  BOOL_VAR_H(textord_tabfind_force_vertical_text);
  double_VAR_H(textord_tabfind_vertical_text_ratio);
  double_VAR_H(textord_tabfind_aligned_gap_fraction);
  INT_VAR_H(tessedit_parallelize);
  BOOL_VAR_H(preserve_interword_spaces);
  STRING_VAR_H(page_separator);
  INT_VAR_H(lstm_choice_mode);
  INT_VAR_H(lstm_choice_iterations);
  double_VAR_H(lstm_rating_coefficient);
  BOOL_VAR_H(pageseg_apply_music_mask);

  //// ambigsrecog.cpp /////////////////////////////////////////////////////////
  FILE *init_recog_training(const char *filename);
  void recog_training_segmented(const char *filename, PAGE_RES *page_res,
                                volatile ETEXT_DESC *monitor, FILE *output_file);
  void ambigs_classify_and_output(const char *label, PAGE_RES_IT *pr_it, FILE *output_file);

private:
  // The filename of a backup config file. If not null, then we currently
  // have a temporary debug config file loaded, and backup_config_file_
  // will be loaded, and set to null when debug is complete.
  const char *backup_config_file_;
  // The filename of a config file to read when processing a debug word.
  std::string word_config_;
  // Image used for input to layout analysis and tesseract recognition.
  // May be modified by the ShiroRekhaSplitter to eliminate the top-line.
  Image pix_binary_;
  // Grey-level input image if the input was not binary, otherwise nullptr.
  Image pix_grey_;
  // Original input image. Color if the input was color.
  Image pix_original_;
  // Thresholds that were used to generate the thresholded image from grey.
  Image pix_thresholds_;
  // Debug images. If non-empty, will be written on destruction.
  DebugPixa pixa_debug_;
  // Input image resolution after any scaling. The resolution is not well
  // transmitted by operations on Pix, so we keep an independent record here.
  int source_resolution_;
  // The shiro-rekha splitter object which is used to split top-lines in
  // Devanagari words to provide a better word and grapheme segmentation.
  ShiroRekhaSplitter splitter_;
  // Page segmentation/layout
  Textord textord_;
  // True if the primary language uses right_to_left reading order.
  bool right_to_left_;
  Image scaled_color_;
  int scaled_factor_;
  FCOORD deskew_;
  FCOORD reskew_;
  TesseractStats stats_;
  // Sub-languages to be tried in addition to this.
  std::vector<Tesseract *> sub_langs_;
  // Most recently used Tesseract out of this and sub_langs_. The default
  // language for the next word.
  Tesseract *most_recently_used_;
  // The size of the font table, ie max possible font id + 1.
  int font_table_size_;
#ifndef DISABLED_LEGACY_ENGINE
  // Equation detector. Note: this pointer is NOT owned by the class.
  EquationDetect *equ_detect_;
#endif // ndef DISABLED_LEGACY_ENGINE
  // LSTM recognizer, if available.
  LSTMRecognizer *lstm_recognizer_;
  // Output "page" number (actually line number) using TrainLineRecognizer.
  int train_line_page_num_;
};

} // namespace tesseract

#endif // TESSERACT_CCMAIN_TESSERACTCLASS_H_
