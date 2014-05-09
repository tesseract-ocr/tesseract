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

#ifdef __linux__
#include <signal.h>
#endif

#if defined(_WIN32)
#ifdef _MSC_VER
#include "mathfix.h"
#elif MINGW
// workaround for stdlib.h with -std=c++11 for _splitpath and _MAX_FNAME
#undef __STRICT_ANSI__
#endif  // _MSC_VER
#include <stdlib.h>
#include <windows.h>
#else
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#endif  // _WIN32

#if !defined(VERSION)
#include "version.h"
#endif

#include "allheaders.h"

#include "baseapi.h"
#include "resultiterator.h"
#include "mutableiterator.h"
#include "thresholder.h"
#include "tesseractclass.h"
#include "pageres.h"
#include "paragraphs.h"
#include "tessvars.h"
#include "control.h"
#include "dict.h"
#include "pgedit.h"
#include "paramsd.h"
#include "output.h"
#include "globaloc.h"
#include "globals.h"
#include "edgblob.h"
#include "equationdetect.h"
#include "tessbox.h"
#include "makerow.h"
#include "otsuthr.h"
#include "osdetect.h"
#include "params.h"
#include "renderer.h"
#include "strngs.h"
#include "openclwrapper.h"

namespace tesseract {

/** Minimum sensible image size to be worth running tesseract. */
const int kMinRectSize = 10;
/** Character returned when Tesseract couldn't recognize as anything. */
const char kTesseractReject = '~';
/** Character used by UNLV error counter as a reject. */
const char kUNLVReject = '~';
/** Character used by UNLV as a suspect marker. */
const char kUNLVSuspect = '^';
/**
 * Filename used for input image file, from which to derive a name to search
 * for a possible UNLV zone file, if none is specified by SetInputName.
 */
const char* kInputFile = "noname.tif";
/**
 * Temp file used for storing current parameters before applying retry values.
 */
const char* kOldVarsFile = "failed_vars.txt";
/** Max string length of an int.  */
const int kMaxIntSize = 22;
/**
 * Minimum believable resolution. Used as a default if there is no other
 * information, as it is safer to under-estimate than over-estimate.
 */
const int kMinCredibleResolution = 70;
/** Maximum believable resolution.  */
const int kMaxCredibleResolution = 2400;

TessBaseAPI::TessBaseAPI()
  : tesseract_(NULL),
    osd_tesseract_(NULL),
    equ_detect_(NULL),
    // Thresholder is initialized to NULL here, but will be set before use by:
    // A constructor of a derived API,  SetThresholder(), or
    // created implicitly when used in InternalSetImage.
    thresholder_(NULL),
    paragraph_models_(NULL),
    block_list_(NULL),
    page_res_(NULL),
    input_file_(NULL),
    input_image_(NULL),
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

/**
 * If compiled with OpenCL AND an available OpenCL
 * device is deemed faster than serial code, then
 * "device" is populated with the cl_device_id
 * and returns sizeof(cl_device_id)
 * otherwise *device=NULL and returns 0.
 */
#ifdef USE_OPENCL
#if USE_DEVICE_SELECTION
#include "opencl_device_selection.h"
#endif
#endif
size_t TessBaseAPI::getOpenCLDevice(void **data) {
#ifdef USE_OPENCL
#if USE_DEVICE_SELECTION
  ds_device device = OpenclDevice::getDeviceSelection();
  if (device.type == DS_DEVICE_OPENCL_DEVICE) {
    *data = reinterpret_cast<void*>(new cl_device_id);
    memcpy(*data, &device.oclDeviceID, sizeof(cl_device_id));
    return sizeof(cl_device_id);
  }
#endif
#endif

  *data = NULL;
  return 0;
}

/**
 * Writes the thresholded image to stderr as a PBM file on receipt of a
 * SIGSEGV, SIGFPE, or SIGBUS signal. (Linux/Unix only).
 */
void TessBaseAPI::CatchSignals() {
#ifdef __linux__
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = &signal_exit;
  action.sa_flags = SA_RESETHAND;
  sigaction(SIGSEGV, &action, NULL);
  sigaction(SIGFPE, &action, NULL);
  sigaction(SIGBUS, &action, NULL);
#else
  // Warn API users that an implementation is needed.
  tprintf("CatchSignals has no non-linux implementation!\n");
#endif
}

/**
 * Set the name of the input file. Needed only for training and
 * loading a UNLV zone file.
 */
void TessBaseAPI::SetInputName(const char* name) {
  if (input_file_ == NULL)
    input_file_ = new STRING(name);
  else
    *input_file_ = name;
}

/** Set the name of the output files. Needed only for debugging. */
void TessBaseAPI::SetOutputName(const char* name) {
  if (output_file_ == NULL)
    output_file_ = new STRING(name);
  else
    *output_file_ = name;
}

bool TessBaseAPI::SetVariable(const char* name, const char* value) {
  if (tesseract_ == NULL) tesseract_ = new Tesseract;
  return ParamUtils::SetParam(name, value, SET_PARAM_CONSTRAINT_NON_INIT_ONLY,
                              tesseract_->params());
}

bool TessBaseAPI::SetDebugVariable(const char* name, const char* value) {
  if (tesseract_ == NULL) tesseract_ = new Tesseract;
  return ParamUtils::SetParam(name, value, SET_PARAM_CONSTRAINT_DEBUG_ONLY,
                              tesseract_->params());
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

/** Get value of named variable as a string, if it exists. */
bool TessBaseAPI::GetVariableAsString(const char *name, STRING *val) {
  return ParamUtils::GetParamAsString(name, tesseract_->params(), val);
}

/** Print Tesseract parameters to the given file. */
void TessBaseAPI::PrintVariables(FILE *fp) const {
  ParamUtils::PrintParams(fp, tesseract_->params());
}

/**
 * The datapath must be the name of the data directory (no ending /) or
 * some other file in which the data directory resides (for instance argv[0].)
 * The language is (usually) an ISO 639-3 string or NULL will default to eng.
 * If numeric_mode is true, then only digits and Roman numerals will
 * be returned.
 * @return: 0 on success and -1 on initialization failure.
 */
int TessBaseAPI::Init(const char* datapath, const char* language,
                      OcrEngineMode oem, char **configs, int configs_size,
                      const GenericVector<STRING> *vars_vec,
                      const GenericVector<STRING> *vars_values,
                      bool set_only_non_debug_params) {
  PERF_COUNT_START("TessBaseAPI::Init")
  // Default language is "eng".
  if (language == NULL) language = "eng";
  // If the datapath, OcrEngineMode or the language have changed - start again.
  // Note that the language_ field stores the last requested language that was
  // initialized successfully, while tesseract_->lang stores the language
  // actually used. They differ only if the requested language was NULL, in
  // which case tesseract_->lang is set to the Tesseract default ("eng").
  if (tesseract_ != NULL &&
      (datapath_ == NULL || language_ == NULL ||
       *datapath_ != datapath || last_oem_requested_ != oem ||
       (*language_ != language && tesseract_->lang != language))) {
    delete tesseract_;
    tesseract_ = NULL;
  }
  // PERF_COUNT_SUB("delete tesseract_")
#ifdef USE_OPENCL
  OpenclDevice od;
  od.InitEnv();
#endif
  PERF_COUNT_SUB("OD::InitEnv()")
  bool reset_classifier = true;
  if (tesseract_ == NULL) {
    reset_classifier = false;
    tesseract_ = new Tesseract;
    if (tesseract_->init_tesseract(
        datapath, output_file_ != NULL ? output_file_->string() : NULL,
        language, oem, configs, configs_size, vars_vec, vars_values,
        set_only_non_debug_params) != 0) {
      return -1;
    }
  }
  PERF_COUNT_SUB("update tesseract_")
  // Update datapath and language requested for the last valid initialization.
  if (datapath_ == NULL)
    datapath_ = new STRING(datapath);
  else
    *datapath_ = datapath;
  if ((strcmp(datapath_->string(), "") == 0) &&
      (strcmp(tesseract_->datadir.string(), "") != 0))
     *datapath_ = tesseract_->datadir;

  if (language_ == NULL)
    language_ = new STRING(language);
  else
    *language_ = language;
  last_oem_requested_ = oem;
  // PERF_COUNT_SUB("update last_oem_requested_")
  // For same language and datapath, just reset the adaptive classifier.
  if (reset_classifier) {
    tesseract_->ResetAdaptiveClassifier();
    PERF_COUNT_SUB("tesseract_->ResetAdaptiveClassifier()")
  }
  PERF_COUNT_END
  return 0;
}

/**
 * Returns the languages string used in the last valid initialization.
 * If the last initialization specified "deu+hin" then that will be
 * returned. If hin loaded eng automatically as well, then that will
 * not be included in this list. To find the languages actually
 * loaded use GetLoadedLanguagesAsVector.
 * The returned string should NOT be deleted.
 */
const char* TessBaseAPI::GetInitLanguagesAsString() const {
  return (language_ == NULL || language_->string() == NULL) ?
      "" : language_->string();
}

/**
 * Returns the loaded languages in the vector of STRINGs.
 * Includes all languages loaded by the last Init, including those loaded
 * as dependencies of other loaded languages.
 */
void TessBaseAPI::GetLoadedLanguagesAsVector(
    GenericVector<STRING>* langs) const {
  langs->clear();
  if (tesseract_ != NULL) {
    langs->push_back(tesseract_->lang);
    int num_subs = tesseract_->num_sub_langs();
    for (int i = 0; i < num_subs; ++i)
      langs->push_back(tesseract_->get_sub_lang(i)->lang);
  }
}

/**
 * Returns the available languages in the vector of STRINGs.
 */
void TessBaseAPI::GetAvailableLanguagesAsVector(
    GenericVector<STRING>* langs) const {
  langs->clear();
  if (tesseract_ != NULL) {
#ifdef _WIN32
    STRING pattern = tesseract_->datadir + "/*." + kTrainedDataSuffix;
    char fname[_MAX_FNAME];
    WIN32_FIND_DATA data;
    BOOL result = TRUE;
    HANDLE handle = FindFirstFile(pattern.string(), &data);
    if (handle != INVALID_HANDLE_VALUE) {
      for (; result; result = FindNextFile(handle, &data)) {
        _splitpath(data.cFileName, NULL, NULL, fname, NULL);
        langs->push_back(STRING(fname));
      }
      FindClose(handle);
    }
#else  // _WIN32
    DIR *dir;
    struct dirent *dirent;
    char *dot;

    STRING extension = STRING(".") + kTrainedDataSuffix;

    dir = opendir(tesseract_->datadir.string());
    if (dir != NULL) {
      while ((dirent = readdir(dir))) {
        // Skip '.', '..', and hidden files
        if (dirent->d_name[0] != '.') {
          if (strstr(dirent->d_name, extension.string()) != NULL) {
            dot = strrchr(dirent->d_name, '.');
            // This ensures that .traineddata is at the end of the file name
            if (strncmp(dot, extension.string(),
                        strlen(extension.string())) == 0) {
              *dot = '\0';
              langs->push_back(STRING(dirent->d_name));
            }
          }
        }
      }
      closedir(dir);
    }
#endif
  }
}

/**
 * Init only the lang model component of Tesseract. The only functions
 * that work after this init are SetVariable and IsValidWord.
 * WARNING: temporary! This function will be removed from here and placed
 * in a separate API at some future time.
 */
int TessBaseAPI::InitLangMod(const char* datapath, const char* language) {
  if (tesseract_ == NULL)
    tesseract_ = new Tesseract;
  else
    ParamUtils::ResetToDefaults(tesseract_->params());
  return tesseract_->init_tesseract_lm(datapath, NULL, language);
}

/**
 * Init only for page layout analysis. Use only for calls to SetImage and
 * AnalysePage. Calls that attempt recognition will generate an error.
 */
void TessBaseAPI::InitForAnalysePage() {
  if (tesseract_ == NULL) {
    tesseract_ = new Tesseract;
    tesseract_->InitAdaptiveClassifier(false);
  }
}

/**
 * Read a "config" file containing a set of parameter name, value pairs.
 * Searches the standard places: tessdata/configs, tessdata/tessconfigs
 * and also accepts a relative or absolute path name.
 */
void TessBaseAPI::ReadConfigFile(const char* filename) {
  tesseract_->read_config_file(filename, SET_PARAM_CONSTRAINT_NON_INIT_ONLY);
}

/** Same as above, but only set debug params from the given config file. */
void TessBaseAPI::ReadDebugConfigFile(const char* filename) {
  tesseract_->read_config_file(filename, SET_PARAM_CONSTRAINT_DEBUG_ONLY);
}

/**
 * Set the current page segmentation mode. Defaults to PSM_AUTO.
 * The mode is stored as an IntParam so it can also be modified by
 * ReadConfigFile or SetVariable("tessedit_pageseg_mode", mode as string).
 */
void TessBaseAPI::SetPageSegMode(PageSegMode mode) {
  if (tesseract_ == NULL)
    tesseract_ = new Tesseract;
  tesseract_->tessedit_pageseg_mode.set_value(mode);
}

/** Return the current page segmentation mode. */
PageSegMode TessBaseAPI::GetPageSegMode() const {
  if (tesseract_ == NULL)
    return PSM_SINGLE_BLOCK;
  return static_cast<PageSegMode>(
    static_cast<int>(tesseract_->tessedit_pageseg_mode));
}

/**
 * Recognize a rectangle from an image and return the result as a string.
 * May be called many times for a single Init.
 * Currently has no error checking.
 * Greyscale of 8 and color of 24 or 32 bits per pixel may be given.
 * Palette color images will not work properly and must be converted to
 * 24 bit.
 * Binary images of 1 bit per pixel may also be given but they must be
 * byte packed with the MSB of the first byte being the first pixel, and a
 * one pixel is WHITE. For binary images set bytes_per_pixel=0.
 * The recognized text is returned as a char* which is coded
 * as UTF8 and must be freed with the delete [] operator.
 */
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

/**
 * Call between pages or documents etc to free up memory and forget
 * adaptive data.
 */
void TessBaseAPI::ClearAdaptiveClassifier() {
  if (tesseract_ == NULL)
    return;
  tesseract_->ResetAdaptiveClassifier();
  tesseract_->ResetDocumentDictionary();
}

/**
 * Provide an image for Tesseract to recognize. Format is as
 * TesseractRect above. Does not copy the image buffer, or take
 * ownership. The source image may be destroyed after Recognize is called,
 * either explicitly or implicitly via one of the Get*Text functions.
 * SetImage clears all recognition results, and sets the rectangle to the
 * full image, so it may be followed immediately by a GetUTF8Text, and it
 * will automatically perform recognition.
 */
void TessBaseAPI::SetImage(const unsigned char* imagedata,
                           int width, int height,
                           int bytes_per_pixel, int bytes_per_line) {
  if (InternalSetImage())
    thresholder_->SetImage(imagedata, width, height,
                           bytes_per_pixel, bytes_per_line);
}

void TessBaseAPI::SetSourceResolution(int ppi) {
  if (thresholder_)
    thresholder_->SetSourceYResolution(ppi);
  else
    tprintf("Please call SetImage before SetSourceResolution.\n");
}

/**
 * Provide an image for Tesseract to recognize. As with SetImage above,
 * Tesseract doesn't take a copy or ownership or pixDestroy the image, so
 * it must persist until after Recognize.
 * Pix vs raw, which to use?
 * Use Pix where possible. A future version of Tesseract may choose to use Pix
 * as its internal representation and discard IMAGE altogether.
 * Because of that, an implementation that sources and targets Pix may end up
 * with less copies than an implementation that does not.
 */
void TessBaseAPI::SetImage(const Pix* pix) {
  if (InternalSetImage())
    thresholder_->SetImage(pix);
}

/**
 * Restrict recognition to a sub-rectangle of the image. Call after SetImage.
 * Each SetRectangle clears the recogntion results so multiple rectangles
 * can be recognized with the same image.
 */
void TessBaseAPI::SetRectangle(int left, int top, int width, int height) {
  if (thresholder_ == NULL)
    return;
  thresholder_->SetRectangle(left, top, width, height);
  ClearResults();
}

/**
 * ONLY available if you have Leptonica installed.
 * Get a copy of the internal thresholded image from Tesseract.
 */
Pix* TessBaseAPI::GetThresholdedImage() {
  if (tesseract_ == NULL)
    return NULL;
  if (tesseract_->pix_binary() == NULL)
    Threshold(tesseract_->mutable_pix_binary());
  return pixClone(tesseract_->pix_binary());
}

/**
 * Get the result of page layout analysis as a leptonica-style
 * Boxa, Pixa pair, in reading order.
 * Can be called before or after Recognize.
 */
Boxa* TessBaseAPI::GetRegions(Pixa** pixa) {
  return GetComponentImages(RIL_BLOCK, false, pixa, NULL);
}

/**
 * Get the textlines as a leptonica-style Boxa, Pixa pair, in reading order.
 * Can be called before or after Recognize.
 * If blockids is not NULL, the block-id of each line is also returned as an
 * array of one element per line. delete [] after use.
 * If paraids is not NULL, the paragraph-id of each line within its block is
 * also returned as an array of one element per line. delete [] after use.
 */
Boxa* TessBaseAPI::GetTextlines(const bool raw_image, const int raw_padding,
                                Pixa** pixa, int** blockids, int** paraids) {
  return GetComponentImages(RIL_TEXTLINE, true, raw_image, raw_padding,
                            pixa, blockids, paraids);
}

/**
 * Get textlines and strips of image regions as a leptonica-style Boxa, Pixa
 * pair, in reading order. Enables downstream handling of non-rectangular
 * regions.
 * Can be called before or after Recognize.
 * If blockids is not NULL, the block-id of each line is also returned as an
 * array of one element per line. delete [] after use.
 */
Boxa* TessBaseAPI::GetStrips(Pixa** pixa, int** blockids) {
  return GetComponentImages(RIL_TEXTLINE, false, pixa, blockids);
}

/**
 * Get the words as a leptonica-style
 * Boxa, Pixa pair, in reading order.
 * Can be called before or after Recognize.
 */
Boxa* TessBaseAPI::GetWords(Pixa** pixa) {
  return GetComponentImages(RIL_WORD, true, pixa, NULL);
}

/**
 * Gets the individual connected (text) components (created
 * after pages segmentation step, but before recognition)
 * as a leptonica-style Boxa, Pixa pair, in reading order.
 * Can be called before or after Recognize.
 */
Boxa* TessBaseAPI::GetConnectedComponents(Pixa** pixa) {
  return GetComponentImages(RIL_SYMBOL, true, pixa, NULL);
}

/**
 * Get the given level kind of components (block, textline, word etc.) as a
 * leptonica-style Boxa, Pixa pair, in reading order.
 * Can be called before or after Recognize.
 * If blockids is not NULL, the block-id of each component is also returned
 * as an array of one element per component. delete [] after use.
 * If text_only is true, then only text components are returned.
 */
Boxa* TessBaseAPI::GetComponentImages(PageIteratorLevel level,
                                      bool text_only, bool raw_image,
                                      const int raw_padding,
                                      Pixa** pixa, int** blockids,
                                      int** paraids) {
  PageIterator* page_it = GetIterator();
  if (page_it == NULL)
    page_it = AnalyseLayout();
  if (page_it == NULL)
    return NULL;  // Failed.

  // Count the components to get a size for the arrays.
  int component_count = 0;
  int left, top, right, bottom;

  TessResultCallback<bool>* get_bbox = NULL;
  if (raw_image) {
    // Get bounding box in original raw image with padding.
    get_bbox = NewPermanentTessCallback(page_it, &PageIterator::BoundingBox,
                                        level, raw_padding,
                                        &left, &top, &right, &bottom);
  } else {
    // Get bounding box from binarized imaged. Note that this could be
    // differently scaled from the original image.
    get_bbox = NewPermanentTessCallback(page_it,
                                        &PageIterator::BoundingBoxInternal,
                                        level, &left, &top, &right, &bottom);
  }
  do {
    if (get_bbox->Run() &&
        (!text_only || PTIsTextType(page_it->BlockType())))
      ++component_count;
  } while (page_it->Next(level));

  Boxa* boxa = boxaCreate(component_count);
  if (pixa != NULL)
    *pixa = pixaCreate(component_count);
  if (blockids != NULL)
    *blockids = new int[component_count];
  if (paraids != NULL)
    *paraids = new int[component_count];

  int blockid = 0;
  int paraid = 0;
  int component_index = 0;
  page_it->Begin();
  do {
    if (get_bbox->Run() &&
        (!text_only || PTIsTextType(page_it->BlockType()))) {
      Box* lbox = boxCreate(left, top, right - left, bottom - top);
      boxaAddBox(boxa, lbox, L_INSERT);
      if (pixa != NULL) {
        Pix* pix = NULL;
        if (raw_image) {
          pix = page_it->GetImage(level, raw_padding, &left, &top);
        } else {
          pix = page_it->GetBinaryImage(level);
        }
        pixaAddPix(*pixa, pix, L_INSERT);
        pixaAddBox(*pixa, lbox, L_CLONE);
      }
      if (paraids != NULL) {
        (*paraids)[component_index] = paraid;
        if (page_it->IsAtFinalElement(RIL_PARA, level))
          ++paraid;
      }
      if (blockids != NULL) {
        (*blockids)[component_index] = blockid;
        if (page_it->IsAtFinalElement(RIL_BLOCK, level)) {
          ++blockid;
          paraid = 0;
        }
      }
      ++component_index;
    }
  } while (page_it->Next(level));
  delete page_it;
  delete get_bbox;
  return boxa;
}

int TessBaseAPI::GetThresholdedImageScaleFactor() const {
  if (thresholder_ == NULL) {
    return 0;
  }
  return thresholder_->GetScaleFactor();
}

/** Dump the internal binary image to a PGM file. */
void TessBaseAPI::DumpPGM(const char* filename) {
  if (tesseract_ == NULL)
    return;
  FILE *fp = fopen(filename, "wb");
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

/**
 * Placeholder for call to Cube and test that the input data is correct.
 * reskew is the direction of baselines in the skewed image in
 * normalized (cos theta, sin theta) form, so (0.866, 0.5) would represent
 * a 30 degree anticlockwise skew.
 */
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

/**
 * Runs page layout analysis in the mode set by SetPageSegMode.
 * May optionally be called prior to Recognize to get access to just
 * the page layout results. Returns an iterator to the results.
 * Returns NULL on error or an empty page.
 * The returned iterator must be deleted after use.
 * WARNING! This class points to data held within the TessBaseAPI class, and
 * therefore can only be used while the TessBaseAPI class still exists and
 * has not been subjected to a call of Init, SetImage, Recognize, Clear, End
 * DetectOS, or anything else that changes the internal PAGE_RES.
 */
PageIterator* TessBaseAPI::AnalyseLayout() {
  if (FindLines() == 0) {
    if (block_list_->empty())
      return NULL;  // The page was empty.
    page_res_ = new PAGE_RES(block_list_, NULL);
    DetectParagraphs(false);
    return new PageIterator(
        page_res_, tesseract_, thresholder_->GetScaleFactor(),
        thresholder_->GetScaledYResolution(),
        rect_left_, rect_top_, rect_width_, rect_height_);
  }
  return NULL;
}

/**
 * Recognize the tesseract global image and return the result as Tesseract
 * internal structures.
 */
int TessBaseAPI::Recognize(ETEXT_DESC* monitor) {
  if (tesseract_ == NULL)
    return -1;
  if (FindLines() != 0)
    return -1;
  if (page_res_ != NULL)
    delete page_res_;
  if (block_list_->empty()) {
    page_res_ = new PAGE_RES(block_list_, &tesseract_->prev_word_best_choice_);
    return 0; // Empty page.
  }

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

  if (truth_cb_ != NULL) {
    tesseract_->wordrec_run_blamer.set_value(true);
    PageIterator *page_it = new PageIterator(
            page_res_, tesseract_, thresholder_->GetScaleFactor(),
            thresholder_->GetScaledYResolution(),
            rect_left_, rect_top_, rect_width_, rect_height_);
    truth_cb_->Run(tesseract_->getDict().getUnicharset(),
                   image_height_, page_it, this->tesseract()->pix_grey());
    delete page_it;
  }

  int result = 0;
  if (tesseract_->interactive_display_mode) {
    #ifndef GRAPHICS_DISABLED
    tesseract_->pgeditor_main(rect_width_, rect_height_, page_res_);
    #endif  // GRAPHICS_DISABLED
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
    bool wait_for_text = true;
    GetBoolVariable("paragraph_text_based", &wait_for_text);
    if (!wait_for_text) DetectParagraphs(false);
    if (tesseract_->recog_all_words(page_res_, monitor, NULL, NULL, 0)) {
      if (wait_for_text) DetectParagraphs(true);
    } else {
      result = -1;
    }
  }
  return result;
}

/** Tests the chopper by exhaustively running chop_one_blob. */
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
  if (tesseract_->interactive_display_mode) return -1;

  recognition_done_ = true;

  page_res_ = new PAGE_RES(block_list_, &(tesseract_->prev_word_best_choice_));

  PAGE_RES_IT page_res_it(page_res_);

  while (page_res_it.word() != NULL) {
    WERD_RES *word_res = page_res_it.word();
    GenericVector<TBOX> boxes;
    tesseract_->MaximallyChopWord(boxes, page_res_it.block()->block,
                                  page_res_it.row()->row, word_res);
    page_res_it.forward();
  }
  return 0;
}

/**
 * Recognizes all the pages in the named file, as a multi-page tiff or
 * list of filenames, or single image, and gets the appropriate kind of text
 * according to parameters: tessedit_create_boxfile,
 * tessedit_make_boxes_from_boxes, tessedit_write_unlv, tessedit_create_hocr.
 * Calls ProcessPage on each page in the input file, which may be a
 * multi-page tiff, single-page other file format, or a plain text list of
 * images to read. If tessedit_page_number is non-negative, processing begins
 * at that page of a multi-page tiff file, or filelist.
 * The text is returned in text_out. Returns false on error.
 * If non-zero timeout_millisec terminates processing after the timeout on
 * a single page.
 * If non-NULL and non-empty, and some page fails for some reason,
 * the page is reprocessed with the retry_config config file. Useful
 * for interactively debugging a bad page.
 */
bool TessBaseAPI::ProcessPages(const char* filename,
                               const char* retry_config, int timeout_millisec,
                               STRING* text_out) {
  TessResultRenderer* renderer = NewRenderer();

  if (!ProcessPages(filename, retry_config, timeout_millisec, renderer)) {
    delete renderer;
    return false;
  }

  const char* out_data;
  inT32 out_len;
  bool success = renderer->GetOutput(&out_data, &out_len);
  if (success) {
    // TODO(ewiseblatt): 20111103
    // if text_out->size() != out_len then we have binary data which STRING wont
    // support so this should fail. Really want to eliminate this interface
    // alltogether so not worrying about at this time.
    text_out->assign(out_data, out_len);
  }
  delete renderer;
  return success;
}

void TessBaseAPI::SetInputImage(Pix *pix) {
  if (input_image_)
    pixDestroy(&input_image_);
  input_image_ = pixClone(pix);
}

Pix* TessBaseAPI::GetInputImage() {
  return input_image_;
}

const char * TessBaseAPI::GetInputName() {
  if (input_file_)
    return input_file_->c_str();
  return NULL;
}

const char *  TessBaseAPI::GetDatapath() {
  return tesseract_->datadir.c_str();
}

int TessBaseAPI::GetSourceYResolution() {
  return thresholder_->GetSourceYResolution();
}

bool TessBaseAPI::ProcessPages(const char* filename,
                               const char* retry_config, int timeout_millisec,
                               TessResultRenderer* renderer) {
  PERF_COUNT_START("ProcessPages")
  int page = tesseract_->tessedit_page_number;
  if (page < 0)
    page = 0;
  FILE* fp = fopen(filename, "rb");
  if (fp == NULL) {
    tprintf("Image file %s cannot be opened!\n", filename);
    return false;
  }
  // Find the number of pages if a tiff file, or zero otherwise.
  int npages = 0;
  int format;
  Pix *pix;
  pix = pixRead(filename);
  format = pixGetInputFormat(pix);
  if (format == IFF_TIFF || format == IFF_TIFF_PACKBITS ||
      format == IFF_TIFF_RLE || format == IFF_TIFF_G3 ||
      format == IFF_TIFF_G4 || format == IFF_TIFF_LZW ||
      format == IFF_TIFF_ZIP)
    tiffGetCount(fp, &npages);
  fclose(fp);

  bool success = true;
  const char* kUnknownTitle = "";
  if (renderer && !renderer->BeginDocument(kUnknownTitle)) {
    success = false;
  }

#ifdef USE_OPENCL
  OpenclDevice od;
#endif

  if (npages > 0) {
    pixDestroy(&pix);
    for (; page < npages; ++page) {
      // only use opencl if compiled w/ OpenCL and selected device is opencl
#ifdef USE_OPENCL
      if ( od.selectedDeviceIsOpenCL() ) {
        pix = od.pixReadTiffCl(filename, page);
      } else {
#endif
        pix = pixReadTiff(filename, page);
#ifdef USE_OPENCL
      }
#endif

      if (pix == NULL) break;

      if ((page >= 0) && (npages > 1))
        tprintf("Page %d of %d\n", page + 1, npages);
      char page_str[kMaxIntSize];
      snprintf(page_str, kMaxIntSize - 1, "%d", page);
      SetVariable("applybox_page", page_str);
      success &= ProcessPage(pix, page, filename, retry_config,
                             timeout_millisec, renderer);
      pixDestroy(&pix);
      if (tesseract_->tessedit_page_number >= 0 || npages == 1) {
        break;
      }
    }
  } else {
    // The file is not a tiff file.
    if (pix != NULL) {
      success &= ProcessPage(pix, 0, filename, retry_config,
                             timeout_millisec, renderer);
      pixDestroy(&pix);
    } else {
      // The file is not an image file, so try it as a list of filenames.
      FILE* fimg = fopen(filename, "rb");
      if (fimg == NULL) {
        tprintf("File %s cannot be opened!\n", filename);
        return false;
      }
      tprintf("Reading %s as a list of filenames...\n", filename);
      char pagename[MAX_PATH];
      // Skip to the requested page number.
      for (int i = 0; i < page &&
           fgets(pagename, sizeof(pagename), fimg) != NULL;
           ++i);
      while (fgets(pagename, sizeof(pagename), fimg) != NULL) {
        chomp_string(pagename);
        pix = pixRead(pagename);
        if (pix == NULL) {
          tprintf("Image file %s cannot be read!\n", pagename);
          fclose(fimg);
          return false;
        }
        tprintf("Page %d : %s\n", page, pagename);
        success &= ProcessPage(pix, page, pagename, retry_config,
                               timeout_millisec, renderer);
        pixDestroy(&pix);
        ++page;
      }
      fclose(fimg);
    }
  }

  bool all_ok = success;
  if (renderer && !renderer->EndDocument()) {
    all_ok = false;
  }
  PERF_COUNT_END
  return all_ok;
}

/**
 * Recognizes a single page for ProcessPages, appending the text to text_out.
 * The pix is the image processed - filename and page_index are metadata
 * used by side-effect processes, such as reading a box file or formatting
 * as hOCR.
 * If non-zero timeout_millisec terminates processing after the timeout.
 * If non-NULL and non-empty, and some page fails for some reason,
 * the page is reprocessed with the retry_config config file. Useful
 * for interactively debugging a bad page.
 * The text is returned in text_out. Returns false on error.
 */
bool TessBaseAPI::ProcessPage(Pix* pix, int page_index, const char* filename,
                              const char* retry_config, int timeout_millisec,
                              STRING* text_out) {
  TessResultRenderer* renderer = NewRenderer();

  if (!ProcessPage(pix, page_index, filename, retry_config, timeout_millisec,
                   renderer)) {
    return false;
  }

  const char* out_data;
  inT32 out_len;
  if (!renderer->GetOutput(&out_data, &out_len)) {
    return false;
  }

  // TODO(ewiseblatt): 20111103
  // if text_out->size() != out_len then we have binary data which STRING wont
  // support so this should fail. Really want to eliminate this interface
  // alltogether so not worrying about at this time.
  text_out->assign(out_data, out_len);

  return true;
}

/**
 * Recognizes a single page for ProcessPages, appending the text to text_out.
 * The pix is the image processed - filename and page_index are metadata
 * used by side-effect processes, such as reading a box file or formatting
 * as hOCR.
 * If non-zero timeout_millisec terminates processing after the timeout.
 * If non-NULL and non-empty, and some page fails for some reason,
 * the page is reprocessed with the retry_config config file. Useful
 * for interactively debugging a bad page.
 * The text is returned in renderer. Returns false on error.
 */
bool TessBaseAPI::ProcessPage(Pix* pix, int page_index, const char* filename,
                              const char* retry_config, int timeout_millisec,
                              TessResultRenderer* renderer) {
  PERF_COUNT_START("ProcessPage")
  SetInputName(filename);
  SetImage(pix);
  SetInputImage(pix);
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
      PERF_COUNT_END
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
    FILE* fp = fopen(kOldVarsFile, "wb");
    PrintVariables(fp);
    fclose(fp);
    // Switch to alternate mode for retry.
    ReadConfigFile(retry_config);
    SetImage(pix);
    Recognize(NULL);
    // Restore saved config variables.
    ReadConfigFile(kOldVarsFile);
  }

  if (renderer) {
    if (failed) {
      renderer->AddError(this);
    } else {
      failed = !renderer->AddImage(this);
    }
  }
  PERF_COUNT_END
  return !failed;
}

/**
 * Get a left-to-right iterator to the results of LayoutAnalysis and/or
 * Recognize. The returned iterator must be deleted after use.
 */
LTRResultIterator* TessBaseAPI::GetLTRIterator() {
  if (tesseract_ == NULL || page_res_ == NULL)
    return NULL;
  return new LTRResultIterator(
      page_res_, tesseract_,
      thresholder_->GetScaleFactor(), thresholder_->GetScaledYResolution(),
      rect_left_, rect_top_, rect_width_, rect_height_);
}

/**
 * Get a reading-order iterator to the results of LayoutAnalysis and/or
 * Recognize. The returned iterator must be deleted after use.
 * WARNING! This class points to data held within the TessBaseAPI class, and
 * therefore can only be used while the TessBaseAPI class still exists and
 * has not been subjected to a call of Init, SetImage, Recognize, Clear, End
 * DetectOS, or anything else that changes the internal PAGE_RES.
 */
ResultIterator* TessBaseAPI::GetIterator() {
  if (tesseract_ == NULL || page_res_ == NULL)
    return NULL;
  return ResultIterator::StartOfParagraph(LTRResultIterator(
      page_res_, tesseract_,
      thresholder_->GetScaleFactor(), thresholder_->GetScaledYResolution(),
      rect_left_, rect_top_, rect_width_, rect_height_));
}

/**
 * Get a mutable iterator to the results of LayoutAnalysis and/or Recognize.
 * The returned iterator must be deleted after use.
 * WARNING! This class points to data held within the TessBaseAPI class, and
 * therefore can only be used while the TessBaseAPI class still exists and
 * has not been subjected to a call of Init, SetImage, Recognize, Clear, End
 * DetectOS, or anything else that changes the internal PAGE_RES.
 */
MutableIterator* TessBaseAPI::GetMutableIterator() {
  if (tesseract_ == NULL || page_res_ == NULL)
    return NULL;
  return new MutableIterator(page_res_, tesseract_,
                             thresholder_->GetScaleFactor(),
                             thresholder_->GetScaledYResolution(),
                             rect_left_, rect_top_, rect_width_, rect_height_);
}

/** Make a text string from the internal data structures. */
char* TessBaseAPI::GetUTF8Text() {
  if (tesseract_ == NULL ||
      (!recognition_done_ && Recognize(NULL) < 0))
    return NULL;
  STRING text("");
  ResultIterator *it = GetIterator();
  do {
    if (it->Empty(RIL_PARA)) continue;
    char *para_text = it->GetUTF8Text(RIL_PARA);
    text += para_text;
    delete []para_text;
  } while (it->Next(RIL_PARA));
  char* result = new char[text.length() + 1];
  strncpy(result, text.string(), text.length() + 1);
  delete it;
  return result;
}

/**
 * Gets the block orientation at the current iterator position.
 */
static tesseract::Orientation GetBlockTextOrientation(const PageIterator *it) {
  tesseract::Orientation orientation;
  tesseract::WritingDirection writing_direction;
  tesseract::TextlineOrder textline_order;
  float deskew_angle;
  it->Orientation(&orientation, &writing_direction, &textline_order,
                  &deskew_angle);
  return orientation;
}

/**
 * Fits a line to the baseline at the given level, and appends its coefficients
 * to the hOCR string.
 * NOTE: The hOCR spec is unclear on how to specify baseline coefficients for
 * rotated textlines. For this reason, on textlines that are not upright, this
 * method currently only inserts a 'textangle' property to indicate the rotation
 * direction and does not add any baseline information to the hocr string.
 */
static void AddBaselineCoordsTohOCR(const PageIterator *it,
                                    PageIteratorLevel level,
                                    STRING* hocr_str) {
  tesseract::Orientation orientation = GetBlockTextOrientation(it);
  if (orientation != ORIENTATION_PAGE_UP) {
    hocr_str->add_str_int("; textangle ", 360 - orientation * 90);
    return;
  }

  int left, top, right, bottom;
  it->BoundingBox(level, &left, &top, &right, &bottom);

  // Try to get the baseline coordinates at this level.
  int x1, y1, x2, y2;
  if (!it->Baseline(level, &x1, &y1, &x2, &y2))
    return;
  // Following the description of this field of the hOCR spec, we convert the
  // baseline coordinates so that "the bottom left of the bounding box is the
  // origin".
  x1 -= left;
  x2 -= left;
  y1 -= bottom;
  y2 -= bottom;

  // Now fit a line through the points so we can extract coefficients for the
  // equation:  y = p1 x + p0
  double p1 = 0;
  double p0 = 0;
  if (x1 == x2) {
    // Problem computing the polynomial coefficients.
    return;
  }
  p1 = (y2 - y1) / static_cast<double>(x2 - x1);
  p0 = y1 - static_cast<double>(p1 * x1);

  hocr_str->add_str_double("; baseline ", round(p1 * 1000.0) / 1000.0);
  hocr_str->add_str_double(" ", round(p0 * 1000.0) / 1000.0);
}

static void AddBoxTohOCR(const PageIterator *it,
                         PageIteratorLevel level,
                         STRING* hocr_str) {
  int left, top, right, bottom;
  it->BoundingBox(level, &left, &top, &right, &bottom);
  hocr_str->add_str_int("' title=\"bbox ", left);
  hocr_str->add_str_int(" ", top);
  hocr_str->add_str_int(" ", right);
  hocr_str->add_str_int(" ", bottom);
  // Add baseline coordinates for textlines only.
  if (level == RIL_TEXTLINE)
    AddBaselineCoordsTohOCR(it, level, hocr_str);
  *hocr_str += "\">";
}

/**
 * Make a HTML-formatted string with hOCR markup from the internal
 * data structures.
 * page_number is 0-based but will appear in the output as 1-based.
 * Image name/input_file_ can be set by SetInputName before calling
 * GetHOCRText
 * STL removed from original patch submission and refactored by rays.
 */
char* TessBaseAPI::GetHOCRText(int page_number) {
  if (tesseract_ == NULL ||
      (page_res_ == NULL && Recognize(NULL) < 0))
    return NULL;

  int lcnt = 1, bcnt = 1, pcnt = 1, wcnt = 1;
  int page_id = page_number + 1;  // hOCR uses 1-based page numbers.

  STRING hocr_str("");

  if (input_file_ == NULL)
      SetInputName(NULL);

#ifdef _WIN32
  // convert input name from ANSI encoding to utf-8
  int str16_len = MultiByteToWideChar(CP_ACP, 0, input_file_->string(), -1,
                                      NULL, NULL);
  wchar_t *uni16_str = new WCHAR[str16_len];
  str16_len = MultiByteToWideChar(CP_ACP, 0, input_file_->string(), -1,
                                  uni16_str, str16_len);
  int utf8_len = WideCharToMultiByte(CP_UTF8, 0, uni16_str, str16_len, NULL,
                                     NULL, NULL, NULL);
  char *utf8_str = new char[utf8_len];
  WideCharToMultiByte(CP_UTF8, 0, uni16_str, str16_len, utf8_str,
                      utf8_len, NULL, NULL);
  *input_file_ = utf8_str;
  delete[] uni16_str;
  delete[] utf8_str;
#endif

  hocr_str.add_str_int("  <div class='ocr_page' id='page_", page_id);
  hocr_str += "' title='image \"";
  hocr_str += input_file_ ? HOcrEscape(input_file_->string()) : "unknown";
  hocr_str.add_str_int("\"; bbox ", rect_left_);
  hocr_str.add_str_int(" ", rect_top_);
  hocr_str.add_str_int(" ", rect_width_);
  hocr_str.add_str_int(" ", rect_height_);
  hocr_str.add_str_int("; ppageno ", page_number);
  hocr_str += "'>\n";

  ResultIterator *res_it = GetIterator();
  while (!res_it->Empty(RIL_BLOCK)) {
    if (res_it->Empty(RIL_WORD)) {
      res_it->Next(RIL_WORD);
      continue;
    }

    // Open any new block/paragraph/textline.
    if (res_it->IsAtBeginningOf(RIL_BLOCK)) {
      hocr_str.add_str_int("   <div class='ocr_carea' id='block_", page_id);
      hocr_str.add_str_int("_", bcnt);
      AddBoxTohOCR(res_it, RIL_BLOCK, &hocr_str);
    }
    if (res_it->IsAtBeginningOf(RIL_PARA)) {
      if (res_it->ParagraphIsLtr()) {
        hocr_str.add_str_int("\n    <p class='ocr_par' dir='ltr' id='par_",
                             page_id);
        hocr_str.add_str_int("_", pcnt);
      } else {
        hocr_str.add_str_int("\n    <p class='ocr_par' dir='rtl' id='par_",
                             page_id);
        hocr_str.add_str_int("_", pcnt);
      }
      AddBoxTohOCR(res_it, RIL_PARA, &hocr_str);
    }
    if (res_it->IsAtBeginningOf(RIL_TEXTLINE)) {
      hocr_str.add_str_int("\n     <span class='ocr_line' id='line_", page_id);
      hocr_str.add_str_int("_", lcnt);
      AddBoxTohOCR(res_it, RIL_TEXTLINE, &hocr_str);
    }

    // Now, process the word...
    hocr_str.add_str_int("<span class='ocrx_word' id='word_", page_id);
    hocr_str.add_str_int("_", wcnt);
    int left, top, right, bottom;
    res_it->BoundingBox(RIL_WORD, &left, &top, &right, &bottom);
    hocr_str.add_str_int("' title='bbox ", left);
    hocr_str.add_str_int(" ", top);
    hocr_str.add_str_int(" ", right);
    hocr_str.add_str_int(" ", bottom);
    hocr_str.add_str_int("; x_wconf ", res_it->Confidence(RIL_WORD));
    hocr_str += "'";
    if (res_it->WordRecognitionLanguage()) {
      hocr_str += " lang='";
      hocr_str += res_it->WordRecognitionLanguage();
      hocr_str += "'";
    }
    switch (res_it->WordDirection()) {
      case DIR_LEFT_TO_RIGHT: hocr_str += " dir='ltr'"; break;
      case DIR_RIGHT_TO_LEFT: hocr_str += " dir='rtl'"; break;
      default:  // Do nothing.
        break;
    }
    hocr_str += ">";
    bool bold, italic, underlined, monospace, serif, smallcaps;
    int pointsize, font_id;
    // TODO(rays): Is hOCR interested in the font name?
    (void) res_it->WordFontAttributes(&bold, &italic, &underlined,
                                      &monospace, &serif, &smallcaps,
                                      &pointsize, &font_id);
    bool last_word_in_line = res_it->IsAtFinalElement(RIL_TEXTLINE, RIL_WORD);
    bool last_word_in_para = res_it->IsAtFinalElement(RIL_PARA, RIL_WORD);
    bool last_word_in_block = res_it->IsAtFinalElement(RIL_BLOCK, RIL_WORD);
    if (bold) hocr_str += "<strong>";
    if (italic) hocr_str += "<em>";
    do {
      const char *grapheme = res_it->GetUTF8Text(RIL_SYMBOL);
      if (grapheme && grapheme[0] != 0) {
        if (grapheme[1] == 0) {
          hocr_str += HOcrEscape(grapheme);
        } else {
          hocr_str += grapheme;
        }
      }
      delete []grapheme;
      res_it->Next(RIL_SYMBOL);
    } while (!res_it->Empty(RIL_BLOCK) && !res_it->IsAtBeginningOf(RIL_WORD));
    if (italic) hocr_str += "</em>";
    if (bold) hocr_str += "</strong>";
    hocr_str += "</span> ";
    wcnt++;
    // Close any ending block/paragraph/textline.
    if (last_word_in_line) {
      hocr_str += "\n     </span>";
      lcnt++;
    }
    if (last_word_in_para) {
      hocr_str += "\n    </p>\n";
      pcnt++;
    }
    if (last_word_in_block) {
      hocr_str += "   </div>\n";
      bcnt++;
    }
  }
  hocr_str += "  </div>\n";

  char *ret = new char[hocr_str.length() + 1];
  strcpy(ret, hocr_str.string());
  delete res_it;
  return ret;
}

/** The 5 numbers output for each box (the usual 4 and a page number.) */
const int kNumbersPerBlob = 5;
/**
 * The number of bytes taken by each number. Since we use inT16 for ICOORD,
 * assume only 5 digits max.
 */
const int kBytesPerNumber = 5;
/**
 * Multiplier for max expected textlength assumes (kBytesPerNumber + space)
 * * kNumbersPerBlob plus the newline. Add to this the
 * original UTF8 characters, and one kMaxBytesPerLine for safety.
 */
const int kBytesPerBlob = kNumbersPerBlob * (kBytesPerNumber + 1) + 1;
const int kBytesPerBoxFileLine = (kBytesPerNumber + 1) * kNumbersPerBlob + 1;
/** Max bytes in the decimal representation of inT64. */
const int kBytesPer64BitNumber = 20;
/**
 * A maximal single box could occupy kNumbersPerBlob numbers at
 * kBytesPer64BitNumber digits (if someone sneaks in a 64 bit value) and a
 * space plus the newline and the maximum length of a UNICHAR.
 * Test against this on each iteration for safety.
 */
const int kMaxBytesPerLine = kNumbersPerBlob * (kBytesPer64BitNumber + 1) + 1 +
    UNICHAR_LEN;

/**
 * The recognized text is returned as a char* which is coded
 * as a UTF8 box file and must be freed with the delete [] operator.
 * page_number is a 0-base page index that will appear in the box file.
 */
char* TessBaseAPI::GetBoxText(int page_number) {
  if (tesseract_ == NULL ||
      (!recognition_done_ && Recognize(NULL) < 0))
    return NULL;
  int blob_count;
  int utf8_length = TextLength(&blob_count);
  int total_length = blob_count * kBytesPerBoxFileLine + utf8_length +
      kMaxBytesPerLine;
  char* result = new char[total_length];
  strcpy(result, "\0");
  int output_length = 0;
  LTRResultIterator* it = GetLTRIterator();
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

/**
 * Conversion table for non-latin characters.
 * Maps characters out of the latin set into the latin set.
 * TODO(rays) incorporate this translation into unicharset.
 */
const int kUniChs[] = {
  0x20ac, 0x201c, 0x201d, 0x2018, 0x2019, 0x2022, 0x2014, 0
};
/** Latin chars corresponding to the unicode chars above. */
const int kLatinChs[] = {
  0x00a2, 0x0022, 0x0022, 0x0027, 0x0027, 0x00b7, 0x002d, 0
};

/**
 * The recognized text is returned as a char* which is coded
 * as UNLV format Latin-1 with specific reject and suspect codes
 * and must be freed with the delete [] operator.
 */
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

/** Returns the average word confidence for Tesseract page result. */
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

/** Returns an array of all word confidences, terminated by -1. */
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
  int debug = 0;
  GetIntVariable("applybox_debug", &debug);
  bool success = true;
  PageSegMode current_psm = GetPageSegMode();
  SetPageSegMode(mode);
  SetVariable("classify_enable_learning", "0");
  char* text = GetUTF8Text();
  if (debug) {
    tprintf("Trying to adapt \"%s\" to \"%s\"\n", text, wordstr);
  }
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
      GenericVector<TBOX> boxes;
      page_res_ = tesseract_->SetupApplyBoxes(boxes, block_list_);
      tesseract_->ReSegmentByClassification(page_res_);
      tesseract_->TidyUp(page_res_);
      PAGE_RES_IT pr_it(page_res_);
      if (pr_it.word() == NULL)
        success = false;
      else
        word_res = pr_it.word();
    } else {
      word_res->BestChoiceToCorrectText();
    }
    if (success) {
      tesseract_->EnableLearning = true;
      tesseract_->LearnWord(NULL, word_res);
    }
    delete [] text;
  } else {
    success = false;
  }
  SetPageSegMode(current_psm);
  return success;
}

