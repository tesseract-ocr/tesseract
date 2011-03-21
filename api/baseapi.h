///////////////////////////////////////////////////////////////////////
// File:        baseapi.h
// Description: Simple API for calling tesseract.
// Author:      Ray Smith
// Created:     Fri Oct 06 15:35:01 PDT 2006
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
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_API_BASEAPI_H__
#define TESSERACT_API_BASEAPI_H__

// To avoid collision with other typenames include the ABSOLUTE MINIMUM
// complexity of includes here. Use forward declarations wherever possible
// and hide includes of complex types in baseapi.cpp.
#include "apitypes.h"
#include "genericvector.h"
#include "thresholder.h"
#include "unichar.h"
#include "tesscallback.h"

class PAGE_RES;
class PAGE_RES_IT;
class BLOCK_LIST;
class DENORM;
class IMAGE;
class PBLOB;
class ROW;
class STRING;
class WERD;
struct Pix;
struct Box;
struct Pixa;
struct Boxa;
class ETEXT_DESC;
struct OSResults;
class TBOX;

#define MAX_NUM_INT_FEATURES 512
struct INT_FEATURE_STRUCT;
typedef INT_FEATURE_STRUCT *INT_FEATURE;
typedef INT_FEATURE_STRUCT INT_FEATURE_ARRAY[MAX_NUM_INT_FEATURES];
struct TBLOB;

#ifdef TESSDLL_EXPORTS
#define TESSDLL_API __declspec(dllexport)
#elif defined(TESSDLL_IMPORTS)
#define TESSDLL_API __declspec(dllimport)
#else
#define TESSDLL_API
#endif


namespace tesseract {

class CubeRecoContext;
class Dawg;
class Dict;
class PageIterator;
class ResultIterator;
class Tesseract;
class Trie;

typedef int (Dict::*DictFunc)(void* void_dawg_args,
                              UNICHAR_ID unichar_id, bool word_end);
typedef double (Dict::*ProbabilityInContextFunc)(const char* lang,
                                                 const char* context,
                                                 int context_bytes,
                                                 const char* character,
                                                 int character_bytes);
typedef TessCallback2<int, PAGE_RES *> TruthCallback;

/**
 * Base class for all tesseract APIs.
 * Specific classes can add ability to work on different inputs or produce
 * different outputs.
 * This class is mostly an interface layer on top of the Tesseract instance
 * class to hide the data types so that users of this class don't have to
 * include any other Tesseract headers.
 */
class TESSDLL_API TessBaseAPI {
 public:
  TessBaseAPI();
  virtual ~TessBaseAPI();

  /**
   * Returns the version identifier as a static string. Do not delete.
   */
  static const char* Version();

  /**
   * Set the name of the input file. Needed only for training and
   * reading a UNLV zone file.
   */
  void SetInputName(const char* name);

  /** Set the name of the bonus output files. Needed only for debugging. */
  void SetOutputName(const char* name);

  /**
   * Set the value of an internal "parameter."
   * Supply the name of the parameter and the value as a string, just as
   * you would in a config file.
   * Returns false if the name lookup failed.
   * Eg SetVariable("tessedit_char_blacklist", "xyz"); to ignore x, y and z.
   * Or SetVariable("bln_numericmode", "1"); to set numeric-only mode.
   * SetVariable may be used before Init, but settings will revert to
   * defaults on End().
   * TODO(rays) Add a command-line option to dump the parameters to stdout
   * and add a pointer to it in the FAQ
   *
   * Note: Must be called after Init().
   */
  bool SetVariable(const char* name, const char* value);

  // Returns true if the parameter was found among Tesseract parameters.
  // Fills in value with the value of the parameter.
  bool GetIntVariable(const char *name, int *value) const;
  bool GetBoolVariable(const char *name, bool *value) const;
  bool GetDoubleVariable(const char *name, double *value) const;
  // Returns the pointer to the string that represents the value of the
  // parameter if it was found among Tesseract parameters.
  const char *GetStringVariable(const char *name) const;

