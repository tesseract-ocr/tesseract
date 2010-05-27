/**********************************************************************
 * File:        baseapi.cpp
 * Description: Simple API for calling tesseract.
 * Author:      Ray Smith
 * Created:     Fri Oct 06 15:35:01 PDT 2006
 *
 * (C) Copyright 2006, Google Inc.
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifdef HAVE_LIBLEPT
// Include leptonica library only if autoconf (or makefile etc) tell us to.
#include "allheaders.h"
#endif

#include "baseapi.h"

#include "thresholder.h"
#include "tesseractmain.h"
#include "tesseractclass.h"
#include "tessedit.h"
#include "ocrclass.h"
#include "pageres.h"
#include "tessvars.h"
#include "control.h"
#include "applybox.h"
#include "pgedit.h"
#include "varabled.h"
#include "output.h"
#include "mainblk.h"
#include "globals.h"
#include "adaptmatch.h"
#include "edgblob.h"
#include "tessbox.h"
#include "tordvars.h"
#include "imgs.h"
#include "makerow.h"
#include "tstruct.h"
#include "tessout.h"
#include "tface.h"
#include "permute.h"
#include "otsuthr.h"
#include "osdetect.h"
#include "chopper.h"
#include "matchtab.h"

namespace tesseract {

// Minimum sensible image size to be worth running tesseract.
const int kMinRectSize = 10;
// Character returned when Tesseract couldn't recognize as anything.
const char kTesseractReject = '~';
// Character used by UNLV error counter as a reject.
const char kUNLVReject = '~';
// Character used by UNLV as a suspect marker.
const char kUNLVSuspect = '^';
// Filename used for input image file, from which to derive a name to search
// for a possible UNLV zone file, if none is specified by SetInputName.
const char* kInputFile = "noname.tif";

TessBaseAPI::TessBaseAPI()
  : tesseract_(NULL),
    // Thresholder is initialized to NULL here, but will be set before use by:
    // A constructor of a derived API,  SetThresholder(), or
    // created implicitly when used in InternalSetImage.
    thresholder_(NULL),
    threshold_done_(false),
    block_list_(NULL),
    page_res_(NULL),
    input_file_(NULL),
    output_file_(NULL),
    datapath_(NULL),
    language_(NULL),
    rect_left_(0), rect_top_(0), rect_width_(0), rect_height_(0),
    image_width_(0), image_height_(0) {
}

TessBaseAPI::~TessBaseAPI() {
  End();
}

// Set the name of the input file. Needed only for training and
// loading a UNLV zone file.
void TessBaseAPI::SetInputName(const char* name) {
  if (input_file_ == NULL)
    input_file_ = new STRING(name);
  else
    *input_file_ = name;
}

// Set the name of the output files. Needed only for debugging.
void TessBaseAPI::SetOutputName(const char* name) {
  if (output_file_ == NULL)
    output_file_ = new STRING(name);
  else
    *output_file_ = name;
}

// Set the value of an internal "variable" (of either old or new types).
// Supply the name of the variable and the value as a string, just as
// you would in a config file.
// Returns false if the name lookup failed.
// SetVariable may be used before Init, to set things that control
// initialization, but note that on End all settings are lost and
// the next Init will use the defaults unless SetVariable is used again.
bool TessBaseAPI::SetVariable(const char* variable, const char* value) {
  if (tesseract_ == NULL)
    tesseract_ = new Tesseract;
  return set_variable(variable, value);
}

// The datapath must be the name of the data directory (no ending /) or
// some other file in which the data directory resides (for instance argv[0].)
// The language is (usually) an ISO 639-3 string or NULL will default to eng.
// If numeric_mode is true, then only digits and Roman numerals will
// be returned.
// Returns 0 on success and -1 on initialization failure.
int TessBaseAPI::Init(const char* datapath, const char* language,
                      char **configs, int configs_size,
                      bool configs_global_only) {
  // If the datapath or the language have changed, then start again.
  // Note that the language_ field stores the last requested language that was
  // initialized successfully, while tesseract_->lang stores the language
  // actually used. They differ only if the requested language was NULL, in
  // which case tesseract_->lang is set to the Tesseract default ("eng").
  if (tesseract_ != NULL &&
      (datapath_ == NULL || language_ == NULL || *datapath_ != datapath
       || (*language_ != language && tesseract_->lang != language))) {
    tesseract_->end_tesseract();
    delete tesseract_;
    tesseract_ = NULL;
  }

  bool reset_classifier = true;
  if (tesseract_ == NULL) {
    reset_classifier = false;
    tesseract_ = new Tesseract;
    if (tesseract_->init_tesseract(
            datapath, output_file_ != NULL ? output_file_->string() : NULL,
            language, configs, configs_size, configs_global_only) != 0) {
      return -1;
    }
  }
  // Update datapath and language requested for the last valid initialization.
  if (datapath_ == NULL)
    datapath_ = new STRING(datapath);
  else
    *datapath_ = datapath;
  if (language_ == NULL)
    language_ = new STRING(language);
  else
    *language_ = language;

  // For same language and datapath, just reset the adaptive classifier.
  if (reset_classifier) tesseract_->ResetAdaptiveClassifier();

  return 0;
}

// Init only the lang model component of Tesseract. The only functions
// that work after this init are SetVariable and IsValidWord.
// WARNING: temporary! This function will be removed from here and placed
// in a separate API at some future time.
int TessBaseAPI::InitLangMod(const char* datapath, const char* language) {
  if (tesseract_ == NULL)
    tesseract_ = new Tesseract;
  return tesseract_->init_tesseract_lm(datapath, NULL, language);
}

// Init only the classifer component of Tesseract. Used to initialize the
// specified language when no dawg models are available.
int TessBaseAPI::InitWithoutLangModel(const char* datapath,
                                      const char* language) {
  // If the datapath or the language have changed, then start again.
  if (tesseract_ != NULL &&
      (datapath_ == NULL || language_ == NULL ||
       *datapath_ != datapath || *language_ != language)) {
    tesseract_->end_tesseract();
    delete tesseract_;
    tesseract_ = NULL;
  }
  if (datapath_ == NULL)
    datapath_ = new STRING(datapath);
  else
    *datapath_ = datapath;
  if (language_ == NULL)
    language_ = new STRING(language);
  else
    *language_ = language;
  if (tesseract_ == NULL) {
    tesseract_ = new Tesseract;
    return tesseract_->init_tesseract_classifier(
        datapath, output_file_ != NULL ? output_file_->string() : NULL,
        language, NULL, 0, false);
  }
  // For same language and datapath, just reset the adaptive classifier.
  tesseract_->ResetAdaptiveClassifier();
  return 0;
}

// Read a "config" file containing a set of variable, value pairs.
// Searches the standard places: tessdata/configs, tessdata/tessconfigs
// and also accepts a relative or absolute path name.
void TessBaseAPI::ReadConfigFile(const char* filename, bool global_only) {
  tesseract_->read_config_file(filename, global_only);
}

// Set the current page segmentation mode. Defaults to PSM_AUTO.
// The mode is stored as an INT_VARIABLE so it can also be modified by
// ReadConfigFile or SetVariable("tessedit_pageseg_mode", mode as string).
void TessBaseAPI::SetPageSegMode(PageSegMode mode) {
  if (tesseract_ == NULL)
    tesseract_ = new Tesseract;
  tesseract_->tessedit_pageseg_mode.set_value(mode);
}

// Return the current page segmentation mode.
PageSegMode TessBaseAPI::GetPageSegMode() const {
  if (tesseract_ == NULL)
    return PSM_SINGLE_BLOCK;
  return static_cast<PageSegMode>(
    static_cast<int>(tesseract_->tessedit_pageseg_mode));
}

// Set the hint for trading accuracy against speed.
// Default is AVS_FASTEST, which is the old behaviour.
// Note that this is only a hint. Depending on the language and/or
// build configuration, speed and accuracy may not be tradeable.
// Also note that despite being an enum, any value in the range
// AVS_FASTEST to AVS_MOST_ACCURATE can be provided, and may or may not
// have an effect, depending on the implementation.
// The mode is stored as an INT_VARIABLE so it can also be modified by
// ReadConfigFile or SetVariable("tessedit_accuracyvspeed", mode as string).
void TessBaseAPI::SetAccuracyVSpeed(AccuracyVSpeed mode) {
  if (tesseract_ == NULL)
    tesseract_ = new Tesseract;
  tesseract_->tessedit_accuracyvspeed.set_value(mode);
}

// Recognize a rectangle from an image and return the result as a string.
// May be called many times for a single Init.
// Currently has no error checking.
// Greyscale of 8 and color of 24 or 32 bits per pixel may be given.
// Palette color images will not work properly and must be converted to
// 24 bit.
// Binary images of 1 bit per pixel may also be given but they must be
// byte packed with the MSB of the first byte being the first pixel, and a
// one pixel is WHITE. For binary images set bytes_per_pixel=0.
// The recognized text is returned as a char* which is coded
// as UTF8 and must be freed with the delete [] operator.
char* TessBaseAPI::TesseractRect(const unsigned char* imagedata,
                                 int bytes_per_pixel,
                                 int bytes_per_line,
                                 int left, int top,
                                 int width, int height) {
  if (tesseract_ == NULL || width < kMinRectSize || height < kMinRectSize)
    return NULL;  // Nothing worth doing.

  // Since this original api didn't give the exact size of the image,
  // we have to invent a reasonable value.
  int bits_per_pixel = bytes_per_pixel == 0 ? 1 : bytes_per_pixel * 8;
  SetImage(imagedata, bytes_per_line * 8 / bits_per_pixel, height + top,
           bytes_per_pixel, bytes_per_line);
  SetRectangle(left, top, width, height);

  return GetUTF8Text();
}

// Call between pages or documents etc to free up memory and forget
// adaptive data.
void TessBaseAPI::ClearAdaptiveClassifier() {
  if (tesseract_ == NULL)
    return;
  tesseract_->ResetAdaptiveClassifier();
}

// Provide an image for Tesseract to recognize. Format is as
// TesseractRect above. Does not copy the image buffer, or take
// ownership. The source image may be destroyed after Recognize is called,
// either explicitly or implicitly via one of the Get*Text functions.
// SetImage clears all recognition results, and sets the rectangle to the
// full image, so it may be followed immediately by a GetUTF8Text, and it
// will automatically perform recognition.
void TessBaseAPI::SetImage(const unsigned char* imagedata,
                           int width, int height,
                           int bytes_per_pixel, int bytes_per_line) {
  if (InternalSetImage())
    thresholder_->SetImage(imagedata, width, height,
                           bytes_per_pixel, bytes_per_line);
}

// Provide an image for Tesseract to recognize. As with SetImage above,
// Tesseract doesn't take a copy or ownership or pixDestroy the image, so
// it must persist until after Recognize.
// Pix vs raw, which to use?
// Use Pix where possible. A future version of Tesseract may choose to use Pix
// as its internal representation and discard IMAGE altogether.
// Because of that, an implementation that sources and targets Pix may end up
// with less copies than an implementation that does not.
void TessBaseAPI::SetImage(const Pix* pix) {
#ifdef HAVE_LIBLEPT
  if (InternalSetImage())
    thresholder_->SetImage(pix);
#endif
}

// Restrict recognition to a sub-rectangle of the image. Call after SetImage.
// Each SetRectangle clears the recogntion results so multiple rectangles
// can be recognized with the same image.
void TessBaseAPI::SetRectangle(int left, int top, int width, int height) {
  if (thresholder_ == NULL)
    return;
  thresholder_->SetRectangle(left, top, width, height);
  ClearResults();
}

// ONLY available if you have Leptonica installed.
// Get a copy of the internal thresholded image from Tesseract.
Pix* TessBaseAPI::GetThresholdedImage() {
#ifdef HAVE_LIBLEPT
  if (tesseract_ == NULL)
    return NULL;
  if (tesseract_->pix_binary() == NULL)
    Threshold(tesseract_->mutable_pix_binary());
  return pixClone(tesseract_->pix_binary());
#endif
}

// Get the result of page layout analysis as a leptonica-style
// Boxa, Pixa pair, in reading order.
// Can be called before or after Recognize.
// For now only gets text regions.
Boxa* TessBaseAPI::GetRegions(Pixa** pixa) {
#ifdef HAVE_LIBLEPT
  if (block_list_ == NULL || block_list_->empty()) {
    FindLines();
  }
  int im_height = pixGetHeight(tesseract_->pix_binary());
  Boxa* boxa = boxaCreate(block_list_->length());
  if (pixa != NULL) {
    *pixa = pixaCreate(boxaGetCount(boxa));
  }
  BLOCK_IT it(block_list_);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    BLOCK* block = it.data();
    POLY_BLOCK* poly = block->poly_block();
    TBOX box;
    if (poly != NULL) {
      if (!poly->IsText())
        continue;  // Use only text blocks.
      POLY_BLOCK image_block(poly->points(), poly->isA());
      image_block.rotate(block->re_rotation());
      box = *image_block.bounding_box();
      if (pixa != NULL) {
        Pix* pix = pixCreate(box.width(), box.height(), 1);
        PB_LINE_IT *lines;
        // Block outline is a polygon, so use a PC_LINE_IT to get the
        // rasterized interior. (Runs of interior pixels on a line.)
        lines = new PB_LINE_IT(&image_block);
        for (int y = box.bottom(); y < box.top(); ++y) {
          ICOORDELT_LIST* segments = lines->get_line(y);
          if (!segments->empty()) {
            ICOORDELT_IT s_it(segments);
            // Each element of segments is a start x and x size of the
            // run of interior pixels.
            for (s_it.mark_cycle_pt(); !s_it.cycled_list(); s_it.forward()) {
              int start = s_it.data()->x();
              int xext = s_it.data()->y();
              // Copy the run from the source image to the block image.
              pixRasterop(pix, start - box.left(),
                          box.height() - 1 - (y - box.bottom()),
                          xext, 1, PIX_SRC, tesseract_->pix_binary(),
                          start, im_height - 1 - y);
            }
          }
          delete segments;
        }
        delete lines;
        pixaAddPix(*pixa, pix, L_INSERT);
      }
    } else {
      if (!block_list_->singleton())
        continue;  // A null poly block can only be used if it is the only block.
      box = block->bounding_box();
      if (pixa != NULL) {
        Pix* pix = pixCreate(box.width(), box.height(), 1);
        // Just copy the whole block as there is only a bounding box.
        pixRasterop(pix, 0, 0, box.width(), box.height(),
                    PIX_SRC, tesseract_->pix_binary(),
                    box.left(), im_height - box.top());
        pixaAddPix(*pixa, pix, L_INSERT);
      }
    }
    Box* lbox = boxCreate(box.left(), im_height - box.top(),
                          box.width(), box.height());
    boxaAddBox(boxa, lbox, L_INSERT);
  }
  return boxa;
#endif
}

// Get the textlines as a leptonica-style
// Boxa, Pixa pair, in reading order.
// Can be called before or after Recognize.
// If blockids is not NULL, the block-id of each line is also returned as an
// array of one element per line. delete [] after use.
Boxa* TessBaseAPI::GetTextlines(Pixa** pixa, int** blockids) {
#ifdef HAVE_LIBLEPT
  if (block_list_ == NULL || block_list_->empty()) {
    FindLines();
  }
  // A local PAGE_RES prevents the clear if Recognize is called after.
  PAGE_RES page_res(block_list_);
  PAGE_RES_IT page_res_it(page_res_ != NULL ? page_res_ : &page_res);
  // Count the lines to get a size for the arrays.
  int line_count = 0;
  for (page_res_it.restart_page(); page_res_it.word() != NULL;
       page_res_it.forward()) {
    if (page_res_it.row() != page_res_it.next_row()) {
      ++line_count;
    }
  }

  int im_height = pixGetHeight(tesseract_->pix_binary());
  Boxa* boxa = boxaCreate(line_count);
  if (pixa != NULL)
    *pixa = pixaCreate(line_count);
  if (blockids != NULL)
    *blockids = new int[line_count];
  int blockid = 0;
  int lineindex = 0;
  for (page_res_it.restart_page(); page_res_it.word() != NULL;
       page_res_it.forward(), ++lineindex) {
    WERD_RES *word = page_res_it.word();
    BLOCK* block = page_res_it.block()->block;
    // Get the line bounding box.
    PAGE_RES_IT word_it(page_res_it);  // Save start of line.
    TBOX line_box = word->word->bounding_box();
    while (page_res_it.next_row() == page_res_it.row()) {
      page_res_it.forward();
      word = page_res_it.word();
      TBOX word_box = word->word->bounding_box();
      word_box.rotate(block->re_rotation());
      line_box += word_box;
    }
    Box* lbox = boxCreate(line_box.left(), im_height - line_box.top(),
                          line_box.width(), line_box.height());
    boxaAddBox(boxa, lbox, L_INSERT);
    if (pixa != NULL) {
      Pix* pix = pixCreate(line_box.width(), line_box.height(), 1);
      // Copy all the words to the output pix.
      while (word_it.row() == page_res_it.row()) {
        word = word_it.word();
        TBOX word_box = word->word->bounding_box();
        word_box.rotate(block->re_rotation());
        pixRasterop(pix, word_box.left() - line_box.left(),
                    line_box.top() - word_box.top(),
                    word_box.width(), word_box.height(),
                    PIX_SRC, tesseract_->pix_binary(),
                    word_box.left(), im_height - word_box.top());
        word_it.forward();
      }
      pixaAddPix(*pixa, pix, L_INSERT);
      pixaAddBox(*pixa, lbox, L_CLONE);
    }
    if (blockids != NULL) {
      (*blockids)[lineindex] = blockid;
      if (page_res_it.block() != page_res_it.next_block())
        ++blockid;
    }
  }
  return boxa;
#endif
}

// Get the words as a leptonica-style
// Boxa, Pixa pair, in reading order.
// Can be called before or after Recognize.
Boxa* TessBaseAPI::GetWords(Pixa** pixa) {
#ifdef HAVE_LIBLEPT
  if (block_list_ == NULL || block_list_->empty()) {
    FindLines();
  }
  // A local PAGE_RES prevents the clear if Recognize is called after.
  PAGE_RES page_res(block_list_);
  PAGE_RES_IT page_res_it(page_res_ != NULL ? page_res_ : &page_res);
  // Count the words to get a size for the arrays.
  int word_count = 0;
  for (page_res_it.restart_page(); page_res_it.word () != NULL;
       page_res_it.forward())
    ++word_count;

  int im_height = pixGetHeight(tesseract_->pix_binary());
  Boxa* boxa = boxaCreate(word_count);
  if (pixa != NULL) {
    *pixa = pixaCreate(word_count);
  }
  for (page_res_it.restart_page(); page_res_it.word () != NULL;
       page_res_it.forward()) {
    WERD_RES *word = page_res_it.word();
    BLOCK* block = page_res_it.block()->block;
    TBOX box = word->word->bounding_box();
    box.rotate(block->re_rotation());
    Box* lbox = boxCreate(box.left(), im_height - box.top(),
                          box.width(), box.height());
    boxaAddBox(boxa, lbox, L_INSERT);
    if (pixa != NULL) {
      Pix* pix = pixCreate(box.width(), box.height(), 1);
      // Copy the whole word bounding box to the output pix.
      pixRasterop(pix, 0, 0, box.width(), box.height(),
                  PIX_SRC, tesseract_->pix_binary(),
                  box.left(), im_height - box.top());
      pixaAddPix(*pixa, pix, L_INSERT);
      pixaAddBox(*pixa, lbox, L_CLONE);
    }
  }
  return boxa;
#endif  // HAVE_LIBLEPT
}

// Dump the internal binary image to a PGM file.
void TessBaseAPI::DumpPGM(const char* filename) {
  if (tesseract_ == NULL)
    return;
  IMAGELINE line;
  line.init(page_image.get_xsize());
  FILE *fp = fopen(filename, "w");
  fprintf(fp, "P5 " INT32FORMAT " " INT32FORMAT " 255\n",
          page_image.get_xsize(), page_image.get_ysize());
  for (int j = page_image.get_ysize()-1; j >= 0 ; --j) {
    page_image.get_line(0, j, page_image.get_xsize(), &line, 0);
    for (int i = 0; i < page_image.get_xsize(); ++i) {
      uinT8 b = line.pixels[i] ? 255 : 0;
      fwrite(&b, 1, 1, fp);
    }
  }
  fclose(fp);
}

// Recognize the tesseract global image and return the result as Tesseract
// internal structures.
int TessBaseAPI::Recognize(struct ETEXT_STRUCT* monitor) {
  if (tesseract_ == NULL)
    return -1;
  if (thresholder_ == NULL || thresholder_->IsEmpty()) {
    tprintf("Please call SetImage before attempting recognition.");
    return -1;
  }
  if (page_res_ != NULL)
    ClearResults();
  if (FindLines() != 0)
    return -1;
  if (tesseract_->tessedit_resegment_from_boxes)
    tesseract_->apply_boxes(*input_file_, block_list_);
  tesseract_->SetBlackAndWhitelist();

  page_res_ = new PAGE_RES(block_list_);
  int result = 0;
  if (interactive_mode) {
    tesseract_->pgeditor_main(block_list_);
    // The page_res is invalid after an interactive session, so cleanup
    // in a way that lets us continue to the next page without crashing.
    delete page_res_;
    page_res_ = NULL;
    return -1;
  } else if (tesseract_->tessedit_train_from_boxes) {
    apply_box_training(*output_file_, block_list_);
  } else if (tesseract_->global_tessedit_ambigs_training) {
    FILE *ambigs_output_file = tesseract_->init_ambigs_training(*input_file_);
    // OCR the page segmented into words by tesseract.
    tesseract_->ambigs_training_segmented(
        *input_file_, page_res_, monitor, ambigs_output_file);
    fclose(ambigs_output_file);
  } else {
    // Now run the main recognition.
    // Running base tesseract if the inttemp for the current language loaded.
    if (tesseract_->inttemp_loaded_) {
      tesseract_->recog_all_words(page_res_, monitor);
    }
  }
  return result;
}

// Tests the chopper by exhaustively running chop_one_blob.
int TessBaseAPI::RecognizeForChopTest(struct ETEXT_STRUCT* monitor) {
  if (tesseract_ == NULL)
    return -1;
  if (thresholder_ == NULL || thresholder_->IsEmpty()) {
    tprintf("Please call SetImage before attempting recognition.");
    return -1;
  }
  if (page_res_ != NULL)
    ClearResults();
  if (FindLines() != 0)
    return -1;
  // Additional conditions under which chopper test cannot be run
  if (tesseract_->tessedit_train_from_boxes_word_level || interactive_mode)
    return -1;
  ASSERT_HOST(tesseract_->inttemp_loaded_);

  page_res_ = new PAGE_RES(block_list_);

  PAGE_RES_IT page_res_it(page_res_);

  tesseract_->tess_matcher = &Tesseract::tess_default_matcher;
  tesseract_->tess_tester = NULL;
  tesseract_->tess_trainer = NULL;

  while (page_res_it.word() != NULL) {
    WERD_RES *word_res = page_res_it.word();
    WERD *word = word_res->word;
    if (word->cblob_list()->empty()) {
      page_res_it.forward();
      continue;
    }
    WERD *bln_word = make_bln_copy(word, page_res_it.row()->row,
                                   page_res_it.block()->block,
                                   word_res->x_height, &word_res->denorm);
    ASSERT_HOST(!bln_word->blob_list()->empty());
    TWERD *tessword = make_tess_word(bln_word, NULL);
    if (tessword->blobs == NULL) {
      make_tess_word(bln_word, NULL);
    }
    TBLOB *pblob;
    TBLOB *blob;
    init_match_table();
    BLOB_CHOICE_LIST *match_result;
    BLOB_CHOICE_LIST_VECTOR *char_choices = new BLOB_CHOICE_LIST_VECTOR();
    tesseract_->tess_denorm = &word_res->denorm;
    tesseract_->tess_word = bln_word;
    ASSERT_HOST(tessword->blobs != NULL);
    for (blob = tessword->blobs, pblob = NULL;
         blob != NULL; blob = blob->next) {
      match_result = tesseract_->classify_blob(pblob, blob, blob->next, NULL,
                                   "chop_word:", Green);
      if (match_result == NULL)
        tprintf("Null classifier output!\n");
      tesseract_->modify_blob_choice(match_result, 0);
      ASSERT_HOST(!match_result->empty());
      *char_choices += match_result;
      pblob = blob;
    }
    inT32 blob_number;
    SEAMS seam_list = start_seam_list(tessword->blobs);
    int right_chop_index = 0;
    while (tesseract_->chop_one_blob(tessword, char_choices,
                                    &blob_number, &seam_list,
                                    &right_chop_index))   {
    }

    word_res->best_choice = new WERD_CHOICE();
    word_res->raw_choice = new WERD_CHOICE();
    word_res->best_choice->make_bad();
    word_res->raw_choice->make_bad();
    tesseract_->getDict().permute_characters(*char_choices, 1000.0,
                                             word_res->best_choice,
                                             word_res->raw_choice);

    word_res->outword = make_ed_word(tessword, bln_word);
    page_res_it.forward();
  }
  return 0;
}

// Make a text string from the internal data structures.
char* TessBaseAPI::GetUTF8Text() {
  if (tesseract_ == NULL ||
      (page_res_ == NULL && Recognize(NULL) < 0))
    return NULL;
  int total_length = TextLength(NULL);
  PAGE_RES_IT   page_res_it(page_res_);
  char* result = new char[total_length];
  char* ptr = result;
  for (page_res_it.restart_page(); page_res_it.word () != NULL;
       page_res_it.forward()) {
    WERD_RES *word = page_res_it.word();
    WERD_CHOICE* choice = word->best_choice;
    if (choice != NULL) {
      strcpy(ptr, choice->unichar_string().string());
      ptr += choice->unichar_string().length();
      if (word->word->flag(W_EOL))
        *ptr++ = '\n';
      else
        *ptr++ = ' ';
    }
  }
  *ptr++ = '\n';
  *ptr = '\0';
  return result;
}

// Helper returns true if there is a paragraph break between bbox_cur,
// and bbox_prev.
// TODO(rays) improve and incorporate deeper into tesseract, so other
// output methods get the benefit.
static bool IsParagraphBreak(TBOX bbox_cur, TBOX bbox_prev,
                             int right, int line_height) {
  // Check if the distance between lines is larger than the normal leading,
  if (fabs((float)(bbox_cur.bottom() - bbox_prev.bottom())) > line_height * 2)
    return true;
  
  // Check if the distance between left bounds of the two lines is nearly the
  // same as between their right bounds (if so, then both lines probably belong
  // to the same paragraph, maybe a centered one).
  if (fabs((float)((bbox_cur.left() - bbox_prev.left()) -
           (bbox_prev.right() - bbox_cur.right()))) < line_height)
    return false;
  
  // Check if there is a paragraph indent at this line (either -ve or +ve).
  if (fabs((float)(bbox_cur.left() - bbox_prev.left())) > line_height)
    return true;
  
  // Check if both current and previous line don't reach the right bound of the
  // block, but the distance is different. This will cause all lines in a verse
  // to be treated as separate paragraphs, but most probably will not split
  // block-quotes to separate lines (at least if the text is justified).
  if (fabs((float)(bbox_cur.right() - bbox_prev.right())) > line_height &&
      right - bbox_cur.right() > line_height &&
      right - bbox_prev.right() > line_height)
    return true;
    
  return false;
}

// Helper to add the hOCR for a box to the given hocr_str.
static void AddBoxTohOCR(const TBOX& box, int image_height, STRING* hocr_str) {
  hocr_str->add_str_int("' title=\"bbox ", box.left());
  hocr_str->add_str_int(" ", image_height - box.top());
  hocr_str->add_str_int(" ", box.right());
  hocr_str->add_str_int(" ", image_height - box.bottom());
  *hocr_str += "\">";
}

// Make a HTML-formatted string with hOCR markup from the internal
// data structures.
// STL removed from original patch submission and refactored by rays.
// page_id is 1-based and will appear in the output.
char* TessBaseAPI::GetHOCRText(int page_id) {
  if (tesseract_ == NULL ||
      (page_res_ == NULL && Recognize(NULL) < 0))
    return NULL;
  
  PAGE_RES_IT page_res_it(page_res_);
  ROW_RES *row = NULL;           // current row
  ROW *real_row = NULL, *prev_row = NULL;
  BLOCK_RES *block = NULL;       // current row
  BLOCK *real_block = NULL;
  int lcnt = 1, bcnt = 1, wcnt = 1;

  STRING hocr_str;

  hocr_str.add_str_int("<div class='ocr_page' id='page_", page_id);
  hocr_str += "' title='image \"";
  hocr_str += *input_file_;
  hocr_str.add_str_int("\"; bbox ", rect_left_);
  hocr_str.add_str_int(" ", rect_top_);
  hocr_str.add_str_int(" ", rect_width_);
  hocr_str.add_str_int(" ", rect_height_);
  hocr_str += "'>\n";

  for (page_res_it.restart_page(); page_res_it.word () != NULL;
       page_res_it.forward()) {
    if (block != page_res_it.block ()) {
      
      if (block != NULL) {
        hocr_str += "</span>\n</p>\n</div>\n";
      }
      
      block = page_res_it.block ();  // current row
      real_block = block->block;
      real_row = NULL;
      row = NULL;
      
      hocr_str.add_str_int("<div class='ocr_carea' id='block_", page_id);
      hocr_str.add_str_int("_", bcnt++);
      AddBoxTohOCR(real_block->bounding_box(), image_height_, &hocr_str);
      hocr_str += "\n<p class='ocr_par'>\n";
    }
    if (row != page_res_it.row ()) {
      
      if (row != NULL) {
        hocr_str += "</span>\n";
      }
      prev_row = real_row;
      
      row = page_res_it.row ();  // current row
      real_row = row->row;
      
      if (prev_row != NULL && 
          IsParagraphBreak(real_row->bounding_box(), prev_row->bounding_box(),
                           real_block->bounding_box().right(),
                           real_row->x_height() + real_row->ascenders()))
        hocr_str += "</p>\n<p class='ocr_par'>\n";
      
      hocr_str.add_str_int("<span class='ocr_line' id='line_", page_id);
      hocr_str.add_str_int("_", lcnt++);
      AddBoxTohOCR(real_row->bounding_box(), image_height_, &hocr_str);
    }

    WERD_RES *word = page_res_it.word();
    WERD_CHOICE* choice = word->best_choice;
    if (choice != NULL) {
      hocr_str.add_str_int("<span class='ocr_word' id='word_", page_id);
      hocr_str.add_str_int("_", wcnt);
      AddBoxTohOCR(word->word->bounding_box(), image_height_, &hocr_str);
      hocr_str.add_str_int("<span class='xocr_word' id='xword_", page_id);
      hocr_str.add_str_int("_", wcnt++);
 	    hocr_str.add_str_int("' title=\"x_wconf ", choice->certainty());
      hocr_str += "\">";
      if (word->bold > 0)
        hocr_str += "<strong>";
      if (word->italic > 0)
        hocr_str += "<em>";
      hocr_str += choice->unichar_string();
      if (word->italic > 0)
        hocr_str += "</em>";
      if (word->bold > 0)
        hocr_str += "</strong>";
      hocr_str += "</span></span>";
      if (!word->word->flag(W_EOL))
        hocr_str += " ";
    }
  }
  hocr_str += "</span>\n</p>\n";  
  hocr_str += "</div>\n</div>\n";  
  char *ret = new char[hocr_str.length() + 1];
  strcpy(ret, hocr_str.string());
  return ret;
}

static int ConvertWordToBoxText(WERD_RES *word,
                                ROW_RES* row,
                                int left,
                                int bottom,
                                int image_width,
                                int image_height,
                                int page_number,
                                char* word_str) {
  // Copy the output word and denormalize it back to image coords.
  WERD copy_outword;
  copy_outword = *(word->outword);
  copy_outword.baseline_denormalise(&word->denorm);
  PBLOB_IT blob_it;
  blob_it.set_to_list(copy_outword.blob_list());
  int length = copy_outword.blob_list()->length();
  int output_size = 0;

  if (length > 0) {
    for (int index = 0, offset = 0; index < length;
         offset += word->best_choice->unichar_lengths()[index++],
         blob_it.forward()) {
      PBLOB* blob = blob_it.data();
      TBOX blob_box = blob->bounding_box();
      if (word->tess_failed ||
          blob_box.left() < 0 ||
          blob_box.right() > image_width ||
          blob_box.bottom() < 0 ||
          blob_box.top() > image_height) {
        // Bounding boxes can be illegal when tess fails on a word.
        blob_box = word->word->bounding_box();  // Use original word as backup.
        tprintf("Using substitute bounding box at (%d,%d)->(%d,%d)\n",
                blob_box.left(), blob_box.bottom(),
                blob_box.right(), blob_box.top());
      }

      // A single classification unit can be composed of several UTF-8
      // characters. Append each of them to the result.
      for (int sub = 0;
           sub < word->best_choice->unichar_lengths()[index]; ++sub) {
        char ch = word->best_choice->unichar_string()[offset + sub];
        // Tesseract uses space for recognition failure. Fix to a reject
        // character, kTesseractReject so we don't create illegal box files.
        if (ch == ' ')
          ch = kTesseractReject;
        word_str[output_size++] = ch;
      }
      sprintf(word_str + output_size, " %d %d %d %d %d\n",
              blob_box.left() + left, blob_box.bottom() + bottom,
              blob_box.right() + left, blob_box.top() + bottom,
              page_number);
      output_size += strlen(word_str + output_size);
    }
  }
  return output_size;
}

// Multiplier for max expected textlength assumes typically 5 numbers @
// (5 digits and a space) plus the newline = 5*(5+1)+1. Add to this the
// orginal UTF8 characters, and one kMaxCharsPerChar.
const int kCharsPerChar = 31;
// A maximal single box could occupy 5 numbers at 20 digits (for 64 bit) and a
// space plus the newline 5*(20+1)+1 and the maximum length of a UNICHAR.
// Test against this on each iteration for safety.
const int kMaxCharsPerChar = 106 + UNICHAR_LEN;

// The recognized text is returned as a char* which is coded
// as a UTF8 box file and must be freed with the delete [] operator.
// page_number is a 0-base page index that will appear in the box file.
char* TessBaseAPI::GetBoxText(int page_number) {
  int bottom = image_height_ - (rect_top_ + rect_height_);
  if (tesseract_ == NULL ||
      (page_res_ == NULL && Recognize(NULL) < 0))
    return NULL;
  int blob_count;
  int utf8_length = TextLength(&blob_count);
  int total_length = blob_count*kCharsPerChar + utf8_length + kMaxCharsPerChar;
  PAGE_RES_IT   page_res_it(page_res_);
  char* result = new char[total_length];
  char* ptr = result;
  for (page_res_it.restart_page(); page_res_it.word () != NULL;
       page_res_it.forward()) {
    WERD_RES *word = page_res_it.word();
    ptr += ConvertWordToBoxText(word, page_res_it.row(), rect_left_, bottom,
                                image_width_, image_height_,
                                page_number, ptr);
    // Just in case...
    if (ptr - result + kMaxCharsPerChar > total_length)
      break;
  }
  *ptr = '\0';
  return result;
}

// Conversion table for non-latin characters.
// Maps characters out of the latin set into the latin set.
// TODO(rays) incorporate this translation into unicharset.
const int kUniChs[] = {
  0x20ac, 0x201c, 0x201d, 0x2018, 0x2019, 0x2022, 0x2014, 0
};
// Latin chars corresponding to the unicode chars above.
const int kLatinChs[] = {
  0x00a2, 0x0022, 0x0022, 0x0027, 0x0027, 0x00b7, 0x002d, 0
};

// The recognized text is returned as a char* which is coded
// as UNLV format Latin-1 with specific reject and suspect codes
// and must be freed with the delete [] operator.
char* TessBaseAPI::GetUNLVText() {
  if (tesseract_ == NULL ||
      (page_res_ == NULL && Recognize(NULL) < 0))
    return NULL;
  bool tilde_crunch_written = false;
  bool last_char_was_newline = true;
  bool last_char_was_tilde = false;

  int total_length = TextLength(NULL);
  PAGE_RES_IT   page_res_it(page_res_);
  char* result = new char[total_length];
  char* ptr = result;
  for (page_res_it.restart_page(); page_res_it.word () != NULL;
       page_res_it.forward()) {
    WERD_RES *word = page_res_it.word();
    // Process the current word.
    if (word->unlv_crunch_mode != CR_NONE) {
      if (word->unlv_crunch_mode != CR_DELETE &&
          (!tilde_crunch_written ||
           (word->unlv_crunch_mode == CR_KEEP_SPACE &&
            word->word->space() > 0 &&
            !word->word->flag(W_FUZZY_NON) &&
            !word->word->flag(W_FUZZY_SP)))) {
        if (!word->word->flag(W_BOL) &&
            word->word->space() > 0 &&
            !word->word->flag(W_FUZZY_NON) &&
            !word->word->flag(W_FUZZY_SP)) {
          /* Write a space to separate from preceeding good text */
          *ptr++ = ' ';
          last_char_was_tilde = false;
        }
        if (!last_char_was_tilde) {
          // Write a reject char.
          last_char_was_tilde = true;
          *ptr++ = kUNLVReject;
          tilde_crunch_written = true;
          last_char_was_newline = false;
        }
      }
    } else {
      // NORMAL PROCESSING of non tilde crunched words.
      tilde_crunch_written = false;

      if (word->word->flag(W_REP_CHAR) && tessedit_consistent_reps)
        ensure_rep_chars_are_consistent(word);

      tesseract_->set_unlv_suspects(word);
      const char* wordstr = word->best_choice->unichar_string().string();
      const STRING& lengths = word->best_choice->unichar_lengths();
      int length = lengths.length();
      int i = 0;
      int offset = 0;

      if (last_char_was_tilde &&
          word->word->space() == 0 && wordstr[offset] == ' ') {
        // Prevent adjacent tilde across words - we know that adjacent tildes
        // within words have been removed.
        // Skip the first character.
        offset = lengths[i++];
      }
      if (i < length && wordstr[offset] != 0) {
        if (!last_char_was_newline)
          *ptr++ = ' ';
        else
          last_char_was_newline = false;
        for (; i < length; offset += lengths[i++]) {
          if (wordstr[offset] == ' ' ||
              wordstr[offset] == kTesseractReject) {
            *ptr++ = kUNLVReject;
            last_char_was_tilde = true;
          } else {
            if (word->reject_map[i].rejected())
              *ptr++ = kUNLVSuspect;
            UNICHAR ch(wordstr + offset, lengths[i]);
            int uni_ch = ch.first_uni();
            for (int j = 0; kUniChs[j] != 0; ++j) {
              if (kUniChs[j] == uni_ch) {
                uni_ch = kLatinChs[j];
                break;
              }
            }
            if (uni_ch <= 0xff) {
              *ptr++ = static_cast<char>(uni_ch);
              last_char_was_tilde = false;
            } else {
              *ptr++ = kUNLVReject;
              last_char_was_tilde = true;
            }
          }
        }
      }
    }
    if (word->word->flag(W_EOL) && !last_char_was_newline) {
      /* Add a new line output */
      *ptr++ = '\n';
      tilde_crunch_written = false;
      last_char_was_newline = true;
      last_char_was_tilde = false;
    }
  }
  *ptr++ = '\n';
  *ptr = '\0';
  return result;
}

