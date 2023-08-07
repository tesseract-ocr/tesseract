// SPDX-License-Identifier: Apache-2.0
// File:        baseapi.h
// Description: Simple API for calling tesseract.
// Author:      Ray Smith
//
// (C) Copyright 2006, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TESSERACT_API_BASEAPI_H_
#define TESSERACT_API_BASEAPI_H_

#ifdef HAVE_CONFIG_H
#  include "config_auto.h" // DISABLED_LEGACY_ENGINE
#endif

#include "export.h"
#include "pageiterator.h"
#include "publictypes.h"
#include "resultiterator.h"
#include "unichar.h"

#include <tesseract/version.h>

#include <cstdio>
#include <vector> // for std::vector

struct Pix;
struct Pixa;
struct Boxa;

namespace tesseract {

class PAGE_RES;
class ParagraphModel;
class BLOCK_LIST;
class ETEXT_DESC;
struct OSResults;
class UNICHARSET;

class Dawg;
class Dict;
class EquationDetect;
class PageIterator;
class ImageThresholder;
class LTRResultIterator;
class ResultIterator;
class MutableIterator;
class TessResultRenderer;
class Tesseract;

// Function to read a std::vector<char> from a whole file.
// Returns false on failure.
using FileReader = bool (*)(const char *filename, std::vector<char> *data);

using DictFunc = int (Dict::*)(void *, const UNICHARSET &, UNICHAR_ID,
                               bool) const;
using ProbabilityInContextFunc = double (Dict::*)(const char *, const char *,
                                                  int, const char *, int);

/**
 * Base class for all tesseract APIs.
 * Specific classes can add ability to work on different inputs or produce
 * different outputs.
 * This class is mostly an interface layer on top of the Tesseract instance
 * class to hide the data types so that users of this class don't have to
 * include any other Tesseract headers.
 */
class TESS_API TessBaseAPI {
public:
  TessBaseAPI();
  virtual ~TessBaseAPI();
  // Copy constructor and assignment operator are currently unsupported.
  TessBaseAPI(TessBaseAPI const &) = delete;
  TessBaseAPI &operator=(TessBaseAPI const &) = delete;

  /**
   * Returns the version identifier as a static string. Do not delete.
   */
  static const char *Version();

  /**
   * If compiled with OpenCL AND an available OpenCL
   * device is deemed faster than serial code, then
   * "device" is populated with the cl_device_id
   * and returns sizeof(cl_device_id)
   * otherwise *device=nullptr and returns 0.
   */
  static size_t getOpenCLDevice(void **device);

  /**
   * Set the name of the input file. Needed for training and
   * reading a UNLV zone file, and for searchable PDF output.
   */
  void SetInputName(const char *name);
  /**
   * These functions are required for searchable PDF output.
   * We need our hands on the input file so that we can include
   * it in the PDF without transcoding. If that is not possible,
   * we need the original image. Finally, resolution metadata
   * is stored in the PDF so we need that as well.
   */
  const char *GetInputName();
  // Takes ownership of the input pix.
  void SetInputImage(Pix *pix);
  Pix *GetInputImage();
  int GetSourceYResolution();
  const char *GetDatapath();

  /** Set the name of the bonus output files. Needed only for debugging. */
  void SetOutputName(const char *name);

  /**
   * Set the value of an internal "parameter."
   * Supply the name of the parameter and the value as a string, just as
   * you would in a config file.
   * Returns false if the name lookup failed.
   * Eg SetVariable("tessedit_char_blacklist", "xyz"); to ignore x, y and z.
   * Or SetVariable("classify_bln_numeric_mode", "1"); to set numeric-only mode.
   * SetVariable may be used before Init, but settings will revert to
   * defaults on End().
   *
   * Note: Must be called after Init(). Only works for non-init variables
   * (init variables should be passed to Init()).
   */
  bool SetVariable(const char *name, const char *value);
  bool SetDebugVariable(const char *name, const char *value);

