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
#else
#error "Sorry: Tesseract no longer compiles without leptonica!"
#endif
#ifdef USE_NLS
#include <libintl.h>
#include <locale.h>
#define _(x) gettext(x)
#else
#define _(x) (x)
#endif

#include "baseapi.h"

#include "resultiterator.h"
#include "thresholder.h"
#include "tesseractmain.h"
#include "tesseractclass.h"
#include "pageres.h"
#include "tessvars.h"
#include "control.h"
#include "pgedit.h"
#include "paramsd.h"
#include "output.h"
#include "globals.h"
#include "edgblob.h"
#include "tessbox.h"
#include "imgs.h"
#include "imgtiff.h"
#include "makerow.h"
#include "permute.h"
#include "otsuthr.h"
#include "osdetect.h"

#ifdef __MSW32__
#include "version.h"
#endif

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
// Temp file used for storing current parameters before applying retry values.
const char* kOldVarsFile = "failed_vars.txt";
// Max string length of an int.
const int kMaxIntSize = 22;

TessBaseAPI::TessBaseAPI()
  : tesseract_(NULL),
    osd_tesseract_(NULL),
    // Thresholder is initialized to NULL here, but will be set before use by:
    // A constructor of a derived API,  SetThresholder(), or
    // created implicitly when used in InternalSetImage.
    thresholder_(NULL),
    block_list_(NULL),
    page_res_(NULL),
    input_file_(NULL),
    output_file_(NULL),
    datapath_(NULL),
    language_(NULL),
    last_oem_requested_(OEM_DEFAULT),
    recognition_done_(false),
    truth_cb_(NULL),
    rect_left_(0), rect_top_(0), rect_width_(0), rect_height_(0),
    image_width_(0), image_height_(0) {
}

TessBaseAPI::~TessBaseAPI() {
  End();
}

/**
 * Returns the version identifier as a static string. Do not delete.
 */
