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

#ifndef TESSERACT_CLASSIFY_CLASSIFY_H__
#define TESSERACT_CLASSIFY_CLASSIFY_H__

#include "adaptive.h"
#include "ccstruct.h"
#include "classify.h"
#include "dict.h"
#include "featdefs.h"
#include "fontinfo.h"
#include "imagedata.h"
#include "intfx.h"
#include "intmatcher.h"
#include "normalis.h"
#include "ratngs.h"
#include "ocrfeatures.h"
#include "unicity_table.h"

class ScrollView;
class WERD_CHOICE;
class WERD_RES;
struct ADAPT_RESULTS;
struct NORM_PROTOS;

static const int kUnknownFontinfoId = -1;
static const int kBlankFontinfoId = -2;

namespace tesseract {

class ShapeClassifier;
struct ShapeRating;
class ShapeTable;
struct UnicharRating;

// How segmented is a blob. In this enum, character refers to a classifiable
// unit, but that is too long and character is usually easier to understand.
enum CharSegmentationType {
  CST_FRAGMENT,  // A partial character.
  CST_WHOLE,     // A correctly segmented character.
  CST_IMPROPER,  // More than one but less than 2 characters.
  CST_NGRAM      // Multiple characters.
};

class Classify : public CCStruct {
 public:
  Classify();
  virtual ~Classify();
  Dict& getDict() {
    return dict_;
  }

  const ShapeTable* shape_table() const {
    return shape_table_;
  }

  // Takes ownership of the given classifier, and uses it for future calls
  // to CharNormClassifier.
  void SetStaticClassifier(ShapeClassifier* static_classifier);

  // Adds a noise classification result that is a bit worse than the worst
  // current result, or the worst possible result if no current results.
  void AddLargeSpeckleTo(int blob_length, BLOB_CHOICE_LIST *choices);

  // Returns true if the blob is small enough to be a large speckle.
  bool LargeSpeckle(const TBLOB &blob);

  /* adaptive.cpp ************************************************************/
  ADAPT_TEMPLATES NewAdaptedTemplates(bool InitFromUnicharset);
  int GetFontinfoId(ADAPT_CLASS Class, uinT8 ConfigId);
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
  int PruneClasses(const INT_TEMPLATES_STRUCT* int_templates, int num_features,
                   int keep_this, const INT_FEATURE_STRUCT* features,
                   const uinT8* normalization_factors,
                   const uinT16* expected_num_features,
                   GenericVector<CP_RESULT_STRUCT>* results);
  void ReadNewCutoffs(FILE *CutoffFile, bool swap, inT64 end_offset,
                      CLASS_CUTOFF_ARRAY Cutoffs);
  void PrintAdaptedTemplates(FILE *File, ADAPT_TEMPLATES Templates);
  void WriteAdaptedTemplates(FILE *File, ADAPT_TEMPLATES Templates);
  ADAPT_TEMPLATES ReadAdaptedTemplates(FILE *File);
  /* normmatch.cpp ************************************************************/
  FLOAT32 ComputeNormMatch(CLASS_ID ClassId,
                           const FEATURE_STRUCT& feature, BOOL8 DebugMatch);
  void FreeNormProtos();
  NORM_PROTOS *ReadNormProtos(FILE *File, inT64 end_offset);
  /* protos.cpp ***************************************************************/
  void ConvertProto(PROTO Proto, int ProtoId, INT_CLASS Class);
  INT_TEMPLATES CreateIntTemplates(CLASSES FloatProtos,
                                   const UNICHARSET& target_unicharset);
  /* adaptmatch.cpp ***********************************************************/

  // Learns the given word using its chopped_word, seam_array, denorm,
  // box_word, best_state, and correct_text to learn both correctly and
  // incorrectly segmented blobs. If fontname is not NULL, then LearnBlob
  // is called and the data will be saved in an internal buffer.
  // Otherwise AdaptToBlob is called for adaption within a document.
  void LearnWord(const char* fontname, WERD_RES* word);