  /**
   * Returns true if the parameter was found among Tesseract parameters.
   * Fills in value with the value of the parameter.
   */
  bool GetIntVariable(const char *name, int *value) const;
  bool GetBoolVariable(const char *name, bool *value) const;
  bool GetDoubleVariable(const char *name, double *value) const;

  /**
   * Returns the pointer to the string that represents the value of the
   * parameter if it was found among Tesseract parameters.
   */
  const char *GetStringVariable(const char *name) const;

#ifndef DISABLED_LEGACY_ENGINE

  /**
   * Print Tesseract fonts table to the given file.
   */
  void PrintFontsTable(FILE *fp) const;

#endif

  /**
   * Print Tesseract parameters to the given file.
   */
  void PrintVariables(FILE *fp) const;

  /**
   * Get value of named variable as a string, if it exists.
   */
  bool GetVariableAsString(const char *name, std::string *val) const;

  /**
   * Instances are now mostly thread-safe and totally independent,
   * but some global parameters remain. Basically it is safe to use multiple
   * TessBaseAPIs in different threads in parallel, UNLESS:
   * you use SetVariable on some of the Params in classify and textord.
   * If you do, then the effect will be to change it for all your instances.
   *
   * Start tesseract. Returns zero on success and -1 on failure.
   * NOTE that the only members that may be called before Init are those
   * listed above here in the class definition.
   *
   * The datapath must be the name of the tessdata directory.
   * The language is (usually) an ISO 639-3 string or nullptr will default to
   * eng. It is entirely safe (and eventually will be efficient too) to call
   * Init multiple times on the same instance to change language, or just
   * to reset the classifier.
   * The language may be a string of the form [~]<lang>[+[~]<lang>]* indicating
   * that multiple languages are to be loaded. Eg hin+eng will load Hindi and
   * English. Languages may specify internally that they want to be loaded
   * with one or more other languages, so the ~ sign is available to override
   * that. Eg if hin were set to load eng by default, then hin+~eng would force
   * loading only hin. The number of loaded languages is limited only by
   * memory, with the caveat that loading additional languages will impact
   * both speed and accuracy, as there is more work to do to decide on the
   * applicable language, and there is more chance of hallucinating incorrect
   * words.
   * WARNING: On changing languages, all Tesseract parameters are reset
   * back to their default values. (Which may vary between languages.)
   * If you have a rare need to set a Variable that controls
   * initialization for a second call to Init you should explicitly
   * call End() and then use SetVariable before Init. This is only a very
   * rare use case, since there are very few uses that require any parameters
   * to be set before Init.
   *
   * If set_only_non_debug_params is true, only params that do not contain
   * "debug" in the name will be set.
   */
  int Init(const char *datapath, const char *language, OcrEngineMode mode,
           char **configs, int configs_size,
           const std::vector<std::string> *vars_vec,
           const std::vector<std::string> *vars_values,
           bool set_only_non_debug_params);
  int Init(const char *datapath, const char *language, OcrEngineMode oem) {
    return Init(datapath, language, oem, nullptr, 0, nullptr, nullptr, false);
  }
  int Init(const char *datapath, const char *language) {
    return Init(datapath, language, OEM_DEFAULT, nullptr, 0, nullptr, nullptr,
                false);
  }
  // In-memory version reads the traineddata file directly from the given
  // data[data_size] array, and/or reads data via a FileReader.
  int Init(const char *data, int data_size, const char *language,
           OcrEngineMode mode, char **configs, int configs_size,
           const std::vector<std::string> *vars_vec,
           const std::vector<std::string> *vars_values,
           bool set_only_non_debug_params, FileReader reader);

  /**
   * Returns the languages string used in the last valid initialization.
   * If the last initialization specified "deu+hin" then that will be
   * returned. If hin loaded eng automatically as well, then that will
   * not be included in this list. To find the languages actually
   * loaded use GetLoadedLanguagesAsVector.
   * The returned string should NOT be deleted.
   */
  const char *GetInitLanguagesAsString() const;