// Returns the average word confidence for Tesseract page result.
int TessBaseAPI::MeanTextConf() {
  int* conf = AllWordConfidences();
  if (!conf) return 0;
  int sum = 0;
  int *pt = conf;
  while (*pt >= 0) sum += *pt++;
  if (pt != conf) sum /= pt - conf;
  delete [] conf;
  return sum;
}

// Returns an array of all word confidences, terminated by -1.
int* TessBaseAPI::AllWordConfidences() {
  if (tesseract_ == NULL ||
      (page_res_ == NULL && Recognize(NULL) < 0))
    return NULL;
  int n_word = 0;
  PAGE_RES_IT res_it(page_res_);
  for (res_it.restart_page(); res_it.word() != NULL; res_it.forward())
    n_word++;

  int* conf = new int[n_word+1];
  n_word = 0;
  for (res_it.restart_page(); res_it.word() != NULL; res_it.forward()) {
    WERD_RES *word = res_it.word();
    WERD_CHOICE* choice = word->best_choice;
    int w_conf = static_cast<int>(100 + 5 * choice->certainty());
                 // This is the eq for converting Tesseract confidence to 1..100
    if (w_conf < 0) w_conf = 0;
    if (w_conf > 100) w_conf = 100;
    conf[n_word++] = w_conf;
  }
  conf[n_word] = -1;
  return conf;
}

