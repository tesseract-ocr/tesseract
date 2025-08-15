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

#include <tesseract/capi.h>

const char *TessVersion() {
  return TessBaseAPI::Version();
}

static char *MakeText(const std::string& srcText) {
  auto *text = new char[srcText.size() + 1];
  srcText.copy(text, srcText.size());
  text[srcText.size()] = 0;
  return text;
}

void TessDeleteText(const char *text) {
  delete[] text;
}

static char **MakeTextArray(const std::vector<std::string>& srcArr) {
  auto **arr = new char *[srcArr.size() + 1];
  for (size_t i = 0; i < srcArr.size(); ++i) {
    arr[i] = MakeText(srcArr[i]);
  }
  arr[srcArr.size()] = nullptr;
  return arr;
}

void TessDeleteTextArray(char **arr) {
  for (char **pos = arr; *pos != nullptr; ++pos) {
    delete[] * pos;
  }
  delete[] arr;
}

void TessDeleteIntArray(const int *arr) {
  delete[] arr;
}

TessResultRenderer *TessTextRendererCreate(const char *outputbase) {
  return new tesseract::TessTextRenderer(outputbase);
}

TessResultRenderer *TessHOcrRendererCreate(const char *outputbase) {
  return new tesseract::TessHOcrRenderer(outputbase);
}

TessResultRenderer *TessHOcrRendererCreate2(const char *outputbase, BOOL font_info) {
  return new tesseract::TessHOcrRenderer(outputbase, font_info != 0);
}

TessResultRenderer *TessAltoRendererCreate(const char *outputbase) {
  return new tesseract::TessAltoRenderer(outputbase);
}

TessResultRenderer *TessPAGERendererCreate(const char *outputbase) {
  return new tesseract::TessPAGERenderer(outputbase);
}

TessResultRenderer *TessTsvRendererCreate(const char *outputbase) {
  return new tesseract::TessTsvRenderer(outputbase);
}

TessResultRenderer *TessPDFRendererCreate(const char *outputbase, const char *datadir,
                                          BOOL textonly) {
  return new tesseract::TessPDFRenderer(outputbase, datadir, textonly != 0);
}

TessResultRenderer *TessUnlvRendererCreate(const char *outputbase) {
  return new tesseract::TessUnlvRenderer(outputbase);
}

TessResultRenderer *TessBoxTextRendererCreate(const char *outputbase) {
  return new tesseract::TessBoxTextRenderer(outputbase);
}

TessResultRenderer *TessWordStrBoxRendererCreate(const char *outputbase) {
  return new tesseract::TessWordStrBoxRenderer(outputbase);
}

TessResultRenderer *TessLSTMBoxRendererCreate(const char *outputbase) {
  return new tesseract::TessLSTMBoxRenderer(outputbase);
}

void TessDeleteResultRenderer(TessResultRenderer *renderer) {
  delete renderer;
}

void TessResultRendererInsert(TessResultRenderer *renderer, TessResultRenderer *next) {
  renderer->insert(next);
}

TessResultRenderer *TessResultRendererNext(TessResultRenderer *renderer) {
  return renderer->next();
}

BOOL TessResultRendererBeginDocument(TessResultRenderer *renderer, const char *title) {
  return static_cast<int>(renderer->BeginDocument(title));
}

BOOL TessResultRendererAddImage(TessResultRenderer *renderer, TessBaseAPI *api) {
  return static_cast<int>(renderer->AddImage(api));
}

BOOL TessResultRendererEndDocument(TessResultRenderer *renderer) {
  return static_cast<int>(renderer->EndDocument());
}

const char *TessResultRendererExtention(TessResultRenderer *renderer) {
  return renderer->file_extension();
}

const char *TessResultRendererTitle(TessResultRenderer *renderer) {
  return renderer->title();
}

int TessResultRendererImageNum(TessResultRenderer *renderer) {
  return renderer->imagenum();
}

TessBaseAPI *TessBaseAPICreate() {
  return new TessBaseAPI;
}

void TessBaseAPIDelete(TessBaseAPI *handle) {
  delete handle;
}

void TessBaseAPISetInputName(TessBaseAPI *handle, const char *name) {
  handle->SetInputName(name);
}

const char *TessBaseAPIGetInputName(TessBaseAPI *handle) {
  return handle->GetInputName();
}