  /**
   * Returns the loaded languages in the vector of std::string.
   * Includes all languages loaded by the last Init, including those loaded
   * as dependencies of other loaded languages.
   */
  void GetLoadedLanguagesAsVector(std::vector<std::string> *langs) const;

  /**
   * Returns the available languages in the sorted vector of std::string.
   */
  void GetAvailableLanguagesAsVector(std::vector<std::string> *langs) const;

  /**
   * Init only for page layout analysis. Use only for calls to SetImage and
   * AnalysePage. Calls that attempt recognition will generate an error.
   */
  void InitForAnalysePage();

  /**
   * Read a "config" file containing a set of param, value pairs.
   * Searches the standard places: tessdata/configs, tessdata/tessconfigs
   * and also accepts a relative or absolute path name.
   * Note: only non-init params will be set (init params are set by Init()).
   */
  void ReadConfigFile(const char *filename);
  /** Same as above, but only set debug params from the given config file. */
  void ReadDebugConfigFile(const char *filename);

  /**
   * Set the current page segmentation mode. Defaults to PSM_SINGLE_BLOCK.
   * The mode is stored as an IntParam so it can also be modified by
   * ReadConfigFile or SetVariable("tessedit_pageseg_mode", mode as string).
   */
  void SetPageSegMode(PageSegMode mode);

  /** Return the current page segmentation mode. */
  PageSegMode GetPageSegMode() const;

  /**
   * Recognize a rectangle from an image and return the result as a string.
   * May be called many times for a single Init.
   * Currently has no error checking.
   * Greyscale of 8 and color of 24 or 32 bits per pixel may be given.
   * Palette color images will not work properly and must be converted to
   * 24 bit.
   * Binary images of 1 bit per pixel may also be given but they must be
   * byte packed with the MSB of the first byte being the first pixel, and a
   * 1 represents WHITE. For binary images set bytes_per_pixel=0.
   * The recognized text is returned as a char* which is coded
   * as UTF8 and must be freed with the delete [] operator.
   *
   * Note that TesseractRect is the simplified convenience interface.
   * For advanced uses, use SetImage, (optionally) SetRectangle, Recognize,
   * and one or more of the Get*Text functions below.
   */
  char *TesseractRect(const unsigned char *imagedata, int bytes_per_pixel,
                      int bytes_per_line, int left, int top, int width,
                      int height);

  /**
   * Call between pages or documents etc to free up memory and forget
   * adaptive data.
   */
  void ClearAdaptiveClassifier();

  /**
   * @defgroup AdvancedAPI Advanced API
   * The following methods break TesseractRect into pieces, so you can
   * get hold of the thresholded image, get the text in different formats,
   * get bounding boxes, confidences etc.
   */
  /* @{ */

  /**
   * Provide an image for Tesseract to recognize. Format is as
   * TesseractRect above. Copies the image buffer and converts to Pix.
   * SetImage clears all recognition results, and sets the rectangle to the
   * full image, so it may be followed immediately by a GetUTF8Text, and it
   * will automatically perform recognition.
   */
  void SetImage(const unsigned char *imagedata, int width, int height,
                int bytes_per_pixel, int bytes_per_line);

  /**
   * Provide an image for Tesseract to recognize. As with SetImage above,
   * Tesseract takes its own copy of the image, so it need not persist until
   * after Recognize.
   * Pix vs raw, which to use?
   * Use Pix where possible. Tesseract uses Pix as its internal representation
   * and it is therefore more efficient to provide a Pix directly.
   */
  void SetImage(Pix *pix);

  /**
   * Set the resolution of the source image in pixels per inch so font size
   * information can be calculated in results.  Call this after SetImage().
   */
  void SetSourceResolution(int ppi);

  /**
   * Restrict recognition to a sub-rectangle of the image. Call after SetImage.
   * Each SetRectangle clears the recogntion results so multiple rectangles
   * can be recognized with the same image.
   */
  void SetRectangle(int left, int top, int width, int height);

