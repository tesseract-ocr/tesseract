/**********************************************************************
 * File:        cube_reco_context.h
 * Description: Declaration of the Cube Recognition Context Class
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

// The CubeRecoContext class abstracts the Cube OCR Engine. Typically a process
// (or a thread) would create one CubeRecoContext object per language.
// The CubeRecoContext object also provides methods to get and set the
// different attribues of the Cube OCR Engine.

#ifndef CUBE_RECO_CONTEXT_H
#define CUBE_RECO_CONTEXT_H

#include <string>
#include "neural_net.h"
#include "lang_model.h"
#include "classifier_base.h"
#include "feature_base.h"
#include "char_set.h"
#include "word_size_model.h"
#include "char_bigrams.h"
#include "word_unigrams.h"

namespace tesseract {

class Tesseract;
class TessdataManager;

class CubeRecoContext {
 public:
  // Reading order enum type
  enum ReadOrder {
   L2R,
   R2L
  };

  // Instantiate using a Tesseract object
  CubeRecoContext(Tesseract *tess_obj);

  ~CubeRecoContext();

  // accessor functions
  inline const string & Lang() const { return lang_; }
  inline CharSet *CharacterSet() const { return char_set_; }
  const UNICHARSET *TessUnicharset() const { return tess_unicharset_; }
  inline CharClassifier *Classifier() const { return char_classifier_; }
  inline WordSizeModel *SizeModel() const { return word_size_model_; }
  inline CharBigrams *Bigrams() const { return char_bigrams_; }
  inline WordUnigrams *WordUnigramsObj() const { return word_unigrams_; }
  inline TuningParams *Params() const { return params_; }
  inline LangModel *LangMod() const { return lang_mod_; }

  // the reading order of the language
  inline ReadOrder ReadingOrder() const {
    return ((lang_ == "ara") ? R2L : L2R);
  }

  // does the language support case
  inline bool HasCase() const {
    return (lang_ != "ara" && lang_ != "hin");
  }

  inline bool Cursive() const {
    return (lang_ == "ara");
  }

  inline bool HasItalics() const {
    return (lang_ != "ara" && lang_ != "hin" && lang_ != "uk");
  }

  inline bool Contextual() const {
    return (lang_ == "ara");
  }

  // RecoContext runtime flags accessor functions
  inline bool SizeNormalization() const { return size_normalization_; }
  inline bool NoisyInput() const { return noisy_input_; }
  inline bool OOD() const { return lang_mod_->OOD(); }
  inline bool Numeric() const { return lang_mod_->Numeric(); }
  inline bool WordList() const { return lang_mod_->WordList(); }
  inline bool Punc() const { return lang_mod_->Punc(); }
  inline bool CaseSensitive() const {
    return char_classifier_->CaseSensitive();
  }

  inline void SetSizeNormalization(bool size_normalization) {
    size_normalization_ = size_normalization;
  }
  inline void SetNoisyInput(bool noisy_input) {
    noisy_input_ = noisy_input;
  }
  inline void SetOOD(bool ood_enabled) {
    lang_mod_->SetOOD(ood_enabled);
  }
  inline void SetNumeric(bool numeric_enabled) {
    lang_mod_->SetNumeric(numeric_enabled);
  }
  inline void SetWordList(bool word_list_enabled) {
    lang_mod_->SetWordList(word_list_enabled);
  }
  inline void SetPunc(bool punc_enabled) {
    lang_mod_->SetPunc(punc_enabled);
  }
  inline void SetCaseSensitive(bool case_sensitive) {
    char_classifier_->SetCaseSensitive(case_sensitive);
  }
  inline tesseract::Tesseract *TesseractObject() const {
    return tess_obj_;
  }

  // Returns the path of the data files
  bool GetDataFilePath(string *path) const;
  // Creates a CubeRecoContext object using a tesseract object. Data
  // files are loaded via the tessdata_manager, and the tesseract
  // unicharset is provided in order to map Cube's unicharset to
  // Tesseract's in the case where the two unicharsets differ.
  static CubeRecoContext *Create(Tesseract *tess_obj,
                                 TessdataManager *tessdata_manager,
                                 UNICHARSET *tess_unicharset);

 private:
  bool loaded_;
  string lang_;
  CharSet *char_set_;
  UNICHARSET *tess_unicharset_;
  WordSizeModel *word_size_model_;
  CharClassifier *char_classifier_;
  CharBigrams *char_bigrams_;
  WordUnigrams *word_unigrams_;
  TuningParams *params_;
  LangModel *lang_mod_;
  Tesseract *tess_obj_;  // CubeRecoContext does not own this pointer
  bool size_normalization_;
  bool noisy_input_;

  // Loads and initialized all the necessary components of a
  // CubeRecoContext. See .cpp for more details.
  bool Load(TessdataManager *tessdata_manager,
            UNICHARSET *tess_unicharset);
};
}

#endif  // CUBE_RECO_CONTEXT_H