void TessBaseAPISetInputImage(TessBaseAPI *handle, Pix *pix) {
  handle->SetInputImage(pix);
}

Pix *TessBaseAPIGetInputImage(TessBaseAPI *handle) {
  return handle->GetInputImage();
}

int TessBaseAPIGetSourceYResolution(TessBaseAPI *handle) {
  return handle->GetSourceYResolution();
}

const char *TessBaseAPIGetDatapath(TessBaseAPI *handle) {
  return handle->GetDatapath();
}

void TessBaseAPISetOutputName(TessBaseAPI *handle, const char *name) {
  handle->SetOutputName(name);
}

BOOL TessBaseAPISetVariable(TessBaseAPI *handle, const char *name, const char *value) {
  return static_cast<int>(handle->SetVariable(name, value));
}

BOOL TessBaseAPISetDebugVariable(TessBaseAPI *handle, const char *name, const char *value) {
  return static_cast<int>(handle->SetDebugVariable(name, value));
}

BOOL TessBaseAPIGetIntVariable(const TessBaseAPI *handle, const char *name, int *value) {
  return static_cast<int>(handle->GetIntVariable(name, value));
}

BOOL TessBaseAPIGetBoolVariable(const TessBaseAPI *handle, const char *name, BOOL *value) {
  bool boolValue;
  bool result = handle->GetBoolVariable(name, &boolValue);
  if (result) {
    *value = static_cast<int>(boolValue);
  }
  return static_cast<int>(result);
}

BOOL TessBaseAPIGetDoubleVariable(const TessBaseAPI *handle, const char *name, double *value) {
  return static_cast<int>(handle->GetDoubleVariable(name, value));
}

const char *TessBaseAPIGetStringVariable(const TessBaseAPI *handle, const char *name) {
  return handle->GetStringVariable(name);
}

void TessBaseAPIPrintVariables(const TessBaseAPI *handle, FILE *fp) {
  handle->PrintVariables(fp);
}

BOOL TessBaseAPIPrintVariablesToFile(const TessBaseAPI *handle, const char *filename) {
  FILE *fp = fopen(filename, "w");
  if (fp != nullptr) {
    handle->PrintVariables(fp);
    fclose(fp);
    return TRUE;
  }
  return FALSE;
}

void TessBaseAPIDumpVariables(const TessBaseAPI *handle, FILE *fp) {
  handle->DumpVariables(fp);
}

BOOL TessBaseAPIDumpVariablesToFile(const TessBaseAPI *handle, const char *filename) {
  FILE *fp = fopen(filename, "w");
  if (fp != nullptr) {
    handle->DumpVariables(fp);
    fclose(fp);
    return TRUE;
  }
  return FALSE;
}

int TessBaseAPIInit4(TessBaseAPI *handle, const char *datapath, const char *language,
                     TessOcrEngineMode mode, char **configs, int configs_size, char **vars_vec,
                     char **vars_values, size_t vars_vec_size, BOOL set_only_non_debug_params) {
  std::vector<std::string> varNames;
  std::vector<std::string> varValues;
  if (vars_vec != nullptr && vars_values != nullptr) {
    for (size_t i = 0; i < vars_vec_size; i++) {
      varNames.emplace_back(vars_vec[i]);
      varValues.emplace_back(vars_values[i]);
    }
  }

  return handle->Init(datapath, language, mode, configs, configs_size, &varNames, &varValues,
                      set_only_non_debug_params != 0);
}

int TessBaseAPIInit1(TessBaseAPI *handle, const char *datapath, const char *language,
                     TessOcrEngineMode oem, char **configs, int configs_size) {
  return handle->Init(datapath, language, oem, configs, configs_size, nullptr, nullptr, false);
}

int TessBaseAPIInit2(TessBaseAPI *handle, const char *datapath, const char *language,
                     TessOcrEngineMode oem) {
  return handle->Init(datapath, language, oem);
}

int TessBaseAPIInit3(TessBaseAPI *handle, const char *datapath, const char *language) {
  return handle->Init(datapath, language);
}

