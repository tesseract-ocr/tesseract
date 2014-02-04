// Copyright 2010 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        mastertrainer.cpp
// Description: Trainer to build the MasterClassifier.
// Author:      Ray Smith
// Created:     Wed Nov 03 18:10:01 PDT 2010
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "mastertrainer.h"
#include <math.h>
#include <time.h>
#include "allheaders.h"
#include "boxread.h"
#include "classify.h"
#include "efio.h"
#include "errorcounter.h"
#include "featdefs.h"
#include "sampleiterator.h"
#include "shapeclassifier.h"
#include "shapetable.h"
#include "svmnode.h"

namespace tesseract {

// Constants controlling clustering. With a low kMinClusteredShapes and a high
// kMaxUnicharsPerCluster, then kFontMergeDistance is the only limiting factor.
// Min number of shapes in the output.
const int kMinClusteredShapes = 1;
// Max number of unichars in any individual cluster.
const int kMaxUnicharsPerCluster = 2000;
// Mean font distance below which to merge fonts and unichars.
const float kFontMergeDistance = 0.025;

MasterTrainer::MasterTrainer(NormalizationMode norm_mode,
                             bool shape_analysis,
                             bool replicate_samples,
                             int debug_level)
  : norm_mode_(norm_mode), samples_(fontinfo_table_),
    junk_samples_(fontinfo_table_), verify_samples_(fontinfo_table_),
    charsetsize_(0),
    enable_shape_anaylsis_(shape_analysis),
    enable_replication_(replicate_samples),
    fragments_(NULL), prev_unichar_id_(-1), debug_level_(debug_level) {
}

MasterTrainer::~MasterTrainer() {
  delete [] fragments_;
  for (int p = 0; p < page_images_.size(); ++p)
    pixDestroy(&page_images_[p]);
}

// WARNING! Serialize/DeSerialize are only partial, providing
// enough data to get the samples back and display them.
// Writes to the given file. Returns false in case of error.
bool MasterTrainer::Serialize(FILE* fp) const {
  if (fwrite(&norm_mode_, sizeof(norm_mode_), 1, fp) != 1) return false;
  if (!unicharset_.save_to_file(fp)) return false;
  if (!feature_space_.Serialize(fp)) return false;
  if (!samples_.Serialize(fp)) return false;
  if (!junk_samples_.Serialize(fp)) return false;
  if (!verify_samples_.Serialize(fp)) return false;
  if (!master_shapes_.Serialize(fp)) return false;
  if (!flat_shapes_.Serialize(fp)) return false;
  if (!fontinfo_table_.Serialize(fp)) return false;
  if (!xheights_.Serialize(fp)) return false;
  return true;
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool MasterTrainer::DeSerialize(bool swap, FILE* fp) {
  if (fread(&norm_mode_, sizeof(norm_mode_), 1, fp) != 1) return false;
  if (swap) {
    ReverseN(&norm_mode_, sizeof(norm_mode_));
  }
  if (!unicharset_.load_from_file(fp)) return false;
  charsetsize_ = unicharset_.size();
  if (!feature_space_.DeSerialize(swap, fp)) return false;
  feature_map_.Init(feature_space_);
  if (!samples_.DeSerialize(swap, fp)) return false;
  if (!junk_samples_.DeSerialize(swap, fp)) return false;
  if (!verify_samples_.DeSerialize(swap, fp)) return false;
  if (!master_shapes_.DeSerialize(swap, fp)) return false;
  if (!flat_shapes_.DeSerialize(swap, fp)) return false;
  if (!fontinfo_table_.DeSerialize(swap, fp)) return false;
  if (!xheights_.DeSerialize(swap, fp)) return false;
  return true;
}

// Load an initial unicharset, or set one up if the file cannot be read.
void MasterTrainer::LoadUnicharset(const char* filename) {
  if (!unicharset_.load_from_file(filename)) {
    tprintf("Failed to load unicharset from file %s\n"
            "Building unicharset for training from scratch...\n",
            filename);
    unicharset_.clear();
    UNICHARSET initialized;
    // Add special characters, as they were removed by the clear, but the
    // default constructor puts them in.
    unicharset_.AppendOtherUnicharset(initialized);
  }
  charsetsize_ = unicharset_.size();
  delete [] fragments_;
  fragments_ = new int[charsetsize_];
  memset(fragments_, 0, sizeof(*fragments_) * charsetsize_);
  samples_.LoadUnicharset(filename);
  junk_samples_.LoadUnicharset(filename);
  verify_samples_.LoadUnicharset(filename);
}

// Reads the samples and their features from the given .tr format file,
// adding them to the trainer with the font_id from the content of the file.
// See mftraining.cpp for a description of the file format.
// If verification, then these are verification samples, not training.
void MasterTrainer::ReadTrainingSamples(const char* page_name,
                                        const FEATURE_DEFS_STRUCT& feature_defs,
                                        bool verification) {
  char buffer[2048];
  int int_feature_type = ShortNameToFeatureType(feature_defs, kIntFeatureType);
  int micro_feature_type = ShortNameToFeatureType(feature_defs,
                                                  kMicroFeatureType);
  int cn_feature_type = ShortNameToFeatureType(feature_defs, kCNFeatureType);
  int geo_feature_type = ShortNameToFeatureType(feature_defs, kGeoFeatureType);

  FILE* fp = Efopen(page_name, "rb");
  if (fp == NULL) {
    tprintf("Failed to open tr file: %s\n", page_name);
    return;
  }
  tr_filenames_.push_back(STRING(page_name));
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    if (buffer[0] == '\n')
      continue;

    char* space = strchr(buffer, ' ');
    if (space == NULL) {
      tprintf("Bad format in tr file, reading fontname, unichar\n");
      continue;
    }
    *space++ = '\0';
    int font_id = GetFontInfoId(buffer);
    if (font_id < 0) font_id = 0;
    int page_number;
    STRING unichar;
    TBOX bounding_box;
    if (!ParseBoxFileStr(space, &page_number, &unichar, &bounding_box)) {
      tprintf("Bad format in tr file, reading box coords\n");
      continue;
    }
    CHAR_DESC char_desc = ReadCharDescription(feature_defs, fp);
    TrainingSample* sample = new TrainingSample;
    sample->set_font_id(font_id);
    sample->set_page_num(page_number + page_images_.size());
    sample->set_bounding_box(bounding_box);
    sample->ExtractCharDesc(int_feature_type, micro_feature_type,
                            cn_feature_type, geo_feature_type, char_desc);
    AddSample(verification, unichar.string(), sample);
    FreeCharDescription(char_desc);
  }
  charsetsize_ = unicharset_.size();
  fclose(fp);
}

// Adds the given single sample to the trainer, setting the classid
// appropriately from the given unichar_str.
void MasterTrainer::AddSample(bool verification, const char* unichar,
                              TrainingSample* sample) {
  if (verification) {
    verify_samples_.AddSample(unichar, sample);
    prev_unichar_id_ = -1;
  } else if (unicharset_.contains_unichar(unichar)) {
    if (prev_unichar_id_ >= 0)
      fragments_[prev_unichar_id_] = -1;
    prev_unichar_id_ = samples_.AddSample(unichar, sample);
    if (flat_shapes_.FindShape(prev_unichar_id_, sample->font_id()) < 0)
      flat_shapes_.AddShape(prev_unichar_id_, sample->font_id());
  } else {
    int junk_id = junk_samples_.AddSample(unichar, sample);
    if (prev_unichar_id_ >= 0) {
      CHAR_FRAGMENT* frag = CHAR_FRAGMENT::parse_from_string(unichar);
      if (frag != NULL && frag->is_natural()) {
        if (fragments_[prev_unichar_id_] == 0)
          fragments_[prev_unichar_id_] = junk_id;
        else if (fragments_[prev_unichar_id_] != junk_id)
          fragments_[prev_unichar_id_] = -1;
      }
      delete frag;
    }
    prev_unichar_id_ = -1;
  }
}

// Loads all pages from the given tif filename and append to page_images_.
// Must be called after ReadTrainingSamples, as the current number of images
// is used as an offset for page numbers in the samples.
void MasterTrainer::LoadPageImages(const char* filename) {
  int page;
  Pix* pix;
  for (page = 0; (pix = pixReadTiff(filename, page)) != NULL; ++page) {
    page_images_.push_back(pix);
  }
  tprintf("Loaded %d page images from %s\n", page, filename);
}

// Cleans up the samples after initial load from the tr files, and prior to
// saving the MasterTrainer:
// Remaps fragmented chars if running shape anaylsis.
// Sets up the samples appropriately for class/fontwise access.
// Deletes outlier samples.
void MasterTrainer::PostLoadCleanup() {
  if (debug_level_ > 0)
    tprintf("PostLoadCleanup...\n");
  if (enable_shape_anaylsis_)
    ReplaceFragmentedSamples();
  SampleIterator sample_it;
  sample_it.Init(NULL, NULL, true, &verify_samples_);
  sample_it.NormalizeSamples();
  verify_samples_.OrganizeByFontAndClass();

  samples_.IndexFeatures(feature_space_);
  // TODO(rays) DeleteOutliers is currently turned off to prove NOP-ness
  // against current training.
  //  samples_.DeleteOutliers(feature_space_, debug_level_ > 0);
  samples_.OrganizeByFontAndClass();
  if (debug_level_ > 0)
    tprintf("ComputeCanonicalSamples...\n");
  samples_.ComputeCanonicalSamples(feature_map_, debug_level_ > 0);
}

// Gets the samples ready for training. Use after both
// ReadTrainingSamples+PostLoadCleanup or DeSerialize.
// Re-indexes the features and computes canonical and cloud features.
void MasterTrainer::PreTrainingSetup() {
  if (debug_level_ > 0)
    tprintf("PreTrainingSetup...\n");
  samples_.IndexFeatures(feature_space_);
  samples_.ComputeCanonicalFeatures();
  if (debug_level_ > 0)
    tprintf("ComputeCloudFeatures...\n");
  samples_.ComputeCloudFeatures(feature_space_.Size());
}

// Sets up the master_shapes_ table, which tells which fonts should stay
// together until they get to a leaf node classifier.
void MasterTrainer::SetupMasterShapes() {
  tprintf("Building master shape table\n");
  int num_fonts = samples_.NumFonts();

  ShapeTable char_shapes_begin_fragment(samples_.unicharset());
  ShapeTable char_shapes_end_fragment(samples_.unicharset());
  ShapeTable char_shapes(samples_.unicharset());
  for (int c = 0; c < samples_.charsetsize(); ++c) {
    ShapeTable shapes(samples_.unicharset());
    for (int f = 0; f < num_fonts; ++f) {
      if (samples_.NumClassSamples(f, c, true) > 0)
        shapes.AddShape(c, f);
    }
    ClusterShapes(kMinClusteredShapes, 1, kFontMergeDistance, &shapes);

    const CHAR_FRAGMENT *fragment = samples_.unicharset().get_fragment(c);

    if (fragment == NULL)
      char_shapes.AppendMasterShapes(shapes, NULL);
    else if (fragment->is_beginning())
      char_shapes_begin_fragment.AppendMasterShapes(shapes, NULL);
    else if (fragment->is_ending())
      char_shapes_end_fragment.AppendMasterShapes(shapes, NULL);
    else
      char_shapes.AppendMasterShapes(shapes, NULL);
  }
  ClusterShapes(kMinClusteredShapes, kMaxUnicharsPerCluster,
                kFontMergeDistance, &char_shapes_begin_fragment);
  char_shapes.AppendMasterShapes(char_shapes_begin_fragment, NULL);
  ClusterShapes(kMinClusteredShapes, kMaxUnicharsPerCluster,
                kFontMergeDistance, &char_shapes_end_fragment);
  char_shapes.AppendMasterShapes(char_shapes_end_fragment, NULL);
  ClusterShapes(kMinClusteredShapes, kMaxUnicharsPerCluster,
                kFontMergeDistance, &char_shapes);
  master_shapes_.AppendMasterShapes(char_shapes, NULL);
  tprintf("Master shape_table:%s\n", master_shapes_.SummaryStr().string());
}

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
void MasterTrainer::IncludeJunk() {
  // Get ids of fragments in junk_samples_ that replace the dead chars.
  const UNICHARSET& junk_set = junk_samples_.unicharset();
  const UNICHARSET& sample_set = samples_.unicharset();
  int num_junks = junk_samples_.num_samples();
  tprintf("Moving %d junk samples to master sample set.\n", num_junks);
  for (int s = 0; s < num_junks; ++s) {
    TrainingSample* sample = junk_samples_.mutable_sample(s);
    int junk_id = sample->class_id();
    const char* junk_utf8 = junk_set.id_to_unichar(junk_id);
    int sample_id = sample_set.unichar_to_id(junk_utf8);
    if (sample_id == INVALID_UNICHAR_ID)
      sample_id = 0;
    sample->set_class_id(sample_id);
    junk_samples_.extract_sample(s);
    samples_.AddSample(sample_id, sample);
  }
  junk_samples_.DeleteDeadSamples();
  samples_.OrganizeByFontAndClass();
}

// Replicates the samples and perturbs them if the enable_replication_ flag
// is set. MUST be used after the last call to OrganizeByFontAndClass on
// the training samples, ie after IncludeJunk if it is going to be used, as
// OrganizeByFontAndClass will eat the replicated samples into the regular
// samples.
void MasterTrainer::ReplicateAndRandomizeSamplesIfRequired() {
  if (enable_replication_) {
    if (debug_level_ > 0)
      tprintf("ReplicateAndRandomize...\n");
    verify_samples_.ReplicateAndRandomizeSamples();
    samples_.ReplicateAndRandomizeSamples();
    samples_.IndexFeatures(feature_space_);
  }
}

// Loads the basic font properties file into fontinfo_table_.
// Returns false on failure.
bool MasterTrainer::LoadFontInfo(const char* filename) {
  FILE* fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to load font_properties from %s\n", filename);
    return false;
  }
  int italic, bold, fixed, serif, fraktur;
  while (!feof(fp)) {
    FontInfo fontinfo;
    char* font_name = new char[1024];
    fontinfo.name = font_name;
    fontinfo.properties = 0;
    fontinfo.universal_id = 0;
    if (fscanf(fp, "%1024s %i %i %i %i %i\n", font_name,
               &italic, &bold, &fixed, &serif, &fraktur) != 6)
      continue;
    fontinfo.properties =
        (italic << 0) +
        (bold << 1) +
        (fixed << 2) +
        (serif << 3) +
        (fraktur << 4);
    if (!fontinfo_table_.contains(fontinfo)) {
      fontinfo_table_.push_back(fontinfo);
    }
  }
  fclose(fp);
  return true;
}