  // Print Tesseract parameters to the given file.
  void PrintVariables(FILE *fp) const;
  // Get value of named variable as a string, if it exists.
  bool GetVariableAsString(const char *name, STRING *val);

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
   * The datapath must be the name of the parent directory of tessdata and
   * must end in / . Any name after the last / will be stripped.
   * The language is (usually) an ISO 639-3 string or NULL will default to eng.
   * It is entirely safe (and eventually will be efficient too) to call
   * Init multiple times on the same instance to change language, or just
   * to reset the classifier.
   * WARNING: On changing languages, all Tesseract parameters are reset
   * back to their default values. (Which may vary between languages.)
   * If you have a rare need to set a Variable that controls
   * initialization for a second call to Init you should explicitly
   * call End() and then use SetVariable before Init. This is only a very
   * rare use case, since there are very few uses that require any parameters
   * to be set before Init.
   */
  int Init(const char* datapath, const char* language, OcrEngineMode mode,
           char **configs, int configs_size,
           const GenericVector<STRING> *vars_vec,
           const GenericVector<STRING> *vars_values,
           bool set_only_init_params);
  int Init(const char* datapath, const char* language, OcrEngineMode oem) {
    return Init(datapath, language, oem, NULL, 0, NULL, NULL, false);
  }
  int Init(const char* datapath, const char* language) {
    return Init(datapath, language, OEM_DEFAULT, NULL, 0, NULL, NULL, false);
  }

  /**
   * Init only the lang model component of Tesseract. The only functions
   * that work after this init are SetVariable and IsValidWord.
   * WARNING: temporary! This function will be removed from here and placed
   * in a separate API at some future time.
   */
  int InitLangMod(const char* datapath, const char* language);

  // Init only for page layout analysis. Use only for calls to SetImage and
  // AnalysePage. Calls that attempt recognition will generate an error.
  void InitForAnalysePage();

  /**
   * Read a "config" file containing a set of variable, value pairs.
   * Searches the standard places: tessdata/configs, tessdata/tessconfigs
   * and also accepts a relative or absolute path name.
   * If init_only is true, only sets the parameters marked with a special
   * INIT flag, which are typically of functional/algorithmic effect
   * rather than debug effect. Used to separate debug settings from
   * working settings.
   */
  void ReadConfigFile(const char* filename, bool init_only);

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
  char* TesseractRect(const unsigned char* imagedata,
                      int bytes_per_pixel, int bytes_per_line,
                      int left, int top, int width, int height);

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
   * TesseractRect above. Does not copy the image buffer, or take
   * ownership. The source image may be destroyed after Recognize is called,
   * either explicitly or implicitly via one of the Get*Text functions.
   * SetImage clears all recognition results, and sets the rectangle to the
   * full image, so it may be followed immediately by a GetUTF8Text, and it
   * will automatically perform recognition.
   */
  void SetImage(const unsigned char* imagedata, int width, int height,
                int bytes_per_pixel, int bytes_per_line);

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
  void SetImage(const Pix* pix);

  /**
   * Restrict recognition to a sub-rectangle of the image. Call after SetImage.
   * Each SetRectangle clears the recogntion results so multiple rectangles
   * can be recognized with the same image.
   */
  void SetRectangle(int left, int top, int width, int height);

  /**
   * In extreme cases only, usually with a subclass of Thresholder, it
   * is possible to provide a different Thresholder. The Thresholder may
   * be preloaded with an image, settings etc, or they may be set after.
   * Note that Tesseract takes ownership of the Thresholder and will
   * delete it when it it is replaced or the API is destructed.
   */
  void SetThresholder(ImageThresholder* thresholder) {
    if (thresholder_ != NULL)
      delete thresholder_;
    thresholder_ = thresholder;
    ClearResults();
  }

  /**
   * Get a copy of the internal thresholded image from Tesseract.
   * Caller takes ownership of the Pix and must pixDestroy it.
   * May be called any time after SetImage, or after TesseractRect.
   */
  Pix* GetThresholdedImage();

  /**
   * Get the result of page layout analysis as a leptonica-style
   * Boxa, Pixa pair, in reading order.
   * Can be called before or after Recognize.
   */
  Boxa* GetRegions(Pixa** pixa);

  /**
   * Get the textlines as a leptonica-style
   * Boxa, Pixa pair, in reading order.
   * Can be called before or after Recognize.
   * If blockids is not NULL, the block-id of each line is also returned
   * as an array of one element per line. delete [] after use.
   */
  Boxa* GetTextlines(Pixa** pixa, int** blockids);

  /**
   * Get the words as a leptonica-style
   * Boxa, Pixa pair, in reading order.
   * Can be called before or after Recognize.
   */
  Boxa* GetWords(Pixa** pixa);

  // Gets the individual connected (text) components (created
  // after pages segmentation step, but before recognition)
  // as a leptonica-style Boxa, Pixa pair, in reading order.
  // Can be called before or after Recognize.
  // Note: the caller is responsible for calling boxaDestroy()
  // on the returned Boxa array and pixaDestroy() on cc array.
  Boxa* GetConnectedComponents(Pixa** cc);

