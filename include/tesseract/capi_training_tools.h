///////////////////////////////////////////////////////////////////////
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
//
///////////////////////////////////////////////////////////////////////

#ifndef API_CAPI_TRAINING_TOOLS_H_
#define API_CAPI_TRAINING_TOOLS_H_

#include "export.h"

#ifdef __cplusplus
extern "C" {
#endif

// #if !defined(DISABLED_LEGACY_ENGINE)
TESS_API int tesseract_ambiguous_words_main(int argc, const char** argv);
TESS_API int tesseract_classifier_tester_main(int argc, const char** argv);
TESS_API int tesseract_cn_training_main(int argc, const char** argv);
TESS_API int tesseract_mf_training_main(int argc, const char** argv);
TESS_API int tesseract_shape_clustering_main(int argc, const char** argv);
// #endif
TESS_API int tesseract_combine_lang_model_main(int argc, const char** argv);
TESS_API int tesseract_combine_tessdata_main(int argc, const char** argv);
TESS_API int tesseract_dawg2wordlist_main(int argc, const char** argv);
TESS_API int tesseract_lstm_eval_main(int argc, const char** argv);
TESS_API int tesseract_lstm_training_main(int argc, const char** argv);
TESS_API int tesseract_merge_unicharsets_main(int argc, const char** argv);
TESS_API int tesseract_set_unicharset_properties_main(int argc, const char** argv);
// #if defined(PANGO_ENABLE_ENGINE)
TESS_API int tesseract_text2image_main(int argc, const char** argv);
// #endif
TESS_API int tesseract_unicharset_extractor_main(int argc, const char** argv);
TESS_API int tesseract_wordlist2dawg_main(int argc, const char** argv);

#ifdef __cplusplus
}
#endif

#endif  // API_CAPI_H_
