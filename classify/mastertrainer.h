// Copyright 2010 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        mastertrainer.h
// Description: Trainer to build the MasterClassifier.
// Author:      Ray Smith
// Created:     Wed Nov 03 18:07:01 PDT 2010
//
// (C) Copyright 2010, Google Inc.
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

#ifndef TESSERACT_TRAINING_MASTERTRAINER_H__
#define TESSERACT_TRAINING_MASTERTRAINER_H__

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "classify.h"
#include "cluster.h"
#include "intfx.h"
#include "elst.h"
#include "errorcounter.h"
#include "featdefs.h"
#include "fontinfo.h"
#include "indexmapbidi.h"
#include "intfeaturespace.h"
#include "intfeaturemap.h"
#include "intmatcher.h"
#include "params.h"
#include "shapetable.h"
#include "trainingsample.h"
#include "trainingsampleset.h"
#include "unicharset.h"

namespace tesseract {

class ShapeClassifier;

// Simple struct to hold the distance between two shapes during clustering.
struct ShapeDist {
  ShapeDist() : shape1(0), shape2(0), distance(0.0f) {}
  ShapeDist(int s1, int s2, float dist)
    : shape1(s1), shape2(s2), distance(dist) {}

  // Sort operator to sort in ascending order of distance.
  bool operator<(const ShapeDist& other) const {
    return distance < other.distance;
  }

  int shape1;
  int shape2;
  float distance;
};

// Class to encapsulate training processes that use the TrainingSampleSet.
// Initially supports shape clustering and mftrainining.
// Other important features of the MasterTrainer are conditioning the data
// by outlier elimination, replication with perturbation, and serialization.
class MasterTrainer {
 public:
  MasterTrainer(NormalizationMode norm_mode, bool shape_analysis,
                bool replicate_samples, int debug_level);
  ~MasterTrainer();

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp);

  // Loads an initial unicharset, or sets one up if the file cannot be read.
  void LoadUnicharset(const char* filename);

  // Sets the feature space definition.
  void SetFeatureSpace(const IntFeatureSpace& fs) {
    feature_space_ = fs;
    feature_map_.Init(fs);
  }

  // Reads the samples and their features from the given file,
  // adding them to the trainer with the font_id from the content of the file.
  // If verification, then these are verification samples, not training.
  void ReadTrainingSamples(const char* page_name,
                           const FEATURE_DEFS_STRUCT& feature_defs,
                           bool verification);

  // Adds the given single sample to the trainer, setting the classid
  // appropriately from the given unichar_str.
  void AddSample(bool verification, const char* unichar_str,
                 TrainingSample* sample);

  // Loads all pages from the given tif filename and append to page_images_.
  // Must be called after ReadTrainingSamples, as the current number of images
  // is used as an offset for page numbers in the samples.
  void LoadPageImages(const char* filename);

  // Cleans up the samples after initial load from the tr files, and prior to
  // saving the MasterTrainer:
  // Remaps fragmented chars if running shape anaylsis.
  // Sets up the samples appropriately for class/fontwise access.
  // Deletes outlier samples.
  void PostLoadCleanup();

  // Gets the samples ready for training. Use after both
  // ReadTrainingSamples+PostLoadCleanup or DeSerialize.
  // Re-indexes the features and computes canonical and cloud features.
  void PreTrainingSetup();

  // Sets up the master_shapes_ table, which tells which fonts should stay
  // together until they get to a leaf node classifier.
  void SetupMasterShapes();

  // Adds the junk_samples_ to the main samples_ set. Junk samples are initially
  // fragments and n-grams (all incorrectly segmented characters).
  // Various training functions may result in incorrectly segmented characters
  // being added to the unicharset of the main samples, perhaps because they
  // form a "radical" decomposition of some (Indic) grapheme, or because they
  // just look the same as a real character (like rn/m)
  // This function moves all the junk samples, to the main samples_ set, but
  // desirable junk, being any sample for which the unichar already exists in
  // the samples_ unicharset gets the unichar-ids re-indexed to match, but
  // anything else gets re-marked as unichar_id 0 (space character) to identify
  // it as junk to the error counter.
  void IncludeJunk();

  // Replicates the samples and perturbs them if the enable_replication_ flag
  // is set. MUST be used after the last call to OrganizeByFontAndClass on
  // the training samples, ie after IncludeJunk if it is going to be used, as
  // OrganizeByFontAndClass will eat the replicated samples into the regular
  // samples.
  void ReplicateAndRandomizeSamplesIfRequired();

  // Loads the basic font properties file into fontinfo_table_.
  // Returns false on failure.
  bool LoadFontInfo(const char* filename);

  // Loads the xheight font properties file into xheights_.
  // Returns false on failure.
  bool LoadXHeights(const char* filename);

  // Reads spacing stats from filename and adds them to fontinfo_table.
  // Returns false on failure.
  bool AddSpacingInfo(const char *filename);

  // Returns the font id corresponding to the given font name.
  // Returns -1 if the font cannot be found.
  int GetFontInfoId(const char* font_name);
  // Returns the font_id of the closest matching font name to the given
  // filename. It is assumed that a substring of the filename will match
  // one of the fonts. If more than one is matched, the longest is returned.
  int GetBestMatchingFontInfoId(const char* filename);

  // Returns the filename of the tr file corresponding to the command-line
  // argument with the given index.
  const STRING& GetTRFileName(int index) const {
    return tr_filenames_[index];
  }

  // Sets up a flat shapetable with one shape per class/font combination.
  void SetupFlatShapeTable(ShapeTable* shape_table);

