// SPDX-License-Identifier: Apache-2.0
// File:        capi.h
// Description: C-API TessBaseAPI
//
// (C) Copyright 2012, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef API_CAPI_H_
#define API_CAPI_H_

#include "export.h"

#ifdef __cplusplus
#  include <tesseract/baseapi.h>
#  include <tesseract/ocrclass.h>
#  include <tesseract/pageiterator.h>
#  include <tesseract/renderer.h>
#  include <tesseract/resultiterator.h>
#endif

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BOOL
#  define BOOL int
#  define TRUE 1
#  define FALSE 0
#endif

#ifdef __cplusplus
typedef tesseract::TessResultRenderer TessResultRenderer;
typedef tesseract::TessBaseAPI TessBaseAPI;
typedef tesseract::PageIterator TessPageIterator;
typedef tesseract::ResultIterator TessResultIterator;
typedef tesseract::MutableIterator TessMutableIterator;
typedef tesseract::ChoiceIterator TessChoiceIterator;
typedef tesseract::OcrEngineMode TessOcrEngineMode;
typedef tesseract::PageSegMode TessPageSegMode;
typedef tesseract::PageIteratorLevel TessPageIteratorLevel;
typedef tesseract::Orientation TessOrientation;
typedef tesseract::ParagraphJustification TessParagraphJustification;
typedef tesseract::WritingDirection TessWritingDirection;
typedef tesseract::TextlineOrder TessTextlineOrder;
typedef tesseract::PolyBlockType TessPolyBlockType;
typedef tesseract::ETEXT_DESC ETEXT_DESC;
#else
typedef struct TessResultRenderer TessResultRenderer;
typedef struct TessBaseAPI TessBaseAPI;
typedef struct TessPageIterator TessPageIterator;
typedef struct TessResultIterator TessResultIterator;
typedef struct TessMutableIterator TessMutableIterator;
typedef struct TessChoiceIterator TessChoiceIterator;
typedef enum TessOcrEngineMode {
  OEM_TESSERACT_ONLY,
  OEM_LSTM_ONLY,
  OEM_TESSERACT_LSTM_COMBINED,
  OEM_DEFAULT
} TessOcrEngineMode;
typedef enum TessPageSegMode {
  PSM_OSD_ONLY,
  PSM_AUTO_OSD,
  PSM_AUTO_ONLY,
  PSM_AUTO,
  PSM_SINGLE_COLUMN,
  PSM_SINGLE_BLOCK_VERT_TEXT,
  PSM_SINGLE_BLOCK,
  PSM_SINGLE_LINE,
  PSM_SINGLE_WORD,
  PSM_CIRCLE_WORD,
  PSM_SINGLE_CHAR,
  PSM_SPARSE_TEXT,
  PSM_SPARSE_TEXT_OSD,
  PSM_RAW_LINE,
  PSM_COUNT
} TessPageSegMode;
typedef enum TessPageIteratorLevel {
  RIL_BLOCK,
  RIL_PARA,
  RIL_TEXTLINE,
  RIL_WORD,
  RIL_SYMBOL
} TessPageIteratorLevel;
typedef enum TessPolyBlockType {
  PT_UNKNOWN,
  PT_FLOWING_TEXT,
  PT_HEADING_TEXT,
  PT_PULLOUT_TEXT,
  PT_EQUATION,
  PT_INLINE_EQUATION,
  PT_TABLE,
  PT_VERTICAL_TEXT,
  PT_CAPTION_TEXT,
  PT_FLOWING_IMAGE,
  PT_HEADING_IMAGE,
  PT_PULLOUT_IMAGE,
  PT_HORZ_LINE,
  PT_VERT_LINE,
  PT_NOISE,
  PT_COUNT
} TessPolyBlockType;
typedef enum TessOrientation {
  ORIENTATION_PAGE_UP,
  ORIENTATION_PAGE_RIGHT,
  ORIENTATION_PAGE_DOWN,
  ORIENTATION_PAGE_LEFT
} TessOrientation;
typedef enum TessParagraphJustification {
  JUSTIFICATION_UNKNOWN,
  JUSTIFICATION_LEFT,
  JUSTIFICATION_CENTER,
  JUSTIFICATION_RIGHT
} TessParagraphJustification;
typedef enum TessWritingDirection {
  WRITING_DIRECTION_LEFT_TO_RIGHT,
  WRITING_DIRECTION_RIGHT_TO_LEFT,
  WRITING_DIRECTION_TOP_TO_BOTTOM
} TessWritingDirection;
typedef enum TessTextlineOrder {
  TEXTLINE_ORDER_LEFT_TO_RIGHT,
  TEXTLINE_ORDER_RIGHT_TO_LEFT,
  TEXTLINE_ORDER_TOP_TO_BOTTOM
} TessTextlineOrder;
typedef struct ETEXT_DESC ETEXT_DESC;
#endif

