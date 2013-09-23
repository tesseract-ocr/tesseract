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

#include "trainingsampleset.h"
#include "allheaders.h"
#include "boxread.h"
#include "fontinfo.h"
#include "indexmapbidi.h"
#include "intfeaturedist.h"
#include "intfeaturemap.h"
#include "intfeaturespace.h"
#include "shapetable.h"
#include "trainingsample.h"
#include "unicity_table.h"

namespace tesseract {

const int kTestChar = -1;  // 37;
// Max number of distances to compute the squared way
const int kSquareLimit = 25;
// Prime numbers for subsampling distances.
const int kPrime1 = 17;
const int kPrime2 = 13;
// Min samples from which to start discarding outliers.
const int kMinOutlierSamples = 5;

TrainingSampleSet::FontClassInfo::FontClassInfo()
  : num_raw_samples(0), canonical_sample(-1), canonical_dist(0.0f) {
}

// Writes to the given file. Returns false in case of error.
bool TrainingSampleSet::FontClassInfo::Serialize(FILE* fp) const {
  if (fwrite(&num_raw_samples, sizeof(num_raw_samples), 1, fp) != 1)
    return false;
  if (fwrite(&canonical_sample, sizeof(canonical_sample), 1, fp) != 1)
    return false;
  if (fwrite(&canonical_dist, sizeof(canonical_dist), 1, fp) != 1) return false;
  if (!samples.Serialize(fp)) return false;
  return true;
}
// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool TrainingSampleSet::FontClassInfo::DeSerialize(bool swap, FILE* fp) {
  if (fread(&num_raw_samples, sizeof(num_raw_samples), 1, fp) != 1)
    return false;
  if (fread(&canonical_sample, sizeof(canonical_sample), 1, fp) != 1)
    return false;
  if (fread(&canonical_dist, sizeof(canonical_dist), 1, fp) != 1) return false;
  if (!samples.DeSerialize(swap, fp)) return false;
  if (swap) {
    ReverseN(&num_raw_samples, sizeof(num_raw_samples));
    ReverseN(&canonical_sample, sizeof(canonical_sample));
    ReverseN(&canonical_dist, sizeof(canonical_dist));
  }
  return true;
}

TrainingSampleSet::TrainingSampleSet(const FontInfoTable& font_table)
  : num_raw_samples_(0), unicharset_size_(0),
    font_class_array_(NULL), fontinfo_table_(font_table) {
}

TrainingSampleSet::~TrainingSampleSet() {
  delete font_class_array_;
}

