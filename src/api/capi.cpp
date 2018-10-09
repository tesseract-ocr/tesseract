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
#   define TESS_CAPI_INCLUDE_BASEAPI
#endif
#include "capi.h"
#include "genericvector.h"
#include "strngs.h"

TESS_API const char* TESS_CALL TessVersion()
{
    return TessBaseAPI::Version();
}

TESS_API void TESS_CALL TessDeleteText(char* text)
{
    delete [] text;
}

TESS_API void TESS_CALL TessDeleteTextArray(char** arr)
{
    for (char** pos = arr; *pos != nullptr; ++pos)
        delete [] *pos;
    delete [] arr;
}

TESS_API void TESS_CALL TessDeleteIntArray(int* arr)
{
    delete [] arr;
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API void TESS_CALL TessDeleteBlockList(BLOCK_LIST* block_list)
{
    TessBaseAPI::DeleteBlockList(block_list);
}
#endif

TESS_API TessResultRenderer* TESS_CALL TessTextRendererCreate(const char* outputbase)
{
    return new TessTextRenderer(outputbase);
}

TESS_API TessResultRenderer* TESS_CALL TessHOcrRendererCreate(const char* outputbase)
{
    return new TessHOcrRenderer(outputbase);
}

TESS_API TessResultRenderer* TESS_CALL TessHOcrRendererCreate2(const char* outputbase, BOOL font_info)
{
    return new TessHOcrRenderer(outputbase, font_info);
}

TESS_API TessResultRenderer* TESS_CALL TessPDFRendererCreate(const char* outputbase, const char* datadir,
                                                             BOOL textonly)
{
    return new TessPDFRenderer(outputbase, datadir, textonly);
}

TESS_API TessResultRenderer* TESS_CALL TessUnlvRendererCreate(const char* outputbase)
{
    return new TessUnlvRenderer(outputbase);
}

TESS_API TessResultRenderer* TESS_CALL TessBoxTextRendererCreate(const char* outputbase)
{
    return new TessBoxTextRenderer(outputbase);
}

TESS_API void TESS_CALL TessDeleteResultRenderer(TessResultRenderer* renderer)
{
    delete renderer;
}

TESS_API void TESS_CALL TessResultRendererInsert(TessResultRenderer* renderer, TessResultRenderer* next)
{
    renderer->insert(next);
}

TESS_API TessResultRenderer* TESS_CALL TessResultRendererNext(TessResultRenderer* renderer)
{
    return renderer->next();
}

TESS_API BOOL TESS_CALL TessResultRendererBeginDocument(TessResultRenderer* renderer, const char* title)
{
    return renderer->BeginDocument(title);
}

TESS_API BOOL TESS_CALL TessResultRendererAddImage(TessResultRenderer* renderer, TessBaseAPI* api)
{
    return renderer->AddImage(api);
}

TESS_API BOOL TESS_CALL TessResultRendererEndDocument(TessResultRenderer* renderer)
{
    return renderer->EndDocument();
}

TESS_API const char* TESS_CALL TessResultRendererExtention(TessResultRenderer* renderer)
{
    return renderer->file_extension();
}

TESS_API const char* TESS_CALL TessResultRendererTitle(TessResultRenderer* renderer)
{
    return renderer->title();
}

TESS_API int TESS_CALL TessResultRendererImageNum(TessResultRenderer* renderer)
{
    return renderer->imagenum();
}

TESS_API TessBaseAPI* TESS_CALL TessBaseAPICreate()
{
    return new TessBaseAPI;
}

TESS_API void TESS_CALL TessBaseAPIDelete(TessBaseAPI* handle)
{
    delete handle;
}

TESS_API size_t TESS_CALL TessBaseAPIGetOpenCLDevice(TessBaseAPI* handle, void **device)
{
    return handle->getOpenCLDevice(device);
}

TESS_API void TESS_CALL TessBaseAPISetInputName(TessBaseAPI* handle, const char* name)
{
    handle->SetInputName(name);
}

TESS_API const char* TESS_CALL TessBaseAPIGetInputName(TessBaseAPI* handle)
{
    return handle->GetInputName();
}

TESS_API void TESS_CALL TessBaseAPISetInputImage(TessBaseAPI* handle, Pix* pix)
{
    handle->SetInputImage(pix);
}

TESS_API Pix* TESS_CALL TessBaseAPIGetInputImage(TessBaseAPI* handle)
{
    return handle->GetInputImage();
}

TESS_API int TESS_CALL TessBaseAPIGetSourceYResolution(TessBaseAPI* handle)
{
    return handle->GetSourceYResolution();
}

TESS_API const char* TESS_CALL TessBaseAPIGetDatapath(TessBaseAPI* handle)
{
    return handle->GetDatapath();
}

TESS_API void TESS_CALL TessBaseAPISetOutputName(TessBaseAPI* handle, const char* name)
{
    handle->SetOutputName(name);
}

TESS_API BOOL TESS_CALL TessBaseAPISetVariable(TessBaseAPI* handle, const char* name, const char* value)
{
    return handle->SetVariable(name, value) ? TRUE : FALSE;
}

TESS_API BOOL TESS_CALL TessBaseAPISetDebugVariable(TessBaseAPI* handle, const char* name, const char* value)
{
    return handle->SetVariable(name, value) ? TRUE : FALSE;
}

TESS_API BOOL TESS_CALL TessBaseAPIGetIntVariable(const TessBaseAPI* handle, const char* name, int* value)
{
    return handle->GetIntVariable(name, value) ? TRUE : FALSE;
}

TESS_API BOOL TESS_CALL TessBaseAPIGetBoolVariable(const TessBaseAPI* handle, const char* name, BOOL* value)
{
    bool boolValue;
    if (handle->GetBoolVariable(name, &boolValue))
    {
        *value = boolValue ? TRUE : FALSE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

TESS_API BOOL TESS_CALL TessBaseAPIGetDoubleVariable(const TessBaseAPI* handle, const char* name, double* value)
{
    return handle->GetDoubleVariable(name, value) ? TRUE : FALSE;
}

TESS_API const char* TESS_CALL TessBaseAPIGetStringVariable(const TessBaseAPI* handle, const char* name)
{
    return handle->GetStringVariable(name);
}

TESS_API void TESS_CALL TessBaseAPIPrintVariables(const TessBaseAPI* handle, FILE* fp)
{
    handle->PrintVariables(fp);
}

TESS_API BOOL TESS_CALL TessBaseAPIPrintVariablesToFile(const TessBaseAPI* handle, const char* filename)
{
    FILE* fp = fopen(filename, "w");
    if (fp != nullptr)
    {
        handle->PrintVariables(fp);
        fclose(fp);
        return TRUE;
    }
    return FALSE;
}

TESS_API BOOL TESS_CALL TessBaseAPIGetVariableAsString(TessBaseAPI* handle, const char* name, STRING* val)
{
    return handle->GetVariableAsString(name, val) ? TRUE : FALSE;
}

TESS_API int TESS_CALL TessBaseAPIInit4(TessBaseAPI* handle, const char* datapath, const char* language,
    TessOcrEngineMode mode, char** configs, int configs_size,
    char** vars_vec, char** vars_values, size_t vars_vec_size,
    BOOL set_only_non_debug_params)
{
    GenericVector<STRING> varNames;
    GenericVector<STRING> varValues;
    if (vars_vec != nullptr && vars_values != nullptr) {
        for (size_t i = 0; i < vars_vec_size; i++) {
            varNames.push_back(STRING(vars_vec[i]));
            varValues.push_back(STRING(vars_values[i]));
        }
    }

    return handle->Init(datapath, language, mode, configs, configs_size, &varNames, &varValues, set_only_non_debug_params);
}


TESS_API int TESS_CALL TessBaseAPIInit1(TessBaseAPI* handle, const char* datapath, const char* language, TessOcrEngineMode oem,
                                        char** configs, int configs_size)
{
    return handle->Init(datapath, language, oem, configs, configs_size, nullptr, nullptr, false);
}

TESS_API int TESS_CALL TessBaseAPIInit2(TessBaseAPI* handle, const char* datapath, const char* language, TessOcrEngineMode oem)
{
    return handle->Init(datapath, language, oem);
}

TESS_API int TESS_CALL TessBaseAPIInit3(TessBaseAPI* handle, const char* datapath, const char* language)
{
    return handle->Init(datapath, language);
}

TESS_API const char* TESS_CALL TessBaseAPIGetInitLanguagesAsString(const TessBaseAPI* handle)
{
    return handle->GetInitLanguagesAsString();
}

TESS_API char** TESS_CALL TessBaseAPIGetLoadedLanguagesAsVector(const TessBaseAPI* handle)
{
    GenericVector<STRING> languages;
    handle->GetLoadedLanguagesAsVector(&languages);
    char** arr = new char*[languages.size() + 1];
    for (int index = 0; index < languages.size(); ++index)
        arr[index] = languages[index].strdup();
    arr[languages.size()] = nullptr;
    return arr;
}

TESS_API char** TESS_CALL TessBaseAPIGetAvailableLanguagesAsVector(const TessBaseAPI* handle)
{
    GenericVector<STRING> languages;
    handle->GetAvailableLanguagesAsVector(&languages);
    char** arr = new char*[languages.size() + 1];
    for (int index = 0; index < languages.size(); ++index)
        arr[index] = languages[index].strdup();
    arr[languages.size()] = nullptr;
    return arr;
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API int TESS_CALL TessBaseAPIInitLangMod(TessBaseAPI* handle, const char* datapath, const char* language)
{
    return handle->InitLangMod(datapath, language);
}
#endif

TESS_API void TESS_CALL TessBaseAPIInitForAnalysePage(TessBaseAPI* handle)
{
    handle->InitForAnalysePage();
}

TESS_API void TESS_CALL TessBaseAPIReadConfigFile(TessBaseAPI* handle, const char* filename)
{
    handle->ReadConfigFile(filename);
}

TESS_API void TESS_CALL TessBaseAPIReadDebugConfigFile(TessBaseAPI* handle, const char* filename)
{
    handle->ReadDebugConfigFile(filename);
}

TESS_API void TESS_CALL TessBaseAPISetPageSegMode(TessBaseAPI* handle, TessPageSegMode mode)
{
    handle->SetPageSegMode(mode);
}

TESS_API TessPageSegMode TESS_CALL TessBaseAPIGetPageSegMode(const TessBaseAPI* handle)
{
    return handle->GetPageSegMode();
}

TESS_API char* TESS_CALL TessBaseAPIRect(TessBaseAPI* handle, const unsigned char* imagedata,
                                               int bytes_per_pixel, int bytes_per_line,
                                               int left, int top, int width, int height)
{
    return handle->TesseractRect(imagedata, bytes_per_pixel, bytes_per_line, left, top, width, height);
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API void TESS_CALL TessBaseAPIClearAdaptiveClassifier(TessBaseAPI* handle)
{
    handle->ClearAdaptiveClassifier();
}
#endif

TESS_API void TESS_CALL TessBaseAPISetImage(TessBaseAPI* handle, const unsigned char* imagedata, int width, int height,
                                                  int bytes_per_pixel, int bytes_per_line)
{
    handle->SetImage(imagedata, width, height, bytes_per_pixel, bytes_per_line);
}

TESS_API void TESS_CALL TessBaseAPISetImage2(TessBaseAPI* handle, struct Pix* pix)
{
    return handle->SetImage(pix);
}

TESS_API void TESS_CALL TessBaseAPISetSourceResolution(TessBaseAPI* handle, int ppi)
{
    handle->SetSourceResolution(ppi);
}

TESS_API void TESS_CALL TessBaseAPISetRectangle(TessBaseAPI* handle, int left, int top, int width, int height)
{
    handle->SetRectangle(left, top, width, height);
}

TESS_API void TESS_CALL TessBaseAPISetThresholder(TessBaseAPI* handle, TessImageThresholder* thresholder)
{
    handle->SetThresholder(thresholder);
}

TESS_API struct Pix* TESS_CALL TessBaseAPIGetThresholdedImage(TessBaseAPI* handle)
{
    return handle->GetThresholdedImage();
}

TESS_API struct Boxa* TESS_CALL TessBaseAPIGetRegions(TessBaseAPI* handle, struct Pixa** pixa)
{
    return handle->GetRegions(pixa);
}

TESS_API struct Boxa* TESS_CALL TessBaseAPIGetTextlines(TessBaseAPI* handle, struct Pixa** pixa, int** blockids)
{
    return handle->GetTextlines(pixa, blockids);
}

TESS_API struct Boxa* TESS_CALL TessBaseAPIGetTextlines1(TessBaseAPI* handle, const BOOL raw_image, const int raw_padding,
                                                                  struct Pixa** pixa, int** blockids, int** paraids)
{
    return handle->GetTextlines(raw_image, raw_padding, pixa, blockids, paraids);
}

TESS_API struct Boxa* TESS_CALL TessBaseAPIGetStrips(TessBaseAPI* handle, struct Pixa** pixa, int** blockids)
{
    return handle->GetStrips(pixa, blockids);
}

TESS_API struct Boxa* TESS_CALL TessBaseAPIGetWords(TessBaseAPI* handle, struct Pixa** pixa)
{
    return handle->GetWords(pixa);
}

TESS_API struct Boxa* TESS_CALL TessBaseAPIGetConnectedComponents(TessBaseAPI* handle, struct Pixa** cc)
{
    return handle->GetConnectedComponents(cc);
}

TESS_API struct Boxa* TESS_CALL TessBaseAPIGetComponentImages(TessBaseAPI* handle, TessPageIteratorLevel level, BOOL text_only, struct Pixa** pixa, int** blockids)
{
    return handle->GetComponentImages(level, text_only != FALSE, pixa, blockids);
}

TESS_API struct Boxa*
TESS_CALL TessBaseAPIGetComponentImages1(TessBaseAPI* handle, const TessPageIteratorLevel level, const BOOL text_only,
                                         const BOOL raw_image, const int raw_padding,
                                         struct Pixa** pixa, int** blockids, int** paraids)
{
    return handle->GetComponentImages(level, text_only != FALSE, raw_image, raw_padding, pixa, blockids, paraids);
}

TESS_API int TESS_CALL TessBaseAPIGetThresholdedImageScaleFactor(const TessBaseAPI* handle)
{
    return handle->GetThresholdedImageScaleFactor();
}

TESS_API TessPageIterator* TESS_CALL TessBaseAPIAnalyseLayout(TessBaseAPI* handle)
{
    return handle->AnalyseLayout();
}

TESS_API int TESS_CALL TessBaseAPIRecognize(TessBaseAPI* handle, ETEXT_DESC* monitor)
{
    return handle->Recognize(monitor);
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API int TESS_CALL TessBaseAPIRecognizeForChopTest(TessBaseAPI* handle, ETEXT_DESC* monitor)
{
    return handle->RecognizeForChopTest(monitor);
}
#endif

TESS_API BOOL TESS_CALL TessBaseAPIProcessPages(TessBaseAPI* handle, const char* filename, const char* retry_config,
                                                int timeout_millisec, TessResultRenderer* renderer)
{
    if (handle->ProcessPages(filename, retry_config, timeout_millisec, renderer))
        return TRUE;
    else
        return FALSE;
}

TESS_API BOOL TESS_CALL TessBaseAPIProcessPage(TessBaseAPI* handle, struct Pix* pix, int page_index, const char* filename,
                                               const char* retry_config, int timeout_millisec, TessResultRenderer* renderer)
{
    if (handle->ProcessPage(pix, page_index, filename, retry_config, timeout_millisec, renderer))
        return TRUE;
    else
        return FALSE;
}

TESS_API TessResultIterator* TESS_CALL TessBaseAPIGetIterator(TessBaseAPI* handle)
{
    return handle->GetIterator();
}

TESS_API TessMutableIterator* TESS_CALL TessBaseAPIGetMutableIterator(TessBaseAPI* handle)
{
    return handle->GetMutableIterator();
}

TESS_API char* TESS_CALL TessBaseAPIGetUTF8Text(TessBaseAPI* handle)
{
    return handle->GetUTF8Text();
}

TESS_API char* TESS_CALL TessBaseAPIGetHOCRText(TessBaseAPI* handle, int page_number)
{
    return handle->GetHOCRText(nullptr, page_number);
}

TESS_API char* TESS_CALL TessBaseAPIGetBoxText(TessBaseAPI* handle, int page_number)
{
    return handle->GetBoxText(page_number);
}

TESS_API char* TESS_CALL TessBaseAPIGetUNLVText(TessBaseAPI* handle)
{
    return handle->GetUNLVText();
}

TESS_API int TESS_CALL TessBaseAPIMeanTextConf(TessBaseAPI* handle)
{
    return handle->MeanTextConf();
}

TESS_API int* TESS_CALL TessBaseAPIAllWordConfidences(TessBaseAPI* handle)
{
    return handle->AllWordConfidences();
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API BOOL TESS_CALL TessBaseAPIAdaptToWordStr(TessBaseAPI* handle, TessPageSegMode mode, const char* wordstr)
{
    return handle->AdaptToWordStr(mode, wordstr) ? TRUE : FALSE;
}
#endif

TESS_API void TESS_CALL TessBaseAPIClear(TessBaseAPI* handle)
{
    handle->Clear();
}

TESS_API void TESS_CALL TessBaseAPIEnd(TessBaseAPI* handle)
{
    handle->End();
}

TESS_API int TESS_CALL TessBaseAPIIsValidWord(TessBaseAPI* handle, const char* word)
{
    return handle->IsValidWord(word);
}

TESS_API BOOL TESS_CALL TessBaseAPIGetTextDirection(TessBaseAPI* handle, int* out_offset, float* out_slope)
{
    return handle->GetTextDirection(out_offset, out_slope) ? TRUE : FALSE;
}

TESS_API void TESS_CALL TessBaseAPISetDictFunc(TessBaseAPI* handle, TessDictFunc f)
{
    handle->SetDictFunc(f);
}

TESS_API void  TESS_CALL TessBaseAPIClearPersistentCache(TessBaseAPI* handle)
{
    handle->ClearPersistentCache();
}

TESS_API void TESS_CALL TessBaseAPISetProbabilityInContextFunc(TessBaseAPI* handle, TessProbabilityInContextFunc f)
{
    handle->SetProbabilityInContextFunc(f);
}

#ifndef DISABLED_LEGACY_ENGINE

TESS_API BOOL TESS_CALL TessBaseAPIDetectOrientationScript(TessBaseAPI* handle,
                                                            int* orient_deg, float* orient_conf, const char** script_name, float* script_conf)
{
    bool success;
    success = handle->DetectOrientationScript(orient_deg, orient_conf, script_name, script_conf);
    return (BOOL)success;
}

TESS_API void TESS_CALL TessBaseAPIGetFeaturesForBlob(TessBaseAPI* handle, TBLOB* blob, INT_FEATURE_STRUCT* int_features,
                                                            int* num_features, int* FeatureOutlineIndex)
{
    handle->GetFeaturesForBlob(blob, int_features, num_features, FeatureOutlineIndex);
}

TESS_API ROW* TESS_CALL TessFindRowForBox(BLOCK_LIST* blocks, int left, int top, int right, int bottom)
{
    return TessBaseAPI::FindRowForBox(blocks, left, top, right, bottom);
}

TESS_API void TESS_CALL TessBaseAPIRunAdaptiveClassifier(TessBaseAPI* handle, TBLOB* blob, int num_max_matches,
                                                               int* unichar_ids, float* ratings, int* num_matches_returned)
{
    handle->RunAdaptiveClassifier(blob, num_max_matches, unichar_ids, ratings, num_matches_returned);
}

#endif  // ndef DISABLED_LEGACY_ENGINE

TESS_API const char* TESS_CALL TessBaseAPIGetUnichar(TessBaseAPI* handle, int unichar_id)
{
    return handle->GetUnichar(unichar_id);
}

TESS_API const TessDawg* TESS_CALL TessBaseAPIGetDawg(const TessBaseAPI* handle, int i)
{
    return handle->GetDawg(i);
}

TESS_API int TESS_CALL TessBaseAPINumDawgs(const TessBaseAPI* handle)
{
    return handle->NumDawgs();
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API ROW* TESS_CALL TessMakeTessOCRRow(float baseline, float xheight, float descender, float ascender)
{
    return TessBaseAPI::MakeTessOCRRow(baseline, xheight, descender, ascender);
}

TESS_API TBLOB* TESS_CALL TessMakeTBLOB(struct Pix* pix)
{
    return TessBaseAPI::MakeTBLOB(pix);
}

TESS_API void TESS_CALL TessNormalizeTBLOB(TBLOB* tblob, ROW* row, BOOL numeric_mode)
{
    TessBaseAPI::NormalizeTBLOB(tblob, row, numeric_mode != FALSE);
}
#endif  // ndef DISABLED_LEGACY_ENGINE

TESS_API TessOcrEngineMode TESS_CALL TessBaseAPIOem(const TessBaseAPI* handle)
{
    return handle->oem();
}

TESS_API void TESS_CALL TessBaseAPIInitTruthCallback(TessBaseAPI* handle, TessTruthCallback* cb)
{
    handle->InitTruthCallback(cb);
}

TESS_API void TESS_CALL TessBaseAPISetMinOrientationMargin(TessBaseAPI* handle, double margin)
{
    handle->set_min_orientation_margin(margin);
}

TESS_API void TESS_CALL TessBaseGetBlockTextOrientations(TessBaseAPI* handle, int** block_orientation, bool** vertical_writing)
{
    handle->GetBlockTextOrientations(block_orientation, vertical_writing);
}

#ifndef DISABLED_LEGACY_ENGINE
TESS_API BLOCK_LIST* TESS_CALL TessBaseAPIFindLinesCreateBlockList(TessBaseAPI* handle)
{
    return handle->FindLinesCreateBlockList();
}
#endif

TESS_API void  TESS_CALL TessPageIteratorDelete(TessPageIterator* handle)
{
    delete handle;
}

TESS_API TessPageIterator* TESS_CALL TessPageIteratorCopy(const TessPageIterator* handle)
{
    return new TessPageIterator(*handle);
}

TESS_API void TESS_CALL TessPageIteratorBegin(TessPageIterator* handle)
{
    handle->Begin();
}

TESS_API BOOL TESS_CALL TessPageIteratorNext(TessPageIterator* handle, TessPageIteratorLevel level)
{
    return handle->Next(level) ? TRUE : FALSE;
}

TESS_API BOOL TESS_CALL TessPageIteratorIsAtBeginningOf(const TessPageIterator* handle, TessPageIteratorLevel level)
{
    return handle->IsAtBeginningOf(level) ? TRUE : FALSE;
}

TESS_API BOOL TESS_CALL TessPageIteratorIsAtFinalElement(const TessPageIterator* handle, TessPageIteratorLevel level,
                                                               TessPageIteratorLevel element)
{
    return handle->IsAtFinalElement(level, element) ? TRUE : FALSE;
}

TESS_API BOOL TESS_CALL TessPageIteratorBoundingBox(const TessPageIterator* handle, TessPageIteratorLevel level,
                                                          int* left, int* top, int* right, int* bottom)
{
    return handle->BoundingBox(level, left, top, right, bottom) ? TRUE : FALSE;
}

TESS_API TessPolyBlockType TESS_CALL TessPageIteratorBlockType(const TessPageIterator* handle)
{
    return handle->BlockType();
}

TESS_API struct Pix* TESS_CALL TessPageIteratorGetBinaryImage(const TessPageIterator* handle, TessPageIteratorLevel level)
{
    return handle->GetBinaryImage(level);
}

TESS_API struct Pix* TESS_CALL TessPageIteratorGetImage(const TessPageIterator* handle, TessPageIteratorLevel level, int padding,
                                                        struct Pix* original_image, int* left, int* top)
{
    return handle->GetImage(level, padding, original_image, left, top);
}

TESS_API BOOL TESS_CALL TessPageIteratorBaseline(const TessPageIterator* handle, TessPageIteratorLevel level,
                                                       int* x1, int* y1, int* x2, int* y2)
{
    return handle->Baseline(level, x1, y1, x2, y2) ? TRUE : FALSE;
}

TESS_API void TESS_CALL TessPageIteratorOrientation(TessPageIterator* handle, TessOrientation* orientation,
                                                          TessWritingDirection* writing_direction, TessTextlineOrder* textline_order,
                                                          float* deskew_angle)
{
    handle->Orientation(orientation, writing_direction, textline_order, deskew_angle);
}

TESS_API void  TESS_CALL TessPageIteratorParagraphInfo(TessPageIterator* handle, TessParagraphJustification* justification,
                                                       BOOL *is_list_item, BOOL *is_crown, int *first_line_indent)
{
    bool bool_is_list_item, bool_is_crown;
    handle->ParagraphInfo(justification, &bool_is_list_item, &bool_is_crown, first_line_indent);
    if (is_list_item)
        *is_list_item = bool_is_list_item ? TRUE : FALSE;
    if (is_crown)
        *is_crown = bool_is_crown ? TRUE : FALSE;
}


TESS_API void TESS_CALL TessResultIteratorDelete(TessResultIterator* handle)
{
    delete handle;
}

TESS_API TessResultIterator* TESS_CALL TessResultIteratorCopy(const TessResultIterator* handle)
{
    return new TessResultIterator(*handle);
}

TESS_API TessPageIterator* TESS_CALL TessResultIteratorGetPageIterator(TessResultIterator* handle)
{
    return handle;
}

TESS_API const TessPageIterator* TESS_CALL TessResultIteratorGetPageIteratorConst(const TessResultIterator* handle)
{
    return handle;
}

TESS_API TessChoiceIterator* TESS_CALL TessResultIteratorGetChoiceIterator(const TessResultIterator* handle)
{
    return new TessChoiceIterator(*handle);
}

TESS_API BOOL  TESS_CALL TessResultIteratorNext(TessResultIterator* handle, TessPageIteratorLevel level)
{
    return handle->Next(level);
}

TESS_API char* TESS_CALL TessResultIteratorGetUTF8Text(const TessResultIterator* handle, TessPageIteratorLevel level)
{
    return handle->GetUTF8Text(level);
}

TESS_API float TESS_CALL TessResultIteratorConfidence(const TessResultIterator* handle, TessPageIteratorLevel level)
{
    return handle->Confidence(level);
}

TESS_API const char* TESS_CALL TessResultIteratorWordRecognitionLanguage(const TessResultIterator* handle)
{
    return handle->WordRecognitionLanguage();
}

TESS_API const char* TESS_CALL TessResultIteratorWordFontAttributes(const TessResultIterator* handle, BOOL* is_bold, BOOL* is_italic,
                                                                          BOOL* is_underlined, BOOL* is_monospace, BOOL* is_serif,
                                                                          BOOL* is_smallcaps, int* pointsize, int* font_id)
{
    bool bool_is_bold, bool_is_italic, bool_is_underlined, bool_is_monospace, bool_is_serif, bool_is_smallcaps;
    const char* ret = handle->WordFontAttributes(&bool_is_bold, &bool_is_italic, &bool_is_underlined, &bool_is_monospace, &bool_is_serif,
                                                 &bool_is_smallcaps, pointsize, font_id);
    if (is_bold)
        *is_bold = bool_is_bold ? TRUE : FALSE;
    if (is_italic)
        *is_italic = bool_is_italic ? TRUE : FALSE;
    if (is_underlined)
        *is_underlined = bool_is_underlined ? TRUE : FALSE;
    if (is_monospace)
        *is_monospace = bool_is_monospace ? TRUE : FALSE;
    if (is_serif)
        *is_serif = bool_is_serif ? TRUE : FALSE;
    if (is_smallcaps)
        *is_smallcaps = bool_is_smallcaps ? TRUE : FALSE;
    return ret;
}

TESS_API BOOL TESS_CALL TessResultIteratorWordIsFromDictionary(const TessResultIterator* handle)
{
    return handle->WordIsFromDictionary() ? TRUE : FALSE;
}

TESS_API BOOL TESS_CALL TessResultIteratorWordIsNumeric(const TessResultIterator* handle)
{
    return handle->WordIsNumeric() ? TRUE : FALSE;
}

TESS_API BOOL TESS_CALL TessResultIteratorSymbolIsSuperscript(const TessResultIterator* handle)
{
    return handle->SymbolIsSuperscript() ? TRUE : FALSE;
}

TESS_API BOOL TESS_CALL TessResultIteratorSymbolIsSubscript(const TessResultIterator* handle)
{
    return handle->SymbolIsSubscript() ? TRUE : FALSE;
}

TESS_API BOOL TESS_CALL TessResultIteratorSymbolIsDropcap(const TessResultIterator* handle)
{
    return handle->SymbolIsDropcap() ? TRUE : FALSE;
}

TESS_API void TESS_CALL TessChoiceIteratorDelete(TessChoiceIterator* handle)
{
    delete handle;
}

TESS_API BOOL  TESS_CALL TessChoiceIteratorNext(TessChoiceIterator* handle)
{
    return handle->Next();
}

TESS_API const char* TESS_CALL TessChoiceIteratorGetUTF8Text(const TessChoiceIterator* handle)
{
    return handle->GetUTF8Text();
}

TESS_API float TESS_CALL TessChoiceIteratorConfidence(const TessChoiceIterator* handle)
{
    return handle->Confidence();
}

TESS_API ETEXT_DESC* TESS_CALL TessMonitorCreate()
{
    return new ETEXT_DESC();
}

TESS_API void TESS_CALL TessMonitorDelete(ETEXT_DESC* monitor)
{
    delete monitor;
}

TESS_API void TESS_CALL TessMonitorSetCancelFunc(ETEXT_DESC* monitor, TessCancelFunc cancelFunc)
{
    monitor->cancel = cancelFunc;
}

TESS_API void TESS_CALL TessMonitorSetCancelThis(ETEXT_DESC* monitor, void* cancelThis)
{
    monitor->cancel_this = cancelThis;
}

TESS_API void* TESS_CALL TessMonitorGetCancelThis(ETEXT_DESC* monitor)
{
    return monitor->cancel_this;
}

TESS_API void TESS_CALL TessMonitorSetProgressFunc(ETEXT_DESC* monitor, TessProgressFunc progressFunc)
{
    monitor->progress_callback2 = progressFunc;
}

TESS_API int TESS_CALL TessMonitorGetProgress(ETEXT_DESC* monitor)
{
    return monitor->progress;
}

TESS_API void TESS_CALL TessMonitorSetDeadlineMSecs(ETEXT_DESC* monitor, int deadline)
{
    monitor->set_deadline_msecs(deadline);
}