  // Builds a blob of length fragments, from the word, starting at start,
  // and then learns it, as having the given correct_text.
  // If fontname is not NULL, then LearnBlob is called and the data will be
  // saved in an internal buffer for static training.
  // Otherwise AdaptToBlob is called for adaption within a document.
  // threshold is a magic number required by AdaptToChar and generated by
  // ComputeAdaptionThresholds.
  // Although it can be partly inferred from the string, segmentation is
  // provided to explicitly clarify the character segmentation.
  void LearnPieces(const char* fontname, int start, int length, float threshold,
                   CharSegmentationType segmentation, const char* correct_text,
                   WERD_RES* word);
  void InitAdaptiveClassifier(bool load_pre_trained_templates);
  void InitAdaptedClass(TBLOB *Blob,
                        CLASS_ID ClassId,
                        int FontinfoId,
                        ADAPT_CLASS Class,
                        ADAPT_TEMPLATES Templates);
  void AmbigClassifier(const GenericVector<INT_FEATURE_STRUCT>& int_features,
                       const INT_FX_RESULT_STRUCT& fx_info,
                       const TBLOB *blob,
                       INT_TEMPLATES templates,
                       ADAPT_CLASS *classes,
                       UNICHAR_ID *ambiguities,
                       ADAPT_RESULTS *results);
  void MasterMatcher(INT_TEMPLATES templates,
                     inT16 num_features,
                     const INT_FEATURE_STRUCT* features,
                     const uinT8* norm_factors,
                     ADAPT_CLASS* classes,
                     int debug,
                     int matcher_multiplier,
                     const TBOX& blob_box,
                     const GenericVector<CP_RESULT_STRUCT>& results,
                     ADAPT_RESULTS* final_results);
  // Converts configs to fonts, and if the result is not adapted, and a
  // shape_table_ is present, the shape is expanded to include all
  // unichar_ids represented, before applying a set of corrections to the
  // distance rating in int_result, (see ComputeCorrectedRating.)
  // The results are added to the final_results output.
  void ExpandShapesAndApplyCorrections(ADAPT_CLASS* classes,
                                       bool debug,
                                       int class_id,
                                       int bottom, int top,
                                       float cp_rating,
                                       int blob_length,
                                       int matcher_multiplier,
                                       const uinT8* cn_factors,
                                       UnicharRating* int_result,
                                       ADAPT_RESULTS* final_results);
  // Applies a set of corrections to the distance im_rating,
  // including the cn_correction, miss penalty and additional penalty
  // for non-alnums being vertical misfits. Returns the corrected distance.
  double ComputeCorrectedRating(bool debug, int unichar_id, double cp_rating,
                                double im_rating, int feature_misses,
                                int bottom, int top,
                                int blob_length, int matcher_multiplier,
                                const uinT8* cn_factors);
  void ConvertMatchesToChoices(const DENORM& denorm, const TBOX& box,
                               ADAPT_RESULTS *Results,
                               BLOB_CHOICE_LIST *Choices);
  void AddNewResult(const UnicharRating& new_result, ADAPT_RESULTS *results);
  int GetAdaptiveFeatures(TBLOB *Blob,
                          INT_FEATURE_ARRAY IntFeatures,
                          FEATURE_SET *FloatFeatures);

#ifndef GRAPHICS_DISABLED
  void DebugAdaptiveClassifier(TBLOB *Blob,
                               ADAPT_RESULTS *Results);
#endif
  PROTO_ID MakeNewTempProtos(FEATURE_SET Features,
                             int NumBadFeat,
                             FEATURE_ID BadFeat[],
                             INT_CLASS IClass,
                             ADAPT_CLASS Class,
                             BIT_VECTOR TempProtoMask);
  int MakeNewTemporaryConfig(ADAPT_TEMPLATES Templates,
                             CLASS_ID ClassId,
                             int FontinfoId,
                             int NumFeatures,
                             INT_FEATURE_ARRAY Features,
                             FEATURE_SET FloatFeatures);
  void MakePermanent(ADAPT_TEMPLATES Templates,
                     CLASS_ID ClassId,
                     int ConfigId,
                     TBLOB *Blob);
  void PrintAdaptiveMatchResults(const ADAPT_RESULTS& results);
  void RemoveExtraPuncs(ADAPT_RESULTS *Results);
  void RemoveBadMatches(ADAPT_RESULTS *Results);
  void SetAdaptiveThreshold(FLOAT32 Threshold);
  void ShowBestMatchFor(int shape_id,
                        const INT_FEATURE_STRUCT* features,
                        int num_features);
  // Returns a string for the classifier class_id: either the corresponding
  // unicharset debug_str or the shape_table_ debug str.
  STRING ClassIDToDebugStr(const INT_TEMPLATES_STRUCT* templates,
                           int class_id, int config_id) const;
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
  int ClassAndConfigIDToFontOrShapeID(int class_id,
                                      int int_result_config) const;
  // Converts a shape_table_ index to a classifier class_id index (not a
  // unichar-id!). Uses a search, so not fast.
  int ShapeIDToClassID(int shape_id) const;
  UNICHAR_ID *BaselineClassifier(
      TBLOB *Blob, const GenericVector<INT_FEATURE_STRUCT>& int_features,
      const INT_FX_RESULT_STRUCT& fx_info,
      ADAPT_TEMPLATES Templates, ADAPT_RESULTS *Results);
  int CharNormClassifier(TBLOB *blob,
                         const TrainingSample& sample,
                         ADAPT_RESULTS *adapt_results);

