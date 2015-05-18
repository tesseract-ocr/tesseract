///////////////////////////////////////////////////////////////////////
// File:        params_training_featdef.cpp
// Description: Utility functions for params training features.
// Author:      David Eger
// Created:     Mon Jun 11 11:26:42 PDT 2012
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

#include <string.h>

#include "params_training_featdef.h"

namespace tesseract {

int ParamsTrainingFeatureByName(const char *name) {
  if (name == NULL)
    return -1;
  int array_size = sizeof(kParamsTrainingFeatureTypeName) /
    sizeof(kParamsTrainingFeatureTypeName[0]);
  for (int i = 0; i < array_size; i++) {
    if (kParamsTrainingFeatureTypeName[i] == NULL)
      continue;
    if (strcmp(name, kParamsTrainingFeatureTypeName[i]) == 0)
      return i;
  }
  return -1;
}

}  // namespace tesseract
