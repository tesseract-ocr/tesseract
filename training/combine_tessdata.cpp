///////////////////////////////////////////////////////////////////////
// File:        combine_tessdata
// Description: Creates a unified traineddata file from several
//              data files produced by the training process.
// Author:      Daria Antonova
// Created:     Wed Jun 03 11:26:43 PST 2009
//
// (C) Copyright 2009, Google Inc.
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

#include "tessdatamanager.h"

int main(int argc, char **argv) {
  if (!(argc == 2)) {
    printf("Usage: %s language_data_path_prefix (e.g. tessdata/eng.)", argv[0]);
    return 1;
  }
  STRING output_file = argv[1];
  output_file += kTrainedDataSuffix;
  tesseract::TessdataManager::CombineDataFiles(argv[1], output_file.string());
}
