///////////////////////////////////////////////////////////////////////
// File:        classify.h
// Description: classify class.
// Author:      Samuel Charron
//
// (C) Copyright 2006, Google Inc.
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

#ifndef TESSERACT_CLASSIFY_CLASSIFY_H_
#define TESSERACT_CLASSIFY_CLASSIFY_H_

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#ifdef DISABLED_LEGACY_ENGINE

#  include "ccstruct.h"
#  include "dict.h"

namespace tesseract {

class Classify : public CCStruct {
public:
  Classify();
  virtual ~Classify();
  virtual Dict &getDict() {
    return dict_;
  }

  // Member variables.

  INT_VAR_H(classify_debug_level);
  BOOL_VAR_H(classify_bln_numeric_mode);
  double_VAR_H(classify_max_rating_ratio);
  double_VAR_H(classify_max_certainty_margin);

private:
  Dict dict_;
};

} // namespace tesseract

#else // DISABLED_LEGACY_ENGINE not defined

#  include "adaptive.h"
#  include "ccstruct.h"
#  include "dict.h"
#  include "featdefs.h"
#  include "fontinfo.h"
#  include "intfx.h"
#  include "intmatcher.h"
#  include "normalis.h"
#  include "ocrfeatures.h"
#  include "ratngs.h"
#  include "unicity_table.h"

namespace tesseract {

class ScrollView;
class WERD_CHOICE;
class WERD_RES;
struct ADAPT_RESULTS;
struct NORM_PROTOS;

static const int kUnknownFontinfoId = -1;
static const int kBlankFontinfoId = -2;

class ShapeClassifier;
struct ShapeRating;
class ShapeTable;
struct UnicharRating;

// How segmented is a blob. In this enum, character refers to a classifiable
// unit, but that is too long and character is usually easier to understand.
enum CharSegmentationType {
  CST_FRAGMENT, // A partial character.
  CST_WHOLE,    // A correctly segmented character.
  CST_IMPROPER, // More than one but less than 2 characters.
  CST_NGRAM     // Multiple characters.
};

class TESS_API Classify : public CCStruct {
public:
  Classify();
  ~Classify() override;
  virtual Dict &getDict() {
    return dict_;
  }

  const ShapeTable *shape_table() const {
    return shape_table_;
  }

  // Takes ownership of the given classifier, and uses it for future calls
  // to CharNormClassifier.
  void SetStaticClassifier(ShapeClassifier *static_classifier);

  // Adds a noise classification result that is a bit worse than the worst
  // current result, or the worst possible result if no current results.
  void AddLargeSpeckleTo(int blob_length, BLOB_CHOICE_LIST *choices);

  // Returns true if the blob is small enough to be a large speckle.
  bool LargeSpeckle(const TBLOB &blob);

  /* adaptive.cpp ************************************************************/
  int GetFontinfoId(ADAPT_CLASS_STRUCT *Class, uint8_t ConfigId);
  // Runs the class pruner from int_templates on the given features, returning
  // the number of classes output in results.
  //    int_templates          Class pruner tables
  //    num_features           Number of features in blob
  //    features               Array of features
  //    normalization_factors  (input) Array of int_templates->NumClasses fudge
  //                           factors from blob normalization process.
  //                           (Indexed by CLASS_INDEX)
  //    expected_num_features  (input) Array of int_templates->NumClasses
  //                           expected number of features for each class.
  //                           (Indexed by CLASS_INDEX)
  //    results                (output) Sorted Array of pruned classes.
  //                           Array must be sized to take the maximum possible
  //                           number of outputs : int_templates->NumClasses.
  int PruneClasses(const INT_TEMPLATES_STRUCT *int_templates, int num_features, int keep_this,
                   const INT_FEATURE_STRUCT *features, const uint8_t *normalization_factors,
                   const uint16_t *expected_num_features, std::vector<CP_RESULT_STRUCT> *results);
  void ReadNewCutoffs(TFile *fp, uint16_t *Cutoffs);
  void PrintAdaptedTemplates(FILE *File, ADAPT_TEMPLATES_STRUCT *Templates);
  void WriteAdaptedTemplates(FILE *File, ADAPT_TEMPLATES_STRUCT *Templates);
  ADAPT_TEMPLATES_STRUCT *ReadAdaptedTemplates(TFile *File);
  /* normmatch.cpp ************************************************************/
  float ComputeNormMatch(CLASS_ID ClassId, const FEATURE_STRUCT &feature, bool DebugMatch);
  void FreeNormProtos();
  NORM_PROTOS *ReadNormProtos(TFile *fp);
  /* protos.cpp ***************************************************************/
  void ConvertProto(PROTO_STRUCT *Proto, int ProtoId, INT_CLASS_STRUCT *Class);
  INT_TEMPLATES_STRUCT *CreateIntTemplates(CLASSES FloatProtos, const UNICHARSET &target_unicharset);
  /* adaptmatch.cpp ***********************************************************/

