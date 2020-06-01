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

#ifndef TESSERACT_TRAINING_TRAININGSAMPLESET_H_
#define TESSERACT_TRAINING_TRAININGSAMPLESET_H_

#include "bitvector.h"
#include <tesseract/genericvector.h>
#include "indexmapbidi.h"
#include "matrix.h"
#include "shapetable.h"
#include "trainingsample.h"

class UNICHARSET;

namespace tesseract {

struct FontInfo;
class FontInfoTable;
class IntFeatureMap;
class IntFeatureSpace;
class TrainingSample;
struct UnicharAndFonts;

// Collection of TrainingSample used for training or testing a classifier.
// Provides several useful methods to operate on the collection as a whole,
// including outlier detection and deletion, providing access by font and
// class, finding the canonical sample, finding the "cloud" features (OR of
// all features in all samples), replication of samples, caching of distance
// metrics.
class TrainingSampleSet {
 public:
  explicit TrainingSampleSet(const FontInfoTable& fontinfo_table);
  ~TrainingSampleSet();

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp);

  // Accessors
  int num_samples() const {
    return samples_.size();
  }
  int num_raw_samples() const {
    return num_raw_samples_;
  }
  int NumFonts() const {
    return font_id_map_.SparseSize();
  }
  const UNICHARSET& unicharset() const {
    return unicharset_;
  }
  int charsetsize() const {
    return unicharset_size_;
  }
  const FontInfoTable& fontinfo_table() const {
    return fontinfo_table_;
  }

  // Loads an initial unicharset, or sets one up if the file cannot be read.
  void LoadUnicharset(const char* filename);

  // Adds a character sample to this sample set.
  // If the unichar is not already in the local unicharset, it is added.
  // Returns the unichar_id of the added sample, from the local unicharset.
  int AddSample(const char* unichar, TrainingSample* sample);
  // Adds a character sample to this sample set with the given unichar_id,
  // which must correspond to the local unicharset (in this).
  void AddSample(int unichar_id, TrainingSample* sample);

  // Returns the number of samples for the given font,class pair.
  // If randomize is true, returns the number of samples accessible
  // with randomizing on. (Increases the number of samples if small.)
  // OrganizeByFontAndClass must have been already called.
  int NumClassSamples(int font_id, int class_id, bool randomize) const;

  // Gets a sample by its index.
  const TrainingSample* GetSample(int index) const;

  // Gets a sample by its font, class, index.
  // OrganizeByFontAndClass must have been already called.
  const TrainingSample* GetSample(int font_id, int class_id, int index) const;

  // Get a sample by its font, class, index. Does not randomize.
  // OrganizeByFontAndClass must have been already called.
  TrainingSample* MutableSample(int font_id, int class_id, int index);

  // Returns a string debug representation of the given sample:
  // font, unichar_str, bounding box, page.
  STRING SampleToString(const TrainingSample& sample) const;

  // Gets the combined set of features used by all the samples of the given
  // font/class combination.
  const BitVector& GetCloudFeatures(int font_id, int class_id) const;
  // Gets the indexed features of the canonical sample of the given
  // font/class combination.
  const GenericVector<int>& GetCanonicalFeatures(int font_id,
                                                 int class_id) const;

  // Returns the distance between the given UniCharAndFonts pair.
  // If matched_fonts, only matching fonts, are considered, unless that yields
  // the empty set.
  // OrganizeByFontAndClass must have been already called.
  float UnicharDistance(const UnicharAndFonts& uf1, const UnicharAndFonts& uf2,
                        bool matched_fonts, const IntFeatureMap& feature_map);

  // Returns the distance between the given pair of font/class pairs.
  // Finds in cache or computes and caches.
  // OrganizeByFontAndClass must have been already called.
  float ClusterDistance(int font_id1, int class_id1,
                        int font_id2, int class_id2,
                        const IntFeatureMap& feature_map);

  // Computes the distance between the given pair of font/class pairs.
  float ComputeClusterDistance(int font_id1, int class_id1,
                               int font_id2, int class_id2,
                               const IntFeatureMap& feature_map) const;

  // Returns the number of canonical features of font/class 2 for which
  // neither the feature nor any of its near neighbors occurs in the cloud
  // of font/class 1. Each such feature is a reliable separation between
  // the classes, ASSUMING that the canonical sample is sufficiently
  // representative that every sample has a feature near that particular
  // feature. To check that this is so on the fly would be prohibitively
  // expensive, but it might be possible to pre-qualify the canonical features
  // to include only those for which this assumption is true.
  // ComputeCanonicalFeatures and ComputeCloudFeatures must have been called
  // first, or the results will be nonsense.
  int ReliablySeparable(int font_id1, int class_id1,
                        int font_id2, int class_id2,
                        const IntFeatureMap& feature_map,
                        bool thorough) const;


  // Returns the total index of the requested sample.
  // OrganizeByFontAndClass must have been already called.
  int GlobalSampleIndex(int font_id, int class_id, int index) const;

