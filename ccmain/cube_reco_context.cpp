/**********************************************************************
 * File:        cube_reco_context.cpp
 * Description: Implementation of the Cube Recognition Context Class
 * Author:    Ahmad Abdulkader
 * Created:   2007
 *
 * (C) Copyright 2008, Google Inc.
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

#include <string>
#include <limits.h>

#include "cube_reco_context.h"

#include "classifier_factory.h"
#include "cube_tuning_params.h"
#include "dict.h"
#include "feature_bmp.h"
#include "tessdatamanager.h"
#include "tesseractclass.h"
#include "tess_lang_model.h"

namespace tesseract {

/**
 * Instantiate a CubeRecoContext object using a Tesseract object.
 * CubeRecoContext will not take ownership of tess_obj, but will
 * record the pointer to it and will make use of various Tesseract
 * components (language model, flags, etc). Thus the caller should
 * keep tess_obj alive so long as the instantiated CubeRecoContext is used.
 */
CubeRecoContext::CubeRecoContext(Tesseract *tess_obj) {
  tess_obj_ = tess_obj;
  lang_ = "";
  loaded_ = false;
  lang_mod_ = NULL;
  params_ = NULL;
  char_classifier_ = NULL;
  char_set_ = NULL;
  word_size_model_ = NULL;
  char_bigrams_ = NULL;
  word_unigrams_ = NULL;
  noisy_input_ = false;
  size_normalization_ = false;
}

CubeRecoContext::~CubeRecoContext() {
  delete char_classifier_;
  char_classifier_ = NULL;

  delete word_size_model_;
  word_size_model_ = NULL;

  delete char_set_;
  char_set_ = NULL;

  delete char_bigrams_;
  char_bigrams_ = NULL;

  delete word_unigrams_;
  word_unigrams_ = NULL;

  delete lang_mod_;
  lang_mod_ = NULL;

  delete params_;
  params_ = NULL;
}

/**
 * Returns the path of the data files by looking up the TESSDATA_PREFIX
 * environment variable and appending a "tessdata" directory to it
 */
bool CubeRecoContext::GetDataFilePath(string *path) const {
  *path = tess_obj_->datadir.string();
  return true;
}

/**
 * The object initialization function that loads all the necessary
 * components of a RecoContext.  TessdataManager is used to load the
 * data from [lang].traineddata file.  If TESSDATA_CUBE_UNICHARSET
 * component is present, Cube will be instantiated with the unicharset
 * specified in this component and the corresponding dictionary
 * (TESSDATA_CUBE_SYSTEM_DAWG), and will map Cube's unicharset to
 * Tesseract's. Otherwise, TessdataManager will assume that Cube will
 * be using Tesseract's unicharset and dawgs, and will load the
 * unicharset from the TESSDATA_UNICHARSET component and will load the
 * dawgs from TESSDATA_*_DAWG components.
 */
bool CubeRecoContext::Load(TessdataManager *tessdata_manager,
                           UNICHARSET *tess_unicharset) {
  ASSERT_HOST(tess_obj_ != NULL);
  tess_unicharset_ = tess_unicharset;
  string data_file_path;

  // Get the data file path.
  if (GetDataFilePath(&data_file_path) == false) {
    fprintf(stderr, "Unable to get data file path\n");
    return false;
  }

  // Get the language from the Tesseract object.
  lang_ = tess_obj_->lang.string();

  // Create the char set.
  if ((char_set_ =
       CharSet::Create(tessdata_manager, tess_unicharset)) == NULL) {
    fprintf(stderr, "Cube ERROR (CubeRecoContext::Load): unable to load "
            "CharSet\n");
    return false;
  }
  // Create the language model.
  string lm_file_name = data_file_path + lang_ + ".cube.lm";
  string lm_params;
  if (!CubeUtils::ReadFileToString(lm_file_name, &lm_params)) {
    fprintf(stderr, "Cube ERROR (CubeRecoContext::Load): unable to read cube "
            "language model params from %s\n", lm_file_name.c_str());
    return false;
  }
  lang_mod_ = new TessLangModel(lm_params, data_file_path,
                                tess_obj_->getDict().load_system_dawg,
                                tessdata_manager, this);

  // Create the optional char bigrams object.
  char_bigrams_ = CharBigrams::Create(data_file_path, lang_);

  // Create the optional word unigrams object.
  word_unigrams_ = WordUnigrams::Create(data_file_path, lang_);

  // Create the optional size model.
  word_size_model_ = WordSizeModel::Create(data_file_path, lang_,
    char_set_, Contextual());

  // Load tuning params.
  params_ = CubeTuningParams::Create(data_file_path, lang_);
  if (params_ == NULL) {
    fprintf(stderr, "Cube ERROR (CubeRecoContext::Load): unable to read "
            "CubeTuningParams from %s\n", data_file_path.c_str());
    return false;
  }

  // Create the char classifier.
  char_classifier_ = CharClassifierFactory::Create(data_file_path, lang_,
                                                   lang_mod_, char_set_,
                                                   params_);
  if (char_classifier_ == NULL) {
    fprintf(stderr, "Cube ERROR (CubeRecoContext::Load): unable to load "
            "CharClassifierFactory object from %s\n", data_file_path.c_str());
    return false;
  }

  loaded_ = true;

  return true;
}

/** Creates a CubeRecoContext object using a tesseract object */
CubeRecoContext * CubeRecoContext::Create(Tesseract *tess_obj,
                                          TessdataManager *tessdata_manager,
                                          UNICHARSET *tess_unicharset) {
  // create the object
  CubeRecoContext *cntxt = new CubeRecoContext(tess_obj);
  // load the necessary components
  if (cntxt->Load(tessdata_manager, tess_unicharset) == false) {
    fprintf(stderr, "Cube ERROR (CubeRecoContext::Create): unable to init "
            "CubeRecoContext object\n");
    delete cntxt;
    return NULL;
  }
  // success
  return cntxt;
}
}  // tesseract}
