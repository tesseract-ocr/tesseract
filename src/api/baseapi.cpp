/**********************************************************************
 * File:        baseapi.cpp
 * Description: Simple API for calling tesseract.
 * Author:      Ray Smith
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

#define _USE_MATH_DEFINES // for M_PI

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "boxword.h"    // for BoxWord
#include "coutln.h"     // for C_OUTLINE_IT, C_OUTLINE_LIST
#include "dawg_cache.h" // for DawgCache
#include "dict.h"       // for Dict
#include "elst.h"       // for ELIST_ITERATOR, ELISTIZE, ELISTIZEH
#include "environ.h"    // for l_uint8
#ifndef DISABLED_LEGACY_ENGINE
#include "equationdetect.h" // for EquationDetect, destructor of equ_detect_
#endif // ndef DISABLED_LEGACY_ENGINE
#include "errcode.h" // for ASSERT_HOST
#include "helpers.h" // for IntCastRounded, chomp_string
#include "host.h"    // for MAX_PATH
#include "imageio.h" // for IFF_TIFF_G4, IFF_TIFF, IFF_TIFF_G3, ...
#ifndef DISABLED_LEGACY_ENGINE
#  include "intfx.h" // for INT_FX_RESULT_STRUCT
#endif
#include "mutableiterator.h" // for MutableIterator
#include "normalis.h"        // for kBlnBaselineOffset, kBlnXHeight
#if defined(USE_OPENCL)
#  include "openclwrapper.h" // for OpenclDevice
#endif
#include "pageres.h"         // for PAGE_RES_IT, WERD_RES, PAGE_RES, CR_DE...
#include "paragraphs.h"      // for DetectParagraphs
#include "params.h"          // for BoolParam, IntParam, DoubleParam, Stri...
#include "pdblock.h"         // for PDBLK
#include "points.h"          // for FCOORD
#include "polyblk.h"         // for POLY_BLOCK
#include "rect.h"            // for TBOX
#include "stepblob.h"        // for C_BLOB_IT, C_BLOB, C_BLOB_LIST
#include "tessdatamanager.h" // for TessdataManager, kTrainedDataSuffix
#include "tesseractclass.h"  // for Tesseract
#include "tprintf.h"         // for tprintf
#include "werd.h"            // for WERD, WERD_IT, W_FUZZY_NON, W_FUZZY_SP
#include "thresholder.h"     // for ImageThresholder

#include <tesseract/baseapi.h>
#include <tesseract/ocrclass.h>       // for ETEXT_DESC
#include <tesseract/osdetect.h>       // for OSResults, OSBestResult, OrientationId...
#include <tesseract/renderer.h>       // for TessResultRenderer
#include <tesseract/resultiterator.h> // for ResultIterator

#include <cmath>    // for round, M_PI
#include <cstdint>  // for int32_t
#include <cstring>  // for strcmp, strcpy
#include <fstream>  // for size_t
#include <iostream> // for std::cin
#include <locale>   // for std::locale::classic
#include <memory>   // for std::unique_ptr
#include <set>      // for std::pair
#include <sstream>  // for std::stringstream
#include <vector>   // for std::vector

#include <allheaders.h> // for pixDestroy, boxCreate, boxaAddBox, box...
#ifdef HAVE_LIBCURL
#  include <curl/curl.h>
#endif

#ifdef __linux__
#  include <csignal> // for sigaction, SA_RESETHAND, SIGBUS, SIGFPE
#endif

#if defined(_WIN32)
#  include <fcntl.h>
#  include <io.h>
#else
#  include <dirent.h> // for closedir, opendir, readdir, DIR, dirent
#  include <libgen.h>
#  include <sys/stat.h> // for stat, S_IFDIR
#  include <sys/types.h>
#  include <unistd.h>
#endif // _WIN32

namespace tesseract {

static BOOL_VAR(stream_filelist, false, "Stream a filelist from stdin");
static STRING_VAR(document_title, "", "Title of output document (used for hOCR and PDF output)");
#ifdef HAVE_LIBCURL
static INT_VAR(curl_timeout, 0, "Timeout for curl in seconds");
#endif

/** Minimum sensible image size to be worth running tesseract. */
const int kMinRectSize = 10;
/** Character returned when Tesseract couldn't recognize as anything. */
const char kTesseractReject = '~';
/** Character used by UNLV error counter as a reject. */
const char kUNLVReject = '~';
/** Character used by UNLV as a suspect marker. */
const char kUNLVSuspect = '^';
/**
 * Temp file used for storing current parameters before applying retry values.
 */
static const char *kOldVarsFile = "failed_vars.txt";

#ifndef DISABLED_LEGACY_ENGINE
/**
 * Filename used for input image file, from which to derive a name to search
 * for a possible UNLV zone file, if none is specified by SetInputName.
 */
static const char *kInputFile = "noname.tif";
static const char kUnknownFontName[] = "UnknownFont";

static STRING_VAR(classify_font_name, kUnknownFontName,
                  "Default font name to be used in training");

// Finds the name of the training font and returns it in fontname, by cutting
// it out based on the expectation that the filename is of the form:
// /path/to/dir/[lang].[fontname].exp[num]
// The [lang], [fontname] and [num] fields should not have '.' characters.
// If the global parameter classify_font_name is set, its value is used instead.
static void ExtractFontName(const char* filename, std::string* fontname) {
  *fontname = classify_font_name;
  if (*fontname == kUnknownFontName) {
    // filename is expected to be of the form [lang].[fontname].exp[num]
    // The [lang], [fontname] and [num] fields should not have '.' characters.
    const char *basename = strrchr(filename, '/');
    const char *firstdot = strchr(basename ? basename : filename, '.');
    const char *lastdot  = strrchr(filename, '.');
    if (firstdot != lastdot && firstdot != nullptr && lastdot != nullptr) {
      ++firstdot;
      *fontname = firstdot;
      fontname->resize(lastdot - firstdot);
    }
  }
}
#endif

/* Add all available languages recursively.
 */
static void addAvailableLanguages(const std::string &datadir, const std::string &base,
                                  std::vector<std::string> *langs) {
  auto base2 = base;
  if (!base2.empty()) {
    base2 += "/";
  }
  const size_t extlen = sizeof(kTrainedDataSuffix);
#ifdef _WIN32
  WIN32_FIND_DATA data;
  HANDLE handle = FindFirstFile((datadir + base2 + "*").c_str(), &data);
  if (handle != INVALID_HANDLE_VALUE) {
    BOOL result = TRUE;
    for (; result;) {
      char *name = data.cFileName;
      // Skip '.', '..', and hidden files
      if (name[0] != '.') {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
          addAvailableLanguages(datadir, base2 + name, langs);
        } else {
          size_t len = strlen(name);
          if (len > extlen && name[len - extlen] == '.' &&
              strcmp(&name[len - extlen + 1], kTrainedDataSuffix) == 0) {
            name[len - extlen] = '\0';
            langs->push_back(base2 + name);
          }
        }
      }
      result = FindNextFile(handle, &data);
    }
    FindClose(handle);
  }
#else // _WIN32
  DIR *dir = opendir((datadir + base).c_str());
  if (dir != nullptr) {
    dirent *de;
    while ((de = readdir(dir))) {
      char *name = de->d_name;
      // Skip '.', '..', and hidden files
      if (name[0] != '.') {
        struct stat st;
        if (stat((datadir + base2 + name).c_str(), &st) == 0 && (st.st_mode & S_IFDIR) == S_IFDIR) {
          addAvailableLanguages(datadir, base2 + name, langs);
        } else {
          size_t len = strlen(name);
          if (len > extlen && name[len - extlen] == '.' &&
              strcmp(&name[len - extlen + 1], kTrainedDataSuffix) == 0) {
            name[len - extlen] = '\0';
            langs->push_back(base2 + name);
          }
        }
      }
    }
    closedir(dir);
  }
#endif
}

TessBaseAPI::TessBaseAPI()
    : tesseract_(nullptr)
    , osd_tesseract_(nullptr)
    , equ_detect_(nullptr)
    , reader_(nullptr)
    ,
    // thresholder_ is initialized to nullptr here, but will be set before use
    // by: A constructor of a derived API or created
    // implicitly when used in InternalSetImage.
    thresholder_(nullptr)
    , paragraph_models_(nullptr)
    , block_list_(nullptr)
    , page_res_(nullptr)
    , last_oem_requested_(OEM_DEFAULT)
    , recognition_done_(false)
    , rect_left_(0)
    , rect_top_(0)
    , rect_width_(0)
    , rect_height_(0)
    , image_width_(0)
    , image_height_(0) {
}

TessBaseAPI::~TessBaseAPI() {
  End();
}

/**
 * Returns the version identifier as a static string. Do not delete.
 */
const char *TessBaseAPI::Version() {
  return TESSERACT_VERSION_STR;
}

/**
 * If compiled with OpenCL AND an available OpenCL
 * device is deemed faster than serial code, then
 * "device" is populated with the cl_device_id
 * and returns sizeof(cl_device_id)
 * otherwise *device=nullptr and returns 0.
 */
size_t TessBaseAPI::getOpenCLDevice(void **data) {
#ifdef USE_OPENCL
  ds_device device = OpenclDevice::getDeviceSelection();
  if (device.type == DS_DEVICE_OPENCL_DEVICE) {
    *data = new cl_device_id;
    memcpy(*data, &device.oclDeviceID, sizeof(cl_device_id));
    return sizeof(cl_device_id);
  }
#endif

  *data = nullptr;
  return 0;
}

/**
 * Set the name of the input file. Needed only for training and
 * loading a UNLV zone file.
 */
void TessBaseAPI::SetInputName(const char *name) {
  input_file_ = name ? name : "";
}

/** Set the name of the output files. Needed only for debugging. */
void TessBaseAPI::SetOutputName(const char *name) {
  output_file_ = name ? name : "";
}

bool TessBaseAPI::SetVariable(const char *name, const char *value) {
  if (tesseract_ == nullptr) {
    tesseract_ = new Tesseract;
  }
  return ParamUtils::SetParam(name, value, SET_PARAM_CONSTRAINT_NON_INIT_ONLY,
                              tesseract_->params());
}

bool TessBaseAPI::SetDebugVariable(const char *name, const char *value) {
  if (tesseract_ == nullptr) {
    tesseract_ = new Tesseract;
  }
  return ParamUtils::SetParam(name, value, SET_PARAM_CONSTRAINT_DEBUG_ONLY, tesseract_->params());
}