// Loads the xheight font properties file into xheights_.
// Returns false on failure.
bool MasterTrainer::LoadXHeights(const char* filename) {
  tprintf("fontinfo table is of size %d\n", fontinfo_table_.size());
  xheights_.init_to_size(fontinfo_table_.size(), -1);
  if (filename == NULL) return true;
  FILE *f = fopen(filename, "rb");
  if (f == NULL) {
    fprintf(stderr, "Failed to load font xheights from %s\n", filename);
    return false;
  }
  tprintf("Reading x-heights from %s ...\n", filename);
  FontInfo fontinfo;
  fontinfo.properties = 0;  // Not used to lookup in the table.
  fontinfo.universal_id = 0;
  char buffer[1024];
  int xht;
  int total_xheight = 0;
  int xheight_count = 0;
  while (!feof(f)) {
    if (fscanf(f, "%1023s %d\n", buffer, &xht) != 2)
      continue;
    buffer[1023] = '\0';
    fontinfo.name = buffer;
    if (!fontinfo_table_.contains(fontinfo)) continue;
    int fontinfo_id = fontinfo_table_.get_index(fontinfo);
    xheights_[fontinfo_id] = xht;
    total_xheight += xht;
    ++xheight_count;
  }
  if (xheight_count == 0) {
    fprintf(stderr, "No valid xheights in %s!\n", filename);
    fclose(f);
    return false;
  }
  int mean_xheight = DivRounded(total_xheight, xheight_count);
  for (int i = 0; i < fontinfo_table_.size(); ++i) {
    if (xheights_[i] < 0)
      xheights_[i] = mean_xheight;
  }
  fclose(f);
  return true;
}  // LoadXHeights