const char* TessBaseAPI::Version() {
  return VERSION;
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

bool TessBaseAPI::SetVariable(const char* name, const char* value) {
  if (tesseract_ == NULL) tesseract_ = new Tesseract;
  return ParamUtils::SetParam(name, value, false, tesseract_->params());
}

bool TessBaseAPI::GetIntVariable(const char *name, int *value) const {
  IntParam *p = ParamUtils::FindParam<IntParam>(
      name, GlobalParams()->int_params, tesseract_->params()->int_params);
  if (p == NULL) return false;
  *value = (inT32)(*p);
  return true;
}

bool TessBaseAPI::GetBoolVariable(const char *name, bool *value) const {
  BoolParam *p = ParamUtils::FindParam<BoolParam>(
      name, GlobalParams()->bool_params, tesseract_->params()->bool_params);
  if (p == NULL) return false;
  *value = (BOOL8)(*p);
  return true;
}

const char *TessBaseAPI::GetStringVariable(const char *name) const {
  StringParam *p = ParamUtils::FindParam<StringParam>(
      name, GlobalParams()->string_params, tesseract_->params()->string_params);
  return (p != NULL) ? p->string() : NULL;
}

bool TessBaseAPI::GetDoubleVariable(const char *name, double *value) const {
  DoubleParam *p = ParamUtils::FindParam<DoubleParam>(
      name, GlobalParams()->double_params, tesseract_->params()->double_params);
  if (p == NULL) return false;
  *value = (double)(*p);
  return true;
}

// Get value of named variable as a string, if it exists.
bool TessBaseAPI::GetVariableAsString(const char *name, STRING *val) {
  return ParamUtils::GetParamAsString(name, tesseract_->params(), val);
}

// Print Tesseract parameters to the given file.
void TessBaseAPI::PrintVariables(FILE *fp) const {
  ParamUtils::PrintParams(fp, tesseract_->params());
}

// The datapath must be the name of the data directory (no ending /) or
// some other file in which the data directory resides (for instance argv[0].)
// The language is (usually) an ISO 639-3 string or NULL will default to eng.
// If numeric_mode is true, then only digits and Roman numerals will
// be returned.
// Returns 0 on success and -1 on initialization failure.
int TessBaseAPI::Init(const char* datapath, const char* language,
                      OcrEngineMode oem, char **configs, int configs_size,
                      const GenericVector<STRING> *vars_vec,
                      const GenericVector<STRING> *vars_values,
                      bool set_only_init_params) {
  // If the datapath, OcrEngineMode or the language have changed - start again.
  // Note that the language_ field stores the last requested language that was
  // initialized successfully, while tesseract_->lang stores the language
  // actually used. They differ only if the requested language was NULL, in
  // which case tesseract_->lang is set to the Tesseract default ("eng").
  if (tesseract_ != NULL &&
      (datapath_ == NULL || language_ == NULL ||
       *datapath_ != datapath || last_oem_requested_ != oem ||
       (*language_ != language && tesseract_->lang != language))) {
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
            language, oem, configs, configs_size, vars_vec, vars_values,
            set_only_init_params) != 0) {
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
  last_oem_requested_ = oem;

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

// Init only for page layout analysis. Use only for calls to SetImage and
// AnalysePage. Calls that attempt recognition will generate an error.
void TessBaseAPI::InitForAnalysePage() {
  if (tesseract_ == NULL) {
    tesseract_ = new Tesseract;
    tesseract_->InitAdaptiveClassifier(false);
  }
}

// Read a "config" file containing a set of parameter name, value pairs.
// Searches the standard places: tessdata/configs, tessdata/tessconfigs
// and also accepts a relative or absolute path name.
void TessBaseAPI::ReadConfigFile(const char* filename, bool init_only) {
  tesseract_->read_config_file(filename, init_only);
}

// Set the current page segmentation mode. Defaults to PSM_AUTO.
// The mode is stored as an IntParam so it can also be modified by
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
  tesseract_->getDict().ResetDocumentDictionary();
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
  if (InternalSetImage())
    thresholder_->SetImage(pix);
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
  if (tesseract_ == NULL)
    return NULL;
  if (tesseract_->pix_binary() == NULL)
    Threshold(tesseract_->mutable_pix_binary());
  return pixClone(tesseract_->pix_binary());
}

// Get the result of page layout analysis as a leptonica-style
// Boxa, Pixa pair, in reading order.
// Can be called before or after Recognize.
Boxa* TessBaseAPI::GetRegions(Pixa** pixa) {
  return GetComponentImages(RIL_BLOCK, pixa, NULL);
}

// Get the textlines as a leptonica-style Boxa, Pixa pair, in reading order.
// Can be called before or after Recognize.
// If blockids is not NULL, the block-id of each line is also returned as an
// array of one element per line. delete [] after use.
Boxa* TessBaseAPI::GetTextlines(Pixa** pixa, int** blockids) {
  return GetComponentImages(RIL_TEXTLINE, pixa, blockids);
}

// Gets the individual connected (text) components (created
// after pages segmentation step, but before recognition)
// as a leptonica-style Boxa, Pixa pair, in reading order.
// Can be called before or after Recognize.
Boxa* TessBaseAPI::GetConnectedComponents(Pixa** pixa) {
  return GetComponentImages(RIL_SYMBOL, pixa, NULL);
}

// Get the words as a leptonica-style
// Boxa, Pixa pair, in reading order.
// Can be called before or after Recognize.
Boxa* TessBaseAPI::GetWords(Pixa** pixa) {
  return GetComponentImages(RIL_WORD, pixa, NULL);
}

// Get the given level kind of components (block, textline, word etc.) as a
// leptonica-style Boxa, Pixa pair, in reading order.
// Can be called before or after Recognize.
// If blockids is not NULL, the block-id of each component is also returned
// as an array of one element per component. delete [] after use.
Boxa* TessBaseAPI::GetComponentImages(PageIteratorLevel level,
                                      Pixa** pixa, int** blockids) {
  PageIterator* page_it = GetIterator();
  if (page_it == NULL)
    page_it = AnalyseLayout();
  if (page_it == NULL)
    return NULL;  // Failed.

  // Count the components to get a size for the arrays.
  int component_count = 0;
  int left, top, right, bottom;
  do {
    if (page_it->BoundingBox(level, &left, &top, &right, &bottom))
      ++component_count;
  } while (page_it->Next(level));

  Boxa* boxa = boxaCreate(component_count);
  if (pixa != NULL)
    *pixa = pixaCreate(component_count);
  if (blockids != NULL)
    *blockids = new int[component_count];

  int blockid = 0;
  int component_index = 0;
  page_it->Begin();
  do {
    if (page_it->BoundingBox(level, &left, &top, &right, &bottom)) {
      Box* lbox = boxCreate(left, top, right - left, bottom - top);
      boxaAddBox(boxa, lbox, L_INSERT);
      if (pixa != NULL) {
        Pix* pix = page_it->GetBinaryImage(level);
        pixaAddPix(*pixa, pix, L_INSERT);
        pixaAddBox(*pixa, lbox, L_CLONE);
      }
      if (blockids != NULL) {
        (*blockids)[component_index] = blockid;
        if (page_it->IsAtFinalElement(RIL_BLOCK, level))
          ++blockid;
      }
      ++component_index;
    }
  } while (page_it->Next(level));
  delete page_it;
  return boxa;
}

// Dump the internal binary image to a PGM file.
void TessBaseAPI::DumpPGM(const char* filename) {
  if (tesseract_ == NULL)
    return;
  FILE *fp = fopen(filename, "w");
  Pix* pix = tesseract_->pix_binary();
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  l_uint32* data = pixGetData(pix);
  fprintf(fp, "P5 %d %d 255\n", width, height);
  for (int y = 0; y < height; ++y, data += pixGetWpl(pix)) {
    for (int x = 0; x < width; ++x) {
      uinT8 b = GET_DATA_BIT(data, x) ? 0 : 255;
      fwrite(&b, 1, 1, fp);
    }
  }
  fclose(fp);
}

// Placeholder for call to Cube and test that the input data is correct.
// reskew is the direction of baselines in the skewed image in
// normalized (cos theta, sin theta) form, so (0.866, 0.5) would represent
// a 30 degree anticlockwise skew.
int CubeAPITest(Boxa* boxa_blocks, Pixa* pixa_blocks,
                Boxa* boxa_words, Pixa* pixa_words,
                const FCOORD& reskew, Pix* page_pix,
                PAGE_RES* page_res) {
  int block_count = boxaGetCount(boxa_blocks);
  ASSERT_HOST(block_count == pixaGetCount(pixa_blocks));
  // Write each block to the current directory as junk_write_display.nnn.png.
  for (int i = 0; i < block_count; ++i) {
    Pix* pix = pixaGetPix(pixa_blocks, i, L_CLONE);
    pixDisplayWrite(pix, 1);
  }
  int word_count = boxaGetCount(boxa_words);
  ASSERT_HOST(word_count == pixaGetCount(pixa_words));
  int pr_word = 0;
  PAGE_RES_IT page_res_it(page_res);
  for (page_res_it.restart_page(); page_res_it.word () != NULL;
       page_res_it.forward(), ++pr_word) {
    WERD_RES *word = page_res_it.word();
    WERD_CHOICE* choice = word->best_choice;
    // Write the first 100 words to files names wordims/<wordstring>.tif.
    if (pr_word < 100) {
      STRING filename("wordims/");
      if (choice != NULL) {
        filename += choice->unichar_string();
      } else {
        char numbuf[32];
        filename += "unclassified";
        snprintf(numbuf, 32, "%03d", pr_word);
        filename += numbuf;
      }
      filename += ".tif";
      Pix* pix = pixaGetPix(pixa_words, pr_word, L_CLONE);
      pixWrite(filename.string(), pix, IFF_TIFF_G4);
    }
  }
  ASSERT_HOST(pr_word == word_count);
  return 0;
}

// Runs page layout analysis in the mode set by SetPageSegMode.
// May optionally be called prior to Recognize to get access to just
// the page layout results. Returns an iterator to the results.
// Returns NULL on error or an empty page.
// The returned iterator must be deleted after use.
// WARNING! This class points to data held within the TessBaseAPI class, and
// therefore can only be used while the TessBaseAPI class still exists and
// has not been subjected to a call of Init, SetImage, Recognize, Clear, End
// DetectOS, or anything else that changes the internal PAGE_RES.
PageIterator* TessBaseAPI::AnalyseLayout() {
  if (FindLines() == 0) {
    if (block_list_->empty())
      return NULL;  // The page was empty.
    page_res_ = new PAGE_RES(block_list_, NULL);
    return new PageIterator(page_res_, tesseract_,
                            thresholder_->GetScaleFactor(),
                            thresholder_->GetScaledYResolution(),
                            rect_left_, rect_top_, rect_width_, rect_height_);
  }
  return NULL;
}

// Recognize the tesseract global image and return the result as Tesseract
// internal structures.
int TessBaseAPI::Recognize(ETEXT_DESC* monitor) {
  if (tesseract_ == NULL)
    return -1;
  if (FindLines() != 0)
    return -1;
  if (page_res_ != NULL)
    delete page_res_;

  tesseract_->SetBlackAndWhitelist();
  recognition_done_ = true;
  if (tesseract_->tessedit_resegment_from_line_boxes)
    page_res_ = tesseract_->ApplyBoxes(*input_file_, true, block_list_);
  else if (tesseract_->tessedit_resegment_from_boxes)
    page_res_ = tesseract_->ApplyBoxes(*input_file_, false, block_list_);
  else
    page_res_ = new PAGE_RES(block_list_, &tesseract_->prev_word_best_choice_);
  if (tesseract_->tessedit_make_boxes_from_boxes) {
    tesseract_->CorrectClassifyWords(page_res_);
    return 0;
  }
  if (truth_cb_ != NULL) truth_cb_->Run(image_height_, page_res_);

  if (tesseract_->interactive_mode) {
    tesseract_->pgeditor_main(rect_width_, rect_height_, page_res_);
    // The page_res is invalid after an interactive session, so cleanup
    // in a way that lets us continue to the next page without crashing.
    delete page_res_;
    page_res_ = NULL;
    return -1;
  } else if (tesseract_->tessedit_train_from_boxes) {
    tesseract_->ApplyBoxTraining(*output_file_, page_res_);
  } else if (tesseract_->tessedit_ambigs_training) {
    FILE *training_output_file = tesseract_->init_recog_training(*input_file_);
    // OCR the page segmented into words by tesseract.
    tesseract_->recog_training_segmented(
        *input_file_, page_res_, monitor, training_output_file);
    fclose(training_output_file);
  } else {
    // Now run the main recognition.
    tesseract_->recog_all_words(page_res_, monitor, NULL, NULL, 0);
  }
  return 0;
}

// Tests the chopper by exhaustively running chop_one_blob.
int TessBaseAPI::RecognizeForChopTest(ETEXT_DESC* monitor) {
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
  if (tesseract_->interactive_mode) return -1;

  recognition_done_ = true;

  page_res_ = new PAGE_RES(block_list_, &(tesseract_->prev_word_best_choice_));

  PAGE_RES_IT page_res_it(page_res_);

  while (page_res_it.word() != NULL) {
    WERD_RES *word_res = page_res_it.word();
    tesseract_->MaximallyChopWord(page_res_it.block()->block,
                                  page_res_it.row()->row,
                                  word_res);
    page_res_it.forward();
  }
  return 0;
}


// Recognizes all the pages in the named file, as a multi-page tiff or
// list of filenames, or single image, and gets the appropriate kind of text
// according to parameters: tessedit_create_boxfile,
// tessedit_make_boxes_from_boxes, tessedit_write_unlv, tessedit_create_hocr.
// Calls ProcessPage on each page in the input file, which may be a
// multi-page tiff, single-page other file format, or a plain text list of
// images to read. If tessedit_page_number is non-negative, processing begins
// at that page of a multi-page tiff file, or filelist.
// The text is returned in text_out. Returns false on error.
// If non-zero timeout_millisec terminates processing after the timeout on
// a single page.
// If non-NULL and non-empty, and some page fails for some reason,
// the page is reprocessed with the retry_config config file. Useful
// for interactively debugging a bad page.
bool TessBaseAPI::ProcessPages(const char* filename,
                               const char* retry_config, int timeout_millisec,
                               STRING* text_out) {
  int page = tesseract_->tessedit_page_number;
  if (page < 0)
    page = 0;
  FILE* fp = fopen(filename, "rb");
  if (fp == NULL) {
    tprintf(_("Image file %s cannot be opened!\n"), filename);
    return false;
  }
  // Find the number of pages if a tiff file, or zero otherwise.
  int npages = CountTiffPages(fp);
  fclose(fp);

  if (tesseract_->tessedit_create_hocr) {
    *text_out =
        "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\""
        " \"http://www.w3.org/TR/html4/loose.dtd\">\n"
        "<html>\n<head>\n<title></title>\n"
        "<meta http-equiv=\"Content-Type\" content=\"text/html;"
        "charset=utf-8\" />\n<meta name='ocr-system' content='tesseract'/>\n"
        "</head>\n<body>\n";
  } else {
    *text_out = "";
  }

  bool success = true;
  Pix *pix;
  if (npages > 0) {
    for (; page < npages && (pix = pixReadTiff(filename, page)) != NULL;
         ++page) {
      if (page > 0)
        tprintf(_("Page %d\n"), page);
      char page_str[kMaxIntSize];
      snprintf(page_str, kMaxIntSize - 1, "%d", page);
      SetVariable("applybox_page", page_str);
      success &= ProcessPage(pix, page, filename, retry_config,
                             timeout_millisec, text_out);
      pixDestroy(&pix);
      if (tesseract_->tessedit_page_number >= 0 || npages == 1) {
        break;
      }
    }
  } else {
    // The file is not a tiff file, so use the general pixRead function.
    pix = pixRead(filename);
    if (pix != NULL) {
      success &= ProcessPage(pix, 0, filename, retry_config,
                             timeout_millisec, text_out);
      pixDestroy(&pix);
    } else {
      // The file is not an image file, so try it as a list of filenames.
      FILE* fimg = fopen(filename, "r");
      if (fimg == NULL) {
        tprintf(_("File %s cannot be opened!\n"), filename);
        return false;
      }
      tprintf(_("Reading %s as a list of filenames...\n"), filename);
      char pagename[MAX_PATH];
      // Skip to the requested page number.
      for (int i = 0; i < page &&
           fgets(pagename, sizeof(pagename), fimg) != NULL;
           ++i);
      while (fgets(pagename, sizeof(pagename), fimg) != NULL) {
        chomp_string(pagename);
        pix = pixRead(pagename);
        if (pix == NULL) {
          tprintf(_("Image file %s cannot be read!\n"), pagename);
          fclose(fimg);
          return false;
        }
        tprintf(_("Page %d : %s\n"), page, pagename);
        success &= ProcessPage(pix, page, pagename, retry_config,
                               timeout_millisec, text_out);
        pixDestroy(&pix);
        ++page;
      }
      fclose(fimg);
    }
  }
  if (tesseract_->tessedit_create_hocr)
    *text_out += "</body>\n</html>\n";
  return success;
}


// Recognizes a single page for ProcessPages, appending the text to text_out.
// The pix is the image processed - filename and page_index are metadata
// used by side-effect processes, such as reading a box file or formatting
// as hOCR.
// If non-zero timeout_millisec terminates processing after the timeout.
// If non-NULL and non-empty, and some page fails for some reason,
// the page is reprocessed with the retry_config config file. Useful
// for interactively debugging a bad page.
// The text is returned in text_out. Returns false on error.
bool TessBaseAPI::ProcessPage(Pix* pix, int page_index, const char* filename,
                              const char* retry_config, int timeout_millisec,
                              STRING* text_out) {
  SetInputName(filename);
  SetImage(pix);
  bool failed = false;
  if (timeout_millisec > 0) {
    // Running with a timeout.
    ETEXT_DESC monitor;
    monitor.cancel = NULL;
    monitor.cancel_this = NULL;
    monitor.set_deadline_msecs(timeout_millisec);
    // Now run the main recognition.
    failed = Recognize(&monitor) < 0;
  } else if (tesseract_->tessedit_pageseg_mode == PSM_OSD_ONLY ||
             tesseract_->tessedit_pageseg_mode == PSM_AUTO_ONLY) {
    // Disabled character recognition.
    PageIterator* it = AnalyseLayout();
    if (it == NULL) {
      failed = true;
    } else {
      delete it;
      return true;
    }
  } else {
    // Normal layout and character recognition with no timeout.
    failed = Recognize(NULL) < 0;
  }
  if (tesseract_->tessedit_write_images) {
    Pix* page_pix = GetThresholdedImage();
    pixWrite("tessinput.tif", page_pix, IFF_TIFF_G4);
  }
  if (failed && retry_config != NULL && retry_config[0] != '\0') {
    // Save current config variables before switching modes.
    FILE* fp = fopen(kOldVarsFile, "w");
    PrintVariables(fp);
    fclose(fp);
    // Switch to alternate mode for retry.
    ReadConfigFile(retry_config, false);
    SetImage(pix);
    Recognize(NULL);
    // Restore saved config variables.
    ReadConfigFile(kOldVarsFile, false);
  }
  // Get text only if successful.
  if (!failed) {
    char* text;
    if (tesseract_->tessedit_create_boxfile ||
        tesseract_->tessedit_make_boxes_from_boxes) {
      text = GetBoxText(page_index);
    } else if (tesseract_->tessedit_write_unlv) {
      text = GetUNLVText();
    } else if (tesseract_->tessedit_create_hocr) {
      text = GetHOCRText(page_index);
    } else {
      text = GetUTF8Text();
    }
    *text_out += text;
    delete [] text;
    return true;
  }
  return false;
}

// Get an iterator to the results of LayoutAnalysis and/or Recognize.
// The returned iterator must be deleted after use.
// WARNING! This class points to data held within the TessBaseAPI class, and
// therefore can only be used while the TessBaseAPI class still exists and
// has not been subjected to a call of Init, SetImage, Recognize, Clear, End
// DetectOS, or anything else that changes the internal PAGE_RES.
ResultIterator* TessBaseAPI::GetIterator() {
  if (tesseract_ == NULL || page_res_ == NULL)
    return NULL;
  return new ResultIterator(page_res_, tesseract_,
                            thresholder_->GetScaleFactor(),
                            thresholder_->GetScaledYResolution(),
                            rect_left_, rect_top_, rect_width_, rect_height_);
}

// Make a text string from the internal data structures.
char* TessBaseAPI::GetUTF8Text() {
  if (tesseract_ == NULL ||
      (!recognition_done_ && Recognize(NULL) < 0))
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
// page_number is 0-based but will appear in the output as 1-based.
// STL removed from original patch submission and refactored by rays.
char* TessBaseAPI::GetHOCRText(int page_number) {
  if (tesseract_ == NULL ||
      (page_res_ == NULL && Recognize(NULL) < 0))
    return NULL;

  PAGE_RES_IT page_res_it(page_res_);
  ROW_RES *row = NULL;           // current row
  ROW *real_row = NULL, *prev_row = NULL;
  BLOCK_RES *block = NULL;       // current row
  BLOCK *real_block = NULL;
  int lcnt = 1, bcnt = 1, wcnt = 1;
  int page_id = page_number + 1;  // hOCR uses 1-based page numbers.

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
    if (block != page_res_it.block()) {
      if (block != NULL) {
        hocr_str += "</span>\n</p>\n</div>\n";
      }

      block = page_res_it.block();  // current row
      real_block = block->block;
      real_row = NULL;
      row = NULL;

      hocr_str.add_str_int("<div class='ocr_carea' id='block_", page_id);
      hocr_str.add_str_int("_", bcnt++);
      AddBoxTohOCR(real_block->bounding_box(), image_height_, &hocr_str);
      hocr_str += "\n<p class='ocr_par'>\n";
    }
    if (row != page_res_it.row()) {
      if (row != NULL) {
        hocr_str += "</span>\n";
      }
      prev_row = real_row;

      row = page_res_it.row();  // current row
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
      hocr_str.add_str_int("<span class='ocrx_word' id='xword_", page_id);
      hocr_str.add_str_int("_", wcnt++);
      hocr_str.add_str_int("' title=\"x_wconf ", choice->certainty());
      hocr_str += "\">";
      if (word->bold > 0)
        hocr_str += "<strong>";
      if (word->italic > 0)
        hocr_str += "<em>";
      int i;
      // escape special characters
      for (i = 0; choice->unichar_string()[i] != '\0'; i++) {
        if (choice->unichar_string()[i] == '<') hocr_str += "&lt;";
        else if (choice->unichar_string()[i] == '>') hocr_str += "&gt;";
        else if (choice->unichar_string()[i] == '&') hocr_str += "&amp;";
        else if (choice->unichar_string()[i] == '"') hocr_str += "&quot;";
        else if (choice->unichar_string()[i] == '\'') hocr_str += "&#39;";
        else hocr_str += choice->unichar_string()[i];
      }
      if (word->italic > 0)
        hocr_str += "</em>";
      if (word->bold > 0)
        hocr_str += "</strong>";
      hocr_str += "</span></span>";
      if (!word->word->flag(W_EOL))
        hocr_str += " ";
    }
  }
  if (block != NULL)
    hocr_str += "</span>\n</p>\n</div>\n";
  hocr_str += "</div>\n";

  char *ret = new char[hocr_str.length() + 1];
  strcpy(ret, hocr_str.string());
  return ret;
}

// The 5 numbers output for each box (the usual 4 and a page number.)
const int kNumbersPerBlob = 5;
// The number of bytes taken by each number. Since we use inT16 for ICOORD,
// assume only 5 digits max.
const int kBytesPerNumber = 5;
// Multiplier for max expected textlength assumes (kBytesPerNumber + space)
// * kNumbersPerBlob plus the newline. Add to this the
// original UTF8 characters, and one kMaxBytesPerLine for safety.
const int kBytesPerBlob = kNumbersPerBlob * (kBytesPerNumber + 1) + 1;
const int kBytesPerBoxFileLine = (kBytesPerNumber + 1) * kNumbersPerBlob + 1;
// Max bytes in the decimal representation of inT64.
const int kBytesPer64BitNumber = 20;
// A maximal single box could occupy kNumbersPerBlob numbers at
// kBytesPer64BitNumber digits (if someone sneaks in a 64 bit value) and a
// space plus the newline and the maximum length of a UNICHAR.
// Test against this on each iteration for safety.
const int kMaxBytesPerLine = kNumbersPerBlob * (kBytesPer64BitNumber + 1) + 1 +
    UNICHAR_LEN;

// The recognized text is returned as a char* which is coded
// as a UTF8 box file and must be freed with the delete [] operator.
// page_number is a 0-base page index that will appear in the box file.
char* TessBaseAPI::GetBoxText(int page_number) {
  if (tesseract_ == NULL ||
      (!recognition_done_ && Recognize(NULL) < 0))
    return NULL;
  int blob_count;
  int utf8_length = TextLength(&blob_count);
  int total_length = blob_count * kBytesPerBoxFileLine + utf8_length +
      kMaxBytesPerLine;
  char* result = new char[total_length];
  int output_length = 0;
  ResultIterator* it = GetIterator();
  do {
    int left, top, right, bottom;
    if (it->BoundingBox(RIL_SYMBOL, &left, &top, &right, &bottom)) {
      char* text = it->GetUTF8Text(RIL_SYMBOL);
      // Tesseract uses space for recognition failure. Fix to a reject
      // character, kTesseractReject so we don't create illegal box files.
      for (int i = 0; text[i] != '\0'; ++i) {
        if (text[i] == ' ')
          text[i] = kTesseractReject;
      }
      snprintf(result + output_length, total_length - output_length,
               "%s %d %d %d %d %d\n",
               text, left, image_height_ - bottom,
               right, image_height_ - top, page_number);
      output_length += strlen(result + output_length);
      delete [] text;
      // Just in case...
      if (output_length + kMaxBytesPerLine > total_length)
        break;
    }
  } while (it->Next(RIL_SYMBOL));
  delete it;
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
      (!recognition_done_ && Recognize(NULL) < 0))
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
      (!recognition_done_ && Recognize(NULL) < 0))
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

