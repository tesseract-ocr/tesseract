#ifndef TESSERACT_API_CAPI_H__
#define TESSERACT_API_CAPI_H__

#ifdef TESS_CAPI_INCLUDE_BASEAPI
#   include "baseapi.h"
#   include "pageiterator.h"
#   include "resultiterator.h"
#   include "renderer.h"
#else
#   include "platform.h"
#   include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TESS_CALL
#   if defined(WIN32)
#       define TESS_CALL __cdecl
#   else
#       define TESS_CALL
#   endif
#endif

#ifndef BOOL
#   define BOOL int
#   define TRUE 1
#   define FALSE 0
#endif

#ifdef TESS_CAPI_INCLUDE_BASEAPI
typedef tesseract::TessResultRenderer TessResultRenderer;
typedef tesseract::TessTextRenderer TessTextRenderer;
typedef tesseract::TessHOcrRenderer TessHOcrRenderer;
typedef tesseract::TessPDFRenderer TessPDFRenderer;
typedef tesseract::TessUnlvRenderer TessUnlvRenderer;
typedef tesseract::TessBoxTextRenderer TessBoxTextRenderer;
typedef tesseract::TessBaseAPI TessBaseAPI;
typedef tesseract::PageIterator TessPageIterator;
typedef tesseract::ResultIterator TessResultIterator;
typedef tesseract::MutableIterator TessMutableIterator;
typedef tesseract::ChoiceIterator TessChoiceIterator;
typedef tesseract::OcrEngineMode TessOcrEngineMode;
typedef tesseract::PageSegMode TessPageSegMode;
typedef tesseract::ImageThresholder TessImageThresholder;
typedef tesseract::PageIteratorLevel TessPageIteratorLevel;
typedef tesseract::DictFunc TessDictFunc;
typedef tesseract::ProbabilityInContextFunc TessProbabilityInContextFunc;
// typedef tesseract::ParamsModelClassifyFunc TessParamsModelClassifyFunc;
typedef tesseract::FillLatticeFunc TessFillLatticeFunc;
typedef tesseract::Dawg TessDawg;
typedef tesseract::TruthCallback TessTruthCallback;
typedef tesseract::CubeRecoContext TessCubeRecoContext;
typedef tesseract::Orientation TessOrientation;
typedef tesseract::ParagraphJustification TessParagraphJustification;
typedef tesseract::WritingDirection TessWritingDirection;
typedef tesseract::TextlineOrder TessTextlineOrder;
typedef PolyBlockType TessPolyBlockType;
#else
typedef struct TessResultRenderer TessResultRenderer;
typedef struct TessTextRenderer TessTextRenderer;
typedef struct TessHOcrRenderer TessHOcrRenderer;
typedef struct TessPDFRenderer TessPDFRenderer;
typedef struct TessUnlvRenderer TessUnlvRenderer;
typedef struct TessBoxTextRenderer TessBoxTextRenderer;
typedef struct TessBaseAPI TessBaseAPI;
typedef struct TessPageIterator TessPageIterator;
typedef struct TessResultIterator TessResultIterator;
typedef struct TessMutableIterator TessMutableIterator;
typedef struct TessChoiceIterator TessChoiceIterator;
typedef enum TessOcrEngineMode     { OEM_TESSERACT_ONLY, OEM_CUBE_ONLY, OEM_TESSERACT_CUBE_COMBINED, OEM_DEFAULT } TessOcrEngineMode;
typedef enum TessPageSegMode       { PSM_OSD_ONLY, PSM_AUTO_OSD, PSM_AUTO_ONLY, PSM_AUTO, PSM_SINGLE_COLUMN, PSM_SINGLE_BLOCK_VERT_TEXT,
                                     PSM_SINGLE_BLOCK, PSM_SINGLE_LINE, PSM_SINGLE_WORD, PSM_CIRCLE_WORD, PSM_SINGLE_CHAR, PSM_SPARSE_TEXT,
                                     PSM_SPARSE_TEXT_OSD, PSM_COUNT } TessPageSegMode;
typedef enum TessPageIteratorLevel { RIL_BLOCK, RIL_PARA, RIL_TEXTLINE, RIL_WORD, RIL_SYMBOL} TessPageIteratorLevel;
typedef enum TessPolyBlockType     { PT_UNKNOWN, PT_FLOWING_TEXT, PT_HEADING_TEXT, PT_PULLOUT_TEXT, PT_EQUATION, PT_INLINE_EQUATION,
                                     PT_TABLE, PT_VERTICAL_TEXT, PT_CAPTION_TEXT, PT_FLOWING_IMAGE, PT_HEADING_IMAGE,
                                     PT_PULLOUT_IMAGE, PT_HORZ_LINE, PT_VERT_LINE, PT_NOISE, PT_COUNT } TessPolyBlockType;