// Free up recognition results and any stored image data, without actually
// freeing any recognition data that would be time-consuming to reload.
// Afterwards, you must call SetImage or TesseractRect before doing
// any Recognize or Get* operation.
void TessBaseAPI::Clear() {
  if (thresholder_ != NULL)
    thresholder_->Clear();
  ClearResults();
  page_image.destroy();
}

// Close down tesseract and free up all memory. End() is equivalent to
// destructing and reconstructing your TessBaseAPI.
// Once End() has been used, none of the other API functions may be used
// other than Init and anything declared above it in the class definition.
void TessBaseAPI::End() {
  if (thresholder_ != NULL) {
    delete thresholder_;
    thresholder_ = NULL;
  }
  if (page_res_ != NULL) {
    delete page_res_;
    page_res_ = NULL;
  }
  if (block_list_ != NULL) {
    delete block_list_;
    block_list_ = NULL;
  }
  if (tesseract_ != NULL) {
    tesseract_->end_tesseract();
    delete tesseract_;
    tesseract_ = NULL;
  }
  if (input_file_ != NULL) {
    delete input_file_;
    input_file_ = NULL;
  }
  if (output_file_ != NULL) {
    delete output_file_;
    output_file_ = NULL;
  }
  if (datapath_ != NULL) {
    delete datapath_;
    datapath_ = NULL;
  }
  if (language_ != NULL) {
    delete language_;
    language_ = NULL;
  }
}