/**
 * Free up recognition results and any stored image data, without actually
 * freeing any recognition data that would be time-consuming to reload.
 * Afterwards, you must call SetImage or TesseractRect before doing
 * any Recognize or Get* operation.
 */
void TessBaseAPI::Clear() {
  if (thresholder_ != NULL)
    thresholder_->Clear();
  ClearResults();
}

/**
 * Close down tesseract and free up all memory. End() is equivalent to
 * destructing and reconstructing your TessBaseAPI.
 * Once End() has been used, none of the other API functions may be used
 * other than Init and anything declared above it in the class definition.
 */
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
  if (paragraph_models_ != NULL) {
    paragraph_models_->delete_data_pointers();
    delete paragraph_models_;
    paragraph_models_ = NULL;
  }
  if (tesseract_ != NULL) {
    delete tesseract_;
    if (osd_tesseract_ == tesseract_)
      osd_tesseract_ = NULL;
    tesseract_ = NULL;
  }
  if (osd_tesseract_ != NULL) {
    delete osd_tesseract_;
    osd_tesseract_ = NULL;
  }
  if (equ_detect_ != NULL) {
    delete equ_detect_;
    equ_detect_ = NULL;
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

// Clear any library-level memory caches.
// There are a variety of expensive-to-load constant data structures (mostly
// language dictionaries) that are cached globally -- surviving the Init()
// and End() of individual TessBaseAPI's.  This function allows the clearing
// of these caches.
void TessBaseAPI::ClearPersistentCache() {
  Dict::GlobalDawgCache()->DeleteUnusedDawgs();
}

/**
 * Check whether a word is valid according to Tesseract's language model
 * returns 0 if the word is invalid, non-zero if valid
 */
int TessBaseAPI::IsValidWord(const char *word) {
  return tesseract_->getDict().valid_word(word);
}


// TODO(rays) Obsolete this function and replace with a more aptly named
// function that returns image coordinates rather than tesseract coordinates.
bool TessBaseAPI::GetTextDirection(int* out_offset, float* out_slope) {
  PageIterator* it = AnalyseLayout();
  if (it == NULL) {
    return false;
  }
  int x1, x2, y1, y2;
  it->Baseline(RIL_TEXTLINE, &x1, &y1, &x2, &y2);
  // Calculate offset and slope (NOTE: Kind of ugly)
  if (x2 <= x1) x2 = x1 + 1;
  // Convert the point pair to slope/offset of the baseline (in image coords.)
  *out_slope = static_cast<float>(y2 - y1) / (x2 - x1);
  *out_offset = static_cast<int>(y1 - *out_slope * x1);
  // Get the y-coord of the baseline at the left and right edges of the
  // textline's bounding box.
  int left, top, right, bottom;
  if (!it->BoundingBox(RIL_TEXTLINE, &left, &top, &right, &bottom)) {
    delete it;
    return false;
  }
  int left_y = IntCastRounded(*out_slope * left + *out_offset);
  int right_y = IntCastRounded(*out_slope * right + *out_offset);
  // Shift the baseline down so it passes through the nearest bottom-corner
  // of the textline's bounding box. This is the difference between the y
  // at the lowest (max) edge of the box and the actual box bottom.
  *out_offset += bottom - MAX(left_y, right_y);
  // Switch back to bottom-up tesseract coordinates. Requires negation of
  // the slope and height - offset for the offset.
  *out_slope = -*out_slope;
  *out_offset = rect_height_ - *out_offset;
  delete it;

  return true;
}

/** Sets Dict::letter_is_okay_ function to point to the given function. */
void TessBaseAPI::SetDictFunc(DictFunc f) {
  if (tesseract_ != NULL) {
    tesseract_->getDict().letter_is_okay_ = f;
  }
}

/**
 * Sets Dict::probability_in_context_ function to point to the given
 * function.
 */
void TessBaseAPI::SetProbabilityInContextFunc(ProbabilityInContextFunc f) {
  if (tesseract_ != NULL) {
    tesseract_->getDict().probability_in_context_ = f;
    // Set it for the sublangs too.
    int num_subs = tesseract_->num_sub_langs();
    for (int i = 0; i < num_subs; ++i) {
      tesseract_->get_sub_lang(i)->getDict().probability_in_context_ = f;
    }
  }
}

/** Sets Wordrec::fill_lattice_ function to point to the given function. */
void TessBaseAPI::SetFillLatticeFunc(FillLatticeFunc f) {
  if (tesseract_ != NULL) tesseract_->fill_lattice_ = f;
}

/** Common code for setting the image. */
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

/**
 * Run the thresholder to make the thresholded image, returned in pix,
 * which must not be NULL. *pix must be initialized to NULL, or point
 * to an existing pixDestroyable Pix.
 * The usual argument to Threshold is Tesseract::mutable_pix_binary().
 */
void TessBaseAPI::Threshold(Pix** pix) {
  ASSERT_HOST(pix != NULL);
  if (*pix != NULL)
    pixDestroy(pix);
  // Zero resolution messes up the algorithms, so make sure it is credible.
  int y_res = thresholder_->GetScaledYResolution();
  if (y_res < kMinCredibleResolution || y_res > kMaxCredibleResolution) {
    // Use the minimum default resolution, as it is safer to under-estimate
    // than over-estimate resolution.
    thresholder_->SetSourceYResolution(kMinCredibleResolution);
  }
  thresholder_->ThresholdToPix(pix);
  thresholder_->GetImageSizes(&rect_left_, &rect_top_,
                              &rect_width_, &rect_height_,
                              &image_width_, &image_height_);
  if (!thresholder_->IsBinary()) {
    tesseract_->set_pix_thresholds(thresholder_->GetPixRectThresholds());
    tesseract_->set_pix_grey(thresholder_->GetPixRectGrey());
  } else {
    tesseract_->set_pix_thresholds(NULL);
    tesseract_->set_pix_grey(NULL);
  }
  // Set the internal resolution that is used for layout parameters from the
  // estimated resolution, rather than the image resolution, which may be
  // fabricated, but we will use the image resolution, if there is one, to
  // report output point sizes.
  int estimated_res = ClipToRange(thresholder_->GetScaledEstimatedResolution(),
                                  kMinCredibleResolution,
                                  kMaxCredibleResolution);
  if (estimated_res != thresholder_->GetScaledEstimatedResolution()) {
    tprintf("Estimated resolution %d out of range! Corrected to %d\n",
            thresholder_->GetScaledEstimatedResolution(), estimated_res);
  }
  tesseract_->set_source_resolution(estimated_res);
  SavePixForCrash(estimated_res, *pix);
}

/** Find lines from the image making the BLOCK_LIST. */
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

  if (tesseract_->textord_equation_detect) {
    if (equ_detect_ == NULL && datapath_ != NULL) {
      equ_detect_ = new EquationDetect(datapath_->string(), NULL);
    }
    tesseract_->SetEquationDetect(equ_detect_);
  }

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
        osd_tesseract_->set_source_resolution(
            thresholder_->GetSourceYResolution());
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
  // If Devanagari is being recognized, we use different images for page seg
  // and for OCR.
  tesseract_->PrepareForTessOCR(block_list_, osd_tess, &osr);
  return 0;
}