typedef enum TessOrientation       { ORIENTATION_PAGE_UP, ORIENTATION_PAGE_RIGHT, ORIENTATION_PAGE_DOWN, ORIENTATION_PAGE_LEFT } TessOrientation;
typedef enum TessParagraphJustification { JUSTIFICATION_UNKNOWN, JUSTIFICATION_LEFT, JUSTIFICATION_CENTER, JUSTIFICATION_RIGHT } TessParagraphJustification;
typedef enum TessWritingDirection  { WRITING_DIRECTION_LEFT_TO_RIGHT, WRITING_DIRECTION_RIGHT_TO_LEFT, WRITING_DIRECTION_TOP_TO_BOTTOM } TessWritingDirection;
typedef enum TessTextlineOrder     { TEXTLINE_ORDER_LEFT_TO_RIGHT, TEXTLINE_ORDER_RIGHT_TO_LEFT, TEXTLINE_ORDER_TOP_TO_BOTTOM } TessTextlineOrder;
typedef struct ETEXT_DESC ETEXT_DESC;
#endif

struct Pix;
struct Boxa;
struct Pixa;

/* General free functions */

TESS_API const char*
               TESS_CALL TessVersion();
TESS_API void  TESS_CALL TessDeleteText(char* text);
TESS_API void  TESS_CALL TessDeleteTextArray(char** arr);
TESS_API void  TESS_CALL TessDeleteIntArray(int* arr);
#ifdef TESS_CAPI_INCLUDE_BASEAPI
TESS_API void  TESS_CALL TessDeleteBlockList(BLOCK_LIST* block_list);
#endif

/* Renderer API */
TESS_API TessResultRenderer* TESS_CALL TessTextRendererCreate(const char* outputbase);
TESS_API TessResultRenderer* TESS_CALL TessHOcrRendererCreate(const char* outputbase);
TESS_API TessResultRenderer* TESS_CALL TessHOcrRendererCreate2(const char* outputbase, BOOL font_info);
TESS_API TessResultRenderer* TESS_CALL TessPDFRendererCreate(const char* outputbase, const char* datadir);
TESS_API TessResultRenderer* TESS_CALL TessUnlvRendererCreate(const char* outputbase);
TESS_API TessResultRenderer* TESS_CALL TessBoxTextRendererCreate(const char* outputbase);

TESS_API void TESS_CALL TessDeleteResultRenderer(TessResultRenderer* renderer);
TESS_API void TESS_CALL TessResultRendererInsert(TessResultRenderer* renderer, TessResultRenderer* next);
TESS_API TessResultRenderer*
              TESS_CALL TessResultRendererNext(TessResultRenderer* renderer);
TESS_API BOOL TESS_CALL TessResultRendererBeginDocument(TessResultRenderer* renderer, const char* title);
TESS_API BOOL TESS_CALL TessResultRendererAddImage(TessResultRenderer* renderer, TessBaseAPI* api);
TESS_API BOOL TESS_CALL TessResultRendererEndDocument(TessResultRenderer* renderer);

TESS_API const char* TESS_CALL TessResultRendererExtention(TessResultRenderer* renderer);
TESS_API const char* TESS_CALL TessResultRendererTitle(TessResultRenderer* renderer);
TESS_API int TESS_CALL TessResultRendererImageNum(TessResultRenderer* renderer);

/* Base API */

TESS_API TessBaseAPI*
               TESS_CALL TessBaseAPICreate();
TESS_API void  TESS_CALL TessBaseAPIDelete(TessBaseAPI* handle);

TESS_API size_t TESS_CALL TessBaseAPIGetOpenCLDevice(TessBaseAPI* handle, void **device);

TESS_API void  TESS_CALL TessBaseAPISetInputName( TessBaseAPI* handle, const char* name);
TESS_API const char* TESS_CALL TessBaseAPIGetInputName(TessBaseAPI* handle);

TESS_API void  TESS_CALL TessBaseAPISetInputImage(TessBaseAPI* handle, struct Pix* pix);
TESS_API struct Pix*  TESS_CALL TessBaseAPIGetInputImage(TessBaseAPI* handle);