typedef bool (*TessCancelFunc)(void *cancel_this, int words);
typedef bool (*TessProgressFunc)(ETEXT_DESC *ths, int left, int right, int top,
                                 int bottom);

struct Pix;
struct Boxa;
struct Pixa;

/* General free functions */

TESS_API const char *TessVersion();
TESS_API void TessDeleteText(const char *text);
TESS_API void TessDeleteTextArray(char **arr);
TESS_API void TessDeleteIntArray(const int *arr);

/* Renderer API */
TESS_API TessResultRenderer *TessTextRendererCreate(const char *outputbase);
TESS_API TessResultRenderer *TessHOcrRendererCreate(const char *outputbase);
TESS_API TessResultRenderer *TessHOcrRendererCreate2(const char *outputbase,
                                                     BOOL font_info);
TESS_API TessResultRenderer *TessAltoRendererCreate(const char *outputbase);
TESS_API TessResultRenderer *TessPAGERendererCreate(const char *outputbase);
TESS_API TessResultRenderer *TessTsvRendererCreate(const char *outputbase);
TESS_API TessResultRenderer *TessPDFRendererCreate(const char *outputbase,
                                                   const char *datadir,
                                                   BOOL textonly);
TESS_API TessResultRenderer *TessUnlvRendererCreate(const char *outputbase);
TESS_API TessResultRenderer *TessBoxTextRendererCreate(const char *outputbase);
TESS_API TessResultRenderer *TessLSTMBoxRendererCreate(const char *outputbase);
TESS_API TessResultRenderer *TessWordStrBoxRendererCreate(
    const char *outputbase);

TESS_API void TessDeleteResultRenderer(TessResultRenderer *renderer);
TESS_API void TessResultRendererInsert(TessResultRenderer *renderer,
                                       TessResultRenderer *next);
TESS_API TessResultRenderer *TessResultRendererNext(
    TessResultRenderer *renderer);
TESS_API BOOL TessResultRendererBeginDocument(TessResultRenderer *renderer,
                                              const char *title);
TESS_API BOOL TessResultRendererAddImage(TessResultRenderer *renderer,
                                         TessBaseAPI *api);
TESS_API BOOL TessResultRendererEndDocument(TessResultRenderer *renderer);

TESS_API const char *TessResultRendererExtention(TessResultRenderer *renderer);
TESS_API const char *TessResultRendererTitle(TessResultRenderer *renderer);
TESS_API int TessResultRendererImageNum(TessResultRenderer *renderer);

/* Base API */

TESS_API TessBaseAPI *TessBaseAPICreate();
TESS_API void TessBaseAPIDelete(TessBaseAPI *handle);

TESS_API void TessBaseAPISetInputName(TessBaseAPI *handle, const char *name);
TESS_API const char *TessBaseAPIGetInputName(TessBaseAPI *handle);

TESS_API void TessBaseAPISetInputImage(TessBaseAPI *handle, struct Pix *pix);
TESS_API struct Pix *TessBaseAPIGetInputImage(TessBaseAPI *handle);

TESS_API int TessBaseAPIGetSourceYResolution(TessBaseAPI *handle);
TESS_API const char *TessBaseAPIGetDatapath(TessBaseAPI *handle);

TESS_API void TessBaseAPISetOutputName(TessBaseAPI *handle, const char *name);

TESS_API BOOL TessBaseAPISetVariable(TessBaseAPI *handle, const char *name,
                                     const char *value);
TESS_API BOOL TessBaseAPISetDebugVariable(TessBaseAPI *handle, const char *name,
                                          const char *value);

TESS_API BOOL TessBaseAPIGetIntVariable(const TessBaseAPI *handle,
                                        const char *name, int *value);
TESS_API BOOL TessBaseAPIGetBoolVariable(const TessBaseAPI *handle,
                                         const char *name, BOOL *value);
TESS_API BOOL TessBaseAPIGetDoubleVariable(const TessBaseAPI *handle,
                                           const char *name, double *value);
TESS_API const char *TessBaseAPIGetStringVariable(const TessBaseAPI *handle,
                                                  const char *name);

TESS_API void TessBaseAPIPrintVariables(const TessBaseAPI *handle, FILE *fp);
TESS_API BOOL TessBaseAPIPrintVariablesToFile(const TessBaseAPI *handle,
                                              const char *filename);