int TessBaseAPIInit5(TessBaseAPI *handle, const char *data, int data_size, const char *language,
                     TessOcrEngineMode mode, char **configs, int configs_size, char **vars_vec,
                     char **vars_values, size_t vars_vec_size, BOOL set_only_non_debug_params) {
  std::vector<std::string> varNames;
  std::vector<std::string> varValues;
  if (vars_vec != nullptr && vars_values != nullptr) {
    for (size_t i = 0; i < vars_vec_size; i++) {
      varNames.emplace_back(vars_vec[i]);
      varValues.emplace_back(vars_values[i]);
    }
  }

  return handle->Init(data, data_size, language, mode, configs, configs_size, &varNames, &varValues,
                      set_only_non_debug_params != 0, nullptr);
}

const char *TessBaseAPIGetInitLanguagesAsString(const TessBaseAPI *handle) {
  return handle->GetInitLanguagesAsString();
}

char **TessBaseAPIGetLoadedLanguagesAsVector(const TessBaseAPI *handle) {
  std::vector<std::string> languages;
  handle->GetLoadedLanguagesAsVector(&languages);
  return MakeTextArray(languages);
}

char **TessBaseAPIGetAvailableLanguagesAsVector(const TessBaseAPI *handle) {
  std::vector<std::string> languages;
  handle->GetAvailableLanguagesAsVector(&languages);
  return MakeTextArray(languages);
}

void TessBaseAPIInitForAnalysePage(TessBaseAPI *handle) {
  handle->InitForAnalysePage();
}

void TessBaseAPIReadConfigFile(TessBaseAPI *handle, const char *filename) {
  handle->ReadConfigFile(filename);
}

void TessBaseAPIReadDebugConfigFile(TessBaseAPI *handle, const char *filename) {
  handle->ReadDebugConfigFile(filename);
}

void TessBaseAPISetPageSegMode(TessBaseAPI *handle, TessPageSegMode mode) {
  handle->SetPageSegMode(mode);
}

TessPageSegMode TessBaseAPIGetPageSegMode(const TessBaseAPI *handle) {
  return handle->GetPageSegMode();
}

char *TessBaseAPIRect(TessBaseAPI *handle, const unsigned char *imagedata, int bytes_per_pixel,
                      int bytes_per_line, int left, int top, int width, int height) {
  return handle->TesseractRect(imagedata, bytes_per_pixel, bytes_per_line, left, top, width,
                               height);
}

#ifndef DISABLED_LEGACY_ENGINE
void TessBaseAPIClearAdaptiveClassifier(TessBaseAPI *handle) {
  handle->ClearAdaptiveClassifier();
}
#endif

void TessBaseAPISetImage(TessBaseAPI *handle, const unsigned char *imagedata, int width, int height,
                         int bytes_per_pixel, int bytes_per_line) {
  handle->SetImage(imagedata, width, height, bytes_per_pixel, bytes_per_line);
}

void TessBaseAPISetImage2(TessBaseAPI *handle, struct Pix *pix) {
  return handle->SetImage(pix);
}

void TessBaseAPISetSourceResolution(TessBaseAPI *handle, int ppi) {
  handle->SetSourceResolution(ppi);
}

void TessBaseAPISetRectangle(TessBaseAPI *handle, int left, int top, int width, int height) {
  handle->SetRectangle(left, top, width, height);
}

struct Pix *TessBaseAPIGetThresholdedImage(TessBaseAPI *handle) {
  return handle->GetThresholdedImage();
}

float TessBaseAPIGetGradient(TessBaseAPI *handle) {
  return handle->GetGradient();
}

void TessBaseAPIClearPersistentCache(TessBaseAPI * /*handle*/) {
  TessBaseAPI::ClearPersistentCache();
}

#ifndef DISABLED_LEGACY_ENGINE

BOOL TessBaseAPIDetectOrientationScript(TessBaseAPI *handle, int *orient_deg, float *orient_conf,
                                        const char **script_name, float *script_conf) {
  auto success = handle->DetectOrientationScript(orient_deg, orient_conf, script_name, script_conf);
  return static_cast<BOOL>(success);
}

#endif

struct Boxa *TessBaseAPIGetRegions(TessBaseAPI *handle, struct Pixa **pixa) {
  return handle->GetRegions(pixa);
}

struct Boxa *TessBaseAPIGetTextlines(TessBaseAPI *handle, struct Pixa **pixa, int **blockids) {
  return handle->GetTextlines(pixa, blockids);
}

struct Boxa *TessBaseAPIGetTextlines1(TessBaseAPI *handle, const BOOL raw_image,
                                      const int raw_padding, struct Pixa **pixa, int **blockids,
                                      int **paraids) {
  return handle->GetTextlines(raw_image != 0, raw_padding, pixa, blockids, paraids);
}

