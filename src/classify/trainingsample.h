// Copyright 2010 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
//
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

#ifndef TESSERACT_TRAINING_TRAININGSAMPLE_H_
#define TESSERACT_TRAINING_TRAININGSAMPLE_H_

#include "elst.h"
#include "featdefs.h"
#include "intfx.h"
#include "intmatcher.h"
#include "matrix.h"
#include "mf.h"
#include "mfdefs.h"
#include "picofeat.h"
#include "shapetable.h"
#include "unicharset.h"

struct Pix;

namespace tesseract {

class IntFeatureMap;
class IntFeatureSpace;
class ShapeTable;

// Number of elements of cn_feature_.
static const int kNumCNParams = 4;
// Number of ways to shift the features when randomizing.
static const int kSampleYShiftSize = 5;
// Number of ways to scale the features when randomizing.
static const int kSampleScaleSize = 3;
// Total number of different ways to manipulate the features when randomizing.
// The first and last combinations are removed to avoid an excessive
// top movement (first) and an identity transformation (last).
// WARNING: To avoid patterned duplication of samples, be sure to keep
// kSampleRandomSize prime!
// Eg with current values (kSampleYShiftSize = 5 and TkSampleScaleSize = 3)
// kSampleRandomSize is 13, which is prime.
static const int kSampleRandomSize = kSampleYShiftSize * kSampleScaleSize - 2;
// ASSERT_IS_PRIME(kSampleRandomSize) !!

class TESS_API TrainingSample : public ELIST_LINK {
public:
  TrainingSample()
      : class_id_(INVALID_UNICHAR_ID)
      , font_id_(0)
      , page_num_(0)
      , num_features_(0)
      , num_micro_features_(0)
      , outline_length_(0)
      , features_(nullptr)
      , micro_features_(nullptr)
      , weight_(1.0)
      , max_dist_(0.0)
      , sample_index_(0)
      , features_are_indexed_(false)
      , features_are_mapped_(false)
      , is_error_(false) {}
  ~TrainingSample();

  // Saves the given features into a TrainingSample. The features are copied,
  // so may be deleted afterwards. Delete the return value after use.
  static TrainingSample *CopyFromFeatures(const INT_FX_RESULT_STRUCT &fx_info,
                                          const TBOX &bounding_box,
                                          const INT_FEATURE_STRUCT *features, int num_features);
  // Returns the cn_feature as a FEATURE_STRUCT* needed by cntraining.
  FEATURE_STRUCT *GetCNFeature() const;
  // Constructs and returns a copy "randomized" by the method given by
  // the randomizer index. If index is out of [0, kSampleRandomSize) then
  // an exact copy is returned.
  TrainingSample *RandomizedCopy(int index) const;
  // Constructs and returns an exact copy.
  TrainingSample *Copy() const;