TESS_API int   TESS_CALL TessBaseAPIGetSourceYResolution(TessBaseAPI* handle);
TESS_API const char* TESS_CALL TessBaseAPIGetDatapath(TessBaseAPI* handle);

TESS_API void  TESS_CALL TessBaseAPISetOutputName(TessBaseAPI* handle, const char* name);

TESS_API BOOL  TESS_CALL TessBaseAPISetVariable(TessBaseAPI* handle, const char* name, const char* value);
TESS_API BOOL  TESS_CALL TessBaseAPISetDebugVariable(TessBaseAPI* handle, const char* name, const char* value);

TESS_API BOOL  TESS_CALL TessBaseAPIGetIntVariable(   const TessBaseAPI* handle, const char* name, int* value);
TESS_API BOOL  TESS_CALL TessBaseAPIGetBoolVariable(  const TessBaseAPI* handle, const char* name, BOOL* value);
TESS_API BOOL  TESS_CALL TessBaseAPIGetDoubleVariable(const TessBaseAPI* handle, const char* name, double* value);
TESS_API const char*
               TESS_CALL TessBaseAPIGetStringVariable(const TessBaseAPI* handle, const char* name);

TESS_API void  TESS_CALL TessBaseAPIPrintVariables(      const TessBaseAPI* handle, FILE* fp);
TESS_API BOOL  TESS_CALL TessBaseAPIPrintVariablesToFile(const TessBaseAPI* handle, const char* filename);
#ifdef TESS_CAPI_INCLUDE_BASEAPI
TESS_API BOOL  TESS_CALL TessBaseAPIGetVariableAsString(TessBaseAPI* handle, const char* name, STRING* val);
#endif

#ifdef TESS_CAPI_INCLUDE_BASEAPI
TESS_API int   TESS_CALL TessBaseAPIInit(TessBaseAPI* handle, const char* datapath, const char* language,
                                         TessOcrEngineMode mode, char** configs, int configs_size,
                                         const STRING* vars_vec, size_t vars_vec_size,
                                         const STRING* vars_values, size_t vars_values_size, BOOL set_only_init_params);
#endif
TESS_API int   TESS_CALL TessBaseAPIInit1(TessBaseAPI* handle, const char* datapath, const char* language, TessOcrEngineMode oem,
                                          char** configs, int configs_size);
TESS_API int   TESS_CALL TessBaseAPIInit2(TessBaseAPI* handle, const char* datapath, const char* language, TessOcrEngineMode oem);
TESS_API int   TESS_CALL TessBaseAPIInit3(TessBaseAPI* handle, const char* datapath, const char* language);

TESS_API int TESS_CALL TessBaseAPIInit4(TessBaseAPI* handle, const char* datapath, const char* language, TessOcrEngineMode mode,
    char** configs, int configs_size,
    char** vars_vec, char** vars_values, size_t vars_vec_size,
    BOOL set_only_non_debug_params);

TESS_API const char*
               TESS_CALL TessBaseAPIGetInitLanguagesAsString(const TessBaseAPI* handle);
TESS_API char**
               TESS_CALL TessBaseAPIGetLoadedLanguagesAsVector(const TessBaseAPI* handle);
TESS_API char**
               TESS_CALL TessBaseAPIGetAvailableLanguagesAsVector(const TessBaseAPI* handle);

TESS_API int   TESS_CALL TessBaseAPIInitLangMod(TessBaseAPI* handle, const char* datapath, const char* language);
TESS_API void  TESS_CALL TessBaseAPIInitForAnalysePage(TessBaseAPI* handle);

TESS_API void  TESS_CALL TessBaseAPIReadConfigFile(TessBaseAPI* handle, const char* filename);
TESS_API void  TESS_CALL TessBaseAPIReadDebugConfigFile(TessBaseAPI* handle, const char* filename);

TESS_API void  TESS_CALL TessBaseAPISetPageSegMode(TessBaseAPI* handle, TessPageSegMode mode);
TESS_API TessPageSegMode
               TESS_CALL TessBaseAPIGetPageSegMode(const TessBaseAPI* handle);

TESS_API char* TESS_CALL TessBaseAPIRect(TessBaseAPI* handle, const unsigned char* imagedata,
                                         int bytes_per_pixel, int bytes_per_line,
                                         int left, int top, int width, int height);

TESS_API void  TESS_CALL TessBaseAPIClearAdaptiveClassifier(TessBaseAPI* handle);

