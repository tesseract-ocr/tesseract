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
#include "fxdefs.h"
#include "intmatcher.h"
#include "ratngs.h"
#include "ocrfeatures.h"
#include "unicity_table.h"

class WERD_CHOICE;
struct ADAPT_RESULTS;
struct NORM_PROTOS;

namespace tesseract {
class Classify : public CCStruct {
 public:
  Classify();
  ~Classify();
  Dict& getDict() {
    return dict_;
  }
  /* adaptive.cpp ************************************************************/
  ADAPT_TEMPLATES NewAdaptedTemplates(bool InitFromUnicharset);
  int ClassPruner(INT_TEMPLATES IntTemplates,
                            inT16 NumFeatures,
                            INT_FEATURE_ARRAY Features,
                            CLASS_NORMALIZATION_ARRAY NormalizationFactors,
                            CLASS_CUTOFF_ARRAY ExpectedNumFeatures,
                            CLASS_PRUNER_RESULTS Results,
                            int Debug);
  void ReadNewCutoffs(FILE *CutoffFile, inT64 end_offset,
                      CLASS_CUTOFF_ARRAY Cutoffs);
  void PrintAdaptedTemplates(FILE *File, ADAPT_TEMPLATES Templates);
  void WriteAdaptedTemplates(FILE *File, ADAPT_TEMPLATES Templates);
  ADAPT_TEMPLATES ReadAdaptedTemplates(FILE *File);
  /* normmatch.cpp ************************************************************/
  FLOAT32 ComputeNormMatch(CLASS_ID ClassId, FEATURE Feature, BOOL8 DebugMatch);
  void FreeNormProtos();
  NORM_PROTOS *ReadNormProtos(FILE *File, inT64 end_offset);
  /* protos.cpp ***************************************************************/
  void ReadClassFile();
  INT_TEMPLATES
      CreateIntTemplates(CLASSES FloatProtos,
                         const UNICHARSET& target_unicharset);
  /* adaptmatch.cpp ***********************************************************/
  void AdaptToWord(TWERD *Word,
                   TEXTROW *Row,
                   const WERD_CHOICE& BestChoice,
                   const WERD_CHOICE& BestRawChoice,
                   const char *rejmap);
  void InitAdaptiveClassifier();
  void InitAdaptedClass(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        CLASS_ID ClassId,
                        ADAPT_CLASS Class,
                        ADAPT_TEMPLATES Templates);
  void AdaptToPunc(TBLOB *Blob,
                   LINE_STATS *LineStats,
                   CLASS_ID ClassId,
                   FLOAT32 Threshold);
  void AmbigClassifier(TBLOB *Blob,
                       LINE_STATS *LineStats,
                       INT_TEMPLATES Templates,
                       UNICHAR_ID *Ambiguities,
                       ADAPT_RESULTS *Results);
  void MasterMatcher(INT_TEMPLATES templates,
                     inT16 num_features,
                     INT_FEATURE_ARRAY features,
                     CLASS_NORMALIZATION_ARRAY norm_factors,
                     ADAPT_CLASS* classes,
                     int debug,
                     int num_classes,
                     CLASS_PRUNER_RESULTS results,
                     ADAPT_RESULTS* final_results);
  void ConvertMatchesToChoices(ADAPT_RESULTS *Results,
                               BLOB_CHOICE_LIST *Choices);
  void AddNewResult(ADAPT_RESULTS *Results,
                    CLASS_ID ClassId,
                    FLOAT32 Rating,
                    int ConfigId);
#ifndef GRAPHICS_DISABLED
  void DebugAdaptiveClassifier(TBLOB *Blob,
                               LINE_STATS *LineStats,
                               ADAPT_RESULTS *Results);
#endif
  void GetAdaptThresholds (TWERD * Word,
                           LINE_STATS * LineStats,
                           const WERD_CHOICE& BestChoice,
                           const WERD_CHOICE& BestRawChoice,
                           FLOAT32 Thresholds[]);