  // Get the given level kind of components (block, textline, word etc.) as a
  // leptonica-style Boxa, Pixa pair, in reading order.
  // Can be called before or after Recognize.
  // If blockids is not NULL, the block-id of each component is also returned
  // as an array of one element per component. delete [] after use.
  Boxa* GetComponentImages(PageIteratorLevel level,
                           Pixa** pixa, int** blockids);

  /**
   * Dump the internal binary image to a PGM file.
   * @deprecated Use GetThresholdedImage and write the image using pixWrite
   * instead if possible.
   */
  void DumpPGM(const char* filename);

  // Runs page layout analysis in the mode set by SetPageSegMode.
  // May optionally be called prior to Recognize to get access to just
  // the page layout results. Returns an iterator to the results.
  // Returns NULL on error.
  // The returned iterator must be deleted after use.
  // WARNING! This class points to data held within the TessBaseAPI class, and
  // therefore can only be used while the TessBaseAPI class still exists and
  // has not been subjected to a call of Init, SetImage, Recognize, Clear, End
  // DetectOS, or anything else that changes the internal PAGE_RES.
  PageIterator* AnalyseLayout();

  /**
   * Recognize the image from SetAndThresholdImage, generating Tesseract
   * internal structures. Returns 0 on success.
   * Optional. The Get*Text functions below will call Recognize if needed.
   * After Recognize, the output is kept internally until the next SetImage.
   */
  int Recognize(ETEXT_DESC* monitor);

  /**
   * Methods to retrieve information after SetAndThresholdImage(),
   * Recognize() or TesseractRect(). (Recognize is called implicitly if needed.)
   */

  /** Variant on Recognize used for testing chopper. */
  int RecognizeForChopTest(ETEXT_DESC* monitor);

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
  bool ProcessPages(const char* filename,
                    const char* retry_config, int timeout_millisec,
                    STRING* text_out);

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
  bool ProcessPage(Pix* pix, int page_index, const char* filename,
                   const char* retry_config, int timeout_millisec,
                   STRING* text_out);

  // Get an iterator to the results of LayoutAnalysis and/or Recognize.
  // The returned iterator must be deleted after use.
  // WARNING! This class points to data held within the TessBaseAPI class, and
  // therefore can only be used while the TessBaseAPI class still exists and
  // has not been subjected to a call of Init, SetImage, Recognize, Clear, End
  // DetectOS, or anything else that changes the internal PAGE_RES.
  ResultIterator* GetIterator();

  /**
   * The recognized text is returned as a char* which is coded
   * as UTF8 and must be freed with the delete [] operator.
   */
  char* GetUTF8Text();
  /**
   * Make a HTML-formatted string with hOCR markup from the internal
   * data structures.
   * page_number is 0-based but will appear in the output as 1-based.
   */
  char* GetHOCRText(int page_number);
  /**
   * The recognized text is returned as a char* which is coded in the same
   * format as a box file used in training. Returned string must be freed with
   * the delete [] operator.
   * Constructs coordinates in the original image - not just the rectangle.
   * page_number is a 0-based page index that will appear in the box file.
   */
  char* GetBoxText(int page_number);
  /**
   * The recognized text is returned as a char* which is coded
   * as UNLV format Latin-1 with specific reject and suspect codes
   * and must be freed with the delete [] operator.
   */
  char* GetUNLVText();
  /** Returns the (average) confidence value between 0 and 100. */
  int MeanTextConf();
  /**
   * Returns all word confidences (between 0 and 100) in an array, terminated
   * by -1.  The calling function must delete [] after use.
   * The number of confidences should correspond to the number of space-
   * delimited words in GetUTF8Text.
   */
  int* AllWordConfidences();

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
  bool AdaptToWordStr(PageSegMode mode, const char* wordstr);

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
   * Check whether a word is valid according to Tesseract's language model
   * @return 0 if the word is invalid, non-zero if valid.
   * @warning temporary! This function will be removed from here and placed
   * in a separate API at some future time.
   */
  int IsValidWord(const char *word);

  bool GetTextDirection(int* out_offset, float* out_slope);

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
  bool DetectOS(OSResults*);

  /** This method returns the features associated with the input image. */
  void GetFeaturesForBlob(TBLOB* blob, const DENORM& denorm,
                          INT_FEATURE_ARRAY int_features,
                          int* num_features, int* FeatureOutlineIndex);

  // This method returns the row to which a box of specified dimensions would
  // belong. If no good match is found, it returns NULL.
  static ROW* FindRowForBox(BLOCK_LIST* blocks, int left, int top,
                            int right, int bottom);

  // Method to run adaptive classifier on a blob.
  // It returns at max num_max_matches results.
  void RunAdaptiveClassifier(TBLOB* blob, const DENORM& denorm,
                             int num_max_matches,
                             int* unichar_ids,
                             float* ratings,
                             int* num_matches_returned);

