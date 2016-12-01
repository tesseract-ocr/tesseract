/**********************************************************************
 * File:        word_size_model.cpp
 * Description: Implementation of the Word Size Model Class
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

#include <math.h>
#include <string>
#include <vector>
#include "word_size_model.h"
#include "cube_utils.h"

namespace tesseract {

WordSizeModel::WordSizeModel(CharSet * char_set, bool contextual) {
  char_set_ = char_set;
  contextual_ = contextual;
}

WordSizeModel::~WordSizeModel() {
  for (int fnt = 0; fnt < font_pair_size_models_.size(); fnt++) {
    FontPairSizeInfo fnt_info = font_pair_size_models_[fnt];
    delete []fnt_info.pair_size_info[0];
    delete []fnt_info.pair_size_info;
  }
}

WordSizeModel *WordSizeModel::Create(const string &data_file_path,
                                     const string &lang,
                                     CharSet *char_set,
                                     bool contextual) {
  WordSizeModel *obj = new WordSizeModel(char_set, contextual);

  if (!obj->Init(data_file_path, lang)) {
    delete obj;
    return NULL;
  }
  return obj;
}

bool WordSizeModel::Init(const string &data_file_path, const string &lang) {
  string stats_file_name;
  stats_file_name = data_file_path + lang;
  stats_file_name += ".cube.size";

  // read file to memory
  string str_data;

  if (!CubeUtils::ReadFileToString(stats_file_name, &str_data)) {
    return false;
  }

  // split to words
  vector<string> tokens;
  CubeUtils::SplitStringUsing(str_data, "\t\r\n", &tokens);
  if (tokens.size() < 1) {
    fprintf(stderr, "Cube ERROR (WordSizeModel::Init): invalid "
            "file contents: %s\n", stats_file_name.c_str());
    return false;
  }

  font_pair_size_models_.clear();

  // token count per line depends on whether the language is contextual or not
  int token_cnt = contextual_ ?
      (kExpectedTokenCount + 4) : kExpectedTokenCount;
  // the count of size classes depends on whether the language is contextual
  // or not. For non contextual languages (Ex: Eng), it is equal to the class
  // count. For contextual languages (Ex: Ara), it is equal to the class count
  // multiplied by the position count (4: start, middle, final, isolated)
  int size_class_cnt = contextual_ ?
      (char_set_->ClassCount() * 4) : char_set_->ClassCount();
  string fnt_name = "";

  for (int tok = 0; tok < tokens.size(); tok += token_cnt) {
    // a new font, write the old font data and re-init
    if (tok == 0 || fnt_name != tokens[tok]) {
      FontPairSizeInfo fnt_info;

      fnt_info.pair_size_info = new PairSizeInfo *[size_class_cnt];

      fnt_info.pair_size_info[0] =
          new PairSizeInfo[size_class_cnt * size_class_cnt];

      memset(fnt_info.pair_size_info[0], 0, size_class_cnt * size_class_cnt *
             sizeof(PairSizeInfo));

      for (int cls = 1; cls < size_class_cnt; cls++) {
        fnt_info.pair_size_info[cls] =
            fnt_info.pair_size_info[cls - 1] + size_class_cnt;
      }

      // strip out path and extension
      string stripped_font_name = tokens[tok].substr(0, tokens[tok].find('.'));
      string::size_type strt_pos = stripped_font_name.find_last_of("/\\");
      if (strt_pos != string::npos) {
        fnt_info.font_name = stripped_font_name.substr(strt_pos);
      } else {
        fnt_info.font_name = stripped_font_name;
      }
      font_pair_size_models_.push_back(fnt_info);
    }

    // parse the data
    int cls_0;
    int cls_1;
    double delta_top;
    double wid_0;
    double hgt_0;
    double wid_1;
    double hgt_1;
    int size_code_0;
    int size_code_1;

    // read and parse the tokens
    if (contextual_) {
      int start_0;
      int end_0;
      int start_1;
      int end_1;
      // The expected format for a character size bigram is as follows:
      // ClassId0<delim>Start-flag0<delim>End-flag0<delim>String0(ignored)
      // Width0<delim>Height0<delim>
      // ClassId1<delim>Start-flag1<delim>End-flag1<delim>String1(ignored)
      // HeightDelta<delim>Width1<delim>Height0<delim>
      // In case of non-contextual languages, the Start and End flags are
      // omitted
      if (sscanf(tokens[tok + 1].c_str(), "%d", &cls_0) != 1 ||
          sscanf(tokens[tok + 2].c_str(), "%d", &start_0) != 1 ||
          sscanf(tokens[tok + 3].c_str(), "%d", &end_0) != 1 ||
          sscanf(tokens[tok + 5].c_str(), "%lf", &wid_0) != 1 ||
          sscanf(tokens[tok + 6].c_str(), "%lf", &hgt_0) != 1 ||
          sscanf(tokens[tok + 7].c_str(), "%d", &cls_1) != 1 ||
          sscanf(tokens[tok + 8].c_str(), "%d", &start_1) != 1 ||
          sscanf(tokens[tok + 9].c_str(), "%d", &end_1) != 1 ||
          sscanf(tokens[tok + 11].c_str(), "%lf", &delta_top) != 1 ||
          sscanf(tokens[tok + 12].c_str(), "%lf", &wid_1) != 1 ||
          sscanf(tokens[tok + 13].c_str(), "%lf", &hgt_1) != 1 ||
          (start_0 != 0 && start_0 != 1) || (end_0 != 0 && end_0 != 1) ||
          (start_1 != 0 && start_1 != 1) || (end_1 != 0 && end_1 != 1)) {
        fprintf(stderr, "Cube ERROR (WordSizeModel::Init): bad format at "
                "line %d\n", 1 + (tok / token_cnt));
        return false;
      }
      size_code_0 = SizeCode(cls_0, start_0, end_0);
      size_code_1 = SizeCode(cls_1, start_1, end_1);
    } else {
      if (sscanf(tokens[tok + 1].c_str(), "%d", &cls_0) != 1 ||
          sscanf(tokens[tok + 3].c_str(), "%lf", &wid_0) != 1 ||
          sscanf(tokens[tok + 4].c_str(), "%lf", &hgt_0) != 1 ||
          sscanf(tokens[tok + 5].c_str(), "%d", &cls_1) != 1 ||
          sscanf(tokens[tok + 7].c_str(), "%lf", &delta_top) != 1 ||
          sscanf(tokens[tok + 8].c_str(), "%lf", &wid_1) != 1 ||
          sscanf(tokens[tok + 9].c_str(), "%lf", &hgt_1) != 1) {
        fprintf(stderr, "Cube ERROR (WordSizeModel::Init): bad format at "
                "line %d\n", 1 + (tok / token_cnt));
        return false;
      }
      size_code_0 = cls_0;
      size_code_1 = cls_1;
    }

    // copy the data to the size tables
    FontPairSizeInfo fnt_info = font_pair_size_models_.back();
    fnt_info.pair_size_info[size_code_0][size_code_1].delta_top =
        static_cast<int>(delta_top * kShapeModelScale);
    fnt_info.pair_size_info[size_code_0][size_code_1].wid_0 =
        static_cast<int>(wid_0 * kShapeModelScale);
    fnt_info.pair_size_info[size_code_0][size_code_1].hgt_0 =
        static_cast<int>(hgt_0 * kShapeModelScale);
    fnt_info.pair_size_info[size_code_0][size_code_1].wid_1 =
        static_cast<int>(wid_1 * kShapeModelScale);
    fnt_info.pair_size_info[size_code_0][size_code_1].hgt_1 =
        static_cast<int>(hgt_1 * kShapeModelScale);

    fnt_name = tokens[tok];
  }

  return true;
}

int WordSizeModel::Cost(CharSamp **samp_array, int samp_cnt) const {
  if (samp_cnt < 2) {
    return 0;
  }
  double best_dist = static_cast<double>(WORST_COST);
  int best_fnt = -1;
  for (int fnt = 0; fnt < font_pair_size_models_.size(); fnt++) {
    const FontPairSizeInfo *fnt_info = &font_pair_size_models_[fnt];
    double mean_dist = 0;
    int pair_cnt = 0;

    for (int smp_0 = 0; smp_0 < samp_cnt; smp_0++) {
      int cls_0 = char_set_->ClassID(samp_array[smp_0]->StrLabel());
      if (cls_0 < 1) {
        continue;
      }
      // compute size code for samp 0 based on class id and position
      int size_code_0;
      if (contextual_) {
        size_code_0 = SizeCode(cls_0,
                               samp_array[smp_0]->FirstChar() == 0 ? 0 : 1,
                               samp_array[smp_0]->LastChar() == 0 ? 0 : 1);
      } else {
        size_code_0 = cls_0;
      }

      int char0_height = samp_array[smp_0]->Height();
      int char0_width = samp_array[smp_0]->Width();
      int char0_top = samp_array[smp_0]->Top();

      for (int smp_1 = smp_0 + 1; smp_1 < samp_cnt; smp_1++) {
        int cls_1 = char_set_->ClassID(samp_array[smp_1]->StrLabel());
        if (cls_1 < 1) {
          continue;
        }
        // compute size code for samp 0 based on class id and position
        int size_code_1;
        if (contextual_) {
          size_code_1 = SizeCode(cls_1,
                                 samp_array[smp_1]->FirstChar() == 0 ? 0 : 1,
                                 samp_array[smp_1]->LastChar() == 0 ? 0 : 1);
        } else {
          size_code_1 = cls_1;
        }
        double dist = PairCost(
            char0_width, char0_height, char0_top, samp_array[smp_1]->Width(),
            samp_array[smp_1]->Height(), samp_array[smp_1]->Top(),
            fnt_info->pair_size_info[size_code_0][size_code_1]);
        if (dist > 0) {
          mean_dist += dist;
          pair_cnt++;
        }
      }  // smp_1
    }  // smp_0
    if (pair_cnt == 0) {
      continue;
    }
    mean_dist /= pair_cnt;
    if (best_fnt == -1 || mean_dist < best_dist) {
      best_dist = mean_dist;
      best_fnt = fnt;
    }
  }
  if (best_fnt == -1) {
    return static_cast<int>(WORST_COST);
  } else {
    return static_cast<int>(best_dist);
  }
}

double WordSizeModel::PairCost(int width_0, int height_0, int top_0,
                               int width_1, int height_1, int top_1,
                               const PairSizeInfo& pair_info) {
  double scale_factor = static_cast<double>(pair_info.hgt_0) /
      static_cast<double>(height_0);
  double dist = 0.0;
  if (scale_factor > 0) {
    double norm_width_0 = width_0 * scale_factor;
    double norm_width_1 = width_1 * scale_factor;
    double norm_height_1 = height_1 * scale_factor;
    double norm_delta_top = (top_1 - top_0) * scale_factor;

    // accumulate the distance between the model character and the
    // predicted one on all dimensions of the pair
    dist += fabs(pair_info.wid_0 - norm_width_0);
    dist += fabs(pair_info.wid_1 - norm_width_1);
    dist += fabs(pair_info.hgt_1 - norm_height_1);
    dist += fabs(pair_info.delta_top - norm_delta_top);
  }
  return dist;
}
}  // namespace tesseract
