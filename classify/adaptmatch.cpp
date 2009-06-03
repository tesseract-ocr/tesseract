/******************************************************************************
 **                         Filename:    adaptmatch.c
 **                         Purpose:     High level adaptive matcher.
 **                         Author:      Dan Johnson
 **                         History:     Mon Mar 11 10:00:10 1991, DSJ, Created.
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include <ctype.h>
#include "adaptmatch.h"
#include "normfeat.h"
#include "mfoutline.h"
#include "picofeat.h"
#include "float2int.h"
#include "outfeat.h"
#include "emalloc.h"
#include "intfx.h"
#include "permnum.h"
#include "speckle.h"
#include "efio.h"
#include "normmatch.h"
#include "stopper.h"
#include "permute.h"
#include "context.h"
#include "ndminx.h"
#include "intproto.h"
#include "const.h"
#include "globals.h"
#include "werd.h"
#include "callcpp.h"
#include "tordvars.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#ifdef __UNIX__
#include <assert.h>
#endif

#define ADAPT_TEMPLATE_SUFFIX ".a"
#define BUILT_IN_TEMPLATES_FILE "inttemp"
#define BUILT_IN_CUTOFFS_FILE "pffmtable"

#define MAX_MATCHES         10
#define UNLIKELY_NUM_FEAT 200
#define NO_DEBUG      0
#define MAX_ADAPTABLE_WERD_SIZE 40

#define ADAPTABLE_WERD    (GOOD_NUMBER + 0.05)

#define Y_DIM_OFFSET    (Y_SHIFT - BASELINE_Y_SHIFT)

#define WORST_POSSIBLE_RATING (1.0)

typedef struct
{
  inT32 BlobLength;
  int NumMatches;
  CLASS_ID Classes[MAX_NUM_CLASSES];
  FLOAT32 Ratings[MAX_CLASS_ID + 1];
  uinT8 Configs[MAX_CLASS_ID + 1];
  FLOAT32 BestRating;
  CLASS_ID BestClass;
  uinT8 BestConfig;
  CLASS_PRUNER_RESULTS CPResults;
}


ADAPT_RESULTS;

typedef struct
{
  ADAPT_TEMPLATES Templates;
  CLASS_ID ClassId;
  int ConfigId;
}


PROTO_KEY;

/**----------------------------------------------------------------------------
          Private Macros
----------------------------------------------------------------------------**/
#define MarginalMatch(Rating)       \
((Rating) > GreatAdaptiveMatch)

#define TempConfigReliable(Config)  \
((Config)->NumTimesSeen >= ReliableConfigThreshold)

#define InitIntFX() (FeaturesHaveBeenExtracted = FALSE)

/**----------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------**/
void AdaptToChar(TBLOB *Blob,
                 LINE_STATS *LineStats,
                 CLASS_ID ClassId,
                 FLOAT32 Threshold);

void AdaptToPunc(TBLOB *Blob,
                 LINE_STATS *LineStats,
                 CLASS_ID ClassId,
                 FLOAT32 Threshold);

void AddNewResult(ADAPT_RESULTS *Results,
                  CLASS_ID ClassId,
                  FLOAT32 Rating,
                  int ConfigId);

void AmbigClassifier(TBLOB *Blob,
                     LINE_STATS *LineStats,
                     INT_TEMPLATES Templates,
                     UNICHAR_ID *Ambiguities,
                     ADAPT_RESULTS *Results);

UNICHAR_ID *BaselineClassifier(TBLOB *Blob,
                               LINE_STATS *LineStats,
                               ADAPT_TEMPLATES Templates,
                               ADAPT_RESULTS *Results);

void make_config_pruner(INT_TEMPLATES templates, CONFIG_PRUNER *config_pruner);

void CharNormClassifier(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        INT_TEMPLATES Templates,
                        ADAPT_RESULTS *Results);

void ClassifyAsNoise(TBLOB *Blob,
                     LINE_STATS *LineStats,
                     ADAPT_RESULTS *Results);

int CompareCurrentRatings(const void *arg1,
                          const void *arg2);

LIST ConvertMatchesToChoices(ADAPT_RESULTS *Results);

void DebugAdaptiveClassifier(TBLOB *Blob,
                             LINE_STATS *LineStats,
                             ADAPT_RESULTS *Results);

void DoAdaptiveMatch(TBLOB *Blob,
                     LINE_STATS *LineStats,
                     ADAPT_RESULTS *Results);

void GetAdaptThresholds(TWERD * Word,
LINE_STATS * LineStats,
const WERD_CHOICE& BestChoice,
const WERD_CHOICE& BestRawChoice, FLOAT32 Thresholds[]);

UNICHAR_ID *GetAmbiguities(TBLOB *Blob,
                           LINE_STATS *LineStats,
                           CLASS_ID CorrectClass);

int GetBaselineFeatures(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        INT_TEMPLATES Templates,
                        INT_FEATURE_ARRAY IntFeatures,
                        CLASS_NORMALIZATION_ARRAY CharNormArray,
                        inT32 *BlobLength);

FLOAT32 GetBestRatingFor(TBLOB *Blob, LINE_STATS *LineStats, CLASS_ID ClassId);

int GetCharNormFeatures(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        INT_TEMPLATES Templates,
                        INT_FEATURE_ARRAY IntFeatures,
                        CLASS_NORMALIZATION_ARRAY CharNormArray,
                        inT32 *BlobLength);