// Reads spacing stats from filename and adds them to fontinfo_table.
bool MasterTrainer::AddSpacingInfo(const char *filename) {
  FILE* fontinfo_file = fopen(filename, "rb");
  if (fontinfo_file == NULL)
    return true;  // We silently ignore missing files!
  // Find the fontinfo_id.
  int fontinfo_id = GetBestMatchingFontInfoId(filename);
  if (fontinfo_id < 0) {
    tprintf("No font found matching fontinfo filename %s\n", filename);
    fclose(fontinfo_file);
    return false;
  }
  tprintf("Reading spacing from %s for font %d...\n", filename, fontinfo_id);
  // TODO(rays) scale should probably be a double, but keep as an int for now
  // to duplicate current behavior.
  int scale = kBlnXHeight / xheights_[fontinfo_id];
  int num_unichars;
  char uch[UNICHAR_LEN];
  char kerned_uch[UNICHAR_LEN];
  int x_gap, x_gap_before, x_gap_after, num_kerned;
  ASSERT_HOST(fscanf(fontinfo_file, "%d\n", &num_unichars) == 1);
  FontInfo *fi = &fontinfo_table_.get(fontinfo_id);
  fi->init_spacing(unicharset_.size());
  FontSpacingInfo *spacing = NULL;
  for (int l = 0; l < num_unichars; ++l) {
    if (fscanf(fontinfo_file, "%s %d %d %d",
               uch, &x_gap_before, &x_gap_after, &num_kerned) != 4) {
      tprintf("Bad format of font spacing file %s\n", filename);
      fclose(fontinfo_file);
      return false;
    }
    bool valid = unicharset_.contains_unichar(uch);
    if (valid) {
      spacing = new FontSpacingInfo();
      spacing->x_gap_before = static_cast<inT16>(x_gap_before * scale);
      spacing->x_gap_after = static_cast<inT16>(x_gap_after * scale);
    }
    for (int k = 0; k < num_kerned; ++k) {
      if (fscanf(fontinfo_file, "%s %d", kerned_uch, &x_gap) != 2) {
        tprintf("Bad format of font spacing file %s\n", filename);
        fclose(fontinfo_file);
        delete spacing;
        return false;
      }
      if (!valid || !unicharset_.contains_unichar(kerned_uch)) continue;
      spacing->kerned_unichar_ids.push_back(
          unicharset_.unichar_to_id(kerned_uch));
      spacing->kerned_x_gaps.push_back(static_cast<inT16>(x_gap * scale));
    }
    if (valid) fi->add_spacing(unicharset_.unichar_to_id(uch), spacing);
  }
  fclose(fontinfo_file);
  return true;
}

