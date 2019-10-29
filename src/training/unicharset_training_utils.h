///////////////////////////////////////////////////////////////////////
// File:        unicharset_training_utils.h
// Description: Training utilities for UNICHARSET.
// Author:      Ray Smith
// Created:     Fri Oct 17 17:14:01 PDT 2014
//
// (C) Copyright 2014, Google Inc.
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

#ifndef TESSERACT_TRAINING_UNICHARSET_TRAINING_UTILS_H_
#define TESSERACT_TRAINING_UNICHARSET_TRAINING_UTILS_H_

#include <string>

#include <tesseract/platform.h>

class STATS;
class UNICHARSET;

namespace tesseract {

// Helper sets the character attribute properties and sets up the script table.
// Does not set tops and bottoms.
void SetupBasicProperties(bool report_errors, bool decompose,
                          UNICHARSET* unicharset);
// Default behavior is to compose, until it is proven that decomposed benefits
// at least one language.
inline void SetupBasicProperties(bool report_errors, UNICHARSET* unicharset) {
  SetupBasicProperties(report_errors, false, unicharset);
}
// Helper sets the properties from universal script unicharsets, if found.
void SetScriptProperties(const std::string& script_dir, UNICHARSET* unicharset);
// Helper gets the combined x-heights string.
std::string GetXheightString(const std::string& script_dir, const UNICHARSET& unicharset);

// Helper to set the properties for an input unicharset file, writes to the
// output file. If an appropriate script unicharset can be found in the
// script_dir directory, then the tops and bottoms are expanded using the
// script unicharset.
// If non-empty, xheight data for the fonts are written to the xheights_file.
void SetPropertiesForInputFile(const std::string& script_dir,
                               const std::string& input_unicharset_file,
                               const std::string& output_unicharset_file,
                               const std::string& output_xheights_file);

}  // namespace tesseract.

#endif  // TESSERACT_TRAINING_UNICHARSET_TRAINING_UTILS_H_
