/**********************************************************************
 * File:        cube_tuning_params.h
 * Description: Declaration of the CubeTuningParameters Class
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

// The CubeTuningParams class abstracts all the parameters that are used
// in Cube and are tuned/learned during the training process. Inherits
// from the TuningParams class.

#ifndef CUBE_TUNING_PARAMS_H
#define CUBE_TUNING_PARAMS_H

#include <string>
#include "tuning_params.h"

namespace tesseract {
class CubeTuningParams : public TuningParams {
 public:
  CubeTuningParams();
  ~CubeTuningParams();

  // Accessor functions
  inline double OODWgt() { return ood_wgt_; }
  inline double NumWgt() { return num_wgt_; }

  inline void SetOODWgt(double wgt) { ood_wgt_ = wgt; }
  inline void SetNumWgt(double wgt) { num_wgt_ = wgt; }

  // Create an object given the data file path and the language by loading
  // the approporiate file
  static CubeTuningParams * Create(const string &data_file,
                                   const string &lang);
  // Save and load the tuning parameters to a specified file
  bool Save(string file_name);
  bool Load(string file_name);

 private:
  double ood_wgt_;
  double num_wgt_;
};
}

#endif  // CUBE_TUNING_PARAMS_H