  // As CharNormClassifier, but operates on a TrainingSample and outputs to
  // a GenericVector of ShapeRating without conversion to classes.
  int CharNormTrainingSample(bool pruner_only, int keep_this,
                             const TrainingSample& sample,
                             GenericVector<UnicharRating>* results);
  UNICHAR_ID *GetAmbiguities(TBLOB *Blob, CLASS_ID CorrectClass);
  void DoAdaptiveMatch(TBLOB *Blob, ADAPT_RESULTS *Results);
  void AdaptToChar(TBLOB* Blob, CLASS_ID ClassId, int FontinfoId,
                   FLOAT32 Threshold, ADAPT_TEMPLATES adaptive_templates);
  void DisplayAdaptedChar(TBLOB* blob, INT_CLASS_STRUCT* int_class);
  bool AdaptableWord(WERD_RES* word);
  void EndAdaptiveClassifier();
  void SettupPass1();
  void SettupPass2();
  void AdaptiveClassifier(TBLOB *Blob, BLOB_CHOICE_LIST *Choices);
  void ClassifyAsNoise(ADAPT_RESULTS *Results);
  void ResetAdaptiveClassifierInternal();
  void SwitchAdaptiveClassifier();
  void StartBackupAdaptiveClassifier();

  int GetCharNormFeature(const INT_FX_RESULT_STRUCT& fx_info,
                         INT_TEMPLATES templates,
                         uinT8* pruner_norm_array,
                         uinT8* char_norm_array);
  // Computes the char_norm_array for the unicharset and, if not NULL, the
  // pruner_array as appropriate according to the existence of the shape_table.
  // The norm_feature is deleted as it is almost certainly no longer needed.
  void ComputeCharNormArrays(FEATURE_STRUCT* norm_feature,
                             INT_TEMPLATES_STRUCT* templates,
                             uinT8* char_norm_array,
                             uinT8* pruner_array);

  bool TempConfigReliable(CLASS_ID class_id, const TEMP_CONFIG &config);
  void UpdateAmbigsGroup(CLASS_ID class_id, TBLOB *Blob);

  bool AdaptiveClassifierIsFull() const { return NumAdaptationsFailed > 0; }
  bool AdaptiveClassifierIsEmpty() const {
    return AdaptedTemplates->NumPermClasses == 0;
  }
  bool LooksLikeGarbage(TBLOB *blob);
  void RefreshDebugWindow(ScrollView **win, const char *msg,
                          int y_offset, const TBOX &wbox);
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
  static void SetupBLCNDenorms(const TBLOB& blob, bool nonlinear_norm,
                               DENORM* bl_denorm, DENORM* cn_denorm,
                               INT_FX_RESULT_STRUCT* fx_info);