/** Delete the pageres and clear the block list ready for a new page. */
void TessBaseAPI::ClearResults() {
  if (tesseract_ != NULL) {
    tesseract_->Clear();
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
  if (paragraph_models_ != NULL) {
    paragraph_models_->delete_data_pointers();
    delete paragraph_models_;
    paragraph_models_ = NULL;
  }
  SavePixForCrash(0, NULL);
}

/**
 * Return the length of the output text string, as UTF8, assuming
 * liberally two spacing marks after each word (as paragraphs end with two
 * newlines), and assuming a single character reject marker for each rejected
 * character.
 * Also return the number of recognized blobs in blob_count.
 */
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
      total_blobs += choice->length() + 2;
      total_length += choice->unichar_string().length() + 2;
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

/**
 * Estimates the Orientation And Script of the image.
 * Returns true if the image was processed successfully.
 */
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

/**
 * Return text orientation of each block as determined in an earlier page layout
 * analysis operation. Orientation is returned as the number of ccw 90-degree
 * rotations (in [0..3]) required to make the text in the block upright
 * (readable). Note that this may not necessary be the block orientation
 * preferred for recognition (such as the case of vertical CJK text).
 *
 * Also returns whether the text in the block is believed to have vertical
 * writing direction (when in an upright page orientation).
 *
 * The returned array is of length equal to the number of text blocks, which may
 * be less than the total number of blocks. The ordering is intended to be
 * consistent with GetTextLines().
 */
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

/** Find lines from the image making the BLOCK_LIST. */
BLOCK_LIST* TessBaseAPI::FindLinesCreateBlockList() {
  FindLines();
  BLOCK_LIST* result = block_list_;
  block_list_ = NULL;
  return result;
}

/**
 * Delete a block list.
 * This is to keep BLOCK_LIST pointer opaque
 * and let go of including the other headers.
 */
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

/** Creates a TBLOB* from the whole pix. */
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
  return TBLOB::PolygonalCopy(false, c_blob_it.data());
}

