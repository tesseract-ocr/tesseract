// SPDX-License-Identifier: Apache-2.0
// File:        osdetect.h
// Description: Orientation and script detection.
// Author:      Samuel Charron
//              Ranjith Unnikrishnan
//
// (C) Copyright 2008, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TESSERACT_CCMAIN_OSDETECT_H_
#define TESSERACT_CCMAIN_OSDETECT_H_

#include "export.h" // for TESS_API

#include <vector> // for std::vector

namespace tesseract {

class BLOBNBOX;
class BLOBNBOX_CLIST;
class BLOB_CHOICE_LIST;
class TO_BLOCK_LIST;
class UNICHARSET;

class Tesseract;

// Max number of scripts in ICU + "NULL" + Japanese and Korean + Fraktur
const int kMaxNumberOfScripts = 116 + 1 + 2 + 1;

struct OSBestResult {
  OSBestResult()
      : orientation_id(0), script_id(0), sconfidence(0.0), oconfidence(0.0) {}
  int orientation_id;
  int script_id;
  float sconfidence;
  float oconfidence;
};

struct OSResults {
  OSResults() : unicharset(nullptr) {
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < kMaxNumberOfScripts; ++j) {
        scripts_na[i][j] = 0;
      }
      orientations[i] = 0;
    }
  }
  void update_best_orientation();
  // Set the estimate of the orientation to the given id.
  void set_best_orientation(int orientation_id);
  // Update/Compute the best estimate of the script assuming the given
  // orientation id.
  void update_best_script(int orientation_id);
  // Return the index of the script with the highest score for this orientation.
  TESS_API int get_best_script(int orientation_id) const;
  // Accumulate scores with given OSResults instance and update the best script.
  void accumulate(const OSResults &osr);

  // Print statistics.
  void print_scores(void) const;
  void print_scores(int orientation_id) const;

  // Array holding scores for each orientation id [0,3].
  // Orientation ids [0..3] map to [0, 270, 180, 90] degree orientations of the
  // page respectively, where the values refer to the amount of clockwise
  // rotation to be applied to the page for the text to be upright and readable.
  float orientations[4];
  // Script confidence scores for each of 4 possible orientations.
  float scripts_na[4][kMaxNumberOfScripts];

  UNICHARSET *unicharset;
  OSBestResult best_result;
};

class OrientationDetector {
public:
  OrientationDetector(const std::vector<int> *allowed_scripts,
                      OSResults *results);
  bool detect_blob(BLOB_CHOICE_LIST *scores);
  int get_orientation();

private:
  OSResults *osr_;
  const std::vector<int> *allowed_scripts_;
};

class ScriptDetector {
public:
  ScriptDetector(const std::vector<int> *allowed_scripts, OSResults *osr,
                 tesseract::Tesseract *tess);
  void detect_blob(BLOB_CHOICE_LIST *scores);
  bool must_stop(int orientation) const;

private:
  OSResults *osr_;
  int korean_id_;
  int japanese_id_;
  int katakana_id_;
  int hiragana_id_;
  int han_id_;
  int hangul_id_;
  int latin_id_;
  int fraktur_id_;
  tesseract::Tesseract *tess_;
  const std::vector<int> *allowed_scripts_;
};

int orientation_and_script_detection(const char *filename, OSResults *,
                                     tesseract::Tesseract *);

int os_detect(TO_BLOCK_LIST *port_blocks, OSResults *osr,
              tesseract::Tesseract *tess);

int os_detect_blobs(const std::vector<int> *allowed_scripts,
                    BLOBNBOX_CLIST *blob_list, OSResults *osr,
                    tesseract::Tesseract *tess);

bool os_detect_blob(BLOBNBOX *bbox, OrientationDetector *o, ScriptDetector *s,
                    OSResults *, tesseract::Tesseract *tess);

// Helper method to convert an orientation index to its value in degrees.
// The value represents the amount of clockwise rotation in degrees that must be
// applied for the text to be upright (readable).
TESS_API int OrientationIdToValue(const int &id);

} // namespace tesseract

#endif // TESSERACT_CCMAIN_OSDETECT_H_