// Returns the font id corresponding to the given font name.
// Returns -1 if the font cannot be found.
int MasterTrainer::GetFontInfoId(const char* font_name) {
  FontInfo fontinfo;
  // We are only borrowing the string, so it is OK to const cast it.
  fontinfo.name = const_cast<char*>(font_name);
  fontinfo.properties = 0;  // Not used to lookup in the table
  fontinfo.universal_id = 0;
  return fontinfo_table_.get_index(fontinfo);
}
// Returns the font_id of the closest matching font name to the given
// filename. It is assumed that a substring of the filename will match
// one of the fonts. If more than one is matched, the longest is returned.
int MasterTrainer::GetBestMatchingFontInfoId(const char* filename) {
  int fontinfo_id = -1;
  int best_len = 0;
  for (int f = 0; f < fontinfo_table_.size(); ++f) {
    if (strstr(filename, fontinfo_table_.get(f).name) != NULL) {
      int len = strlen(fontinfo_table_.get(f).name);
      // Use the longest matching length in case a substring of a font matched.
      if (len > best_len) {
        best_len = len;
        fontinfo_id = f;
      }
    }
  }
  return fontinfo_id;
}

// Sets up a flat shapetable with one shape per class/font combination.
void MasterTrainer::SetupFlatShapeTable(ShapeTable* shape_table) {
  // To exactly mimic the results of the previous implementation, the shapes
  // must be clustered in order the fonts arrived, and reverse order of the
  // characters within each font.
  // Get a list of the fonts in the order they appeared.
  GenericVector<int> active_fonts;
  int num_shapes = flat_shapes_.NumShapes();
  for (int s = 0; s < num_shapes; ++s) {
    int font = flat_shapes_.GetShape(s)[0].font_ids[0];
    int f = 0;
    for (f = 0; f < active_fonts.size(); ++f) {
      if (active_fonts[f] == font)
        break;
    }
    if (f == active_fonts.size())
      active_fonts.push_back(font);
  }
  // For each font in order, add all the shapes with that font in reverse order.
  int num_fonts = active_fonts.size();
  for (int f = 0; f < num_fonts; ++f) {
    for (int s = num_shapes - 1; s >= 0; --s) {
      int font = flat_shapes_.GetShape(s)[0].font_ids[0];
      if (font == active_fonts[f]) {
        shape_table->AddShape(flat_shapes_.GetShape(s));
      }
    }
  }
}