  // Learns the given word using its chopped_word, seam_array, denorm,
  // box_word, best_state, and correct_text to learn both correctly and
  // incorrectly segmented blobs. If fontname is not nullptr, then LearnBlob
  // is called and the data will be saved in an internal buffer.
  // Otherwise AdaptToBlob is called for adaption within a document.
  void LearnWord(const char *fontname, WERD_RES *word);

  // Builds a blob of length fragments, from the word, starting at start,
  // and then learns it, as having the given correct_text.
  // If fontname is not nullptr, then LearnBlob is called and the data will be
  // saved in an internal buffer for static training.
  // Otherwise AdaptToBlob is called for adaption within a document.
  // threshold is a magic number required by AdaptToChar and generated by
  // ComputeAdaptionThresholds.
  // Although it can be partly inferred from the string, segmentation is
  // provided to explicitly clarify the character segmentation.
  void LearnPieces(const char *fontname, int start, int length, float threshold,
                   CharSegmentationType segmentation, const char *correct_text, WERD_RES *word);
  void InitAdaptiveClassifier(TessdataManager *mgr);
  void InitAdaptedClass(TBLOB *Blob, CLASS_ID ClassId, int FontinfoId, ADAPT_CLASS_STRUCT *Class,
                        ADAPT_TEMPLATES_STRUCT *Templates);
  void AmbigClassifier(const std::vector<INT_FEATURE_STRUCT> &int_features,
                       const INT_FX_RESULT_STRUCT &fx_info, const TBLOB *blob,
                       INT_TEMPLATES_STRUCT *templates, ADAPT_CLASS_STRUCT **classes, UNICHAR_ID *ambiguities,
                       ADAPT_RESULTS *results);
  void MasterMatcher(INT_TEMPLATES_STRUCT *templates, int16_t num_features,
                     const INT_FEATURE_STRUCT *features, const uint8_t *norm_factors,
                     ADAPT_CLASS_STRUCT **classes, int debug, int matcher_multiplier, const TBOX &blob_box,
                     const std::vector<CP_RESULT_STRUCT> &results, ADAPT_RESULTS *final_results);
  // Converts configs to fonts, and if the result is not adapted, and a
  // shape_table_ is present, the shape is expanded to include all
  // unichar_ids represented, before applying a set of corrections to the
  // distance rating in int_result, (see ComputeCorrectedRating.)
  // The results are added to the final_results output.
  void ExpandShapesAndApplyCorrections(ADAPT_CLASS_STRUCT **classes, bool debug, int class_id, int bottom,
                                       int top, float cp_rating, int blob_length,
                                       int matcher_multiplier, const uint8_t *cn_factors,
                                       UnicharRating *int_result, ADAPT_RESULTS *final_results);
  // Applies a set of corrections to the distance im_rating,
  // including the cn_correction, miss penalty and additional penalty
  // for non-alnums being vertical misfits. Returns the corrected distance.
  double ComputeCorrectedRating(bool debug, int unichar_id, double cp_rating, double im_rating,
                                int feature_misses, int bottom, int top, int blob_length,
                                int matcher_multiplier, const uint8_t *cn_factors);
  void ConvertMatchesToChoices(const DENORM &denorm, const TBOX &box, ADAPT_RESULTS *Results,
                               BLOB_CHOICE_LIST *Choices);
  void AddNewResult(const UnicharRating &new_result, ADAPT_RESULTS *results);
  int GetAdaptiveFeatures(TBLOB *Blob, INT_FEATURE_ARRAY IntFeatures, FEATURE_SET *FloatFeatures);

#  ifndef GRAPHICS_DISABLED
  void DebugAdaptiveClassifier(TBLOB *Blob, ADAPT_RESULTS *Results);
#  endif
  PROTO_ID MakeNewTempProtos(FEATURE_SET Features, int NumBadFeat, FEATURE_ID BadFeat[],
                             INT_CLASS_STRUCT *IClass, ADAPT_CLASS_STRUCT *Class, BIT_VECTOR TempProtoMask);
  int MakeNewTemporaryConfig(ADAPT_TEMPLATES_STRUCT *Templates, CLASS_ID ClassId, int FontinfoId,
                             int NumFeatures, INT_FEATURE_ARRAY Features,
                             FEATURE_SET FloatFeatures);
  void MakePermanent(ADAPT_TEMPLATES_STRUCT *Templates, CLASS_ID ClassId, int ConfigId, TBLOB *Blob);
  void PrintAdaptiveMatchResults(const ADAPT_RESULTS &results);
  void RemoveExtraPuncs(ADAPT_RESULTS *Results);
  void RemoveBadMatches(ADAPT_RESULTS *Results);
  void SetAdaptiveThreshold(float Threshold);
  void ShowBestMatchFor(int shape_id, const INT_FEATURE_STRUCT *features, int num_features);
  // Returns a string for the classifier class_id: either the corresponding
  // unicharset debug_str or the shape_table_ debug str.
  std::string ClassIDToDebugStr(const INT_TEMPLATES_STRUCT *templates, int class_id,
                                int config_id) const;
  // Converts a classifier class_id index with a config ID to:
  // shape_table_ present: a shape_table_ index OR
  // No shape_table_: a font ID.
  // Without shape training, each class_id, config pair represents a single
  // unichar id/font combination, so this function looks up the corresponding
  // font id.
  // With shape training, each class_id, config pair represents a single
  // shape table index, so the fontset_table stores the shape table index,
  // and the shape_table_ must be consulted to obtain the actual unichar_id/
  // font combinations that the shape represents.
  int ClassAndConfigIDToFontOrShapeID(int class_id, int int_result_config) const;
  // Converts a shape_table_ index to a classifier class_id index (not a
  // unichar-id!). Uses a search, so not fast.
  int ShapeIDToClassID(int shape_id) const;
  UNICHAR_ID *BaselineClassifier(TBLOB *Blob, const std::vector<INT_FEATURE_STRUCT> &int_features,
                                 const INT_FX_RESULT_STRUCT &fx_info, ADAPT_TEMPLATES_STRUCT *Templates,
                                 ADAPT_RESULTS *Results);
  int CharNormClassifier(TBLOB *blob, const TrainingSample &sample, ADAPT_RESULTS *adapt_results);