  // WARNING! Serialize/DeSerialize do not save/restore the "cache" data
  // members, which is mostly the mapped features, and the weight.
  // It is assumed these can all be reconstructed from what is saved.
  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE *fp) const;
  // Creates from the given file. Returns nullptr in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  static TrainingSample *DeSerializeCreate(bool swap, FILE *fp);
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE *fp);

  // Extracts the needed information from the CHAR_DESC_STRUCT.
  void ExtractCharDesc(int feature_type, int micro_type, int cn_type, int geo_type,
                       CHAR_DESC_STRUCT *char_desc);

  // Sets the mapped_features_ from the features_ using the provided
  // feature_space to the indexed versions of the features.
  void IndexFeatures(const IntFeatureSpace &feature_space);

  // Returns a pix representing the sample. (Int features only.)
  Image RenderToPix(const UNICHARSET *unicharset) const;
  // Displays the features in the given window with the given color.
  void DisplayFeatures(ScrollView::Color color, ScrollView *window) const;

  // Returns a pix of the original sample image. The pix is padded all round
  // by padding wherever possible.
  // The returned Pix must be pixDestroyed after use.
  // If the input page_pix is nullptr, nullptr is returned.
  Image GetSamplePix(int padding, Image page_pix) const;

  // Accessors.
  UNICHAR_ID class_id() const {
    return class_id_;
  }
  void set_class_id(int id) {
    class_id_ = id;
  }
  int font_id() const {
    return font_id_;
  }
  void set_font_id(int id) {
    font_id_ = id;
  }
  int page_num() const {
    return page_num_;
  }
  void set_page_num(int page) {
    page_num_ = page;
  }
  const TBOX &bounding_box() const {
    return bounding_box_;
  }
  void set_bounding_box(const TBOX &box) {
    bounding_box_ = box;
  }
  uint32_t num_features() const {
    return num_features_;
  }
  const INT_FEATURE_STRUCT *features() const {
    return features_;
  }
  uint32_t num_micro_features() const {
    return num_micro_features_;
  }
  const MicroFeature *micro_features() const {
    return micro_features_;
  }
  int outline_length() const {
    return outline_length_;
  }
  float cn_feature(int index) const {
    return cn_feature_[index];
  }
  int geo_feature(int index) const {
    return geo_feature_[index];
  }
  double weight() const {
    return weight_;
  }
  void set_weight(double value) {
    weight_ = value;
  }
  double max_dist() const {
    return max_dist_;
  }
  void set_max_dist(double value) {
    max_dist_ = value;
  }
  int sample_index() const {
    return sample_index_;
  }
  void set_sample_index(int value) {
    sample_index_ = value;
  }
  bool features_are_mapped() const {
    return features_are_mapped_;
  }
  const std::vector<int> &mapped_features() const {
    ASSERT_HOST(features_are_mapped_);
    return mapped_features_;
  }
  const std::vector<int> &indexed_features() const {
    ASSERT_HOST(features_are_indexed_);
    return mapped_features_;
  }
  bool is_error() const {
    return is_error_;
  }
  void set_is_error(bool value) {
    is_error_ = value;
  }

private:
  // Unichar id that this sample represents. There obviously must be a
  // reference UNICHARSET somewhere. Usually in TrainingSampleSet.
  UNICHAR_ID class_id_;
  // Font id in which this sample was printed. Refers to a fontinfo_table_ in
  // MasterTrainer.
  int font_id_;
  // Number of page that the sample came from.
  int page_num_;
  // Bounding box of sample in original image.
  TBOX bounding_box_;
  // Number of INT_FEATURE_STRUCT in features_ array.
  uint32_t num_features_;
  // Number of MicroFeature in micro_features_ array.
  uint32_t num_micro_features_;
  // Total length of outline in the baseline normalized coordinate space.
  // See comment in WERD_RES class definition for a discussion of coordinate
  // spaces.
  int outline_length_;
  // Array of features.
  INT_FEATURE_STRUCT *features_;
  // Array of features.
  MicroFeature *micro_features_;
  // The one and only CN feature. Indexed by NORM_PARAM_NAME enum.
  float cn_feature_[kNumCNParams];
  // The one and only geometric feature. (Aims at replacing cn_feature_).
  // Indexed by GeoParams enum in picofeat.h
  int geo_feature_[GeoCount];

  // Non-serialized cache data.
  // Weight used for boosting training.
  double weight_;
  // Maximum distance to other samples of same class/font used in computing
  // the canonical sample.
  double max_dist_;
  // Global index of this sample.
  int sample_index_;

public:
  // both are used in training tools
  // hide after refactoring

  // Indexed/mapped features, as indicated by the bools below.
  std::vector<int> mapped_features_;
  bool features_are_indexed_;
  bool features_are_mapped_;

private:
  // True if the last classification was an error by the current definition.
  bool is_error_;

  // Randomizing factors.
  static const int kYShiftValues[kSampleYShiftSize];
  static const double kScaleValues[kSampleScaleSize];
};

ELISTIZEH(TrainingSample)

} // namespace tesseract

#endif // TESSERACT_TRAINING_TRAININGSAMPLE_H_