// Sets up a Clusterer for mftraining on a single shape_id.
// Call FreeClusterer on the return value after use.
CLUSTERER* MasterTrainer::SetupForClustering(
    const ShapeTable& shape_table,
    const FEATURE_DEFS_STRUCT& feature_defs,
    int shape_id,
    int* num_samples) {

  int desc_index = ShortNameToFeatureType(feature_defs, kMicroFeatureType);
  int num_params = feature_defs.FeatureDesc[desc_index]->NumParams;
  ASSERT_HOST(num_params == MFCount);
  CLUSTERER* clusterer = MakeClusterer(
      num_params, feature_defs.FeatureDesc[desc_index]->ParamDesc);

  // We want to iterate over the samples of just the one shape.
  IndexMapBiDi shape_map;
  shape_map.Init(shape_table.NumShapes(), false);
  shape_map.SetMap(shape_id, true);
  shape_map.Setup();
  // Reverse the order of the samples to match the previous behavior.
  GenericVector<const TrainingSample*> sample_ptrs;
  SampleIterator it;
  it.Init(&shape_map, &shape_table, false, &samples_);
  for (it.Begin(); !it.AtEnd(); it.Next()) {
    sample_ptrs.push_back(&it.GetSample());
  }
  int sample_id = 0;
  for (int i = sample_ptrs.size() - 1; i >= 0; --i) {
    const TrainingSample* sample = sample_ptrs[i];
    int num_features = sample->num_micro_features();
    for (int f = 0; f < num_features; ++f)
      MakeSample(clusterer, sample->micro_features()[f], sample_id);
    ++sample_id;
  }
  *num_samples = sample_id;
  return clusterer;
}

// Writes the given float_classes (produced by SetupForFloat2Int) as inttemp
// to the given inttemp_file, and the corresponding pffmtable.
// The unicharset is the original encoding of graphemes, and shape_set should
// match the size of the shape_table, and may possibly be totally fake.
void MasterTrainer::WriteInttempAndPFFMTable(const UNICHARSET& unicharset,
                                             const UNICHARSET& shape_set,
                                             const ShapeTable& shape_table,
                                             CLASS_STRUCT* float_classes,
                                             const char* inttemp_file,
                                             const char* pffmtable_file) {
  tesseract::Classify *classify = new tesseract::Classify();
  // Move the fontinfo table to classify.
  fontinfo_table_.MoveTo(&classify->get_fontinfo_table());
  INT_TEMPLATES int_templates = classify->CreateIntTemplates(float_classes,
                                                             shape_set);
  FILE* fp = fopen(inttemp_file, "wb");
  classify->WriteIntTemplates(fp, int_templates, shape_set);
  fclose(fp);
  // Now write pffmtable. This is complicated by the fact that the adaptive
  // classifier still wants one indexed by unichar-id, but the static
  // classifier needs one indexed by its shape class id.
  // We put the shapetable_cutoffs in a GenericVector, and compute the
  // unicharset cutoffs along the way.
  GenericVector<uinT16> shapetable_cutoffs;
  GenericVector<uinT16> unichar_cutoffs;
  for (int c = 0; c < unicharset.size(); ++c)
    unichar_cutoffs.push_back(0);
  /* then write out each class */
  for (int i = 0; i < int_templates->NumClasses; ++i) {
    INT_CLASS Class = ClassForClassId(int_templates, i);
    // Todo: Test with min instead of max
    // int MaxLength = LengthForConfigId(Class, 0);
    uinT16 max_length = 0;
    for (int config_id = 0; config_id < Class->NumConfigs; config_id++) {
      // Todo: Test with min instead of max
      // if (LengthForConfigId (Class, config_id) < MaxLength)
      uinT16 length = Class->ConfigLengths[config_id];
      if (length > max_length)
        max_length = Class->ConfigLengths[config_id];
      int shape_id = float_classes[i].font_set.get(config_id);
      const Shape& shape = shape_table.GetShape(shape_id);
      for (int c = 0; c < shape.size(); ++c) {
        int unichar_id = shape[c].unichar_id;
        if (length > unichar_cutoffs[unichar_id])
          unichar_cutoffs[unichar_id] = length;
      }
    }
    shapetable_cutoffs.push_back(max_length);
  }
  fp = fopen(pffmtable_file, "wb");
  shapetable_cutoffs.Serialize(fp);
  for (int c = 0; c < unicharset.size(); ++c) {
    const char *unichar = unicharset.id_to_unichar(c);
    if (strcmp(unichar, " ") == 0) {
      unichar = "NULL";
    }
    fprintf(fp, "%s %d\n", unichar, unichar_cutoffs[c]);
  }
  fclose(fp);
  free_int_templates(int_templates);
}