// Check whether a word is valid according to Tesseract's language model
// returns 0 if the word is invalid, non-zero if valid
int TessBaseAPI::IsValidWord(const char *word) {
  return tesseract_->getDict().valid_word(word);
}


bool TessBaseAPI::GetTextDirection(int* out_offset, float* out_slope) {
  if (page_res_ == NULL)
    FindLines();
  if (block_list_->length() < 1) {
    return false;
  }

  // Get first block
  BLOCK_IT block_it(block_list_);
  block_it.move_to_first();
  ROW_LIST* rows = block_it.data()->row_list();
  if (rows->length() != 1) {
    return false;
  }

  // Get first line of block
  ROW_IT row_it(rows);
  row_it.move_to_first();
  ROW* row = row_it.data();

  // Calculate offset and slope (NOTE: Kind of ugly)
  *out_offset = static_cast<int>(row->base_line(0.0));
  *out_slope = row->base_line(1.0) - row->base_line(0.0);

  return true;
}

// Set the letter_is_okay function to point somewhere else.
void TessBaseAPI::SetDictFunc(DictFunc f) {
  if (tesseract_ != NULL) {
    tesseract_->getDict().letter_is_okay_ = f;
  }
}

// Common code for setting the image.
bool TessBaseAPI::InternalSetImage() {
  if (tesseract_ == NULL) {
    tprintf("Please call Init before attempting to send an image.");
    return false;
  }
  if (thresholder_ == NULL)
    thresholder_ = new ImageThresholder;
  ClearResults();
  return true;
}