TESS_API void  TESS_CALL TessBaseAPISetImage(TessBaseAPI* handle, const unsigned char* imagedata, int width, int height,
                                             int bytes_per_pixel, int bytes_per_line);
TESS_API void  TESS_CALL TessBaseAPISetImage2(TessBaseAPI* handle, struct Pix* pix);

TESS_API void TESS_CALL TessBaseAPISetSourceResolution(TessBaseAPI* handle, int ppi);

TESS_API void  TESS_CALL TessBaseAPISetRectangle(TessBaseAPI* handle, int left, int top, int width, int height);

#ifdef TESS_CAPI_INCLUDE_BASEAPI
TESS_API void  TESS_CALL TessBaseAPISetThresholder(TessBaseAPI* handle, TessImageThresholder* thresholder);
#endif

TESS_API struct Pix*
               TESS_CALL TessBaseAPIGetThresholdedImage(   TessBaseAPI* handle);
TESS_API struct Boxa*
               TESS_CALL TessBaseAPIGetRegions(            TessBaseAPI* handle, struct Pixa** pixa);
TESS_API struct Boxa*
               TESS_CALL TessBaseAPIGetTextlines(          TessBaseAPI* handle, struct Pixa** pixa, int** blockids);
TESS_API struct Boxa*
               TESS_CALL TessBaseAPIGetTextlines1(         TessBaseAPI* handle, const BOOL raw_image, const int raw_padding,
                                                                                struct Pixa** pixa, int** blockids, int** paraids);
TESS_API struct Boxa*
               TESS_CALL TessBaseAPIGetStrips(             TessBaseAPI* handle, struct Pixa** pixa, int** blockids);
TESS_API struct Boxa*
               TESS_CALL TessBaseAPIGetWords(              TessBaseAPI* handle, struct Pixa** pixa);
TESS_API struct Boxa*
               TESS_CALL TessBaseAPIGetConnectedComponents(TessBaseAPI* handle, struct Pixa** cc);
TESS_API struct Boxa*
               TESS_CALL TessBaseAPIGetComponentImages(    TessBaseAPI* handle, const TessPageIteratorLevel level, const BOOL text_only,
                                                           struct Pixa** pixa, int** blockids);
TESS_API struct Boxa*
               TESS_CALL TessBaseAPIGetComponentImages1(   TessBaseAPI* handle, const TessPageIteratorLevel level, const BOOL text_only,
                                                           const BOOL raw_image, const int raw_padding,
                                                           struct Pixa** pixa, int** blockids, int** paraids);

TESS_API int   TESS_CALL TessBaseAPIGetThresholdedImageScaleFactor(const TessBaseAPI* handle);

TESS_API void  TESS_CALL TessBaseAPIDumpPGM(TessBaseAPI* handle, const char* filename);

TESS_API TessPageIterator*
               TESS_CALL TessBaseAPIAnalyseLayout(TessBaseAPI* handle);

TESS_API int   TESS_CALL TessBaseAPIRecognize(TessBaseAPI* handle, ETEXT_DESC* monitor);
TESS_API int   TESS_CALL TessBaseAPIRecognizeForChopTest(TessBaseAPI* handle, ETEXT_DESC* monitor);
TESS_API BOOL  TESS_CALL TessBaseAPIProcessPages(TessBaseAPI* handle,  const char* filename, const char* retry_config,
                                                 int timeout_millisec, TessResultRenderer* renderer);
TESS_API BOOL  TESS_CALL TessBaseAPIProcessPage(TessBaseAPI* handle, struct Pix* pix, int page_index, const char* filename,
                                               const char* retry_config, int timeout_millisec, TessResultRenderer* renderer);

TESS_API TessResultIterator*
               TESS_CALL TessBaseAPIGetIterator(TessBaseAPI* handle);
TESS_API TessMutableIterator*
               TESS_CALL TessBaseAPIGetMutableIterator(TessBaseAPI* handle);

TESS_API char* TESS_CALL TessBaseAPIGetUTF8Text(TessBaseAPI* handle);
TESS_API char* TESS_CALL TessBaseAPIGetHOCRText(TessBaseAPI* handle, int page_number);
TESS_API char* TESS_CALL TessBaseAPIGetBoxText(TessBaseAPI* handle, int page_number);
TESS_API char* TESS_CALL TessBaseAPIGetUNLVText(TessBaseAPI* handle);
TESS_API int   TESS_CALL TessBaseAPIMeanTextConf(TessBaseAPI* handle);
TESS_API int*  TESS_CALL TessBaseAPIAllWordConfidences(TessBaseAPI* handle);
TESS_API BOOL  TESS_CALL TessBaseAPIAdaptToWordStr(TessBaseAPI* handle, TessPageSegMode mode, const char* wordstr);