bool TessBaseAPI::GetIntVariable(const char *name, int *value) const {
  auto *p = ParamUtils::FindParam<IntParam>(name, GlobalParams()->int_params,
                                            tesseract_->params()->int_params);
  if (p == nullptr) {
    return false;
  }
  *value = (int32_t)(*p);
  return true;
}

bool TessBaseAPI::GetBoolVariable(const char *name, bool *value) const {
  auto *p = ParamUtils::FindParam<BoolParam>(name, GlobalParams()->bool_params,
                                             tesseract_->params()->bool_params);
  if (p == nullptr) {
    return false;
  }
  *value = bool(*p);
  return true;
}

const char *TessBaseAPI::GetStringVariable(const char *name) const {
  auto *p = ParamUtils::FindParam<StringParam>(name, GlobalParams()->string_params,
                                               tesseract_->params()->string_params);
  return (p != nullptr) ? p->c_str() : nullptr;
}

bool TessBaseAPI::GetDoubleVariable(const char *name, double *value) const {
  auto *p = ParamUtils::FindParam<DoubleParam>(name, GlobalParams()->double_params,
                                               tesseract_->params()->double_params);
  if (p == nullptr) {
    return false;
  }
  *value = (double)(*p);
  return true;
}

/** Get value of named variable as a string, if it exists. */
bool TessBaseAPI::GetVariableAsString(const char *name, std::string *val) const {
  return ParamUtils::GetParamAsString(name, tesseract_->params(), val);
}

#ifndef DISABLED_LEGACY_ENGINE

/** Print Tesseract fonts table to the given file. */
void TessBaseAPI::PrintFontsTable(FILE *fp) const {
  const int fontinfo_size = tesseract_->get_fontinfo_table().size();
  for (int font_index = 1; font_index < fontinfo_size; ++font_index) {
    FontInfo font = tesseract_->get_fontinfo_table().at(font_index);
    fprintf(fp, "ID=%3d: %s is_italic=%s is_bold=%s"
                " is_fixed_pitch=%s is_serif=%s is_fraktur=%s\n",
                font_index, font.name,
                font.is_italic() ? "true" : "false",
                font.is_bold() ? "true" : "false",
                font.is_fixed_pitch() ? "true" : "false",
                font.is_serif() ? "true" : "false",
                font.is_fraktur() ? "true" : "false");
  }
}

#endif

/** Print Tesseract parameters to the given file. */
void TessBaseAPI::PrintVariables(FILE *fp) const {
  ParamUtils::PrintParams(fp, tesseract_->params());
}

/**
 * The datapath must be the name of the data directory or
 * some other file in which the data directory resides (for instance argv[0].)
 * The language is (usually) an ISO 639-3 string or nullptr will default to eng.
 * If numeric_mode is true, then only digits and Roman numerals will
 * be returned.
 * @return: 0 on success and -1 on initialization failure.
 */
int TessBaseAPI::Init(const char *datapath, const char *language, OcrEngineMode oem, char **configs,
                      int configs_size, const std::vector<std::string> *vars_vec,
                      const std::vector<std::string> *vars_values, bool set_only_non_debug_params) {
  return Init(datapath, 0, language, oem, configs, configs_size, vars_vec, vars_values,
              set_only_non_debug_params, nullptr);
}