struct Boxa *TessBaseAPIGetStrips(TessBaseAPI *handle, struct Pixa **pixa, int **blockids) {
  return handle->GetStrips(pixa, blockids);
}

struct Boxa *TessBaseAPIGetWords(TessBaseAPI *handle, struct Pixa **pixa) {
  return handle->GetWords(pixa);
}

struct Boxa *TessBaseAPIGetConnectedComponents(TessBaseAPI *handle, struct Pixa **cc) {
  return handle->GetConnectedComponents(cc);
}

struct Boxa *TessBaseAPIGetComponentImages(TessBaseAPI *handle, TessPageIteratorLevel level,
                                           BOOL text_only, struct Pixa **pixa, int **blockids) {
  return handle->GetComponentImages(level, static_cast<bool>(text_only), pixa, blockids);
}

struct Boxa *TessBaseAPIGetComponentImages1(TessBaseAPI *handle, const TessPageIteratorLevel level,
                                            const BOOL text_only, const BOOL raw_image,
                                            const int raw_padding, struct Pixa **pixa,
                                            int **blockids, int **paraids) {
  return handle->GetComponentImages(level, static_cast<bool>(text_only), raw_image != 0,
                                    raw_padding, pixa, blockids, paraids);
}

int TessBaseAPIGetThresholdedImageScaleFactor(const TessBaseAPI *handle) {
  return handle->GetThresholdedImageScaleFactor();
}

TessPageIterator *TessBaseAPIAnalyseLayout(TessBaseAPI *handle) {
  return handle->AnalyseLayout();
}

int TessBaseAPIRecognize(TessBaseAPI *handle, ETEXT_DESC *monitor) {
  return handle->Recognize(monitor);
}

BOOL TessBaseAPIProcessPages(TessBaseAPI *handle, const char *filename, const char *retry_config,
                             int timeout_millisec, TessResultRenderer *renderer) {
  return static_cast<int>(handle->ProcessPages(filename, retry_config, timeout_millisec, renderer));
}

BOOL TessBaseAPIProcessPage(TessBaseAPI *handle, struct Pix *pix, int page_index,
                            const char *filename, const char *retry_config, int timeout_millisec,
                            TessResultRenderer *renderer) {
  return static_cast<int>(
      handle->ProcessPage(pix, page_index, filename, retry_config, timeout_millisec, renderer));
}

TessResultIterator *TessBaseAPIGetIterator(TessBaseAPI *handle) {
  return handle->GetIterator();
}

TessMutableIterator *TessBaseAPIGetMutableIterator(TessBaseAPI *handle) {
  return handle->GetMutableIterator();
}

char *TessBaseAPIGetUTF8Text(TessBaseAPI *handle) {
  return handle->GetUTF8Text();
}

char *TessBaseAPIGetHOCRText(TessBaseAPI *handle, int page_number) {
  return handle->GetHOCRText(nullptr, page_number);
}

char *TessBaseAPIGetAltoText(TessBaseAPI *handle, int page_number) {
  return handle->GetAltoText(page_number);
}

char *TessBaseAPIGetPAGEText(TessBaseAPI *handle, int page_number) {
  return handle->GetPAGEText(page_number);
}

char *TessBaseAPIGetTsvText(TessBaseAPI *handle, int page_number) {
  return handle->GetTSVText(page_number);
}

char *TessBaseAPIGetBoxText(TessBaseAPI *handle, int page_number) {
  return handle->GetBoxText(page_number);
}

char *TessBaseAPIGetWordStrBoxText(TessBaseAPI *handle, int page_number) {
  return handle->GetWordStrBoxText(page_number);
}

char *TessBaseAPIGetLSTMBoxText(TessBaseAPI *handle, int page_number) {
  return handle->GetLSTMBoxText(page_number);
}

char *TessBaseAPIGetUNLVText(TessBaseAPI *handle) {
  return handle->GetUNLVText();
}

int TessBaseAPIMeanTextConf(TessBaseAPI *handle) {
  return handle->MeanTextConf();
}

int *TessBaseAPIAllWordConfidences(TessBaseAPI *handle) {
  return handle->AllWordConfidences();
}