  // Extracts sets of 3-D features of length kStandardFeatureLength (=12.8), as
  // (x,y) position and angle as measured counterclockwise from the vector
  // <-1, 0>, from blob using two normalizations defined by bl_denorm and
  // cn_denorm. See SetpuBLCNDenorms for definitions.
  // If outline_cn_counts is not NULL, on return it contains the cumulative
  // number of cn features generated for each outline in the blob (in order).
  // Thus after the first outline, there were (*outline_cn_counts)[0] features,
  // after the second outline, there were (*outline_cn_counts)[1] features etc.
  static void ExtractFeatures(const TBLOB& blob,
                              bool nonlinear_norm,
                              GenericVector<INT_FEATURE_STRUCT>* bl_features,
                              GenericVector<INT_FEATURE_STRUCT>* cn_features,
                              INT_FX_RESULT_STRUCT* results,
                              GenericVector<int>* outline_cn_counts);
  /* float2int.cpp ************************************************************/
  void ClearCharNormArray(uinT8* char_norm_array);
  void ComputeIntCharNormArray(const FEATURE_STRUCT& norm_feature,
                               uinT8* char_norm_array);
  void ComputeIntFeatures(FEATURE_SET Features, INT_FEATURE_ARRAY IntFeatures);
  /* intproto.cpp *************************************************************/
  INT_TEMPLATES ReadIntTemplates(FILE *File);
  void WriteIntTemplates(FILE *File, INT_TEMPLATES Templates,
                         const UNICHARSET& target_unicharset);
  CLASS_ID GetClassToDebug(const char *Prompt, bool* adaptive_on,
                           bool* pretrained_on, int* shape_id);
  void ShowMatchDisplay();
  /* font detection ***********************************************************/
  UnicityTable<FontInfo>& get_fontinfo_table() {
    return fontinfo_table_;
  }
  const UnicityTable<FontInfo>& get_fontinfo_table() const {
    return fontinfo_table_;
  }
  UnicityTable<FontSet>& get_fontset_table() {
    return fontset_table_;
  }
  /* mfoutline.cpp ***********************************************************/
  void NormalizeOutlines(LIST Outlines, FLOAT32 *XScale, FLOAT32 *YScale);
  /* outfeat.cpp ***********************************************************/
  FEATURE_SET ExtractOutlineFeatures(TBLOB *Blob);
  /* picofeat.cpp ***********************************************************/
  FEATURE_SET ExtractPicoFeatures(TBLOB *Blob);
  FEATURE_SET ExtractIntCNFeatures(const TBLOB& blob,
                                   const INT_FX_RESULT_STRUCT& fx_info);
  FEATURE_SET ExtractIntGeoFeatures(const TBLOB& blob,
                                    const INT_FX_RESULT_STRUCT& fx_info);
  /* blobclass.cpp ***********************************************************/
  // Extracts features from the given blob and saves them in the tr_file_data_
  // member variable.
  // fontname:  Name of font that this blob was printed in.
  // cn_denorm: Character normalization transformation to apply to the blob.
  // fx_info:   Character normalization parameters computed with cn_denorm.
  // blob_text: Ground truth text for the blob.
  void LearnBlob(const STRING& fontname, TBLOB* Blob, const DENORM& cn_denorm,
                 const INT_FX_RESULT_STRUCT& fx_info, const char* blob_text);
  // Writes stored training data to a .tr file based on the given filename.
  // Returns false on error.
  bool WriteTRFile(const STRING& filename);

  // Member variables.

  // Parameters.
  // Set during training (in lang.config) to indicate whether the divisible
  // blobs chopper should be used (true for latin script.)
  BOOL_VAR_H(allow_blob_division, true, "Use divisible blobs chopping");
  // Set during training (in lang.config) to indicate whether the divisible
  // blobs chopper should be used in preference to chopping. Set to true for
  // southern Indic scripts.
  BOOL_VAR_H(prioritize_division, FALSE,
             "Prioritize blob division over chopping");
  INT_VAR_H(tessedit_single_match, FALSE, "Top choice only from CP");
  BOOL_VAR_H(classify_enable_learning, true, "Enable adaptive classifier");
  INT_VAR_H(classify_debug_level, 0, "Classify debug level");

