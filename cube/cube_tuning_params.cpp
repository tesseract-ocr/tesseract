/**********************************************************************
 * File:        cube_tuning_params.cpp
 * Description: Implementation of the CubeTuningParameters Class
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

#include <string>
#include <vector>
#include "cube_tuning_params.h"
#include "tuning_params.h"
#include "cube_utils.h"

namespace tesseract {
CubeTuningParams::CubeTuningParams() {
  reco_wgt_ = 1.0;
  size_wgt_ = 1.0;
  char_bigrams_wgt_ = 1.0;
  word_unigrams_wgt_ = 0.0;
  max_seg_per_char_ = 8;
  beam_width_ = 32;
  tp_classifier_ = NN;
  tp_feat_ = BMP;
  conv_grid_size_ = 32;
  hist_wind_wid_ = 0;
  max_word_aspect_ratio_ = 10.0;
  min_space_height_ratio_ = 0.2;
  max_space_height_ratio_ = 0.3;
  min_con_comp_size_ = 0;
  combiner_run_thresh_ = 1.0;
  combiner_classifier_thresh_ = 0.5;
  ood_wgt_ = 1.0;
  num_wgt_ = 1.0;

}

CubeTuningParams::~CubeTuningParams() {
}

// Create an Object given the data file path and the language by loading
// the approporiate file
CubeTuningParams *CubeTuningParams::Create(const string &data_file_path,
                                           const string &lang) {
  CubeTuningParams *obj = new CubeTuningParams();

  string tuning_params_file;
  tuning_params_file = data_file_path + lang;
  tuning_params_file += ".cube.params";

  if (!obj->Load(tuning_params_file)) {
    fprintf(stderr, "Cube ERROR (CubeTuningParams::Create): unable to "
            "load tuning parameters from %s\n", tuning_params_file.c_str());
    delete obj;
    obj = NULL;
  }

  return obj;
}

// Loads the params file
bool CubeTuningParams::Load(string tuning_params_file) {
  // load the string into memory
  string param_str;

  if (CubeUtils::ReadFileToString(tuning_params_file, &param_str) == false) {
    fprintf(stderr, "Cube ERROR (CubeTuningParams::Load): unable to read "
            "file %s\n", tuning_params_file.c_str());
    return false;
  }

  // split into lines
  vector<string> str_vec;
  CubeUtils::SplitStringUsing(param_str, "\r\n", &str_vec);
  if (str_vec.size() < 8) {
    fprintf(stderr, "Cube ERROR (CubeTuningParams::Load): number of rows "
            "in parameter file is too low\n");
    return false;
  }

  // for all entries
  for (int entry = 0; entry < str_vec.size(); entry++) {
    // tokenize
    vector<string> str_tok;

    // should be only two tokens
    CubeUtils::SplitStringUsing(str_vec[entry], "=", &str_tok);
    if (str_tok.size() != 2) {
      fprintf(stderr, "Cube ERROR (CubeTuningParams::Load): invalid format in "
              "line: %s.\n", str_vec[entry].c_str());
      return false;
    }

    double val = 0;
    char peekchar = (str_tok[1].c_str())[0];
    if ((peekchar >= '0' && peekchar <= '9') ||
         peekchar == '-' || peekchar == '+' ||
         peekchar == '.') {
      // read the value
      if (sscanf(str_tok[1].c_str(), "%lf", &val) != 1) {
        fprintf(stderr, "Cube ERROR (CubeTuningParams::Load): invalid format "
                "in line: %s.\n", str_vec[entry].c_str());
        return false;
      }
    }

    // token type
    if (str_tok[0] == "RecoWgt") {
      reco_wgt_ = val;
    } else if (str_tok[0] == "SizeWgt") {
      size_wgt_ = val;
    } else if (str_tok[0] == "CharBigramsWgt") {
      char_bigrams_wgt_ = val;
    } else if (str_tok[0] == "WordUnigramsWgt") {
      word_unigrams_wgt_ = val;
    } else if (str_tok[0] == "MaxSegPerChar") {
      max_seg_per_char_ = static_cast<int>(val);
    } else if (str_tok[0] == "BeamWidth") {
      beam_width_ = static_cast<int>(val);
    } else if (str_tok[0] == "Classifier") {
      if (str_tok[1] == "NN") {
        tp_classifier_ = TuningParams::NN;
      } else if (str_tok[1] == "HYBRID_NN") {
        tp_classifier_ = TuningParams::HYBRID_NN;
      } else {
        fprintf(stderr, "Cube ERROR (CubeTuningParams::Load): invalid "
                "classifier type in line: %s.\n", str_vec[entry].c_str());
        return false;
      }
    } else if (str_tok[0] == "FeatureType") {
      if (str_tok[1] == "BMP") {
        tp_feat_ = TuningParams::BMP;
      } else if (str_tok[1] == "CHEBYSHEV") {
        tp_feat_ = TuningParams::CHEBYSHEV;
      } else if (str_tok[1] == "HYBRID") {
        tp_feat_ = TuningParams::HYBRID;
      } else {
        fprintf(stderr, "Cube ERROR (CubeTuningParams::Load): invalid feature "
                "type in line: %s.\n", str_vec[entry].c_str());
        return false;
      }
    } else if (str_tok[0] == "ConvGridSize") {
      conv_grid_size_ = static_cast<int>(val);
    } else if (str_tok[0] == "HistWindWid") {
      hist_wind_wid_ = val;
    } else if (str_tok[0] == "MinConCompSize") {
      min_con_comp_size_ = val;
    } else if (str_tok[0] == "MaxWordAspectRatio") {
      max_word_aspect_ratio_ = val;
    } else if (str_tok[0] == "MinSpaceHeightRatio") {
      min_space_height_ratio_ = val;
    } else if (str_tok[0] == "MaxSpaceHeightRatio") {
      max_space_height_ratio_ = val;
    } else if (str_tok[0] == "CombinerRunThresh") {
      combiner_run_thresh_ = val;
    } else if (str_tok[0] == "CombinerClassifierThresh") {
      combiner_classifier_thresh_ = val;
    } else if (str_tok[0] == "OODWgt") {
      ood_wgt_ = val;
    } else if (str_tok[0] == "NumWgt") {
      num_wgt_ = val;
    } else {
      fprintf(stderr, "Cube ERROR (CubeTuningParams::Load): unknown parameter "
              "in line: %s.\n", str_vec[entry].c_str());
      return false;
    }
  }

  return true;
}

// Save the parameters to a file
bool CubeTuningParams::Save(string file_name) {
  FILE *params_file = fopen(file_name.c_str(), "wb");
  if (params_file == NULL) {
    fprintf(stderr, "Cube ERROR (CubeTuningParams::Save): error opening file "
            "%s for write.\n", file_name.c_str());
    return false;
  }

  fprintf(params_file, "RecoWgt=%.4f\n", reco_wgt_);
  fprintf(params_file, "SizeWgt=%.4f\n", size_wgt_);
  fprintf(params_file, "CharBigramsWgt=%.4f\n", char_bigrams_wgt_);
  fprintf(params_file, "WordUnigramsWgt=%.4f\n", word_unigrams_wgt_);
  fprintf(params_file, "MaxSegPerChar=%d\n", max_seg_per_char_);
  fprintf(params_file, "BeamWidth=%d\n", beam_width_);
  fprintf(params_file, "ConvGridSize=%d\n", conv_grid_size_);
  fprintf(params_file, "HistWindWid=%d\n", hist_wind_wid_);
  fprintf(params_file, "MinConCompSize=%d\n", min_con_comp_size_);
  fprintf(params_file, "MaxWordAspectRatio=%.4f\n", max_word_aspect_ratio_);
  fprintf(params_file, "MinSpaceHeightRatio=%.4f\n", min_space_height_ratio_);
  fprintf(params_file, "MaxSpaceHeightRatio=%.4f\n", max_space_height_ratio_);
  fprintf(params_file, "CombinerRunThresh=%.4f\n", combiner_run_thresh_);
  fprintf(params_file, "CombinerClassifierThresh=%.4f\n",
          combiner_classifier_thresh_);
  fprintf(params_file, "OODWgt=%.4f\n", ood_wgt_);
  fprintf(params_file, "NumWgt=%.4f\n", num_wgt_);

  fclose(params_file);
  return true;
}
}