#ifndef DISABLED_LEGACY_ENGINE
BOOL TessBaseAPIAdaptToWordStr(TessBaseAPI *handle, TessPageSegMode mode, const char *wordstr) {
  return static_cast<int>(handle->AdaptToWordStr(mode, wordstr));
}
#endif

void TessBaseAPIClear(TessBaseAPI *handle) {
  handle->Clear();
}

void TessBaseAPIEnd(TessBaseAPI *handle) {
  handle->End();
}

int TessBaseAPIIsValidWord(TessBaseAPI *handle, const char *word) {
  return handle->IsValidWord(word);
}

BOOL TessBaseAPIGetTextDirection(TessBaseAPI *handle, int *out_offset, float *out_slope) {
  return static_cast<int>(handle->GetTextDirection(out_offset, out_slope));
}

const char *TessBaseAPIGetUnichar(TessBaseAPI *handle, int unichar_id) {
  return handle->GetUnichar(unichar_id);
}

void TessBaseAPISetMinOrientationMargin(TessBaseAPI *handle, double margin) {
  handle->set_min_orientation_margin(margin);
}

int TessBaseAPINumDawgs(const TessBaseAPI *handle) {
  return handle->NumDawgs();
}

TessOcrEngineMode TessBaseAPIOem(const TessBaseAPI *handle) {
  return handle->oem();
}

void TessBaseGetBlockTextOrientations(TessBaseAPI *handle, int **block_orientation,
                                      bool **vertical_writing) {
  handle->GetBlockTextOrientations(block_orientation, vertical_writing);
}

void TessPageIteratorDelete(TessPageIterator *handle) {
  delete handle;
}

TessPageIterator *TessPageIteratorCopy(const TessPageIterator *handle) {
  return new TessPageIterator(*handle);
}

void TessPageIteratorBegin(TessPageIterator *handle) {
  handle->Begin();
}

BOOL TessPageIteratorNext(TessPageIterator *handle, TessPageIteratorLevel level) {
  return static_cast<int>(handle->Next(level));
}

BOOL TessPageIteratorIsAtBeginningOf(const TessPageIterator *handle, TessPageIteratorLevel level) {
  return static_cast<int>(handle->IsAtBeginningOf(level));
}

BOOL TessPageIteratorIsAtFinalElement(const TessPageIterator *handle, TessPageIteratorLevel level,
                                      TessPageIteratorLevel element) {
  return static_cast<int>(handle->IsAtFinalElement(level, element));
}

BOOL TessPageIteratorBoundingBox(const TessPageIterator *handle, TessPageIteratorLevel level,
                                 int *left, int *top, int *right, int *bottom) {
  return static_cast<int>(handle->BoundingBox(level, left, top, right, bottom));
}

TessPolyBlockType TessPageIteratorBlockType(const TessPageIterator *handle) {
  return handle->BlockType();
}

struct Pix *TessPageIteratorGetBinaryImage(const TessPageIterator *handle,
                                           TessPageIteratorLevel level) {
  return handle->GetBinaryImage(level);
}

struct Pix *TessPageIteratorGetImage(const TessPageIterator *handle, TessPageIteratorLevel level,
                                     int padding, struct Pix *original_image, int *left, int *top) {
  return handle->GetImage(level, padding, original_image, left, top);
}

BOOL TessPageIteratorBaseline(const TessPageIterator *handle, TessPageIteratorLevel level, int *x1,
                              int *y1, int *x2, int *y2) {
  return static_cast<int>(handle->Baseline(level, x1, y1, x2, y2));
}

void TessPageIteratorOrientation(TessPageIterator *handle, TessOrientation *orientation,
                                 TessWritingDirection *writing_direction,
                                 TessTextlineOrder *textline_order, float *deskew_angle) {
  handle->Orientation(orientation, writing_direction, textline_order, deskew_angle);
}

void TessPageIteratorParagraphInfo(TessPageIterator *handle,
                                   TessParagraphJustification *justification, BOOL *is_list_item,
                                   BOOL *is_crown, int *first_line_indent) {
  bool bool_is_list_item;
  bool bool_is_crown;
  handle->ParagraphInfo(justification, &bool_is_list_item, &bool_is_crown, first_line_indent);
  if (is_list_item != nullptr) {
    *is_list_item = static_cast<int>(bool_is_list_item);
  }
  if (is_crown != nullptr) {
    *is_crown = static_cast<int>(bool_is_crown);
  }
}