// Run the thresholder to make the thresholded image. If pix is not NULL,
// the source is thresholded to pix instead of the internal IMAGE.
void TessBaseAPI::Threshold(Pix** pix) {
#ifdef HAVE_LIBLEPT
  if (pix != NULL)
    thresholder_->ThresholdToPix(pix);
  else
    thresholder_->ThresholdToIMAGE(&page_image);
#else
  thresholder_->ThresholdToIMAGE(&page_image);
#endif
  thresholder_->GetImageSizes(&rect_left_, &rect_top_,
                              &rect_width_, &rect_height_,
                              &image_width_, &image_height_);
  threshold_done_ = true;
}

// Find lines from the image making the BLOCK_LIST.
int TessBaseAPI::FindLines() {
  if (!block_list_->empty()) {
    return 0;
  }
  if (tesseract_ == NULL) {
    tesseract_ = new Tesseract;
    tesseract_->InitAdaptiveClassifier();
  }
#ifdef HAVE_LIBLEPT
  if (tesseract_->pix_binary() == NULL)
    Threshold(tesseract_->mutable_pix_binary());
#endif
  if (!threshold_done_)
    Threshold(NULL);

  if (tesseract_->SegmentPage(input_file_, &page_image, block_list_) < 0)
    return -1;
  ASSERT_HOST(page_image.get_xsize() == rect_width_ ||
              page_image.get_xsize() == rect_width_ - 1);
  ASSERT_HOST(page_image.get_ysize() == rect_height_ ||
              page_image.get_ysize() == rect_height_ - 1);
  return 0;
}