// In-memory version reads the traineddata file directly from the given
// data[data_size] array. Also implements the version with a datapath in data,
// flagged by data_size = 0.
int TessBaseAPI::Init(const char *data, int data_size, const char *language, OcrEngineMode oem,
                      char **configs, int configs_size, const std::vector<std::string> *vars_vec,
                      const std::vector<std::string> *vars_values, bool set_only_non_debug_params,
                      FileReader reader) {
  if (language == nullptr) {
    language = "";
  }
  if (data == nullptr) {
    data = "";
  }
  std::string datapath = data_size == 0 ? data : language;
  // If the datapath, OcrEngineMode or the language have changed - start again.
  // Note that the language_ field stores the last requested language that was
  // initialized successfully, while tesseract_->lang stores the language
  // actually used. They differ only if the requested language was nullptr, in
  // which case tesseract_->lang is set to the Tesseract default ("eng").
  if (tesseract_ != nullptr &&
      (datapath_.empty() || language_.empty() || datapath_ != datapath ||
       last_oem_requested_ != oem || (language_ != language && tesseract_->lang != language))) {
    delete tesseract_;
    tesseract_ = nullptr;
  }
#ifdef USE_OPENCL
  OpenclDevice od;
  od.InitEnv();
#endif
  bool reset_classifier = true;
  if (tesseract_ == nullptr) {
    reset_classifier = false;
    tesseract_ = new Tesseract;
    if (reader != nullptr) {
      reader_ = reader;
    }
    TessdataManager mgr(reader_);
    if (data_size != 0) {
      mgr.LoadMemBuffer(language, data, data_size);
    }
    if (tesseract_->init_tesseract(datapath, output_file_, language, oem, configs,
                                   configs_size, vars_vec, vars_values, set_only_non_debug_params,
                                   &mgr) != 0) {
      return -1;
    }
  }

  // Update datapath and language requested for the last valid initialization.
  datapath_ = datapath;
  if (datapath_.empty() && !tesseract_->datadir.empty()) {
    datapath_ = tesseract_->datadir;
  }

  language_ = language;
  last_oem_requested_ = oem;

#ifndef DISABLED_LEGACY_ENGINE
  // For same language and datapath, just reset the adaptive classifier.
  if (reset_classifier) {
    tesseract_->ResetAdaptiveClassifier();
  }
#endif // ndef DISABLED_LEGACY_ENGINE
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
const char *TessBaseAPI::GetInitLanguagesAsString() const {
  return language_.c_str();
}

/**
 * Returns the loaded languages in the vector of std::string.
 * Includes all languages loaded by the last Init, including those loaded
 * as dependencies of other loaded languages.
 */
void TessBaseAPI::GetLoadedLanguagesAsVector(std::vector<std::string> *langs) const {
  langs->clear();
  if (tesseract_ != nullptr) {
    langs->push_back(tesseract_->lang);
    int num_subs = tesseract_->num_sub_langs();
    for (int i = 0; i < num_subs; ++i) {
      langs->push_back(tesseract_->get_sub_lang(i)->lang);
    }
  }
}

/**
 * Returns the available languages in the sorted vector of std::string.
 */
void TessBaseAPI::GetAvailableLanguagesAsVector(std::vector<std::string> *langs) const {
  langs->clear();
  if (tesseract_ != nullptr) {
    addAvailableLanguages(tesseract_->datadir, "", langs);
    std::sort(langs->begin(), langs->end());
  }
}

/**
 * Init only for page layout analysis. Use only for calls to SetImage and
 * AnalysePage. Calls that attempt recognition will generate an error.
 */
void TessBaseAPI::InitForAnalysePage() {
  if (tesseract_ == nullptr) {
    tesseract_ = new Tesseract;
#ifndef DISABLED_LEGACY_ENGINE
    tesseract_->InitAdaptiveClassifier(nullptr);
#endif
  }
}

/**
 * Read a "config" file containing a set of parameter name, value pairs.
 * Searches the standard places: tessdata/configs, tessdata/tessconfigs
 * and also accepts a relative or absolute path name.
 */
void TessBaseAPI::ReadConfigFile(const char *filename) {
  tesseract_->read_config_file(filename, SET_PARAM_CONSTRAINT_NON_INIT_ONLY);
}

/** Same as above, but only set debug params from the given config file. */
void TessBaseAPI::ReadDebugConfigFile(const char *filename) {
  tesseract_->read_config_file(filename, SET_PARAM_CONSTRAINT_DEBUG_ONLY);
}

/**
 * Set the current page segmentation mode. Defaults to PSM_AUTO.
 * The mode is stored as an IntParam so it can also be modified by
 * ReadConfigFile or SetVariable("tessedit_pageseg_mode", mode as string).
 */
void TessBaseAPI::SetPageSegMode(PageSegMode mode) {
  if (tesseract_ == nullptr) {
    tesseract_ = new Tesseract;
  }
  tesseract_->tessedit_pageseg_mode.set_value(mode);
}

/** Return the current page segmentation mode. */
PageSegMode TessBaseAPI::GetPageSegMode() const {
  if (tesseract_ == nullptr) {
    return PSM_SINGLE_BLOCK;
  }
  return static_cast<PageSegMode>(static_cast<int>(tesseract_->tessedit_pageseg_mode));
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
char *TessBaseAPI::TesseractRect(const unsigned char *imagedata, int bytes_per_pixel,
                                 int bytes_per_line, int left, int top, int width, int height) {
  if (tesseract_ == nullptr || width < kMinRectSize || height < kMinRectSize) {
    return nullptr; // Nothing worth doing.
  }

  // Since this original api didn't give the exact size of the image,
  // we have to invent a reasonable value.
  int bits_per_pixel = bytes_per_pixel == 0 ? 1 : bytes_per_pixel * 8;
  SetImage(imagedata, bytes_per_line * 8 / bits_per_pixel, height + top, bytes_per_pixel,
           bytes_per_line);
  SetRectangle(left, top, width, height);

  return GetUTF8Text();
}

#ifndef DISABLED_LEGACY_ENGINE
/**
 * Call between pages or documents etc to free up memory and forget
 * adaptive data.
 */
void TessBaseAPI::ClearAdaptiveClassifier() {
  if (tesseract_ == nullptr) {
    return;
  }
  tesseract_->ResetAdaptiveClassifier();
  tesseract_->ResetDocumentDictionary();
}
#endif // ndef DISABLED_LEGACY_ENGINE

/**
 * Provide an image for Tesseract to recognize. Format is as
 * TesseractRect above. Copies the image buffer and converts to Pix.
 * SetImage clears all recognition results, and sets the rectangle to the
 * full image, so it may be followed immediately by a GetUTF8Text, and it
 * will automatically perform recognition.
 */
void TessBaseAPI::SetImage(const unsigned char *imagedata, int width, int height,
                           int bytes_per_pixel, int bytes_per_line) {
  if (InternalSetImage()) {
    thresholder_->SetImage(imagedata, width, height, bytes_per_pixel, bytes_per_line);
    SetInputImage(thresholder_->GetPixRect());
  }
}

void TessBaseAPI::SetSourceResolution(int ppi) {
  if (thresholder_) {
    thresholder_->SetSourceYResolution(ppi);
  } else {
    tprintf("Please call SetImage before SetSourceResolution.\n");
  }
}

/**
 * Provide an image for Tesseract to recognize. As with SetImage above,
 * Tesseract takes its own copy of the image, so it need not persist until
 * after Recognize.
 * Pix vs raw, which to use?
 * Use Pix where possible. Tesseract uses Pix as its internal representation
 * and it is therefore more efficient to provide a Pix directly.
 */
void TessBaseAPI::SetImage(Pix *pix) {
  if (InternalSetImage()) {
    if (pixGetSpp(pix) == 4 && pixGetInputFormat(pix) == IFF_PNG) {
      // remove alpha channel from png
      Pix *p1 = pixRemoveAlpha(pix);
      pixSetSpp(p1, 3);
      (void)pixCopy(pix, p1);
      pixDestroy(&p1);
    }
    thresholder_->SetImage(pix);
    SetInputImage(thresholder_->GetPixRect());
  }
}

/**
 * Restrict recognition to a sub-rectangle of the image. Call after SetImage.
 * Each SetRectangle clears the recogntion results so multiple rectangles
 * can be recognized with the same image.
 */
void TessBaseAPI::SetRectangle(int left, int top, int width, int height) {
  if (thresholder_ == nullptr) {
    return;
  }
  thresholder_->SetRectangle(left, top, width, height);
  ClearResults();
}

/**
 * ONLY available after SetImage if you have Leptonica installed.
 * Get a copy of the internal thresholded image from Tesseract.
 */
Pix *TessBaseAPI::GetThresholdedImage() {
  if (tesseract_ == nullptr || thresholder_ == nullptr) {
    return nullptr;
  }
  if (tesseract_->pix_binary() == nullptr && !Threshold(&tesseract_->mutable_pix_binary()->pix_)) {
    return nullptr;
  }
  return tesseract_->pix_binary().clone();
}

/**
 * Get the result of page layout analysis as a leptonica-style
 * Boxa, Pixa pair, in reading order.
 * Can be called before or after Recognize.
 */
Boxa *TessBaseAPI::GetRegions(Pixa **pixa) {
  return GetComponentImages(RIL_BLOCK, false, pixa, nullptr);
}

/**
 * Get the textlines as a leptonica-style Boxa, Pixa pair, in reading order.
 * Can be called before or after Recognize.
 * If blockids is not nullptr, the block-id of each line is also returned as an
 * array of one element per line. delete [] after use.
 * If paraids is not nullptr, the paragraph-id of each line within its block is
 * also returned as an array of one element per line. delete [] after use.
 */
Boxa *TessBaseAPI::GetTextlines(const bool raw_image, const int raw_padding, Pixa **pixa,
                                int **blockids, int **paraids) {
  return GetComponentImages(RIL_TEXTLINE, true, raw_image, raw_padding, pixa, blockids, paraids);
}

/**
 * Get textlines and strips of image regions as a leptonica-style Boxa, Pixa
 * pair, in reading order. Enables downstream handling of non-rectangular
 * regions.
 * Can be called before or after Recognize.
 * If blockids is not nullptr, the block-id of each line is also returned as an
 * array of one element per line. delete [] after use.
 */
Boxa *TessBaseAPI::GetStrips(Pixa **pixa, int **blockids) {
  return GetComponentImages(RIL_TEXTLINE, false, pixa, blockids);
}

/**
 * Get the words as a leptonica-style
 * Boxa, Pixa pair, in reading order.
 * Can be called before or after Recognize.
 */
Boxa *TessBaseAPI::GetWords(Pixa **pixa) {
  return GetComponentImages(RIL_WORD, true, pixa, nullptr);
}

/**
 * Gets the individual connected (text) components (created
 * after pages segmentation step, but before recognition)
 * as a leptonica-style Boxa, Pixa pair, in reading order.
 * Can be called before or after Recognize.
 */
Boxa *TessBaseAPI::GetConnectedComponents(Pixa **pixa) {
  return GetComponentImages(RIL_SYMBOL, true, pixa, nullptr);
}

/**
 * Get the given level kind of components (block, textline, word etc.) as a
 * leptonica-style Boxa, Pixa pair, in reading order.
 * Can be called before or after Recognize.
 * If blockids is not nullptr, the block-id of each component is also returned
 * as an array of one element per component. delete [] after use.
 * If text_only is true, then only text components are returned.
 */
Boxa *TessBaseAPI::GetComponentImages(PageIteratorLevel level, bool text_only, bool raw_image,
                                      const int raw_padding, Pixa **pixa, int **blockids,
                                      int **paraids) {
  /*non-const*/ std::unique_ptr</*non-const*/ PageIterator> page_it(GetIterator());
  if (page_it == nullptr) {
    page_it.reset(AnalyseLayout());
  }
  if (page_it == nullptr) {
    return nullptr; // Failed.
  }

  // Count the components to get a size for the arrays.
  int component_count = 0;
  int left, top, right, bottom;

  if (raw_image) {
    // Get bounding box in original raw image with padding.
    do {
      if (page_it->BoundingBox(level, raw_padding, &left, &top, &right, &bottom) &&
          (!text_only || PTIsTextType(page_it->BlockType()))) {
        ++component_count;
      }
    } while (page_it->Next(level));
  } else {
    // Get bounding box from binarized imaged. Note that this could be
    // differently scaled from the original image.
    do {
      if (page_it->BoundingBoxInternal(level, &left, &top, &right, &bottom) &&
          (!text_only || PTIsTextType(page_it->BlockType()))) {
        ++component_count;
      }
    } while (page_it->Next(level));
  }

  Boxa *boxa = boxaCreate(component_count);
  if (pixa != nullptr) {
    *pixa = pixaCreate(component_count);
  }
  if (blockids != nullptr) {
    *blockids = new int[component_count];
  }
  if (paraids != nullptr) {
    *paraids = new int[component_count];
  }

  int blockid = 0;
  int paraid = 0;
  int component_index = 0;
  page_it->Begin();
  do {
    bool got_bounding_box;
    if (raw_image) {
      got_bounding_box = page_it->BoundingBox(level, raw_padding, &left, &top, &right, &bottom);
    } else {
      got_bounding_box = page_it->BoundingBoxInternal(level, &left, &top, &right, &bottom);
    }
    if (got_bounding_box && (!text_only || PTIsTextType(page_it->BlockType()))) {
      Box *lbox = boxCreate(left, top, right - left, bottom - top);
      boxaAddBox(boxa, lbox, L_INSERT);
      if (pixa != nullptr) {
        Pix *pix = nullptr;
        if (raw_image) {
          pix = page_it->GetImage(level, raw_padding, GetInputImage(), &left, &top);
        } else {
          pix = page_it->GetBinaryImage(level);
        }
        pixaAddPix(*pixa, pix, L_INSERT);
        pixaAddBox(*pixa, lbox, L_CLONE);
      }
      if (paraids != nullptr) {
        (*paraids)[component_index] = paraid;
        if (page_it->IsAtFinalElement(RIL_PARA, level)) {
          ++paraid;
        }
      }
      if (blockids != nullptr) {
        (*blockids)[component_index] = blockid;
        if (page_it->IsAtFinalElement(RIL_BLOCK, level)) {
          ++blockid;
          paraid = 0;
        }
      }
      ++component_index;
    }
  } while (page_it->Next(level));
  return boxa;
}

int TessBaseAPI::GetThresholdedImageScaleFactor() const {
  if (thresholder_ == nullptr) {
    return 0;
  }
  return thresholder_->GetScaleFactor();
}

/**
 * Runs page layout analysis in the mode set by SetPageSegMode.
 * May optionally be called prior to Recognize to get access to just
 * the page layout results. Returns an iterator to the results.
 * If merge_similar_words is true, words are combined where suitable for use
 * with a line recognizer. Use if you want to use AnalyseLayout to find the
 * textlines, and then want to process textline fragments with an external
 * line recognizer.
 * Returns nullptr on error or an empty page.
 * The returned iterator must be deleted after use.
 * WARNING! This class points to data held within the TessBaseAPI class, and
 * therefore can only be used while the TessBaseAPI class still exists and
 * has not been subjected to a call of Init, SetImage, Recognize, Clear, End
 * DetectOS, or anything else that changes the internal PAGE_RES.
 */
PageIterator *TessBaseAPI::AnalyseLayout() {
  return AnalyseLayout(false);
}

PageIterator *TessBaseAPI::AnalyseLayout(bool merge_similar_words) {
  if (FindLines() == 0) {
    if (block_list_->empty()) {
      return nullptr; // The page was empty.
    }
    page_res_ = new PAGE_RES(merge_similar_words, block_list_, nullptr);
    DetectParagraphs(false);
    return new PageIterator(page_res_, tesseract_, thresholder_->GetScaleFactor(),
                            thresholder_->GetScaledYResolution(), rect_left_, rect_top_,
                            rect_width_, rect_height_);
  }
  return nullptr;
}

/**
 * Recognize the tesseract global image and return the result as Tesseract
 * internal structures.
 */
int TessBaseAPI::Recognize(ETEXT_DESC *monitor) {
  if (tesseract_ == nullptr) {
    return -1;
  }
  if (FindLines() != 0) {
    return -1;
  }
  delete page_res_;
  if (block_list_->empty()) {
    page_res_ = new PAGE_RES(false, block_list_, &tesseract_->prev_word_best_choice_);
    return 0; // Empty page.
  }

  tesseract_->SetBlackAndWhitelist();
  recognition_done_ = true;
#ifndef DISABLED_LEGACY_ENGINE
  if (tesseract_->tessedit_resegment_from_line_boxes) {
    page_res_ = tesseract_->ApplyBoxes(input_file_.c_str(), true, block_list_);
  } else if (tesseract_->tessedit_resegment_from_boxes) {
    page_res_ = tesseract_->ApplyBoxes(input_file_.c_str(), false, block_list_);
  } else
#endif // ndef DISABLED_LEGACY_ENGINE
  {
    page_res_ =
        new PAGE_RES(tesseract_->AnyLSTMLang(), block_list_, &tesseract_->prev_word_best_choice_);
  }

  if (page_res_ == nullptr) {
    return -1;
  }

  if (tesseract_->tessedit_train_line_recognizer) {
    if (!tesseract_->TrainLineRecognizer(input_file_.c_str(), output_file_, block_list_)) {
      return -1;
    }
    tesseract_->CorrectClassifyWords(page_res_);
    return 0;
  }
#ifndef DISABLED_LEGACY_ENGINE
  if (tesseract_->tessedit_make_boxes_from_boxes) {
    tesseract_->CorrectClassifyWords(page_res_);
    return 0;
  }
#endif // ndef DISABLED_LEGACY_ENGINE

  int result = 0;
  if (tesseract_->interactive_display_mode) {
#ifndef GRAPHICS_DISABLED
    tesseract_->pgeditor_main(rect_width_, rect_height_, page_res_);
#endif // !GRAPHICS_DISABLED
       // The page_res is invalid after an interactive session, so cleanup
       // in a way that lets us continue to the next page without crashing.
    delete page_res_;
    page_res_ = nullptr;
    return -1;
#ifndef DISABLED_LEGACY_ENGINE
  } else if (tesseract_->tessedit_train_from_boxes) {
    std::string fontname;
    ExtractFontName(output_file_.c_str(), &fontname);
    tesseract_->ApplyBoxTraining(fontname, page_res_);
  } else if (tesseract_->tessedit_ambigs_training) {
    FILE *training_output_file = tesseract_->init_recog_training(input_file_.c_str());
    // OCR the page segmented into words by tesseract.
    tesseract_->recog_training_segmented(input_file_.c_str(), page_res_, monitor,
                                         training_output_file);
    fclose(training_output_file);
#endif // ndef DISABLED_LEGACY_ENGINE
  } else {
    // Now run the main recognition.
    bool wait_for_text = true;
    GetBoolVariable("paragraph_text_based", &wait_for_text);
    if (!wait_for_text) {
      DetectParagraphs(false);
    }
    if (tesseract_->recog_all_words(page_res_, monitor, nullptr, nullptr, 0)) {
      if (wait_for_text) {
        DetectParagraphs(true);
      }
    } else {
      result = -1;
    }
  }
  return result;
}

// Takes ownership of the input pix.
void TessBaseAPI::SetInputImage(Pix *pix) {
  tesseract_->set_pix_original(pix);
}

Pix *TessBaseAPI::GetInputImage() {
  return tesseract_->pix_original();
}

const char *TessBaseAPI::GetInputName() {
  if (!input_file_.empty()) {
    return input_file_.c_str();
  }
  return nullptr;
}

const char *TessBaseAPI::GetDatapath() {
  return tesseract_->datadir.c_str();
}

int TessBaseAPI::GetSourceYResolution() {
  if (thresholder_ == nullptr)
    return -1;
  return thresholder_->GetSourceYResolution();
}

// If flist exists, get data from there. Otherwise get data from buf.
// Seems convoluted, but is the easiest way I know of to meet multiple
// goals. Support streaming from stdin, and also work on platforms
// lacking fmemopen.
// TODO: check different logic for flist/buf and simplify.
bool TessBaseAPI::ProcessPagesFileList(FILE *flist, std::string *buf, const char *retry_config,
                                       int timeout_millisec, TessResultRenderer *renderer,
                                       int tessedit_page_number) {
  if (!flist && !buf) {
    return false;
  }
  unsigned page = (tessedit_page_number >= 0) ? tessedit_page_number : 0;
  char pagename[MAX_PATH];

  std::vector<std::string> lines;
  if (!flist) {
    std::string line;
    for (const auto ch : *buf) {
      if (ch == '\n') {
        lines.push_back(line);
        line.clear();
      } else {
        line.push_back(ch);
      }
    }
    if (!line.empty()) {
      // Add last line without terminating LF.
      lines.push_back(line);
    }
    if (lines.empty()) {
      return false;
    }
  }

  // Skip to the requested page number.
  for (unsigned i = 0; i < page; i++) {
    if (flist) {
      if (fgets(pagename, sizeof(pagename), flist) == nullptr) {
        break;
      }
    }
  }

  // Begin producing output
  if (renderer && !renderer->BeginDocument(document_title.c_str())) {
    return false;
  }

  // Loop over all pages - or just the requested one
  while (true) {
    if (flist) {
      if (fgets(pagename, sizeof(pagename), flist) == nullptr) {
        break;
      }
    } else {
      if (page >= lines.size()) {
        break;
      }
      snprintf(pagename, sizeof(pagename), "%s", lines[page].c_str());
    }
    chomp_string(pagename);
    Pix *pix = pixRead(pagename);
    if (pix == nullptr) {
      tprintf("Image file %s cannot be read!\n", pagename);
      return false;
    }
    tprintf("Page %u : %s\n", page, pagename);
    bool r = ProcessPage(pix, page, pagename, retry_config, timeout_millisec, renderer);
    pixDestroy(&pix);
    if (!r) {
      return false;
    }
    if (tessedit_page_number >= 0) {
      break;
    }
    ++page;
  }

  // Finish producing output
  if (renderer && !renderer->EndDocument()) {
    return false;
  }
  return true;
}

bool TessBaseAPI::ProcessPagesMultipageTiff(const l_uint8 *data, size_t size, const char *filename,
                                            const char *retry_config, int timeout_millisec,
                                            TessResultRenderer *renderer,
                                            int tessedit_page_number) {
  Pix *pix = nullptr;
  int page = (tessedit_page_number >= 0) ? tessedit_page_number : 0;
  size_t offset = 0;
  for (;; ++page) {
    if (tessedit_page_number >= 0) {
      page = tessedit_page_number;
      pix = (data) ? pixReadMemTiff(data, size, page) : pixReadTiff(filename, page);
    } else {
      pix = (data) ? pixReadMemFromMultipageTiff(data, size, &offset)
                   : pixReadFromMultipageTiff(filename, &offset);
    }
    if (pix == nullptr) {
      break;
    }
    if (offset || page > 0) {
      // Only print page number for multipage TIFF file.
      tprintf("Page %d\n", page + 1);
    }
    auto page_string = std::to_string(page);
    SetVariable("applybox_page", page_string.c_str());
    bool r = ProcessPage(pix, page, filename, retry_config, timeout_millisec, renderer);
    pixDestroy(&pix);
    if (!r) {
      return false;
    }
    if (tessedit_page_number >= 0) {
      break;
    }
    if (!offset) {
      break;
    }
  }
  return true;
}

// Master ProcessPages calls ProcessPagesInternal and then does any post-
// processing required due to being in a training mode.
bool TessBaseAPI::ProcessPages(const char *filename, const char *retry_config, int timeout_millisec,
                               TessResultRenderer *renderer) {
  bool result = ProcessPagesInternal(filename, retry_config, timeout_millisec, renderer);
#ifndef DISABLED_LEGACY_ENGINE
  if (result) {
    if (tesseract_->tessedit_train_from_boxes && !tesseract_->WriteTRFile(output_file_.c_str())) {
      tprintf("Write of TR file failed: %s\n", output_file_.c_str());
      return false;
    }
  }
#endif // ndef DISABLED_LEGACY_ENGINE
  return result;
}

#ifdef HAVE_LIBCURL
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size = size * nmemb;
  auto *buf = reinterpret_cast<std::string *>(userp);
  buf->append(reinterpret_cast<const char *>(contents), size);
  return size;
}
#endif

// In the ideal scenario, Tesseract will start working on data as soon
// as it can. For example, if you stream a filelist through stdin, we
// should start the OCR process as soon as the first filename is
// available. This is particularly useful when hooking Tesseract up to
// slow hardware such as a book scanning machine.
//
// Unfortunately there are tradeoffs. You can't seek on stdin. That
// makes automatic detection of datatype (TIFF? filelist? PNG?)
// impractical.  So we support a command line flag to explicitly
// identify the scenario that really matters: filelists on
// stdin. We'll still do our best if the user likes pipes.
bool TessBaseAPI::ProcessPagesInternal(const char *filename, const char *retry_config,
                                       int timeout_millisec, TessResultRenderer *renderer) {
  bool stdInput = !strcmp(filename, "stdin") || !strcmp(filename, "-");
  if (stdInput) {
#ifdef WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1)
      tprintf("ERROR: cin to binary: %s", strerror(errno));
#endif // WIN32
  }

  if (stream_filelist) {
    return ProcessPagesFileList(stdin, nullptr, retry_config, timeout_millisec, renderer,
                                tesseract_->tessedit_page_number);
  }

  // At this point we are officially in autodection territory.
  // That means any data in stdin must be buffered, to make it
  // seekable.
  std::string buf;
  const l_uint8 *data = nullptr;
  if (stdInput) {
    buf.assign((std::istreambuf_iterator<char>(std::cin)), (std::istreambuf_iterator<char>()));
    data = reinterpret_cast<const l_uint8 *>(buf.data());
  } else if (strstr(filename, "://") != nullptr) {
    // Get image or image list by URL.
#ifdef HAVE_LIBCURL
    CURL *curl = curl_easy_init();
    if (curl == nullptr) {
      fprintf(stderr, "Error, curl_easy_init failed\n");
      return false;
    } else {
      CURLcode curlcode;
      auto error = [curl, &curlcode](const char *function) {
        fprintf(stderr, "Error, %s failed with error %s\n", function, curl_easy_strerror(curlcode));
        curl_easy_cleanup(curl);
        return false;
      };
      curlcode = curl_easy_setopt(curl, CURLOPT_URL, filename);
      if (curlcode != CURLE_OK) {
        return error("curl_easy_setopt");
      }
      // Follow HTTP, HTTPS, FTP and FTPS redirects.
      curlcode = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
      if (curlcode != CURLE_OK) {
        return error("curl_easy_setopt");
      }
      // Allow no more than 8 redirections to prevent endless loops.
      curlcode = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 8);
      if (curlcode != CURLE_OK) {
        return error("curl_easy_setopt");
      }
      int timeout = curl_timeout;
      if (timeout > 0) {
        curlcode = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        if (curlcode != CURLE_OK) {
          return error("curl_easy_setopt");
        }
        curlcode = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        if (curlcode != CURLE_OK) {
          return error("curl_easy_setopt");
        }
      }
      curlcode = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
      if (curlcode != CURLE_OK) {
        return error("curl_easy_setopt");
      }
      curlcode = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
      if (curlcode != CURLE_OK) {
        return error("curl_easy_setopt");
      }
      curlcode = curl_easy_perform(curl);
      if (curlcode != CURLE_OK) {
        return error("curl_easy_perform");
      }
      curl_easy_cleanup(curl);
      data = reinterpret_cast<const l_uint8 *>(buf.data());
    }
#else
    fprintf(stderr, "Error, this tesseract has no URL support\n");
    return false;
#endif
  } else {
    // Check whether the input file can be read.
    if (FILE *file = fopen(filename, "rb")) {
      fclose(file);
    } else {
      fprintf(stderr, "Error, cannot read input file %s: %s\n", filename, strerror(errno));
      return false;
    }
  }

  // Here is our autodetection
  int format;
  int r =
      (data != nullptr) ? findFileFormatBuffer(data, &format) : findFileFormat(filename, &format);

  // Maybe we have a filelist
  if (r != 0 || format == IFF_UNKNOWN) {
    std::string s;
    if (data != nullptr) {
      s = buf.c_str();
    } else {
      std::ifstream t(filename);
      std::string u((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
      s = u.c_str();
    }
    return ProcessPagesFileList(nullptr, &s, retry_config, timeout_millisec, renderer,
                                tesseract_->tessedit_page_number);
  }

  // Maybe we have a TIFF which is potentially multipage
  bool tiff = (format == IFF_TIFF || format == IFF_TIFF_PACKBITS || format == IFF_TIFF_RLE ||
               format == IFF_TIFF_G3 || format == IFF_TIFF_G4 || format == IFF_TIFF_LZW ||
#if LIBLEPT_MAJOR_VERSION > 1 || LIBLEPT_MINOR_VERSION > 76
               format == IFF_TIFF_JPEG ||
#endif
               format == IFF_TIFF_ZIP);

  // Fail early if we can, before producing any output
  Pix *pix = nullptr;
  if (!tiff) {
    pix = (data != nullptr) ? pixReadMem(data, buf.size()) : pixRead(filename);
    if (pix == nullptr) {
      return false;
    }
  }

  // Begin the output
  if (renderer && !renderer->BeginDocument(document_title.c_str())) {
    pixDestroy(&pix);
    return false;
  }

  // Produce output
  r = (tiff) ? ProcessPagesMultipageTiff(data, buf.size(), filename, retry_config, timeout_millisec,
                                         renderer, tesseract_->tessedit_page_number)
             : ProcessPage(pix, 0, filename, retry_config, timeout_millisec, renderer);

  // Clean up memory as needed
  pixDestroy(&pix);

  // End the output
  if (!r || (renderer && !renderer->EndDocument())) {
    return false;
  }
  return true;
}

bool TessBaseAPI::ProcessPage(Pix *pix, int page_index, const char *filename,
                              const char *retry_config, int timeout_millisec,
                              TessResultRenderer *renderer) {
  SetInputName(filename);
  SetImage(pix);
  bool failed = false;

  if (tesseract_->tessedit_pageseg_mode == PSM_AUTO_ONLY) {
    // Disabled character recognition
    if (! std::unique_ptr<const PageIterator>(AnalyseLayout())) {
      failed = true;
    }
  } else if (tesseract_->tessedit_pageseg_mode == PSM_OSD_ONLY) {
    failed = FindLines() != 0;
  } else if (timeout_millisec > 0) {
    // Running with a timeout.
    ETEXT_DESC monitor;
    monitor.cancel = nullptr;
    monitor.cancel_this = nullptr;
    monitor.set_deadline_msecs(timeout_millisec);

    // Now run the main recognition.
    failed = Recognize(&monitor) < 0;
  } else {
    // Normal layout and character recognition with no timeout.
    failed = Recognize(nullptr) < 0;
  }

  if (tesseract_->tessedit_write_images) {
    Pix *page_pix = GetThresholdedImage();
    std::string output_filename = output_file_ + ".processed";
    if (page_index > 0) {
      output_filename += std::to_string(page_index);
    }
    output_filename += ".tif";
    pixWrite(output_filename.c_str(), page_pix, IFF_TIFF_G4);
    pixDestroy(&page_pix);
  }

  if (failed && retry_config != nullptr && retry_config[0] != '\0') {
    // Save current config variables before switching modes.
    FILE *fp = fopen(kOldVarsFile, "wb");
    if (fp == nullptr) {
      tprintf("Error, failed to open file \"%s\"\n", kOldVarsFile);
    } else {
      PrintVariables(fp);
      fclose(fp);
    }
    // Switch to alternate mode for retry.
    ReadConfigFile(retry_config);
    SetImage(pix);
    Recognize(nullptr);
    // Restore saved config variables.
    ReadConfigFile(kOldVarsFile);
  }

  if (renderer && !failed) {
    failed = !renderer->AddImage(this);
  }

  return !failed;
}

/**
 * Get a left-to-right iterator to the results of LayoutAnalysis and/or
 * Recognize. The returned iterator must be deleted after use.
 */
LTRResultIterator *TessBaseAPI::GetLTRIterator() {
  if (tesseract_ == nullptr || page_res_ == nullptr) {
    return nullptr;
  }
  return new LTRResultIterator(page_res_, tesseract_, thresholder_->GetScaleFactor(),
                               thresholder_->GetScaledYResolution(), rect_left_, rect_top_,
                               rect_width_, rect_height_);
}

/**
 * Get a reading-order iterator to the results of LayoutAnalysis and/or
 * Recognize. The returned iterator must be deleted after use.
 * WARNING! This class points to data held within the TessBaseAPI class, and
 * therefore can only be used while the TessBaseAPI class still exists and
 * has not been subjected to a call of Init, SetImage, Recognize, Clear, End
 * DetectOS, or anything else that changes the internal PAGE_RES.
 */
ResultIterator *TessBaseAPI::GetIterator() {
  if (tesseract_ == nullptr || page_res_ == nullptr) {
    return nullptr;
  }
  return ResultIterator::StartOfParagraph(LTRResultIterator(
      page_res_, tesseract_, thresholder_->GetScaleFactor(), thresholder_->GetScaledYResolution(),
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
MutableIterator *TessBaseAPI::GetMutableIterator() {
  if (tesseract_ == nullptr || page_res_ == nullptr) {
    return nullptr;
  }
  return new MutableIterator(page_res_, tesseract_, thresholder_->GetScaleFactor(),
                             thresholder_->GetScaledYResolution(), rect_left_, rect_top_,
                             rect_width_, rect_height_);
}

/** Make a text string from the internal data structures. */
char *TessBaseAPI::GetUTF8Text() {
  if (tesseract_ == nullptr || (!recognition_done_ && Recognize(nullptr) < 0)) {
    return nullptr;
  }
  std::string text("");
  const std::unique_ptr</*non-const*/ ResultIterator> it(GetIterator());
  do {
    if (it->Empty(RIL_PARA)) {
      continue;
    }
    auto block_type = it->BlockType();
    switch (block_type) {
      case PT_FLOWING_IMAGE:
      case PT_HEADING_IMAGE:
      case PT_PULLOUT_IMAGE:
      case PT_HORZ_LINE:
      case PT_VERT_LINE:
        // Ignore images and lines for text output.
        continue;
      case PT_NOISE:
        tprintf("TODO: Please report image which triggers the noise case.\n");
        ASSERT_HOST(false);
      default:
        break;
    }

    const std::unique_ptr<const char[]> para_text(it->GetUTF8Text(RIL_PARA));
    text += para_text.get();
  } while (it->Next(RIL_PARA));
  char *result = new char[text.length() + 1];
  strncpy(result, text.c_str(), text.length() + 1);
  return result;
}

static void AddBoxToTSV(const PageIterator *it, PageIteratorLevel level, std::string &text) {
  int left, top, right, bottom;
  it->BoundingBox(level, &left, &top, &right, &bottom);
  text += "\t" + std::to_string(left);
  text += "\t" + std::to_string(top);
  text += "\t" + std::to_string(right - left);
  text += "\t" + std::to_string(bottom - top);
}

/**
 * Make a TSV-formatted string from the internal data structures.
 * page_number is 0-based but will appear in the output as 1-based.
 * Returned string must be freed with the delete [] operator.
 */
char *TessBaseAPI::GetTSVText(int page_number) {
  if (tesseract_ == nullptr || (page_res_ == nullptr && Recognize(nullptr) < 0)) {
    return nullptr;
  }

  int lcnt = 1, bcnt = 1, pcnt = 1, wcnt = 1;
  int page_id = page_number + 1; // we use 1-based page numbers.

  int page_num = page_id;
  int block_num = 0;
  int par_num = 0;
  int line_num = 0;
  int word_num = 0;

  std::string tsv_str;
  tsv_str += "1\t" + std::to_string(page_num); // level 1 - page
  tsv_str += "\t" + std::to_string(block_num);
  tsv_str += "\t" + std::to_string(par_num);
  tsv_str += "\t" + std::to_string(line_num);
  tsv_str += "\t" + std::to_string(word_num);
  tsv_str += "\t" + std::to_string(rect_left_);
  tsv_str += "\t" + std::to_string(rect_top_);
  tsv_str += "\t" + std::to_string(rect_width_);
  tsv_str += "\t" + std::to_string(rect_height_);
  tsv_str += "\t-1\t\n";

  const std::unique_ptr</*non-const*/ ResultIterator> res_it(GetIterator());
  while (!res_it->Empty(RIL_BLOCK)) {
    if (res_it->Empty(RIL_WORD)) {
      res_it->Next(RIL_WORD);
      continue;
    }

    // Add rows for any new block/paragraph/textline.
    if (res_it->IsAtBeginningOf(RIL_BLOCK)) {
      block_num++;
      par_num = 0;
      line_num = 0;
      word_num = 0;
      tsv_str += "2\t" + std::to_string(page_num); // level 2 - block
      tsv_str += "\t" + std::to_string(block_num);
      tsv_str += "\t" + std::to_string(par_num);
      tsv_str += "\t" + std::to_string(line_num);
      tsv_str += "\t" + std::to_string(word_num);
      AddBoxToTSV(res_it.get(), RIL_BLOCK, tsv_str);
      tsv_str += "\t-1\t\n"; // end of row for block
    }
    if (res_it->IsAtBeginningOf(RIL_PARA)) {
      par_num++;
      line_num = 0;
      word_num = 0;
      tsv_str += "3\t" + std::to_string(page_num); // level 3 - paragraph
      tsv_str += "\t" + std::to_string(block_num);
      tsv_str += "\t" + std::to_string(par_num);
      tsv_str += "\t" + std::to_string(line_num);
      tsv_str += "\t" + std::to_string(word_num);
      AddBoxToTSV(res_it.get(), RIL_PARA, tsv_str);
      tsv_str += "\t-1\t\n"; // end of row for para
    }
    if (res_it->IsAtBeginningOf(RIL_TEXTLINE)) {
      line_num++;
      word_num = 0;
      tsv_str += "4\t" + std::to_string(page_num); // level 4 - line
      tsv_str += "\t" + std::to_string(block_num);
      tsv_str += "\t" + std::to_string(par_num);
      tsv_str += "\t" + std::to_string(line_num);
      tsv_str += "\t" + std::to_string(word_num);
      AddBoxToTSV(res_it.get(), RIL_TEXTLINE, tsv_str);
      tsv_str += "\t-1\t\n"; // end of row for line
    }

    // Now, process the word...
    int left, top, right, bottom;
    res_it->BoundingBox(RIL_WORD, &left, &top, &right, &bottom);
    word_num++;
    tsv_str += "5\t" + std::to_string(page_num); // level 5 - word
    tsv_str += "\t" + std::to_string(block_num);
    tsv_str += "\t" + std::to_string(par_num);
    tsv_str += "\t" + std::to_string(line_num);
    tsv_str += "\t" + std::to_string(word_num);
    tsv_str += "\t" + std::to_string(left);
    tsv_str += "\t" + std::to_string(top);
    tsv_str += "\t" + std::to_string(right - left);
    tsv_str += "\t" + std::to_string(bottom - top);
    tsv_str += "\t" + std::to_string(res_it->Confidence(RIL_WORD));
    tsv_str += "\t";

    // Increment counts if at end of block/paragraph/textline.
    if (res_it->IsAtFinalElement(RIL_TEXTLINE, RIL_WORD)) {
      lcnt++;
    }
    if (res_it->IsAtFinalElement(RIL_PARA, RIL_WORD)) {
      pcnt++;
    }
    if (res_it->IsAtFinalElement(RIL_BLOCK, RIL_WORD)) {
      bcnt++;
    }

    do {
      tsv_str += std::unique_ptr<const char[]>(res_it->GetUTF8Text(RIL_SYMBOL)).get();
      res_it->Next(RIL_SYMBOL);
    } while (!res_it->Empty(RIL_BLOCK) && !res_it->IsAtBeginningOf(RIL_WORD));
    tsv_str += "\n"; // end of row
    wcnt++;
  }

  char *ret = new char[tsv_str.length() + 1];
  strcpy(ret, tsv_str.c_str());
  return ret;
}

/** The 5 numbers output for each box (the usual 4 and a page number.) */
const int kNumbersPerBlob = 5;
/**
 * The number of bytes taken by each number. Since we use int16_t for ICOORD,
 * assume only 5 digits max.
 */
const int kBytesPerNumber = 5;
/**
 * Multiplier for max expected textlength assumes (kBytesPerNumber + space)
 * * kNumbersPerBlob plus the newline. Add to this the
 * original UTF8 characters, and one kMaxBytesPerLine for safety.
 */
const int kBytesPerBoxFileLine = (kBytesPerNumber + 1) * kNumbersPerBlob + 1;
/** Max bytes in the decimal representation of int64_t. */
const int kBytesPer64BitNumber = 20;
/**
 * A maximal single box could occupy kNumbersPerBlob numbers at
 * kBytesPer64BitNumber digits (if someone sneaks in a 64 bit value) and a
 * space plus the newline and the maximum length of a UNICHAR.
 * Test against this on each iteration for safety.
 */
const int kMaxBytesPerLine = kNumbersPerBlob * (kBytesPer64BitNumber + 1) + 1 + UNICHAR_LEN;

/**
 * The recognized text is returned as a char* which is coded
 * as a UTF8 box file.
 * page_number is a 0-base page index that will appear in the box file.
 * Returned string must be freed with the delete [] operator.
 */
char *TessBaseAPI::GetBoxText(int page_number) {
  if (tesseract_ == nullptr || (!recognition_done_ && Recognize(nullptr) < 0)) {
    return nullptr;
  }
  int blob_count;
  int utf8_length = TextLength(&blob_count);
  int total_length = blob_count * kBytesPerBoxFileLine + utf8_length + kMaxBytesPerLine;
  char *result = new char[total_length];
  result[0] = '\0';
  int output_length = 0;
  LTRResultIterator *it = GetLTRIterator();
  do {
    int left, top, right, bottom;
    if (it->BoundingBox(RIL_SYMBOL, &left, &top, &right, &bottom)) {
      const std::unique_ptr</*non-const*/ char[]> text(it->GetUTF8Text(RIL_SYMBOL));
      // Tesseract uses space for recognition failure. Fix to a reject
      // character, kTesseractReject so we don't create illegal box files.
      for (int i = 0; text[i] != '\0'; ++i) {
        if (text[i] == ' ') {
          text[i] = kTesseractReject;
        }
      }
      snprintf(result + output_length, total_length - output_length, "%s %d %d %d %d %d\n",
               text.get(), left, image_height_ - bottom, right, image_height_ - top, page_number);
      output_length += strlen(result + output_length);
      // Just in case...
      if (output_length + kMaxBytesPerLine > total_length) {
        break;
      }
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
const int kUniChs[] = {0x20ac, 0x201c, 0x201d, 0x2018, 0x2019, 0x2022, 0x2014, 0};
/** Latin chars corresponding to the unicode chars above. */
const int kLatinChs[] = {0x00a2, 0x0022, 0x0022, 0x0027, 0x0027, 0x00b7, 0x002d, 0};

/**
 * The recognized text is returned as a char* which is coded
 * as UNLV format Latin-1 with specific reject and suspect codes.
 * Returned string must be freed with the delete [] operator.
 */
char *TessBaseAPI::GetUNLVText() {
  if (tesseract_ == nullptr || (!recognition_done_ && Recognize(nullptr) < 0)) {
    return nullptr;
  }
  bool tilde_crunch_written = false;
  bool last_char_was_newline = true;
  bool last_char_was_tilde = false;

  int total_length = TextLength(nullptr);
  PAGE_RES_IT page_res_it(page_res_);
  char *result = new char[total_length];
  char *ptr = result;
  for (page_res_it.restart_page(); page_res_it.word() != nullptr; page_res_it.forward()) {
    WERD_RES *word = page_res_it.word();
    // Process the current word.
    if (word->unlv_crunch_mode != CR_NONE) {
      if (word->unlv_crunch_mode != CR_DELETE &&
          (!tilde_crunch_written ||
           (word->unlv_crunch_mode == CR_KEEP_SPACE && word->word->space() > 0 &&
            !word->word->flag(W_FUZZY_NON) && !word->word->flag(W_FUZZY_SP)))) {
        if (!word->word->flag(W_BOL) && word->word->space() > 0 && !word->word->flag(W_FUZZY_NON) &&
            !word->word->flag(W_FUZZY_SP)) {
          /* Write a space to separate from preceding good text */
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
      const char *wordstr = word->best_choice->unichar_string().c_str();
      const auto &lengths = word->best_choice->unichar_lengths();
      int length = lengths.length();
      int i = 0;
      int offset = 0;

      if (last_char_was_tilde && word->word->space() == 0 && wordstr[offset] == ' ') {
        // Prevent adjacent tilde across words - we know that adjacent tildes
        // within words have been removed.
        // Skip the first character.
        offset = lengths[i++];
      }
      if (i < length && wordstr[offset] != 0) {
        if (!last_char_was_newline) {
          *ptr++ = ' ';
        } else {
          last_char_was_newline = false;
        }
        for (; i < length; offset += lengths[i++]) {
          if (wordstr[offset] == ' ' || wordstr[offset] == kTesseractReject) {
            *ptr++ = kUNLVReject;
            last_char_was_tilde = true;
          } else {
            if (word->reject_map[i].rejected()) {
              *ptr++ = kUNLVSuspect;
            }
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

#ifndef DISABLED_LEGACY_ENGINE

/**
 * Detect the orientation of the input image and apparent script (alphabet).
 * orient_deg is the detected clockwise rotation of the input image in degrees
 * (0, 90, 180, 270)
 * orient_conf is the confidence (15.0 is reasonably confident)
 * script_name is an ASCII string, the name of the script, e.g. "Latin"
 * script_conf is confidence level in the script
 * Returns true on success and writes values to each parameter as an output
 */
bool TessBaseAPI::DetectOrientationScript(int *orient_deg, float *orient_conf,
                                          const char **script_name, float *script_conf) {
  OSResults osr;

  bool osd = DetectOS(&osr);
  if (!osd) {
    return false;
  }

  int orient_id = osr.best_result.orientation_id;
  int script_id = osr.get_best_script(orient_id);
  if (orient_conf) {
    *orient_conf = osr.best_result.oconfidence;
  }
  if (orient_deg) {
    *orient_deg = orient_id * 90; // convert quadrant to degrees
  }

  if (script_name) {
    const char *script = osr.unicharset->get_script_from_script_id(script_id);

    *script_name = script;
  }

  if (script_conf) {
    *script_conf = osr.best_result.sconfidence;
  }

  return true;
}

/**
 * The recognized text is returned as a char* which is coded
 * as UTF8 and must be freed with the delete [] operator.
 * page_number is a 0-based page index that will appear in the osd file.
 */
char *TessBaseAPI::GetOsdText(int page_number) {
  int orient_deg;
  float orient_conf;
  const char *script_name;
  float script_conf;

  if (!DetectOrientationScript(&orient_deg, &orient_conf, &script_name, &script_conf)) {
    return nullptr;
  }

  // clockwise rotation needed to make the page upright
  int rotate = OrientationIdToValue(orient_deg / 90);

  std::stringstream stream;
  // Use "C" locale (needed for float values orient_conf and script_conf).
  stream.imbue(std::locale::classic());
  // Use fixed notation with 2 digits after the decimal point for float values.
  stream.precision(2);
  stream << std::fixed << "Page number: " << page_number << "\n"
         << "Orientation in degrees: " << orient_deg << "\n"
         << "Rotate: " << rotate << "\n"
         << "Orientation confidence: " << orient_conf << "\n"
         << "Script: " << script_name << "\n"
         << "Script confidence: " << script_conf << "\n";
  const std::string &text = stream.str();
  char *result = new char[text.length() + 1];
  strcpy(result, text.c_str());
  return result;
}

#endif // ndef DISABLED_LEGACY_ENGINE

/** Returns the average word confidence for Tesseract page result. */
int TessBaseAPI::MeanTextConf() {
  int *conf = AllWordConfidences();
  if (!conf) {
    return 0;
  }
  int sum = 0;
  int *pt = conf;
  while (*pt >= 0) {
    sum += *pt++;
  }
  if (pt != conf) {
    sum /= pt - conf;
  }
  delete[] conf;
  return sum;
}

/** Returns an array of all word confidences, terminated by -1. */
int *TessBaseAPI::AllWordConfidences() {
  if (tesseract_ == nullptr || (!recognition_done_ && Recognize(nullptr) < 0)) {
    return nullptr;
  }
  int n_word = 0;
  PAGE_RES_IT res_it(page_res_);
  for (res_it.restart_page(); res_it.word() != nullptr; res_it.forward()) {
    n_word++;
  }

  int *conf = new int[n_word + 1];
  n_word = 0;
  for (res_it.restart_page(); res_it.word() != nullptr; res_it.forward()) {
    WERD_RES *word = res_it.word();
    WERD_CHOICE *choice = word->best_choice;
    int w_conf = static_cast<int>(100 + 5 * choice->certainty());
    // This is the eq for converting Tesseract confidence to 1..100
    if (w_conf < 0) {
      w_conf = 0;
    }
    if (w_conf > 100) {
      w_conf = 100;
    }
    conf[n_word++] = w_conf;
  }
  conf[n_word] = -1;
  return conf;
}

#ifndef DISABLED_LEGACY_ENGINE
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
bool TessBaseAPI::AdaptToWordStr(PageSegMode mode, const char *wordstr) {
  int debug = 0;
  GetIntVariable("applybox_debug", &debug);
  bool success = true;
  PageSegMode current_psm = GetPageSegMode();
  SetPageSegMode(mode);
  SetVariable("classify_enable_learning", "0");
  const std::unique_ptr<const char[]> text(GetUTF8Text());
  if (debug) {
    tprintf("Trying to adapt \"%s\" to \"%s\"\n", text.get(), wordstr);
  }
  if (text != nullptr) {
    PAGE_RES_IT it(page_res_);
    WERD_RES *word_res = it.word();
    if (word_res != nullptr) {
      word_res->word->set_text(wordstr);
      // Check to see if text matches wordstr.
      int w = 0;
      int t;
      for (t = 0; text[t] != '\0'; ++t) {
        if (text[t] == '\n' || text[t] == ' ') {
          continue;
        }
        while (wordstr[w] == ' ') {
          ++w;
        }
        if (text[t] != wordstr[w]) {
          break;
        }
        ++w;
      }
      if (text[t] != '\0' || wordstr[w] != '\0') {
        // No match.
        delete page_res_;
        std::vector<TBOX> boxes;
        page_res_ = tesseract_->SetupApplyBoxes(boxes, block_list_);
        tesseract_->ReSegmentByClassification(page_res_);
        tesseract_->TidyUp(page_res_);
        PAGE_RES_IT pr_it(page_res_);
        if (pr_it.word() == nullptr) {
          success = false;
        } else {
          word_res = pr_it.word();
        }
      } else {
        word_res->BestChoiceToCorrectText();
      }
      if (success) {
        tesseract_->EnableLearning = true;
        tesseract_->LearnWord(nullptr, word_res);
      }
    } else {
      success = false;
    }
  } else {
    success = false;
  }
  SetPageSegMode(current_psm);
  return success;
}
#endif // ndef DISABLED_LEGACY_ENGINE

/**
 * Free up recognition results and any stored image data, without actually
 * freeing any recognition data that would be time-consuming to reload.
 * Afterwards, you must call SetImage or TesseractRect before doing
 * any Recognize or Get* operation.
 */
void TessBaseAPI::Clear() {
  if (thresholder_ != nullptr) {
    thresholder_->Clear();
  }
  ClearResults();
  if (tesseract_ != nullptr) {
    SetInputImage(nullptr);
  }
}

/**
 * Close down tesseract and free up all memory. End() is equivalent to
 * destructing and reconstructing your TessBaseAPI.
 * Once End() has been used, none of the other API functions may be used
 * other than Init and anything declared above it in the class definition.
 */
void TessBaseAPI::End() {
  Clear();
  delete thresholder_;
  thresholder_ = nullptr;
  delete page_res_;
  page_res_ = nullptr;
  delete block_list_;
  block_list_ = nullptr;
  if (paragraph_models_ != nullptr) {
    for (auto model : *paragraph_models_) {
      delete model;
    }
    delete paragraph_models_;
    paragraph_models_ = nullptr;
  }
#ifndef DISABLED_LEGACY_ENGINE
  if (osd_tesseract_ == tesseract_) {
    osd_tesseract_ = nullptr;
  }
  delete osd_tesseract_;
  osd_tesseract_ = nullptr;
  delete equ_detect_;
  equ_detect_ = nullptr;
#endif // ndef DISABLED_LEGACY_ENGINE
  delete tesseract_;
  tesseract_ = nullptr;
  input_file_.clear();
  output_file_.clear();
  datapath_.clear();
  language_.clear();
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
int TessBaseAPI::IsValidWord(const char *word) const {
  return tesseract_->getDict().valid_word(word);
}
// Returns true if utf8_character is defined in the UniCharset.
bool TessBaseAPI::IsValidCharacter(const char *utf8_character) const {
  return tesseract_->unicharset.contains_unichar(utf8_character);
}

// TODO(rays) Obsolete this function and replace with a more aptly named
// function that returns image coordinates rather than tesseract coordinates.
bool TessBaseAPI::GetTextDirection(int *out_offset, float *out_slope) {
  const std::unique_ptr<const PageIterator> it(AnalyseLayout());
  if (it == nullptr) {
    return false;
  }
  int x1, x2, y1, y2;
  it->Baseline(RIL_TEXTLINE, &x1, &y1, &x2, &y2);
  // Calculate offset and slope (NOTE: Kind of ugly)
  if (x2 <= x1) {
    x2 = x1 + 1;
  }
  // Convert the point pair to slope/offset of the baseline (in image coords.)
  *out_slope = static_cast<float>(y2 - y1) / (x2 - x1);
  *out_offset = static_cast<int>(y1 - *out_slope * x1);
  // Get the y-coord of the baseline at the left and right edges of the
  // textline's bounding box.
  int left, top, right, bottom;
  if (!it->BoundingBox(RIL_TEXTLINE, &left, &top, &right, &bottom)) {
    return false;
  }
  int left_y = IntCastRounded(*out_slope * left + *out_offset);
  int right_y = IntCastRounded(*out_slope * right + *out_offset);
  // Shift the baseline down so it passes through the nearest bottom-corner
  // of the textline's bounding box. This is the difference between the y
  // at the lowest (max) edge of the box and the actual box bottom.
  *out_offset += bottom - std::max(left_y, right_y);
  // Switch back to bottom-up tesseract coordinates. Requires negation of
  // the slope and height - offset for the offset.
  *out_slope = -*out_slope;
  *out_offset = rect_height_ - *out_offset;

  return true;
}

/** Sets Dict::letter_is_okay_ function to point to the given function. */
void TessBaseAPI::SetDictFunc(DictFunc f) {
  if (tesseract_ != nullptr) {
    tesseract_->getDict().letter_is_okay_ = f;
  }
}

/**
 * Sets Dict::probability_in_context_ function to point to the given
 * function.
 *
 * @param f A single function that returns the probability of the current
 * "character" (in general a utf-8 string), given the context of a previous
 * utf-8 string.
 */
void TessBaseAPI::SetProbabilityInContextFunc(ProbabilityInContextFunc f) {
  if (tesseract_ != nullptr) {
    tesseract_->getDict().probability_in_context_ = f;
    // Set it for the sublangs too.
    int num_subs = tesseract_->num_sub_langs();
    for (int i = 0; i < num_subs; ++i) {
      tesseract_->get_sub_lang(i)->getDict().probability_in_context_ = f;
    }
  }
}

/** Common code for setting the image. */
bool TessBaseAPI::InternalSetImage() {
  if (tesseract_ == nullptr) {
    tprintf("Please call Init before attempting to set an image.\n");
    return false;
  }
  if (thresholder_ == nullptr) {
    thresholder_ = new ImageThresholder;
  }
  ClearResults();
  return true;
}

/**
 * Run the thresholder to make the thresholded image, returned in pix,
 * which must not be nullptr. *pix must be initialized to nullptr, or point
 * to an existing pixDestroyable Pix.
 * The usual argument to Threshold is Tesseract::mutable_pix_binary().
 */
bool TessBaseAPI::Threshold(Pix **pix) {
  ASSERT_HOST(pix != nullptr);
  if (*pix != nullptr) {
    pixDestroy(pix);
  }
  // Zero resolution messes up the algorithms, so make sure it is credible.
  int user_dpi = 0;
  GetIntVariable("user_defined_dpi", &user_dpi);
  int y_res = thresholder_->GetScaledYResolution();
  if (user_dpi && (user_dpi < kMinCredibleResolution || user_dpi > kMaxCredibleResolution)) {
    tprintf(
        "Warning: User defined image dpi is outside of expected range "
        "(%d - %d)!\n",
        kMinCredibleResolution, kMaxCredibleResolution);
  }
  // Always use user defined dpi
  if (user_dpi) {
    thresholder_->SetSourceYResolution(user_dpi);
  } else if (y_res < kMinCredibleResolution || y_res > kMaxCredibleResolution) {
    if (y_res != 0) {
      // Show warning only if a resolution was given.
      tprintf("Warning: Invalid resolution %d dpi. Using %d instead.\n",
              y_res, kMinCredibleResolution);
    }
    thresholder_->SetSourceYResolution(kMinCredibleResolution);
  }

  auto thresholding_method = static_cast<ThresholdMethod>(static_cast<int>(tesseract_->thresholding_method));

  if (thresholding_method == ThresholdMethod::Otsu) {
    Image pix_binary(*pix);
    if (!thresholder_->ThresholdToPix(&pix_binary)) {
      return false;
    }
    *pix = pix_binary;

    if (!thresholder_->IsBinary()) {
      tesseract_->set_pix_thresholds(thresholder_->GetPixRectThresholds());
      tesseract_->set_pix_grey(thresholder_->GetPixRectGrey());
    } else {
      tesseract_->set_pix_thresholds(nullptr);
      tesseract_->set_pix_grey(nullptr);
    }
  } else {
    auto [ok, pix_grey, pix_binary, pix_thresholds] = thresholder_->Threshold(this, thresholding_method);

    if (!ok) {
      return false;
    }
    *pix = pix_binary;

    tesseract_->set_pix_thresholds(pix_thresholds);
    tesseract_->set_pix_grey(pix_grey);
  }

  thresholder_->GetImageSizes(&rect_left_, &rect_top_, &rect_width_, &rect_height_, &image_width_,
                              &image_height_);

  // Set the internal resolution that is used for layout parameters from the
  // estimated resolution, rather than the image resolution, which may be
  // fabricated, but we will use the image resolution, if there is one, to
  // report output point sizes.
  int estimated_res = ClipToRange(thresholder_->GetScaledEstimatedResolution(),
                                  kMinCredibleResolution, kMaxCredibleResolution);
  if (estimated_res != thresholder_->GetScaledEstimatedResolution()) {
    tprintf(
        "Estimated internal resolution %d out of range! "
        "Corrected to %d.\n",
        thresholder_->GetScaledEstimatedResolution(), estimated_res);
  }
  tesseract_->set_source_resolution(estimated_res);
  return true;
}

/** Find lines from the image making the BLOCK_LIST. */
int TessBaseAPI::FindLines() {
  if (thresholder_ == nullptr || thresholder_->IsEmpty()) {
    tprintf("Please call SetImage before attempting recognition.\n");
    return -1;
  }
  if (recognition_done_) {
    ClearResults();
  }
  if (!block_list_->empty()) {
    return 0;
  }
  if (tesseract_ == nullptr) {
    tesseract_ = new Tesseract;
#ifndef DISABLED_LEGACY_ENGINE
    tesseract_->InitAdaptiveClassifier(nullptr);
#endif
  }
  if (tesseract_->pix_binary() == nullptr && !Threshold(&tesseract_->mutable_pix_binary()->pix_)) {
    return -1;
  }

  tesseract_->PrepareForPageseg();

#ifndef DISABLED_LEGACY_ENGINE
  if (tesseract_->textord_equation_detect) {
    if (equ_detect_ == nullptr && !datapath_.empty()) {
      equ_detect_ = new EquationDetect(datapath_.c_str(), nullptr);
    }
    if (equ_detect_ == nullptr) {
      tprintf("Warning: Could not set equation detector\n");
    } else {
      tesseract_->SetEquationDetect(equ_detect_);
    }
  }
#endif // ndef DISABLED_LEGACY_ENGINE

  Tesseract *osd_tess = osd_tesseract_;
  OSResults osr;
#ifndef DISABLED_LEGACY_ENGINE
  if (PSM_OSD_ENABLED(tesseract_->tessedit_pageseg_mode) && osd_tess == nullptr) {
    if (strcmp(language_.c_str(), "osd") == 0) {
      osd_tess = tesseract_;
    } else {
      osd_tesseract_ = new Tesseract;
      TessdataManager mgr(reader_);
      if (datapath_.empty()) {
        tprintf(
            "Warning: Auto orientation and script detection requested,"
            " but data path is undefined\n");
        delete osd_tesseract_;
        osd_tesseract_ = nullptr;
      } else if (osd_tesseract_->init_tesseract(datapath_, "", "osd", OEM_TESSERACT_ONLY,
                                                nullptr, 0, nullptr, nullptr, false, &mgr) == 0) {
        osd_tess = osd_tesseract_;
        osd_tesseract_->set_source_resolution(thresholder_->GetSourceYResolution());
      } else {
        tprintf(
            "Warning: Auto orientation and script detection requested,"
            " but osd language failed to load\n");
        delete osd_tesseract_;
        osd_tesseract_ = nullptr;
      }
    }
  }
#endif // ndef DISABLED_LEGACY_ENGINE

  if (tesseract_->SegmentPage(input_file_.c_str(), block_list_, osd_tess, &osr) < 0) {
    return -1;
  }

  // If Devanagari is being recognized, we use different images for page seg
  // and for OCR.
  tesseract_->PrepareForTessOCR(block_list_, osd_tess, &osr);
  return 0;
}

/** Delete the pageres and clear the block list ready for a new page. */
void TessBaseAPI::ClearResults() {
  if (tesseract_ != nullptr) {
    tesseract_->Clear();
  }
  delete page_res_;
  page_res_ = nullptr;
  recognition_done_ = false;
  if (block_list_ == nullptr) {
    block_list_ = new BLOCK_LIST;
  } else {
    block_list_->clear();
  }
  if (paragraph_models_ != nullptr) {
    for (auto model : *paragraph_models_) {
      delete model;
    }
    delete paragraph_models_;
    paragraph_models_ = nullptr;
  }
}

/**
 * Return the length of the output text string, as UTF8, assuming
 * liberally two spacing marks after each word (as paragraphs end with two
 * newlines), and assuming a single character reject marker for each rejected
 * character.
 * Also return the number of recognized blobs in blob_count.
 */
int TessBaseAPI::TextLength(int *blob_count) const {
  if (tesseract_ == nullptr || page_res_ == nullptr) {
    return 0;
  }

  PAGE_RES_IT page_res_it(page_res_);
  int total_length = 2;
  int total_blobs = 0;
  // Iterate over the data structures to extract the recognition result.
  for (page_res_it.restart_page(); page_res_it.word() != nullptr; page_res_it.forward()) {
    WERD_RES *word = page_res_it.word();
    WERD_CHOICE *choice = word->best_choice;
    if (choice != nullptr) {
      total_blobs += choice->length() + 2;
      total_length += choice->unichar_string().length() + 2;
      for (int i = 0; i < word->reject_map.length(); ++i) {
        if (word->reject_map[i].rejected()) {
          ++total_length;
        }
      }
    }
  }
  if (blob_count != nullptr) {
    *blob_count = total_blobs;
  }
  return total_length;
}

#ifndef DISABLED_LEGACY_ENGINE
/**
 * Estimates the Orientation And Script of the image.
 * Returns true if the image was processed successfully.
 */
bool TessBaseAPI::DetectOS(OSResults *osr) {
  if (tesseract_ == nullptr) {
    return false;
  }
  ClearResults();
  if (tesseract_->pix_binary() == nullptr && !Threshold(&tesseract_->mutable_pix_binary()->pix_)) {
    return false;
  }

  if (input_file_.empty()) {
    input_file_ = kInputFile;
  }
  return orientation_and_script_detection(input_file_.c_str(), osr, tesseract_) > 0;
}
#endif // #ifndef DISABLED_LEGACY_ENGINE

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
void TessBaseAPI::GetBlockTextOrientations(int **block_orientation, bool **vertical_writing) {
  delete[] * block_orientation;
  *block_orientation = nullptr;
  delete[] * vertical_writing;
  *vertical_writing = nullptr;
  BLOCK_IT block_it(block_list_);

  block_it.move_to_first();
  int num_blocks = 0;
  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    if (!block_it.data()->pdblk.poly_block()->IsText()) {
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
  for (block_it.mark_cycle_pt(); !block_it.cycled_list(); block_it.forward()) {
    if (!block_it.data()->pdblk.poly_block()->IsText()) {
      continue;
    }
    FCOORD re_rotation = block_it.data()->re_rotation();
    float re_theta = re_rotation.angle();
    FCOORD classify_rotation = block_it.data()->classify_rotation();
    float classify_theta = classify_rotation.angle();
    double rot_theta = -(re_theta - classify_theta) * 2.0 / M_PI;
    if (rot_theta < 0) {
      rot_theta += 4;
    }
    int num_rotations = static_cast<int>(rot_theta + 0.5);
    (*block_orientation)[i] = num_rotations;
    // The classify_rotation is non-zero only if the text has vertical
    // writing direction.
    (*vertical_writing)[i] = classify_rotation.y() != 0.0f;
    ++i;
  }
}

void TessBaseAPI::DetectParagraphs(bool after_text_recognition) {
  int debug_level = 0;
  GetIntVariable("paragraph_debug_level", &debug_level);
  if (paragraph_models_ == nullptr) {
    paragraph_models_ = new std::vector<ParagraphModel *>;
  }
  MutableIterator *result_it = GetMutableIterator();
  do { // Detect paragraphs for this block
    std::vector<ParagraphModel *> models;
    ::tesseract::DetectParagraphs(debug_level, after_text_recognition, result_it, &models);
    paragraph_models_->insert(paragraph_models_->end(), models.begin(), models.end());
  } while (result_it->Next(RIL_BLOCK));
  delete result_it;
}

/** This method returns the string form of the specified unichar. */
const char *TessBaseAPI::GetUnichar(int unichar_id) const {
  return tesseract_->unicharset.id_to_unichar(unichar_id);
}

/** Return the pointer to the i-th dawg loaded into tesseract_ object. */
const Dawg *TessBaseAPI::GetDawg(int i) const {
  if (tesseract_ == nullptr || i >= NumDawgs()) {
    return nullptr;
  }
  return tesseract_->getDict().GetDawg(i);
}

/** Return the number of dawgs loaded into tesseract_ object. */
int TessBaseAPI::NumDawgs() const {
  return tesseract_ == nullptr ? 0 : tesseract_->getDict().NumDawgs();
}

/** Escape a char string - remove <>&"' with HTML codes. */
std::string HOcrEscape(const char *text) {
  std::string ret;
  const char *ptr;
  for (ptr = text; *ptr; ptr++) {
    switch (*ptr) {
      case '<':
        ret += "&lt;";
        break;
      case '>':
        ret += "&gt;";
        break;
      case '&':
        ret += "&amp;";
        break;
      case '"':
        ret += "&quot;";
        break;
      case '\'':
        ret += "&#39;";
        break;
      default:
        ret += *ptr;
    }
  }
  return ret;
}

} // namespace tesseract