TESS_API void  TESS_CALL TessBaseAPIClear(TessBaseAPI* handle);
TESS_API void  TESS_CALL TessBaseAPIEnd(TessBaseAPI* handle);

TESS_API int   TESS_CALL TessBaseAPIIsValidWord(TessBaseAPI* handle, const char* word);
TESS_API BOOL  TESS_CALL TessBaseAPIGetTextDirection(TessBaseAPI* handle, int* out_offset, float* out_slope);

#ifdef TESS_CAPI_INCLUDE_BASEAPI
TESS_API void  TESS_CALL TessBaseAPISetDictFunc(TessBaseAPI* handle, TessDictFunc f);
TESS_API void  TESS_CALL TessBaseAPIClearPersistentCache(TessBaseAPI* handle);
TESS_API void  TESS_CALL TessBaseAPISetProbabilityInContextFunc(TessBaseAPI* handle, TessProbabilityInContextFunc f);

TESS_API void  TESS_CALL TessBaseAPISetFillLatticeFunc(TessBaseAPI* handle, TessFillLatticeFunc f);
TESS_API BOOL  TESS_CALL TessBaseAPIDetectOS(TessBaseAPI* handle, OSResults* results);

TESS_API void  TESS_CALL TessBaseAPIGetFeaturesForBlob(TessBaseAPI* handle, TBLOB* blob, INT_FEATURE_STRUCT* int_features,
                                                       int* num_features, int* FeatureOutlineIndex);

TESS_API ROW*  TESS_CALL TessFindRowForBox(BLOCK_LIST* blocks, int left, int top, int right, int bottom);
TESS_API void  TESS_CALL TessBaseAPIRunAdaptiveClassifier(TessBaseAPI* handle, TBLOB* blob, int num_max_matches,
                                                          int* unichar_ids, float* ratings, int* num_matches_returned);
#endif

TESS_API const char*
               TESS_CALL TessBaseAPIGetUnichar(TessBaseAPI* handle, int unichar_id);

#ifdef TESS_CAPI_INCLUDE_BASEAPI
TESS_API const TessDawg*
               TESS_CALL TessBaseAPIGetDawg(const TessBaseAPI* handle, int i);
TESS_API int   TESS_CALL TessBaseAPINumDawgs(const TessBaseAPI* handle);
#endif

#ifdef TESS_CAPI_INCLUDE_BASEAPI
TESS_API ROW*  TESS_CALL TessMakeTessOCRRow(float baseline, float xheight, float descender, float ascender);
TESS_API TBLOB*
               TESS_CALL TessMakeTBLOB(Pix* pix);
TESS_API void  TESS_CALL TessNormalizeTBLOB(TBLOB* tblob, ROW* row, BOOL numeric_mode);

TESS_API TessOcrEngineMode
               TESS_CALL TessBaseAPIOem(const TessBaseAPI* handle);
TESS_API void  TESS_CALL TessBaseAPIInitTruthCallback(TessBaseAPI* handle, TessTruthCallback* cb);

TESS_API TessCubeRecoContext*
               TESS_CALL TessBaseAPIGetCubeRecoContext(const TessBaseAPI* handle);
#endif

TESS_API void  TESS_CALL TessBaseAPISetMinOrientationMargin(TessBaseAPI* handle, double margin);
#ifdef TESS_CAPI_INCLUDE_BASEAPI
TESS_API void  TESS_CALL TessBaseGetBlockTextOrientations(TessBaseAPI* handle, int** block_orientation, BOOL** vertical_writing);

TESS_API BLOCK_LIST*
               TESS_CALL TessBaseAPIFindLinesCreateBlockList(TessBaseAPI* handle);
#endif

/* Page iterator */

TESS_API void  TESS_CALL TessPageIteratorDelete(TessPageIterator* handle);
TESS_API TessPageIterator*
               TESS_CALL TessPageIteratorCopy(const TessPageIterator* handle);