TESS_API int TessBaseAPIInit1(TessBaseAPI *handle, const char *datapath,
                              const char *language, TessOcrEngineMode oem,
                              char **configs, int configs_size);
TESS_API int TessBaseAPIInit2(TessBaseAPI *handle, const char *datapath,
                              const char *language, TessOcrEngineMode oem);
TESS_API int TessBaseAPIInit3(TessBaseAPI *handle, const char *datapath,
                              const char *language);

TESS_API int TessBaseAPIInit4(TessBaseAPI *handle, const char *datapath,
                              const char *language, TessOcrEngineMode mode,
                              char **configs, int configs_size, char **vars_vec,
                              char **vars_values, size_t vars_vec_size,
                              BOOL set_only_non_debug_params);

TESS_API int TessBaseAPIInit5(TessBaseAPI *handle, const char *data, int data_size,
                              const char *language, TessOcrEngineMode mode,
                              char **configs, int configs_size, char **vars_vec,
                              char **vars_values, size_t vars_vec_size,
                              BOOL set_only_non_debug_params);

TESS_API const char *TessBaseAPIGetInitLanguagesAsString(
    const TessBaseAPI *handle);
TESS_API char **TessBaseAPIGetLoadedLanguagesAsVector(
    const TessBaseAPI *handle);
TESS_API char **TessBaseAPIGetAvailableLanguagesAsVector(
    const TessBaseAPI *handle);

TESS_API void TessBaseAPIInitForAnalysePage(TessBaseAPI *handle);

TESS_API void TessBaseAPIReadConfigFile(TessBaseAPI *handle,
                                        const char *filename);
TESS_API void TessBaseAPIReadDebugConfigFile(TessBaseAPI *handle,
                                             const char *filename);

TESS_API void TessBaseAPISetPageSegMode(TessBaseAPI *handle,
                                        TessPageSegMode mode);
TESS_API TessPageSegMode TessBaseAPIGetPageSegMode(const TessBaseAPI *handle);

TESS_API char *TessBaseAPIRect(TessBaseAPI *handle,
                               const unsigned char *imagedata,
                               int bytes_per_pixel, int bytes_per_line,
                               int left, int top, int width, int height);

TESS_API void TessBaseAPIClearAdaptiveClassifier(TessBaseAPI *handle);

TESS_API void TessBaseAPISetImage(TessBaseAPI *handle,
                                  const unsigned char *imagedata, int width,
                                  int height, int bytes_per_pixel,
                                  int bytes_per_line);
TESS_API void TessBaseAPISetImage2(TessBaseAPI *handle, struct Pix *pix);

TESS_API void TessBaseAPISetSourceResolution(TessBaseAPI *handle, int ppi);

TESS_API void TessBaseAPISetRectangle(TessBaseAPI *handle, int left, int top,
                                      int width, int height);

TESS_API struct Pix *TessBaseAPIGetThresholdedImage(TessBaseAPI *handle);
TESS_API float TessBaseAPIGetGradient(TessBaseAPI *handle);
TESS_API struct Boxa *TessBaseAPIGetRegions(TessBaseAPI *handle,
                                            struct Pixa **pixa);
TESS_API struct Boxa *TessBaseAPIGetTextlines(TessBaseAPI *handle,
                                              struct Pixa **pixa,
                                              int **blockids);
TESS_API struct Boxa *TessBaseAPIGetTextlines1(TessBaseAPI *handle,
                                               BOOL raw_image, int raw_padding,
                                               struct Pixa **pixa,
                                               int **blockids, int **paraids);
TESS_API struct Boxa *TessBaseAPIGetStrips(TessBaseAPI *handle,
                                           struct Pixa **pixa, int **blockids);
TESS_API struct Boxa *TessBaseAPIGetWords(TessBaseAPI *handle,
                                          struct Pixa **pixa);
TESS_API struct Boxa *TessBaseAPIGetConnectedComponents(TessBaseAPI *handle,
                                                        struct Pixa **cc);
TESS_API struct Boxa *TessBaseAPIGetComponentImages(TessBaseAPI *handle,
                                                    TessPageIteratorLevel level,
                                                    BOOL text_only,
                                                    struct Pixa **pixa,
                                                    int **blockids);
TESS_API struct Boxa *TessBaseAPIGetComponentImages1(
    TessBaseAPI *handle, TessPageIteratorLevel level, BOOL text_only,
    BOOL raw_image, int raw_padding, struct Pixa **pixa, int **blockids,
    int **paraids);

