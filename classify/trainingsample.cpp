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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "trainingsample.h"

#include <math.h>
#include "allheaders.h"
#include "helpers.h"
#include "intfeaturemap.h"
#include "normfeat.h"
#include "shapetable.h"

namespace tesseract {

ELISTIZE(TrainingSample)

// Center of randomizing operations.
const int kRandomizingCenter = 128;

// Randomizing factors.
const int TrainingSample::kYShiftValues[kSampleYShiftSize] = {
    6, 3, -3, -6, 0
};
const double TrainingSample::kScaleValues[kSampleScaleSize] = {
    1.0625, 0.9375, 1.0
};

TrainingSample::~TrainingSample() {
  delete [] features_;
  delete [] micro_features_;
}

// WARNING! Serialize/DeSerialize do not save/restore the "cache" data
// members, which is mostly the mapped features, and the weight.
// It is assumed these can all be reconstructed from what is saved.
// Writes to the given file. Returns false in case of error.
bool TrainingSample::Serialize(FILE* fp) const {
  if (fwrite(&class_id_, sizeof(class_id_), 1, fp) != 1) return false;
  if (fwrite(&font_id_, sizeof(font_id_), 1, fp) != 1) return false;
  if (fwrite(&page_num_, sizeof(page_num_), 1, fp) != 1) return false;
  if (!bounding_box_.Serialize(fp)) return false;
  if (fwrite(&num_features_, sizeof(num_features_), 1, fp) != 1) return false;
  if (fwrite(&num_micro_features_, sizeof(num_micro_features_), 1, fp) != 1)
    return false;
  if (fwrite(&outline_length_, sizeof(outline_length_), 1, fp) != 1)
    return false;
  if (fwrite(features_, sizeof(*features_), num_features_, fp) != num_features_)
    return false;
  if (fwrite(micro_features_, sizeof(*micro_features_), num_micro_features_,
             fp) != num_micro_features_)
    return false;
  if (fwrite(cn_feature_, sizeof(*cn_feature_), kNumCNParams, fp) !=
      kNumCNParams) return false;
  if (fwrite(geo_feature_, sizeof(*geo_feature_), GeoCount, fp) != GeoCount)
    return false;
  return true;
}

// Creates from the given file. Returns NULL in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
TrainingSample* TrainingSample::DeSerializeCreate(bool swap, FILE* fp) {
  TrainingSample* sample = new TrainingSample;
  if (sample->DeSerialize(swap, fp)) return sample;
  delete sample;
  return NULL;
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool TrainingSample::DeSerialize(bool swap, FILE* fp) {
  if (fread(&class_id_, sizeof(class_id_), 1, fp) != 1) return false;
  if (fread(&font_id_, sizeof(font_id_), 1, fp) != 1) return false;
  if (fread(&page_num_, sizeof(page_num_), 1, fp) != 1) return false;
  if (!bounding_box_.DeSerialize(swap, fp)) return false;
  if (fread(&num_features_, sizeof(num_features_), 1, fp) != 1) return false;
  if (fread(&num_micro_features_, sizeof(num_micro_features_), 1, fp) != 1)
    return false;
  if (fread(&outline_length_, sizeof(outline_length_), 1, fp) != 1)
    return false;
  if (swap) {
    ReverseN(&class_id_, sizeof(class_id_));
    ReverseN(&num_features_, sizeof(num_features_));
    ReverseN(&num_micro_features_, sizeof(num_micro_features_));
    ReverseN(&outline_length_, sizeof(outline_length_));
  }
  delete [] features_;
  features_ = new INT_FEATURE_STRUCT[num_features_];
  if (fread(features_, sizeof(*features_), num_features_, fp) != num_features_)
    return false;
  delete [] micro_features_;
  micro_features_ = new MicroFeature[num_micro_features_];
  if (fread(micro_features_, sizeof(*micro_features_), num_micro_features_,
            fp) != num_micro_features_)
    return false;
  if (fread(cn_feature_, sizeof(*cn_feature_), kNumCNParams, fp) !=
            kNumCNParams) return false;
  if (fread(geo_feature_, sizeof(*geo_feature_), GeoCount, fp) != GeoCount)
    return false;
  return true;
}

// Saves the given features into a TrainingSample.
TrainingSample* TrainingSample::CopyFromFeatures(
    const INT_FX_RESULT_STRUCT& fx_info,
    const TBOX& bounding_box,
    const INT_FEATURE_STRUCT* features,
    int num_features) {
  TrainingSample* sample = new TrainingSample;
  sample->num_features_ = num_features;
  sample->features_ = new INT_FEATURE_STRUCT[num_features];
  sample->outline_length_ = fx_info.Length;
  memcpy(sample->features_, features, num_features * sizeof(features[0]));
  sample->geo_feature_[GeoBottom] = bounding_box.bottom();
  sample->geo_feature_[GeoTop] = bounding_box.top();
  sample->geo_feature_[GeoWidth] = bounding_box.width();

  // Generate the cn_feature_ from the fx_info.
  sample->cn_feature_[CharNormY] =
      MF_SCALE_FACTOR * (fx_info.Ymean - kBlnBaselineOffset);
  sample->cn_feature_[CharNormLength] =
      MF_SCALE_FACTOR * fx_info.Length / LENGTH_COMPRESSION;
  sample->cn_feature_[CharNormRx] = MF_SCALE_FACTOR * fx_info.Rx;
  sample->cn_feature_[CharNormRy] = MF_SCALE_FACTOR * fx_info.Ry;

  sample->features_are_indexed_ = false;
  sample->features_are_mapped_ = false;
  return sample;
}

// Returns the cn_feature as a FEATURE_STRUCT* needed by cntraining.
FEATURE_STRUCT* TrainingSample::GetCNFeature() const {
  FEATURE feature = NewFeature(&CharNormDesc);
  for (int i = 0; i < kNumCNParams; ++i)
    feature->Params[i] = cn_feature_[i];
  return feature;
}

// Constructs and returns a copy randomized by the method given by
// the randomizer index. If index is out of [0, kSampleRandomSize) then
// an exact copy is returned.
TrainingSample* TrainingSample::RandomizedCopy(int index) const {
  TrainingSample* sample = Copy();
  if (index >= 0 && index < kSampleRandomSize) {
    ++index;  // Remove the first combination.
    int yshift = kYShiftValues[index / kSampleScaleSize];
    double scaling = kScaleValues[index % kSampleScaleSize];
    for (int i = 0; i < num_features_; ++i) {
      double result = (features_[i].X - kRandomizingCenter) * scaling;
      result += kRandomizingCenter;
      sample->features_[i].X = ClipToRange(static_cast<int>(result + 0.5), 0,
                                           MAX_UINT8);
      result = (features_[i].Y - kRandomizingCenter) * scaling;
      result += kRandomizingCenter + yshift;
      sample->features_[i].Y = ClipToRange(static_cast<int>(result + 0.5), 0,
                                           MAX_UINT8);
    }
  }
  return sample;
}

// Constructs and returns an exact copy.
TrainingSample* TrainingSample::Copy() const {
  TrainingSample* sample = new TrainingSample;
  sample->class_id_ = class_id_;
  sample->font_id_ = font_id_;
  sample->weight_ = weight_;
  sample->sample_index_ = sample_index_;
  sample->num_features_ = num_features_;
  if (num_features_ > 0) {
    sample->features_ = new INT_FEATURE_STRUCT[num_features_];
    memcpy(sample->features_, features_, num_features_ * sizeof(features_[0]));
  }
  sample->num_micro_features_ = num_micro_features_;
  if (num_micro_features_ > 0) {
    sample->micro_features_ = new MicroFeature[num_micro_features_];
    memcpy(sample->micro_features_, micro_features_,
           num_micro_features_ * sizeof(micro_features_[0]));
  }
  memcpy(sample->cn_feature_, cn_feature_, sizeof(*cn_feature_) * kNumCNParams);
  memcpy(sample->geo_feature_, geo_feature_, sizeof(*geo_feature_) * GeoCount);
  return sample;
}

// Extracts the needed information from the CHAR_DESC_STRUCT.
void TrainingSample::ExtractCharDesc(int int_feature_type,
                                     int micro_type,
                                     int cn_type,
                                     int geo_type,
                                     CHAR_DESC_STRUCT* char_desc) {
  // Extract the INT features.
  if (features_ != NULL) delete [] features_;
  FEATURE_SET_STRUCT* char_features = char_desc->FeatureSets[int_feature_type];
  if (char_features == NULL) {
    tprintf("Error: no features to train on of type %s\n",
            kIntFeatureType);
    num_features_ = 0;
    features_ = NULL;
  } else {
    num_features_ = char_features->NumFeatures;
    features_ = new INT_FEATURE_STRUCT[num_features_];
    for (int f = 0; f < num_features_; ++f) {
      features_[f].X =
          static_cast<uinT8>(char_features->Features[f]->Params[IntX]);
      features_[f].Y =
          static_cast<uinT8>(char_features->Features[f]->Params[IntY]);
      features_[f].Theta =
          static_cast<uinT8>(char_features->Features[f]->Params[IntDir]);
      features_[f].CP_misses = 0;
    }
  }
  // Extract the Micro features.
  if (micro_features_ != NULL) delete [] micro_features_;
  char_features = char_desc->FeatureSets[micro_type];
  if (char_features == NULL) {
    tprintf("Error: no features to train on of type %s\n",
            kMicroFeatureType);
    num_micro_features_ = 0;
    micro_features_ = NULL;
  } else {
    num_micro_features_ = char_features->NumFeatures;
    micro_features_ = new MicroFeature[num_micro_features_];
    for (int f = 0; f < num_micro_features_; ++f) {
      for (int d = 0; d < MFCount; ++d) {
        micro_features_[f][d] = char_features->Features[f]->Params[d];
      }
    }
  }
  // Extract the CN feature.
  char_features = char_desc->FeatureSets[cn_type];
  if (char_features == NULL) {
    tprintf("Error: no CN feature to train on.\n");
  } else {
    ASSERT_HOST(char_features->NumFeatures == 1);
    cn_feature_[CharNormY] = char_features->Features[0]->Params[CharNormY];
    cn_feature_[CharNormLength] =
        char_features->Features[0]->Params[CharNormLength];
    cn_feature_[CharNormRx] = char_features->Features[0]->Params[CharNormRx];
    cn_feature_[CharNormRy] = char_features->Features[0]->Params[CharNormRy];
  }
  // Extract the Geo feature.
  char_features = char_desc->FeatureSets[geo_type];
  if (char_features == NULL) {
    tprintf("Error: no Geo feature to train on.\n");
  } else {
    ASSERT_HOST(char_features->NumFeatures == 1);
    geo_feature_[GeoBottom] = char_features->Features[0]->Params[GeoBottom];
    geo_feature_[GeoTop] = char_features->Features[0]->Params[GeoTop];
    geo_feature_[GeoWidth] = char_features->Features[0]->Params[GeoWidth];
  }
  features_are_indexed_ = false;
  features_are_mapped_ = false;
}

// Sets the mapped_features_ from the features_ using the provided
// feature_space to the indexed versions of the features.
void TrainingSample::IndexFeatures(const IntFeatureSpace& feature_space) {
  GenericVector<int> indexed_features;
  feature_space.IndexAndSortFeatures(features_, num_features_,
                                     &mapped_features_);
  features_are_indexed_ = true;
  features_are_mapped_ = false;
}

// Sets the mapped_features_ from the features using the provided
// feature_map.
void TrainingSample::MapFeatures(const IntFeatureMap& feature_map) {
  GenericVector<int> indexed_features;
  feature_map.feature_space().IndexAndSortFeatures(features_, num_features_,
                                                   &indexed_features);
  feature_map.MapIndexedFeatures(indexed_features, &mapped_features_);
  features_are_indexed_ = false;
  features_are_mapped_ = true;
}

// Returns a pix representing the sample. (Int features only.)
Pix* TrainingSample::RenderToPix(const UNICHARSET* unicharset) const {
  Pix* pix = pixCreate(kIntFeatureExtent, kIntFeatureExtent, 1);
  for (int f = 0; f < num_features_; ++f) {
    int start_x = features_[f].X;
    int start_y = kIntFeatureExtent - features_[f].Y;
    double dx = cos((features_[f].Theta / 256.0) * 2.0 * PI - PI);
    double dy = -sin((features_[f].Theta / 256.0) * 2.0 * PI - PI);
    for (int i = 0; i <= 5; ++i) {
      int x = static_cast<int>(start_x + dx * i);
      int y = static_cast<int>(start_y + dy * i);
      if (x >= 0 && x < 256 && y >= 0 && y < 256)
        pixSetPixel(pix, x, y, 1);
    }
  }
  if (unicharset != NULL)
    pixSetText(pix, unicharset->id_to_unichar(class_id_));
  return pix;
}

// Displays the features in the given window with the given color.
void TrainingSample::DisplayFeatures(ScrollView::Color color,
                                     ScrollView* window) const {
  #ifndef GRAPHICS_DISABLED
  for (int f = 0; f < num_features_; ++f) {
    RenderIntFeature(window, &features_[f], color);
  }
  #endif  // GRAPHICS_DISABLED
}

// Returns a pix of the original sample image. The pix is padded all round
// by padding wherever possible.
// The returned Pix must be pixDestroyed after use.
// If the input page_pix is NULL, NULL is returned.
Pix* TrainingSample::GetSamplePix(int padding, Pix* page_pix) const {
  if (page_pix == NULL)
    return NULL;
  int page_width = pixGetWidth(page_pix);
  int page_height = pixGetHeight(page_pix);
  TBOX padded_box = bounding_box();
  padded_box.pad(padding, padding);
  // Clip the padded_box to the limits of the page
  TBOX page_box(0, 0, page_width, page_height);
  padded_box &= page_box;
  Box* box = boxCreate(page_box.left(), page_height - page_box.top(),
                       page_box.width(), page_box.height());
  Pix* sample_pix = pixClipRectangle(page_pix, box, NULL);
  boxDestroy(&box);
  return sample_pix;
}

}  // namespace tesseract