  // Gets the canonical sample for the given font, class pair.
  // ComputeCanonicalSamples must have been called first.
  const TrainingSample* GetCanonicalSample(int font_id, int class_id) const;
  // Gets the max distance for the given canonical sample.
  // ComputeCanonicalSamples must have been called first.
  float GetCanonicalDist(int font_id, int class_id) const;

  // Returns a mutable pointer to the sample with the given index.
  TrainingSample* mutable_sample(int index) {
    return samples_[index];
  }
  // Gets ownership of the sample with the given index, removing it from this.
  TrainingSample* extract_sample(int index) {
    TrainingSample* sample = samples_[index];
    samples_[index] = nullptr;
    return sample;
  }

  // Generates indexed features for all samples with the supplied feature_space.
  void IndexFeatures(const IntFeatureSpace& feature_space);

  // Marks the given sample for deletion.
  // Deletion is actually completed by DeleteDeadSamples.
  void KillSample(TrainingSample* sample);

  // Deletes all samples with a negative sample index marked by KillSample.
  // Must be called before OrganizeByFontAndClass, and OrganizeByFontAndClass
  // must be called after as the samples have been renumbered.
  void DeleteDeadSamples();

  // Callback function returns true if the given sample is to be deleted, due
  // to having a negative classid.
  bool DeleteableSample(const TrainingSample* sample);

  // Construct an array to access the samples by font,class pair.
  void OrganizeByFontAndClass();

  // Constructs the font_id_map_ which maps real font_ids (sparse) to a compact
  // index for the font_class_array_.
  void SetupFontIdMap();

  // Finds the sample for each font, class pair that has least maximum
  // distance to all the other samples of the same font, class.
  // OrganizeByFontAndClass must have been already called.
  void ComputeCanonicalSamples(const IntFeatureMap& map, bool debug);

  // Replicates the samples to a minimum frequency defined by
  // 2 * kSampleRandomSize, or for larger counts duplicates all samples.
  // After replication, the replicated samples are perturbed slightly, but
  // in a predictable and repeatable way.
  // Use after OrganizeByFontAndClass().
  void ReplicateAndRandomizeSamples();

  // Caches the indexed features of the canonical samples.
  // ComputeCanonicalSamples must have been already called.
  void ComputeCanonicalFeatures();
  // Computes the combined set of features used by all the samples of each
  // font/class combination. Use after ReplicateAndRandomizeSamples.
  void ComputeCloudFeatures(int feature_space_size);

  // Adds all fonts of the given class to the shape.
  void AddAllFontsForClass(int class_id, Shape* shape) const;

  // Display the samples with the given indexed feature that also match
  // the given shape.
  void DisplaySamplesWithFeature(int f_index, const Shape& shape,
                                 const IntFeatureSpace& feature_space,
                                 ScrollView::Color color,
                                 ScrollView* window) const;

 private:
  // Struct to store a triplet of unichar, font, distance in the distance cache.
  struct FontClassDistance {
    int unichar_id;
    int font_id;  // Real font id.
    float distance;
  };
  // Simple struct to store information related to each font/class combination.
  struct FontClassInfo {
    FontClassInfo();

    // Writes to the given file. Returns false in case of error.
    bool Serialize(FILE* fp) const;
    // Reads from the given file. Returns false in case of error.
    // If swap is true, assumes a big/little-endian swap is needed.
    bool DeSerialize(bool swap, FILE* fp);

    // Number of raw samples.
    int32_t num_raw_samples;
    // Index of the canonical sample.
    int32_t canonical_sample;
    // Max distance of the canonical sample from any other.
    float canonical_dist;
    // Sample indices for the samples, including replicated.
    GenericVector<int32_t> samples;

    // Non-serialized cache data.
    // Indexed features of the canonical sample.
    GenericVector<int> canonical_features;
    // The mapped features of all the samples.
    BitVector cloud_features;

    // Caches for ClusterDistance.
    // Caches for other fonts but matching this unichar. -1 indicates not set.
    // Indexed by compact font index from font_id_map_.
    GenericVector<float> font_distance_cache;
    // Caches for other unichars but matching this font. -1 indicates not set.
    GenericVector<float> unichar_distance_cache;
    // Cache for the rest (non matching font and unichar.)
    // A cache of distances computed by ReliablySeparable.
    GenericVector<FontClassDistance> distance_cache;
  };

  PointerVector<TrainingSample> samples_;
  // Number of samples before replication/randomization.
  int num_raw_samples_;
  // Character set we are training for.
  UNICHARSET unicharset_;
  // Character set size to which the 2-d arrays below refer.
  int unicharset_size_;
  // Map to allow the font_class_array_ below to be compact.
  // The sparse space is the real font_id, used in samples_ .
  // The compact space is an index to font_class_array_
  IndexMapBiDi font_id_map_;
  // A 2-d array of FontClassInfo holding information related to each
  // (font_id, class_id) pair.
  GENERIC_2D_ARRAY<FontClassInfo>* font_class_array_;

  // Reference to the fontinfo_table_ in MasterTrainer. Provides names
  // for font_ids in the samples. Not serialized!
  const FontInfoTable& fontinfo_table_;
};

}  // namespace tesseract.


#endif  // TRAININGSAMPLESETSET_H_
