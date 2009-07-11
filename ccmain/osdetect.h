///////////////////////////////////////////////////////////////////////
// File:        osdetect.h
// Description: Orientation and script detection.
// Author:      Samuel Charron
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
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_CCMAIN_OSDETECT_H__
#define TESSERACT_CCMAIN_OSDETECT_H__

#include "strngs.h"
#include "unicharset.h"

class TO_BLOCK_LIST;
class BLOBNBOX;
class BLOB_CHOICE_LIST;

namespace tesseract {
class Tesseract;
}

// Max number of scripts in ICU + "NULL" + Japanese and Korean + Fraktur
const int kMaxNumberOfScripts = 116 + 1 + 2 + 1;

struct OSBestResult {
  int orientation;
  const char* script;
  float sconfidence;
  float oconfidence;
};

struct OSResults {
  OSResults() {
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < kMaxNumberOfScripts; ++j)
        scripts_na[i][j] = 0;
      orientations[i] = 0;
    }
  }
  float orientations[4];
  float scripts_na[4][kMaxNumberOfScripts];

  UNICHARSET* unicharset;
  OSBestResult best_result;
};

class OrientationDetector {
 public:
  OrientationDetector(OSResults*);
  bool detect_blob(BLOB_CHOICE_LIST* scores);
  void update_best_orientation();
  int get_orientation();
 private:
  OSResults* osr_;
};

class ScriptDetector {
 public:
  ScriptDetector(OSResults*, tesseract::Tesseract* tess);
  void detect_blob(BLOB_CHOICE_LIST* scores);
  void update_best_script(int);
  void get_script() ;
  bool must_stop(int orientation);
 private:
  OSResults* osr_;
  static const char* korean_script_;
  static const char* japanese_script_;
  static const char* fraktur_script_;
  int korean_id_;
  int japanese_id_;
  int katakana_id_;
  int hiragana_id_;
  int han_id_;
  int hangul_id_;
  int latin_id_;
  int fraktur_id_;
  tesseract::Tesseract* tess_;
};

bool orientation_and_script_detection(STRING& filename,
                                      OSResults*,
                                      tesseract::Tesseract*);

bool os_detect(TO_BLOCK_LIST* port_blocks,
               OSResults* osr,
               tesseract::Tesseract* tess);

bool os_detect_blob(BLOBNBOX* bbox, OrientationDetector* o,
                    ScriptDetector* s, OSResults*,
                    tesseract::Tesseract* tess);
#endif  // TESSERACT_CCMAIN_OSDETECT_H__