  // Sets up a Clusterer for mftraining on a single shape_id.
  // Call FreeClusterer on the return value after use.
  CLUSTERER* SetupForClustering(const ShapeTable& shape_table,
                                const FEATURE_DEFS_STRUCT& feature_defs,
                                int shape_id, int* num_samples);

  // Writes the given float_classes (produced by SetupForFloat2Int) as inttemp
  // to the given inttemp_file, and the corresponding pffmtable.
  // The unicharset is the original encoding of graphemes, and shape_set should
  // match the size of the shape_table, and may possibly be totally fake.
  void WriteInttempAndPFFMTable(const UNICHARSET& unicharset,
                                const UNICHARSET& shape_set,
                                const ShapeTable& shape_table,
                                CLASS_STRUCT* float_classes,
                                const char* inttemp_file,
                                const char* pffmtable_file);

  const UNICHARSET& unicharset() const {
    return samples_.unicharset();
  }
  TrainingSampleSet* GetSamples() {
    return &samples_;
  }
  const ShapeTable& master_shapes() const {
    return master_shapes_;
  }

  // Generates debug output relating to the canonical distance between the
  // two given UTF8 grapheme strings.
  void DebugCanonical(const char* unichar_str1, const char* unichar_str2);
  #ifndef GRAPHICS_DISABLED
  // Debugging for cloud/canonical features.
  // Displays a Features window containing:
  // If unichar_str2 is in the unicharset, and canonical_font is non-negative,
  // displays the canonical features of the char/font combination in red.
  // If unichar_str1 is in the unicharset, and cloud_font is non-negative,
  // displays the cloud feature of the char/font combination in green.
  // The canonical features are drawn first to show which ones have no
  // matches in the cloud features.
  // Until the features window is destroyed, each click in the features window
  // will display the samples that have that feature in a separate window.
  void DisplaySamples(const char* unichar_str1, int cloud_font,
                      const char* unichar_str2, int canonical_font);
  #endif  // GRAPHICS_DISABLED

  void TestClassifierVOld(bool replicate_samples,
                          ShapeClassifier* test_classifier,
                          ShapeClassifier* old_classifier);

  // Tests the given test_classifier on the internal samples.
  // See TestClassifier for details.
  void TestClassifierOnSamples(CountTypes error_mode,
                               int report_level,
                               bool replicate_samples,
                               ShapeClassifier* test_classifier,
                               STRING* report_string);
  // Tests the given test_classifier on the given samples
  // error_mode indicates what counts as an error.
  // report_levels:
  // 0 = no output.
  // 1 = bottom-line error rate.
  // 2 = bottom-line error rate + time.
  // 3 = font-level error rate + time.
  // 4 = list of all errors + short classifier debug output on 16 errors.
  // 5 = list of all errors + short classifier debug output on 25 errors.
  // If replicate_samples is true, then the test is run on an extended test
  // sample including replicated and systematically perturbed samples.
  // If report_string is non-NULL, a summary of the results for each font
  // is appended to the report_string.
  double TestClassifier(CountTypes error_mode,
                        int report_level,
                        bool replicate_samples,
                        TrainingSampleSet* samples,
                        ShapeClassifier* test_classifier,
                        STRING* report_string);

  // Returns the average (in some sense) distance between the two given
  // shapes, which may contain multiple fonts and/or unichars.
  // This function is public to facilitate testing.
  float ShapeDistance(const ShapeTable& shapes, int s1, int s2);

 private:
  // Replaces samples that are always fragmented with the corresponding
  // fragment samples.
  void ReplaceFragmentedSamples();

  // Runs a hierarchical agglomerative clustering to merge shapes in the given
  // shape_table, while satisfying the given constraints:
  // * End with at least min_shapes left in shape_table,
  // * No shape shall have more than max_shape_unichars in it,
  // * Don't merge shapes where the distance between them exceeds max_dist.
  void ClusterShapes(int min_shapes, int max_shape_unichars,
                     float max_dist, ShapeTable* shape_table);

 private:
  NormalizationMode norm_mode_;
  // Character set we are training for.
  UNICHARSET unicharset_;
  // Original feature space. Subspace mapping is contained in feature_map_.
  IntFeatureSpace feature_space_;
  TrainingSampleSet samples_;
  TrainingSampleSet junk_samples_;
  TrainingSampleSet verify_samples_;
  // Master shape table defines what fonts stay together until the leaves.
  ShapeTable master_shapes_;
  // Flat shape table has each unichar/font id pair in a separate shape.
  ShapeTable flat_shapes_;
  // Font metrics gathered from multiple files.
  FontInfoTable fontinfo_table_;
  // Array of xheights indexed by font ids in fontinfo_table_;
  GenericVector<inT32> xheights_;

  // Non-serialized data initialized by other means or used temporarily
  // during loading of training samples.
  // Number of different class labels in unicharset_.
  int charsetsize_;
  // Flag to indicate that we are running shape analysis and need fragments
  // fixing.
  bool enable_shape_anaylsis_;
  // Flag to indicate that sample replication is required.
  bool enable_replication_;
  // Array of classids of fragments that replace the correctly segmented chars.
  int* fragments_;
  // Classid of previous correctly segmented sample that was added.
  int prev_unichar_id_;
  // Debug output control.
  int debug_level_;
  // Feature map used to construct reduced feature spaces for compact
  // classifiers.
  IntFeatureMap feature_map_;
  // Vector of Pix pointers used for classifiers that need the image.
  // Indexed by page_num_ in the samples.
  // These images are owned by the trainer and need to be pixDestroyed.
  GenericVector<Pix*> page_images_;
  // Vector of filenames of loaded tr files.
  GenericVector<STRING> tr_filenames_;
};

}  // namespace tesseract.

#endif