  // As CharNormClassifier, but operates on a TrainingSample and outputs to
  // a vector of ShapeRating without conversion to classes.
  int CharNormTrainingSample(bool pruner_only, int keep_this, const TrainingSample &sample,
                             std::vector<UnicharRating> *results);
  UNICHAR_ID *GetAmbiguities(TBLOB *Blob, CLASS_ID CorrectClass);
  void DoAdaptiveMatch(TBLOB *Blob, ADAPT_RESULTS *Results);
  void AdaptToChar(TBLOB *Blob, CLASS_ID ClassId, int FontinfoId, float Threshold,
                   ADAPT_TEMPLATES_STRUCT *adaptive_templates);
  void DisplayAdaptedChar(TBLOB *blob, INT_CLASS_STRUCT *int_class);
  bool AdaptableWord(WERD_RES *word);
  void EndAdaptiveClassifier();
  void SetupPass1();
  void SetupPass2();
  void AdaptiveClassifier(TBLOB *Blob, BLOB_CHOICE_LIST *Choices);
  void ClassifyAsNoise(ADAPT_RESULTS *Results);
  void ResetAdaptiveClassifierInternal();
  void SwitchAdaptiveClassifier();
  void StartBackupAdaptiveClassifier();

  int GetCharNormFeature(const INT_FX_RESULT_STRUCT &fx_info, INT_TEMPLATES_STRUCT *templates,
                         uint8_t *pruner_norm_array, uint8_t *char_norm_array);
  // Computes the char_norm_array for the unicharset and, if not nullptr, the
  // pruner_array as appropriate according to the existence of the shape_table.
  // The norm_feature is deleted as it is almost certainly no longer needed.
  void ComputeCharNormArrays(FEATURE_STRUCT *norm_feature, INT_TEMPLATES_STRUCT *templates,
                             uint8_t *char_norm_array, uint8_t *pruner_array);

