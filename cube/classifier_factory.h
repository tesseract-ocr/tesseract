/**********************************************************************
 * File:        classifier_factory.h
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

// The CharClassifierFactory provides a single static method to create an
// instance of the desired classifier

#ifndef CHAR_CLASSIFIER_FACTORY_H
#define CHAR_CLASSIFIER_FACTORY_H

#include <string>
#include "classifier_base.h"
#include "lang_model.h"

namespace tesseract {
class CharClassifierFactory {
 public:
  // Creates a CharClassifier object of the appropriate type depending on the
  // classifier type in the settings file
  static CharClassifier *Create(const string &data_file_path,
                                const string &lang,
                                LangModel *lang_mod,
                                CharSet *char_set,
                                TuningParams *params);
};
}  // tesseract

#endif  // CHAR_CLASSIFIER_FACTORY_H