// Writes to the given file. Returns false in case of error.
bool TrainingSampleSet::Serialize(FILE* fp) const {
  if (!samples_.Serialize(fp)) return false;
  if (!unicharset_.save_to_file(fp)) return false;
  if (!font_id_map_.Serialize(fp)) return false;
  inT8 not_null = font_class_array_ != NULL;
  if (fwrite(&not_null, sizeof(not_null), 1, fp) != 1) return false;
  if (not_null) {
    if (!font_class_array_->SerializeClasses(fp)) return false;
  }
  return true;
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool TrainingSampleSet::DeSerialize(bool swap, FILE* fp) {
  if (!samples_.DeSerialize(swap, fp)) return false;
  num_raw_samples_ = samples_.size();
  if (!unicharset_.load_from_file(fp)) return false;
  if (!font_id_map_.DeSerialize(swap, fp)) return false;
  if (font_class_array_ != NULL) {
    delete font_class_array_;
    font_class_array_ = NULL;
  }
  inT8 not_null;
  if (fread(&not_null, sizeof(not_null), 1, fp) != 1) return false;
  if (not_null) {
    FontClassInfo empty;
    font_class_array_ = new GENERIC_2D_ARRAY<FontClassInfo >(1, 1 , empty);
    if (!font_class_array_->DeSerializeClasses(swap, fp)) return false;
  }
  unicharset_size_ = unicharset_.size();
  return true;
}

// Load an initial unicharset, or set one up if the file cannot be read.
void TrainingSampleSet::LoadUnicharset(const char* filename) {
  if (!unicharset_.load_from_file(filename)) {
    tprintf("Failed to load unicharset from file %s\n"
            "Building unicharset from scratch...\n",
            filename);
    unicharset_.clear();
    // Add special characters as they were removed by the clear.
    UNICHARSET empty;
    unicharset_.AppendOtherUnicharset(empty);
  }
  unicharset_size_ = unicharset_.size();
}

// Adds a character sample to this sample set.
// If the unichar is not already in the local unicharset, it is added.
// Returns the unichar_id of the added sample, from the local unicharset.
int TrainingSampleSet::AddSample(const char* unichar, TrainingSample* sample) {
  if (!unicharset_.contains_unichar(unichar)) {
    unicharset_.unichar_insert(unichar);
    if (unicharset_.size() > MAX_NUM_CLASSES) {
      tprintf("Error: Size of unicharset in TrainingSampleSet::AddSample is "
              "greater than MAX_NUM_CLASSES\n");
      return -1;
    }
  }
  UNICHAR_ID char_id = unicharset_.unichar_to_id(unichar);
  AddSample(char_id, sample);
  return char_id;
}

// Adds a character sample to this sample set with the given unichar_id,
// which must correspond to the local unicharset (in this).
void TrainingSampleSet::AddSample(int unichar_id, TrainingSample* sample) {
  sample->set_class_id(unichar_id);
  samples_.push_back(sample);
  num_raw_samples_ = samples_.size();
  unicharset_size_ = unicharset_.size();
}

// Returns the number of samples for the given font,class pair.
// If randomize is true, returns the number of samples accessible
// with randomizing on. (Increases the number of samples if small.)
// OrganizeByFontAndClass must have been already called.
int TrainingSampleSet::NumClassSamples(int font_id, int class_id,
                                       bool randomize) const {
  ASSERT_HOST(font_class_array_ != NULL);
  if (font_id < 0 || class_id < 0 ||
      font_id >= font_id_map_.SparseSize() || class_id >= unicharset_size_) {
    // There are no samples because the font or class doesn't exist.
    return 0;
  }
  int font_index = font_id_map_.SparseToCompact(font_id);
  if (font_index < 0)
    return 0;  // The font has no samples.
  if (randomize)
    return (*font_class_array_)(font_index, class_id).samples.size();
  else
    return (*font_class_array_)(font_index, class_id).num_raw_samples;
}

// Gets a sample by its index.
const TrainingSample* TrainingSampleSet::GetSample(int index) const {
  return samples_[index];
}

// Gets a sample by its font, class, index.
// OrganizeByFontAndClass must have been already called.
const TrainingSample* TrainingSampleSet::GetSample(int font_id, int class_id,
                                                   int index) const {
  ASSERT_HOST(font_class_array_ != NULL);
  int font_index = font_id_map_.SparseToCompact(font_id);
  if (font_index < 0) return NULL;
  int sample_index = (*font_class_array_)(font_index, class_id).samples[index];
  return samples_[sample_index];
}

// Get a sample by its font, class, index. Does not randomize.
// OrganizeByFontAndClass must have been already called.
TrainingSample* TrainingSampleSet::MutableSample(int font_id, int class_id,
                                                 int index) {
  ASSERT_HOST(font_class_array_ != NULL);
  int font_index = font_id_map_.SparseToCompact(font_id);
  if (font_index < 0) return NULL;
  int sample_index = (*font_class_array_)(font_index, class_id).samples[index];
  return samples_[sample_index];
}

// Returns a string debug representation of the given sample:
// font, unichar_str, bounding box, page.
STRING TrainingSampleSet::SampleToString(const TrainingSample& sample) const {
  STRING boxfile_str;
  MakeBoxFileStr(unicharset_.id_to_unichar(sample.class_id()),
                 sample.bounding_box(), sample.page_num(), &boxfile_str);
  return STRING(fontinfo_table_.get(sample.font_id()).name) + " " + boxfile_str;
}

// Gets the combined set of features used by all the samples of the given
// font/class combination.
const BitVector& TrainingSampleSet::GetCloudFeatures(
    int font_id, int class_id) const {
  int font_index = font_id_map_.SparseToCompact(font_id);
  ASSERT_HOST(font_index >= 0);
  return (*font_class_array_)(font_index, class_id).cloud_features;
}
// Gets the indexed features of the canonical sample of the given
// font/class combination.
const GenericVector<int>& TrainingSampleSet::GetCanonicalFeatures(
    int font_id, int class_id) const {
  int font_index = font_id_map_.SparseToCompact(font_id);
  ASSERT_HOST(font_index >= 0);
  return (*font_class_array_)(font_index, class_id).canonical_features;
}

// Returns the distance between the given UniCharAndFonts pair.
// If matched_fonts, only matching fonts, are considered, unless that yields
// the empty set.
// OrganizeByFontAndClass must have been already called.
float TrainingSampleSet::UnicharDistance(const UnicharAndFonts& uf1,
                                         const UnicharAndFonts& uf2,
                                         bool matched_fonts,
                                         const IntFeatureMap& feature_map) {
  int num_fonts1 = uf1.font_ids.size();
  int c1 = uf1.unichar_id;
  int num_fonts2 = uf2.font_ids.size();
  int c2 = uf2.unichar_id;
  double dist_sum = 0.0;
  int dist_count = 0;
  bool debug = false;
  if (matched_fonts) {
    // Compute distances only where fonts match.
    for (int i = 0; i < num_fonts1; ++i) {
      int f1 = uf1.font_ids[i];
      for (int j = 0; j < num_fonts2; ++j) {
        int f2 = uf2.font_ids[j];
        if (f1 == f2) {
          dist_sum += ClusterDistance(f1, c1, f2, c2, feature_map);
          ++dist_count;
        }
      }
    }
  } else if (num_fonts1 * num_fonts2 <= kSquareLimit) {
    // Small enough sets to compute all the distances.
    for (int i = 0; i < num_fonts1; ++i) {
      int f1 = uf1.font_ids[i];
      for (int j = 0; j < num_fonts2; ++j) {
        int f2 = uf2.font_ids[j];
        dist_sum += ClusterDistance(f1, c1, f2, c2, feature_map);
        if (debug) {
            tprintf("Cluster dist %d %d %d %d = %g\n",
                    f1, c1, f2, c2,
                    ClusterDistance(f1, c1, f2, c2, feature_map));
        }
        ++dist_count;
      }
    }
  } else {
    // Subsample distances, using the largest set once, and stepping through
    // the smaller set so as to ensure that all the pairs are different.
    int increment = kPrime1 != num_fonts2 ? kPrime1 : kPrime2;
    int index = 0;
    int num_samples = MAX(num_fonts1, num_fonts2);
    for (int i = 0; i < num_samples; ++i, index += increment) {
      int f1 = uf1.font_ids[i % num_fonts1];
      int f2 = uf2.font_ids[index % num_fonts2];
      if (debug) {
          tprintf("Cluster dist %d %d %d %d = %g\n",
                  f1, c1, f2, c2, ClusterDistance(f1, c1, f2, c2, feature_map));
      }
      dist_sum += ClusterDistance(f1, c1, f2, c2, feature_map);
      ++dist_count;
    }
  }
  if (dist_count == 0) {
    if (matched_fonts)
      return UnicharDistance(uf1, uf2, false, feature_map);
    return 0.0f;
  }
  return dist_sum / dist_count;
}

// Returns the distance between the given pair of font/class pairs.
// Finds in cache or computes and caches.
// OrganizeByFontAndClass must have been already called.
float TrainingSampleSet::ClusterDistance(int font_id1, int class_id1,
                                         int font_id2, int class_id2,
                                         const IntFeatureMap& feature_map) {
  ASSERT_HOST(font_class_array_ != NULL);
  int font_index1 = font_id_map_.SparseToCompact(font_id1);
  int font_index2 = font_id_map_.SparseToCompact(font_id2);
  if (font_index1 < 0 || font_index2 < 0)
    return 0.0f;
  FontClassInfo& fc_info = (*font_class_array_)(font_index1, class_id1);
  if (font_id1 == font_id2) {
    // Special case cache for speed.
    if (fc_info.unichar_distance_cache.size() == 0)
      fc_info.unichar_distance_cache.init_to_size(unicharset_size_, -1.0f);
    if (fc_info.unichar_distance_cache[class_id2] < 0) {
      // Distance has to be calculated.
      float result = ComputeClusterDistance(font_id1, class_id1,
                                            font_id2, class_id2,
                                            feature_map);
      fc_info.unichar_distance_cache[class_id2] = result;
      // Copy to the symmetric cache entry.
      FontClassInfo& fc_info2 = (*font_class_array_)(font_index2, class_id2);
      if (fc_info2.unichar_distance_cache.size() == 0)
        fc_info2.unichar_distance_cache.init_to_size(unicharset_size_, -1.0f);
      fc_info2.unichar_distance_cache[class_id1] = result;
    }
    return fc_info.unichar_distance_cache[class_id2];
  } else if (class_id1 == class_id2) {
    // Another special-case cache for equal class-id.
    if (fc_info.font_distance_cache.size() == 0)
      fc_info.font_distance_cache.init_to_size(font_id_map_.CompactSize(),
                                               -1.0f);
    if (fc_info.font_distance_cache[font_index2] < 0) {
      // Distance has to be calculated.
      float result = ComputeClusterDistance(font_id1, class_id1,
                                            font_id2, class_id2,
                                            feature_map);
      fc_info.font_distance_cache[font_index2] = result;
      // Copy to the symmetric cache entry.
      FontClassInfo& fc_info2 = (*font_class_array_)(font_index2, class_id2);
      if (fc_info2.font_distance_cache.size() == 0)
        fc_info2.font_distance_cache.init_to_size(font_id_map_.CompactSize(),
                                                  -1.0f);
      fc_info2.font_distance_cache[font_index1] = result;
    }
    return fc_info.font_distance_cache[font_index2];
  }
  // Both font and class are different. Linear search for class_id2/font_id2
  // in what is a hopefully short list of distances.
  int cache_index = 0;
  while (cache_index < fc_info.distance_cache.size() &&
         (fc_info.distance_cache[cache_index].unichar_id != class_id2 ||
          fc_info.distance_cache[cache_index].font_id != font_id2))
    ++cache_index;
  if (cache_index == fc_info.distance_cache.size()) {
    // Distance has to be calculated.
    float result = ComputeClusterDistance(font_id1, class_id1,
                                          font_id2, class_id2,
                                          feature_map);
    FontClassDistance fc_dist = { class_id2, font_id2, result };
    fc_info.distance_cache.push_back(fc_dist);
    // Copy to the symmetric cache entry. We know it isn't there already, as
    // we always copy to the symmetric entry.
    FontClassInfo& fc_info2 = (*font_class_array_)(font_index2, class_id2);
    fc_dist.unichar_id = class_id1;
    fc_dist.font_id = font_id1;
    fc_info2.distance_cache.push_back(fc_dist);
  }
  return fc_info.distance_cache[cache_index].distance;
}

// Computes the distance between the given pair of font/class pairs.
float TrainingSampleSet::ComputeClusterDistance(
    int font_id1, int class_id1, int font_id2, int class_id2,
    const IntFeatureMap& feature_map) const {
  int dist = ReliablySeparable(font_id1, class_id1, font_id2, class_id2,
                               feature_map, false);
  dist += ReliablySeparable(font_id2, class_id2, font_id1, class_id1,
                            feature_map, false);
  int denominator = GetCanonicalFeatures(font_id1, class_id1).size();
  denominator += GetCanonicalFeatures(font_id2, class_id2).size();
  return static_cast<float>(dist) / denominator;
}

// Helper to add a feature and its near neighbors to the good_features.
// levels indicates how many times to compute the offset features of what is
// already there. This is done by iteration rather than recursion.
static void AddNearFeatures(const IntFeatureMap& feature_map, int f, int levels,
                            GenericVector<int>* good_features) {
  int prev_num_features = 0;
  good_features->push_back(f);
  int num_features = 1;
  for (int level = 0; level < levels; ++level) {
    for (int i = prev_num_features; i < num_features; ++i) {
      int feature = (*good_features)[i];
      for (int dir = -kNumOffsetMaps; dir <= kNumOffsetMaps; ++dir) {
        if (dir == 0) continue;
        int f1 = feature_map.OffsetFeature(feature, dir);
        if (f1 >= 0) {
          good_features->push_back(f1);
        }
      }
    }
    prev_num_features = num_features;
    num_features = good_features->size();
  }
}

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
int TrainingSampleSet::ReliablySeparable(int font_id1, int class_id1,
                                         int font_id2, int class_id2,
                                         const IntFeatureMap& feature_map,
                                         bool thorough) const {
  int result = 0;
  const TrainingSample* sample2 = GetCanonicalSample(font_id2, class_id2);
  if (sample2 == NULL)
    return 0;  // There are no canonical features.
  const GenericVector<int>& canonical2 = GetCanonicalFeatures(font_id2,
                                                              class_id2);
  const BitVector& cloud1 = GetCloudFeatures(font_id1, class_id1);
  if (cloud1.size() == 0)
    return canonical2.size();  // There are no cloud features.

  // Find a canonical2 feature that is not in cloud1.
  for (int f = 0; f < canonical2.size(); ++f) {
    int feature = canonical2[f];
    if (cloud1[feature])
      continue;
    // Gather the near neighbours of f.
    GenericVector<int> good_features;
    AddNearFeatures(feature_map, feature, 1, &good_features);
    // Check that none of the good_features are in the cloud.
    int i;
    for (i = 0; i < good_features.size(); ++i) {
      int good_f = good_features[i];
      if (cloud1[good_f]) {
        break;
      }
    }
    if (i < good_features.size())
       continue;  // Found one in the cloud.
    ++result;
  }
  return result;
}

// Returns the total index of the requested sample.
// OrganizeByFontAndClass must have been already called.
int TrainingSampleSet::GlobalSampleIndex(int font_id, int class_id,
                                         int index) const {
  ASSERT_HOST(font_class_array_ != NULL);
  int font_index = font_id_map_.SparseToCompact(font_id);
  if (font_index < 0) return -1;
  return (*font_class_array_)(font_index, class_id).samples[index];
}

// Gets the canonical sample for the given font, class pair.
// ComputeCanonicalSamples must have been called first.
const TrainingSample* TrainingSampleSet::GetCanonicalSample(
    int font_id, int class_id) const {
  ASSERT_HOST(font_class_array_ != NULL);
  int font_index = font_id_map_.SparseToCompact(font_id);
  if (font_index < 0) return NULL;
  int sample_index = (*font_class_array_)(font_index,
                                          class_id).canonical_sample;
  return sample_index >= 0 ? samples_[sample_index] : NULL;
}

// Gets the max distance for the given canonical sample.
// ComputeCanonicalSamples must have been called first.
float TrainingSampleSet::GetCanonicalDist(int font_id, int class_id) const {
  ASSERT_HOST(font_class_array_ != NULL);
  int font_index = font_id_map_.SparseToCompact(font_id);
  if (font_index < 0) return 0.0f;
  if ((*font_class_array_)(font_index, class_id).canonical_sample >= 0)
    return (*font_class_array_)(font_index, class_id).canonical_dist;
  else
    return 0.0f;
}

// Generates indexed features for all samples with the supplied feature_space.
void TrainingSampleSet::IndexFeatures(const IntFeatureSpace& feature_space) {
  for (int s = 0; s < samples_.size(); ++s)
    samples_[s]->IndexFeatures(feature_space);
}

// Delete outlier samples with few features that are shared with others.
// IndexFeatures must have been called already.
void TrainingSampleSet::DeleteOutliers(const IntFeatureSpace& feature_space,
                                       bool debug) {
  if (font_class_array_ == NULL)
    OrganizeByFontAndClass();
  Pixa* pixa = NULL;
  if (debug)
    pixa = pixaCreate(0);
  GenericVector<int> feature_counts;
  int fs_size = feature_space.Size();
  int font_size = font_id_map_.CompactSize();
  for (int font_index = 0; font_index < font_size; ++font_index) {
    for (int c = 0; c < unicharset_size_; ++c) {
      // Create a histogram of the features used by all samples of this
      // font/class combination.
      feature_counts.init_to_size(fs_size, 0);
      FontClassInfo& fcinfo = (*font_class_array_)(font_index, c);
      int sample_count = fcinfo.samples.size();
      if (sample_count < kMinOutlierSamples)
        continue;
      for (int i = 0; i < sample_count; ++i) {
        int s = fcinfo.samples[i];
        const GenericVector<int>& features = samples_[s]->indexed_features();
        for (int f = 0; f < features.size(); ++f) {
          ++feature_counts[features[f]];
        }
      }
      for (int i = 0; i < sample_count; ++i) {
        int s = fcinfo.samples[i];
        const TrainingSample& sample = *samples_[s];
        const GenericVector<int>& features = sample.indexed_features();
        // A feature that has a histogram count of 1 is only used by this
        // sample, making it 'bad'. All others are 'good'.
        int good_features = 0;
        int bad_features = 0;
        for (int f = 0; f < features.size(); ++f) {
          if (feature_counts[features[f]] > 1)
            ++good_features;
          else
            ++bad_features;
        }
        // If more than 1/3 features are bad, then this is an outlier.
        if (bad_features * 2 > good_features) {
          tprintf("Deleting outlier sample of %s, %d good, %d bad\n",
                  SampleToString(sample).string(),
                  good_features, bad_features);
          if (debug) {
            pixaAddPix(pixa, sample.RenderToPix(&unicharset_), L_INSERT);
            // Add the previous sample as well, so it is easier to see in
            // the output what is wrong with this sample.
            int t;
            if (i == 0)
              t = fcinfo.samples[1];
            else
              t = fcinfo.samples[i - 1];
            const TrainingSample &csample = *samples_[t];
            pixaAddPix(pixa, csample.RenderToPix(&unicharset_), L_INSERT);
          }
          // Mark the sample for deletion.
          KillSample(samples_[s]);
        }
      }
    }
  }
  // Truly delete all bad samples and renumber everything.
  DeleteDeadSamples();
  if (pixa != NULL) {
    Pix* pix = pixaDisplayTiledInRows(pixa, 1, 2600, 1.0, 0, 10, 10);
    pixaDestroy(&pixa);
    pixWrite("outliers.png", pix, IFF_PNG);
    pixDestroy(&pix);
  }
}

// Marks the given sample index for deletion.
// Deletion is actually completed by DeleteDeadSamples.
void TrainingSampleSet::KillSample(TrainingSample* sample) {
  sample->set_sample_index(-1);
}

// Deletes all samples with zero features marked by KillSample.
void TrainingSampleSet::DeleteDeadSamples() {
  samples_.compact(
      NewPermanentTessCallback(this, &TrainingSampleSet::DeleteableSample));
  num_raw_samples_ = samples_.size();
  // Samples must be re-organized now we have deleted a few.
}

// Callback function returns true if the given sample is to be deleted, due
// to having a negative classid.
bool TrainingSampleSet::DeleteableSample(const TrainingSample* sample) {
  return sample == NULL || sample->class_id() < 0;
}

static Pix* DebugSample(const UNICHARSET& unicharset,
                        TrainingSample* sample) {
  tprintf("\nOriginal features:\n");
  for (int i = 0; i < sample->num_features(); ++i) {
    sample->features()[i].print();
  }
  if (sample->features_are_mapped()) {
    tprintf("\nMapped features:\n");
    for (int i = 0; i < sample->mapped_features().size(); ++i) {
      tprintf("%d ", sample->mapped_features()[i]);
    }
    tprintf("\n");
  }
  return sample->RenderToPix(&unicharset);
}

// Construct an array to access the samples by font,class pair.
void TrainingSampleSet::OrganizeByFontAndClass() {
  // Font indexes are sparse, so we used a map to compact them, so we can
  // have an efficient 2-d array of fonts and character classes.
  SetupFontIdMap();
  int compact_font_size = font_id_map_.CompactSize();
  // Get a 2-d array of generic vectors.
  if (font_class_array_ != NULL)
    delete font_class_array_;
  FontClassInfo empty;
  font_class_array_ = new GENERIC_2D_ARRAY<FontClassInfo>(
      compact_font_size, unicharset_size_, empty);
  for (int s = 0; s < samples_.size(); ++s) {
    int font_id = samples_[s]->font_id();
    int class_id = samples_[s]->class_id();
    if (font_id < 0 || font_id >= font_id_map_.SparseSize()) {
      tprintf("Font id = %d/%d, class id = %d/%d on sample %d\n",
              font_id, font_id_map_.SparseSize(), class_id, unicharset_size_,
              s);
    }
    ASSERT_HOST(font_id >= 0 && font_id < font_id_map_.SparseSize());
    ASSERT_HOST(class_id >= 0 && class_id < unicharset_size_);
    int font_index = font_id_map_.SparseToCompact(font_id);
    (*font_class_array_)(font_index, class_id).samples.push_back(s);
  }
  // Set the num_raw_samples member of the FontClassInfo, to set the boundary
  // between the raw samples and the replicated ones.
  for (int f = 0; f < compact_font_size; ++f) {
    for (int c = 0; c < unicharset_size_; ++c)
      (*font_class_array_)(f, c).num_raw_samples =
          (*font_class_array_)(f, c).samples.size();
  }
  // This is the global number of samples and also marks the boundary between
  // real and replicated samples.
  num_raw_samples_ = samples_.size();
}

// Constructs the font_id_map_ which maps real font_ids (sparse) to a compact
// index for the font_class_array_.
void TrainingSampleSet::SetupFontIdMap() {
  // Number of samples for each font_id.
  GenericVector<int> font_counts;
  for (int s = 0; s < samples_.size(); ++s) {
    int font_id = samples_[s]->font_id();
    while (font_id >= font_counts.size())
      font_counts.push_back(0);
    ++font_counts[font_id];
  }
  font_id_map_.Init(font_counts.size(), false);
  for (int f = 0; f < font_counts.size(); ++f) {
    font_id_map_.SetMap(f, font_counts[f] > 0);
  }
  font_id_map_.Setup();
}


// Finds the sample for each font, class pair that has least maximum
// distance to all the other samples of the same font, class.
// OrganizeByFontAndClass must have been already called.
void TrainingSampleSet::ComputeCanonicalSamples(const IntFeatureMap& map,
                                                bool debug) {
  ASSERT_HOST(font_class_array_ != NULL);
  IntFeatureDist f_table;
  if (debug) tprintf("feature table size %d\n", map.sparse_size());
  f_table.Init(&map);
  int worst_s1 = 0;
  int worst_s2 = 0;
  double global_worst_dist = 0.0;
  // Compute distances independently for each font and char index.
  int font_size = font_id_map_.CompactSize();
  for (int font_index = 0; font_index < font_size; ++font_index) {
    int font_id = font_id_map_.CompactToSparse(font_index);
    for (int c = 0; c < unicharset_size_; ++c) {
      int samples_found = 0;
      FontClassInfo& fcinfo = (*font_class_array_)(font_index, c);
      if (fcinfo.samples.size() == 0 ||
          (kTestChar >= 0 && c != kTestChar)) {
        fcinfo.canonical_sample = -1;
        fcinfo.canonical_dist = 0.0f;
        if (debug) tprintf("Skipping class %d\n", c);
        continue;
      }
      // The canonical sample will be the one with the min_max_dist, which
      // is the sample with the lowest maximum distance to all other samples.
      double min_max_dist = 2.0;
      // We keep track of the farthest apart pair (max_s1, max_s2) which
      // are max_max_dist apart, so we can see how bad the variability is.
      double max_max_dist = 0.0;
      int max_s1 = 0;
      int max_s2 = 0;
      fcinfo.canonical_sample = fcinfo.samples[0];
      fcinfo.canonical_dist = 0.0f;
      for (int i = 0; i < fcinfo.samples.size(); ++i) {
        int s1 = fcinfo.samples[i];
        const GenericVector<int>& features1 = samples_[s1]->indexed_features();
        f_table.Set(features1, features1.size(), true);
        double max_dist = 0.0;
        // Run the full squared-order search for similar samples. It is still
        // reasonably fast because f_table.FeatureDistance is fast, but we
        // may have to reconsider if we start playing with too many samples
        // of a single char/font.
        for (int j = 0; j < fcinfo.samples.size(); ++j) {
          int s2 = fcinfo.samples[j];
          if (samples_[s2]->class_id() != c  ||
              samples_[s2]->font_id() != font_id ||
              s2 == s1)
            continue;
          GenericVector<int> features2 = samples_[s2]->indexed_features();
          double dist = f_table.FeatureDistance(features2);
          if (dist > max_dist) {
            max_dist = dist;
            if (dist > max_max_dist) {
              max_s1 = s1;
              max_s2 = s2;
            }
          }
        }
        // Using Set(..., false) is far faster than re initializing, due to
        // the sparseness of the feature space.
        f_table.Set(features1, features1.size(), false);
        samples_[s1]->set_max_dist(max_dist);
        ++samples_found;
        if (max_dist < min_max_dist) {
          fcinfo.canonical_sample = s1;
          fcinfo.canonical_dist = max_dist;
        }
        UpdateRange(max_dist, &min_max_dist, &max_max_dist);
      }
      if (max_max_dist > global_worst_dist) {
        // Keep a record of the worst pair over all characters/fonts too.
        global_worst_dist = max_max_dist;
        worst_s1 = max_s1;
        worst_s2 = max_s2;
      }
      if (debug) {
        tprintf("Found %d samples of class %d=%s, font %d, "
                "dist range [%g, %g], worst pair= %s, %s\n",
                samples_found, c, unicharset_.debug_str(c).string(),
                font_index, min_max_dist, max_max_dist,
                SampleToString(*samples_[max_s1]).string(),
                SampleToString(*samples_[max_s2]).string());
      }
    }
  }
  if (debug) {
    tprintf("Global worst dist = %g, between sample %d and %d\n",
            global_worst_dist, worst_s1, worst_s2);
    Pix* pix1 = DebugSample(unicharset_, samples_[worst_s1]);
    Pix* pix2 = DebugSample(unicharset_, samples_[worst_s2]);
    pixOr(pix1, pix1, pix2);
    pixWrite("worstpair.png", pix1, IFF_PNG);
    pixDestroy(&pix1);
    pixDestroy(&pix2);
  }
}

// Replicates the samples to a minimum frequency defined by
// 2 * kSampleRandomSize, or for larger counts duplicates all samples.
// After replication, the replicated samples are perturbed slightly, but
// in a predictable and repeatable way.
// Use after OrganizeByFontAndClass().
void TrainingSampleSet::ReplicateAndRandomizeSamples() {
  ASSERT_HOST(font_class_array_ != NULL);
  int font_size = font_id_map_.CompactSize();
  for (int font_index = 0; font_index < font_size; ++font_index) {
    for (int c = 0; c < unicharset_size_; ++c) {
      FontClassInfo& fcinfo = (*font_class_array_)(font_index, c);
      int sample_count = fcinfo.samples.size();
      int min_samples = 2 * MAX(kSampleRandomSize, sample_count);
      if (sample_count > 0 && sample_count < min_samples) {
        int base_count = sample_count;
        for (int base_index = 0; sample_count < min_samples; ++sample_count) {
          int src_index = fcinfo.samples[base_index++];
          if (base_index >= base_count) base_index = 0;
          TrainingSample* sample = samples_[src_index]->RandomizedCopy(
              sample_count % kSampleRandomSize);
          int sample_index = samples_.size();
          sample->set_sample_index(sample_index);
          samples_.push_back(sample);
          fcinfo.samples.push_back(sample_index);
        }
      }
    }
  }
}

// Caches the indexed features of the canonical samples.
// ComputeCanonicalSamples must have been already called.
// TODO(rays) see note on ReliablySeparable and try restricting the
// canonical features to those that truly represent all samples.
void TrainingSampleSet::ComputeCanonicalFeatures() {
  ASSERT_HOST(font_class_array_ != NULL);
  int font_size = font_id_map_.CompactSize();
  for (int font_index = 0; font_index < font_size; ++font_index) {
    int font_id = font_id_map_.CompactToSparse(font_index);
    for (int c = 0; c < unicharset_size_; ++c) {
      int num_samples = NumClassSamples(font_id, c, false);
      if (num_samples == 0)
        continue;
      const TrainingSample* sample = GetCanonicalSample(font_id, c);
      FontClassInfo& fcinfo = (*font_class_array_)(font_index, c);
      fcinfo.canonical_features = sample->indexed_features();
    }
  }
}

// Computes the combined set of features used by all the samples of each
// font/class combination. Use after ReplicateAndRandomizeSamples.
void TrainingSampleSet::ComputeCloudFeatures(int feature_space_size) {
  ASSERT_HOST(font_class_array_ != NULL);
  int font_size = font_id_map_.CompactSize();
  for (int font_index = 0; font_index < font_size; ++font_index) {
    int font_id = font_id_map_.CompactToSparse(font_index);
    for (int c = 0; c < unicharset_size_; ++c) {
      int num_samples = NumClassSamples(font_id, c, false);
      if (num_samples == 0)
        continue;
      FontClassInfo& fcinfo = (*font_class_array_)(font_index, c);
      fcinfo.cloud_features.Init(feature_space_size);
      for (int s = 0; s < num_samples; ++s) {
        const TrainingSample* sample = GetSample(font_id, c, s);
        const GenericVector<int>& sample_features = sample->indexed_features();
        for (int i = 0; i < sample_features.size(); ++i)
          fcinfo.cloud_features.SetBit(sample_features[i]);
      }
    }
  }
}

// Adds all fonts of the given class to the shape.
void TrainingSampleSet::AddAllFontsForClass(int class_id, Shape* shape) const {
  for (int f = 0; f < font_id_map_.CompactSize(); ++f) {
    int font_id = font_id_map_.CompactToSparse(f);
    shape->AddToShape(class_id, font_id);
  }
}

// Display the samples with the given indexed feature that also match
// the given shape.
void TrainingSampleSet::DisplaySamplesWithFeature(int f_index,
                                                  const Shape& shape,
                                                  const IntFeatureSpace& space,
                                                  ScrollView::Color color,
                                                  ScrollView* window) const {
  for (int s = 0; s < num_raw_samples(); ++s) {
    const TrainingSample* sample = GetSample(s);
    if (shape.ContainsUnichar(sample->class_id())) {
      GenericVector<int> indexed_features;
      space.IndexAndSortFeatures(sample->features(), sample->num_features(),
                                 &indexed_features);
      for (int f = 0; f < indexed_features.size(); ++f) {
        if (indexed_features[f] == f_index) {
          sample->DisplayFeatures(color, window);
        }
      }
    }
  }
}


}  // namespace tesseract.