  /* mfoutline.cpp ***********************************************************/
  /* control knobs used to control normalization of outlines */
  INT_VAR_H(classify_norm_method, character, "Normalization Method   ...");
  double_VAR_H(classify_char_norm_range, 0.2,
             "Character Normalization Range ...");
  double_VAR_H(classify_min_norm_scale_x, 0.0, "Min char x-norm scale ...");
  double_VAR_H(classify_max_norm_scale_x, 0.325, "Max char x-norm scale ...");
  double_VAR_H(classify_min_norm_scale_y, 0.0, "Min char y-norm scale ...");
  double_VAR_H(classify_max_norm_scale_y, 0.325, "Max char y-norm scale ...");
  double_VAR_H(classify_max_rating_ratio, 1.5,
               "Veto ratio between classifier ratings");
  double_VAR_H(classify_max_certainty_margin, 5.5,
               "Veto difference between classifier certainties");

  /* adaptmatch.cpp ***********************************************************/
  BOOL_VAR_H(tess_cn_matching, 0, "Character Normalized Matching");
  BOOL_VAR_H(tess_bn_matching, 0, "Baseline Normalized Matching");
  BOOL_VAR_H(classify_enable_adaptive_matcher, 1, "Enable adaptive classifier");
  BOOL_VAR_H(classify_use_pre_adapted_templates, 0,
             "Use pre-adapted classifier templates");
  BOOL_VAR_H(classify_save_adapted_templates, 0,
             "Save adapted templates to a file");
  BOOL_VAR_H(classify_enable_adaptive_debugger, 0, "Enable match debugger");
  BOOL_VAR_H(classify_nonlinear_norm, 0,
             "Non-linear stroke-density normalization");
  INT_VAR_H(matcher_debug_level, 0, "Matcher Debug Level");
  INT_VAR_H(matcher_debug_flags, 0, "Matcher Debug Flags");
  INT_VAR_H(classify_learning_debug_level, 0, "Learning Debug Level: ");
  double_VAR_H(matcher_good_threshold, 0.125, "Good Match (0-1)");
  double_VAR_H(matcher_reliable_adaptive_result, 0.0, "Great Match (0-1)");
  double_VAR_H(matcher_perfect_threshold, 0.02, "Perfect Match (0-1)");
  double_VAR_H(matcher_bad_match_pad, 0.15, "Bad Match Pad (0-1)");
  double_VAR_H(matcher_rating_margin, 0.1, "New template margin (0-1)");
  double_VAR_H(matcher_avg_noise_size, 12.0, "Avg. noise blob length: ");
  INT_VAR_H(matcher_permanent_classes_min, 1, "Min # of permanent classes");
  INT_VAR_H(matcher_min_examples_for_prototyping, 3,
            "Reliable Config Threshold");
  INT_VAR_H(matcher_sufficient_examples_for_prototyping, 5,
            "Enable adaption even if the ambiguities have not been seen");
  double_VAR_H(matcher_clustering_max_angle_delta, 0.015,
               "Maximum angle delta for prototype clustering");
  double_VAR_H(classify_misfit_junk_penalty, 0.0,
               "Penalty to apply when a non-alnum is vertically out of "
               "its expected textline position");
  double_VAR_H(rating_scale, 1.5, "Rating scaling factor");
  double_VAR_H(certainty_scale, 20.0, "Certainty scaling factor");
  double_VAR_H(tessedit_class_miss_scale, 0.00390625,
               "Scale factor for features not used");
  double_VAR_H(classify_adapted_pruning_factor, 2.5,
               "Prune poor adapted results this much worse than best result");
  double_VAR_H(classify_adapted_pruning_threshold, -1.0,
               "Threshold at which classify_adapted_pruning_factor starts");
  INT_VAR_H(classify_adapt_proto_threshold, 230,
            "Threshold for good protos during adaptive 0-255");
  INT_VAR_H(classify_adapt_feature_threshold, 230,
            "Threshold for good features during adaptive 0-255");
  BOOL_VAR_H(disable_character_fragments, TRUE,
             "Do not include character fragments in the"
             " results of the classifier");
  double_VAR_H(classify_character_fragments_garbage_certainty_threshold, -3.0,
               "Exclude fragments that do not match any whole character"
               " with at least this certainty");
  BOOL_VAR_H(classify_debug_character_fragments, FALSE,
             "Bring up graphical debugging windows for fragments training");
  BOOL_VAR_H(matcher_debug_separate_windows, FALSE,
             "Use two different windows for debugging the matching: "
             "One for the protos and one for the features.");
  STRING_VAR_H(classify_learn_debug_str, "", "Class str to debug learning");