  /**
   * Get a copy of the internal thresholded image from Tesseract.
   * Caller takes ownership of the Pix and must pixDestroy it.
   * May be called any time after SetImage, or after TesseractRect.
   */
  Pix *GetThresholdedImage();

  /**
   * Get the result of page layout analysis as a leptonica-style
   * Boxa, Pixa pair, in reading order.
   * Can be called before or after Recognize.
   */
  Boxa *GetRegions(Pixa **pixa);

  /**
   * Get the textlines as a leptonica-style
   * Boxa, Pixa pair, in reading order.
   * Can be called before or after Recognize.
   * If raw_image is true, then extract from the original image instead of the
   * thresholded image and pad by raw_padding pixels.
   * If blockids is not nullptr, the block-id of each line is also returned as
   * an array of one element per line. delete [] after use. If paraids is not
   * nullptr, the paragraph-id of each line within its block is also returned as
   * an array of one element per line. delete [] after use.
   */
  Boxa *GetTextlines(bool raw_image, int raw_padding, Pixa **pixa,
                     int **blockids, int **paraids);
  /*
   Helper method to extract from the thresholded image. (most common usage)
*/
  Boxa *GetTextlines(Pixa **pixa, int **blockids) {
    return GetTextlines(false, 0, pixa, blockids, nullptr);
  }

  /**
   * Get textlines and strips of image regions as a leptonica-style Boxa, Pixa
   * pair, in reading order. Enables downstream handling of non-rectangular
   * regions.
   * Can be called before or after Recognize.
   * If blockids is not nullptr, the block-id of each line is also returned as
   * an array of one element per line. delete [] after use.
   */
  Boxa *GetStrips(Pixa **pixa, int **blockids);

  /**
   * Get the words as a leptonica-style
   * Boxa, Pixa pair, in reading order.
   * Can be called before or after Recognize.
   */
  Boxa *GetWords(Pixa **pixa);

  /**
   * Gets the individual connected (text) components (created
   * after pages segmentation step, but before recognition)
   * as a leptonica-style Boxa, Pixa pair, in reading order.
   * Can be called before or after Recognize.
   * Note: the caller is responsible for calling boxaDestroy()
   * on the returned Boxa array and pixaDestroy() on cc array.
   */
  Boxa *GetConnectedComponents(Pixa **cc);

  /**
   * Get the given level kind of components (block, textline, word etc.) as a
   * leptonica-style Boxa, Pixa pair, in reading order.
   * Can be called before or after Recognize.
   * If blockids is not nullptr, the block-id of each component is also returned
   * as an array of one element per component. delete [] after use.
   * If blockids is not nullptr, the paragraph-id of each component with its
   * block is also returned as an array of one element per component. delete []
   * after use. If raw_image is true, then portions of the original image are
   * extracted instead of the thresholded image and padded with raw_padding. If
   * text_only is true, then only text components are returned.
   */
  Boxa *GetComponentImages(PageIteratorLevel level, bool text_only,
                           bool raw_image, int raw_padding, Pixa **pixa,
                           int **blockids, int **paraids);
  // Helper function to get binary images with no padding (most common usage).
  Boxa *GetComponentImages(const PageIteratorLevel level, const bool text_only,
                           Pixa **pixa, int **blockids) {
    return GetComponentImages(level, text_only, false, 0, pixa, blockids,
                              nullptr);
  }

  /**
   * Returns the scale factor of the thresholded image that would be returned by
   * GetThresholdedImage() and the various GetX() methods that call
   * GetComponentImages().
   * Returns 0 if no thresholder has been set.
   */
  int GetThresholdedImageScaleFactor() const;

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
  PageIterator *AnalyseLayout();
  PageIterator *AnalyseLayout(bool merge_similar_words);

  /**
   * Recognize the image from SetAndThresholdImage, generating Tesseract
   * internal structures. Returns 0 on success.
   * Optional. The Get*Text functions below will call Recognize if needed.
   * After Recognize, the output is kept internally until the next SetImage.
   */
  int Recognize(ETEXT_DESC *monitor);