  int MakeNewTemporaryConfig(ADAPT_TEMPLATES Templates,
                             CLASS_ID ClassId,
                             int NumFeatures,
                             INT_FEATURE_ARRAY Features,
                             FEATURE_SET FloatFeatures);
  void MakePermanent(ADAPT_TEMPLATES Templates,
                     CLASS_ID ClassId,
                     int ConfigId,
                     TBLOB *Blob,
                     LINE_STATS *LineStats);
  void PrintAdaptiveMatchResults(FILE *File, ADAPT_RESULTS *Results);
  void RemoveExtraPuncs(ADAPT_RESULTS *Results);
  void RemoveBadMatches(ADAPT_RESULTS *Results);
  void ShowBestMatchFor(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        CLASS_ID ClassId,
                        BOOL8 AdaptiveOn,
                        BOOL8 PreTrainedOn);
  UNICHAR_ID *BaselineClassifier(TBLOB *Blob,
                                 LINE_STATS *LineStats,
                                 ADAPT_TEMPLATES Templates,
                                 ADAPT_RESULTS *Results);
  int CharNormClassifier(TBLOB *Blob,
                         LINE_STATS *LineStats,
                         INT_TEMPLATES Templates,
                         ADAPT_RESULTS *Results);
  UNICHAR_ID *GetAmbiguities(TBLOB *Blob,
                             LINE_STATS *LineStats,
                             CLASS_ID CorrectClass);
  void DoAdaptiveMatch(TBLOB *Blob,
                       LINE_STATS *LineStats,
                       ADAPT_RESULTS *Results);
  void AdaptToChar(TBLOB *Blob,
                   LINE_STATS *LineStats,
                   CLASS_ID ClassId,
                   FLOAT32 Threshold);
  int AdaptableWord(TWERD *Word,
                  const WERD_CHOICE &BestChoiceWord,
                  const WERD_CHOICE &RawChoiceWord);
  void EndAdaptiveClassifier();
  void PrintAdaptiveStatistics(FILE *File);
  void SettupPass1();
  void SettupPass2();
  void AdaptiveClassifier(TBLOB *Blob,
                          TBLOB *DotBlob,
                          TEXTROW *Row,
                          BLOB_CHOICE_LIST *Choices,
                          CLASS_PRUNER_RESULTS cp_results);
  void ClassifyAsNoise(ADAPT_RESULTS *Results);
  void ResetAdaptiveClassifier();

  FLOAT32 GetBestRatingFor(TBLOB *Blob,
                           LINE_STATS *LineStats,
                           CLASS_ID ClassId);
  int GetCharNormFeatures(TBLOB *Blob,
                          LINE_STATS *LineStats,
                          INT_TEMPLATES Templates,
                          INT_FEATURE_ARRAY IntFeatures,
                          CLASS_NORMALIZATION_ARRAY CharNormArray,
                          inT32 *BlobLength);
  int GetIntCharNormFeatures(TBLOB *Blob,
                             LINE_STATS *LineStats,
                             INT_TEMPLATES Templates,
                             INT_FEATURE_ARRAY IntFeatures,
                             CLASS_NORMALIZATION_ARRAY CharNormArray,
                             inT32 *BlobLength);

  /* float2int.cpp ************************************************************/
  void ComputeIntCharNormArray(FEATURE NormFeature,
                               INT_TEMPLATES Templates,
                               CLASS_NORMALIZATION_ARRAY CharNormArray);
  /* intproto.cpp *************************************************************/
  INT_TEMPLATES ReadIntTemplates(FILE *File);
  void WriteIntTemplates(FILE *File, INT_TEMPLATES Templates,
                         const UNICHARSET& target_unicharset);
  CLASS_ID GetClassToDebug(const char *Prompt);
  /* font detection ***********************************************************/
  UnicityTable<FontInfo>& get_fontinfo_table() {
    return fontinfo_table_;
  }
  UnicityTable<FontSet>& get_fontset_table() {
    return fontset_table_;
  }
  /* adaptmatch.cpp ***********************************************************/
  /* name of current image file being processed */
  INT_VAR_H(tessedit_single_match, FALSE, "Top choice only from CP");
  /* use class variables to hold onto built-in templates and adapted
     templates */
  INT_TEMPLATES PreTrainedTemplates;
  ADAPT_TEMPLATES AdaptedTemplates;
  // Successful load of inttemp allows base tesseract classfier to be used.
  bool inttemp_loaded_;

  /* create dummy proto and config masks for use with the built-in templates */
  BIT_VECTOR AllProtosOn;
  BIT_VECTOR PrunedProtos;
  BIT_VECTOR AllConfigsOn;
  BIT_VECTOR AllProtosOff;
  BIT_VECTOR AllConfigsOff;
  BIT_VECTOR TempProtoMask;
  // External control of adaption.
  BOOL_VAR_H(classify_enable_learning, true, "Enable adaptive classifier");
  // Internal control of Adaption so it doesn't work on pass2.
  BOOL_VAR_H(classify_recog_devanagari, false,
             "Whether recognizing a language with devanagari script.");
  bool EnableLearning;
  /* normmatch.cpp */
  NORM_PROTOS *NormProtos;
  /* font detection ***********************************************************/
  UnicityTable<FontInfo> fontinfo_table_;
  UnicityTable<FontSet> fontset_table_;
 private:
  Dict dict_;
};
}  // namespace tesseract

#endif  // TESSERACT_CLASSIFY_CLASSIFY_H__
