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


#ifndef TESSERACT_CLASSIFY_SAMPLEITERATOR_H_
#define TESSERACT_CLASSIFY_SAMPLEITERATOR_H_

namespace tesseract {

class IndexMapBiDi;
class IntFeatureMap;
class ShapeTable;
class TrainingSample;
class TrainingSampleSet;
struct UnicharAndFonts;

// Iterator class to encapsulate the complex iteration involved in getting
// all samples of all shapes needed for a classification problem.
//
// =====INPUTS TO Init FUNCTION=====
// The charset_map defines a subset of the sample_set classes (with a nullptr
// shape_table, or the shape_table classes if not nullptr.)
//
// The shape_table (if not nullptr) defines the mapping from shapes to
// font_id/class_id pairs. Each shape is a list of unichar_id and font lists.
//
// The sample_set holds the samples and provides indexed access to samples
// of font_id/class_id pairs.
//
// If randomize is true, the samples are perturbed slightly, but the
// perturbation is guaranteed to be the same for multiple identical
// iterations.
//
// =====DIFFERENT COMBINATIONS OF INPUTS=====
// nullptr shape_table:
// Without a shape_table, everything works in UNICHAR_IDs.
//
// nullptr shape_table, nullptr charset_map:
// Iterations simply run over the samples in the order the samples occur in the
// input files.
// GetCompactClassID and GetSparseClassID both return the sample UNICHAR_ID.
//
// nullptr shape_table, non-nullptr charset_map:
// When shape_table is nullptr, the charset_map indexes unichar_ids directly,
// and an iteration returns all samples of all chars in the charset_map, which
// is a subset of the full unicharset.
// The iteration will be in groups of the same unichar_id, in the order
// defined by the charset_map.
// GetCompactClassID returns the charset_map index of a sample, and
// GetSparseClassID returns the sample UNICHAR_ID.
//
// Non-nullptr shape_table:
// With a shape_table, samples are grouped according to the shape_table, so
// multiple UNICHAR_IDs and fonts may be grouped together, and everything
// works in shape_ids.
//
// Non-nullptr shape_table, nullptr charset_map.
// Iterations simply run over the samples in the order of shape_id.
// GetCompactClassID and GetSparseClassID both return the shape_id.
// (If you want the unichar_id or font_id, the sample still has them.)
//
// Non-nullptr shape_table, non-nullptr charset_map.
// When shape_table is not nullptr, the charset_map indexes and subsets shapes in
// the shape_table, and iterations will be in shape_table order, not
// charset_map order.
// GetCompactClassID returns the charset_map index of a shape, and
// GetSparseClassID returns the shape_id.
//
// =====What is SampleIterator good for?=====
// Inside a classifier training module, the SampleIterator has abstracted away
// all the different modes above.
// Use the following iteration to train your classifier:
// for (it.Begin(); !it.AtEnd(); it.Next()) {
//   const TrainingSample& sample = it.GetSample();
//   int class_id = it.GetCompactClassID();
// Your classifier may or may not be dealing with a shape_table, and may be
// dealing with some subset of the character/shape set. It doesn't need to
// know and shouldn't care. It is just learning shapes with compact class ids
// in the range [0, it.CompactCharsetSize()).
class SampleIterator {
 public:
  SampleIterator();
  ~SampleIterator();

  void Clear();

  // See class comment for arguments.
  void Init(const IndexMapBiDi* charset_map,
            const ShapeTable* shape_table,
            bool randomize,
            TrainingSampleSet* sample_set);

  // Iterator functions designed for use with a simple for loop:
  // for (it.Begin(); !it.AtEnd(); it.Next()) {
  //   const TrainingSample& sample = it.GetSample();
  //   int class_id = it.GetCompactClassID();
  //   ...
  // }
  void Begin();
  bool AtEnd() const;
  const TrainingSample& GetSample() const;
  TrainingSample* MutableSample() const;
  // Returns the total index (from the original set of samples) of the current
  // sample.
  int GlobalSampleIndex() const;
  // Returns the index of the current sample in compact charset space, so
  // in a 2-class problem between x and y, the returned indices will all be
  // 0 or 1, and have nothing to do with the unichar_ids.
  // If the charset_map_ is nullptr, then this is equal to GetSparseClassID().
  int GetCompactClassID() const;
  // Returns the index of the current sample in sparse charset space, so
  // in a 2-class problem between x and y, the returned indices will all be
  // x or y, where x and y may be unichar_ids (no shape_table_) or shape_ids
  // with a shape_table_.
  int GetSparseClassID() const;
  // Moves on to the next indexable sample. If the end is reached, leaves
  // the state such that AtEnd() is true.
  void Next();

  // Returns the size of the compact charset space.
  int CompactCharsetSize() const;
  // Returns the size of the sparse charset space.
  int SparseCharsetSize() const;

  const IndexMapBiDi& charset_map() const {
    return *charset_map_;
  }
  const ShapeTable* shape_table() const {
    return shape_table_;
  }
  // Sample set operations.
  const TrainingSampleSet* sample_set() const {
    return sample_set_;
  }

  // A set of functions that do something to all the samples accessed by the
  // iterator, as it is currently setup.

  // Apply the supplied feature_space/feature_map transform to all samples
  // accessed by this iterator.
  void MapSampleFeatures(const IntFeatureMap& feature_map);

  // Adjust the weights of all the samples to be uniform in the given charset.
  // Returns the number of samples in the iterator.
  int UniformSamples();

  // Normalize the weights of all the samples defined by the iterator so they
  // sum to 1. Returns the minimum assigned sample weight.
  double NormalizeSamples();

 private:
  // Helper returns the current UnicharAndFont shape_entry.
  const UnicharAndFonts* GetShapeEntry() const;

  // Map to subset the actual charset space.
  const IndexMapBiDi* charset_map_;
  // Shape table to recombine character classes into shapes
  const ShapeTable* shape_table_;
  // The samples to iterate over.
  TrainingSampleSet* sample_set_;
  // Flag to control randomizing the sample features.
  bool randomize_;
  // Shape table owned by this used to iterate character classes.
  ShapeTable* owned_shape_table_;

  // Top-level iteration. Shape index in sparse charset_map space.
  int shape_index_;
  int num_shapes_;
  // Index to the character class within a shape.
  int shape_char_index_;
  int num_shape_chars_;
  // Index to the font within a shape/class pair.
  int shape_font_index_;
  int num_shape_fonts_;
  // The lowest level iteration. sample_index_/num_samples_ counts samples
  // in the current shape/class/font combination.
  int sample_index_;
  int num_samples_;
};

}  // namespace tesseract.

#endif  // TESSERACT_CLASSIFY_SAMPLEITERATOR_H_