// Delete the pageres and clear the block list ready for a new page.
void TessBaseAPI::ClearResults() {
  threshold_done_ = false;
  if (tesseract_ != NULL)
    tesseract_->Clear();
  if (page_res_ != NULL) {
    delete page_res_;
    page_res_ = NULL;
  }
  if (block_list_ == NULL)
    block_list_ = new BLOCK_LIST;
  else
    block_list_->clear();
}

// Return the length of the output text string, as UTF8, assuming
// one newline per line and one per block, with a terminator,
// and assuming a single character reject marker for each rejected character.
// Also return the number of recognized blobs in blob_count.
int TessBaseAPI::TextLength(int* blob_count) {
  if (tesseract_ == NULL || page_res_ == NULL)
    return 0;

  PAGE_RES_IT   page_res_it(page_res_);
  int total_length = 2;
  int total_blobs = 0;
  // Iterate over the data structures to extract the recognition result.
  for (page_res_it.restart_page(); page_res_it.word () != NULL;
       page_res_it.forward()) {
    WERD_RES *word = page_res_it.word();
    WERD_CHOICE* choice = word->best_choice;
    if (choice != NULL) {
      total_blobs += choice->length() + 1;
      total_length += choice->unichar_string().length() + 1;
      for (int i = 0; i < word->reject_map.length(); ++i) {
        if (word->reject_map[i].rejected())
          ++total_length;
      }
    }
  }
  if (blob_count != NULL)
    *blob_count = total_blobs;
  return total_length;
}

