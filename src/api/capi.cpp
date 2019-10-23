///////////////////////////////////////////////////////////////////////
// File:        capi.cpp
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
//
///////////////////////////////////////////////////////////////////////

#ifndef TESS_CAPI_INCLUDE_BASEAPI
#  define TESS_CAPI_INCLUDE_BASEAPI
#endif
#include "capi.h"
#include "genericvector.h"
#include "strngs.h"

TESS_API const char* TessVersion() {
  return TessBaseAPI::Version();
}

TESS_API void TessDeleteText(const char* text) {
  delete[] text;
}

TESS_API void TessDeleteTextArray(char** arr) {
  for (char** pos = arr; *pos != nullptr; ++pos) {
    delete[] * pos;
  }
  delete[] arr;
}

TESS_API void TessDeleteIntArray(const int* arr) {
  delete[] arr;
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API void TessDeleteBlockList(BLOCK_LIST* block_list) {
  TessBaseAPI::DeleteBlockList(block_list);
}
#endif

TESS_API TessResultRenderer*
TessTextRendererCreate(const char* outputbase) {
  return new TessTextRenderer(outputbase);
}

TESS_API TessResultRenderer*
TessHOcrRendererCreate(const char* outputbase) {
  return new TessHOcrRenderer(outputbase);
}

TESS_API TessResultRenderer*
TessHOcrRendererCreate2(const char* outputbase, BOOL font_info) {
  return new TessHOcrRenderer(outputbase, font_info != 0);
}

TESS_API TessResultRenderer*
TessAltoRendererCreate(const char* outputbase) {
  return new TessAltoRenderer(outputbase);
}

TESS_API TessResultRenderer*
TessTsvRendererCreate(const char* outputbase) {
  return new TessTsvRenderer(outputbase);
}

TESS_API TessResultRenderer* TessPDFRendererCreate(
    const char* outputbase, const char* datadir, BOOL textonly) {
  return new TessPDFRenderer(outputbase, datadir, textonly != 0);
}

TESS_API TessResultRenderer*
TessUnlvRendererCreate(const char* outputbase) {
  return new TessUnlvRenderer(outputbase);
}

TESS_API TessResultRenderer*
TessBoxTextRendererCreate(const char* outputbase) {
  return new TessBoxTextRenderer(outputbase);
}

TESS_API TessResultRenderer*
TessWordStrBoxRendererCreate(const char* outputbase) {
  return new TessWordStrBoxRenderer(outputbase);
}

TESS_API TessResultRenderer*
TessLSTMBoxRendererCreate(const char* outputbase) {
  return new TessLSTMBoxRenderer(outputbase);
}

TESS_API void TessDeleteResultRenderer(TessResultRenderer* renderer) {
  delete renderer;
}

TESS_API void TessResultRendererInsert(TessResultRenderer* renderer,
                                                 TessResultRenderer* next) {
  renderer->insert(next);
}

TESS_API TessResultRenderer*
TessResultRendererNext(TessResultRenderer* renderer) {
  return renderer->next();
}

TESS_API BOOL TessResultRendererBeginDocument(
    TessResultRenderer* renderer, const char* title) {
  return static_cast<int>(renderer->BeginDocument(title));
}

TESS_API BOOL TessResultRendererAddImage(TessResultRenderer* renderer,
                                                   TessBaseAPI* api) {
  return static_cast<int>(renderer->AddImage(api));
}

TESS_API BOOL
TessResultRendererEndDocument(TessResultRenderer* renderer) {
  return static_cast<int>(renderer->EndDocument());
}

TESS_API const char*
TessResultRendererExtention(TessResultRenderer* renderer) {
  return renderer->file_extension();
}

TESS_API const char*
TessResultRendererTitle(TessResultRenderer* renderer) {
  return renderer->title();
}

TESS_API int
TessResultRendererImageNum(TessResultRenderer* renderer) {
  return renderer->imagenum();
}

TESS_API TessBaseAPI* TessBaseAPICreate() {
  return new TessBaseAPI;
}

TESS_API void TessBaseAPIDelete(TessBaseAPI* handle) {
  delete handle;
}

TESS_API size_t TessBaseAPIGetOpenCLDevice(TessBaseAPI* /*handle*/,
                                                     void** device) {
  return TessBaseAPI::getOpenCLDevice(device);
}

TESS_API void TessBaseAPISetInputName(TessBaseAPI* handle,
                                                const char* name) {
  handle->SetInputName(name);
}

TESS_API const char* TessBaseAPIGetInputName(TessBaseAPI* handle) {
  return handle->GetInputName();
}

TESS_API void TessBaseAPISetInputImage(TessBaseAPI* handle,
                                                 Pix* pix) {
  handle->SetInputImage(pix);
}

TESS_API Pix* TessBaseAPIGetInputImage(TessBaseAPI* handle) {
  return handle->GetInputImage();
}

TESS_API int TessBaseAPIGetSourceYResolution(TessBaseAPI* handle) {
  return handle->GetSourceYResolution();
}

TESS_API const char* TessBaseAPIGetDatapath(TessBaseAPI* handle) {
  return handle->GetDatapath();
}

TESS_API void TessBaseAPISetOutputName(TessBaseAPI* handle,
                                                 const char* name) {
  handle->SetOutputName(name);
}

TESS_API BOOL TessBaseAPISetVariable(TessBaseAPI* handle,
                                               const char* name,
                                               const char* value) {
  return static_cast<int>(handle->SetVariable(name, value));
}

TESS_API BOOL TessBaseAPISetDebugVariable(TessBaseAPI* handle,
                                                    const char* name,
                                                    const char* value) {
  return static_cast<int>(handle->SetDebugVariable(name, value));
}

TESS_API BOOL TessBaseAPIGetIntVariable(const TessBaseAPI* handle,
                                                  const char* name,
                                                  int* value) {
  return static_cast<int>(handle->GetIntVariable(name, value));
}

TESS_API BOOL TessBaseAPIGetBoolVariable(const TessBaseAPI* handle,
                                                   const char* name,
                                                   BOOL* value) {
  bool boolValue;
  bool result = handle->GetBoolVariable(name, &boolValue);
  if (result) {
    *value = static_cast<int>(boolValue);
  }
  return static_cast<int>(result);
}

TESS_API BOOL TessBaseAPIGetDoubleVariable(const TessBaseAPI* handle,
                                                     const char* name,
                                                     double* value) {
  return static_cast<int>(handle->GetDoubleVariable(name, value));
}

TESS_API const char*
TessBaseAPIGetStringVariable(const TessBaseAPI* handle, const char* name) {
  return handle->GetStringVariable(name);
}

TESS_API void TessBaseAPIPrintVariables(const TessBaseAPI* handle,
                                                  FILE* fp) {
  handle->PrintVariables(fp);
}

TESS_API BOOL TessBaseAPIPrintVariablesToFile(
    const TessBaseAPI* handle, const char* filename) {
  FILE* fp = fopen(filename, "w");
  if (fp != nullptr) {
    handle->PrintVariables(fp);
    fclose(fp);
    return TRUE;
  }
  return FALSE;
}

TESS_API BOOL TessBaseAPIGetVariableAsString(TessBaseAPI* handle,
                                                       const char* name,
                                                       STRING* val) {
  return static_cast<int>(handle->GetVariableAsString(name, val));
}

TESS_API int TessBaseAPIInit4(
    TessBaseAPI* handle, const char* datapath, const char* language,
    TessOcrEngineMode mode, char** configs, int configs_size, char** vars_vec,
    char** vars_values, size_t vars_vec_size, BOOL set_only_non_debug_params) {
  GenericVector<STRING> varNames;
  GenericVector<STRING> varValues;
  if (vars_vec != nullptr && vars_values != nullptr) {
    for (size_t i = 0; i < vars_vec_size; i++) {
      varNames.push_back(STRING(vars_vec[i]));
      varValues.push_back(STRING(vars_values[i]));
    }
  }

  return handle->Init(datapath, language, mode, configs, configs_size,
                      &varNames, &varValues, set_only_non_debug_params != 0);
}

TESS_API int TessBaseAPIInit1(TessBaseAPI* handle,
                                        const char* datapath,
                                        const char* language,
                                        TessOcrEngineMode oem, char** configs,
                                        int configs_size) {
  return handle->Init(datapath, language, oem, configs, configs_size, nullptr,
                      nullptr, false);
}

TESS_API int TessBaseAPIInit2(TessBaseAPI* handle,
                                        const char* datapath,
                                        const char* language,
                                        TessOcrEngineMode oem) {
  return handle->Init(datapath, language, oem);
}

TESS_API int TessBaseAPIInit3(TessBaseAPI* handle,
                                        const char* datapath,
                                        const char* language) {
  return handle->Init(datapath, language);
}

TESS_API const char*
TessBaseAPIGetInitLanguagesAsString(const TessBaseAPI* handle) {
  return handle->GetInitLanguagesAsString();
}

TESS_API char**
TessBaseAPIGetLoadedLanguagesAsVector(const TessBaseAPI* handle) {
  GenericVector<STRING> languages;
  handle->GetLoadedLanguagesAsVector(&languages);
  char** arr = new char*[languages.size() + 1];
  for (int index = 0; index < languages.size(); ++index) {
    arr[index] = languages[index].strdup();
  }
  arr[languages.size()] = nullptr;
  return arr;
}

TESS_API char**
TessBaseAPIGetAvailableLanguagesAsVector(const TessBaseAPI* handle) {
  GenericVector<STRING> languages;
  handle->GetAvailableLanguagesAsVector(&languages);
  char** arr = new char*[languages.size() + 1];
  for (int index = 0; index < languages.size(); ++index) {
    arr[index] = languages[index].strdup();
  }
  arr[languages.size()] = nullptr;
  return arr;
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API int TessBaseAPIInitLangMod(TessBaseAPI* handle,
                                              const char* datapath,
                                              const char* language) {
  return handle->InitLangMod(datapath, language);
}
#endif

TESS_API void TessBaseAPIInitForAnalysePage(TessBaseAPI* handle) {
  handle->InitForAnalysePage();
}

TESS_API void TessBaseAPIReadConfigFile(TessBaseAPI* handle,
                                                  const char* filename) {
  handle->ReadConfigFile(filename);
}

TESS_API void TessBaseAPIReadDebugConfigFile(TessBaseAPI* handle,
                                                       const char* filename) {
  handle->ReadDebugConfigFile(filename);
}

TESS_API void TessBaseAPISetPageSegMode(TessBaseAPI* handle,
                                                  TessPageSegMode mode) {
  handle->SetPageSegMode(mode);
}

TESS_API TessPageSegMode
TessBaseAPIGetPageSegMode(const TessBaseAPI* handle) {
  return handle->GetPageSegMode();
}

TESS_API char* TessBaseAPIRect(TessBaseAPI* handle,
                                         const unsigned char* imagedata,
                                         int bytes_per_pixel,
                                         int bytes_per_line, int left, int top,
                                         int width, int height) {
  return handle->TesseractRect(imagedata, bytes_per_pixel, bytes_per_line, left,
                               top, width, height);
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API void
TessBaseAPIClearAdaptiveClassifier(TessBaseAPI* handle) {
  handle->ClearAdaptiveClassifier();
}
#endif

TESS_API void TessBaseAPISetImage(TessBaseAPI* handle,
                                            const unsigned char* imagedata,
                                            int width, int height,
                                            int bytes_per_pixel,
                                            int bytes_per_line) {
  handle->SetImage(imagedata, width, height, bytes_per_pixel, bytes_per_line);
}

TESS_API void TessBaseAPISetImage2(TessBaseAPI* handle,
                                             struct Pix* pix) {
  return handle->SetImage(pix);
}

TESS_API void TessBaseAPISetSourceResolution(TessBaseAPI* handle,
                                                       int ppi) {
  handle->SetSourceResolution(ppi);
}

TESS_API void TessBaseAPISetRectangle(TessBaseAPI* handle, int left,
                                                int top, int width,
                                                int height) {
  handle->SetRectangle(left, top, width, height);
}

TESS_API void TessBaseAPISetThresholder(
    TessBaseAPI* handle, TessImageThresholder* thresholder) {
  handle->SetThresholder(thresholder);
}

TESS_API struct Pix*
TessBaseAPIGetThresholdedImage(TessBaseAPI* handle) {
  return handle->GetThresholdedImage();
}

TESS_API struct Boxa* TessBaseAPIGetRegions(TessBaseAPI* handle,
                                                      struct Pixa** pixa) {
  return handle->GetRegions(pixa);
}

TESS_API struct Boxa* TessBaseAPIGetTextlines(TessBaseAPI* handle,
                                                        struct Pixa** pixa,
                                                        int** blockids) {
  return handle->GetTextlines(pixa, blockids);
}

TESS_API struct Boxa* TessBaseAPIGetTextlines1(
    TessBaseAPI* handle, const BOOL raw_image, const int raw_padding,
    struct Pixa** pixa, int** blockids, int** paraids) {
  return handle->GetTextlines(raw_image != 0, raw_padding, pixa, blockids,
                              paraids);
}

TESS_API struct Boxa* TessBaseAPIGetStrips(TessBaseAPI* handle,
                                                     struct Pixa** pixa,
                                                     int** blockids) {
  return handle->GetStrips(pixa, blockids);
}

TESS_API struct Boxa* TessBaseAPIGetWords(TessBaseAPI* handle,
                                                    struct Pixa** pixa) {
  return handle->GetWords(pixa);
}

TESS_API struct Boxa*
TessBaseAPIGetConnectedComponents(TessBaseAPI* handle, struct Pixa** cc) {
  return handle->GetConnectedComponents(cc);
}

TESS_API struct Boxa* TessBaseAPIGetComponentImages(
    TessBaseAPI* handle, TessPageIteratorLevel level, BOOL text_only,
    struct Pixa** pixa, int** blockids) {
  return handle->GetComponentImages(level, static_cast<bool>(text_only), pixa,
                                    blockids);
}

TESS_API struct Boxa* TessBaseAPIGetComponentImages1(
    TessBaseAPI* handle, const TessPageIteratorLevel level,
    const BOOL text_only, const BOOL raw_image, const int raw_padding,
    struct Pixa** pixa, int** blockids, int** paraids) {
  return handle->GetComponentImages(level, static_cast<bool>(text_only),
                                    raw_image != 0, raw_padding, pixa, blockids,
                                    paraids);
}

TESS_API int
TessBaseAPIGetThresholdedImageScaleFactor(const TessBaseAPI* handle) {
  return handle->GetThresholdedImageScaleFactor();
}

TESS_API TessPageIterator*
TessBaseAPIAnalyseLayout(TessBaseAPI* handle) {
  return handle->AnalyseLayout();
}

TESS_API int TessBaseAPIRecognize(TessBaseAPI* handle,
                                            ETEXT_DESC* monitor) {
  return handle->Recognize(monitor);
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API int TessBaseAPIRecognizeForChopTest(TessBaseAPI* handle,
                                                       ETEXT_DESC* monitor) {
  return handle->RecognizeForChopTest(monitor);
}
#endif

TESS_API BOOL TessBaseAPIProcessPages(TessBaseAPI* handle,
                                                const char* filename,
                                                const char* retry_config,
                                                int timeout_millisec,
                                                TessResultRenderer* renderer) {
  return static_cast<int>(
      handle->ProcessPages(filename, retry_config, timeout_millisec, renderer));
}

TESS_API BOOL TessBaseAPIProcessPage(TessBaseAPI* handle,
                                               struct Pix* pix, int page_index,
                                               const char* filename,
                                               const char* retry_config,
                                               int timeout_millisec,
                                               TessResultRenderer* renderer) {
  return static_cast<int>(handle->ProcessPage(
      pix, page_index, filename, retry_config, timeout_millisec, renderer));
}

TESS_API TessResultIterator*
TessBaseAPIGetIterator(TessBaseAPI* handle) {
  return handle->GetIterator();
}

TESS_API TessMutableIterator*
TessBaseAPIGetMutableIterator(TessBaseAPI* handle) {
  return handle->GetMutableIterator();
}

TESS_API char* TessBaseAPIGetUTF8Text(TessBaseAPI* handle) {
  return handle->GetUTF8Text();
}

TESS_API char* TessBaseAPIGetHOCRText(TessBaseAPI* handle,
                                                int page_number) {
  return handle->GetHOCRText(nullptr, page_number);
}

TESS_API char* TessBaseAPIGetAltoText(TessBaseAPI* handle,
                                                int page_number) {
  return handle->GetAltoText(page_number);
}

TESS_API char* TessBaseAPIGetTsvText(TessBaseAPI* handle,
                                               int page_number) {
  return handle->GetTSVText(page_number);
}

TESS_API char* TessBaseAPIGetBoxText(TessBaseAPI* handle,
                                               int page_number) {
  return handle->GetBoxText(page_number);
}

TESS_API char* TessBaseAPIGetWordStrBoxText(TessBaseAPI* handle,
                                                      int page_number) {
  return handle->GetWordStrBoxText(page_number);
}

TESS_API char* TessBaseAPIGetLSTMBoxText(TessBaseAPI* handle,
                                                   int page_number) {
  return handle->GetLSTMBoxText(page_number);
}

TESS_API char* TessBaseAPIGetUNLVText(TessBaseAPI* handle) {
  return handle->GetUNLVText();
}

TESS_API int TessBaseAPIMeanTextConf(TessBaseAPI* handle) {
  return handle->MeanTextConf();
}

TESS_API int* TessBaseAPIAllWordConfidences(TessBaseAPI* handle) {
  return handle->AllWordConfidences();
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API BOOL TessBaseAPIAdaptToWordStr(TessBaseAPI* handle,
                                                  TessPageSegMode mode,
                                                  const char* wordstr) {
  return static_cast<int>(handle->AdaptToWordStr(mode, wordstr));
}
#endif

TESS_API void TessBaseAPIClear(TessBaseAPI* handle) {
  handle->Clear();
}

TESS_API void TessBaseAPIEnd(TessBaseAPI* handle) {
  handle->End();
}

TESS_API int TessBaseAPIIsValidWord(TessBaseAPI* handle,
                                              const char* word) {
  return handle->IsValidWord(word);
}

TESS_API BOOL TessBaseAPIGetTextDirection(TessBaseAPI* handle,
                                                    int* out_offset,
                                                    float* out_slope) {
  return static_cast<int>(handle->GetTextDirection(out_offset, out_slope));
}

TESS_API void TessBaseAPISetDictFunc(TessBaseAPI* handle,
                                               TessDictFunc f) {
  handle->SetDictFunc(f);
}

TESS_API void
TessBaseAPIClearPersistentCache(TessBaseAPI* /*handle*/) {
  TessBaseAPI::ClearPersistentCache();
}

TESS_API void TessBaseAPISetProbabilityInContextFunc(
    TessBaseAPI* handle, TessProbabilityInContextFunc f) {
  handle->SetProbabilityInContextFunc(f);
}

#ifndef DISABLED_LEGACY_ENGINE

TESS_API BOOL TessBaseAPIDetectOrientationScript(
    TessBaseAPI* handle, int* orient_deg, float* orient_conf,
    const char** script_name, float* script_conf) {
  bool success;
  success = handle->DetectOrientationScript(orient_deg, orient_conf,
                                            script_name, script_conf);
  return static_cast<BOOL>(success);
}

TESS_API void TessBaseAPIGetFeaturesForBlob(
    TessBaseAPI* handle, TBLOB* blob, INT_FEATURE_STRUCT* int_features,
    int* num_features, int* FeatureOutlineIndex) {
  handle->GetFeaturesForBlob(blob, int_features, num_features,
                             FeatureOutlineIndex);
}

TESS_API ROW* TessFindRowForBox(BLOCK_LIST* blocks, int left, int top,
                                          int right, int bottom) {
  return TessBaseAPI::FindRowForBox(blocks, left, top, right, bottom);
}

TESS_API void TessBaseAPIRunAdaptiveClassifier(
    TessBaseAPI* handle, TBLOB* blob, int num_max_matches, int* unichar_ids,
    float* ratings, int* num_matches_returned) {
  handle->RunAdaptiveClassifier(blob, num_max_matches, unichar_ids, ratings,
                                num_matches_returned);
}

#endif  // ndef DISABLED_LEGACY_ENGINE

TESS_API const char* TessBaseAPIGetUnichar(TessBaseAPI* handle,
                                                     int unichar_id) {
  return handle->GetUnichar(unichar_id);
}

TESS_API const TessDawg* TessBaseAPIGetDawg(const TessBaseAPI* handle,
                                                      int i) {
  return handle->GetDawg(i);
}

TESS_API int TessBaseAPINumDawgs(const TessBaseAPI* handle) {
  return handle->NumDawgs();
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API ROW* TessMakeTessOCRRow(float baseline, float xheight,
                                           float descender, float ascender) {
  return TessBaseAPI::MakeTessOCRRow(baseline, xheight, descender, ascender);
}

TESS_API TBLOB* TessMakeTBLOB(struct Pix* pix) {
  return TessBaseAPI::MakeTBLOB(pix);
}

TESS_API void TessNormalizeTBLOB(TBLOB* tblob, ROW* row,
                                           BOOL numeric_mode) {
  TessBaseAPI::NormalizeTBLOB(tblob, row, static_cast<bool>(numeric_mode));
}
#endif  // ndef DISABLED_LEGACY_ENGINE

TESS_API TessOcrEngineMode TessBaseAPIOem(const TessBaseAPI* handle) {
  return handle->oem();
}

TESS_API void TessBaseAPIInitTruthCallback(TessBaseAPI* handle,
                                                     TessTruthCallback cb) {
  handle->InitTruthCallback(cb);
}

TESS_API void TessBaseAPISetMinOrientationMargin(TessBaseAPI* handle,
                                                           double margin) {
  handle->set_min_orientation_margin(margin);
}

TESS_API void TessBaseGetBlockTextOrientations(
    TessBaseAPI* handle, int** block_orientation, bool** vertical_writing) {
  handle->GetBlockTextOrientations(block_orientation, vertical_writing);
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API BLOCK_LIST*
TessBaseAPIFindLinesCreateBlockList(TessBaseAPI* handle) {
  return handle->FindLinesCreateBlockList();
}
#endif

TESS_API void TessPageIteratorDelete(TessPageIterator* handle) {
  delete handle;
}

TESS_API TessPageIterator*
TessPageIteratorCopy(const TessPageIterator* handle) {
  return new TessPageIterator(*handle);
}

TESS_API void TessPageIteratorBegin(TessPageIterator* handle) {
  handle->Begin();
}

TESS_API BOOL TessPageIteratorNext(TessPageIterator* handle,
                                             TessPageIteratorLevel level) {
  return static_cast<int>(handle->Next(level));
}

TESS_API BOOL TessPageIteratorIsAtBeginningOf(
    const TessPageIterator* handle, TessPageIteratorLevel level) {
  return static_cast<int>(handle->IsAtBeginningOf(level));
}

TESS_API BOOL TessPageIteratorIsAtFinalElement(
    const TessPageIterator* handle, TessPageIteratorLevel level,
    TessPageIteratorLevel element) {
  return static_cast<int>(handle->IsAtFinalElement(level, element));
}

TESS_API BOOL TessPageIteratorBoundingBox(
    const TessPageIterator* handle, TessPageIteratorLevel level, int* left,
    int* top, int* right, int* bottom) {
  return static_cast<int>(handle->BoundingBox(level, left, top, right, bottom));
}

TESS_API TessPolyBlockType
TessPageIteratorBlockType(const TessPageIterator* handle) {
  return handle->BlockType();
}

TESS_API struct Pix* TessPageIteratorGetBinaryImage(
    const TessPageIterator* handle, TessPageIteratorLevel level) {
  return handle->GetBinaryImage(level);
}

TESS_API struct Pix* TessPageIteratorGetImage(
    const TessPageIterator* handle, TessPageIteratorLevel level, int padding,
    struct Pix* original_image, int* left, int* top) {
  return handle->GetImage(level, padding, original_image, left, top);
}

TESS_API BOOL TessPageIteratorBaseline(const TessPageIterator* handle,
                                                 TessPageIteratorLevel level,
                                                 int* x1, int* y1, int* x2,
                                                 int* y2) {
  return static_cast<int>(handle->Baseline(level, x1, y1, x2, y2));
}

TESS_API void TessPageIteratorOrientation(
    TessPageIterator* handle, TessOrientation* orientation,
    TessWritingDirection* writing_direction, TessTextlineOrder* textline_order,
    float* deskew_angle) {
  handle->Orientation(orientation, writing_direction, textline_order,
                      deskew_angle);
}

TESS_API void TessPageIteratorParagraphInfo(
    TessPageIterator* handle, TessParagraphJustification* justification,
    BOOL* is_list_item, BOOL* is_crown, int* first_line_indent) {
  bool bool_is_list_item;
  bool bool_is_crown;
  handle->ParagraphInfo(justification, &bool_is_list_item, &bool_is_crown,
                        first_line_indent);
  if (is_list_item != nullptr) {
    *is_list_item = static_cast<int>(bool_is_list_item);
  }
  if (is_crown != nullptr) {
    *is_crown = static_cast<int>(bool_is_crown);
  }
}

TESS_API void TessResultIteratorDelete(TessResultIterator* handle) {
  delete handle;
}

TESS_API TessResultIterator*
TessResultIteratorCopy(const TessResultIterator* handle) {
  return new TessResultIterator(*handle);
}

TESS_API TessPageIterator*
TessResultIteratorGetPageIterator(TessResultIterator* handle) {
  return handle;
}

TESS_API const TessPageIterator*
TessResultIteratorGetPageIteratorConst(const TessResultIterator* handle) {
  return handle;
}

TESS_API TessChoiceIterator*
TessResultIteratorGetChoiceIterator(const TessResultIterator* handle) {
  return new TessChoiceIterator(*handle);
}

TESS_API BOOL TessResultIteratorNext(TessResultIterator* handle,
                                               TessPageIteratorLevel level) {
  return static_cast<int>(handle->Next(level));
}

TESS_API char* TessResultIteratorGetUTF8Text(
    const TessResultIterator* handle, TessPageIteratorLevel level) {
  return handle->GetUTF8Text(level);
}

TESS_API float TessResultIteratorConfidence(
    const TessResultIterator* handle, TessPageIteratorLevel level) {
  return handle->Confidence(level);
}

TESS_API const char*
TessResultIteratorWordRecognitionLanguage(const TessResultIterator* handle) {
  return handle->WordRecognitionLanguage();
}

TESS_API const char* TessResultIteratorWordFontAttributes(
    const TessResultIterator* handle, BOOL* is_bold, BOOL* is_italic,
    BOOL* is_underlined, BOOL* is_monospace, BOOL* is_serif, BOOL* is_smallcaps,
    int* pointsize, int* font_id) {
  bool bool_is_bold;
  bool bool_is_italic;
  bool bool_is_underlined;
  bool bool_is_monospace;
  bool bool_is_serif;
  bool bool_is_smallcaps;
  const char* ret = handle->WordFontAttributes(
      &bool_is_bold, &bool_is_italic, &bool_is_underlined, &bool_is_monospace,
      &bool_is_serif, &bool_is_smallcaps, pointsize, font_id);
  if (is_bold != nullptr) {
    *is_bold = static_cast<int>(bool_is_bold);
  }
  if (is_italic != nullptr) {
    *is_italic = static_cast<int>(bool_is_italic);
  }
  if (is_underlined != nullptr) {
    *is_underlined = static_cast<int>(bool_is_underlined);
  }
  if (is_monospace != nullptr) {
    *is_monospace = static_cast<int>(bool_is_monospace);
  }
  if (is_serif != nullptr) {
    *is_serif = static_cast<int>(bool_is_serif);
  }
  if (is_smallcaps != nullptr) {
    *is_smallcaps = static_cast<int>(bool_is_smallcaps);
  }
  return ret;
}

TESS_API BOOL
TessResultIteratorWordIsFromDictionary(const TessResultIterator* handle) {
  return static_cast<int>(handle->WordIsFromDictionary());
}

TESS_API BOOL
TessResultIteratorWordIsNumeric(const TessResultIterator* handle) {
  return static_cast<int>(handle->WordIsNumeric());
}

TESS_API BOOL
TessResultIteratorSymbolIsSuperscript(const TessResultIterator* handle) {
  return static_cast<int>(handle->SymbolIsSuperscript());
}

TESS_API BOOL
TessResultIteratorSymbolIsSubscript(const TessResultIterator* handle) {
  return static_cast<int>(handle->SymbolIsSubscript());
}

TESS_API BOOL
TessResultIteratorSymbolIsDropcap(const TessResultIterator* handle) {
  return static_cast<int>(handle->SymbolIsDropcap());
}

TESS_API void TessChoiceIteratorDelete(TessChoiceIterator* handle) {
  delete handle;
}

TESS_API BOOL TessChoiceIteratorNext(TessChoiceIterator* handle) {
  return static_cast<int>(handle->Next());
}

TESS_API const char*
TessChoiceIteratorGetUTF8Text(const TessChoiceIterator* handle) {
  return handle->GetUTF8Text();
}

TESS_API float
TessChoiceIteratorConfidence(const TessChoiceIterator* handle) {
  return handle->Confidence();
}

TESS_API ETEXT_DESC* TessMonitorCreate() {
  return new ETEXT_DESC();
}

TESS_API void TessMonitorDelete(ETEXT_DESC* monitor) {
  delete monitor;
}

TESS_API void TessMonitorSetCancelFunc(ETEXT_DESC* monitor,
                                                 TessCancelFunc cancelFunc) {
  monitor->cancel = cancelFunc;
}

TESS_API void TessMonitorSetCancelThis(ETEXT_DESC* monitor,
                                                 void* cancelThis) {
  monitor->cancel_this = cancelThis;
}

TESS_API void* TessMonitorGetCancelThis(ETEXT_DESC* monitor) {
  return monitor->cancel_this;
}

TESS_API void
TessMonitorSetProgressFunc(ETEXT_DESC* monitor, TessProgressFunc progressFunc) {
  monitor->progress_callback2 = progressFunc;
}

TESS_API int TessMonitorGetProgress(ETEXT_DESC* monitor) {
  return monitor->progress;
}

TESS_API void TessMonitorSetDeadlineMSecs(ETEXT_DESC* monitor,
                                                    int deadline) {
  monitor->set_deadline_msecs(deadline);
}