  /**
   * Methods to retrieve information after SetAndThresholdImage(),
   * Recognize() or TesseractRect(). (Recognize is called implicitly if needed.)
   */

  /**
   * Turns images into symbolic text.
   *
   * filename can point to a single image, a multi-page TIFF,
   * or a plain text list of image filenames.
   *
   * retry_config is useful for debugging. If not nullptr, you can fall
   * back to an alternate configuration if a page fails for some
   * reason.
   *
   * timeout_millisec terminates processing if any single page
   * takes too long. Set to 0 for unlimited time.
   *
   * renderer is responible for creating the output. For example,
   * use the TessTextRenderer if you want plaintext output, or
   * the TessPDFRender to produce searchable PDF.
   *
   * If tessedit_page_number is non-negative, will only process that
   * single page. Works for multi-page tiff file, or filelist.
   *
   * Returns true if successful, false on error.
   */
  bool ProcessPages(const char *filename, const char *retry_config,
                    int timeout_millisec, TessResultRenderer *renderer);
  // Does the real work of ProcessPages.
  bool ProcessPagesInternal(const char *filename, const char *retry_config,
                            int timeout_millisec, TessResultRenderer *renderer);

  /**
   * Turn a single image into symbolic text.
   *
   * The pix is the image processed. filename and page_index are
   * metadata used by side-effect processes, such as reading a box
   * file or formatting as hOCR.
   *
   * See ProcessPages for descriptions of other parameters.
   */
  bool ProcessPage(Pix *pix, int page_index, const char *filename,
                   const char *retry_config, int timeout_millisec,
                   TessResultRenderer *renderer);

  /**
   * Get a reading-order iterator to the results of LayoutAnalysis and/or
   * Recognize. The returned iterator must be deleted after use.
   * WARNING! This class points to data held within the TessBaseAPI class, and
   * therefore can only be used while the TessBaseAPI class still exists and
   * has not been subjected to a call of Init, SetImage, Recognize, Clear, End
   * DetectOS, or anything else that changes the internal PAGE_RES.
   */
  ResultIterator *GetIterator();

  /**
   * Get a mutable iterator to the results of LayoutAnalysis and/or Recognize.
   * The returned iterator must be deleted after use.
   * WARNING! This class points to data held within the TessBaseAPI class, and
   * therefore can only be used while the TessBaseAPI class still exists and
   * has not been subjected to a call of Init, SetImage, Recognize, Clear, End
   * DetectOS, or anything else that changes the internal PAGE_RES.
   */
  MutableIterator *GetMutableIterator();

  /**
   * The recognized text is returned as a char* which is coded
   * as UTF8 and must be freed with the delete [] operator.
   */
  char *GetUTF8Text();

  /**
   * Make a HTML-formatted string with hOCR markup from the internal
   * data structures.
   * page_number is 0-based but will appear in the output as 1-based.
   * monitor can be used to
   *  cancel the recognition
   *  receive progress callbacks
   * Returned string must be freed with the delete [] operator.
   */
  char *GetHOCRText(ETEXT_DESC *monitor, int page_number);

  /**
   * Make a HTML-formatted string with hOCR markup from the internal
   * data structures.
   * page_number is 0-based but will appear in the output as 1-based.
   * Returned string must be freed with the delete [] operator.
   */
  char *GetHOCRText(int page_number);

  /**
   * Make an XML-formatted string with Alto markup from the internal
   * data structures.
   */
  char *GetAltoText(ETEXT_DESC *monitor, int page_number);

  /**
   * Make an XML-formatted string with Alto markup from the internal
   * data structures.
   */
  char *GetAltoText(int page_number);

  /**
   * Make a TSV-formatted string from the internal data structures.
   * page_number is 0-based but will appear in the output as 1-based.
   * Returned string must be freed with the delete [] operator.
   */
  char *GetTSVText(int page_number);