// Generate debug output relating to the canonical distance between the
// two given UTF8 grapheme strings.
void MasterTrainer::DebugCanonical(const char* unichar_str1,
                                   const char* unichar_str2) {
  int class_id1 = unicharset_.unichar_to_id(unichar_str1);
  int class_id2 = unicharset_.unichar_to_id(unichar_str2);
  if (class_id2 == INVALID_UNICHAR_ID)
    class_id2 = class_id1;
  if (class_id1 == INVALID_UNICHAR_ID) {
    tprintf("No unicharset entry found for %s\n", unichar_str1);
    return;
  } else {
    tprintf("Font ambiguities for unichar %d = %s and %d = %s\n",
            class_id1, unichar_str1, class_id2, unichar_str2);
  }
  int num_fonts = samples_.NumFonts();
  const IntFeatureMap& feature_map = feature_map_;
  // Iterate the fonts to get the similarity with other fonst of the same
  // class.
  tprintf("      ");
  for (int f = 0; f < num_fonts; ++f) {
    if (samples_.NumClassSamples(f, class_id2, false) == 0)
      continue;
    tprintf("%6d", f);
  }
  tprintf("\n");
  for (int f1 = 0; f1 < num_fonts; ++f1) {
    // Map the features of the canonical_sample.
    if (samples_.NumClassSamples(f1, class_id1, false) == 0)
      continue;
    tprintf("%4d  ", f1);
    for (int f2 = 0; f2 < num_fonts; ++f2) {
      if (samples_.NumClassSamples(f2, class_id2, false) == 0)
        continue;
      float dist = samples_.ClusterDistance(f1, class_id1, f2, class_id2,
                                            feature_map);
      tprintf(" %5.3f", dist);
    }
    tprintf("\n");
  }
  // Build a fake ShapeTable containing all the sample types.
  ShapeTable shapes(unicharset_);
  for (int f = 0; f < num_fonts; ++f) {
    if (samples_.NumClassSamples(f, class_id1, true) > 0)
      shapes.AddShape(class_id1, f);
    if (class_id1 != class_id2 &&
        samples_.NumClassSamples(f, class_id2, true) > 0)
      shapes.AddShape(class_id2, f);
  }
}

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
void MasterTrainer::DisplaySamples(const char* unichar_str1, int cloud_font,
                                   const char* unichar_str2,
                                   int canonical_font) {
  const IntFeatureMap& feature_map = feature_map_;
  const IntFeatureSpace& feature_space = feature_map.feature_space();
  ScrollView* f_window = CreateFeatureSpaceWindow("Features", 100, 500);
  ClearFeatureSpaceWindow(norm_mode_ == NM_BASELINE ? baseline : character,
                          f_window);
  int class_id2 = samples_.unicharset().unichar_to_id(unichar_str2);
  if (class_id2 != INVALID_UNICHAR_ID && canonical_font >= 0) {
    const TrainingSample* sample = samples_.GetCanonicalSample(canonical_font,
                                                               class_id2);
    for (int f = 0; f < sample->num_features(); ++f) {
      RenderIntFeature(f_window, &sample->features()[f], ScrollView::RED);
    }
  }
  int class_id1 = samples_.unicharset().unichar_to_id(unichar_str1);
  if (class_id1 != INVALID_UNICHAR_ID && cloud_font >= 0) {
    const BitVector& cloud = samples_.GetCloudFeatures(cloud_font, class_id1);
    for (int f = 0; f < cloud.size(); ++f) {
      if (cloud[f]) {
        INT_FEATURE_STRUCT feature =
            feature_map.InverseIndexFeature(f);
        RenderIntFeature(f_window, &feature, ScrollView::GREEN);
      }
    }
  }
  f_window->Update();
  ScrollView* s_window = CreateFeatureSpaceWindow("Samples", 100, 500);
  SVEventType ev_type;
  do {
    SVEvent* ev;
    // Wait until a click or popup event.
    ev = f_window->AwaitEvent(SVET_ANY);
    ev_type = ev->type;
    if (ev_type == SVET_CLICK) {
      int feature_index = feature_space.XYToFeatureIndex(ev->x, ev->y);
      if (feature_index >= 0) {
        // Iterate samples and display those with the feature.
        Shape shape;
        shape.AddToShape(class_id1, cloud_font);
        s_window->Clear();
        samples_.DisplaySamplesWithFeature(feature_index, shape,
                                           feature_space, ScrollView::GREEN,
                                           s_window);
        s_window->Update();
      }
    }
    delete ev;
  } while (ev_type != SVET_DESTROY);
}
#endif  // GRAPHICS_DISABLED