int GetIntBaselineFeatures(TBLOB *Blob,
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

void InitMatcherRatings(register FLOAT32 *Rating);

int MakeNewTemporaryConfig(ADAPT_TEMPLATES Templates,
                           CLASS_ID ClassId,
                           int NumFeatures,
                           INT_FEATURE_ARRAY Features,
                           FEATURE_SET FloatFeatures);

PROTO_ID MakeNewTempProtos(FEATURE_SET Features,
int NumBadFeat,
FEATURE_ID BadFeat[],
INT_CLASS IClass,
ADAPT_CLASS Class, BIT_VECTOR TempProtoMask);

void MakePermanent(ADAPT_TEMPLATES Templates,
                   CLASS_ID ClassId,
                   int ConfigId,
                   TBLOB *Blob,
                   LINE_STATS *LineStats);

int MakeTempProtoPerm(void *item1, void *item2);

int NumBlobsIn(TWERD *Word);

int NumOutlinesInBlob(TBLOB *Blob);

void PrintAdaptiveMatchResults(FILE *File, ADAPT_RESULTS *Results);

void RemoveBadMatches(ADAPT_RESULTS *Results);

void RemoveExtraPuncs(ADAPT_RESULTS *Results);

void SetAdaptiveThreshold(FLOAT32 Threshold);
void ShowBestMatchFor(TBLOB *Blob,
                      LINE_STATS *LineStats,
                      CLASS_ID ClassId,
                      BOOL8 AdaptiveOn,
                      BOOL8 PreTrainedOn);


/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
/* name of current image file being processed */
extern char imagefile[];
INT_VAR(tessedit_single_match, FALSE, "Top choice only from CP");

/* variables used to hold performance statistics */
static int AdaptiveMatcherCalls = 0;
static int BaselineClassifierCalls = 0;
static int CharNormClassifierCalls = 0;
static int AmbigClassifierCalls = 0;
static int NumWordsAdaptedTo = 0;
static int NumCharsAdaptedTo = 0;
static int NumBaselineClassesTried = 0;
static int NumCharNormClassesTried = 0;
static int NumAmbigClassesTried = 0;
static int NumClassesOutput = 0;
static int NumAdaptationsFailed = 0;

/* define globals used to hold onto extracted features.  This is used
to map from the old scheme in which baseline features and char norm
features are extracted separately, to the new scheme in which they
are extracted at the same time. */
static BOOL8 FeaturesHaveBeenExtracted = FALSE;
static BOOL8 FeaturesOK = TRUE;
static INT_FEATURE_ARRAY BaselineFeatures;
static INT_FEATURE_ARRAY CharNormFeatures;
static INT_FX_RESULT_STRUCT FXInfo;

/* use a global variable to hold onto the current ratings so that the
comparison function passes to qsort can get at them */
static FLOAT32 *CurrentRatings;

/* define globals to hold filenames of training data */
static const char *BuiltInTemplatesFile = BUILT_IN_TEMPLATES_FILE;
static const char *BuiltInCutoffsFile = BUILT_IN_CUTOFFS_FILE;
static CLASS_CUTOFF_ARRAY CharNormCutoffs;
static CLASS_CUTOFF_ARRAY BaselineCutoffs;

/* use global variables to hold onto built-in templates and adapted
templates */
static INT_TEMPLATES PreTrainedTemplates;
static ADAPT_TEMPLATES AdaptedTemplates;

/* create dummy proto and config masks for use with the built-in templates */
static BIT_VECTOR AllProtosOn;
static BIT_VECTOR PrunedProtos;
static BIT_VECTOR AllConfigsOn;
static BIT_VECTOR AllProtosOff;
static BIT_VECTOR AllConfigsOff;
static BIT_VECTOR TempProtoMask;

/* define control knobs for adaptive matcher */
make_toggle_const(EnableAdaptiveMatcher, 1, MakeEnableAdaptiveMatcher);
/* PREV DEFAULT 0 */

make_toggle_const(UsePreAdaptedTemplates, 0, MakeUsePreAdaptedTemplates);
make_toggle_const(SaveAdaptedTemplates, 0, MakeSaveAdaptedTemplates);

make_toggle_var(EnableAdaptiveDebugger, 0, MakeEnableAdaptiveDebugger,
18, 1, SetEnableAdaptiveDebugger, "Enable match debugger");

make_int_var(MatcherDebugLevel, 0, MakeMatcherDebugLevel,
18, 2, SetMatcherDebugLevel, "Matcher Debug Level: ");

make_int_var(MatchDebugFlags, 0, MakeMatchDebugFlags,
18, 3, SetMatchDebugFlags, "Matcher Debug Flags: ");

make_toggle_var(EnableLearning, 1, MakeEnableLearning,
18, 4, SetEnableLearning, "Enable learning");
/* PREV DEFAULT 0 */
                                 /*record it for multiple pages */
static int old_enable_learning = 1;

make_int_var(LearningDebugLevel, 0, MakeLearningDebugLevel,
18, 5, SetLearningDebugLevel, "Learning Debug Level: ");

make_float_var(GoodAdaptiveMatch, 0.125, MakeGoodAdaptiveMatch,
18, 6, SetGoodAdaptiveMatch, "Good Match    (0-1): ");

make_float_var(GreatAdaptiveMatch, 0.0, MakeGreatAdaptiveMatch,
18, 7, SetGreatAdaptiveMatch, "Great Match   (0-1): ");
/* PREV DEFAULT 0.10 */

make_float_var(PerfectRating, 0.02, MakePerfectRating,
18, 8, SetPerfectRating, "Perfect Match (0-1): ");

make_float_var(BadMatchPad, 0.15, MakeBadMatchPad,
18, 9, SetBadMatchPad, "Bad Match Pad (0-1): ");

make_float_var(RatingMargin, 0.1, MakeRatingMargin,
18, 10, SetRatingMargin, "New template margin (0-1): ");

make_float_var(NoiseBlobLength, 12.0, MakeNoiseBlobLength,
18, 11, SetNoiseBlobLength, "Avg. noise blob length: ");

make_int_var(MinNumPermClasses, 1, MakeMinNumPermClasses,
18, 12, SetMinNumPermClasses, "Min # of permanent classes: ");
/* PREV DEFAULT 200 */

make_int_var(ReliableConfigThreshold, 2, MakeReliableConfigThreshold,
18, 13, SetReliableConfigThreshold,
"Reliable Config Threshold: ");

make_float_var(MaxAngleDelta, 0.015, MakeMaxAngleDelta,
18, 14, SetMaxAngleDelta,
"Maximum angle delta for proto clustering: ");

make_toggle_var(EnableIntFX, 1, MakeEnableIntFX,
18, 15, SetEnableIntFX, "Enable integer fx");
/* PREV DEFAULT 0 */

make_toggle_var(EnableNewAdaptRules, 1, MakeEnableNewAdaptRules,
18, 16, SetEnableNewAdaptRules,
"Enable new adaptation rules");
/* PREV DEFAULT 0 */

make_float_var(RatingScale, 1.5, MakeRatingScale,
18, 17, SetRatingScale, "Rating scale: ");

make_float_var(CertaintyScale, 20.0, MakeCertaintyScale,
18, 18, SetCertaintyScale, "CertaintyScale: ");

make_int_var(FailedAdaptionsBeforeReset, 150, MakeFailedAdaptionsBeforeReset,
18, 19, SetFailedAdaptionsBeforeReset,
"Number of failed adaptions before adapted templates reset: ");
double_VAR(tessedit_class_miss_scale, 0.00390625,
           "Scale factor for features not used");

int tess_cn_matching = 0;
int tess_bn_matching = 0;

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
LIST AdaptiveClassifier(TBLOB *Blob, TBLOB *DotBlob, TEXTROW *Row) {
/*
 **                         Parameters:
 **              Blob            blob to be classified
 **              DotBlob         (obsolete)
 **              Row             row of text that word appears in
 **                         Globals:
 **                         CurrentRatings
              used by compare function for qsort
**                          Operation: This routine calls the adaptive matcher which returns
**      (in an array) the class id of each class matched.  It also
**                          returns the number of classes matched.
**                          For each class matched it places the best rating
**                          found for that class into the Ratings array.
**                          Bad matches are then removed so that they don't need to be
**                          sorted.  The remaining good matches are then sorted and
**                          converted to choices.
**                          This routine also performs some simple speckle filtering.
**                          Return: List of choices found by adaptive matcher.
**                          Exceptions: none
**                          History: Mon Mar 11 10:00:58 1991, DSJ, Created.
*/
  LIST Choices;
  ADAPT_RESULTS* Results = new ADAPT_RESULTS;
  LINE_STATS LineStats;

  if (FailedAdaptionsBeforeReset >= 0 &&
      NumAdaptationsFailed >= FailedAdaptionsBeforeReset) {
    NumAdaptationsFailed = 0;
    ResetAdaptiveClassifier();
  }
  if (AdaptedTemplates == NULL)
    AdaptedTemplates = NewAdaptedTemplates ();
  EnterClassifyMode;

  Results->BlobLength = MAX_INT32;
  Results->NumMatches = 0;
  Results->BestRating = WORST_POSSIBLE_RATING;
  Results->BestClass = NO_CLASS;
  Results->BestConfig = 0;
  GetLineStatsFromRow(Row, &LineStats);
  InitMatcherRatings (Results->Ratings);

  DoAdaptiveMatch(Blob, &LineStats, Results);
  RemoveBadMatches(Results);

  /* save ratings in a global so that CompareCurrentRatings() can see them */
  CurrentRatings = Results->Ratings;
  qsort((void*) (Results->Classes), Results->NumMatches,
    sizeof (CLASS_ID), CompareCurrentRatings);
  RemoveExtraPuncs(Results);
  Choices = ConvertMatchesToChoices(Results);

  if (MatcherDebugLevel >= 1) {
    cprintf ("AD Matches =  ");
    PrintAdaptiveMatchResults(stdout, Results);
  }

  if (LargeSpeckle (Blob, Row))
    Choices = AddLargeSpeckleTo (Choices);

#ifndef GRAPHICS_DISABLED
  if (EnableAdaptiveDebugger)
    DebugAdaptiveClassifier(Blob, &LineStats, Results);
#endif

  NumClassesOutput += count (Choices);
  if (Choices == NIL) {
    char empty_lengths[] = {0};
    if (!bln_numericmode)
      tprintf ("Nil classification!\n");  // Should never normally happen.
    return (append_choice (NIL, "", empty_lengths, 50.0f, -20.0f, -1));
  }

  delete Results;
  return Choices;
}                                /* AdaptiveClassifier */


/*---------------------------------------------------------------------------*/
void AdaptToWord(TWERD *Word,
                 TEXTROW *Row,
                 const WERD_CHOICE& BestChoice,
                 const WERD_CHOICE& BestRawChoice,
                 const char *rejmap) {
/*
 **                         Parameters:
 **                         Word
              word to be adapted to
**                          Row
              row of text that word is found in
**                          BestChoice
              best choice for word found by system
**                          BestRawChoice
              best choice for word found by classifier only
**                          Globals:
**                          EnableLearning
              TRUE if learning is enabled
**                          Operation: This routine implements a preliminary version of the
**                          rules which are used to decide which characters to adapt to.
**                          A word is adapted to if it is in the dictionary or if it
**                          is a "good" number (no trailing units, etc.).  It cannot
**                          contain broken or merged characters.  Within that word, only
**                          letters and digits are adapted to (no punctuation).
**                          Return: none
**                          Exceptions: none
**                          History: Thu Mar 14 07:40:36 1991, DSJ, Created.
*/
  TBLOB *Blob;
  LINE_STATS LineStats;
  FLOAT32 Thresholds[MAX_ADAPTABLE_WERD_SIZE];
  FLOAT32 *Threshold;
  const char *map = rejmap;
  char map_char = '1';
  const char* BestChoice_string = BestChoice.string().string();
  const char* BestChoice_lengths = BestChoice.lengths().string();

  if (strlen(BestChoice_lengths) > MAX_ADAPTABLE_WERD_SIZE)
    return;

  if (EnableLearning) {
    NumWordsAdaptedTo++;

    #ifndef SECURE_NAMES
    if (LearningDebugLevel >= 1)
      cprintf ("\n\nAdapting to word = %s\n", BestChoice.string().string());
    #endif
    GetLineStatsFromRow(Row, &LineStats);

    GetAdaptThresholds(Word,
                       &LineStats,
                       BestChoice,
                       BestRawChoice,
                       Thresholds);

    for (Blob = Word->blobs, Threshold = Thresholds; Blob != NULL;
         Blob = Blob->next, BestChoice_string += *(BestChoice_lengths++),
             Threshold++) {
      InitIntFX();

      if (rejmap != NULL)
        map_char = *map++;

      assert (map_char == '1' || map_char == '0');

      if (map_char == '1') {

//        if (unicharset.get_isalpha (BestChoice_string, *BestChoice_lengths) ||
//            unicharset.get_isdigit (BestChoice_string, *BestChoice_lengths)) {
          /* SPECIAL RULE:  don't adapt to an 'i' which is the first char
             in a word because they are too ambiguous with 'I'.
             The new adaptation rules should account for this
             automatically, since they exclude ambiguous words from
             adaptation, but for safety's sake we'll leave the rule in.
             Also, don't adapt to i's that have only 1 blob in them
             because this creates too much ambiguity for broken
             characters. */
          if (*BestChoice_lengths == 1 &&
              (*BestChoice_string == 'i'
               || (il1_adaption_test && *BestChoice_string == 'I' &&
               (Blob->next == NULL ||
               unicharset.get_islower (BestChoice_string + *BestChoice_lengths,
                                       *(BestChoice_lengths + 1)))))
              && (Blob == Word->blobs
                  || (!(unicharset.get_isalpha (BestChoice_string -
                                                *(BestChoice_lengths - 1),
                                                *(BestChoice_lengths - 1)) ||
                        unicharset.get_isdigit (BestChoice_string -
                                                *(BestChoice_lengths - 1),
                                                *(BestChoice_lengths - 1))))

                  || (!il1_adaption_test && NumOutlinesInBlob(Blob) != 2))) {
            if (LearningDebugLevel >= 1)
              cprintf ("Rejecting char = %s\n", unicharset.id_to_unichar(
                           unicharset.unichar_to_id(BestChoice_string,
                                                    *BestChoice_lengths)));
          }
          else {
            #ifndef SECURE_NAMES
            if (LearningDebugLevel >= 1)
              cprintf ("Adapting to char = %s, thr= %g\n",
                       unicharset.id_to_unichar(
                           unicharset.unichar_to_id(BestChoice_string,
                                                    *BestChoice_lengths)),
                           *Threshold);
            #endif
            AdaptToChar(Blob, &LineStats,
                        unicharset.unichar_to_id(BestChoice_string,
                                                 *BestChoice_lengths),
                        *Threshold);
          }
//        }
//        else
//          AdaptToPunc(Blob, &LineStats,
//                      unicharset.unichar_to_id(BestChoice_string,
//                                               *BestChoice_lengths),
//                      *Threshold);
      }
    }
    if (LearningDebugLevel >= 1)
      cprintf ("\n");
  }
}                                /* AdaptToWord */


/*---------------------------------------------------------------------------*/
void EndAdaptiveClassifier() {
/*
 **                         Parameters: none
 **                         Globals:
 **                         AdaptedTemplates
              current set of adapted templates
**                          SaveAdaptedTemplates
              TRUE if templates should be saved
**                          EnableAdaptiveMatcher
              TRUE if adaptive matcher is enabled
**                          Operation: This routine performs cleanup operations on the
**                          adaptive classifier.  It should be called before the
**                          program is terminated.  Its main function is to save
**                          the adapted templates to a file.
**                          Return: none
**                          Exceptions: none
**                          History: Tue Mar 19 14:37:06 1991, DSJ, Created.
*/
  char Filename[256];
  FILE *File;

  #ifndef SECURE_NAMES
  if (EnableAdaptiveMatcher && SaveAdaptedTemplates) {
    strcpy(Filename, imagefile);
    strcat(Filename, ADAPT_TEMPLATE_SUFFIX);
    File = fopen (Filename, "wb");
    if (File == NULL)
      cprintf ("Unable to save adapted templates to %s!\n", Filename);
    else {
      cprintf ("\nSaving adapted templates to %s ...", Filename);
      fflush(stdout);
      WriteAdaptedTemplates(File, AdaptedTemplates);
      cprintf ("\n");
      fclose(File);
    }
  }
  #endif
  if (PreTrainedTemplates == NULL)
    return;  // This function isn't safe to run twice.
  EndDangerousAmbigs();
  FreeNormProtos();
  free_int_templates(PreTrainedTemplates);
  PreTrainedTemplates = NULL;
  FreeBitVector(AllProtosOn);
  FreeBitVector(PrunedProtos);
  FreeBitVector(AllConfigsOn);
  FreeBitVector(AllProtosOff);
  FreeBitVector(AllConfigsOff);
  FreeBitVector(TempProtoMask);
  AllProtosOn = NULL;
  PrunedProtos = NULL;
  AllConfigsOn = NULL;
  AllProtosOff = NULL;
  AllConfigsOff = NULL;
  TempProtoMask = NULL;
}                                /* EndAdaptiveClassifier */


/*---------------------------------------------------------------------------*/
void InitAdaptiveClassifier() {
/*
 **                         Parameters: none
 **                         Globals:
 **                         BuiltInTemplatesFile
              file to get built-in temps from
**                          BuiltInCutoffsFile
              file to get avg. feat per class from
**                          PreTrainedTemplates
              pre-trained configs and protos
**                          AdaptedTemplates
              templates adapted to current page
**                          CharNormCutoffs
              avg # of features per class
**                          AllProtosOn
              dummy proto mask with all bits 1
**                          AllConfigsOn
              dummy config mask with all bits 1
**                          UsePreAdaptedTemplates
              enables use of pre-adapted templates
**                          Operation: This routine reads in the training information needed
**                          by the adaptive classifier and saves it into global
**                          variables.
**                          Return: none
**                          Exceptions: none
**                          History: Mon Mar 11 12:49:34 1991, DSJ, Created.
*/
  int i;
  FILE *File;
  STRING Filename;

  if (!EnableAdaptiveMatcher)
    return;
  if (PreTrainedTemplates != NULL)
    EndAdaptiveClassifier();  // Don't leak with multiple inits.

  Filename = language_data_path_prefix;
  Filename += BuiltInTemplatesFile;
  #ifndef SECURE_NAMES
  //      cprintf( "\nReading built-in templates from %s ...",
  //              Filename);
  fflush(stdout);
  #endif

  #ifdef __UNIX__
  File = Efopen (Filename.string(), "r");
  #else
  File = Efopen (Filename.string(), "rb");
  #endif
  PreTrainedTemplates = ReadIntTemplates (File, TRUE);
  fclose(File);

  Filename = language_data_path_prefix;
  Filename += BuiltInCutoffsFile;
  #ifndef SECURE_NAMES
  //      cprintf( "\nReading built-in pico-feature cutoffs from %s ...",
  //              Filename);
  fflush(stdout);
  #endif
  ReadNewCutoffs (Filename.string(), PreTrainedTemplates->IndexFor,
                  CharNormCutoffs);

  GetNormProtos();

  InitIntegerMatcher();
  InitIntegerFX();

  AllProtosOn = NewBitVector(MAX_NUM_PROTOS);
  PrunedProtos = NewBitVector(MAX_NUM_PROTOS);
  AllConfigsOn = NewBitVector(MAX_NUM_CONFIGS);
  AllProtosOff = NewBitVector(MAX_NUM_PROTOS);
  AllConfigsOff = NewBitVector(MAX_NUM_CONFIGS);
  TempProtoMask = NewBitVector(MAX_NUM_PROTOS);
  set_all_bits(AllProtosOn, WordsInVectorOfSize(MAX_NUM_PROTOS));
  set_all_bits(PrunedProtos, WordsInVectorOfSize(MAX_NUM_PROTOS));
  set_all_bits(AllConfigsOn, WordsInVectorOfSize(MAX_NUM_CONFIGS));
  zero_all_bits(AllProtosOff, WordsInVectorOfSize(MAX_NUM_PROTOS));
  zero_all_bits(AllConfigsOff, WordsInVectorOfSize(MAX_NUM_CONFIGS));

  if (UsePreAdaptedTemplates) {
    Filename = imagefile;
    Filename += ADAPT_TEMPLATE_SUFFIX;
    File = fopen (Filename.string(), "rb");
    if (File == NULL)
      AdaptedTemplates = NewAdaptedTemplates ();
    else {
      #ifndef SECURE_NAMES
      cprintf ("\nReading pre-adapted templates from %s ...", Filename.string());
      fflush(stdout);
      #endif
      AdaptedTemplates = ReadAdaptedTemplates (File);
      cprintf ("\n");
      fclose(File);
      PrintAdaptedTemplates(stdout, AdaptedTemplates);

      for (i = 0; i < (AdaptedTemplates->Templates)->NumClasses; i++) {
        BaselineCutoffs[i] =
          CharNormCutoffs[PreTrainedTemplates->IndexFor[
          AdaptedTemplates->Templates->ClassIdFor[i]]];
      }
    }
  } else {
    if (AdaptedTemplates != NULL)
      free_adapted_templates(AdaptedTemplates);
    AdaptedTemplates = NewAdaptedTemplates ();
  }
  old_enable_learning = EnableLearning;

}                                /* InitAdaptiveClassifier */

void ResetAdaptiveClassifier() {
  free_adapted_templates(AdaptedTemplates);
  AdaptedTemplates = NULL;
}


/*---------------------------------------------------------------------------*/
void InitAdaptiveClassifierVars() {
/*
 **                         Parameters: none
 **                         Globals: none
 **                         Operation: This routine installs the control knobs used by the
 **                         adaptive matcher.
 **                         Return: none
 **                         Exceptions: none
 **                         History: Mon Mar 11 12:49:34 1991, DSJ, Created.
 */
  VALUE dummy;

  string_variable (BuiltInTemplatesFile, "BuiltInTemplatesFile",
    BUILT_IN_TEMPLATES_FILE);
  string_variable (BuiltInCutoffsFile, "BuiltInCutoffsFile",
    BUILT_IN_CUTOFFS_FILE);

  MakeEnableAdaptiveMatcher();
  MakeUsePreAdaptedTemplates();
  MakeSaveAdaptedTemplates();

  MakeEnableLearning();
  MakeEnableAdaptiveDebugger();
  MakeBadMatchPad();
  MakeGoodAdaptiveMatch();
  MakeGreatAdaptiveMatch();
  MakeNoiseBlobLength();
  MakeMinNumPermClasses();
  MakeReliableConfigThreshold();
  MakeMaxAngleDelta();
  MakeLearningDebugLevel();
  MakeMatcherDebugLevel();
  MakeMatchDebugFlags();
  MakeRatingMargin();
  MakePerfectRating();
  MakeEnableIntFX();
  MakeEnableNewAdaptRules();
  MakeRatingScale();
  MakeCertaintyScale();
  MakeFailedAdaptionsBeforeReset();

  InitPicoFXVars();
  InitOutlineFXVars();  //?

}                                /* InitAdaptiveClassifierVars */


/*---------------------------------------------------------------------------*/
void PrintAdaptiveStatistics(FILE *File) {
/*
 **                         Parameters:
 **                         File
              open text file to print adaptive statistics to
**                          Globals: none
**                          Operation: Print to File the statistics which have been gathered
**                          for the adaptive matcher.
**                          Return: none
**                          Exceptions: none
**                          History: Thu Apr 18 14:37:37 1991, DSJ, Created.
*/
  #ifndef SECURE_NAMES

  fprintf (File, "\nADAPTIVE MATCHER STATISTICS:\n");
  fprintf (File, "\tNum blobs classified = %d\n", AdaptiveMatcherCalls);
  fprintf (File, "\tNum classes output   = %d (Avg = %4.2f)\n",
    NumClassesOutput,
    ((AdaptiveMatcherCalls == 0) ? (0.0) :
  ((float) NumClassesOutput / AdaptiveMatcherCalls)));
  fprintf (File, "\t\tBaseline Classifier: %4d calls (%4.2f classes/call)\n",
    BaselineClassifierCalls,
    ((BaselineClassifierCalls == 0) ? (0.0) :
  ((float) NumBaselineClassesTried / BaselineClassifierCalls)));
  fprintf (File, "\t\tCharNorm Classifier: %4d calls (%4.2f classes/call)\n",
    CharNormClassifierCalls,
    ((CharNormClassifierCalls == 0) ? (0.0) :
  ((float) NumCharNormClassesTried / CharNormClassifierCalls)));
  fprintf (File, "\t\tAmbig    Classifier: %4d calls (%4.2f classes/call)\n",
    AmbigClassifierCalls,
    ((AmbigClassifierCalls == 0) ? (0.0) :
  ((float) NumAmbigClassesTried / AmbigClassifierCalls)));

  fprintf (File, "\nADAPTIVE LEARNER STATISTICS:\n");
  fprintf (File, "\tNumber of words adapted to: %d\n", NumWordsAdaptedTo);
  fprintf (File, "\tNumber of chars adapted to: %d\n", NumCharsAdaptedTo);

  if (UsePreAdaptedTemplates)
    PrintAdaptedTemplates(File, AdaptedTemplates);
  #endif
}                                /* PrintAdaptiveStatistics */


/*---------------------------------------------------------------------------*/
void SettupPass1() {
/*
 **                         Parameters: none
 **                         Globals:
 **                         EnableLearning
              set to TRUE by this routine
**                          Operation: This routine prepares the adaptive matcher for the start
**                          of the first pass.  Learning is enabled (unless it is
**                          disabled for the whole program).
**                          Return: none
**                          Exceptions: none
**                          History: Mon Apr 15 16:39:29 1991, DSJ, Created.
*/
  /* Note: this is somewhat redundant, it simply says that if learning is
  enabled then it will remain enabled on the first pass.  If it is
  disabled, then it will remain disabled.  This is only put here to
  make it very clear that learning is controlled directly by the global
    setting of EnableLearning. */
  EnableLearning = old_enable_learning;

  SettupStopperPass1();

}                                /* SettupPass1 */


/*---------------------------------------------------------------------------*/
void SettupPass2() {
/*
 **                         Parameters: none
 **                         Globals:
 **                         EnableLearning
              set to FALSE by this routine
**                          Operation: This routine prepares the adaptive matcher for the start
**                          of the second pass.  Further learning is disabled.
**                          Return: none
**                          Exceptions: none
**                          History: Mon Apr 15 16:39:29 1991, DSJ, Created.
*/
  EnableLearning = FALSE;
  SettupStopperPass2();

}                                /* SettupPass2 */


/*---------------------------------------------------------------------------*/
void MakeNewAdaptedClass(TBLOB *Blob,
                         LINE_STATS *LineStats,
                         CLASS_ID ClassId,
                         ADAPT_TEMPLATES Templates) {
/*
 **                         Parameters:
 **                         Blob
              blob to model new class after
**                          LineStats
              statistics for text row blob is in
**                          ClassId
              id of new class to be created
**                          Templates
              adapted templates to add new class to
**                          Globals:
**                          AllProtosOn
              dummy mask with all 1's
**                          BaselineCutoffs
              kludge needed to get cutoffs
**                          PreTrainedTemplates
              kludge needed to get cutoffs
**                          Operation: This routine creates a new adapted class and uses Blob
**                          as the model for the first config in that class.
**                          Return: none
**                          Exceptions: none
**                          History: Thu Mar 14 12:49:39 1991, DSJ, Created.
*/
  FEATURE_SET Features;
  int Fid, Pid;
  FEATURE Feature;
  int NumFeatures;
  TEMP_PROTO TempProto;
  PROTO Proto;
  ADAPT_CLASS Class;
  INT_CLASS IClass;
  CLASS_INDEX ClassIndex;
  TEMP_CONFIG Config;

  NormMethod = baseline;
  Features = ExtractOutlineFeatures (Blob, LineStats);
  NumFeatures = Features->NumFeatures;
  if (NumFeatures > UNLIKELY_NUM_FEAT) {
    FreeFeatureSet(Features);
    return;
  }

  Class = NewAdaptedClass ();
  ClassIndex = AddAdaptedClass (Templates, Class, ClassId);
  Config = NewTempConfig (NumFeatures - 1);
  TempConfigFor (Class, 0) = Config;

  /* this is a kludge to construct cutoffs for adapted templates */
  if (Templates == AdaptedTemplates)
    BaselineCutoffs[ClassIndex] =
        CharNormCutoffs[PreTrainedTemplates->IndexFor[ClassId]];

  IClass = ClassForClassId (Templates->Templates, ClassId);

  for (Fid = 0; Fid < Features->NumFeatures; Fid++) {
    Pid = AddIntProto (IClass);
    assert (Pid != NO_PROTO);

    Feature = Features->Features[Fid];
    TempProto = NewTempProto ();
    Proto = &(TempProto->Proto);

    /* compute proto params - NOTE that Y_DIM_OFFSET must be used because
       ConvertProto assumes that the Y dimension varies from -0.5 to 0.5
       instead of the -0.25 to 0.75 used in baseline normalization */
    Proto->Angle = Feature->Params[OutlineFeatDir];
    Proto->X = Feature->Params[OutlineFeatX];
    Proto->Y = Feature->Params[OutlineFeatY] - Y_DIM_OFFSET;
    Proto->Length = Feature->Params[OutlineFeatLength];
    FillABC(Proto);

    TempProto->ProtoId = Pid;
    SET_BIT (Config->Protos, Pid);

    ConvertProto(Proto, Pid, IClass);
    AddProtoToProtoPruner(Proto, Pid, IClass);

    Class->TempProtos = push (Class->TempProtos, TempProto);
  }
  FreeFeatureSet(Features);

  AddIntConfig(IClass);
  ConvertConfig (AllProtosOn, 0, IClass);

  if (LearningDebugLevel >= 1) {
    cprintf ("Added new class '%s' with index %d and %d protos.\n",
             unicharset.id_to_unichar(ClassId), ClassIndex, NumFeatures);
  }
}                                /* MakeNewAdaptedClass */


/*---------------------------------------------------------------------------*/
int GetAdaptiveFeatures(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        INT_FEATURE_ARRAY IntFeatures,
                        FEATURE_SET *FloatFeatures) {
/*
 **                         Parameters:
 **                         Blob
              blob to extract features from
**                          LineStats
              statistics about text row blob is in
**                          IntFeatures
              array to fill with integer features
**                          FloatFeatures
              place to return actual floating-pt features
**                          Globals: none
**                          Operation: This routine sets up the feature extractor to extract
**                          baseline normalized pico-features.
**                          The extracted pico-features are converted
**                          to integer form and placed in IntFeatures.  The original
**                          floating-pt. features are returned in FloatFeatures.
**                          Return: Number of pico-features returned (0 if an error occurred)
**                          Exceptions: none
**                          History: Tue Mar 12 17:55:18 1991, DSJ, Created.
*/
  FEATURE_SET Features;
  int NumFeatures;

  NormMethod = baseline;
  Features = ExtractPicoFeatures (Blob, LineStats);

  NumFeatures = Features->NumFeatures;
  if (NumFeatures > UNLIKELY_NUM_FEAT) {
    FreeFeatureSet(Features);
    return (0);
  }

  ComputeIntFeatures(Features, IntFeatures);
  *FloatFeatures = Features;

  return (NumFeatures);

}                                /* GetAdaptiveFeatures */


/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
int AdaptableWord(TWERD *Word,
                  const char *BestChoice,
                  const char *BestChoice_lengths,
                  const char *BestRawChoice,
                  const char *BestRawChoice_lengths) {
/*
 **                         Parameters:
 **                         Word
              current word
**                          BestChoice
              best overall choice for word with context
**                          BestRawChoice
              best choice for word without context
**                          Globals: none
**                          Operation: Return TRUE if the specified word is acceptable for
**                          adaptation.
**                          Return: TRUE or FALSE
**                          Exceptions: none
**                          History: Thu May 30 14:25:06 1991, DSJ, Created.
*/
  int BestChoiceLength;

  return (                       /* rules that apply in general - simplest to compute first */
  /*        EnableLearning                                                && */
                                 /* new rules */
    BestChoice != NULL && BestRawChoice != NULL && Word != NULL &&
    (BestChoiceLength = strlen (BestChoice_lengths)) > 0 &&
    BestChoiceLength == NumBlobsIn (Word) &&
    BestChoiceLength <= MAX_ADAPTABLE_WERD_SIZE && (
    (EnableNewAdaptRules
    &&
    CurrentBestChoiceAdjustFactor
    ()
    <=
    ADAPTABLE_WERD
    &&
    AlternativeChoicesWorseThan
    (ADAPTABLE_WERD)
    &&
    CurrentBestChoiceIs
    (BestChoice, BestChoice_lengths))
    ||
                                 /* old rules */
    (!EnableNewAdaptRules
    &&
    BestChoiceLength
    ==
    strlen
    (BestRawChoice_lengths)
    &&
    ((valid_word (BestChoice) && case_ok (BestChoice, BestChoice_lengths)) || (valid_number (BestChoice, BestChoice_lengths) && pure_number (BestChoice, BestChoice_lengths))) && punctuation_ok (BestChoice, BestChoice_lengths) != -1 && punctuation_ok (BestChoice, BestChoice_lengths) <= 1)));

}                                /* AdaptableWord */


/*---------------------------------------------------------------------------*/
void AdaptToChar(TBLOB *Blob,
                 LINE_STATS *LineStats,
                 CLASS_ID ClassId,
                 FLOAT32 Threshold) {
/*
 **                         Parameters:
 **                         Blob
              blob to add to templates for ClassId
**                          LineStats
              statistics about text line blob is in
**                          ClassId
              class to add blob to
**                          Threshold
              minimum match rating to existing template
**                          Globals:
**                          AdaptedTemplates
              current set of adapted templates
**                          AllProtosOn
              dummy mask to match against all protos
**                          AllConfigsOn
              dummy mask to match against all configs
**                          Operation:
**                          Return: none
**                          Exceptions: none
**                          History: Thu Mar 14 09:36:03 1991, DSJ, Created.
*/
  int NumFeatures;
  INT_FEATURE_ARRAY IntFeatures;
  INT_RESULT_STRUCT IntResult;
  CLASS_INDEX ClassIndex;
  INT_CLASS IClass;
  ADAPT_CLASS Class;
  TEMP_CONFIG TempConfig;
  FEATURE_SET FloatFeatures;
  int NewTempConfigId;

  NumCharsAdaptedTo++;
  if (!LegalClassId (ClassId))
    return;

  if (UnusedClassIdIn (AdaptedTemplates->Templates, ClassId)) {
    MakeNewAdaptedClass(Blob, LineStats, ClassId, AdaptedTemplates);
  }
  else {
    IClass = ClassForClassId (AdaptedTemplates->Templates, ClassId);
    ClassIndex = AdaptedTemplates->Templates->IndexFor[ClassId];
    Class = AdaptedTemplates->Class[ClassIndex];

    NumFeatures = GetAdaptiveFeatures (Blob, LineStats,
      IntFeatures, &FloatFeatures);
    if (NumFeatures <= 0)
      return;

    SetBaseLineMatch();
    IntegerMatcher (IClass, AllProtosOn, AllConfigsOn,
      NumFeatures, NumFeatures, IntFeatures, 0,
      &IntResult, NO_DEBUG);

    SetAdaptiveThreshold(Threshold);

    if (IntResult.Rating <= Threshold) {
      if (ConfigIsPermanent (Class, IntResult.Config)) {
        if (LearningDebugLevel >= 1)
          cprintf ("Found good match to perm config %d = %4.1f%%.\n",
            IntResult.Config, (1.0 - IntResult.Rating) * 100.0);
        FreeFeatureSet(FloatFeatures);
        return;
      }

      TempConfig = TempConfigFor (Class, IntResult.Config);
      IncreaseConfidence(TempConfig);
      if (LearningDebugLevel >= 1)
        cprintf ("Increasing reliability of temp config %d to %d.\n",
          IntResult.Config, TempConfig->NumTimesSeen);

      if (TempConfigReliable (TempConfig))
        MakePermanent (AdaptedTemplates, ClassId, IntResult.Config,
          Blob, LineStats);
    }
    else {
      if (LearningDebugLevel >= 1)
        cprintf ("Found poor match to temp config %d = %4.1f%%.\n",
          IntResult.Config, (1.0 - IntResult.Rating) * 100.0);
      NewTempConfigId = MakeNewTemporaryConfig(AdaptedTemplates,
                                               ClassId,
                                               NumFeatures,
                                               IntFeatures,
                                               FloatFeatures);

      if (NewTempConfigId >= 0 &&
          TempConfigReliable (TempConfigFor (Class, NewTempConfigId)))
        MakePermanent (AdaptedTemplates, ClassId, NewTempConfigId,
                       Blob, LineStats);

#ifndef GRAPHICS_DISABLED
	  if (LearningDebugLevel >= 1) {
        IntegerMatcher (IClass, AllProtosOn, AllConfigsOn,
          NumFeatures, NumFeatures, IntFeatures, 0,
          &IntResult, NO_DEBUG);
        cprintf ("Best match to temp config %d = %4.1f%%.\n",
          IntResult.Config, (1.0 - IntResult.Rating) * 100.0);
        if (LearningDebugLevel >= 2) {
          uinT32 ConfigMask;
          ConfigMask = 1 << IntResult.Config;
          ShowMatchDisplay();
          IntegerMatcher (IClass, AllProtosOn, (BIT_VECTOR)&ConfigMask,
            NumFeatures, NumFeatures, IntFeatures, 0,
            &IntResult, 6 | 0x19);
          UpdateMatchDisplay();
          GetClassToDebug ("Adapting");
        }
      }
#endif  // GRAPHICS_DISABLED
    }
    FreeFeatureSet(FloatFeatures);
  }
}                                /* AdaptToChar */


/*---------------------------------------------------------------------------*/
void AdaptToPunc(TBLOB *Blob,
                 LINE_STATS *LineStats,
                 CLASS_ID ClassId,
                 FLOAT32 Threshold) {
/*
 **                         Parameters:
 **                         Blob
              blob to add to templates for ClassId
**                          LineStats
              statistics about text line blob is in
**                          ClassId
              class to add blob to
**                          Threshold
              minimum match rating to existing template
**                          Globals:
**                          PreTrainedTemplates
              current set of built-in templates
**                          Operation:
**                          Return: none
**                          Exceptions: none
**                          History: Thu Mar 14 09:36:03 1991, DSJ, Created.
*/
  ADAPT_RESULTS Results;
  int i;

  Results.BlobLength = MAX_INT32;
  Results.NumMatches = 0;
  Results.BestRating = WORST_POSSIBLE_RATING;
  Results.BestClass = NO_CLASS;
  Results.BestConfig = 0;
  InitMatcherRatings (Results.Ratings);
  CharNormClassifier(Blob, LineStats, PreTrainedTemplates, &Results);
  RemoveBadMatches(&Results);

  if (Results.NumMatches != 1) {
    if (LearningDebugLevel >= 1) {
      cprintf ("Rejecting punc = %s (Alternatives = ",
               unicharset.id_to_unichar(ClassId));

      for (i = 0; i < Results.NumMatches; i++)
        cprintf ("%s", unicharset.id_to_unichar(Results.Classes[i]));
      cprintf (")\n");
    }
    return;
  }

  #ifndef SECURE_NAMES
  if (LearningDebugLevel >= 1)
    cprintf ("Adapting to punc = %s, thr= %g\n",
             unicharset.id_to_unichar(ClassId), Threshold);
  #endif
  AdaptToChar(Blob, LineStats, ClassId, Threshold);

}                                /* AdaptToPunc */


/*---------------------------------------------------------------------------*/
void AddNewResult(ADAPT_RESULTS *Results,
                  CLASS_ID ClassId,
                  FLOAT32 Rating,
                  int ConfigId) {
/*
 **                         Parameters:
 **                         Results
              results to add new result to
**                          ClassId
              class of new result
**                          Rating
              rating of new result
**                          ConfigId
              config id of new result
**                          Globals:
**                          BadMatchPad
              defines limits of an acceptable match
**                          Operation: This routine adds the result of a classification into
**                          Results.  If the new rating is much worse than the current
**                          best rating, it is not entered into results because it
**                          would end up being stripped later anyway.  If the new rating
**                          is better than the old rating for the class, it replaces the
**                          old rating.  If this is the first rating for the class, the
**                          class is added to the list of matched classes in Results.
**                          If the new rating is better than the best so far, it
**                          becomes the best so far.
**                          Return: none
**                          Exceptions: none
**                          History: Tue Mar 12 18:19:29 1991, DSJ, Created.
*/
  FLOAT32 OldRating;
  INT_CLASS_STRUCT* CharClass = NULL;

  OldRating = Results->Ratings[ClassId];
  if (Rating <= Results->BestRating + BadMatchPad && Rating < OldRating) {
    Results->Ratings[ClassId] = Rating;
    if (ClassId != NO_CLASS)
      CharClass = ClassForClassId(PreTrainedTemplates, ClassId);
    if (CharClass != NULL && CharClass->NumConfigs == 32)
      Results->Configs[ClassId] = ConfigId;
    else
      Results->Configs[ClassId] = ~0;

    if (Rating < Results->BestRating) {
      Results->BestRating = Rating;
      Results->BestClass = ClassId;
      Results->BestConfig = ConfigId;
    }

    /* if this is first rating for class, add to list of classes matched */
    if (OldRating == WORST_POSSIBLE_RATING)
      Results->Classes[Results->NumMatches++] = ClassId;
  }
}                                /* AddNewResult */


/*---------------------------------------------------------------------------*/
void AmbigClassifier(TBLOB *Blob,
                     LINE_STATS *LineStats,
                     INT_TEMPLATES Templates,
                     UNICHAR_ID *Ambiguities,
                     ADAPT_RESULTS *Results) {
/*
 **                         Parameters:
 **                         Blob
              blob to be classified
**                          LineStats
              statistics for text line Blob is in
**                          Templates
              built-in templates to classify against
**                          Ambiguities
              array of class id's to match against
**                          Results
              place to put match results
**                          Globals:
**                          AllProtosOn
              mask that enables all protos
**                          AllConfigsOn
              mask that enables all configs
**                          Operation: This routine is identical to CharNormClassifier()
**                          except that it does no class pruning.  It simply matches
**                          the unknown blob against the classes listed in
**                          Ambiguities.
**                          Return: none
**                          Exceptions: none
**                          History: Tue Mar 12 19:40:36 1991, DSJ, Created.
*/
  int NumFeatures;
  INT_FEATURE_ARRAY IntFeatures;
  CLASS_NORMALIZATION_ARRAY CharNormArray;
  INT_RESULT_STRUCT IntResult;
  CLASS_ID ClassId;
  CLASS_INDEX ClassIndex;

  AmbigClassifierCalls++;

  NumFeatures = GetCharNormFeatures (Blob, LineStats,
    Templates,
    IntFeatures, CharNormArray,
    &(Results->BlobLength));
  if (NumFeatures <= 0)
    return;

  if (MatcherDebugLevel >= 2)
    cprintf ("AM Matches =  ");

  while (*Ambiguities >= 0) {
    ClassId = *Ambiguities;
    ClassIndex = Templates->IndexFor[ClassId];

    SetCharNormMatch();
    IntegerMatcher (ClassForClassId (Templates, ClassId),
      AllProtosOn, AllConfigsOn,
      Results->BlobLength, NumFeatures, IntFeatures,
      CharNormArray[ClassIndex], &IntResult, NO_DEBUG);

    if (MatcherDebugLevel >= 2)
      cprintf ("%s-%-2d %2.0f  ", unicharset.id_to_unichar(ClassId),
               IntResult.Config,
               IntResult.Rating * 100.0);

    AddNewResult (Results, ClassId, IntResult.Rating, IntResult.Config);

    Ambiguities++;

    NumAmbigClassesTried++;
  }
  if (MatcherDebugLevel >= 2)
    cprintf ("\n");

}                                /* AmbigClassifier */

/*---------------------------------------------------------------------------*/
// Factored-out calls to IntegerMatcher based on class pruner results.
// Returns integer matcher results inside CLASS_PRUNER_RESULTS structure.
void MasterMatcher(INT_TEMPLATES templates,
                   inT16 num_features,
                   INT_FEATURE_ARRAY features,
                   CLASS_NORMALIZATION_ARRAY norm_factors,
                   ADAPT_CLASS* classes,
                   int debug,
                   int num_classes,
                   CLASS_PRUNER_RESULTS results,
                   ADAPT_RESULTS* final_results) {
  for (int c = 0; c < num_classes; c++) {
    CLASS_ID class_id = results[c].Class;
    INT_RESULT_STRUCT& int_result = results[c].IMResult;
    CLASS_INDEX class_index = templates->IndexFor[class_id];
    BIT_VECTOR protos = classes != NULL ? classes[class_index]->PermProtos
                                        : AllProtosOn;
    BIT_VECTOR configs = classes != NULL ? classes[class_index]->PermConfigs
                                         : AllConfigsOn;

    IntegerMatcher(ClassForClassId(templates, class_id),
                   protos, configs, final_results->BlobLength,
                   num_features, features, norm_factors[class_index],
                   &int_result, NO_DEBUG);
    // Compute class feature corrections.
    double miss_penalty = tessedit_class_miss_scale *
                          int_result.FeatureMisses;
    if (MatcherDebugLevel >= 2 || display_ratings > 1) {
      cprintf("%s-%-2d %2.1f(CP%2.1f, IM%2.1f + MP%2.1f)  ",
              unicharset.id_to_unichar(class_id), int_result.Config,
              (int_result.Rating + miss_penalty) * 100.0,
              results[c].Rating * 100.0,
              int_result.Rating * 100.0, miss_penalty * 100.0);
      if (c % 4 == 3)
        cprintf ("\n");
    }
    int_result.Rating += miss_penalty;
    if (int_result.Rating > WORST_POSSIBLE_RATING)
      int_result.Rating = WORST_POSSIBLE_RATING;
    AddNewResult(final_results, class_id, int_result.Rating, int_result.Config);
  }
  if (MatcherDebugLevel >= 2 || display_ratings > 1)
    cprintf("\n");
}

/*---------------------------------------------------------------------------*/
UNICHAR_ID *BaselineClassifier(TBLOB *Blob,
                               LINE_STATS *LineStats,
                               ADAPT_TEMPLATES Templates,
                               ADAPT_RESULTS *Results) {
/*
 **                         Parameters:
 **                         Blob
              blob to be classified
**                          LineStats
              statistics for text line Blob is in
**                          Templates
              current set of adapted templates
**                          Results
              place to put match results
**                          Globals:
**                          BaselineCutoffs
              expected num features for each class
**                          Operation: This routine extracts baseline normalized features
**                          from the unknown character and matches them against the
**                          specified set of templates.  The classes which match
**                          are added to Results.
**                          Return: Array of possible ambiguous chars that should be checked.
**                          Exceptions: none
**                          History: Tue Mar 12 19:38:03 1991, DSJ, Created.
*/
  int NumFeatures;
  int NumClasses;
  INT_FEATURE_ARRAY IntFeatures;
  CLASS_NORMALIZATION_ARRAY CharNormArray;
  CLASS_ID ClassId;
  CLASS_INDEX ClassIndex;

  BaselineClassifierCalls++;

  NumFeatures = GetBaselineFeatures (Blob, LineStats,
    Templates->Templates,
    IntFeatures, CharNormArray,
    &(Results->BlobLength));
  if (NumFeatures <= 0)
    return NULL;

  NumClasses = ClassPruner (Templates->Templates, NumFeatures,
    IntFeatures, CharNormArray,
    BaselineCutoffs, Results->CPResults,
    MatchDebugFlags);

  NumBaselineClassesTried += NumClasses;

  if (MatcherDebugLevel >= 2 || display_ratings > 1)
    cprintf ("BL Matches =  ");

  SetBaseLineMatch();
  MasterMatcher(Templates->Templates, NumFeatures, IntFeatures, CharNormArray,
                Templates->Class, MatchDebugFlags, NumClasses,
                Results->CPResults, Results);

  ClassId = Results->BestClass;
  if (ClassId == NO_CLASS)
    return (NULL);
  /* this is a bug - maybe should return "" */

  ClassIndex = Templates->Templates->IndexFor[ClassId];
  return (Templates->Class[ClassIndex]->
    Config[Results->BestConfig].Perm);
}                                /* BaselineClassifier */


/*---------------------------------------------------------------------------*/
void CharNormClassifier(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        INT_TEMPLATES Templates,
                        ADAPT_RESULTS *Results) {
/*
 **                         Parameters:
 **                         Blob
              blob to be classified
**                          LineStats
              statistics for text line Blob is in
**                          Templates
              templates to classify unknown against
**                          Results
              place to put match results
**                          Globals:
**                          CharNormCutoffs
              expected num features for each class
**                          AllProtosOn
              mask that enables all protos
**                          AllConfigsOn
              mask that enables all configs
**                          Operation: This routine extracts character normalized features
**                          from the unknown character and matches them against the
**                          specified set of templates.  The classes which match
**                          are added to Results.
**                          Return: none
**                          Exceptions: none
**                          History: Tue Mar 12 16:02:52 1991, DSJ, Created.
*/
  int NumFeatures;
  int NumClasses;
  INT_FEATURE_ARRAY IntFeatures;
  CLASS_NORMALIZATION_ARRAY CharNormArray;

  CharNormClassifierCalls++;

  NumFeatures = GetCharNormFeatures(Blob, LineStats,
    Templates,
    IntFeatures, CharNormArray,
    &(Results->BlobLength));
  if (NumFeatures <= 0)
    return;

  NumClasses = ClassPruner(Templates, NumFeatures,
                           IntFeatures, CharNormArray,
                           CharNormCutoffs, Results->CPResults,
                           MatchDebugFlags);

  if (tessedit_single_match && NumClasses > 1)
    NumClasses = 1;
  NumCharNormClassesTried += NumClasses;

  if (MatcherDebugLevel >= 2 || display_ratings > 1)
    cprintf("CN Matches =  ");

  SetCharNormMatch();
  MasterMatcher(Templates, NumFeatures, IntFeatures, CharNormArray,
                NULL, MatchDebugFlags, NumClasses,
                Results->CPResults, Results);
}                                /* CharNormClassifier */


/*---------------------------------------------------------------------------*/
void ClassifyAsNoise(TBLOB *Blob,
                     LINE_STATS *LineStats,
                     ADAPT_RESULTS *Results) {
/*
 **                         Parameters:
 **                         Blob
              blob to be classified
**                          LineStats
              statistics for text line Blob is in
**                          Results
              results to add noise classification to
**                          Globals:
**                          NoiseBlobLength
              avg. length of a noise blob
**                          Operation: This routine computes a rating which reflects the
**                          likelihood that the blob being classified is a noise
**                          blob.  NOTE: assumes that the blob length has already been
**                          computed and placed into Results.
**                          Return: none
**                          Exceptions: none
**                          History: Tue Mar 12 18:36:52 1991, DSJ, Created.
*/
  register FLOAT32 Rating;

  Rating = Results->BlobLength / NoiseBlobLength;
  Rating *= Rating;
  Rating /= 1.0 + Rating;

  AddNewResult (Results, NO_CLASS, Rating, 0);
}                                /* ClassifyAsNoise */


/*---------------------------------------------------------------------------*/
int CompareCurrentRatings(                     //CLASS_ID              *Class1,
                          const void *arg1,
                          const void *arg2) {  //CLASS_ID              *Class2)
/*
 **                         Parameters:
 **                         Class1, Class2
              classes whose ratings are to be compared
**                          Globals:
**                          CurrentRatings
              contains actual ratings for each class
**                          Operation: This routine gets the ratings for the 2 specified classes
**                          from a global variable (CurrentRatings) and returns:
**          -1 if Rating1 < Rating2
**                          0 if Rating1 = Rating2
**                          1 if Rating1 > Rating2
**                          Return: Order of classes based on their ratings (see above).
**                          Exceptions: none
**                          History: Tue Mar 12 14:18:31 1991, DSJ, Created.
*/
  FLOAT32 Rating1, Rating2;
  CLASS_ID *Class1 = (CLASS_ID *) arg1;
  CLASS_ID *Class2 = (CLASS_ID *) arg2;

  Rating1 = CurrentRatings[*Class1];
  Rating2 = CurrentRatings[*Class2];

  if (Rating1 < Rating2)
    return (-1);
  else if (Rating1 > Rating2)
    return (1);
  else
    return (0);

}                                /* CompareCurrentRatings */


/*---------------------------------------------------------------------------*/
LIST ConvertMatchesToChoices(ADAPT_RESULTS *Results) {
/*
 **                         Parameters:
 **                         Results
              adaptive matcher results to convert to choices
**                          Globals: none
**                          Operation: This routine creates a choice for each matching class
**                          in Results (up to MAX_MATCHES) and returns a list of
**                          these choices.  The match
**                          ratings are converted to be the ratings and certainties
**                          as used by the context checkers.
**                          Return: List of choices.
**                          Exceptions: none
**                          History: Tue Mar 12 08:55:37 1991, DSJ, Created.
*/
  int i;
  LIST Choices;
  CLASS_ID NextMatch;
  FLOAT32 Rating;
  FLOAT32 Certainty;
  const char *NextMatch_unichar;
  char choice_lengths[2] = {0, 0};

  if (Results->NumMatches > MAX_MATCHES)
    Results->NumMatches = MAX_MATCHES;

  for (Choices = NIL, i = 0; i < Results->NumMatches; i++) {
    NextMatch = Results->Classes[i];
    Rating = Certainty = Results->Ratings[NextMatch];
    Rating *= RatingScale * Results->BlobLength;
    Certainty *= -CertaintyScale;
    if (NextMatch != NO_CLASS)
      NextMatch_unichar = unicharset.id_to_unichar(NextMatch);
    else
      NextMatch_unichar = "";
    choice_lengths[0] = strlen(NextMatch_unichar);
    Choices = append_choice (Choices,
                             NextMatch_unichar,
                             choice_lengths,
                             Rating, Certainty,
                             Results->Configs[NextMatch],
                             unicharset.get_script(NextMatch));
  }
  return (Choices);

}                                /* ConvertMatchesToChoices */


/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
void DebugAdaptiveClassifier(TBLOB *Blob,
                             LINE_STATS *LineStats,
                             ADAPT_RESULTS *Results) {
/*
 **                         Parameters:
 **                         Blob
              blob whose classification is being debugged
**                          LineStats
              statistics for text line blob is in
**                          Results
              results of match being debugged
**                          Globals: none
**                          Operation:
**                          Return: none
**                          Exceptions: none
**                          History: Wed Mar 13 16:44:41 1991, DSJ, Created.
*/
  const char *Prompt =
    "Left-click in IntegerMatch Window to continue or right click to debug...";
  const char *DebugMode = "All Templates";
  CLASS_ID LastClass = Results->BestClass;
  CLASS_ID ClassId;
  BOOL8 AdaptiveOn = TRUE;
  BOOL8 PreTrainedOn = TRUE;

  ShowMatchDisplay();
  cprintf ("\nDebugging class = %s  (%s) ...\n",
           unicharset.id_to_unichar(LastClass), DebugMode);
  ShowBestMatchFor(Blob, LineStats, LastClass, AdaptiveOn, PreTrainedOn);
  UpdateMatchDisplay();

  while ((ClassId = GetClassToDebug (Prompt)) != 0) {
#if 0
    switch (ClassId) {
      case 'b':
        AdaptiveOn = TRUE;
        PreTrainedOn = FALSE;
        DebugMode = "Adaptive Templates Only";
        break;

      case 'c':
        AdaptiveOn = FALSE;
        PreTrainedOn = TRUE;
        DebugMode = "PreTrained Templates Only";
        break;

      case 'a':
        AdaptiveOn = TRUE;
        PreTrainedOn = TRUE;
        DebugMode = "All Templates";
        break;

      default:
        LastClass = ClassId;
        break;
    }
#endif
    LastClass = ClassId;

    ShowMatchDisplay();
    cprintf ("\nDebugging class = %d = %s  (%s) ...\n",
             LastClass, unicharset.id_to_unichar(LastClass), DebugMode);
    ShowBestMatchFor(Blob, LineStats, LastClass, AdaptiveOn, PreTrainedOn);
    UpdateMatchDisplay();
  }
}                                /* DebugAdaptiveClassifier */
#endif

/*---------------------------------------------------------------------------*/
void DoAdaptiveMatch(TBLOB *Blob,
                     LINE_STATS *LineStats,
                     ADAPT_RESULTS *Results) {
/*
 **                         Parameters:
 **                         Blob
              blob to be classified
**                          LineStats
              statistics for text line Blob is in
**                          Results
              place to put match results
**                          Globals:
**                          PreTrainedTemplates
              built-in training templates
**                          AdaptedTemplates
              templates adapted for this page
**                          GreatAdaptiveMatch
              rating limit for a great match
**                          Operation: This routine performs an adaptive classification.
**                          If we have not yet adapted to enough classes, a simple
**                          classification to the pre-trained templates is performed.
**                          Otherwise, we match the blob against the adapted templates.
**                          If the adapted templates do not match well, we try a
**                          match against the pre-trained templates.  If an adapted
**                          template match is found, we do a match to any pre-trained
**                          templates which could be ambiguous.  The results from all
**                          of these classifications are merged together into Results.
**                          Return: none
**                          Exceptions: none
**                          History: Tue Mar 12 08:50:11 1991, DSJ, Created.
*/
  UNICHAR_ID *Ambiguities;

  AdaptiveMatcherCalls++;
  InitIntFX();

  if (AdaptedTemplates->NumPermClasses < MinNumPermClasses
  || tess_cn_matching) {
    CharNormClassifier(Blob, LineStats, PreTrainedTemplates, Results);
  }
  else {
    Ambiguities = BaselineClassifier (Blob, LineStats,
      AdaptedTemplates, Results);

    if ((Results->NumMatches > 0 && MarginalMatch (Results->BestRating)
    && !tess_bn_matching) || Results->NumMatches == 0) {
      CharNormClassifier(Blob, LineStats, PreTrainedTemplates, Results);
    }
    else if (Ambiguities && *Ambiguities >= 0) {
      AmbigClassifier(Blob,
                      LineStats,
                      PreTrainedTemplates,
                      Ambiguities,
                      Results);
    }
  }

  if (Results->NumMatches == 0)
    ClassifyAsNoise(Blob, LineStats, Results);
  /**/}   /* DoAdaptiveMatch */

  /*---------------------------------------------------------------------------*/
  void
  GetAdaptThresholds (TWERD * Word,
  LINE_STATS * LineStats,
  const WERD_CHOICE& BestChoice,
  const WERD_CHOICE& BestRawChoice, FLOAT32 Thresholds[]) {
  /*
   **                           Parameters:
   **                           Word
                current word
  **                            LineStats
                line stats for row word is in
  **                            BestChoice
                best choice for current word with context
  **                            BestRawChoice
                best choice for current word without context
  **                            Thresholds
                array of thresholds to be filled in
  **                            Globals:
  **                            EnableNewAdaptRules
  **                            GoodAdaptiveMatch
  **                            PerfectRating
  **                            RatingMargin
  **                            Operation: This routine tries to estimate how tight the adaptation
  **                            threshold should be set for each character in the current
  **                            word.  In general, the routine tries to set tighter
  **                            thresholds for a character when the current set of templates
  **                            would have made an error on that character.  It tries
  **                            to set a threshold tight enough to eliminate the error.
  **                            Two different sets of rules can be used to determine the
  **                            desired thresholds.
  **                            Return: none (results are returned in Thresholds)
  **                            Exceptions: none
  **                            History: Fri May 31 09:22:08 1991, DSJ, Created.
  */
    TBLOB *Blob;
    const char* BestChoice_string = BestChoice.string().string();
    const char* BestChoice_lengths = BestChoice.lengths().string();
    const char* BestRawChoice_string = BestRawChoice.string().string();
    const char* BestRawChoice_lengths = BestRawChoice.lengths().string();

    if (EnableNewAdaptRules &&   /* new rules */
        CurrentBestChoiceIs (BestChoice_string, BestChoice_lengths)) {
      FindClassifierErrors(PerfectRating,
                           GoodAdaptiveMatch,
                           RatingMargin,
                           Thresholds);
    }
    else {                       /* old rules */
      for (Blob = Word->blobs;
        Blob != NULL;
        Blob = Blob->next, BestChoice_string += *(BestChoice_lengths++),
               BestRawChoice_string += *(BestRawChoice_lengths++), Thresholds++)
        if (*(BestChoice_lengths) == *(BestRawChoice_lengths) &&
            strncmp(BestChoice_string, BestRawChoice_string,
                    *(BestChoice_lengths)) == 0)
        *Thresholds = GoodAdaptiveMatch;
      else {
        /* the blob was incorrectly classified - find the rating threshold
           needed to create a template which will correct the error with
           some margin.  However, don't waste time trying to make
           templates which are too tight. */
        *Thresholds = GetBestRatingFor (Blob, LineStats,
                                        unicharset.unichar_to_id(
                                            BestChoice_string,
                                            *BestChoice_lengths));
        *Thresholds *= (1.0 - RatingMargin);
        if (*Thresholds > GoodAdaptiveMatch)
          *Thresholds = GoodAdaptiveMatch;
        if (*Thresholds < PerfectRating)
          *Thresholds = PerfectRating;
      }
    }
  }                              /* GetAdaptThresholds */

  /*---------------------------------------------------------------------------*/
  UNICHAR_ID *GetAmbiguities(TBLOB *Blob,
                             LINE_STATS *LineStats,
                             CLASS_ID CorrectClass) {
  /*
   **                           Parameters:
   **                           Blob
                blob to get classification ambiguities for
  **                            LineStats
                statistics for text line blob is in
  **                            CorrectClass
                correct class for Blob
  **                            Globals:
  **                            CurrentRatings
                used by qsort compare routine
  **                            PreTrainedTemplates
                built-in templates
  **                            Operation: This routine matches blob to the built-in templates
  **                            to find out if there are any classes other than the correct
  **                            class which are potential ambiguities.
  **                            Return: String containing all possible ambiguous classes.
  **                            Exceptions: none
  **                            History: Fri Mar 15 08:08:22 1991, DSJ, Created.
  */
    ADAPT_RESULTS Results;
    UNICHAR_ID *Ambiguities;
    int i;

    EnterClassifyMode;

    Results.NumMatches = 0;
    Results.BestRating = WORST_POSSIBLE_RATING;
    Results.BestClass = NO_CLASS;
    Results.BestConfig = 0;
    InitMatcherRatings (Results.Ratings);

    CharNormClassifier(Blob, LineStats, PreTrainedTemplates, &Results);
    RemoveBadMatches(&Results);

    /* save ratings in a global so that CompareCurrentRatings() can see them */
    CurrentRatings = Results.Ratings;
    qsort ((void *) (Results.Classes), Results.NumMatches,
      sizeof (CLASS_ID), CompareCurrentRatings);

    /* copy the class id's into an string of ambiguities - don't copy if
       the correct class is the only class id matched */
    Ambiguities = (UNICHAR_ID *) Emalloc (sizeof (UNICHAR_ID) *
                                          (Results.NumMatches + 1));
    if (Results.NumMatches > 1 ||
    (Results.NumMatches == 1 && Results.Classes[0] != CorrectClass)) {
      for (i = 0; i < Results.NumMatches; i++)
        Ambiguities[i] = Results.Classes[i];
      Ambiguities[i] = -1;
    }
    else
      Ambiguities[0] = -1;

    return (Ambiguities);

  }                              /* GetAmbiguities */

  /*---------------------------------------------------------------------------*/
  int GetBaselineFeatures(TBLOB *Blob,
                          LINE_STATS *LineStats,
                          INT_TEMPLATES Templates,
                          INT_FEATURE_ARRAY IntFeatures,
                          CLASS_NORMALIZATION_ARRAY CharNormArray,
                          inT32 *BlobLength) {
  /*
   **                           Parameters:
   **                           Blob
                blob to extract features from
  **                            LineStats
                statistics about text row blob is in
  **                            Templates
                used to compute char norm adjustments
  **                            IntFeatures
                array to fill with integer features
  **                            CharNormArray
                array to fill with dummy char norm adjustments
  **                            BlobLength
                length of blob in baseline-normalized units
  **                            Globals: none
  **                            Operation: This routine sets up the feature extractor to extract
  **                            baseline normalized pico-features.
  **                            The extracted pico-features are converted
  **                            to integer form and placed in IntFeatures.  CharNormArray
  **                            is filled with 0's to indicate to the matcher that no
  **                            character normalization adjustment needs to be done.
  **                            The total length of all blob outlines
  **                            in baseline normalized units is also returned.
  **                            Return: Number of pico-features returned (0 if an error occurred)
  **                            Exceptions: none
  **                            History: Tue Mar 12 17:55:18 1991, DSJ, Created.
  */
    FEATURE_SET Features;
    int NumFeatures;

    if (EnableIntFX)
      return (GetIntBaselineFeatures (Blob, LineStats, Templates,
        IntFeatures, CharNormArray, BlobLength));

    NormMethod = baseline;
    Features = ExtractPicoFeatures (Blob, LineStats);

    NumFeatures = Features->NumFeatures;
    *BlobLength = NumFeatures;
    if (NumFeatures > UNLIKELY_NUM_FEAT) {
      FreeFeatureSet(Features);
      return (0);
    }

    ComputeIntFeatures(Features, IntFeatures);
    ClearCharNormArray(Templates, CharNormArray);

    FreeFeatureSet(Features);
    return (NumFeatures);

  }                              /* GetBaselineFeatures */

  /*---------------------------------------------------------------------------*/
  FLOAT32 GetBestRatingFor(TBLOB *Blob,
                           LINE_STATS *LineStats,
                           CLASS_ID ClassId) {
  /*
   **                           Parameters:
   **                           Blob
                blob to get best rating for
  **                            LineStats
                statistics about text line blob is in
  **                            ClassId
                class blob is to be compared to
  **                            Globals:
  **                            PreTrainedTemplates
                built-in templates
  **                            AdaptedTemplates
                current set of adapted templates
  **                            AllProtosOn
                dummy mask to enable all protos
  **                            AllConfigsOn
                dummy mask to enable all configs
  **                            Operation: This routine classifies Blob against both sets of
  **                            templates for the specified class and returns the best
  **                            rating found.
  **                            Return: Best rating for match of Blob to ClassId.
  **                            Exceptions: none
  **                            History: Tue Apr  9 09:01:24 1991, DSJ, Created.
  */
    int NumCNFeatures, NumBLFeatures;
    INT_FEATURE_ARRAY CNFeatures, BLFeatures;
    INT_RESULT_STRUCT CNResult, BLResult;
    CLASS_NORMALIZATION_ARRAY CNAdjust, BLAdjust;
    CLASS_INDEX ClassIndex;
    inT32 BlobLength;

    CNResult.Rating = BLResult.Rating = 1.0;

    if (!LegalClassId (ClassId))
      return (1.0);

    if (!UnusedClassIdIn (PreTrainedTemplates, ClassId)) {
      NumCNFeatures = GetCharNormFeatures (Blob, LineStats,
        PreTrainedTemplates,
        CNFeatures, CNAdjust, &BlobLength);
      if (NumCNFeatures > 0) {
        ClassIndex = PreTrainedTemplates->IndexFor[ClassId];

        SetCharNormMatch();
        IntegerMatcher (ClassForClassId (PreTrainedTemplates, ClassId),
          AllProtosOn, AllConfigsOn,
          BlobLength, NumCNFeatures, CNFeatures,
          CNAdjust[ClassIndex], &CNResult, NO_DEBUG);
      }
    }

    if (!UnusedClassIdIn (AdaptedTemplates->Templates, ClassId)) {
      NumBLFeatures = GetBaselineFeatures (Blob, LineStats,
        AdaptedTemplates->Templates,
        BLFeatures, BLAdjust, &BlobLength);
      if (NumBLFeatures > 0) {
        ClassIndex = AdaptedTemplates->Templates->IndexFor[ClassId];

        SetBaseLineMatch();
        IntegerMatcher (ClassForClassId
          (AdaptedTemplates->Templates, ClassId),
          AdaptedTemplates->Class[ClassIndex]->PermProtos,
          AdaptedTemplates->Class[ClassIndex]->PermConfigs,
          BlobLength, NumBLFeatures, BLFeatures,
          BLAdjust[ClassIndex], &BLResult, NO_DEBUG);
      }
    }

    return (MIN (BLResult.Rating, CNResult.Rating));

  }                              /* GetBestRatingFor */

  /*---------------------------------------------------------------------------*/
  int GetCharNormFeatures(TBLOB *Blob,
                          LINE_STATS *LineStats,
                          INT_TEMPLATES Templates,
                          INT_FEATURE_ARRAY IntFeatures,
                          CLASS_NORMALIZATION_ARRAY CharNormArray,
                          inT32 *BlobLength) {
  /*
   **                           Parameters:
   **                           Blob
                blob to extract features from
  **                            LineStats
                statistics about text row blob is in
  **                            Templates
                used to compute char norm adjustments
  **                            IntFeatures
                array to fill with integer features
  **                            CharNormArray
                array to fill with char norm adjustments
  **                            BlobLength
                length of blob in baseline-normalized units
  **                            Globals: none
  **                            Operation: This routine sets up the feature extractor to extract
  **                            character normalization features and character normalized
  **                            pico-features.  The extracted pico-features are converted
  **                            to integer form and placed in IntFeatures.  The character
  **                            normalization features are matched to each class in
  **                            templates and the resulting adjustment factors are returned
  **                            in CharNormArray.  The total length of all blob outlines
  **                            in baseline normalized units is also returned.
  **                            Return: Number of pico-features returned (0 if an error occurred)
  **                            Exceptions: none
  **                            History: Tue Mar 12 17:55:18 1991, DSJ, Created.
  */
    return (GetIntCharNormFeatures (Blob, LineStats, Templates,
      IntFeatures, CharNormArray, BlobLength));
  }                              /* GetCharNormFeatures */

  /*---------------------------------------------------------------------------*/
  int GetIntBaselineFeatures(TBLOB *Blob,
                             LINE_STATS *LineStats,
                             INT_TEMPLATES Templates,
                             INT_FEATURE_ARRAY IntFeatures,
                             CLASS_NORMALIZATION_ARRAY CharNormArray,
                             inT32 *BlobLength) {
  /*
   **                           Parameters:
   **                           Blob
                blob to extract features from
  **                            LineStats
                statistics about text row blob is in
  **                            Templates
                used to compute char norm adjustments
  **                            IntFeatures
                array to fill with integer features
  **                            CharNormArray
                array to fill with dummy char norm adjustments
  **                            BlobLength
                length of blob in baseline-normalized units
  **                            Globals:
  **                            FeaturesHaveBeenExtracted
                TRUE if fx has been done
  **                            BaselineFeatures
                holds extracted baseline feat
  **                            CharNormFeatures
                holds extracted char norm feat
  **                            FXInfo
                holds misc. FX info
  **                            Operation: This routine calls the integer (Hardware) feature
  **                            extractor if it has not been called before for this blob.
  **                            The results from the feature extractor are placed into
  **                            globals so that they can be used in other routines without
  **                            re-extracting the features.
  **                            It then copies the baseline features into the IntFeatures
  **                            array provided by the caller.
  **                            Return: Number of features extracted or 0 if an error occured.
  **                            Exceptions: none
  **                            History: Tue May 28 10:40:52 1991, DSJ, Created.
  */
    register INT_FEATURE Src, Dest, End;

    if (!FeaturesHaveBeenExtracted) {
      FeaturesOK = ExtractIntFeat (Blob, BaselineFeatures,
        CharNormFeatures, &FXInfo);
      FeaturesHaveBeenExtracted = TRUE;
    }

    if (!FeaturesOK) {
      *BlobLength = FXInfo.NumBL;
      return (0);
    }

    for (Src = BaselineFeatures, End = Src + FXInfo.NumBL, Dest = IntFeatures;
      Src < End; *Dest++ = *Src++);

    ClearCharNormArray(Templates, CharNormArray);
    *BlobLength = FXInfo.NumBL;
    return (FXInfo.NumBL);

  }                              /* GetIntBaselineFeatures */

  /*---------------------------------------------------------------------------*/
  int GetIntCharNormFeatures(TBLOB *Blob,
                             LINE_STATS *LineStats,
                             INT_TEMPLATES Templates,
                             INT_FEATURE_ARRAY IntFeatures,
                             CLASS_NORMALIZATION_ARRAY CharNormArray,
                             inT32 *BlobLength) {
  /*
   **                           Parameters:
   **                           Blob
                blob to extract features from
  **                            LineStats
                statistics about text row blob is in
  **                            Templates
                used to compute char norm adjustments
  **                            IntFeatures
                array to fill with integer features
  **                            CharNormArray
                array to fill with dummy char norm adjustments
  **                            BlobLength
                length of blob in baseline-normalized units
  **                            Globals:
  **                            FeaturesHaveBeenExtracted
                TRUE if fx has been done
  **                            BaselineFeatures
                holds extracted baseline feat
  **                            CharNormFeatures
                holds extracted char norm feat
  **                            FXInfo
                holds misc. FX info
  **                            Operation: This routine calls the integer (Hardware) feature
  **                            extractor if it has not been called before for this blob.
  **                            The results from the feature extractor are placed into
  **                            globals so that they can be used in other routines without
  **                            re-extracting the features.
  **                            It then copies the char norm features into the IntFeatures
  **                            array provided by the caller.
  **                            Return: Number of features extracted or 0 if an error occured.
  **                            Exceptions: none
  **                            History: Tue May 28 10:40:52 1991, DSJ, Created.
  */
    register INT_FEATURE Src, Dest, End;
    FEATURE NormFeature;
    FLOAT32 Baseline, Scale;

    if (!FeaturesHaveBeenExtracted) {
      FeaturesOK = ExtractIntFeat (Blob, BaselineFeatures,
        CharNormFeatures, &FXInfo);
      FeaturesHaveBeenExtracted = TRUE;
    }

    if (!FeaturesOK) {
      *BlobLength = FXInfo.NumBL;
      return (0);
    }

    for (Src = CharNormFeatures, End = Src + FXInfo.NumCN, Dest = IntFeatures;
      Src < End; *Dest++ = *Src++);

    NormFeature = NewFeature (&CharNormDesc);
    Baseline = BaselineAt (LineStats, FXInfo.Xmean);
    Scale = ComputeScaleFactor (LineStats);
    NormFeature->Params[CharNormY] = (FXInfo.Ymean - Baseline) * Scale;
    NormFeature->Params[CharNormLength] =
      FXInfo.Length * Scale / LENGTH_COMPRESSION;
    NormFeature->Params[CharNormRx] = FXInfo.Rx * Scale;
    NormFeature->Params[CharNormRy] = FXInfo.Ry * Scale;
    ComputeIntCharNormArray(NormFeature, Templates, CharNormArray);
    FreeFeature(NormFeature);

    *BlobLength = FXInfo.NumBL;
    return (FXInfo.NumCN);

  }                              /* GetIntCharNormFeatures */

  /*---------------------------------------------------------------------------*/
  void InitMatcherRatings(register FLOAT32 *Rating) {
  /*
   **                           Parameters:
   **                           Rating
                ptr to array of ratings to be initialized
  **                            Globals: none
  **                            Operation: This routine initializes the best rating for each class
  **                            to be the worst possible rating (1.0).
  **                            Return: none
  **                            Exceptions: none
  **                            History: Tue Mar 12 13:43:28 1991, DSJ, Created.
  */
    register FLOAT32 *LastRating;
    register FLOAT32 WorstRating = WORST_POSSIBLE_RATING;

    for (LastRating = Rating + MAX_CLASS_ID;
      Rating <= LastRating; *Rating++ = WorstRating);

  }                              /* InitMatcherRatings */

  /*---------------------------------------------------------------------------*/
  int MakeNewTemporaryConfig(ADAPT_TEMPLATES Templates,
                              CLASS_ID ClassId,
                              int NumFeatures,
                              INT_FEATURE_ARRAY Features,
                              FEATURE_SET FloatFeatures) {
  /*
   **                           Parameters:
   **                           Templates
                adapted templates to add new config to
  **                            ClassId
                class id to associate with new config
  **                            NumFeatures
                number of features in IntFeatures
  **                            Features
                features describing model for new config
  **                            FloatFeatures
                floating-pt representation of features
  **                            Globals:
  **                            AllProtosOn
                mask to enable all protos
  **                            AllConfigsOff
                mask to disable all configs
  **                            TempProtoMask
                defines old protos matched in new config
  **                            Operation:
  **                            Return: The id of the new config created, a negative integer in
  **                                                    case of error.
  **                            Exceptions: none
  **                            History: Fri Mar 15 08:49:46 1991, DSJ, Created.
  */
    CLASS_INDEX ClassIndex;
    INT_CLASS IClass;
    ADAPT_CLASS Class;
    PROTO_ID OldProtos[MAX_NUM_PROTOS];
    FEATURE_ID BadFeatures[MAX_NUM_INT_FEATURES];
    int NumOldProtos;
    int NumBadFeatures;
    int MaxProtoId, OldMaxProtoId;
    int BlobLength = 0;
    int MaskSize;
    int ConfigId;
    TEMP_CONFIG Config;
    int i;
    int debug_level = NO_DEBUG;

    if (LearningDebugLevel >= 3)
      debug_level =
        PRINT_MATCH_SUMMARY | PRINT_FEATURE_MATCHES | PRINT_PROTO_MATCHES;

    ClassIndex = Templates->Templates->IndexFor[ClassId];
    IClass = ClassForClassId (Templates->Templates, ClassId);
    Class = Templates->Class[ClassIndex];

    if (IClass->NumConfigs >= MAX_NUM_CONFIGS)
    {
      ++NumAdaptationsFailed;
      if (LearningDebugLevel >= 1)
        cprintf ("Cannot make new temporary config: maximum number exceeded.\n");
      return -1;
    }

    OldMaxProtoId = IClass->NumProtos - 1;

    NumOldProtos = FindGoodProtos (IClass, AllProtosOn, AllConfigsOff,
      BlobLength, NumFeatures, Features,
      OldProtos, debug_level);

    MaskSize = WordsInVectorOfSize (MAX_NUM_PROTOS);
    zero_all_bits(TempProtoMask, MaskSize);
    for (i = 0; i < NumOldProtos; i++)
      SET_BIT (TempProtoMask, OldProtos[i]);

    NumBadFeatures = FindBadFeatures (IClass, TempProtoMask, AllConfigsOn,
      BlobLength, NumFeatures, Features,
      BadFeatures, debug_level);

    MaxProtoId = MakeNewTempProtos (FloatFeatures, NumBadFeatures, BadFeatures,
      IClass, Class, TempProtoMask);
    if (MaxProtoId == NO_PROTO)
    {
      ++NumAdaptationsFailed;
      if (LearningDebugLevel >= 1)
        cprintf ("Cannot make new temp protos: maximum number exceeded.\n");
      return -1;
    }

    ConfigId = AddIntConfig (IClass);
    ConvertConfig(TempProtoMask, ConfigId, IClass);
    Config = NewTempConfig (MaxProtoId);
    TempConfigFor (Class, ConfigId) = Config;
    copy_all_bits (TempProtoMask, Config->Protos, Config->ProtoVectorSize);

    if (LearningDebugLevel >= 1)
      cprintf ("Making new temp config %d using %d old and %d new protos.\n",
        ConfigId, NumOldProtos, MaxProtoId - OldMaxProtoId);

    return ConfigId;
  }                              /* MakeNewTemporaryConfig */

  /*---------------------------------------------------------------------------*/
  PROTO_ID
  MakeNewTempProtos (FEATURE_SET Features,
  int NumBadFeat,
  FEATURE_ID BadFeat[],
  INT_CLASS IClass,
  ADAPT_CLASS Class, BIT_VECTOR TempProtoMask) {
  /*
   **                           Parameters:
   **                           Features
                floating-pt features describing new character
  **                            NumBadFeat
                number of bad features to turn into protos
  **                            BadFeat
                feature id's of bad features
  **                            IClass
                integer class templates to add new protos to
  **                            Class
                adapted class templates to add new protos to
  **                            TempProtoMask
                proto mask to add new protos to
  **                            Globals: none
  **                            Operation: This routine finds sets of sequential bad features
  **                            that all have the same angle and converts each set into
  **                            a new temporary proto.  The temp proto is added to the
  **                            proto pruner for IClass, pushed onto the list of temp
  **                            protos in Class, and added to TempProtoMask.
  **                            Return: Max proto id in class after all protos have been added.
  **                            Exceptions: none
  **                            History: Fri Mar 15 11:39:38 1991, DSJ, Created.
  */
    FEATURE_ID *ProtoStart;
    FEATURE_ID *ProtoEnd;
    FEATURE_ID *LastBad;
    TEMP_PROTO TempProto;
    PROTO Proto;
    FEATURE F1, F2;
    FLOAT32 X1, X2, Y1, Y2;
    FLOAT32 A1, A2, AngleDelta;
    FLOAT32 SegmentLength;
    PROTO_ID Pid;

    for (ProtoStart = BadFeat, LastBad = ProtoStart + NumBadFeat;
    ProtoStart < LastBad; ProtoStart = ProtoEnd) {
      F1 = Features->Features[*ProtoStart];
      X1 = F1->Params[PicoFeatX];
      Y1 = F1->Params[PicoFeatY];
      A1 = F1->Params[PicoFeatDir];

      for (ProtoEnd = ProtoStart + 1,
        SegmentLength = GetPicoFeatureLength ();
        ProtoEnd < LastBad;
      ProtoEnd++, SegmentLength += GetPicoFeatureLength ()) {
        F2 = Features->Features[*ProtoEnd];
        X2 = F2->Params[PicoFeatX];
        Y2 = F2->Params[PicoFeatY];
        A2 = F2->Params[PicoFeatDir];

        AngleDelta = fabs (A1 - A2);
        if (AngleDelta > 0.5)
          AngleDelta = 1.0 - AngleDelta;

        if (AngleDelta > MaxAngleDelta ||
          fabs (X1 - X2) > SegmentLength ||
          fabs (Y1 - Y2) > SegmentLength)
          break;
      }

      F2 = Features->Features[*(ProtoEnd - 1)];
      X2 = F2->Params[PicoFeatX];
      Y2 = F2->Params[PicoFeatY];
      A2 = F2->Params[PicoFeatDir];

      Pid = AddIntProto (IClass);
      if (Pid == NO_PROTO)
        return (NO_PROTO);

      TempProto = NewTempProto ();
      Proto = &(TempProto->Proto);

      /* compute proto params - NOTE that Y_DIM_OFFSET must be used because
         ConvertProto assumes that the Y dimension varies from -0.5 to 0.5
         instead of the -0.25 to 0.75 used in baseline normalization */
      Proto->Length = SegmentLength;
      Proto->Angle = A1;
      Proto->X = (X1 + X2) / 2.0;
      Proto->Y = (Y1 + Y2) / 2.0 - Y_DIM_OFFSET;
      FillABC(Proto);

      TempProto->ProtoId = Pid;
      SET_BIT(TempProtoMask, Pid);

      ConvertProto(Proto, Pid, IClass);
      AddProtoToProtoPruner(Proto, Pid, IClass);

      Class->TempProtos = push (Class->TempProtos, TempProto);
    }
    return (IClass->NumProtos - 1);
  }                              /* MakeNewTempProtos */

  /*---------------------------------------------------------------------------*/
  void MakePermanent(ADAPT_TEMPLATES Templates,
                     CLASS_ID ClassId,
                     int ConfigId,
                     TBLOB *Blob,
                     LINE_STATS *LineStats) {
  /*
   **                           Parameters:
   **                           Templates
                current set of adaptive templates
  **                            ClassId
                class containing config to be made permanent
  **                            ConfigId
                config to be made permanent
  **                            Blob
                current blob being adapted to
  **                            LineStats
                statistics about text line Blob is in
  **                            Globals: none
  **                            Operation:
  **                            Return: none
  **                            Exceptions: none
  **                            History: Thu Mar 14 15:54:08 1991, DSJ, Created.
  */
    UNICHAR_ID *Ambigs;
    TEMP_CONFIG Config;
    CLASS_INDEX ClassIndex;
    ADAPT_CLASS Class;
    PROTO_KEY ProtoKey;

    ClassIndex = Templates->Templates->IndexFor[ClassId];
    Class = Templates->Class[ClassIndex];
    Config = TempConfigFor (Class, ConfigId);

    MakeConfigPermanent(Class, ConfigId);
    if (Class->NumPermConfigs == 0)
      Templates->NumPermClasses++;
    Class->NumPermConfigs++;

    ProtoKey.Templates = Templates;
    ProtoKey.ClassId = ClassId;
    ProtoKey.ConfigId = ConfigId;
    Class->TempProtos = delete_d (Class->TempProtos, &ProtoKey,
      MakeTempProtoPerm);
    FreeTempConfig(Config);

    Ambigs = GetAmbiguities (Blob, LineStats, ClassId);
    PermConfigFor (Class, ConfigId) = Ambigs;

    if (LearningDebugLevel >= 1) {
      cprintf ("Making config %d permanent with ambiguities '",
               ConfigId, Ambigs);
      for (UNICHAR_ID *AmbigsPointer = Ambigs;
           *AmbigsPointer >= 0; ++AmbigsPointer)
        cprintf("%s", unicharset.id_to_unichar(*AmbigsPointer));
      cprintf("'.\n");
    }

  }                              /* MakePermanent */

  /*---------------------------------------------------------------------------*/
  int MakeTempProtoPerm(void *item1,    //TEMP_PROTO    TempProto,
                        void *item2) {  //PROTO_KEY             *ProtoKey)
  /*
   **                           Parameters:
   **                           TempProto
                temporary proto to compare to key
  **                            ProtoKey
                defines which protos to make permanent
  **                            Globals: none
  **                            Operation: This routine converts TempProto to be permanent if
  **                            its proto id is used by the configuration specified in
  **                            ProtoKey.
  **                            Return: TRUE if TempProto is converted, FALSE otherwise
  **                            Exceptions: none
  **                            History: Thu Mar 14 18:49:54 1991, DSJ, Created.
  */
    CLASS_INDEX ClassIndex;
    ADAPT_CLASS Class;
    TEMP_CONFIG Config;
    TEMP_PROTO TempProto;
    PROTO_KEY *ProtoKey;

    TempProto = (TEMP_PROTO) item1;
    ProtoKey = (PROTO_KEY *) item2;

    ClassIndex = ProtoKey->Templates->Templates->IndexFor[ProtoKey->ClassId];
    Class = ProtoKey->Templates->Class[ClassIndex];
    Config = TempConfigFor (Class, ProtoKey->ConfigId);

    if (TempProto->ProtoId > Config->MaxProtoId ||
      !test_bit (Config->Protos, TempProto->ProtoId))
      return (FALSE);

    MakeProtoPermanent (Class, TempProto->ProtoId);
    AddProtoToClassPruner (&(TempProto->Proto), ProtoKey->ClassId,
      ProtoKey->Templates->Templates);
    FreeTempProto(TempProto);

    return (TRUE);

  }                              /* MakeTempProtoPerm */

  /*---------------------------------------------------------------------------*/
  int NumBlobsIn(TWERD *Word) {
  /*
   **                           Parameters:
   **                           Word
                word to count blobs in
  **                            Globals: none
  **                            Operation: This routine returns the number of blobs in Word.
  **                            Return: Number of blobs in Word.
  **                            Exceptions: none
  **                            History: Thu Mar 14 08:30:27 1991, DSJ, Created.
  */
    register TBLOB *Blob;
    register int NumBlobs;

    if (Word == NULL)
      return (0);

    for (Blob = Word->blobs, NumBlobs = 0;
      Blob != NULL; Blob = Blob->next, NumBlobs++);

    return (NumBlobs);

  }                              /* NumBlobsIn */

  /*---------------------------------------------------------------------------*/
  int NumOutlinesInBlob(TBLOB *Blob) {
  /*
   **                           Parameters:
   **                           Blob
                blob to count outlines in
  **                            Globals: none
  **                            Operation: This routine returns the number of OUTER outlines
  **                            in Blob.
  **                            Return: Number of outer outlines in Blob.
  **                            Exceptions: none
  **                            History: Mon Jun 10 15:46:20 1991, DSJ, Created.
  */
    register TESSLINE *Outline;
    register int NumOutlines;

    if (Blob == NULL)
      return (0);

    for (Outline = Blob->outlines, NumOutlines = 0;
      Outline != NULL; Outline = Outline->next, NumOutlines++);

    return (NumOutlines);

  }                              /* NumOutlinesInBlob */

  /*---------------------------------------------------------------------------*/
  void PrintAdaptiveMatchResults(FILE *File, ADAPT_RESULTS *Results) {
  /*
   **                           Parameters:
   **                           File
                open text file to write Results to
  **                            Results
                match results to write to File
  **                            Globals: none
  **                            Operation: This routine writes the matches in Results to File.
  **                            Return: none
  **                            Exceptions: none
  **                            History: Mon Mar 18 09:24:53 1991, DSJ, Created.
  */
    for (int i = 0; i < Results->NumMatches; ++i) {
      cprintf("%s(%d) %.2f  ",
               unicharset.debug_str(Results->Classes[i]).string(),
               Results->Classes[i],
               Results->Ratings[Results->Classes[i]] * 100.0);
    }
  }                              /* PrintAdaptiveMatchResults */

  /*---------------------------------------------------------------------------*/
  void RemoveBadMatches(ADAPT_RESULTS *Results) {
  /*
   **                           Parameters:
   **                           Results
                contains matches to be filtered
  **                            Globals:
  **                            BadMatchPad
                defines a "bad match"
  **                            Operation: This routine steps thru each matching class in Results
  **                            and removes it from the match list if its rating
  **                            is worse than the BestRating plus a pad.  In other words,
  **                            all good matches get moved to the front of the classes
  **                            array.
  **                            Return: none
  **                            Exceptions: none
  **                            History: Tue Mar 12 13:51:03 1991, DSJ, Created.
  */
    int Next, NextGood;
    FLOAT32 *Rating = Results->Ratings;
    CLASS_ID *Match = Results->Classes;
    FLOAT32 BadMatchThreshold;
    static const char* romans = "i v x I V X";
    BadMatchThreshold = Results->BestRating + BadMatchPad;

    if (bln_numericmode) {
      UNICHAR_ID unichar_id_one = unicharset.contains_unichar("1") ?
          unicharset.unichar_to_id("1") : -1;
      UNICHAR_ID unichar_id_zero = unicharset.contains_unichar("0") ?
          unicharset.unichar_to_id("0") : -1;
      for (Next = NextGood = 0; Next < Results->NumMatches; Next++) {
        if (Rating[Match[Next]] <= BadMatchThreshold) {
          if (!unicharset.get_isalpha(Match[Next]) ||
              strstr(romans, unicharset.id_to_unichar(Match[Next])) != NULL) {
            Match[NextGood++] = Match[Next];
          } else if (unichar_id_one >= 0 && unicharset.eq(Match[Next], "l") &&
                     Rating[unichar_id_one] >= BadMatchThreshold) {
            Match[NextGood++] = unichar_id_one;
            Rating[unichar_id_one] = Rating[unicharset.unichar_to_id("l")];
          } else if (unichar_id_zero >= 0 && unicharset.eq(Match[Next], "O") &&
                     Rating[unichar_id_zero] >= BadMatchThreshold) {
              Match[NextGood++] = unichar_id_zero;
              Rating[unichar_id_zero] = Rating[unicharset.unichar_to_id("O")];
          }
        }
      }
    }
    else {
      for (Next = NextGood = 0; Next < Results->NumMatches; Next++) {
        if (Rating[Match[Next]] <= BadMatchThreshold)
          Match[NextGood++] = Match[Next];
      }
    }

    Results->NumMatches = NextGood;

  }                              /* RemoveBadMatches */

  /*----------------------------------------------------------------------------------*/
  void RemoveExtraPuncs(ADAPT_RESULTS *Results) {
  /*
   **                           Parameters:
   **                           Results
                contains matches to be filtered
  **                            Globals:
  **                            BadMatchPad
                defines a "bad match"
  **                            Operation: This routine steps thru each matching class in Results
  **                            and removes it from the match list if its rating
  **                            is worse than the BestRating plus a pad.  In other words,
  **                            all good matches get moved to the front of the classes
  **                            array.
  **                            Return: none
  **                            Exceptions: none
  **                            History: Tue Mar 12 13:51:03 1991, DSJ, Created.
  */
    int Next, NextGood;
    int punc_count;              /*no of garbage characters */
    int digit_count;
    CLASS_ID *Match = Results->Classes;
                                 /*garbage characters */
    static char punc_chars[] = ". , ; : / ` ~ ' - = \\ | \" ! _ ^";
    static char digit_chars[] = "0 1 2 3 4 5 6 7 8 9";

    punc_count = 0;
    digit_count = 0;
    for (Next = NextGood = 0; Next < Results->NumMatches; Next++) {
      if (strstr (punc_chars,
                  unicharset.id_to_unichar(Match[Next])) == NULL) {
        if (strstr (digit_chars,
                    unicharset.id_to_unichar(Match[Next])) == NULL) {
          Match[NextGood++] = Match[Next];
        }
        else {
          if (digit_count < 1)
            Match[NextGood++] = Match[Next];
          digit_count++;
        }
      }
      else {
        if (punc_count < 2)
          Match[NextGood++] = Match[Next];
        punc_count++;            /*count them */
      }
    }
    Results->NumMatches = NextGood;
  }                              /* RemoveExtraPuncs */

  /*---------------------------------------------------------------------------*/
  void SetAdaptiveThreshold(FLOAT32 Threshold) {
  /*
   **                           Parameters:
   **                           Threshold
                threshold for creating new templates
  **                            Globals:
  **                            GoodAdaptiveMatch
                default good match rating
  **                            Operation: This routine resets the internal thresholds inside
  **                            the integer matcher to correspond to the specified
  **                            threshold.
  **                            Return: none
  **                            Exceptions: none
  **                            History: Tue Apr  9 08:33:13 1991, DSJ, Created.
  */
    if (Threshold == GoodAdaptiveMatch) {
      /* the blob was probably classified correctly - use the default rating
         threshold */
      SetProtoThresh (0.9);
      SetFeatureThresh (0.9);
    }
    else {
      /* the blob was probably incorrectly classified */
      SetProtoThresh (1.0 - Threshold);
      SetFeatureThresh (1.0 - Threshold);
    }
  }                              /* SetAdaptiveThreshold */

  /*---------------------------------------------------------------------------*/
  void ShowBestMatchFor(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        CLASS_ID ClassId,
                        BOOL8 AdaptiveOn,
                        BOOL8 PreTrainedOn) {
  /*
   **                           Parameters:
   **                           Blob
                blob to show best matching config for
  **                            LineStats
                statistics for text line Blob is in
  **                            ClassId
                class whose configs are to be searched
  **                            AdaptiveOn
                TRUE if adaptive configs are enabled
  **                            PreTrainedOn
                TRUE if pretrained configs are enabled
  **                            Globals:
  **                            PreTrainedTemplates
                built-in training
  **                            AdaptedTemplates
                adaptive templates
  **                            AllProtosOn
                dummy proto mask
  **                            AllConfigsOn
                dummy config mask
  **                            Operation: This routine compares Blob to both sets of templates
  **        (adaptive and pre-trained) and then displays debug
  **                            information for the config which matched best.
  **                            Return: none
  **                            Exceptions: none
  **                            History: Fri Mar 22 08:43:52 1991, DSJ, Created.
  */
    int NumCNFeatures = 0, NumBLFeatures = 0;
    INT_FEATURE_ARRAY CNFeatures, BLFeatures;
    INT_RESULT_STRUCT CNResult, BLResult;
    CLASS_NORMALIZATION_ARRAY CNAdjust, BLAdjust;
    CLASS_INDEX ClassIndex;
    inT32 BlobLength;
    uinT32 ConfigMask;
    static int next_config = -1;

    if (PreTrainedOn) next_config = -1;

    CNResult.Rating = BLResult.Rating = 2.0;

    if (!LegalClassId (ClassId)) {
      cprintf ("%d is not a legal class id!!\n", ClassId);
      return;
    }

    if (PreTrainedOn) {
      if (UnusedClassIdIn (PreTrainedTemplates, ClassId))
        cprintf ("No built-in templates for class %d = %s\n",
                 ClassId, unicharset.id_to_unichar(ClassId));
      else {
        NumCNFeatures = GetCharNormFeatures (Blob, LineStats,
          PreTrainedTemplates,
          CNFeatures, CNAdjust,
          &BlobLength);
        if (NumCNFeatures <= 0)
          cprintf ("Illegal blob (char norm features)!\n");
        else {
          ClassIndex = PreTrainedTemplates->IndexFor[ClassId];

          SetCharNormMatch();
          IntegerMatcher (ClassForClassId (PreTrainedTemplates, ClassId),
            AllProtosOn, AllConfigsOn,
            BlobLength, NumCNFeatures, CNFeatures,
            CNAdjust[ClassIndex], &CNResult, NO_DEBUG);

          cprintf ("Best built-in template match is config %2d (%4.1f) (cn=%d)\n",
            CNResult.Config, CNResult.Rating * 100.0, CNAdjust[ClassIndex]);
        }
      }
    }

    if (AdaptiveOn) {
      if (UnusedClassIdIn (AdaptedTemplates->Templates, ClassId))
        cprintf ("No AD templates for class %d = %s\n",
                 ClassId, unicharset.id_to_unichar(ClassId));
      else {
        NumBLFeatures = GetBaselineFeatures (Blob, LineStats,
          AdaptedTemplates->Templates,
          BLFeatures, BLAdjust,
          &BlobLength);
        if (NumBLFeatures <= 0)
          cprintf ("Illegal blob (baseline features)!\n");
        else {
          ClassIndex =AdaptedTemplates->Templates->IndexFor[ClassId];

          SetBaseLineMatch();
          IntegerMatcher (ClassForClassId
            (AdaptedTemplates->Templates, ClassId),
                          AllProtosOn, AllConfigsOn,
  //          AdaptedTemplates->Class[ClassIndex]->PermProtos,
  //          AdaptedTemplates->Class[ClassIndex]->PermConfigs,
            BlobLength, NumBLFeatures, BLFeatures,
            BLAdjust[ClassIndex], &BLResult, NO_DEBUG);

          #ifndef SECURE_NAMES
          int ClassIndex = AdaptedTemplates->Templates->IndexFor[ClassId];
          ADAPT_CLASS Class = AdaptedTemplates->Class[ClassIndex];
          cprintf ("Best adaptive template match is config %2d (%4.1f) %s\n",
                   BLResult.Config, BLResult.Rating * 100.0,
                   ConfigIsPermanent(Class, BLResult.Config) ? "Perm" : "Temp");
          #endif
        }
      }
    }

    cprintf ("\n");
    if (BLResult.Rating < CNResult.Rating) {
      ClassIndex = AdaptedTemplates->Templates->IndexFor[ClassId];
      if (next_config < 0) {
        ConfigMask = 1 << BLResult.Config;
        next_config = 0;
      } else {
        ConfigMask = 1 << next_config;
        ++next_config;
      }
      NormMethod = baseline;

      SetBaseLineMatch();
      IntegerMatcher (ClassForClassId (AdaptedTemplates->Templates, ClassId),
                      AllProtosOn,
//        AdaptedTemplates->Class[ClassIndex]->PermProtos,
        (BIT_VECTOR) & ConfigMask,
        BlobLength, NumBLFeatures, BLFeatures,
        BLAdjust[ClassIndex], &BLResult, MatchDebugFlags);
      cprintf ("Adaptive template match for config %2d is %4.1f\n",
        BLResult.Config, BLResult.Rating * 100.0);
    }
    else {
      ClassIndex = PreTrainedTemplates->IndexFor[ClassId];
      ConfigMask = 1 << CNResult.Config;
      NormMethod = character;

      SetCharNormMatch();
                                 //xiaofan
      IntegerMatcher (ClassForClassId (PreTrainedTemplates, ClassId), AllProtosOn, (BIT_VECTOR) & ConfigMask,
        BlobLength, NumCNFeatures, CNFeatures,
        CNAdjust[ClassIndex], &CNResult, MatchDebugFlags);
    }
  }                              /* ShowBestMatchFor */