  /**
   * Make a box file for LSTM training from the internal data structures.
   * Constructs coordinates in the original image - not just the rectangle.
   * page_number is a 0-based page index that will appear in the box file.
   * Returned string must be freed with the delete [] operator.
   */
  char *GetLSTMBoxText(int page_number);

  /**
   * The recognized text is returned as a char* which is coded in the same
   * format as a box file used in training.
   * Constructs coordinates in the original image - not just the rectangle.
   * page_number is a 0-based page index that will appear in the box file.
   * Returned string must be freed with the delete [] operator.
   */
  char *GetBoxText(int page_number);

  /**
   * The recognized text is returned as a char* which is coded in the same
   * format as a WordStr box file used in training.
   * page_number is a 0-based page index that will appear in the box file.
   * Returned string must be freed with the delete [] operator.
   */
  char *GetWordStrBoxText(int page_number);

  /**
   * The recognized text is returned as a char* which is coded
   * as UNLV format Latin-1 with specific reject and suspect codes.
   * Returned string must be freed with the delete [] operator.
   */
  char *GetUNLVText();

  /**
   * Detect the orientation of the input image and apparent script (alphabet).
   * orient_deg is the detected clockwise rotation of the input image in degrees
   * (0, 90, 180, 270)
   * orient_conf is the confidence (15.0 is reasonably confident)
   * script_name is an ASCII string, the name of the script, e.g. "Latin"
   * script_conf is confidence level in the script
   * Returns true on success and writes values to each parameter as an output
   */
  bool DetectOrientationScript(int *orient_deg, float *orient_conf,
                               const char **script_name, float *script_conf);

  /**
   * The recognized text is returned as a char* which is coded
   * as UTF8 and must be freed with the delete [] operator.
   * page_number is a 0-based page index that will appear in the osd file.
   */
  char *GetOsdText(int page_number);

  /** Returns the (average) confidence value between 0 and 100. */
  int MeanTextConf();
  /**
   * Returns all word confidences (between 0 and 100) in an array, terminated
   * by -1.  The calling function must delete [] after use.
   * The number of confidences should correspond to the number of space-
   * delimited words in GetUTF8Text.
   */
  int *AllWordConfidences();

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
  bool AdaptToWordStr(PageSegMode mode, const char *wordstr);
#endif //  ndef DISABLED_LEGACY_ENGINE

  /**
   * Free up recognition results and any stored image data, without actually
   * freeing any recognition data that would be time-consuming to reload.
   * Afterwards, you must call SetImage or TesseractRect before doing
   * any Recognize or Get* operation.
   */
  void Clear();

  /**
   * Close down tesseract and free up all memory. End() is equivalent to
   * destructing and reconstructing your TessBaseAPI.
   * Once End() has been used, none of the other API functions may be used
   * other than Init and anything declared above it in the class definition.
   */
  void End();

  /**
   * Clear any library-level memory caches.
   * There are a variety of expensive-to-load constant data structures (mostly
   * language dictionaries) that are cached globally -- surviving the Init()
   * and End() of individual TessBaseAPI's.  This function allows the clearing
   * of these caches.
   **/
  static void ClearPersistentCache();

  /**
   * Check whether a word is valid according to Tesseract's language model
   * @return 0 if the word is invalid, non-zero if valid.
   * @warning temporary! This function will be removed from here and placed
   * in a separate API at some future time.
   */
  int IsValidWord(const char *word) const;
  // Returns true if utf8_character is defined in the UniCharset.
  bool IsValidCharacter(const char *utf8_character) const;

  bool GetTextDirection(int *out_offset, float *out_slope);

  /** Sets Dict::letter_is_okay_ function to point to the given function. */
  void SetDictFunc(DictFunc f);

  /** Sets Dict::probability_in_context_ function to point to the given
   * function.
   */
  void SetProbabilityInContextFunc(ProbabilityInContextFunc f);

  /**
   * Estimates the Orientation And Script of the image.
   * @return true if the image was processed successfully.
   */
  bool DetectOS(OSResults *);