void MasterTrainer::TestClassifierVOld(bool replicate_samples,
                                       ShapeClassifier* test_classifier,
                                       ShapeClassifier* old_classifier) {
  SampleIterator sample_it;
  sample_it.Init(NULL, NULL, replicate_samples, &samples_);
  ErrorCounter::DebugNewErrors(test_classifier, old_classifier,
                               CT_UNICHAR_TOPN_ERR, fontinfo_table_,
                               page_images_, &sample_it);
}

// Tests the given test_classifier on the internal samples.
// See TestClassifier for details.
void MasterTrainer::TestClassifierOnSamples(CountTypes error_mode,
                                            int report_level,
                                            bool replicate_samples,
                                            ShapeClassifier* test_classifier,
                                            STRING* report_string) {
  TestClassifier(error_mode, report_level, replicate_samples, &samples_,
                 test_classifier, report_string);
}

// Tests the given test_classifier on the given samples.
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
double MasterTrainer::TestClassifier(CountTypes error_mode,
                                     int report_level,
                                     bool replicate_samples,
                                     TrainingSampleSet* samples,
                                     ShapeClassifier* test_classifier,
                                     STRING* report_string) {
  SampleIterator sample_it;
  sample_it.Init(NULL, NULL, replicate_samples, samples);
  if (report_level > 0) {
    int num_samples = 0;
    for (sample_it.Begin(); !sample_it.AtEnd(); sample_it.Next())
      ++num_samples;
    tprintf("Iterator has charset size of %d/%d, %d shapes, %d samples\n",
            sample_it.SparseCharsetSize(), sample_it.CompactCharsetSize(),
            test_classifier->GetShapeTable()->NumShapes(), num_samples);
    tprintf("Testing %sREPLICATED:\n", replicate_samples ? "" : "NON-");
  }
  double unichar_error = 0.0;
  ErrorCounter::ComputeErrorRate(test_classifier, report_level,
                                 error_mode, fontinfo_table_,
                                 page_images_, &sample_it, &unichar_error,
                                 NULL, report_string);
  return unichar_error;
}

// Returns the average (in some sense) distance between the two given
// shapes, which may contain multiple fonts and/or unichars.
float MasterTrainer::ShapeDistance(const ShapeTable& shapes, int s1, int s2) {
  const IntFeatureMap& feature_map = feature_map_;
  const Shape& shape1 = shapes.GetShape(s1);
  const Shape& shape2 = shapes.GetShape(s2);
  int num_chars1 = shape1.size();
  int num_chars2 = shape2.size();
  float dist_sum = 0.0f;
  int dist_count = 0;
  if (num_chars1 > 1 || num_chars2 > 1) {
    // In the multi-char case try to optimize the calculation by computing
    // distances between characters of matching font where possible.
    for (int c1 = 0; c1 < num_chars1; ++c1) {
      for (int c2 = 0; c2 < num_chars2; ++c2) {
        dist_sum += samples_.UnicharDistance(shape1[c1], shape2[c2],
                                             true, feature_map);
        ++dist_count;
      }
    }
  } else {
    // In the single unichar case, there is little alternative, but to compute
    // the squared-order distance between pairs of fonts.
    dist_sum = samples_.UnicharDistance(shape1[0], shape2[0],
                                        false, feature_map);
    ++dist_count;
  }
  return dist_sum / dist_count;
}

// Replaces samples that are always fragmented with the corresponding
// fragment samples.
void MasterTrainer::ReplaceFragmentedSamples() {
  if (fragments_ == NULL) return;
  // Remove samples that are replaced by fragments. Each class that was
  // always naturally fragmented should be replaced by its fragments.
  int num_samples = samples_.num_samples();
  for (int s = 0; s < num_samples; ++s) {
    TrainingSample* sample = samples_.mutable_sample(s);
    if (fragments_[sample->class_id()] > 0)
      samples_.KillSample(sample);
  }
  samples_.DeleteDeadSamples();

  // Get ids of fragments in junk_samples_ that replace the dead chars.
  const UNICHARSET& frag_set = junk_samples_.unicharset();
#if 0
  // TODO(rays) The original idea was to replace only graphemes that were
  // always naturally fragmented, but that left a lot of the Indic graphemes
  // out. Determine whether we can go back to that idea now that spacing
  // is fixed in the training images, or whether this code is obsolete.
  bool* good_junk = new bool[frag_set.size()];
  memset(good_junk, 0, sizeof(*good_junk) * frag_set.size());
  for (int dead_ch = 1; dead_ch < unicharset_.size(); ++dead_ch) {
    int frag_ch = fragments_[dead_ch];
    if (frag_ch <= 0) continue;
    const char* frag_utf8 = frag_set.id_to_unichar(frag_ch);
    CHAR_FRAGMENT* frag = CHAR_FRAGMENT::parse_from_string(frag_utf8);
    // Mark the chars for all parts of the fragment as good in good_junk.
    for (int part = 0; part < frag->get_total(); ++part) {
      frag->set_pos(part);
      int good_ch = frag_set.unichar_to_id(frag->to_string().string());
      if (good_ch != INVALID_UNICHAR_ID)
        good_junk[good_ch] = true;  // We want this one.
    }
  }
#endif
  // For now just use all the junk that was from natural fragments.
  // Get samples of fragments in junk_samples_ that replace the dead chars.
  int num_junks = junk_samples_.num_samples();
  for (int s = 0; s < num_junks; ++s) {
    TrainingSample* sample = junk_samples_.mutable_sample(s);
    int junk_id = sample->class_id();
    const char* frag_utf8 = frag_set.id_to_unichar(junk_id);
    CHAR_FRAGMENT* frag = CHAR_FRAGMENT::parse_from_string(frag_utf8);
    if (frag != NULL && frag->is_natural()) {
      junk_samples_.extract_sample(s);
      samples_.AddSample(frag_set.id_to_unichar(junk_id), sample);
    }
  }
  junk_samples_.DeleteDeadSamples();
  junk_samples_.OrganizeByFontAndClass();
  samples_.OrganizeByFontAndClass();
  unicharset_.clear();
  unicharset_.AppendOtherUnicharset(samples_.unicharset());
  // delete [] good_junk;
  // Fragments_ no longer needed?
  delete [] fragments_;
  fragments_ = NULL;
}