  bool TempConfigReliable(CLASS_ID class_id, const TEMP_CONFIG_STRUCT *config);
  void UpdateAmbigsGroup(CLASS_ID class_id, TBLOB *Blob);

  bool AdaptiveClassifierIsFull() const {
    return NumAdaptationsFailed > 0;
  }
  bool AdaptiveClassifierIsEmpty() const {
    return AdaptedTemplates->NumPermClasses == 0;
  }
  bool LooksLikeGarbage(TBLOB *blob);
#ifndef GRAPHICS_DISABLED
  void RefreshDebugWindow(ScrollView **win, const char *msg, int y_offset, const TBOX &wbox);
#endif
  // intfx.cpp
  // Computes the DENORMS for bl(baseline) and cn(character) normalization
  // during feature extraction. The input denorm describes the current state
  // of the blob, which is usually a baseline-normalized word.
  // The Transforms setup are as follows:
  // Baseline Normalized (bl) Output:
  //   We center the grapheme by aligning the x-coordinate of its centroid with
  //   x=128 and leaving the already-baseline-normalized y as-is.
  //
  // Character Normalized (cn) Output:
  //   We align the grapheme's centroid at the origin and scale it
  //   asymmetrically in x and y so that the 2nd moments are a standard value
  //   (51.2) ie the result is vaguely square.
  // If classify_nonlinear_norm is true:
  //   A non-linear normalization is setup that attempts to evenly distribute
  //   edges across x and y.
  //
  // Some of the fields of fx_info are also setup:
  // Length: Total length of outline.
  // Rx:     Rounded y second moment. (Reversed by convention.)
  // Ry:     rounded x second moment.
  // Xmean:  Rounded x center of mass of the blob.
  // Ymean:  Rounded y center of mass of the blob.
  static void SetupBLCNDenorms(const TBLOB &blob, bool nonlinear_norm, DENORM *bl_denorm,
                               DENORM *cn_denorm, INT_FX_RESULT_STRUCT *fx_info);