/**
   * Applies the given word to the adaptive classifier if possible.
   * The word must be SPACE-DELIMITED UTF-8 - l i k e t h i s , so it can
   * tell the boundaries of the graphemes.
   * Assumes that SetImage/SetRectangle have been used to set the image
   * to the given word. The mode arg should be PSM_SINGLE_WORD or
   * PSM_CIRCLE_WORD, as that will be used to control layout analysis.
   * The currently set PageSegMode is preserved.
   * Returns false if adaption was not possible for some reason.
 */
bool TessBaseAPI::AdaptToWordStr(PageSegMode mode, const char* wordstr) {
  bool success = true;
  PageSegMode current_psm = GetPageSegMode();
  SetPageSegMode(mode);
  SetVariable("classify_enable_learning", "0");
  char* text = GetUTF8Text();
  if (text != NULL) {
    PAGE_RES_IT it(page_res_);
    WERD_RES* word_res = it.word();
    if (word_res != NULL) {
      word_res->word->set_text(wordstr);
    } else {
      success = false;
    }
    // Check to see if text matches wordstr.
    int w = 0;
    int t = 0;
    for (t = 0; text[t] != '\0'; ++t) {
      if (text[t] == '\n' || text[t] == ' ')
        continue;
      while (wordstr[w] != '\0' && wordstr[w] == ' ')
        ++w;
      if (text[t] != wordstr[w])
        break;
      ++w;
    }
    if (text[t] != '\0' || wordstr[w] != '\0') {
      // No match.
      delete page_res_;
      page_res_ = tesseract_->SetupApplyBoxes(block_list_);
      tesseract_->ReSegmentByClassification(page_res_);
      tesseract_->TidyUp(page_res_);
      PAGE_RES_IT pr_it(page_res_);
      if (pr_it.word() == NULL)
        success = false;
      else
        word_res = pr_it.word();
    } else {
      word_res->BestChoiceToCorrectText(tesseract_->unicharset);
    }
    if (success) {
      tesseract_->EnableLearning = true;
      tesseract_->LearnWord(NULL, NULL, word_res);
    }
    delete [] text;
  } else {
    success = false;
  }
  SetPageSegMode(current_psm);
  return success;
}