// Runs a hierarchical agglomerative clustering to merge shapes in the given
// shape_table, while satisfying the given constraints:
// * End with at least min_shapes left in shape_table,
// * No shape shall have more than max_shape_unichars in it,
// * Don't merge shapes where the distance between them exceeds max_dist.
const float kInfiniteDist = 999.0f;
void MasterTrainer::ClusterShapes(int min_shapes,  int max_shape_unichars,
                                  float max_dist, ShapeTable* shapes) {
  int num_shapes = shapes->NumShapes();
  int max_merges = num_shapes - min_shapes;
  GenericVector<ShapeDist>* shape_dists =
      new GenericVector<ShapeDist>[num_shapes];
  float min_dist = kInfiniteDist;
  int min_s1 = 0;
  int min_s2 = 0;
  tprintf("Computing shape distances...");
  for (int s1 = 0; s1 < num_shapes; ++s1) {
    for (int s2 = s1 + 1; s2 < num_shapes; ++s2) {
      ShapeDist dist(s1, s2, ShapeDistance(*shapes, s1, s2));
      shape_dists[s1].push_back(dist);
      if (dist.distance < min_dist) {
        min_dist = dist.distance;
        min_s1 = s1;
        min_s2 = s2;
      }
    }
    tprintf(" %d", s1);
  }
  tprintf("\n");
  int num_merged = 0;
  while (num_merged < max_merges && min_dist < max_dist) {
    tprintf("Distance = %f: ", min_dist);
    int num_unichars = shapes->MergedUnicharCount(min_s1, min_s2);
    shape_dists[min_s1][min_s2 - min_s1 - 1].distance = kInfiniteDist;
    if (num_unichars > max_shape_unichars) {
      tprintf("Merge of %d and %d with %d would exceed max of %d unichars\n",
              min_s1, min_s2, num_unichars, max_shape_unichars);
    } else {
      shapes->MergeShapes(min_s1, min_s2);
      shape_dists[min_s2].clear();
      ++num_merged;

      for (int s = 0; s < min_s1; ++s) {
        if (!shape_dists[s].empty()) {
          shape_dists[s][min_s1 - s - 1].distance =
              ShapeDistance(*shapes, s, min_s1);
          shape_dists[s][min_s2 - s -1].distance = kInfiniteDist;
        }
      }
      for (int s2 = min_s1 + 1; s2 < num_shapes; ++s2) {
        if (shape_dists[min_s1][s2 - min_s1 - 1].distance < kInfiniteDist)
          shape_dists[min_s1][s2 - min_s1 - 1].distance =
              ShapeDistance(*shapes, min_s1, s2);
      }
      for (int s = min_s1 + 1; s < min_s2; ++s) {
        if (!shape_dists[s].empty()) {
          shape_dists[s][min_s2 - s - 1].distance = kInfiniteDist;
        }
      }
    }
    min_dist = kInfiniteDist;
    for (int s1 = 0; s1 < num_shapes; ++s1) {
      for (int i = 0; i < shape_dists[s1].size(); ++i) {
        if (shape_dists[s1][i].distance < min_dist) {
          min_dist = shape_dists[s1][i].distance;
          min_s1 = s1;
          min_s2 = s1 + 1 + i;
        }
      }
    }
  }
  tprintf("Stopped with %d merged, min dist %f\n", num_merged, min_dist);
  delete [] shape_dists;
  if (debug_level_ > 1) {
    for (int s1 = 0; s1 < num_shapes; ++s1) {
      if (shapes->MasterDestinationIndex(s1) == s1) {
        tprintf("Master shape:%s\n", shapes->DebugStr(s1).string());
      }
    }
  }
}


}  // namespace tesseract.