TESS_API void  TESS_CALL TessPageIteratorBegin(TessPageIterator* handle);
TESS_API BOOL  TESS_CALL TessPageIteratorNext(TessPageIterator* handle, TessPageIteratorLevel level);
TESS_API BOOL  TESS_CALL TessPageIteratorIsAtBeginningOf(const TessPageIterator* handle, TessPageIteratorLevel level);
TESS_API BOOL  TESS_CALL TessPageIteratorIsAtFinalElement(const TessPageIterator* handle, TessPageIteratorLevel level,
                                                          TessPageIteratorLevel element);

TESS_API BOOL  TESS_CALL TessPageIteratorBoundingBox(const TessPageIterator* handle, TessPageIteratorLevel level,
                                                     int* left, int* top, int* right, int* bottom);
TESS_API TessPolyBlockType
               TESS_CALL TessPageIteratorBlockType(const TessPageIterator* handle);

TESS_API struct Pix*
               TESS_CALL TessPageIteratorGetBinaryImage(const TessPageIterator* handle, TessPageIteratorLevel level);
TESS_API struct Pix*
               TESS_CALL TessPageIteratorGetImage(const TessPageIterator* handle, TessPageIteratorLevel level, int padding,
                                                  struct Pix* original_image, int* left, int* top);

TESS_API BOOL  TESS_CALL TessPageIteratorBaseline(const TessPageIterator* handle, TessPageIteratorLevel level,
                                                  int* x1, int* y1, int* x2, int* y2);

TESS_API void  TESS_CALL TessPageIteratorOrientation(TessPageIterator* handle, TessOrientation* orientation,
                                                     TessWritingDirection* writing_direction, TessTextlineOrder* textline_order,
                                                     float* deskew_angle);

TESS_API void  TESS_CALL TessPageIteratorParagraphInfo(TessPageIterator* handle, TessParagraphJustification* justification,
                                                       BOOL *is_list_item, BOOL *is_crown, int *first_line_indent);

/* Result iterator */

TESS_API void  TESS_CALL TessResultIteratorDelete(TessResultIterator* handle);
TESS_API TessResultIterator*
               TESS_CALL TessResultIteratorCopy(const TessResultIterator* handle);
TESS_API TessPageIterator*
               TESS_CALL TessResultIteratorGetPageIterator(TessResultIterator* handle);
TESS_API const TessPageIterator*
               TESS_CALL TessResultIteratorGetPageIteratorConst(const TessResultIterator* handle);
TESS_API TessChoiceIterator*
               TESS_CALL TessResultIteratorGetChoiceIterator(const TessResultIterator* handle);

TESS_API BOOL  TESS_CALL TessResultIteratorNext(TessResultIterator* handle, TessPageIteratorLevel level);
TESS_API char* TESS_CALL TessResultIteratorGetUTF8Text(const TessResultIterator* handle, TessPageIteratorLevel level);
TESS_API float TESS_CALL TessResultIteratorConfidence(const TessResultIterator* handle, TessPageIteratorLevel level);
TESS_API const char*
               TESS_CALL TessResultIteratorWordRecognitionLanguage(const TessResultIterator* handle);
TESS_API const char*
               TESS_CALL TessResultIteratorWordFontAttributes(const TessResultIterator* handle, BOOL* is_bold, BOOL* is_italic,
                                                              BOOL* is_underlined, BOOL* is_monospace, BOOL* is_serif,
                                                              BOOL* is_smallcaps, int* pointsize, int* font_id);

TESS_API BOOL  TESS_CALL TessResultIteratorWordIsFromDictionary(const TessResultIterator* handle);
TESS_API BOOL  TESS_CALL TessResultIteratorWordIsNumeric(const TessResultIterator* handle);
TESS_API BOOL  TESS_CALL TessResultIteratorSymbolIsSuperscript(const TessResultIterator* handle);
TESS_API BOOL  TESS_CALL TessResultIteratorSymbolIsSubscript(const TessResultIterator* handle);
TESS_API BOOL  TESS_CALL TessResultIteratorSymbolIsDropcap(const TessResultIterator* handle);

TESS_API void  TESS_CALL TessChoiceIteratorDelete(TessChoiceIterator* handle);
TESS_API BOOL  TESS_CALL TessChoiceIteratorNext(TessChoiceIterator* handle);
TESS_API const char* TESS_CALL TessChoiceIteratorGetUTF8Text(const TessChoiceIterator* handle);
TESS_API float TESS_CALL TessChoiceIteratorConfidence(const TessChoiceIterator* handle);

#ifdef __cplusplus
}
#endif

#endif /* TESSERACT_API_CAPI_H__ */