/**
 * This method baseline normalizes a TBLOB in-place. The input row is used
 * for normalization. The denorm is an optional parameter in which the
 * normalization-antidote is returned.
 */
void TessBaseAPI::NormalizeTBLOB(TBLOB *tblob, ROW *row, bool numeric_mode) {
  TBOX box = tblob->bounding_box();
  float x_center = (box.left() + box.right()) / 2.0f;
  float baseline = row->base_line(x_center);
  float scale = kBlnXHeight / row->x_height();
  tblob->Normalize(NULL, NULL, NULL, x_center, baseline, scale, scale,
                   0.0f, static_cast<float>(kBlnBaselineOffset), false, NULL);
}

/**
 * Return a TBLOB * from the whole pix.
 * To be freed later with delete.
 */
TBLOB *make_tesseract_blob(float baseline, float xheight,
                           float descender, float ascender,
                           bool numeric_mode, Pix* pix) {
  TBLOB *tblob = TessBaseAPI::MakeTBLOB(pix);

  // Normalize TBLOB
  ROW *row =
      TessBaseAPI::MakeTessOCRRow(baseline, xheight, descender, ascender);
  TessBaseAPI::NormalizeTBLOB(tblob, row, numeric_mode);
  delete row;
  return tblob;
}

/**
 * Adapt to recognize the current image as the given character.
 * The image must be preloaded into pix_binary_ and be just an image
 * of a single character.
 */
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
  float best_rating = -100;


  // Classify to get a raw choice.
  BLOB_CHOICE_LIST choices;
  tesseract_->AdaptiveClassifier(blob, &choices);
  BLOB_CHOICE_IT choice_it;
  choice_it.set_to_list(&choices);
  for (choice_it.mark_cycle_pt(); !choice_it.cycled_list();
       choice_it.forward()) {
    if (choice_it.data()->rating() > best_rating) {
      best_rating = choice_it.data()->rating();
    }
  }

  threshold = tesseract_->matcher_good_threshold;

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