TESS_API int TessBaseAPIGetThresholdedImageScaleFactor(
    const TessBaseAPI *handle);

TESS_API TessPageIterator *TessBaseAPIAnalyseLayout(TessBaseAPI *handle);

TESS_API int TessBaseAPIRecognize(TessBaseAPI *handle, ETEXT_DESC *monitor);

TESS_API BOOL TessBaseAPIProcessPages(TessBaseAPI *handle, const char *filename,
                                      const char *retry_config,
                                      int timeout_millisec,
                                      TessResultRenderer *renderer);
TESS_API BOOL TessBaseAPIProcessPage(TessBaseAPI *handle, struct Pix *pix,
                                     int page_index, const char *filename,
                                     const char *retry_config,
                                     int timeout_millisec,
                                     TessResultRenderer *renderer);

TESS_API TessResultIterator *TessBaseAPIGetIterator(TessBaseAPI *handle);
TESS_API TessMutableIterator *TessBaseAPIGetMutableIterator(
    TessBaseAPI *handle);

TESS_API char *TessBaseAPIGetUTF8Text(TessBaseAPI *handle);
TESS_API char *TessBaseAPIGetHOCRText(TessBaseAPI *handle, int page_number);

TESS_API char *TessBaseAPIGetAltoText(TessBaseAPI *handle, int page_number);
TESS_API char *TessBaseAPIGetPAGEText(TessBaseAPI *handle, int page_number);
TESS_API char *TessBaseAPIGetTsvText(TessBaseAPI *handle, int page_number);

TESS_API char *TessBaseAPIGetBoxText(TessBaseAPI *handle, int page_number);
TESS_API char *TessBaseAPIGetLSTMBoxText(TessBaseAPI *handle, int page_number);
TESS_API char *TessBaseAPIGetWordStrBoxText(TessBaseAPI *handle,
                                            int page_number);

TESS_API char *TessBaseAPIGetUNLVText(TessBaseAPI *handle);
TESS_API int TessBaseAPIMeanTextConf(TessBaseAPI *handle);

TESS_API int *TessBaseAPIAllWordConfidences(TessBaseAPI *handle);

#ifndef DISABLED_LEGACY_ENGINE
TESS_API BOOL TessBaseAPIAdaptToWordStr(TessBaseAPI *handle,
                                        TessPageSegMode mode,
                                        const char *wordstr);
#endif // #ifndef DISABLED_LEGACY_ENGINE

TESS_API void TessBaseAPIClear(TessBaseAPI *handle);
TESS_API void TessBaseAPIEnd(TessBaseAPI *handle);

TESS_API int TessBaseAPIIsValidWord(TessBaseAPI *handle, const char *word);
TESS_API BOOL TessBaseAPIGetTextDirection(TessBaseAPI *handle, int *out_offset,
                                          float *out_slope);

TESS_API const char *TessBaseAPIGetUnichar(TessBaseAPI *handle, int unichar_id);

TESS_API void TessBaseAPIClearPersistentCache(TessBaseAPI *handle);

#ifndef DISABLED_LEGACY_ENGINE

// Call TessDeleteText(*best_script_name) to free memory allocated by this
// function
TESS_API BOOL TessBaseAPIDetectOrientationScript(TessBaseAPI *handle,
                                                 int *orient_deg,
                                                 float *orient_conf,
                                                 const char **script_name,
                                                 float *script_conf);
#endif // #ifndef DISABLED_LEGACY_ENGINE

TESS_API void TessBaseAPISetMinOrientationMargin(TessBaseAPI *handle,
                                                 double margin);

TESS_API int TessBaseAPINumDawgs(const TessBaseAPI *handle);

TESS_API TessOcrEngineMode TessBaseAPIOem(const TessBaseAPI *handle);

TESS_API void TessBaseGetBlockTextOrientations(TessBaseAPI *handle,
                                               int **block_orientation,
                                               bool **vertical_writing);

/* Page iterator */

TESS_API void TessPageIteratorDelete(TessPageIterator *handle);

TESS_API TessPageIterator *TessPageIteratorCopy(const TessPageIterator *handle);

TESS_API void TessPageIteratorBegin(TessPageIterator *handle);

TESS_API BOOL TessPageIteratorNext(TessPageIterator *handle,
                                   TessPageIteratorLevel level);

TESS_API BOOL TessPageIteratorIsAtBeginningOf(const TessPageIterator *handle,
                                              TessPageIteratorLevel level);