  /* intmatcher.cpp **********************************************************/
  INT_VAR_H(classify_class_pruner_threshold, 229,
            "Class Pruner Threshold 0-255");
  INT_VAR_H(classify_class_pruner_multiplier, 15,
            "Class Pruner Multiplier 0-255:       ");
  INT_VAR_H(classify_cp_cutoff_strength, 7,
            "Class Pruner CutoffStrength:         ");
  INT_VAR_H(classify_integer_matcher_multiplier, 10,
            "Integer Matcher Multiplier  0-255:   ");

  // Use class variables to hold onto built-in templates and adapted templates.
  INT_TEMPLATES PreTrainedTemplates;
  ADAPT_TEMPLATES AdaptedTemplates;
  // The backup adapted templates are created from the previous page (only)
  // so they are always ready and reasonably well trained if the primary
  // adapted templates become full.
  ADAPT_TEMPLATES BackupAdaptedTemplates;

  // Create dummy proto and config masks for use with the built-in templates.
  BIT_VECTOR AllProtosOn;
  BIT_VECTOR AllConfigsOn;
  BIT_VECTOR AllConfigsOff;
  BIT_VECTOR TempProtoMask;
  bool EnableLearning;
  /* normmatch.cpp */
  NORM_PROTOS *NormProtos;
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

  INT_VAR_H(il1_adaption_test, 0, "Don't adapt to i/I at beginning of word");
  BOOL_VAR_H(classify_bln_numeric_mode, 0,
             "Assume the input is numbers [0-9].");
  double_VAR_H(speckle_large_max_size, 0.30, "Max large speckle size");
  double_VAR_H(speckle_rating_penalty, 10.0,
               "Penalty to add to worst rating for noise");

 protected:
  IntegerMatcher im_;
  FEATURE_DEFS_STRUCT feature_defs_;
  // If a shape_table_ is present, it is used to remap classifier output in
  // ExpandShapesAndApplyCorrections. font_ids referenced by configs actually
  // mean an index to the shape_table_ and the choices returned are *all* the
  // shape_table_ entries at that index.
  ShapeTable* shape_table_;

 private:
  Dict dict_;
  // The currently active static classifier.
  ShapeClassifier* static_classifier_;

  /* variables used to hold performance statistics */
  int NumAdaptationsFailed;

  // Training data gathered here for all the images in a document.
  STRING tr_file_data_;

  // Expected number of features in the class pruner, used to penalize
  // unknowns that have too few features (like a c being classified as e) so
  // it doesn't recognize everything as '@' or '#'.
  // CharNormCutoffs is for the static classifier (with no shapetable).
  // BaselineCutoffs gets a copy of CharNormCutoffs as an estimate of the real
  // value in the adaptive classifier. Both are indexed by unichar_id.
  // shapetable_cutoffs_ provides a similar value for each shape in the
  // shape_table_
  uinT16* CharNormCutoffs;
  uinT16* BaselineCutoffs;
  GenericVector<uinT16> shapetable_cutoffs_;
  ScrollView* learn_debug_win_;
  ScrollView* learn_fragmented_word_debug_win_;
  ScrollView* learn_fragments_debug_win_;
};
}  // namespace tesseract

#endif  // TESSERACT_CLASSIFY_CLASSIFY_H__
