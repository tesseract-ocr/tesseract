// Copyright 2011 Google Inc. All Rights Reserved.
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

#include "sampleiterator.h"

#include "indexmapbidi.h"
#include "shapetable.h"
#include "trainingsample.h"
#include "trainingsampleset.h"

namespace tesseract {

// ================== SampleIterator Implementation =================

SampleIterator::SampleIterator()
  : charset_map_(NULL),
    shape_table_(NULL),
    sample_set_(NULL),
    randomize_(false),
    owned_shape_table_(NULL) {
  num_shapes_ = 0;
  Begin();
}

SampleIterator::~SampleIterator() {
  Clear();
}

void SampleIterator::Clear() {
  delete owned_shape_table_;
  owned_shape_table_ = NULL;
}

// See class comment for arguments.
void SampleIterator::Init(const IndexMapBiDi* charset_map,
                          const ShapeTable* shape_table,
                          bool randomize,
                          TrainingSampleSet* sample_set) {
  Clear();
  charset_map_ = charset_map;
  shape_table_ = shape_table;
  sample_set_ = sample_set;
  randomize_ = randomize;
  if (shape_table_ == NULL && charset_map_ != NULL) {
    // The caller wishes to iterate by class. The easiest way to do this
    // is to create a dummy shape_table_ that we will own.
    int num_fonts = sample_set_->NumFonts();
    owned_shape_table_ = new ShapeTable(sample_set_->unicharset());
    int charsetsize = sample_set_->unicharset().size();
    for (int c = 0; c < charsetsize; ++c) {
      // We always add a shape for each character to keep the index in sync
      // with the unichar_id.
      int shape_id = owned_shape_table_->AddShape(c, 0);
      for (int f = 1; f < num_fonts; ++f) {
        if (sample_set_->NumClassSamples(f, c, true) > 0) {
          owned_shape_table_->AddToShape(shape_id, c, f);
        }
      }
    }
    shape_table_ = owned_shape_table_;
  }
  if (shape_table_ != NULL) {
    num_shapes_ = shape_table_->NumShapes();
  } else {
    num_shapes_ = randomize ? sample_set_->num_samples()
                            : sample_set_->num_raw_samples();
  }
  Begin();
}

// Iterator functions designed for use with a simple for loop:
// for (it.Begin(); !it.AtEnd(); it.Next()) {
//   const TrainingSample& sample = it.GetSample();
// }
void SampleIterator::Begin() {
  shape_index_ = -1;
  shape_char_index_ = 0;
  num_shape_chars_ = 0;
  shape_font_index_ = 0;
  num_shape_fonts_ = 0;
  sample_index_ = 0;
  num_samples_ = 0;
  // Find the first indexable sample.
  Next();
}

bool SampleIterator::AtEnd() const {
  return shape_index_ >= num_shapes_;
}

const TrainingSample& SampleIterator::GetSample() const {
  if (shape_table_ != NULL) {
    const UnicharAndFonts* shape_entry = GetShapeEntry();
    int char_id = shape_entry->unichar_id;
    int font_id = shape_entry->font_ids[shape_font_index_];
    return *sample_set_->GetSample(font_id, char_id, sample_index_);
  } else {
    return *sample_set_->GetSample(shape_index_);
  }
}

TrainingSample* SampleIterator::MutableSample() const {
  if (shape_table_ != NULL) {
    const UnicharAndFonts* shape_entry = GetShapeEntry();
    int char_id = shape_entry->unichar_id;
    int font_id = shape_entry->font_ids[shape_font_index_];
    return sample_set_->MutableSample(font_id, char_id, sample_index_);
  } else {
    return sample_set_->mutable_sample(shape_index_);
  }
}

// Returns the total index (from the original set of samples) of the current
// sample.
int SampleIterator::GlobalSampleIndex() const {
  if (shape_table_ != NULL) {
    const UnicharAndFonts* shape_entry = GetShapeEntry();
    int char_id = shape_entry->unichar_id;
    int font_id = shape_entry->font_ids[shape_font_index_];
    return sample_set_->GlobalSampleIndex(font_id, char_id, sample_index_);
  } else {
    return shape_index_;
  }
}

// Returns the index of the current sample in compact charset space, so
// in a 2-class problem between x and y, the returned indices will all be
// 0 or 1, and have nothing to do with the unichar_ids.
// If the charset_map_ is NULL, then this is equal to GetSparseClassID().
int SampleIterator::GetCompactClassID() const {
  return charset_map_ != NULL ? charset_map_->SparseToCompact(shape_index_)
                              : GetSparseClassID();
}
// Returns the index of the current sample in sparse charset space, so
// in a 2-class problem between x and y, the returned indices will all be
// x or y, where x and y may be unichar_ids (no shape_table_) or shape_ids
// with a shape_table_.
int SampleIterator::GetSparseClassID() const {
  return shape_table_ != NULL ? shape_index_ : GetSample().class_id();
}

// Moves on to the next indexable sample. If the end is reached, leaves
// the state such that AtEnd() is true.
void SampleIterator::Next() {
  if (shape_table_ != NULL) {
    // Next sample in this class/font combination.
    ++sample_index_;
    if (sample_index_ < num_samples_)
      return;
    // Next font in this class in this shape.
    sample_index_ = 0;
    do {
      ++shape_font_index_;
      if (shape_font_index_ >= num_shape_fonts_) {
        // Next unichar in this shape.
        shape_font_index_ = 0;
        ++shape_char_index_;
        if (shape_char_index_ >= num_shape_chars_) {
          // Find the next shape that is mapped in the charset_map_.
          shape_char_index_ = 0;
          do {
            ++shape_index_;
          } while (shape_index_ < num_shapes_ &&
                   charset_map_ != NULL &&
                   charset_map_->SparseToCompact(shape_index_) < 0);
          if (shape_index_ >= num_shapes_)
            return;  // The end.
          num_shape_chars_ = shape_table_->GetShape(shape_index_).size();
        }
      }
      const UnicharAndFonts* shape_entry = GetShapeEntry();
      num_shape_fonts_ = shape_entry->font_ids.size();
      int char_id = shape_entry->unichar_id;
      int font_id = shape_entry->font_ids[shape_font_index_];
      num_samples_ = sample_set_->NumClassSamples(font_id, char_id, randomize_);
    } while (num_samples_ == 0);
  } else {
    // We are just iterating over the samples.
    ++shape_index_;
  }
}

// Returns the size of the compact charset space.
int SampleIterator::CompactCharsetSize() const {
  return charset_map_ != NULL ? charset_map_->CompactSize()
                              : SparseCharsetSize();
}

// Returns the size of the sparse charset space.
int SampleIterator::SparseCharsetSize() const {
  return charset_map_ != NULL
      ? charset_map_->SparseSize()
      : (shape_table_ != NULL ? shape_table_->NumShapes()
                              : sample_set_->charsetsize());
}

// Apply the supplied feature_space/feature_map transform to all samples
// accessed by this iterator.
void SampleIterator::MapSampleFeatures(const IntFeatureMap& feature_map) {
  for (Begin(); !AtEnd(); Next()) {
    TrainingSample* sample = MutableSample();
    sample->MapFeatures(feature_map);
  }
}

// Adjust the weights of all the samples to be uniform in the given charset.
// Returns the number of samples in the iterator.
int SampleIterator::UniformSamples() {
  int num_good_samples = 0;
  for (Begin(); !AtEnd(); Next()) {
    TrainingSample* sample = MutableSample();
    sample->set_weight(1.0);
    ++num_good_samples;
  }
  NormalizeSamples();
  return num_good_samples;
}

// Normalize the weights of all the samples in the charset_map so they sum
// to 1. Returns the minimum assigned sample weight.
double SampleIterator::NormalizeSamples() {
  double total_weight = 0.0;
  int sample_count = 0;
  for (Begin(); !AtEnd(); Next()) {
    const TrainingSample& sample = GetSample();
    total_weight += sample.weight();
    ++sample_count;
  }
  // Normalize samples.
  double min_assigned_sample_weight = 1.0;
  if (total_weight > 0.0) {
    for (Begin(); !AtEnd(); Next()) {
      TrainingSample* sample = MutableSample();
      double weight = sample->weight() / total_weight;
      if (weight < min_assigned_sample_weight)
        min_assigned_sample_weight = weight;
      sample->set_weight(weight);
    }
  }
  return min_assigned_sample_weight;
}

// Helper returns the current UnicharAndFont shape_entry.
const UnicharAndFonts* SampleIterator::GetShapeEntry() const {
  const Shape& shape = shape_table_->GetShape(shape_index_);
  return &shape[shape_char_index_];
}

}  // namespace tesseract.