  // Extracts sets of 3-D features of length kStandardFeatureLength (=12.8), as
  // (x,y) position and angle as measured counterclockwise from the vector
  // <-1, 0>, from blob using two normalizations defined by bl_denorm and
  // cn_denorm. See SetpuBLCNDenorms for definitions.
  // If outline_cn_counts is not nullptr, on return it contains the cumulative
  // number of cn features generated for each outline in the blob (in order).
  // Thus after the first outline, there were (*outline_cn_counts)[0] features,
  // after the second outline, there were (*outline_cn_counts)[1] features etc.
  static void ExtractFeatures(const TBLOB &blob, bool nonlinear_norm,
                              std::vector<INT_FEATURE_STRUCT> *bl_features,
                              std::vector<INT_FEATURE_STRUCT> *cn_features,
                              INT_FX_RESULT_STRUCT *results, std::vector<int> *outline_cn_counts);
  /* float2int.cpp ************************************************************/
  void ClearCharNormArray(uint8_t *char_norm_array);
  void ComputeIntCharNormArray(const FEATURE_STRUCT &norm_feature, uint8_t *char_norm_array);
  void ComputeIntFeatures(FEATURE_SET Features, INT_FEATURE_ARRAY IntFeatures);
  /* intproto.cpp *************************************************************/
  INT_TEMPLATES_STRUCT *ReadIntTemplates(TFile *fp);
  void WriteIntTemplates(FILE *File, INT_TEMPLATES_STRUCT *Templates, const UNICHARSET &target_unicharset);
  CLASS_ID GetClassToDebug(const char *Prompt, bool *adaptive_on, bool *pretrained_on,
                           int *shape_id);
  void ShowMatchDisplay();
  /* font detection ***********************************************************/
  UnicityTable<FontInfo> &get_fontinfo_table() {
    return fontinfo_table_;
  }
  const UnicityTable<FontInfo> &get_fontinfo_table() const {
    return fontinfo_table_;
  }
  UnicityTable<FontSet> &get_fontset_table() {
    return fontset_table_;
  }
  /* mfoutline.cpp ***********************************************************/
  void NormalizeOutlines(LIST Outlines, float *XScale, float *YScale);
  /* outfeat.cpp ***********************************************************/
  FEATURE_SET ExtractOutlineFeatures(TBLOB *Blob);
  /* picofeat.cpp ***********************************************************/
  FEATURE_SET ExtractPicoFeatures(TBLOB *Blob);
  FEATURE_SET ExtractIntCNFeatures(const TBLOB &blob, const INT_FX_RESULT_STRUCT &fx_info);
  FEATURE_SET ExtractIntGeoFeatures(const TBLOB &blob, const INT_FX_RESULT_STRUCT &fx_info);
  /* blobclass.cpp ***********************************************************/
  // Extracts features from the given blob and saves them in the tr_file_data_
  // member variable.
  // fontname:  Name of font that this blob was printed in.
  // cn_denorm: Character normalization transformation to apply to the blob.
  // fx_info:   Character normalization parameters computed with cn_denorm.
  // blob_text: Ground truth text for the blob.
  void LearnBlob(const std::string &fontname, TBLOB *Blob, const DENORM &cn_denorm,
                 const INT_FX_RESULT_STRUCT &fx_info, const char *blob_text);
  // Writes stored training data to a .tr file based on the given filename.
  // Returns false on error.
  bool WriteTRFile(const char *filename);

  // Member variables.

  // Parameters.
  // Set during training (in lang.config) to indicate whether the divisible
  // blobs chopper should be used (true for latin script.)
  BOOL_VAR_H(allow_blob_division);
  // Set during training (in lang.config) to indicate whether the divisible
  // blobs chopper should be used in preference to chopping. Set to true for
  // southern Indic scripts.
  BOOL_VAR_H(prioritize_division);
  BOOL_VAR_H(classify_enable_learning);
  INT_VAR_H(classify_debug_level);

  /* mfoutline.cpp ***********************************************************/
  /* control knobs used to control normalization of outlines */
  INT_VAR_H(classify_norm_method);
  double_VAR_H(classify_char_norm_range);
  double_VAR_H(classify_max_rating_ratio);
  double_VAR_H(classify_max_certainty_margin);

  /* adaptmatch.cpp ***********************************************************/
  BOOL_VAR_H(tess_cn_matching);
  BOOL_VAR_H(tess_bn_matching);
  BOOL_VAR_H(classify_enable_adaptive_matcher);
  BOOL_VAR_H(classify_use_pre_adapted_templates);
  BOOL_VAR_H(classify_save_adapted_templates);
  BOOL_VAR_H(classify_enable_adaptive_debugger);
  BOOL_VAR_H(classify_nonlinear_norm);
  INT_VAR_H(matcher_debug_level);
  INT_VAR_H(matcher_debug_flags);
  INT_VAR_H(classify_learning_debug_level);
  double_VAR_H(matcher_good_threshold);
  double_VAR_H(matcher_reliable_adaptive_result);
  double_VAR_H(matcher_perfect_threshold);
  double_VAR_H(matcher_bad_match_pad);
  double_VAR_H(matcher_rating_margin);
  double_VAR_H(matcher_avg_noise_size);
  INT_VAR_H(matcher_permanent_classes_min);
  INT_VAR_H(matcher_min_examples_for_prototyping);
  INT_VAR_H(matcher_sufficient_examples_for_prototyping);
  double_VAR_H(matcher_clustering_max_angle_delta);
  double_VAR_H(classify_misfit_junk_penalty);
  double_VAR_H(rating_scale);
  double_VAR_H(tessedit_class_miss_scale);
  double_VAR_H(classify_adapted_pruning_factor);
  double_VAR_H(classify_adapted_pruning_threshold);
  INT_VAR_H(classify_adapt_proto_threshold);
  INT_VAR_H(classify_adapt_feature_threshold);
  BOOL_VAR_H(disable_character_fragments);
  double_VAR_H(classify_character_fragments_garbage_certainty_threshold);
  BOOL_VAR_H(classify_debug_character_fragments);
  BOOL_VAR_H(matcher_debug_separate_windows);
  STRING_VAR_H(classify_learn_debug_str);

