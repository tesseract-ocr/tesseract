/**********************************************************************
 * File:        tuning_params.h
 * Description: Declaration of the Tuning Parameters Base Class
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

// The TuningParams class abstracts all the parameters that can be learned or
// tuned during the training process. It is a base class that all TuningParams
// classes should inherit from.

#ifndef TUNING_PARAMS_H
#define TUNING_PARAMS_H

#include <string>
#ifdef USE_STD_NAMESPACE
using std::string;
#endif

namespace tesseract {
class TuningParams {
 public:
  enum type_classifer {
    NN,
    HYBRID_NN
  };
  enum type_feature {
    BMP,
    CHEBYSHEV,
    HYBRID
  };

  TuningParams() {}
  virtual ~TuningParams() {}
  // Accessor functions
  inline double RecoWgt() const { return reco_wgt_; }
  inline double SizeWgt() const { return size_wgt_; }
  inline double CharBigramWgt() const { return char_bigrams_wgt_; }
  inline double WordUnigramWgt() const { return word_unigrams_wgt_; }
  inline int MaxSegPerChar() const { return max_seg_per_char_; }
  inline int BeamWidth() const { return beam_width_; }
  inline int TypeClassifier() const { return tp_classifier_; }
  inline int TypeFeature() const { return tp_feat_; }
  inline int ConvGridSize() const { return conv_grid_size_; }
  inline int HistWindWid() const { return hist_wind_wid_; }
  inline int MinConCompSize() const { return min_con_comp_size_; }
  inline double MaxWordAspectRatio() const { return max_word_aspect_ratio_; }
  inline double MinSpaceHeightRatio() const { return min_space_height_ratio_; }
  inline double MaxSpaceHeightRatio() const { return max_space_height_ratio_; }
  inline double CombinerRunThresh() const { return combiner_run_thresh_; }
  inline double CombinerClassifierThresh() const {
    return combiner_classifier_thresh_; }

  inline void SetRecoWgt(double wgt) { reco_wgt_ = wgt; }
  inline void SetSizeWgt(double wgt) { size_wgt_ = wgt; }
  inline void SetCharBigramWgt(double wgt) { char_bigrams_wgt_ = wgt; }
  inline void SetWordUnigramWgt(double wgt) { word_unigrams_wgt_ = wgt; }
  inline void SetMaxSegPerChar(int max_seg_per_char) {
    max_seg_per_char_ = max_seg_per_char;
  }
  inline void SetBeamWidth(int beam_width) { beam_width_ = beam_width; }
  inline void SetTypeClassifier(type_classifer tp_classifier) {
    tp_classifier_ = tp_classifier;
  }
  inline void SetTypeFeature(type_feature tp_feat) {tp_feat_ = tp_feat;}
  inline void SetHistWindWid(int hist_wind_wid) {
    hist_wind_wid_ = hist_wind_wid;
  }

  virtual bool Save(string file_name) = 0;
  virtual bool Load(string file_name) = 0;

 protected:
  // weight of recognition cost. This includes the language model cost
  double reco_wgt_;
  // weight of size cost
  double size_wgt_;
  // weight of character bigrams cost
  double char_bigrams_wgt_;
  // weight of word unigrams cost
  double word_unigrams_wgt_;
  // Maximum number of segments per character
  int max_seg_per_char_;
  // Beam width equal to the maximum number of nodes kept in the beam search
  // trellis column after pruning
  int beam_width_;
  // Classifier type: See enum type_classifer for classifier types
  type_classifer tp_classifier_;
  // Feature types: See enum type_feature for feature types
  type_feature   tp_feat_;
  // Grid size to scale a grapheme bitmap used by the BMP feature type
  int conv_grid_size_;
  // Histogram window size as a ratio of the word height used in computing
  // the vertical pixel density histogram in the segmentation algorithm
  int hist_wind_wid_;
  // Minimum possible size of a connected component
  int min_con_comp_size_;
  // Maximum aspect ratio of a word (width / height)
  double max_word_aspect_ratio_;
  // Minimum ratio relative to the line height of a gap to be considered as
  // a word break
  double min_space_height_ratio_;
  // Maximum ratio relative to the line height of a gap to be considered as
  // a definite word break
  double max_space_height_ratio_;
  // When Cube and Tesseract are run in combined mode, only run
  // combiner classifier when tesseract confidence is below this
  // threshold. When Cube is run without Tesseract, this is ignored.
  double combiner_run_thresh_;
  // When Cube and tesseract are run in combined mode, threshold on
  // output of combiner binary classifier (chosen from ROC during
  // combiner training). When Cube is run without Tesseract, this is ignored.
  double combiner_classifier_thresh_;
};
}

#endif  // TUNING_PARAMS_H