// Free up recognition results and any stored image data, without actually
// freeing any recognition data that would be time-consuming to reload.
// Afterwards, you must call SetImage or TesseractRect before doing
// any Recognize or Get* operation.
void TessBaseAPI::Clear() {
  if (thresholder_ != NULL)
    thresholder_->Clear();
  ClearResults();
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
    if (osd_tesseract_ == tesseract_)
      osd_tesseract_ = NULL;
    tesseract_ = NULL;
  }
  if (osd_tesseract_ != NULL) {
    osd_tesseract_->end_tesseract();
    delete osd_tesseract_;
    osd_tesseract_ = NULL;
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

// Sets Dict::letter_is_okay_ function to point to the given function.
void TessBaseAPI::SetDictFunc(DictFunc f) {
  if (tesseract_ != NULL) {
    tesseract_->getDict().letter_is_okay_ = f;
  }
}

// Sets Dict::probability_in_context_ function to point to the given function.
void TessBaseAPI::SetProbabilityInContextFunc(ProbabilityInContextFunc f) {
  if (tesseract_ != NULL) {
    tesseract_->getDict().probability_in_context_ = f;
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

// Run the thresholder to make the thresholded image, returned in pix,
// which must not be NULL. *pix must be initialized to NULL, or point
// to an existing pixDestroyable Pix.
// The usual argument to Threshold is Tesseract::mutable_pix_binary().
void TessBaseAPI::Threshold(Pix** pix) {
  ASSERT_HOST(pix != NULL);
  if (!thresholder_->IsBinary()) {
    tesseract_->set_pix_grey(thresholder_->GetPixRectGrey());
  }
  if (*pix != NULL)
    pixDestroy(pix);
  thresholder_->ThresholdToPix(pix);
  thresholder_->GetImageSizes(&rect_left_, &rect_top_,
                              &rect_width_, &rect_height_,
                              &image_width_, &image_height_);
}

// Find lines from the image making the BLOCK_LIST.
int TessBaseAPI::FindLines() {
  if (thresholder_ == NULL || thresholder_->IsEmpty()) {
    tprintf("Please call SetImage before attempting recognition.");
    return -1;
  }
  if (recognition_done_)
    ClearResults();
  if (!block_list_->empty()) {
    return 0;
  }
  if (tesseract_ == NULL) {
    tesseract_ = new Tesseract;
    tesseract_->InitAdaptiveClassifier(false);
  }
  if (tesseract_->pix_binary() == NULL)
    Threshold(tesseract_->mutable_pix_binary());
  if (tesseract_->ImageWidth() > MAX_INT16 ||
      tesseract_->ImageHeight() > MAX_INT16) {
    tprintf("Image too large: (%d, %d)\n",
            tesseract_->ImageWidth(), tesseract_->ImageHeight());
    return -1;
  }

  tesseract_->PrepareForPageseg();

  Tesseract* osd_tess = osd_tesseract_;
  OSResults osr;
  if (PSM_OSD_ENABLED(tesseract_->tessedit_pageseg_mode) && osd_tess == NULL) {
    if (strcmp(language_->string(), "osd") == 0) {
      osd_tess = tesseract_;
    } else {
      osd_tesseract_ = new Tesseract;
      if (osd_tesseract_->init_tesseract(
          datapath_->string(), NULL, "osd", OEM_TESSERACT_ONLY,
          NULL, 0, NULL, NULL, false) == 0) {
        osd_tess = osd_tesseract_;
      } else {
        tprintf("Warning: Auto orientation and script detection requested,"
                " but osd language failed to load\n");
        delete osd_tesseract_;
        osd_tesseract_ = NULL;
      }
    }
  }

  if (tesseract_->SegmentPage(input_file_, block_list_, osd_tess, &osr) < 0)
    return -1;
  // If OCR is to be run using Tesseract, OCR-able blobs are required for
  // training, or interactive mode is needed, prepare data and images for ocr.
  if (tesseract_->interactive_mode ||
      tesseract_->tessedit_train_from_boxes ||
      tesseract_->tessedit_ambigs_training ||
      tesseract_->tessedit_ocr_engine_mode == OEM_TESSERACT_ONLY ||
      tesseract_->tessedit_ocr_engine_mode ==
      OEM_TESSERACT_CUBE_COMBINED) {
    tesseract_->PrepareForTessOCR(block_list_, osd_tess, &osr);
  }
  return 0;
}

// Delete the pageres and clear the block list ready for a new page.
void TessBaseAPI::ClearResults() {
  if (tesseract_ != NULL) {
    tesseract_->Clear();
    tesseract_->ResetFeaturesHaveBeenExtracted();
  }
  if (page_res_ != NULL) {
    delete page_res_;
    page_res_ = NULL;
  }
  recognition_done_ = false;
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
  if (tesseract_->pix_binary() == NULL)
    Threshold(tesseract_->mutable_pix_binary());
  if (input_file_ == NULL)
    input_file_ = new STRING(kInputFile);
  return orientation_and_script_detection(*input_file_, osr, tesseract_);
}

void TessBaseAPI::set_min_orientation_margin(double margin) {
  tesseract_->min_orientation_margin.set_value(margin);
}

// Return text orientation of each block as determined in an earlier page layout
// analysis operation. Orientation is returned as the number of ccw 90-degree
// rotations (in [0..3]) required to make the text in the block upright
// (readable). Note that this may not necessary be the block orientation
// preferred for recognition (such as the case of vertical CJK text).
//
// Also returns whether the text in the block is believed to have vertical
// writing direction (when in an upright page orientation).
//
// The returned array is of length equal to the number of text blocks, which may
// be less than the total number of blocks. The ordering is intended to be
// consistent with GetTextLines().
void TessBaseAPI::GetBlockTextOrientations(int** block_orientation,
                                           bool** vertical_writing) {
  delete[] *block_orientation;
  *block_orientation = NULL;
  delete[] *vertical_writing;
  *vertical_writing = NULL;
  BLOCK_IT block_it(block_list_);

  block_it.move_to_first();
  int num_blocks = 0;
  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    if (!block_it.data()->poly_block()->IsText()) {
      continue;
    }
    ++num_blocks;
  }
  if (!num_blocks) {
    tprintf("WARNING: Found no blocks\n");
    return;
  }
  *block_orientation = new int[num_blocks];
  *vertical_writing = new bool[num_blocks];
  block_it.move_to_first();
  int i = 0;
  for (block_it.mark_cycle_pt(); !block_it.cycled_list();
       block_it.forward()) {
    if (!block_it.data()->poly_block()->IsText()) {
      continue;
    }
    FCOORD re_rotation = block_it.data()->re_rotation();
    float re_theta = re_rotation.angle();
    FCOORD classify_rotation = block_it.data()->classify_rotation();
    float classify_theta = classify_rotation.angle();
    double rot_theta = - (re_theta - classify_theta) * 2.0 / PI;
    if (rot_theta < 0) rot_theta += 4;
    int num_rotations = static_cast<int>(rot_theta + 0.5);
    (*block_orientation)[i] = num_rotations;
    // The classify_rotation is non-zero only if the text has vertical
    // writing direction.
    (*vertical_writing)[i] = classify_rotation.y() != 0.0f;
    ++i;
  }
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


ROW *TessBaseAPI::MakeTessOCRRow(float baseline,
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

// Creates a TBLOB* from the whole pix.
TBLOB *TessBaseAPI::MakeTBLOB(Pix *pix) {
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  BLOCK block("a character", TRUE, 0, 0, 0, 0, width, height);

  // Create C_BLOBs from the page
  extract_edges(pix, &block);

  // Merge all C_BLOBs
  C_BLOB_LIST *list = block.blob_list();
  C_BLOB_IT c_blob_it(list);
  if (c_blob_it.empty())
    return NULL;
  // Move all the outlines to the first blob.
  C_OUTLINE_IT ol_it(c_blob_it.data()->out_list());
  for (c_blob_it.forward();
       !c_blob_it.at_first();
       c_blob_it.forward()) {
      C_BLOB *c_blob = c_blob_it.data();
      ol_it.add_list_after(c_blob->out_list());
  }
  // Convert the first blob to the output TBLOB.
  return TBLOB::PolygonalCopy(c_blob_it.data());
}

// This method baseline normalizes a TBLOB in-place. The input row is used
// for normalization. The denorm is an optional parameter in which the
// normalization-antidote is returned.
void TessBaseAPI::NormalizeTBLOB(TBLOB *tblob, ROW *row,
                                 bool numeric_mode, DENORM *denorm) {
  TWERD word;
  word.blobs = tblob;
  if (denorm != NULL) {
    word.SetupBLNormalize(NULL, row, row->x_height(), numeric_mode, denorm);
    word.Normalize(*denorm);
  } else {
    DENORM normer;
    word.SetupBLNormalize(NULL, row, row->x_height(), numeric_mode, &normer);
    word.Normalize(normer);
  }
  word.blobs = NULL;
}

// Return a TBLOB * from the whole pix.
// To be freed later with delete.
TBLOB *make_tesseract_blob(float baseline, float xheight,
                           float descender, float ascender,
                           bool numeric_mode, Pix* pix) {
  TBLOB *tblob = TessBaseAPI::MakeTBLOB(pix);

  // Normalize TBLOB
  ROW *row =
      TessBaseAPI::MakeTessOCRRow(baseline, xheight, descender, ascender);
  TessBaseAPI::NormalizeTBLOB(tblob, row, numeric_mode, NULL);
  delete row;
  return tblob;
}


// Adapt to recognize the current image as the given character.
// The image must be preloaded into pix_binary_ and be just an image
// of a single character.
void TessBaseAPI::AdaptToCharacter(const char *unichar_repr,
                                   int length,
                                   float baseline,
                                   float xheight,
                                   float descender,
                                   float ascender) {
  UNICHAR_ID id = tesseract_->unicharset.unichar_to_id(unichar_repr, length);
  TBLOB *blob = make_tesseract_blob(baseline, xheight, descender, ascender,
                                    tesseract_->classify_bln_numeric_mode,
                                    tesseract_->pix_binary());
  float threshold;
  UNICHAR_ID best_class = 0;
  float best_rating = -100;


  // Classify to get a raw choice.
  BLOB_CHOICE_LIST choices;
  DENORM denorm;
  tesseract_->set_denorm(&denorm);
  tesseract_->AdaptiveClassifier(blob, &choices, NULL);
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
    threshold = tesseract_->matcher_good_threshold;
  } else {
    /* the blob was incorrectly classified - find the rating threshold
       needed to create a template which will correct the error with
       some margin.  However, don't waste time trying to make
       templates which are too tight. */
    threshold = tesseract_->GetBestRatingFor(blob, id);
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
    tesseract_->AdaptToChar(blob, id, kUnknownFontinfoId, threshold);
  delete blob;
}


PAGE_RES* TessBaseAPI::RecognitionPass1(BLOCK_LIST* block_list) {
  PAGE_RES *page_res = new PAGE_RES(block_list,
                                    &(tesseract_->prev_word_best_choice_));
  tesseract_->recog_all_words(page_res, NULL, NULL, NULL, 1);
  return page_res;
}

PAGE_RES* TessBaseAPI::RecognitionPass2(BLOCK_LIST* block_list,
                                        PAGE_RES* pass1_result) {
  if (!pass1_result)
    pass1_result = new PAGE_RES(block_list,
                                &(tesseract_->prev_word_best_choice_));
  tesseract_->recog_all_words(pass1_result, NULL, NULL, NULL, 2);
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
  rating = 100 + rating;
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
    TBOX real_rect = word->word->bounding_box();

    if (word_count)
      add_space(out);
    int n = strlen(len);
    for (int i = 0; i < n; i++) {
      TESS_CHAR *tc = new TESS_CHAR(rating_to_cost(word->best_choice->rating()),
                                    str, *len);
      tc->box = real_rect.intersection(word->box_word->BlobBox(i));
      out->add_after_then_move(tc);
       str += *len;
      len++;
    }
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

// This method returns the features associated with the input blob.
void TessBaseAPI::GetFeaturesForBlob(TBLOB* blob, const DENORM& denorm,
                                     INT_FEATURE_ARRAY int_features,
                                     int* num_features,
                                     int* FeatureOutlineIndex) {
  if (tesseract_) {
    tesseract_->ResetFeaturesHaveBeenExtracted();
  }
  tesseract_->set_denorm(&denorm);
  CLASS_NORMALIZATION_ARRAY norm_array;
  inT32 len;
  *num_features = tesseract_->GetIntCharNormFeatures(
      blob, tesseract_->PreTrainedTemplates,
      int_features, norm_array, &len, FeatureOutlineIndex);
}

// This method returns the row to which a box of specified dimensions would
// belong. If no good match is found, it returns NULL.
ROW* TessBaseAPI::FindRowForBox(BLOCK_LIST* blocks,
                                int left, int top, int right, int bottom) {
  TBOX box(left, bottom, right, top);
  BLOCK_IT b_it(blocks);
  for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
    BLOCK* block = b_it.data();
    if (!box.major_overlap(block->bounding_box()))
      continue;
    ROW_IT r_it(block->row_list());
    for (r_it.mark_cycle_pt(); !r_it.cycled_list(); r_it.forward()) {
      ROW* row = r_it.data();
      if (!box.major_overlap(row->bounding_box()))
        continue;
      WERD_IT w_it(row->word_list());
      for (w_it.mark_cycle_pt(); !w_it.cycled_list(); w_it.forward()) {
        WERD* word = w_it.data();
        if (box.major_overlap(word->bounding_box()))
          return row;
      }
    }
  }
  return NULL;
}

// Method to run adaptive classifier on a blob.
void TessBaseAPI::RunAdaptiveClassifier(TBLOB* blob, const DENORM& denorm,
                                        int num_max_matches,
                                        int* unichar_ids,
                                        float* ratings,
                                        int* num_matches_returned) {
  BLOB_CHOICE_LIST* choices = new BLOB_CHOICE_LIST;
  tesseract_->set_denorm(&denorm);
  tesseract_->AdaptiveClassifier(blob, choices, NULL);
  BLOB_CHOICE_IT choices_it(choices);
  int& index = *num_matches_returned;
  index = 0;
  for (choices_it.mark_cycle_pt();
       !choices_it.cycled_list() && index < num_max_matches;
       choices_it.forward()) {
    BLOB_CHOICE* choice = choices_it.data();
    unichar_ids[index] = choice->unichar_id();
    ratings[index] = choice->rating();
    ++index;
  }
  *num_matches_returned = index;
  delete choices;
}

// This method returns the string form of the specified unichar.
const char* TessBaseAPI::GetUnichar(int unichar_id) {
  return tesseract_->unicharset.id_to_unichar(unichar_id);
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

// Return a pointer to underlying CubeRecoContext object if present.
CubeRecoContext *TessBaseAPI::GetCubeRecoContext() const {
  return (tesseract_ == NULL) ? NULL : tesseract_->GetCubeRecoContext();
}
}  // namespace tesseract.