void TessBaseAPI::DetectParagraphs(bool after_text_recognition) {
  int debug_level = 0;
  GetIntVariable("paragraph_debug_level", &debug_level);
  if (paragraph_models_ == NULL)
    paragraph_models_ = new GenericVector<ParagraphModel*>;
  MutableIterator *result_it = GetMutableIterator();
  do {  // Detect paragraphs for this block
    GenericVector<ParagraphModel *> models;
    ::tesseract::DetectParagraphs(debug_level, after_text_recognition,
                                  result_it, &models);
    *paragraph_models_ += models;
  } while (result_it->Next(RIL_BLOCK));
  delete result_it;
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

/**
 * Extract the OCR results, costs (penalty points for uncertainty),
 * and the bounding boxes of the characters.
 */
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

/**
 * Extract the OCR results, costs (penalty points for uncertainty),
 * and the bounding boxes of the characters.
 */
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

/** This method returns the features associated with the input blob. */
// The resulting features are returned in int_features, which must be
// of size MAX_NUM_INT_FEATURES. The number of features is returned in
// num_features (or 0 if there was a failure).
// On return feature_outline_index is filled with an index of the outline
// corresponding to each feature in int_features.
// TODO(rays) Fix the caller to out outline_counts instead.
void TessBaseAPI::GetFeaturesForBlob(TBLOB* blob,
                                     INT_FEATURE_STRUCT* int_features,
                                     int* num_features,
                                     int* feature_outline_index) {
  GenericVector<int> outline_counts;
  GenericVector<INT_FEATURE_STRUCT> bl_features;
  GenericVector<INT_FEATURE_STRUCT> cn_features;
  INT_FX_RESULT_STRUCT fx_info;
  tesseract_->ExtractFeatures(*blob, false, &bl_features,
                              &cn_features, &fx_info, &outline_counts);
  if (cn_features.size() == 0 || cn_features.size() > MAX_NUM_INT_FEATURES) {
    *num_features = 0;
    return;  // Feature extraction failed.
  }
  *num_features = cn_features.size();
  memcpy(int_features, &cn_features[0], *num_features * sizeof(cn_features[0]));
  // TODO(rays) Pass outline_counts back and simplify the calling code.
  if (feature_outline_index != NULL) {
    int f = 0;
    for (int i = 0; i < outline_counts.size(); ++i) {
      while (f < outline_counts[i])
        feature_outline_index[f++] = i;
    }
  }
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

/** Method to run adaptive classifier on a blob. */
void TessBaseAPI::RunAdaptiveClassifier(TBLOB* blob,
                                        int num_max_matches,
                                        int* unichar_ids,
                                        float* ratings,
                                        int* num_matches_returned) {
  BLOB_CHOICE_LIST* choices = new BLOB_CHOICE_LIST;
  tesseract_->AdaptiveClassifier(blob, choices);
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

/** This method returns the string form of the specified unichar. */
const char* TessBaseAPI::GetUnichar(int unichar_id) {
  return tesseract_->unicharset.id_to_unichar(unichar_id);
}

/** Return the pointer to the i-th dawg loaded into tesseract_ object. */
const Dawg *TessBaseAPI::GetDawg(int i) const {
  if (tesseract_ == NULL || i >= NumDawgs()) return NULL;
  return tesseract_->getDict().GetDawg(i);
}

/** Return the number of dawgs loaded into tesseract_ object. */
int TessBaseAPI::NumDawgs() const {
  return tesseract_ == NULL ? 0 : tesseract_->getDict().NumDawgs();
}

/** Return a pointer to underlying CubeRecoContext object if present. */
CubeRecoContext *TessBaseAPI::GetCubeRecoContext() const {
  return (tesseract_ == NULL) ? NULL : tesseract_->GetCubeRecoContext();
}

TessResultRenderer* TessBaseAPI::NewRenderer() {
  if (tesseract_->tessedit_create_boxfile
      || tesseract_->tessedit_make_boxes_from_boxes) {
    return new TessBoxTextRenderer();
  } else if (tesseract_->tessedit_create_hocr) {
    return new TessHOcrRenderer();
  } else if (tesseract_->tessedit_create_pdf) {
    return new TessPDFRenderer(tesseract_->datadir.c_str());
  } else if (tesseract_->tessedit_write_unlv) {
    return new TessUnlvRenderer();
  } else if (tesseract_->tessedit_create_boxfile) {
    return new TessBoxTextRenderer();
  } else {
    return new TessTextRenderer();
  }
}

/** Escape a char string - remove <>&"' with HTML codes. */
const char* HOcrEscape(const char* text) {
  const char *ptr;
  STRING ret;
  for (ptr = text; *ptr; ptr++) {
    switch (*ptr) {
      case '<': ret += "&lt;"; break;
      case '>': ret += "&gt;"; break;
      case '&': ret += "&amp;"; break;
      case '"': ret += "&quot;"; break;
      case '\'': ret += "&#39;"; break;
      default: ret += *ptr;
    }
  }
  return ret.string();
}
}  // namespace tesseract.