  /**
   * Return text orientation of each block as determined by an earlier run
   * of layout analysis.
   */
  void GetBlockTextOrientations(int **block_orientation,
                                bool **vertical_writing);

  /** This method returns the string form of the specified unichar. */
  const char *GetUnichar(int unichar_id) const;

  /** Return the pointer to the i-th dawg loaded into tesseract_ object. */
  const Dawg *GetDawg(int i) const;

  /** Return the number of dawgs loaded into tesseract_ object. */
  int NumDawgs() const;

  Tesseract *tesseract() const {
    return tesseract_;
  }

  OcrEngineMode oem() const {
    return last_oem_requested_;
  }

  void set_min_orientation_margin(double margin);
  /* @} */

protected:
  /** Common code for setting the image. Returns true if Init has been called.
   */
  bool InternalSetImage();

  /**
   * Run the thresholder to make the thresholded image. If pix is not nullptr,
   * the source is thresholded to pix instead of the internal IMAGE.
   */
  virtual bool Threshold(Pix **pix);

  /**
   * Find lines from the image making the BLOCK_LIST.
   * @return 0 on success.
   */
  int FindLines();

  /** Delete the pageres and block list ready for a new page. */
  void ClearResults();

  /**
   * Return an LTR Result Iterator -- used only for training, as we really want
   * to ignore all BiDi smarts at that point.
   * delete once you're done with it.
   */
  LTRResultIterator *GetLTRIterator();

  /**
   * Return the length of the output text string, as UTF8, assuming
   * one newline per line and one per block, with a terminator,
   * and assuming a single character reject marker for each rejected character.
   * Also return the number of recognized blobs in blob_count.
   */
  int TextLength(int *blob_count) const;

  //// paragraphs.cpp ////////////////////////////////////////////////////
  void DetectParagraphs(bool after_text_recognition);

  const PAGE_RES *GetPageRes() const {
    return page_res_;
  }

protected:
  Tesseract *tesseract_;          ///< The underlying data object.
  Tesseract *osd_tesseract_;      ///< For orientation & script detection.
  EquationDetect *equ_detect_;    ///< The equation detector.
  FileReader reader_;             ///< Reads files from any filesystem.
  ImageThresholder *thresholder_; ///< Image thresholding module.
  std::vector<ParagraphModel *> *paragraph_models_;
  BLOCK_LIST *block_list_;           ///< The page layout.
  PAGE_RES *page_res_;               ///< The page-level data.
  std::string input_file_;           ///< Name used by training code.
  std::string output_file_;          ///< Name used by debug code.
  std::string datapath_;             ///< Current location of tessdata.
  std::string language_;             ///< Last initialized language.
  OcrEngineMode last_oem_requested_; ///< Last ocr language mode requested.
  bool recognition_done_;            ///< page_res_ contains recognition data.

  /**
   * @defgroup ThresholderParams Thresholder Parameters
   * Parameters saved from the Thresholder. Needed to rebuild coordinates.
   */
  /* @{ */
  int rect_left_;
  int rect_top_;
  int rect_width_;
  int rect_height_;
  int image_width_;
  int image_height_;
  /* @} */

private:
  // A list of image filenames gets special consideration
  bool ProcessPagesFileList(FILE *fp, std::string *buf,
                            const char *retry_config, int timeout_millisec,
                            TessResultRenderer *renderer,
                            int tessedit_page_number);
  // TIFF supports multipage so gets special consideration.
  bool ProcessPagesMultipageTiff(const unsigned char *data, size_t size,
                                 const char *filename, const char *retry_config,
                                 int timeout_millisec,
                                 TessResultRenderer *renderer,
                                 int tessedit_page_number);
}; // class TessBaseAPI.

/** Escape a char string - replace &<>"' with HTML codes. */
std::string HOcrEscape(const char *text);

} // namespace tesseract

#endif // TESSERACT_API_BASEAPI_H_