  // This method returns the string form of the specified unichar.
  const char* GetUnichar(int unichar_id);

  /** Return the pointer to the i-th dawg loaded into tesseract_ object. */
  const Dawg *GetDawg(int i) const;

  /** Return the number of dawgs loaded into tesseract_ object. */
  int NumDawgs() const;

  /** Return the language used in the last valid initialization. */
  const char* GetLastInitLanguage() const;

  // Returns a ROW object created from the input row specification.
  static ROW *MakeTessOCRRow(float baseline, float xheight,
                             float descender, float ascender);

  // Returns a TBLOB corresponding to the entire input image.
  static TBLOB *MakeTBLOB(Pix *pix);

  // This method baseline normalizes a TBLOB in-place. The input row is used
  // for normalization. The denorm is an optional parameter in which the
  // normalization-antidote is returned.
  static void NormalizeTBLOB(TBLOB *tblob, ROW *row,
                             bool numeric_mode, DENORM *denorm);

  Tesseract* const tesseract() const {
    return tesseract_;
  }

  void InitTruthCallback(TruthCallback *cb) { truth_cb_ = cb; }

  // Return a pointer to underlying CubeRecoContext object if present.
  CubeRecoContext *GetCubeRecoContext() const;

  void set_min_orientation_margin(double margin);

  // Return text orientation of each block as determined by an earlier run
  // of layout analysis.
  void GetBlockTextOrientations(int** block_orientation,
                                bool** vertical_writing);

  /** Find lines from the image making the BLOCK_LIST. */
  BLOCK_LIST* FindLinesCreateBlockList();

  /**
   * Delete a block list.
   * This is to keep BLOCK_LIST pointer opaque
   * and let go of including the other headers.
   */
  static void DeleteBlockList(BLOCK_LIST* block_list);
 /* @} */

 protected:

  /** Common code for setting the image. Returns true if Init has been called. */
  bool InternalSetImage();

  /**
   * Run the thresholder to make the thresholded image. If pix is not NULL,
   * the source is thresholded to pix instead of the internal IMAGE.
   */
  virtual void Threshold(Pix** pix);

  /**
   * Find lines from the image making the BLOCK_LIST.
   * @return 0 on success.
   */
  int FindLines();

  /** Delete the pageres and block list ready for a new page. */
  void ClearResults();

  /**
   * Return the length of the output text string, as UTF8, assuming
   * one newline per line and one per block, with a terminator,
   * and assuming a single character reject marker for each rejected character.
   * Also return the number of recognized blobs in blob_count.
   */
  int TextLength(int* blob_count);

  /** @defgroup ocropusAddOns ocropus add-ons */
  /* @{ */

  /**
   * Adapt to recognize the current image as the given character.
   * The image must be preloaded and be just an image of a single character.
   */
  void AdaptToCharacter(const char *unichar_repr,
                        int length,
                        float baseline,
                        float xheight,
                        float descender,
                        float ascender);

  /** Recognize text doing one pass only, using settings for a given pass. */
  PAGE_RES* RecognitionPass1(BLOCK_LIST* block_list);
  PAGE_RES* RecognitionPass2(BLOCK_LIST* block_list, PAGE_RES* pass1_result);

  /**
   * Extract the OCR results, costs (penalty points for uncertainty),
   * and the bounding boxes of the characters.
   */
  static int TesseractExtractResult(char** text,
                                    int** lengths,
                                    float** costs,
                                    int** x0,
                                    int** y0,
                                    int** x1,
                                    int** y1,
                                    PAGE_RES* page_res);

  const PAGE_RES* GetPageRes() const {
    return page_res_;
  };

 protected:
  Tesseract*        tesseract_;       ///< The underlying data object.
  Tesseract*        osd_tesseract_;   ///< For orientation & script detection.
  ImageThresholder* thresholder_;     ///< Image thresholding module.
  BLOCK_LIST*       block_list_;      ///< The page layout.
  PAGE_RES*         page_res_;        ///< The page-level data.
  STRING*           input_file_;      ///< Name used by training code.
  STRING*           output_file_;     ///< Name used by debug code.
  STRING*           datapath_;        ///< Current location of tessdata.
  STRING*           language_;        ///< Last initialized language.
  OcrEngineMode last_oem_requested_;  ///< Last ocr language mode requested.
  bool          recognition_done_;   ///< page_res_ contains recognition data.
  TruthCallback *truth_cb_;           /// fxn for setting truth_* in WERD_RES

  /**
   * @defgroup ThresholderParams
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
};

}  // namespace tesseract.

#endif  // TESSERACT_API_BASEAPI_H__
