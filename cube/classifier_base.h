/**********************************************************************
 * File:        classifier_base.h
 * Description: Declaration of the Base Character Classifier
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

// The CharClassifier class is the abstract class for any character/grapheme
// classifier.

#ifndef CHAR_CLASSIFIER_BASE_H
#define CHAR_CLASSIFIER_BASE_H

#include <string>
#include "char_samp.h"
#include "char_altlist.h"
#include "char_set.h"
#include "feature_base.h"
#include "lang_model.h"
#include "tuning_params.h"

namespace tesseract {
class CharClassifier {
 public:
  CharClassifier(CharSet *char_set, TuningParams *params,
                 FeatureBase *feat_extract) {
    char_set_ = char_set;
    params_ = params;
    feat_extract_ = feat_extract;
    fold_sets_ = NULL;
    fold_set_cnt_ = 0;
    fold_set_len_ = NULL;
    init_ = false;
    case_sensitive_ = true;
  }

  virtual ~CharClassifier() {
    if (fold_sets_  != NULL) {
      for (int fold_set = 0; fold_set < fold_set_cnt_; fold_set++) {
        delete []fold_sets_[fold_set];
      }
      delete []fold_sets_;
      fold_sets_ = NULL;
    }
    delete []fold_set_len_;
    fold_set_len_ = NULL;
    delete feat_extract_;
    feat_extract_ = NULL;
  }

  // pure virtual functions that need to be implemented by any inheriting class
  virtual CharAltList * Classify(CharSamp *char_samp) = 0;
  virtual int CharCost(CharSamp *char_samp) = 0;
  virtual bool Train(CharSamp *char_samp, int ClassID) = 0;
  virtual bool SetLearnParam(char *var_name, float val) = 0;
  virtual bool Init(const string &data_file_path, const string &lang,
                    LangModel *lang_mod) = 0;

  // accessors
  FeatureBase *FeatureExtractor() {return feat_extract_;}
  inline bool CaseSensitive() const { return case_sensitive_; }
  inline void SetCaseSensitive(bool case_sensitive) {
    case_sensitive_ = case_sensitive;
  }

 protected:
  virtual void Fold() = 0;
  virtual bool LoadFoldingSets(const string &data_file_path,
                               const string &lang,
                               LangModel *lang_mod) = 0;
  FeatureBase *feat_extract_;
  CharSet *char_set_;
  TuningParams *params_;
  int **fold_sets_;
  int *fold_set_len_;
  int fold_set_cnt_;
  bool init_;
  bool case_sensitive_;
};
}  // tesseract

#endif  // CHAR_CLASSIFIER_BASE_H