  /* intmatcher.cpp **********************************************************/
  INT_VAR_H(classify_class_pruner_threshold);
  INT_VAR_H(classify_class_pruner_multiplier);
  INT_VAR_H(classify_cp_cutoff_strength);
  INT_VAR_H(classify_integer_matcher_multiplier);

  BOOL_VAR_H(classify_bln_numeric_mode);
  double_VAR_H(speckle_large_max_size);
  double_VAR_H(speckle_rating_penalty);

  // Use class variables to hold onto built-in templates and adapted templates.
  INT_TEMPLATES_STRUCT *PreTrainedTemplates = nullptr;
  ADAPT_TEMPLATES_STRUCT *AdaptedTemplates = nullptr;
  // The backup adapted templates are created from the previous page (only)
  // so they are always ready and reasonably well trained if the primary
  // adapted templates become full.
  ADAPT_TEMPLATES_STRUCT *BackupAdaptedTemplates = nullptr;

  // Create dummy proto and config masks for use with the built-in templates.
  BIT_VECTOR AllProtosOn = nullptr;
  BIT_VECTOR AllConfigsOn = nullptr;
  BIT_VECTOR AllConfigsOff = nullptr;
  BIT_VECTOR TempProtoMask = nullptr;
  /* normmatch.cpp */
  NORM_PROTOS *NormProtos = nullptr;
  /* font detection ***********************************************************/
  UnicityTable<FontInfo> fontinfo_table_;
  // Without shape training, each class_id, config pair represents a single
  // unichar id/font combination, so each fontset_table_ entry holds font ids
  // for each config in the class.
  // With shape training, each class_id, config pair represents a single
  // shape_table_ index, so the fontset_table_ stores the shape_table_ index,
  // and the shape_table_ must be consulted to obtain the actual unichar_id/
  // font combinations that the shape represents.
  UnicityTable<FontSet> fontset_table_;

protected:
  IntegerMatcher im_;
  FEATURE_DEFS_STRUCT feature_defs_;
  // If a shape_table_ is present, it is used to remap classifier output in
  // ExpandShapesAndApplyCorrections. font_ids referenced by configs actually
  // mean an index to the shape_table_ and the choices returned are *all* the
  // shape_table_ entries at that index.
  ShapeTable *shape_table_ = nullptr;

private:
  // The currently active static classifier.
  ShapeClassifier *static_classifier_ = nullptr;
#ifndef GRAPHICS_DISABLED
  ScrollView *learn_debug_win_ = nullptr;
  ScrollView *learn_fragmented_word_debug_win_ = nullptr;
  ScrollView *learn_fragments_debug_win_ = nullptr;
#endif

  // Training data gathered here for all the images in a document.
  std::string tr_file_data_;

  Dict dict_;

  std::vector<uint16_t> shapetable_cutoffs_;

  /* variables used to hold performance statistics */
  int NumAdaptationsFailed = 0;

  // Expected number of features in the class pruner, used to penalize
  // unknowns that have too few features (like a c being classified as e) so
  // it doesn't recognize everything as '@' or '#'.
  // CharNormCutoffs is for the static classifier (with no shapetable).
  // BaselineCutoffs gets a copy of CharNormCutoffs as an estimate of the real
  // value in the adaptive classifier. Both are indexed by unichar_id.
  // shapetable_cutoffs_ provides a similar value for each shape in the
  // shape_table_
  uint16_t CharNormCutoffs[MAX_NUM_CLASSES];
  uint16_t BaselineCutoffs[MAX_NUM_CLASSES];

public:
  bool EnableLearning = true;
};

} // namespace tesseract

#endif // DISABLED_LEGACY_ENGINE

#endif // TESSERACT_CLASSIFY_CLASSIFY_H_
