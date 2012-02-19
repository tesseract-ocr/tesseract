/**********************************************************************
 * File:        tesseract_cube_combiner.h
 * Description: Declaration of the Tesseract & Cube results combiner Class
 * Author:    Ahmad Abdulkader
 * Created:   2008
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

// The TesseractCubeCombiner class provides the functionality of combining
// the recognition results of Tesseract and Cube at the word level

#ifndef TESSERACT_CCMAIN_TESSERACT_CUBE_COMBINER_H
#define TESSERACT_CCMAIN_TESSERACT_CUBE_COMBINER_H

#include <string>
#include <vector>
#include "pageres.h"

#ifdef _WIN32
#include <windows.h>
using namespace std;
#endif

#ifdef USE_STD_NAMESPACE
using std::string;
using std::vector;
#endif

namespace tesseract {

class CubeObject;
class NeuralNet;
class CubeRecoContext;
class WordAltList;

class TesseractCubeCombiner {
 public:
  explicit TesseractCubeCombiner(CubeRecoContext *cube_cntxt);
  virtual ~TesseractCubeCombiner();

  // There are 2 public methods for combining the results of tesseract
  // and cube. Both return the probability that the Tesseract result is
  // correct. The difference between the two interfaces is in how the
  // passed-in CubeObject is used.

  // The CubeObject parameter is used for 2 purposes: 1) to retrieve
  // cube's alt list, and 2) to compute cube's word cost for the
  // tesseract result. Both uses may modify the state of the
  // CubeObject (including the BeamSearch state) with a call to
  // RecognizeWord().
  float CombineResults(WERD_RES *tess_res, CubeObject *cube_obj);

  // The alt_list parameter is expected to have been extracted from the
  // CubeObject that recognized the word to be combined. The cube_obj
  // parameter passed in is a separate instance to be used only by
  // the combiner.
  float CombineResults(WERD_RES *tess_res, CubeObject *cube_obj,
                       WordAltList *alt_list);

  // Public method for computing the combiner features. The agreement
  // output parameter will be true if both answers are identical,
  // false otherwise. Modifies the cube_alt_list, so no assumptions
  // should be made about its state upon return.
  bool ComputeCombinerFeatures(const string &tess_res,
                               int tess_confidence,
                               CubeObject *cube_obj,
                               WordAltList *cube_alt_list,
                               vector<double> *features,
                               bool *agreement);

  // Is the word valid according to Tesseract's language model
  bool ValidWord(const string &str);

  // Loads the combiner neural network from file, using cube_cntxt_
  // to find path.
  bool LoadCombinerNet();
 private:
  // Normalize a UTF-8 string. Converts the UTF-8 string to UTF32 and optionally
  // strips punc and/or normalizes case and then converts back
  string NormalizeString(const string &str, bool remove_punc, bool norm_case);

  // Compares 2 strings after optionally normalizing them and or stripping
  // punctuation
  int CompareStrings(const string &str1, const string &str2, bool ignore_punc,
                     bool norm_case);

  NeuralNet *combiner_net_;  // pointer to the combiner NeuralNet object
  CubeRecoContext *cube_cntxt_;  // used for language ID and data paths
};
}

#endif  // TESSERACT_CCMAIN_TESSERACT_CUBE_COMBINER_H