TESS_API BOOL TessPageIteratorIsAtFinalElement(const TessPageIterator *handle,
                                               TessPageIteratorLevel level,
                                               TessPageIteratorLevel element);

TESS_API BOOL TessPageIteratorBoundingBox(const TessPageIterator *handle,
                                          TessPageIteratorLevel level,
                                          int *left, int *top, int *right,
                                          int *bottom);

TESS_API TessPolyBlockType
TessPageIteratorBlockType(const TessPageIterator *handle);

TESS_API struct Pix *TessPageIteratorGetBinaryImage(
    const TessPageIterator *handle, TessPageIteratorLevel level);

TESS_API struct Pix *TessPageIteratorGetImage(const TessPageIterator *handle,
                                              TessPageIteratorLevel level,
                                              int padding,
                                              struct Pix *original_image,
                                              int *left, int *top);

TESS_API BOOL TessPageIteratorBaseline(const TessPageIterator *handle,
                                       TessPageIteratorLevel level, int *x1,
                                       int *y1, int *x2, int *y2);

TESS_API void TessPageIteratorOrientation(
    TessPageIterator *handle, TessOrientation *orientation,
    TessWritingDirection *writing_direction, TessTextlineOrder *textline_order,
    float *deskew_angle);

TESS_API void TessPageIteratorParagraphInfo(
    TessPageIterator *handle, TessParagraphJustification *justification,
    BOOL *is_list_item, BOOL *is_crown, int *first_line_indent);

/* Result iterator */

TESS_API void TessResultIteratorDelete(TessResultIterator *handle);
TESS_API TessResultIterator *TessResultIteratorCopy(
    const TessResultIterator *handle);
TESS_API TessPageIterator *TessResultIteratorGetPageIterator(
    TessResultIterator *handle);
TESS_API const TessPageIterator *TessResultIteratorGetPageIteratorConst(
    const TessResultIterator *handle);
TESS_API TessChoiceIterator *TessResultIteratorGetChoiceIterator(
    const TessResultIterator *handle);

TESS_API BOOL TessResultIteratorNext(TessResultIterator *handle,
                                     TessPageIteratorLevel level);
TESS_API char *TessResultIteratorGetUTF8Text(const TessResultIterator *handle,
                                             TessPageIteratorLevel level);
TESS_API float TessResultIteratorConfidence(const TessResultIterator *handle,
                                            TessPageIteratorLevel level);
TESS_API const char *TessResultIteratorWordRecognitionLanguage(
    const TessResultIterator *handle);
TESS_API const char *TessResultIteratorWordFontAttributes(
    const TessResultIterator *handle, BOOL *is_bold, BOOL *is_italic,
    BOOL *is_underlined, BOOL *is_monospace, BOOL *is_serif, BOOL *is_smallcaps,
    int *pointsize, int *font_id);

TESS_API BOOL
TessResultIteratorWordIsFromDictionary(const TessResultIterator *handle);
TESS_API BOOL TessResultIteratorWordIsNumeric(const TessResultIterator *handle);
TESS_API BOOL
TessResultIteratorSymbolIsSuperscript(const TessResultIterator *handle);
TESS_API BOOL
TessResultIteratorSymbolIsSubscript(const TessResultIterator *handle);
TESS_API BOOL
TessResultIteratorSymbolIsDropcap(const TessResultIterator *handle);

TESS_API void TessChoiceIteratorDelete(TessChoiceIterator *handle);
TESS_API BOOL TessChoiceIteratorNext(TessChoiceIterator *handle);
TESS_API const char *TessChoiceIteratorGetUTF8Text(
    const TessChoiceIterator *handle);
TESS_API float TessChoiceIteratorConfidence(const TessChoiceIterator *handle);

/* Progress monitor */

TESS_API ETEXT_DESC *TessMonitorCreate();
TESS_API void TessMonitorDelete(ETEXT_DESC *monitor);
TESS_API void TessMonitorSetCancelFunc(ETEXT_DESC *monitor,
                                       TessCancelFunc cancelFunc);
TESS_API void TessMonitorSetCancelThis(ETEXT_DESC *monitor, void *cancelThis);
TESS_API void *TessMonitorGetCancelThis(ETEXT_DESC *monitor);
TESS_API void TessMonitorSetProgressFunc(ETEXT_DESC *monitor,
                                         TessProgressFunc progressFunc);
TESS_API int TessMonitorGetProgress(ETEXT_DESC *monitor);
TESS_API void TessMonitorSetDeadlineMSecs(ETEXT_DESC *monitor, int deadline);

#ifdef __cplusplus
}
#endif

#endif // API_CAPI_H_