// Estimates the Orientation And Script of the image.
// Returns true if the image was processed successfully.
bool TessBaseAPI::DetectOS(OSResults* osr) {
  if (tesseract_ == NULL)
    return false;
  ClearResults();
  Threshold(NULL);
  if (input_file_ == NULL)
    input_file_ = new STRING(kInputFile);
  return orientation_and_script_detection(*input_file_, osr, tesseract_);
}

// ____________________________________________________________________________
// Ocropus add-ons.

// Find lines from the image making the BLOCK_LIST.
BLOCK_LIST* TessBaseAPI::FindLinesCreateBlockList() {
  FindLines();
  BLOCK_LIST* result = block_list_;
  block_list_ = NULL;
  return result;
}

// Delete a block list.
// This is to keep BLOCK_LIST pointer opaque
// and let go of including the other headers.
void TessBaseAPI::DeleteBlockList(BLOCK_LIST *block_list) {
  delete block_list;
}


static ROW *make_tess_ocrrow(float baseline,
                             float xheight,
                             float descender,
                             float ascender) {
  inT32 xstarts[] = {-32000};
  double quad_coeffs[] = {0, 0, baseline};
  return new ROW(1,
                 xstarts,
                 quad_coeffs,
                 xheight,
                 ascender - (baseline + xheight),
                 descender - baseline,
                 0,
                 0);
}

// Almost a copy of make_tess_row() from ccmain/tstruct.cpp.
static void fill_dummy_row(float baseline, float xheight,
                           float descender, float ascender,
                           TEXTROW* tessrow) {
  tessrow->baseline.segments = 1;
  tessrow->baseline.xstarts[0] = -32767;
  tessrow->baseline.xstarts[1] = 32767;
  tessrow->baseline.quads[0].a = 0;
  tessrow->baseline.quads[0].b = 0;
  tessrow->baseline.quads[0].c = bln_baseline_offset;
  tessrow->xheight.segments = 1;
  tessrow->xheight.xstarts[0] = -32767;
  tessrow->xheight.xstarts[1] = 32767;
  tessrow->xheight.quads[0].a = 0;
  tessrow->xheight.quads[0].b = 0;
  tessrow->xheight.quads[0].c = bln_baseline_offset + bln_x_height;
  tessrow->lineheight = bln_x_height;
  tessrow->ascrise = bln_x_height * (ascender - (xheight + baseline)) / xheight;
  tessrow->descdrop = bln_x_height * (descender - baseline) / xheight;
}


// Return a TBLOB * from the whole page_image.
// To be freed later with free_blob().
TBLOB *make_tesseract_blob(float baseline, float xheight,
                           float descender, float ascender) {
  BLOCK *block = new BLOCK("a character",
                           TRUE,
                           0, 0,
                           0, 0,
                           page_image.get_xsize(),
                           page_image.get_ysize());

  // Create C_BLOBs from the page
  extract_edges(NULL, &page_image, &page_image,
                ICOORD(page_image.get_xsize(), page_image.get_ysize()),
                block);

  // Create one PBLOB from all C_BLOBs
  C_BLOB_LIST *list = block->blob_list();
  C_BLOB_IT c_blob_it(list);
  PBLOB *pblob = new PBLOB;  // will be (hopefully) deleted by the pblob_list
  for (c_blob_it.mark_cycle_pt();
       !c_blob_it.cycled_list();
       c_blob_it.forward()) {
      C_BLOB *c_blob = c_blob_it.data();
      PBLOB c_as_p(c_blob, baseline + xheight);
      merge_blobs(pblob, &c_as_p);
  }
  PBLOB_LIST *pblob_list = new PBLOB_LIST;  // will be deleted by the word
  PBLOB_IT pblob_it(pblob_list);
  pblob_it.add_after_then_move(pblob);

  // Normalize PBLOB
  WERD word(pblob_list, 0, " ");
  ROW *row = make_tess_ocrrow(baseline, xheight, descender, ascender);
  word.baseline_normalise(row);
  delete row;

  // Create a TBLOB from PBLOB
  return make_tess_blob(pblob, /* flatten: */ TRUE);
}


// Adapt to recognize the current image as the given character.
// The image must be preloaded and be just an image of a single character.
void TessBaseAPI::AdaptToCharacter(const char *unichar_repr,
                                   int length,
                                   float baseline,
                                   float xheight,
                                   float descender,
                                   float ascender) {
  UNICHAR_ID id = tesseract_->unicharset.unichar_to_id(unichar_repr, length);
  LINE_STATS LineStats;
  TEXTROW row;
  fill_dummy_row(baseline, xheight, descender, ascender, &row);
  GetLineStatsFromRow(&row, &LineStats);

  TBLOB *blob = make_tesseract_blob(baseline, xheight, descender, ascender);
  float threshold;
  UNICHAR_ID best_class = 0;
  float best_rating = -100;


  // Classify to get a raw choice.
  BLOB_CHOICE_LIST choices;
  tesseract_->AdaptiveClassifier(blob, NULL, &row, &choices, NULL);
  BLOB_CHOICE_IT choice_it;
  choice_it.set_to_list(&choices);
  for (choice_it.mark_cycle_pt(); !choice_it.cycled_list();
       choice_it.forward()) {
    if (choice_it.data()->rating() > best_rating) {
      best_rating = choice_it.data()->rating();
      best_class = choice_it.data()->unichar_id();
    }
  }

  if (id == best_class) {
    threshold = matcher_good_threshold;
  } else {
    /* the blob was incorrectly classified - find the rating threshold
       needed to create a template which will correct the error with
       some margin.  However, don't waste time trying to make
       templates which are too tight. */
    threshold = tesseract_->GetBestRatingFor(blob, &LineStats, id);
    threshold *= .9;
    const float max_threshold = .125;
    const float min_threshold = .02;

    if (threshold > max_threshold)
        threshold = max_threshold;

    // I have cuddled the following line to set it out of the strike
    // of the coverage testing tool. I have no idea how to trigger
    // this situation nor I have any necessity to do it. --mezhirov
    if (threshold < min_threshold) threshold = min_threshold;
  }

  if (blob->outlines)
    tesseract_->AdaptToChar(blob, &LineStats, id, threshold);
  free_blob(blob);
}


PAGE_RES* TessBaseAPI::RecognitionPass1(BLOCK_LIST* block_list) {
  PAGE_RES *page_res = new PAGE_RES(block_list);
  tesseract_->recog_all_words(page_res, NULL, NULL, 1);
  return page_res;
}

PAGE_RES* TessBaseAPI::RecognitionPass2(BLOCK_LIST* block_list,
                                        PAGE_RES* pass1_result) {
  if (!pass1_result)
    pass1_result = new PAGE_RES(block_list);
  tesseract_->recog_all_words(pass1_result, NULL, NULL, 2);
  return pass1_result;
}

struct TESS_CHAR : ELIST_LINK {
  char *unicode_repr;
  int length;  // of unicode_repr
  float cost;
  TBOX box;

  TESS_CHAR(float _cost, const char *repr, int len = -1) : cost(_cost) {
    length = (len == -1 ? strlen(repr) : len);
    unicode_repr = new char[length + 1];
    strncpy(unicode_repr, repr, length);
  }

  TESS_CHAR() {  // Satisfies ELISTIZE.
  }
  ~TESS_CHAR() {
    delete [] unicode_repr;
  }
};

ELISTIZEH(TESS_CHAR)
ELISTIZE(TESS_CHAR)

static void add_space(TESS_CHAR_IT* it) {
  TESS_CHAR *t = new TESS_CHAR(0, " ");
  it->add_after_then_move(t);
}


static float rating_to_cost(float rating) {
  rating = 100 + 5*rating;
  // cuddled that to save from coverage profiler
  // (I have never seen ratings worse than -100,
  //  but the check won't hurt)
  if (rating < 0) rating = 0;
  return rating;
}