void TessResultIteratorDelete(TessResultIterator *handle) {
  delete handle;
}

TessResultIterator *TessResultIteratorCopy(const TessResultIterator *handle) {
  return new TessResultIterator(*handle);
}

TessPageIterator *TessResultIteratorGetPageIterator(TessResultIterator *handle) {
  return handle;
}

const TessPageIterator *TessResultIteratorGetPageIteratorConst(const TessResultIterator *handle) {
  return handle;
}

TessChoiceIterator *TessResultIteratorGetChoiceIterator(const TessResultIterator *handle) {
  return new TessChoiceIterator(*handle);
}

BOOL TessResultIteratorNext(TessResultIterator *handle, TessPageIteratorLevel level) {
  return static_cast<int>(handle->Next(level));
}

char *TessResultIteratorGetUTF8Text(const TessResultIterator *handle, TessPageIteratorLevel level) {
  return handle->GetUTF8Text(level);
}

float TessResultIteratorConfidence(const TessResultIterator *handle, TessPageIteratorLevel level) {
  return handle->Confidence(level);
}

const char *TessResultIteratorWordRecognitionLanguage(const TessResultIterator *handle) {
  return handle->WordRecognitionLanguage();
}

const char *TessResultIteratorWordFontAttributes(const TessResultIterator *handle, BOOL *is_bold,
                                                 BOOL *is_italic, BOOL *is_underlined,
                                                 BOOL *is_monospace, BOOL *is_serif,
                                                 BOOL *is_smallcaps, int *pointsize, int *font_id) {
  bool bool_is_bold;
  bool bool_is_italic;
  bool bool_is_underlined;
  bool bool_is_monospace;
  bool bool_is_serif;
  bool bool_is_smallcaps;
  const char *ret = handle->WordFontAttributes(&bool_is_bold, &bool_is_italic, &bool_is_underlined,
                                               &bool_is_monospace, &bool_is_serif,
                                               &bool_is_smallcaps, pointsize, font_id);
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

BOOL TessResultIteratorWordIsFromDictionary(const TessResultIterator *handle) {
  return static_cast<int>(handle->WordIsFromDictionary());
}

BOOL TessResultIteratorWordIsNumeric(const TessResultIterator *handle) {
  return static_cast<int>(handle->WordIsNumeric());
}

BOOL TessResultIteratorSymbolIsSuperscript(const TessResultIterator *handle) {
  return static_cast<int>(handle->SymbolIsSuperscript());
}

BOOL TessResultIteratorSymbolIsSubscript(const TessResultIterator *handle) {
  return static_cast<int>(handle->SymbolIsSubscript());
}

BOOL TessResultIteratorSymbolIsDropcap(const TessResultIterator *handle) {
  return static_cast<int>(handle->SymbolIsDropcap());
}

void TessChoiceIteratorDelete(TessChoiceIterator *handle) {
  delete handle;
}

BOOL TessChoiceIteratorNext(TessChoiceIterator *handle) {
  return static_cast<int>(handle->Next());
}

const char *TessChoiceIteratorGetUTF8Text(const TessChoiceIterator *handle) {
  return handle->GetUTF8Text();
}

float TessChoiceIteratorConfidence(const TessChoiceIterator *handle) {
  return handle->Confidence();
}

ETEXT_DESC *TessMonitorCreate() {
  return new ETEXT_DESC();
}

void TessMonitorDelete(ETEXT_DESC *monitor) {
  delete monitor;
}

void TessMonitorSetCancelFunc(ETEXT_DESC *monitor, TessCancelFunc cancelFunc) {
  monitor->cancel = cancelFunc;
}

void TessMonitorSetCancelThis(ETEXT_DESC *monitor, void *cancelThis) {
  monitor->cancel_this = cancelThis;
}

void *TessMonitorGetCancelThis(ETEXT_DESC *monitor) {
  return monitor->cancel_this;
}

void TessMonitorSetProgressFunc(ETEXT_DESC *monitor, TessProgressFunc progressFunc) {
  monitor->progress_callback2 = progressFunc;
}

int TessMonitorGetProgress(ETEXT_DESC *monitor) {
  return monitor->progress;
}

void TessMonitorSetDeadlineMSecs(ETEXT_DESC *monitor, int deadline) {
  monitor->set_deadline_msecs(deadline);
}