// Extract the OCR results, costs (penalty points for uncertainty),
// and the bounding boxes of the characters.
static void extract_result(TESS_CHAR_IT* out,
                           PAGE_RES* page_res) {
  PAGE_RES_IT page_res_it(page_res);
  int word_count = 0;
  while (page_res_it.word() != NULL) {
    WERD_RES *word = page_res_it.word();
    const char *str = word->best_choice->unichar_string().string();
    const char *len = word->best_choice->unichar_lengths().string();

    if (word_count)
      add_space(out);
    TBOX bln_rect;
    PBLOB_LIST *blobs = word->outword->blob_list();
    PBLOB_IT it(blobs);
    int n = strlen(len);
    TBOX** boxes_to_fix = new TBOX*[n];
    for (int i = 0; i < n; i++) {
      PBLOB *blob = it.data();
      TBOX current = blob->bounding_box();
      bln_rect = bln_rect.bounding_union(current);
      TESS_CHAR *tc = new TESS_CHAR(rating_to_cost(word->best_choice->certainty()),
                                    str, *len);
      tc->box = current;
      boxes_to_fix[i] = &tc->box;

      out->add_after_then_move(tc);
      it.forward();
      str += *len;
      len++;
    }

    // Find the word bbox before normalization.
    // Here we can't use the C_BLOB bboxes directly,
    // since connected letters are not yet cut.
    TBOX real_rect = word->word->bounding_box();

    // Denormalize boxes by transforming the bbox of the whole bln word
    // into the denorm bbox (`real_rect') of the whole word.
    double x_stretch = static_cast<double>(real_rect.width())
                     / bln_rect.width();
    double y_stretch = static_cast<double>(real_rect.height())
                     / bln_rect.height();
    for (int j = 0; j < n; j++) {
      TBOX *box = boxes_to_fix[j];
      int x0 = static_cast<int>(real_rect.left() +
                   x_stretch * (box->left() - bln_rect.left()) + 0.5);
      int x1 = static_cast<int>(real_rect.left() +
                   x_stretch * (box->right() - bln_rect.left()) + 0.5);
      int y0 = static_cast<int>(real_rect.bottom() +
                   y_stretch * (box->bottom() - bln_rect.bottom()) + 0.5);
      int y1 = static_cast<int>(real_rect.bottom() +
                   y_stretch * (box->top() - bln_rect.bottom()) + 0.5);
      *box = TBOX(ICOORD(x0, y0), ICOORD(x1, y1));
    }
    delete [] boxes_to_fix;

    page_res_it.forward();
    word_count++;
  }
}


// Extract the OCR results, costs (penalty points for uncertainty),
// and the bounding boxes of the characters.
int TessBaseAPI::TesseractExtractResult(char** text,
                                        int** lengths,
                                        float** costs,
                                        int** x0,
                                        int** y0,
                                        int** x1,
                                        int** y1,
                                        PAGE_RES* page_res) {
  TESS_CHAR_LIST tess_chars;
  TESS_CHAR_IT tess_chars_it(&tess_chars);
  extract_result(&tess_chars_it, page_res);
  tess_chars_it.move_to_first();
  int n = tess_chars.length();
  int text_len = 0;
  *lengths = new int[n];
  *costs = new float[n];
  *x0 = new int[n];
  *y0 = new int[n];
  *x1 = new int[n];
  *y1 = new int[n];
  int i = 0;
  for (tess_chars_it.mark_cycle_pt();
       !tess_chars_it.cycled_list();
       tess_chars_it.forward(), i++) {
    TESS_CHAR *tc = tess_chars_it.data();
    text_len += (*lengths)[i] = tc->length;
    (*costs)[i] = tc->cost;
    (*x0)[i] = tc->box.left();
    (*y0)[i] = tc->box.bottom();
    (*x1)[i] = tc->box.right();
    (*y1)[i] = tc->box.top();
  }
  char *p = *text = new char[text_len];

  tess_chars_it.move_to_first();
  for (tess_chars_it.mark_cycle_pt();
        !tess_chars_it.cycled_list();
       tess_chars_it.forward()) {
    TESS_CHAR *tc = tess_chars_it.data();
    strncpy(p, tc->unicode_repr, tc->length);
    p += tc->length;
  }
  return n;
}

// This method returns the features associated with the current image.
// Make sure setimage has been called before calling this method.
void TessBaseAPI::GetFeatures(INT_FEATURE_ARRAY int_features,
                              int* num_features) {
  if (page_res_ != NULL)
    ClearResults();
  if (!threshold_done_)
    Threshold(NULL);
  // We have only one block, which is of the size of the page.
  BLOCK_LIST* blocks = new BLOCK_LIST;
  BLOCK *block = new BLOCK("",                       // filename.
                           TRUE,                     // proportional.
                           0,                        // kerning.
                           0,                        // spacing.
                           0,                        // Left.
                           0,                        // Bottom.
                           page_image.get_xsize(),   // Right.
                           page_image.get_ysize());  // Top.
  ICOORD bleft, tright;
  block->bounding_box (bleft, tright);

  BLOCK_IT block_it_add = blocks;
  block_it_add.add_to_end(block);

  ICOORD page_tr(page_image.get_xsize(), page_image.get_ysize());
  TEXTROW tessrow;
  make_tess_row(NULL,       // Denormalizer.
                &tessrow);  // Output row.
  LINE_STATS line_stats;
  GetLineStatsFromRow(&tessrow, &line_stats);

  // Perform a CC analysis to detect the blobs.
  BLOCK_IT block_it = blocks;
  for (block_it.mark_cycle_pt (); !block_it.cycled_list ();
       block_it.forward ()) {
    BLOCK* block = block_it.data();
#ifndef GRAPHICS_DISABLED
    extract_edges(NULL,         // Scrollview window.
                  &page_image,  // Image.
                  &page_image,  // Thresholded image.
                  page_tr,      // corner of page.
                  block);       // block.
#else
    extract_edges(&page_image,  // Image.
                  &page_image,  // Thresholded image.
                  page_tr,      // corner of page.
                  block);       // block.
#endif
    C_BLOB_IT blob_it = block->blob_list();
    PBLOB *pblob = new PBLOB;
    // Iterate over all blobs found and get their features.
    for (blob_it.mark_cycle_pt(); !blob_it.cycled_list();
         blob_it.forward()) {
      C_BLOB* blob = blob_it.data();
      blob = blob;
      PBLOB c_as_p(blob, page_image.get_ysize());
      merge_blobs(pblob, &c_as_p);
    }

    PBLOB_LIST *pblob_list = new PBLOB_LIST;
    PBLOB_IT pblob_it(pblob_list);
    pblob_it.add_after_then_move(pblob);
    WERD word(pblob_list,  // Blob list.
              0,           // Blanks in front.
              " ");        // Correct text.
    ROW *row = make_tess_ocrrow(0,                       // baseline.
                                page_image.get_ysize(),  // xheight.
                                0,                       // ascent.
                                0);                      // descent.
    word.baseline_normalise(row);
    delete row;
    if (pblob->out_list () == NULL) {
      tprintf("Blob list is empty");
    }
    TBLOB* tblob = make_tess_blob(pblob,  // Blob.
                                  TRUE);  // Flatten.

    CLASS_NORMALIZATION_ARRAY norm_array;
    inT32 len;
    *num_features = tesseract_->GetCharNormFeatures(
        tblob, &line_stats,
        tesseract_->PreTrainedTemplates,
        int_features, norm_array, &len);
  }
  delete blocks;
}

// Return the pointer to the i-th dawg loaded into tesseract_ object.
const Dawg *TessBaseAPI::GetDawg(int i) const {
  if (tesseract_ == NULL || i >= NumDawgs()) return NULL;
  return tesseract_->getDict().GetDawg(i);
}

// Return the number of dawgs loaded into tesseract_ object.
int TessBaseAPI::NumDawgs() const {
  return tesseract_ == NULL ? 0 : tesseract_->getDict().NumDawgs();
}

// Return the language used in the last valid initialization.
const char* TessBaseAPI::GetLastInitLanguage() const {
  return (tesseract_ == NULL || tesseract_->lang.string() == NULL) ?
      "" : tesseract_->lang.string();
}
}  // namespace tesseract.
