/******************************************************************************
 ** Filename:    adaptmatch.c
 ** Purpose:     High level adaptive matcher.
 ** Author:      Dan Johnson
 ** History:     Mon Mar 11 10:00:10 1991, DSJ, Created.
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

/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include <ctype.h>
#include "adaptmatch.h"
#include "normfeat.h"
#include "mfoutline.h"
#include "picofeat.h"
#include "float2int.h"
#include "outfeat.h"
#include "emalloc.h"
#include "intfx.h"
#include "speckle.h"
#include "efio.h"
#include "normmatch.h"
#include "permute.h"
#include "context.h"
#include "ndminx.h"
#include "intproto.h"
#include "const.h"
#include "globals.h"
#include "werd.h"
#include "callcpp.h"
#include "tordvars.h"
#include "varable.h"
#include "classify.h"
#include "unicharset.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#ifdef __UNIX__
#include <assert.h>
#endif

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define ADAPT_TEMPLATE_SUFFIX ".a"

#define MAX_MATCHES         10
#define UNLIKELY_NUM_FEAT 200
#define NO_DEBUG      0
#define MAX_ADAPTABLE_WERD_SIZE 40

#define ADAPTABLE_WERD    (GOOD_WERD + 0.05)

#define Y_DIM_OFFSET    (Y_SHIFT - BASELINE_Y_SHIFT)

#define WORST_POSSIBLE_RATING (1.0)

struct ADAPT_RESULTS
{
  inT32 BlobLength;
  int NumMatches;
  bool HasNonfragment;
  CLASS_ID Classes[MAX_NUM_CLASSES];
  FLOAT32 Ratings[MAX_CLASS_ID + 1];
  uinT8 Configs[MAX_CLASS_ID + 1];
  FLOAT32 BestRating;
  CLASS_ID BestClass;
  uinT8 BestConfig;
  CLASS_PRUNER_RESULTS CPResults;

  /// Initializes data members to the default values. Sets the initial
  /// rating of each class to be the worst possible rating (1.0).
  inline void Initialize() {
     BlobLength = MAX_INT32;
     NumMatches = 0;
     HasNonfragment = false;
     BestRating = WORST_POSSIBLE_RATING;
     BestClass = NO_CLASS;
     BestConfig = 0;
     for (int i = 0; i <= MAX_CLASS_ID; ++i) {
       Ratings[i] = WORST_POSSIBLE_RATING;
     }
  }
};



typedef struct
{
  ADAPT_TEMPLATES Templates;
  CLASS_ID ClassId;
  int ConfigId;
}


PROTO_KEY;

/*-----------------------------------------------------------------------------
          Private Macros
-----------------------------------------------------------------------------*/
#define MarginalMatch(Rating)       \
((Rating) > matcher_great_threshold)

#define TempConfigReliable(Config)  \
((Config)->NumTimesSeen >= matcher_min_examples_for_prototyping)

#define InitIntFX() (FeaturesHaveBeenExtracted = FALSE)

/*-----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/
void AdaptToChar(TBLOB *Blob,
                 LINE_STATS *LineStats,
                 CLASS_ID ClassId,
                 FLOAT32 Threshold);

void AdaptToPunc(TBLOB *Blob,
                 LINE_STATS *LineStats,
                 CLASS_ID ClassId,
                 FLOAT32 Threshold);

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

void ClassifyAsNoise(ADAPT_RESULTS *Results);

int CompareCurrentRatings(const void *arg1,
                          const void *arg2);

void ConvertMatchesToChoices(ADAPT_RESULTS *Results,
                             BLOB_CHOICE_LIST *Choices);

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

namespace tesseract {
int GetBaselineFeatures(TBLOB *Blob,
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

}  // namespace tesseract.

void InitMatcherRatings(register FLOAT32 *Rating);

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


/*-----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/
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

/** use a global variable to hold onto the current ratings so that the
comparison function passes to qsort can get at them */
static FLOAT32 *CurrentRatings;

/* define globals to hold filename of training data */
static CLASS_CUTOFF_ARRAY CharNormCutoffs;
static CLASS_CUTOFF_ARRAY BaselineCutoffs;

/* define control knobs for adaptive matcher */
BOOL_VAR(classify_enable_adaptive_matcher, 1, "Enable adaptive classifier");

BOOL_VAR(classify_use_pre_adapted_templates, 0,
         "Use pre-adapted classifier templates");

BOOL_VAR(classify_save_adapted_templates, 0,
         "Save adapted templates to a file");

BOOL_VAR(classify_enable_adaptive_debugger, 0, "Enable match debugger");

INT_VAR(matcher_debug_level, 0, "Matcher Debug Level");
INT_VAR(matcher_debug_flags, 0, "Matcher Debug Flags");

INT_VAR(classify_learning_debug_level, 0, "Learning Debug Level: ");

double_VAR(matcher_good_threshold, 0.125, "Good Match (0-1)");
double_VAR(matcher_great_threshold, 0.0, "Great Match (0-1)");

double_VAR(matcher_perfect_threshold, 0.02, "Perfect Match (0-1)");
double_VAR(matcher_bad_match_pad, 0.15, "Bad Match Pad (0-1)");
double_VAR(matcher_rating_margin, 0.1, "New template margin (0-1)");
double_VAR(matcher_avg_noise_size, 12.0, "Avg. noise blob length: ");

INT_VAR(matcher_permanent_classes_min, 1, "Min # of permanent classes");

INT_VAR(matcher_min_examples_for_prototyping, 3, "Reliable Config Threshold");

double_VAR(matcher_clustering_max_angle_delta, 0.015,
           "Maximum angle delta for prototype clustering");

BOOL_VAR(classify_enable_int_fx, 1, "Enable integer fx");

BOOL_VAR(classify_enable_new_adapt_rules, 1, "Enable new adaptation rules");

double_VAR(rating_scale, 1.5, "Rating scaling factor");
extern double_VAR_H(certainty_scale, 20.0, "Certainty scaling factor");

INT_VAR(matcher_failed_adaptations_before_reset, 150,
        "Number of failed adaptions before adapted templates reset");

double_VAR(tessedit_class_miss_scale, 0.00390625,
           "Scale factor for features not used");

BOOL_VAR(tess_cn_matching, 0, "Character Normalized Matching");
BOOL_VAR(tess_bn_matching, 0, "Baseline Normalized Matching");

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 * This routine calls the adaptive matcher
 * which returns (in an array) the class id of each
 * class matched.
 *
 * It also returns the number of classes matched.
 * For each class matched it places the best rating
 * found for that class into the Ratings array.
 *
 * Bad matches are then removed so that they don't
 * need to be sorted.  The remaining good matches are
 * then sorted and converted to choices.
 *
 * This routine also performs some simple speckle
 * filtering.
 *
 * @note Exceptions: none
 * @note History: Mon Mar 11 10:00:58 1991, DSJ, Created.
 *
 * @param Blob    blob to be classified
 * @param DotBlob (obsolete)
 * @param Row     row of text that word appears in
 * @param[out] Choices    List of choices found by adaptive matcher.
 * @param[out] CPResults  Array of CPResultStruct of size MAX_NUM_CLASSES is
 * filled on return with the choices found by the
 * class pruner and the ratings therefrom. Also
 * contains the detailed results of the integer matcher.
 *
 * Globals: 
 * - CurrentRatings  used by compare function for qsort
 */
void Classify::AdaptiveClassifier(TBLOB *Blob,
                                  TBLOB *DotBlob,
                                  TEXTROW *Row,
                                  BLOB_CHOICE_LIST *Choices,
                                  CLASS_PRUNER_RESULTS CPResults) {
  assert(Choices != NULL);
  ADAPT_RESULTS *Results = new ADAPT_RESULTS();
  LINE_STATS LineStats;

  if (matcher_failed_adaptations_before_reset >= 0 &&
      NumAdaptationsFailed >= matcher_failed_adaptations_before_reset) {
    NumAdaptationsFailed = 0;
    ResetAdaptiveClassifier();
  }
  if (AdaptedTemplates == NULL)
    AdaptedTemplates = NewAdaptedTemplates (true);

  EnterClassifyMode;

  Results->Initialize();
  GetLineStatsFromRow(Row, &LineStats);

  DoAdaptiveMatch(Blob, &LineStats, Results);
  if (CPResults != NULL)
    memcpy(CPResults, Results->CPResults,
           sizeof(CPResults[0]) * Results->NumMatches);
  RemoveBadMatches(Results);

  /* save ratings in a global so that CompareCurrentRatings() can see them */
  CurrentRatings = Results->Ratings;
  qsort ((void *) (Results->Classes), Results->NumMatches,
    sizeof (CLASS_ID), CompareCurrentRatings);

  RemoveExtraPuncs(Results);
  ConvertMatchesToChoices(Results, Choices);

  if (matcher_debug_level >= 1) {
    cprintf ("AD Matches =  ");
    PrintAdaptiveMatchResults(stdout, Results);
  }

  if (LargeSpeckle (Blob, Row))
    AddLargeSpeckleTo(Choices);

#ifndef GRAPHICS_DISABLED
  if (classify_enable_adaptive_debugger)
    DebugAdaptiveClassifier(Blob, &LineStats, Results);
#endif

  NumClassesOutput += Choices->length();
  if (Choices->length() == 0) {
    if (!bln_numericmode)
      tprintf ("Empty classification!\n");  // Should never normally happen.
    Choices = new BLOB_CHOICE_LIST();
    BLOB_CHOICE_IT temp_it;
    temp_it.set_to_list(Choices);
    temp_it.add_to_end(new BLOB_CHOICE(0, 50.0f, -20.0f, -1, NULL));
  }

  delete Results;
}                                /* AdaptiveClassifier */


/*---------------------------------------------------------------------------*/
/**
 * This routine implements a preliminary
 * version of the rules which are used to decide
 * which characters to adapt to.
 *
 * A word is adapted to if it is in the dictionary or
 * if it is a "good" number (no trailing units, etc.).
 * It cannot contain broken or merged characters.
 *
 * Within that word, only letters and digits are
 * adapted to (no punctuation).
 *
 * @param Word word to be adapted to
 * @param Row row of text that word is found in
 * @param BestChoice best choice for word found by system
 * @param BestRawChoice best choice for word found by classifier only
 * @param rejmap Reject map
 *
 * Globals:
 * - #EnableLearning TRUE if learning is enabled
 *
 * @note Exceptions: none
 * @note History: Thu Mar 14 07:40:36 1991, DSJ, Created.
*/
void Classify::AdaptToWord(TWERD *Word,
                           TEXTROW *Row,
                           const WERD_CHOICE& BestChoice,
                           const WERD_CHOICE& BestRawChoice,
                           const char *rejmap) {
  TBLOB *Blob;
  LINE_STATS LineStats;
  FLOAT32 Thresholds[MAX_ADAPTABLE_WERD_SIZE];
  FLOAT32 *Threshold;
  const char *map = rejmap;
  char map_char = '1';
  const char* BestChoice_string = BestChoice.unichar_string().string();
  const char* BestChoice_lengths = BestChoice.unichar_lengths().string();

  if (strlen(BestChoice_lengths) > MAX_ADAPTABLE_WERD_SIZE)
    return;

  if (EnableLearning) {
    NumWordsAdaptedTo++;

    #ifndef SECURE_NAMES
    if (classify_learning_debug_level >= 1)
      cprintf ("\n\nAdapting to word = %s\n",
               BestChoice.debug_string(unicharset).string());
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
            if (classify_learning_debug_level >= 1)
              cprintf ("Rejecting char = %s\n", unicharset.id_to_unichar(
                           unicharset.unichar_to_id(BestChoice_string,
                                                    *BestChoice_lengths)));
          }
          else {
            #ifndef SECURE_NAMES
            if (classify_learning_debug_level >= 1)
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
    if (classify_learning_debug_level >= 1)
      cprintf ("\n");
  }
}                                /* AdaptToWord */


/*---------------------------------------------------------------------------*/
/**
 * This routine performs cleanup operations
 * on the adaptive classifier.  It should be called
 * before the program is terminated.  Its main function
 * is to save the adapted templates to a file.
 *
 * Globals:
 * - #AdaptedTemplates current set of adapted templates
 * - #classify_save_adapted_templates TRUE if templates should be saved
 * - #classify_enable_adaptive_matcher TRUE if adaptive matcher is enabled
 *
 * @note Exceptions: none
 * @note History: Tue Mar 19 14:37:06 1991, DSJ, Created.
 */
void Classify::EndAdaptiveClassifier() {
  STRING Filename;
  FILE *File;

  #ifndef SECURE_NAMES
  if (AdaptedTemplates != NULL &&
      classify_enable_adaptive_matcher && classify_save_adapted_templates) {
    Filename = imagefile + ADAPT_TEMPLATE_SUFFIX;
    File = fopen (Filename.string(), "wb");
    if (File == NULL)
      cprintf ("Unable to save adapted templates to %s!\n", Filename.string());
    else {
      cprintf ("\nSaving adapted templates to %s ...", Filename.string());
      fflush(stdout);
      WriteAdaptedTemplates(File, AdaptedTemplates);
      cprintf ("\n");
      fclose(File);
    }
  }
  #endif

  if (AdaptedTemplates != NULL) {
    free_adapted_templates(AdaptedTemplates);
    AdaptedTemplates = NULL;
  }

  if (PreTrainedTemplates != NULL) {
    free_int_templates(PreTrainedTemplates);
    PreTrainedTemplates = NULL;
  }
  getDict().EndDangerousAmbigs();
  FreeNormProtos();
  if (AllProtosOn != NULL) {
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
  }
}                                /* EndAdaptiveClassifier */


/*---------------------------------------------------------------------------*/
/**
 * This routine reads in the training
 * information needed by the adaptive classifier
 * and saves it into global variables.
 *
 * Globals:
 * - BuiltInTemplatesFile file to get built-in temps from
 * - BuiltInCutoffsFile file to get avg. feat per class from
 * - #PreTrainedTemplates pre-trained configs and protos
 * - #AdaptedTemplates templates adapted to current page
 * - CharNormCutoffs avg # of features per class
 * - #AllProtosOn dummy proto mask with all bits 1
 * - #AllConfigsOn dummy config mask with all bits 1
 * - #classify_use_pre_adapted_templates enables use of pre-adapted templates
 *
 * @note Exceptions: none
 * @note History: Mon Mar 11 12:49:34 1991, DSJ, Created.
 */
void Classify::InitAdaptiveClassifier() {
  if (!classify_enable_adaptive_matcher)
    return;
  if (AllProtosOn != NULL)
    EndAdaptiveClassifier();  // Don't leak with multiple inits.

  // If there is no language_data_path_prefix, the classifier will be
  // adaptive only.
  if (language_data_path_prefix.length() > 0) {
    if (!tessdata_manager.SeekToStart(TESSDATA_INTTEMP)) {
      inttemp_loaded_ = false;
    } else {
      PreTrainedTemplates =
        ReadIntTemplates(tessdata_manager.GetDataFilePtr());
      if (global_tessdata_manager_debug_level) tprintf("Loaded inttemp\n");

      ASSERT_HOST(tessdata_manager.SeekToStart(TESSDATA_PFFMTABLE));
      ReadNewCutoffs(tessdata_manager.GetDataFilePtr(),
                     tessdata_manager.GetEndOffset(TESSDATA_PFFMTABLE),
                     CharNormCutoffs);
      if (global_tessdata_manager_debug_level) tprintf("Loaded pffmtable\n");

      ASSERT_HOST(tessdata_manager.SeekToStart(TESSDATA_NORMPROTO));
      NormProtos =
        ReadNormProtos(tessdata_manager.GetDataFilePtr(),
                       tessdata_manager.GetEndOffset(TESSDATA_NORMPROTO));
      if (global_tessdata_manager_debug_level) tprintf("Loaded normproto\n");

      inttemp_loaded_ = true;
    }
  }

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

  if (classify_use_pre_adapted_templates) {
    FILE *File;
    STRING Filename;

    Filename = imagefile;
    Filename += ADAPT_TEMPLATE_SUFFIX;
    File = fopen(Filename.string(), "rb");
    if (File == NULL) {
      AdaptedTemplates = NewAdaptedTemplates(true);
    } else {
      #ifndef SECURE_NAMES
      cprintf("\nReading pre-adapted templates from %s ...\n",
              Filename.string());
      fflush(stdout);
      #endif
      AdaptedTemplates = ReadAdaptedTemplates(File);
      cprintf("\n");
      fclose(File);
      PrintAdaptedTemplates(stdout, AdaptedTemplates);

      for (int i = 0; i < AdaptedTemplates->Templates->NumClasses; i++) {
        BaselineCutoffs[i] = CharNormCutoffs[i];
      }
    }
  } else {
    if (AdaptedTemplates != NULL)
      free_adapted_templates(AdaptedTemplates);
    AdaptedTemplates = NewAdaptedTemplates(true);
  }
}                                /* InitAdaptiveClassifier */

void Classify::ResetAdaptiveClassifier() {
  free_adapted_templates(AdaptedTemplates);
  AdaptedTemplates = NULL;
}
}  // namespace tesseract


/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 * Print to File the statistics which have
 * been gathered for the adaptive matcher.
 *
 * @param File open text file to print adaptive statistics to
 *
 * Globals: none
 *
 * @note Exceptions: none
 * @note History: Thu Apr 18 14:37:37 1991, DSJ, Created.
 */
void Classify::PrintAdaptiveStatistics(FILE *File) {
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

  PrintAdaptedTemplates(File, AdaptedTemplates);
  #endif
}                                /* PrintAdaptiveStatistics */


/*---------------------------------------------------------------------------*/
/**
 * This routine prepares the adaptive
 * matcher for the start
 * of the first pass.  Learning is enabled (unless it
 * is disabled for the whole program).
 *
 * @note this is somewhat redundant, it simply says that if learning is
 * enabled then it will remain enabled on the first pass.  If it is
 * disabled, then it will remain disabled.  This is only put here to
 * make it very clear that learning is controlled directly by the global
 * setting of EnableLearning. 
 *
 * Globals:
 * - #EnableLearning
 * set to TRUE by this routine
 *
 * @note Exceptions: none
 * @note History: Mon Apr 15 16:39:29 1991, DSJ, Created.
 */
void Classify::SettupPass1() {
  EnableLearning = classify_enable_learning;

  getDict().SettupStopperPass1();

}                                /* SettupPass1 */


/*---------------------------------------------------------------------------*/
/**
 * This routine prepares the adaptive
 * matcher for the start of the second pass.  Further
 * learning is disabled.
 *
 * Globals:
 * - #EnableLearning set to FALSE by this routine
 *
 * @note Exceptions: none
 * @note History: Mon Apr 15 16:39:29 1991, DSJ, Created.
 */
void Classify::SettupPass2() {
  EnableLearning = FALSE;
  getDict().SettupStopperPass2();

}                                /* SettupPass2 */


/*---------------------------------------------------------------------------*/
/**
 * This routine creates a new adapted
 * class and uses Blob as the model for the first
 * config in that class.
 *
 * @param Blob blob to model new class after
 * @param LineStats statistics for text row blob is in
 * @param ClassId id of the class to be initialized
 * @param Class adapted class to be initialized
 * @param Templates adapted templates to add new class to
 *
 * Globals:
 * - #AllProtosOn dummy mask with all 1's
 * - BaselineCutoffs kludge needed to get cutoffs
 * - #PreTrainedTemplates kludge needed to get cutoffs
 *
 * @note Exceptions: none
 * @note History: Thu Mar 14 12:49:39 1991, DSJ, Created.
 */
void Classify::InitAdaptedClass(TBLOB *Blob,
                                LINE_STATS *LineStats,
                                CLASS_ID ClassId,
                                ADAPT_CLASS Class,
                                ADAPT_TEMPLATES Templates) {
  FEATURE_SET Features;
  int Fid, Pid;
  FEATURE Feature;
  int NumFeatures;
  TEMP_PROTO TempProto;
  PROTO Proto;
  INT_CLASS IClass;
  TEMP_CONFIG Config;

  classify_norm_method.set_value(baseline);
  Features = ExtractOutlineFeatures (Blob, LineStats);
  NumFeatures = Features->NumFeatures;
  if (NumFeatures > UNLIKELY_NUM_FEAT || NumFeatures <= 0) {
    FreeFeatureSet(Features);
    return;
  }

  Config = NewTempConfig (NumFeatures - 1);
  TempConfigFor (Class, 0) = Config;

  /* this is a kludge to construct cutoffs for adapted templates */
  if (Templates == AdaptedTemplates)
    BaselineCutoffs[ClassId] = CharNormCutoffs[ClassId];

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

  if (classify_learning_debug_level >= 1) {
    cprintf ("Added new class '%s' with class id %d and %d protos.\n",
             unicharset.id_to_unichar(ClassId), ClassId, NumFeatures);
  }

  if (IsEmptyAdaptedClass(Class))
    (Templates->NumNonEmptyClasses)++;
}                                /* InitAdaptedClass */
}  // namespace tesseract


/*---------------------------------------------------------------------------*/
/**
 * This routine sets up the feature
 * extractor to extract baseline normalized
 * pico-features.
 *
 * The extracted pico-features are converted
 * to integer form and placed in IntFeatures. The
 * original floating-pt. features are returned in
 * FloatFeatures.
 *
 * Globals: none
 * @param Blob blob to extract features from
 * @param LineStats statistics about text row blob is in
 * @param[out] IntFeatures array to fill with integer features
 * @param[out] FloatFeatures place to return actual floating-pt features
 *
 * @return Number of pico-features returned (0 if
 * an error occurred)
 * @note Exceptions: none
 * @note History: Tue Mar 12 17:55:18 1991, DSJ, Created.
 */
int GetAdaptiveFeatures(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        INT_FEATURE_ARRAY IntFeatures,
                        FEATURE_SET *FloatFeatures) {
  FEATURE_SET Features;
  int NumFeatures;

  classify_norm_method.set_value(baseline);
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


/*-----------------------------------------------------------------------------
              Private Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 * Return TRUE if the specified word is
 * acceptable for adaptation.
 *
 * Globals: none
 *
 * @param Word current word
 * @param BestChoiceWord best overall choice for word with context
 * @param RawChoiceWord best choice for word without context
 *
 * @return TRUE or FALSE
 * @note Exceptions: none
 * @note History: Thu May 30 14:25:06 1991, DSJ, Created.
 */
int Classify::AdaptableWord(TWERD *Word,
                            const WERD_CHOICE &BestChoiceWord,
                            const WERD_CHOICE &RawChoiceWord) {
  int BestChoiceLength = BestChoiceWord.length();
  return (  // rules that apply in general - simplest to compute first
    BestChoiceLength > 0 &&
    BestChoiceLength == NumBlobsIn (Word) &&
    BestChoiceLength <= MAX_ADAPTABLE_WERD_SIZE && (
    (classify_enable_new_adapt_rules &&
     getDict().CurrentBestChoiceAdjustFactor() <= ADAPTABLE_WERD &&
     getDict().AlternativeChoicesWorseThan(ADAPTABLE_WERD) &&
     getDict().CurrentBestChoiceIs(BestChoiceWord)) ||
    (!classify_enable_new_adapt_rules &&  // old rules
     BestChoiceLength == RawChoiceWord.length() &&
     ((getDict().valid_word_or_number(BestChoiceWord) &&
       Context::case_ok(BestChoiceWord, getDict().getUnicharset()))))));
}

/*---------------------------------------------------------------------------*/
/**
 * @param Blob blob to add to templates for ClassId
 * @param LineStats statistics about text line blob is in
 * @param ClassId class to add blob to
 * @param Threshold minimum match rating to existing template
 *
 * Globals:
 * - AdaptedTemplates current set of adapted templates
 * - AllProtosOn dummy mask to match against all protos
 * - AllConfigsOn dummy mask to match against all configs
 *
 * @return none
 * @note Exceptions: none
 * @note History: Thu Mar 14 09:36:03 1991, DSJ, Created.
 */
  void Classify::AdaptToChar(TBLOB *Blob,
                             LINE_STATS *LineStats,
                             CLASS_ID ClassId,
                             FLOAT32 Threshold) {
  int NumFeatures;
  INT_FEATURE_ARRAY IntFeatures;
  INT_RESULT_STRUCT IntResult;
  INT_CLASS IClass;
  ADAPT_CLASS Class;
  TEMP_CONFIG TempConfig;
  FEATURE_SET FloatFeatures;
  int NewTempConfigId;

  NumCharsAdaptedTo++;
  if (!LegalClassId (ClassId))
    return;

  Class = AdaptedTemplates->Class[ClassId];
  assert(Class != NULL);
  if (IsEmptyAdaptedClass(Class)) {
    InitAdaptedClass(Blob, LineStats, ClassId, Class, AdaptedTemplates);
  }
  else {
    IClass = ClassForClassId (AdaptedTemplates->Templates, ClassId);

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
        if (classify_learning_debug_level >= 1)
          cprintf ("Found good match to perm config %d = %4.1f%%.\n",
            IntResult.Config, (1.0 - IntResult.Rating) * 100.0);
        FreeFeatureSet(FloatFeatures);
        return;
      }

      TempConfig = TempConfigFor (Class, IntResult.Config);
      IncreaseConfidence(TempConfig);
      if (classify_learning_debug_level >= 1)
        cprintf ("Increasing reliability of temp config %d to %d.\n",
          IntResult.Config, TempConfig->NumTimesSeen);

      if (TempConfigReliable (TempConfig))
        MakePermanent (AdaptedTemplates, ClassId, IntResult.Config,
          Blob, LineStats);
    }
    else {
      if (classify_learning_debug_level >= 1)
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
      if (classify_learning_debug_level >= 1) {
        IntegerMatcher (IClass, AllProtosOn, AllConfigsOn,
          NumFeatures, NumFeatures, IntFeatures, 0,
          &IntResult, NO_DEBUG);
        cprintf ("Best match to temp config %d = %4.1f%%.\n",
          IntResult.Config, (1.0 - IntResult.Rating) * 100.0);
        if (classify_learning_debug_level >= 2) {
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
#endif
    }
    FreeFeatureSet(FloatFeatures);
  }
}                                /* AdaptToChar */


/*---------------------------------------------------------------------------*/
/**
 * @param Blob blob to add to templates for ClassId
 * @param LineStats statistics about text line blob is in
 * @param ClassId class to add blob to
 * @param Threshold minimum match rating to existing template
 *
 * Globals:
 * - PreTrainedTemplates current set of built-in templates
 *
 * @note Exceptions: none
 * @note History: Thu Mar 14 09:36:03 1991, DSJ, Created.
 */
void Classify::AdaptToPunc(TBLOB *Blob,
                           LINE_STATS *LineStats,
                           CLASS_ID ClassId,
                           FLOAT32 Threshold) {
  ADAPT_RESULTS *Results = new ADAPT_RESULTS();
  int i;

  Results->Initialize();
  CharNormClassifier(Blob, LineStats, PreTrainedTemplates, Results);
  RemoveBadMatches(Results);

  if (Results->NumMatches != 1) {
    if (classify_learning_debug_level >= 1) {
      cprintf ("Rejecting punc = %s (Alternatives = ",
               unicharset.id_to_unichar(ClassId));

      for (i = 0; i < Results->NumMatches; i++)
        cprintf ("%s", unicharset.id_to_unichar(Results->Classes[i]));
      cprintf (")\n");
    }
  } else {

    #ifndef SECURE_NAMES
    if (classify_learning_debug_level >= 1)
      cprintf ("Adapting to punc = %s, thr= %g\n",
               unicharset.id_to_unichar(ClassId), Threshold);
    #endif
    AdaptToChar(Blob, LineStats, ClassId, Threshold);
  }
  delete Results;
}                                /* AdaptToPunc */


/*---------------------------------------------------------------------------*/
/**
 * This routine adds the result of a classification into
 * Results.  If the new rating is much worse than the current
 * best rating, it is not entered into results because it
 * would end up being stripped later anyway.  If the new rating
 * is better than the old rating for the class, it replaces the
 * old rating.  If this is the first rating for the class, the
 * class is added to the list of matched classes in Results.
 * If the new rating is better than the best so far, it
 * becomes the best so far.
 *
 * Globals:
 * - #matcher_bad_match_pad defines limits of an acceptable match
 *
 * @param[out] Results results to add new result to
 * @param ClassId class of new result
 * @param Rating rating of new result
 * @param ConfigId config id of new result
 *
 * @note Exceptions: none
 * @note History: Tue Mar 12 18:19:29 1991, DSJ, Created.
 */
void Classify::AddNewResult(ADAPT_RESULTS *Results,
                            CLASS_ID ClassId,
                            FLOAT32 Rating,
                            int ConfigId) {
  FLOAT32 OldRating;
  INT_CLASS_STRUCT* CharClass = NULL;

  OldRating = Results->Ratings[ClassId];
  if (Rating <= Results->BestRating + matcher_bad_match_pad && Rating < OldRating) {
    if (!unicharset.get_fragment(ClassId)) {
      Results->HasNonfragment = true;
    }
    Results->Ratings[ClassId] = Rating;
    if (ClassId != NO_CLASS)
      CharClass = ClassForClassId(PreTrainedTemplates, ClassId);
    if (CharClass != NULL)
      Results->Configs[ClassId] = ConfigId;
    else
      Results->Configs[ClassId] = ~0;

    if (Rating < Results->BestRating &&
        // Ensure that fragments do not affect best rating, class and config.
        // This is needed so that at least one non-fragmented character is
        // always present in the Results.
        // TODO(daria): verify that this helps accuracy and does not
        // hurt performance.
        !unicharset.get_fragment(ClassId)) {
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
/**
 * This routine is identical to CharNormClassifier()
 * except that it does no class pruning.  It simply matches
 * the unknown blob against the classes listed in
 * Ambiguities.
 *
 * Globals:
 * - #AllProtosOn mask that enables all protos
 * - #AllConfigsOn mask that enables all configs
 *
 * @param Blob blob to be classified
 * @param LineStats statistics for text line Blob is in
 * @param Templates built-in templates to classify against
 * @param Ambiguities array of class id's to match against
 * @param[out] Results place to put match results
 *
 * @note Exceptions: none
 * @note History: Tue Mar 12 19:40:36 1991, DSJ, Created.
 */
void Classify::AmbigClassifier(TBLOB *Blob,
                               LINE_STATS *LineStats,
                               INT_TEMPLATES Templates,
                               UNICHAR_ID *Ambiguities,
                               ADAPT_RESULTS *Results) {
  int NumFeatures;
  INT_FEATURE_ARRAY IntFeatures;
  CLASS_NORMALIZATION_ARRAY CharNormArray;
  INT_RESULT_STRUCT IntResult;
  CLASS_ID ClassId;

  AmbigClassifierCalls++;

  NumFeatures = GetCharNormFeatures (Blob, LineStats,
    Templates,
    IntFeatures, CharNormArray,
    &(Results->BlobLength));
  if (NumFeatures <= 0)
    return;

  if (matcher_debug_level >= 2)
    cprintf ("AM Matches =  ");

  while (*Ambiguities >= 0) {
    ClassId = *Ambiguities;

    SetCharNormMatch();
    IntegerMatcher (ClassForClassId (Templates, ClassId),
      AllProtosOn, AllConfigsOn,
      Results->BlobLength, NumFeatures, IntFeatures,
      CharNormArray[ClassId], &IntResult, NO_DEBUG);

    if (matcher_debug_level >= 2)
      cprintf ("%s-%-2d %2.0f  ", unicharset.id_to_unichar(ClassId),
               IntResult.Config,
               IntResult.Rating * 100.0);

    AddNewResult (Results, ClassId, IntResult.Rating, IntResult.Config);

    Ambiguities++;

    NumAmbigClassesTried++;
  }
  if (matcher_debug_level >= 2)
    cprintf ("\n");

}                                /* AmbigClassifier */

/*---------------------------------------------------------------------------*/
/// Factored-out calls to IntegerMatcher based on class pruner results.
/// Returns integer matcher results inside CLASS_PRUNER_RESULTS structure.
void Classify::MasterMatcher(INT_TEMPLATES templates,
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
    BIT_VECTOR protos = classes != NULL ? classes[class_id]->PermProtos
                                        : AllProtosOn;
    BIT_VECTOR configs = classes != NULL ? classes[class_id]->PermConfigs
                                         : AllConfigsOn;

    IntegerMatcher(ClassForClassId(templates, class_id),
                   protos, configs, final_results->BlobLength,
                   num_features, features, norm_factors[class_id],
                   &int_result, debug);
    // Compute class feature corrections.
    double miss_penalty = tessedit_class_miss_scale *
                          int_result.FeatureMisses;
    if (matcher_debug_level >= 2 || tord_display_ratings > 1) {
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
    // Add unichars ambiguous with class_id with the same rating as class_id.
    if (use_definite_ambigs_for_classifier) {
      const UnicharIdVector *definite_ambigs =
        getDict().getUnicharAmbigs().OneToOneDefiniteAmbigs(class_id);
      int ambigs_size = (definite_ambigs == NULL) ? 0 : definite_ambigs->size();
      for (int ambig = 0; ambig < ambigs_size; ++ambig) {
        UNICHAR_ID ambig_class_id = (*definite_ambigs)[ambig];
        if (matcher_debug_level >= 3) {
          tprintf("class: %d definite ambig: %d rating: old %.4f new %.4f\n",
                  class_id, ambig_class_id,
                  final_results->Ratings[ambig_class_id], int_result.Rating);
        }
        if (final_results->Ratings[ambig_class_id] < WORST_POSSIBLE_RATING) {
          // ambig_class_id was already added to final_results,
          // so just need to modify the rating.
          if (int_result.Rating < final_results->Ratings[ambig_class_id]) {
            final_results->Ratings[ambig_class_id] = int_result.Rating;
          }
        } else {
          AddNewResult(final_results, ambig_class_id,
                       int_result.Rating, int_result.Config);
        }
      }
    }
  }
  if (matcher_debug_level >= 2 || tord_display_ratings > 1)
    cprintf("\n");
}
}  // namespace tesseract

/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 * This routine extracts baseline normalized features
 * from the unknown character and matches them against the
 * specified set of templates.  The classes which match
 * are added to Results.
 *
 * Globals:
 * - BaselineCutoffs expected num features for each class
 *
 * @param Blob blob to be classified
 * @param LineStats statistics for text line Blob is in
 * @param Templates current set of adapted templates
 * @param Results place to put match results
 *
 * @return Array of possible ambiguous chars that should be checked.
 * @note Exceptions: none
 * @note History: Tue Mar 12 19:38:03 1991, DSJ, Created.
 */
UNICHAR_ID *Classify::BaselineClassifier(TBLOB *Blob,
                                         LINE_STATS *LineStats,
                                         ADAPT_TEMPLATES Templates,
                                         ADAPT_RESULTS *Results) {
  int NumFeatures;
  int NumClasses;
  INT_FEATURE_ARRAY IntFeatures;
  CLASS_NORMALIZATION_ARRAY CharNormArray;
  CLASS_ID ClassId;

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
    matcher_debug_flags);

  NumBaselineClassesTried += NumClasses;

  if (matcher_debug_level >= 2 || tord_display_ratings > 1)
    cprintf ("BL Matches =  ");

  SetBaseLineMatch();
  MasterMatcher(Templates->Templates, NumFeatures, IntFeatures, CharNormArray,
                Templates->Class, matcher_debug_flags, NumClasses,
                Results->CPResults, Results);

  ClassId = Results->BestClass;
  if (ClassId == NO_CLASS)
    return (NULL);
  /* this is a bug - maybe should return "" */

  return (Templates->Class[ClassId]->Config[Results->BestConfig].Perm);
}                                /* BaselineClassifier */


/*---------------------------------------------------------------------------*/
/**
 * This routine extracts character normalized features
 * from the unknown character and matches them against the
 * specified set of templates.  The classes which match
 * are added to Results.
 *
 * @param Blob blob to be classified
 * @param LineStats statistics for text line Blob is in
 * @param Templates templates to classify unknown against
 * @param Results place to put match results
 *
 * Globals:
 * - CharNormCutoffs expected num features for each class
 * - AllProtosOn mask that enables all protos
 * - AllConfigsOn mask that enables all configs
 *
 * @note Exceptions: none
 * @note History: Tue Mar 12 16:02:52 1991, DSJ, Created.
 */
int Classify::CharNormClassifier(TBLOB *Blob,
                                 LINE_STATS *LineStats,
                                 INT_TEMPLATES Templates,
                                 ADAPT_RESULTS *Results) {
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
    return 0;

  NumClasses = ClassPruner(Templates, NumFeatures,
                           IntFeatures, CharNormArray,
                           CharNormCutoffs, Results->CPResults,
                           matcher_debug_flags);

  if (tessedit_single_match && NumClasses > 1)
    NumClasses = 1;
  NumCharNormClassesTried += NumClasses;

  SetCharNormMatch();
  MasterMatcher(Templates, NumFeatures, IntFeatures, CharNormArray,
                NULL, matcher_debug_flags, NumClasses,
                Results->CPResults, Results);
  return NumFeatures;
}                                /* CharNormClassifier */


/*---------------------------------------------------------------------------*/
/**
 * This routine computes a rating which reflects the
 * likelihood that the blob being classified is a noise
 * blob.  NOTE: assumes that the blob length has already been
 * computed and placed into Results.
 *
 * @param Results results to add noise classification to
 *
 * Globals:
 * - matcher_avg_noise_size avg. length of a noise blob
 *
 * @note Exceptions: none
 * @note History: Tue Mar 12 18:36:52 1991, DSJ, Created.
 */
void Classify::ClassifyAsNoise(ADAPT_RESULTS *Results) {
  register FLOAT32 Rating;

  Rating = Results->BlobLength / matcher_avg_noise_size;
  Rating *= Rating;
  Rating /= 1.0 + Rating;

  AddNewResult (Results, NO_CLASS, Rating, 0);
}                                /* ClassifyAsNoise */
}  // namespace tesseract


/*---------------------------------------------------------------------------*/
/**
 * This routine gets the ratings for the 2 specified classes
 * from a global variable (CurrentRatings) and returns:
 * - -1 if Rating1 < Rating2
 * - 0 if Rating1 = Rating2
 * - 1 if Rating1 > Rating2
 *
 * @param arg1
 * @param arg2 classes whose ratings are to be compared
 * 
 * Globals:
 * - CurrentRatings contains actual ratings for each class
 * 
 * @return Order of classes based on their ratings (see above).
 * @note Exceptions: none
 * @note History: Tue Mar 12 14:18:31 1991, DSJ, Created.
 */
int CompareCurrentRatings(const void *arg1,
                          const void *arg2) {
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
namespace tesseract {
/// The function converts the given match ratings to the list of blob
/// choices with ratings and certainties (used by the context checkers).
/// If character fragments are present in the results, this function also makes
/// sure that there is at least one non-fragmented classification included.
/// For each classification result check the unicharset for "definite"
/// ambiguities and modify the resulting Choices accordingly.
void Classify::ConvertMatchesToChoices(ADAPT_RESULTS *Results,
                                       BLOB_CHOICE_LIST *Choices) {
  assert(Choices != NULL);
  int i;
  CLASS_ID NextMatch;
  FLOAT32 Rating;
  FLOAT32 Certainty;
  BLOB_CHOICE_IT temp_it;
  bool contains_nonfrag = false;
  temp_it.set_to_list(Choices);
  int choices_length = 0;
  for (i = 0; i < Results->NumMatches; i++) {
    NextMatch = Results->Classes[i];
    bool current_is_frag = (unicharset.get_fragment(NextMatch) != NULL);
    if (temp_it.length()+1 == MAX_MATCHES &&
        !contains_nonfrag && current_is_frag) {
      continue;  // look for a non-fragmented character to fill the
                 // last spot in Choices if only fragments are present
    }
    // BlobLength can never be legally 0, this means recognition failed.
    // But we must return a classification result because some invoking
    // functions (chopper/permuter) do not anticipate a null blob choice.
    // So we need to assign a poor, but not infinitely bad score.
    if (Results->BlobLength == 0) {
      Certainty = -20;
      Rating = 100;    // should be -certainty * real_blob_length
    } else {
      Rating = Certainty = Results->Ratings[NextMatch];
      Rating *= rating_scale * Results->BlobLength;
      Certainty *= -certainty_scale;
    }
    temp_it.add_to_end(new BLOB_CHOICE(NextMatch, Rating, Certainty,
                                       Results->Configs[NextMatch],
                                       unicharset.get_script(NextMatch)));
    contains_nonfrag |= !current_is_frag;  // update contains_nonfrag
    choices_length++;
    if (choices_length >= MAX_MATCHES) break;
  }
  Results->NumMatches = choices_length;
}  // ConvertMatchesToChoices


/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
/**
 *
 * @param Blob blob whose classification is being debugged
 * @param LineStats statistics for text line blob is in
 * @param Results results of match being debugged
 *
 * Globals: none
 *
 * @note Exceptions: none
 * @note History: Wed Mar 13 16:44:41 1991, DSJ, Created.
 */
void Classify::DebugAdaptiveClassifier(TBLOB *Blob,
                                       LINE_STATS *LineStats,
                                       ADAPT_RESULTS *Results) {
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
/**
 * This routine performs an adaptive classification.
 * If we have not yet adapted to enough classes, a simple
 * classification to the pre-trained templates is performed.
 * Otherwise, we match the blob against the adapted templates.
 * If the adapted templates do not match well, we try a
 * match against the pre-trained templates.  If an adapted
 * template match is found, we do a match to any pre-trained
 * templates which could be ambiguous.  The results from all
 * of these classifications are merged together into Results.
 *
 * @param Blob blob to be classified
 * @param LineStats statistics for text line Blob is in
 * @param Results place to put match results
 *
 * Globals:
 * - PreTrainedTemplates built-in training templates
 * - AdaptedTemplates templates adapted for this page
 * - matcher_great_threshold rating limit for a great match
 *
 * @note Exceptions: none
 * @note History: Tue Mar 12 08:50:11 1991, DSJ, Created.
 */
void Classify::DoAdaptiveMatch(TBLOB *Blob,
                     LINE_STATS *LineStats,
                     ADAPT_RESULTS *Results) {
  UNICHAR_ID *Ambiguities;

  AdaptiveMatcherCalls++;
  InitIntFX();

  if (AdaptedTemplates->NumPermClasses < matcher_permanent_classes_min
      || tess_cn_matching) {
    CharNormClassifier(Blob, LineStats, PreTrainedTemplates, Results);
  }
  else {
    Ambiguities = BaselineClassifier(Blob, LineStats,
                                     AdaptedTemplates, Results);
    if ((Results->NumMatches > 0 && MarginalMatch (Results->BestRating)
         && !tess_bn_matching) || Results->NumMatches == 0) {
      CharNormClassifier(Blob, LineStats, PreTrainedTemplates, Results);
    } else if (Ambiguities && *Ambiguities >= 0) {
      AmbigClassifier(Blob,
                      LineStats,
                      PreTrainedTemplates,
                      Ambiguities,
                      Results);
    }
  }

  // Force the blob to be classified as noise
  // if the results contain only fragments.
  // TODO(daria): verify that this is better than
  // just adding a NULL classificaiton.
  if (!Results->HasNonfragment) {
    Results->NumMatches = 0;
  }
  if (Results->NumMatches == 0)
    ClassifyAsNoise(Results);
}   /* DoAdaptiveMatch */

/*---------------------------------------------------------------------------*/
/**
 * This routine tries to estimate how tight the adaptation
 * threshold should be set for each character in the current
 * word.  In general, the routine tries to set tighter
 * thresholds for a character when the current set of templates
 * would have made an error on that character.  It tries
 * to set a threshold tight enough to eliminate the error.
 * Two different sets of rules can be used to determine the
 * desired thresholds.
 *
 * @param Word current word
 * @param LineStats line stats for row word is in
 * @param BestChoice best choice for current word with context
 * @param BestRawChoice best choice for current word without context
 * @param[out] Thresholds array of thresholds to be filled in
 *
 * Globals:
 * - classify_enable_new_adapt_rules
 * - matcher_good_threshold
 * - matcher_perfect_threshold
 * - matcher_rating_margin
 *
 * @return none (results are returned in Thresholds)
 * @note Exceptions: none
 * @note History: Fri May 31 09:22:08 1991, DSJ, Created.
 */
void
Classify::GetAdaptThresholds (TWERD * Word,
                              LINE_STATS * LineStats,
                              const WERD_CHOICE& BestChoice,
                              const WERD_CHOICE& BestRawChoice,
                              FLOAT32 Thresholds[]) {
  TBLOB *Blob;
  const char* BestChoice_string = BestChoice.unichar_string().string();
  const char* BestChoice_lengths = BestChoice.unichar_lengths().string();
  const char* BestRawChoice_string = BestRawChoice.unichar_string().string();
  const char* BestRawChoice_lengths = BestRawChoice.unichar_lengths().string();

  if (classify_enable_new_adapt_rules &&   /* new rules */
      getDict().CurrentBestChoiceIs(BestChoice)) {
    getDict().FindClassifierErrors(matcher_perfect_threshold,
                                   matcher_good_threshold,
                                   matcher_rating_margin,
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
        *Thresholds = matcher_good_threshold;
      else {
        /* the blob was incorrectly classified - find the rating threshold
           needed to create a template which will correct the error with
           some margin.  However, don't waste time trying to make
           templates which are too tight. */
        *Thresholds = GetBestRatingFor (Blob, LineStats,
                                        unicharset.unichar_to_id(
                                            BestChoice_string,
                                            *BestChoice_lengths));
        *Thresholds *= (1.0 - matcher_rating_margin);
        if (*Thresholds > matcher_good_threshold)
          *Thresholds = matcher_good_threshold;
        if (*Thresholds < matcher_perfect_threshold)
          *Thresholds = matcher_perfect_threshold;
      }
  }
}                              /* GetAdaptThresholds */

/*---------------------------------------------------------------------------*/
/**
 * This routine matches blob to the built-in templates
 * to find out if there are any classes other than the correct
 * class which are potential ambiguities.
 *
 * @param Blob blob to get classification ambiguities for
 * @param LineStats statistics for text line blob is in
 * @param CorrectClass correct class for Blob
 *
 * Globals:
 * - CurrentRatings used by qsort compare routine
 * - PreTrainedTemplates built-in templates
 *
 * @return String containing all possible ambiguous classes.
 * @note Exceptions: none
 * @note History: Fri Mar 15 08:08:22 1991, DSJ, Created.
 */
UNICHAR_ID *Classify::GetAmbiguities(TBLOB *Blob,
                                     LINE_STATS *LineStats,
                                     CLASS_ID CorrectClass) {
  ADAPT_RESULTS *Results = new ADAPT_RESULTS();
  UNICHAR_ID *Ambiguities;
  int i;

  EnterClassifyMode;

  Results->Initialize();

  CharNormClassifier(Blob, LineStats, PreTrainedTemplates, Results);
  RemoveBadMatches(Results);

  /* save ratings in a global so that CompareCurrentRatings() can see them */
  CurrentRatings = Results->Ratings;
  qsort ((void *) (Results->Classes), Results->NumMatches,
         sizeof (CLASS_ID), CompareCurrentRatings);

  /* copy the class id's into an string of ambiguities - don't copy if
     the correct class is the only class id matched */
  Ambiguities = (UNICHAR_ID *) Emalloc (sizeof (UNICHAR_ID) *
                                        (Results->NumMatches + 1));
  if (Results->NumMatches > 1 ||
      (Results->NumMatches == 1 && Results->Classes[0] != CorrectClass)) {
    for (i = 0; i < Results->NumMatches; i++)
      Ambiguities[i] = Results->Classes[i];
    Ambiguities[i] = -1;
  }
  else
    Ambiguities[0] = -1;

  delete Results;
  return (Ambiguities);
}                              /* GetAmbiguities */

/*---------------------------------------------------------------------------*/
/**
 * This routine sets up the feature extractor to extract
 * baseline normalized pico-features.
 *
 * The extracted pico-features are converted
 * to integer form and placed in IntFeatures.  CharNormArray
 * is filled with 0's to indicate to the matcher that no
 * character normalization adjustment needs to be done.
 *
 * The total length of all blob outlines
 * in baseline normalized units is also returned.
 *
 * @param Blob blob to extract features from
 * @param LineStats statistics about text row blob is in
 * @param Templates used to compute char norm adjustments
 * @param IntFeatures array to fill with integer features
 * @param CharNormArray array to fill with dummy char norm adjustments
 * @param BlobLength length of blob in baseline-normalized units
 *
 * Globals: none
 *
 * @return Number of pico-features returned (0 if an error occurred)
 * @note Exceptions: none
 * @note History: Tue Mar 12 17:55:18 1991, DSJ, Created.
 */
int GetBaselineFeatures(TBLOB *Blob,
                        LINE_STATS *LineStats,
                        INT_TEMPLATES Templates,
                        INT_FEATURE_ARRAY IntFeatures,
                        CLASS_NORMALIZATION_ARRAY CharNormArray,
                        inT32 *BlobLength) {
  FEATURE_SET Features;
  int NumFeatures;

  if (classify_enable_int_fx)
    return (GetIntBaselineFeatures (Blob, LineStats, Templates,
                                    IntFeatures, CharNormArray, BlobLength));

  classify_norm_method.set_value(baseline);
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
  return NumFeatures;
}                              /* GetBaselineFeatures */

/**
 * This routine classifies Blob against both sets of
 * templates for the specified class and returns the best
 * rating found.
 *
 * @param Blob blob to get best rating for
 * @param LineStats statistics about text line blob is in
 * @param ClassId class blob is to be compared to
 *
 * Globals:
 * - PreTrainedTemplates built-in templates
 * - AdaptedTemplates current set of adapted templates
 * - AllProtosOn dummy mask to enable all protos
 * - AllConfigsOn dummy mask to enable all configs
 *
 * @return Best rating for match of Blob to ClassId.
 * @note Exceptions: none
 * @note History: Tue Apr  9 09:01:24 1991, DSJ, Created.
 */
FLOAT32 Classify::GetBestRatingFor(TBLOB *Blob,
                                   LINE_STATS *LineStats,
                                   CLASS_ID ClassId) {
  int NumCNFeatures, NumBLFeatures;
  INT_FEATURE_ARRAY CNFeatures, BLFeatures;
  INT_RESULT_STRUCT CNResult, BLResult;
  inT32 BlobLength;

  CNResult.Rating = BLResult.Rating = 1.0;

  if (!LegalClassId(ClassId))
    return (1.0);

  uinT8 *CNAdjust = new uinT8[MAX_NUM_CLASSES];
  uinT8 *BLAdjust = new uinT8[MAX_NUM_CLASSES];

  if (!UnusedClassIdIn(PreTrainedTemplates, ClassId)) {
    NumCNFeatures = GetCharNormFeatures(Blob, LineStats,
                                        PreTrainedTemplates,
                                        CNFeatures, CNAdjust, &BlobLength);
    if (NumCNFeatures > 0) {
      SetCharNormMatch();
      IntegerMatcher(ClassForClassId(PreTrainedTemplates, ClassId),
                     AllProtosOn, AllConfigsOn,
                     BlobLength, NumCNFeatures, CNFeatures,
                     CNAdjust[ClassId], &CNResult, NO_DEBUG);
    }
  }

  if (!UnusedClassIdIn(AdaptedTemplates->Templates, ClassId)) {
    NumBLFeatures = GetBaselineFeatures(Blob, LineStats,
                                        AdaptedTemplates->Templates,
                                        BLFeatures, BLAdjust, &BlobLength);
    if (NumBLFeatures > 0) {
      SetBaseLineMatch();
      IntegerMatcher(ClassForClassId(AdaptedTemplates->Templates, ClassId),
                     AdaptedTemplates->Class[ClassId]->PermProtos,
                     AdaptedTemplates->Class[ClassId]->PermConfigs,
                     BlobLength, NumBLFeatures, BLFeatures,
                     BLAdjust[ClassId], &BLResult, NO_DEBUG);
    }
  }

  // Clean up.
  delete[] CNAdjust;
  delete[] BLAdjust;

  return (MIN (BLResult.Rating, CNResult.Rating));
}                              /* GetBestRatingFor */

/*---------------------------------------------------------------------------*/
/**
 * This routine sets up the feature extractor to extract
 * character normalization features and character normalized
 * pico-features.  The extracted pico-features are converted
 * to integer form and placed in IntFeatures.  The character
 * normalization features are matched to each class in
 * templates and the resulting adjustment factors are returned
 * in CharNormArray.  The total length of all blob outlines
 * in baseline normalized units is also returned.
 *
 * @param Blob blob to extract features from
 * @param LineStats statistics about text row blob is in
 * @param Templates used to compute char norm adjustments
 * @param IntFeatures array to fill with integer features
 * @param CharNormArray array to fill with char norm adjustments
 * @param BlobLength length of blob in baseline-normalized units
 *
 * Globals: none
 *
 * @return Number of pico-features returned (0 if an error occurred)
 * @note Exceptions: none
 * @note History: Tue Mar 12 17:55:18 1991, DSJ, Created.
 */
int Classify::GetCharNormFeatures(TBLOB *Blob,
                                  LINE_STATS *LineStats,
                                  INT_TEMPLATES Templates,
                                  INT_FEATURE_ARRAY IntFeatures,
                                  CLASS_NORMALIZATION_ARRAY CharNormArray,
                                  inT32 *BlobLength) {
  return (GetIntCharNormFeatures (Blob, LineStats, Templates,
                                  IntFeatures, CharNormArray, BlobLength));
}                              /* GetCharNormFeatures */

/*---------------------------------------------------------------------------*/
/**
 * This routine calls the integer (Hardware) feature
 * extractor if it has not been called before for this blob.
 * The results from the feature extractor are placed into
 * globals so that they can be used in other routines without
 * re-extracting the features.
 * It then copies the baseline features into the IntFeatures
 * array provided by the caller.
 *
 * @param Blob blob to extract features from
 * @param LineStats statistics about text row blob is in
 * @param Templates used to compute char norm adjustments
 * @param IntFeatures array to fill with integer features
 * @param CharNormArray array to fill with dummy char norm adjustments
 * @param BlobLength length of blob in baseline-normalized units
 *
 * Globals:
 * - FeaturesHaveBeenExtracted TRUE if fx has been done
 * - BaselineFeatures holds extracted baseline feat
 * - CharNormFeatures holds extracted char norm feat
 * - FXInfo holds misc. FX info
 *
 * @return Number of features extracted or 0 if an error occured.
 * @note Exceptions: none
 * @note History: Tue May 28 10:40:52 1991, DSJ, Created.
 */
int GetIntBaselineFeatures(TBLOB *Blob,
                           LINE_STATS *LineStats,
                           INT_TEMPLATES Templates,
                           INT_FEATURE_ARRAY IntFeatures,
                           CLASS_NORMALIZATION_ARRAY CharNormArray,
                           inT32 *BlobLength) {
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
       Src < End;
       *Dest++ = *Src++);

  ClearCharNormArray(Templates, CharNormArray);
  *BlobLength = FXInfo.NumBL;
  return (FXInfo.NumBL);
}                              /* GetIntBaselineFeatures */

/*---------------------------------------------------------------------------*/
/**
 * This routine calls the integer (Hardware) feature
 * extractor if it has not been called before for this blob.
 *
 * The results from the feature extractor are placed into
 * globals so that they can be used in other routines without
 * re-extracting the features.
 *
 * It then copies the char norm features into the IntFeatures
 * array provided by the caller.
 *
 * @param Blob blob to extract features from
 * @param LineStats statistics about text row blob is in
 * @param Templates used to compute char norm adjustments
 * @param IntFeatures array to fill with integer features
 * @param CharNormArray array to fill with dummy char norm adjustments
 * @param BlobLength length of blob in baseline-normalized units
 *
 * Globals:
 * - FeaturesHaveBeenExtracted TRUE if fx has been done
 * - BaselineFeatures holds extracted baseline feat
 * - CharNormFeatures holds extracted char norm feat
 * - FXInfo holds misc. FX info
 *
 * @return Number of features extracted or 0 if an error occured.
 * @note Exceptions: none
 * @note History: Tue May 28 10:40:52 1991, DSJ, Created.
 */
int Classify::GetIntCharNormFeatures(TBLOB *Blob,
                                     LINE_STATS *LineStats,
                                     INT_TEMPLATES Templates,
                                     INT_FEATURE_ARRAY IntFeatures,
                                     CLASS_NORMALIZATION_ARRAY CharNormArray,
                                     inT32 *BlobLength) {
  register INT_FEATURE Src, Dest, End;
  FEATURE NormFeature;
  FLOAT32 Baseline, Scale;

  if (!FeaturesHaveBeenExtracted) {
    FeaturesOK = ExtractIntFeat(Blob, BaselineFeatures,
                                CharNormFeatures, &FXInfo);
    FeaturesHaveBeenExtracted = TRUE;
  }

  if (!FeaturesOK) {
    *BlobLength = FXInfo.NumBL;
    return (0);
  }

  for (Src = CharNormFeatures, End = Src + FXInfo.NumCN, Dest = IntFeatures;
       Src < End;
       *Dest++ = *Src++);

  NormFeature = NewFeature(&CharNormDesc);
  Baseline = BaselineAt(LineStats, FXInfo.Xmean);
  Scale = ComputeScaleFactor(LineStats);
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
/**
 *
 * @param Templates adapted templates to add new config to
 * @param ClassId class id to associate with new config
 * @param NumFeatures number of features in IntFeatures
 * @param Features features describing model for new config
 * @param FloatFeatures floating-pt representation of features
 *
 * Globals:
 * - AllProtosOn mask to enable all protos
 * - AllConfigsOff mask to disable all configs
 * - TempProtoMask defines old protos matched in new config
 * 
 * @return The id of the new config created, a negative integer in
 * case of error.
 * @note Exceptions: none
 * @note History: Fri Mar 15 08:49:46 1991, DSJ, Created.
 */
int Classify::MakeNewTemporaryConfig(ADAPT_TEMPLATES Templates,
                                     CLASS_ID ClassId,
                                     int NumFeatures,
                                     INT_FEATURE_ARRAY Features,
                                     FEATURE_SET FloatFeatures) {
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

  if (classify_learning_debug_level >= 3)
    debug_level =
        PRINT_MATCH_SUMMARY | PRINT_FEATURE_MATCHES | PRINT_PROTO_MATCHES;

  IClass = ClassForClassId(Templates->Templates, ClassId);
  Class = Templates->Class[ClassId];

  if (IClass->NumConfigs >= MAX_NUM_CONFIGS) {
    ++NumAdaptationsFailed;
    if (classify_learning_debug_level >= 1)
      cprintf("Cannot make new temporary config: maximum number exceeded.\n");
    return -1;
  }

  OldMaxProtoId = IClass->NumProtos - 1;

  NumOldProtos = FindGoodProtos(IClass, AllProtosOn, AllConfigsOff,
                                BlobLength, NumFeatures, Features,
                                OldProtos, debug_level);

  MaskSize = WordsInVectorOfSize(MAX_NUM_PROTOS);
  zero_all_bits(TempProtoMask, MaskSize);
  for (i = 0; i < NumOldProtos; i++)
    SET_BIT(TempProtoMask, OldProtos[i]);

  NumBadFeatures = FindBadFeatures(IClass, TempProtoMask, AllConfigsOn,
                                   BlobLength, NumFeatures, Features,
                                   BadFeatures, debug_level);

  MaxProtoId = MakeNewTempProtos(FloatFeatures, NumBadFeatures, BadFeatures,
                                 IClass, Class, TempProtoMask);
  if (MaxProtoId == NO_PROTO) {
    ++NumAdaptationsFailed;
    if (classify_learning_debug_level >= 1)
      cprintf("Cannot make new temp protos: maximum number exceeded.\n");
    return -1;
  }

  ConfigId = AddIntConfig(IClass);
  ConvertConfig(TempProtoMask, ConfigId, IClass);
  Config = NewTempConfig(MaxProtoId);
  TempConfigFor(Class, ConfigId) = Config;
  copy_all_bits(TempProtoMask, Config->Protos, Config->ProtoVectorSize);

  if (classify_learning_debug_level >= 1)
    cprintf("Making new temp config %d using %d old and %d new protos.\n",
             ConfigId, NumOldProtos, MaxProtoId - OldMaxProtoId);

  return ConfigId;
}                              /* MakeNewTemporaryConfig */
}  // namespace tesseract

/*---------------------------------------------------------------------------*/
/**
 * This routine finds sets of sequential bad features
 * that all have the same angle and converts each set into
 * a new temporary proto.  The temp proto is added to the
 * proto pruner for IClass, pushed onto the list of temp
 * protos in Class, and added to TempProtoMask.
 *
 * @param Features floating-pt features describing new character
 * @param NumBadFeat number of bad features to turn into protos
 * @param BadFeat feature id's of bad features
 * @param IClass integer class templates to add new protos to
 * @param Class adapted class templates to add new protos to
 * @param TempProtoMask proto mask to add new protos to
 *
 * Globals: none
 *
 * @return Max proto id in class after all protos have been added.
 * Exceptions: none
 * History: Fri Mar 15 11:39:38 1991, DSJ, Created.
 */
PROTO_ID
MakeNewTempProtos(FEATURE_SET Features,
                  int NumBadFeat,
                  FEATURE_ID BadFeat[],
                  INT_CLASS IClass,
                  ADAPT_CLASS Class, BIT_VECTOR TempProtoMask) {
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
         SegmentLength = GetPicoFeatureLength();
         ProtoEnd < LastBad;
         ProtoEnd++, SegmentLength += GetPicoFeatureLength()) {
      F2 = Features->Features[*ProtoEnd];
      X2 = F2->Params[PicoFeatX];
      Y2 = F2->Params[PicoFeatY];
      A2 = F2->Params[PicoFeatDir];

      AngleDelta = fabs(A1 - A2);
      if (AngleDelta > 0.5)
        AngleDelta = 1.0 - AngleDelta;

      if (AngleDelta > matcher_clustering_max_angle_delta ||
          fabs(X1 - X2) > SegmentLength ||
          fabs(Y1 - Y2) > SegmentLength)
        break;
    }

    F2 = Features->Features[*(ProtoEnd - 1)];
    X2 = F2->Params[PicoFeatX];
    Y2 = F2->Params[PicoFeatY];
    A2 = F2->Params[PicoFeatDir];

    Pid = AddIntProto(IClass);
    if (Pid == NO_PROTO)
      return (NO_PROTO);

    TempProto = NewTempProto();
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

    Class->TempProtos = push(Class->TempProtos, TempProto);
  }
  return IClass->NumProtos - 1;
}                              /* MakeNewTempProtos */

/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 *
 * @param Templates current set of adaptive templates
 * @param ClassId class containing config to be made permanent
 * @param ConfigId config to be made permanent
 * @param Blob current blob being adapted to
 * @param LineStats statistics about text line Blob is in
 * 
 * Globals: none
 *
 * @note Exceptions: none
 * @note History: Thu Mar 14 15:54:08 1991, DSJ, Created.
 */
void Classify::MakePermanent(ADAPT_TEMPLATES Templates,
                             CLASS_ID ClassId,
                             int ConfigId,
                             TBLOB *Blob,
                             LINE_STATS *LineStats) {
  UNICHAR_ID *Ambigs;
  TEMP_CONFIG Config;
  ADAPT_CLASS Class;
  PROTO_KEY ProtoKey;

  Class = Templates->Class[ClassId];
  Config = TempConfigFor(Class, ConfigId);

  MakeConfigPermanent(Class, ConfigId);
  if (Class->NumPermConfigs == 0)
    Templates->NumPermClasses++;
  Class->NumPermConfigs++;

  ProtoKey.Templates = Templates;
  ProtoKey.ClassId = ClassId;
  ProtoKey.ConfigId = ConfigId;
  Class->TempProtos = delete_d(Class->TempProtos, &ProtoKey,
                               MakeTempProtoPerm);
  FreeTempConfig(Config);

  Ambigs = GetAmbiguities(Blob, LineStats, ClassId);
  PermConfigFor(Class, ConfigId) = Ambigs;

  if (classify_learning_debug_level >= 1) {
    cprintf("Making config %d permanent with ambiguities '",
            ConfigId, Ambigs);
    for (UNICHAR_ID *AmbigsPointer = Ambigs;
         *AmbigsPointer >= 0; ++AmbigsPointer)
      cprintf("%s", unicharset.id_to_unichar(*AmbigsPointer));
    cprintf("'.\n");
  }
}                              /* MakePermanent */
}  // namespace tesseract

/*---------------------------------------------------------------------------*/
/**
 * This routine converts TempProto to be permanent if
 * its proto id is used by the configuration specified in
 * ProtoKey.
 * 
 * @param TempProto temporary proto to compare to key
 * @param ProtoKey defines which protos to make permanent
 * 
 * Globals: none
 * 
 * @return TRUE if TempProto is converted, FALSE otherwise
 * @note Exceptions: none
 * @note History: Thu Mar 14 18:49:54 1991, DSJ, Created.
 */
int MakeTempProtoPerm(void *item1, void *item2) {
  ADAPT_CLASS Class;
  TEMP_CONFIG Config;
  TEMP_PROTO TempProto;
  PROTO_KEY *ProtoKey;

  TempProto = (TEMP_PROTO) item1;
  ProtoKey = (PROTO_KEY *) item2;

  Class = ProtoKey->Templates->Class[ProtoKey->ClassId];
  Config = TempConfigFor(Class, ProtoKey->ConfigId);

  if (TempProto->ProtoId > Config->MaxProtoId ||
      !test_bit (Config->Protos, TempProto->ProtoId))
    return FALSE;

  MakeProtoPermanent(Class, TempProto->ProtoId);
  AddProtoToClassPruner(&(TempProto->Proto), ProtoKey->ClassId,
                         ProtoKey->Templates->Templates);
  FreeTempProto(TempProto);

  return TRUE;
}                              /* MakeTempProtoPerm */

/*---------------------------------------------------------------------------*/
/**
 * This routine returns the number of blobs in Word.
 *
 * @param Word word to count blobs in
 *
 * Globals: none
 *
 * @return Number of blobs in Word.
 * @note Exceptions: none
 * @note History: Thu Mar 14 08:30:27 1991, DSJ, Created.
 */
int NumBlobsIn(TWERD *Word) {
  register TBLOB *Blob;
  register int NumBlobs;

  if (Word == NULL)
    return (0);

  for (Blob = Word->blobs, NumBlobs = 0;
       Blob != NULL; Blob = Blob->next, NumBlobs++);

  return (NumBlobs);

}                              /* NumBlobsIn */

/*---------------------------------------------------------------------------*/
/**
 * This routine returns the number of OUTER outlines
 * in Blob.
 *
 * @param Blob blob to count outlines in
 *
 * Globals: none
 * @return Number of outer outlines in Blob.
 * @note Exceptions: none
 * @note History: Mon Jun 10 15:46:20 1991, DSJ, Created.
 */
int NumOutlinesInBlob(TBLOB *Blob) {
  register TESSLINE *Outline;
  register int NumOutlines;

  if (Blob == NULL)
    return (0);

  for (Outline = Blob->outlines, NumOutlines = 0;
       Outline != NULL; Outline = Outline->next, NumOutlines++);

  return (NumOutlines);

}                              /* NumOutlinesInBlob */

/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 * This routine writes the matches in Results to File.
 *
 * @param File open text file to write Results to
 * @param Results match results to write to File
 *
 * Globals: none
 *
 * @note Exceptions: none
 * @note History: Mon Mar 18 09:24:53 1991, DSJ, Created.
 */
void Classify::PrintAdaptiveMatchResults(FILE *File, ADAPT_RESULTS *Results) {
  for (int i = 0; i < Results->NumMatches; ++i) {
    cprintf("%s(%d) %.2f  ",
            unicharset.debug_str(Results->Classes[i]).string(),
            Results->Classes[i],
            Results->Ratings[Results->Classes[i]] * 100.0);
  }
  printf("\n");
}                              /* PrintAdaptiveMatchResults */

/*---------------------------------------------------------------------------*/
/**
 * This routine steps thru each matching class in Results
 * and removes it from the match list if its rating
 * is worse than the BestRating plus a pad.  In other words,
 * all good matches get moved to the front of the classes
 * array.
 *
 * @param Results contains matches to be filtered
 *
 * Globals:
 * - matcher_bad_match_pad defines a "bad match"
 * 
 * @note Exceptions: none
 * @note History: Tue Mar 12 13:51:03 1991, DSJ, Created.
 */
void Classify::RemoveBadMatches(ADAPT_RESULTS *Results) {
  int Next, NextGood;
  FLOAT32 *Rating = Results->Ratings;
  CLASS_ID *Match = Results->Classes;
  FLOAT32 BadMatchThreshold;
  static const char* romans = "i v x I V X";
  BadMatchThreshold = Results->BestRating + matcher_bad_match_pad;

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

/*----------------------------------------------------------------------------*/
/**
 * This routine steps thru each matching class in Results
 * and removes it from the match list if its rating
 * is worse than the BestRating plus a pad.  In other words,
 * all good matches get moved to the front of the classes
 * array.
 *
 * @parm Results contains matches to be filtered
 *
 * Globals:
 * - matcher_bad_match_pad defines a "bad match"
 * 
 * @note Exceptions: none
 * @note History: Tue Mar 12 13:51:03 1991, DSJ, Created.
 */
void Classify::RemoveExtraPuncs(ADAPT_RESULTS *Results) {
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
}  // namespace tesseract

/*---------------------------------------------------------------------------*/
/**
 * This routine resets the internal thresholds inside
 * the integer matcher to correspond to the specified
 * threshold.
 *
 * @param Threshold threshold for creating new templates
 *
 * Globals:
 * - matcher_good_threshold default good match rating
 *
 * @note Exceptions: none
 * @note History: Tue Apr  9 08:33:13 1991, DSJ, Created.
 */
void SetAdaptiveThreshold(FLOAT32 Threshold) {
  if (Threshold == matcher_good_threshold) {
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
namespace tesseract {
/**
 * This routine compares Blob to both sets of templates
 * (adaptive and pre-trained) and then displays debug
 * information for the config which matched best.
 *
 * @param Blob blob to show best matching config for
 * @param LineStats statistics for text line Blob is in
 * @param ClassId class whose configs are to be searched
 * @param AdaptiveOn TRUE if adaptive configs are enabled
 * @param PreTrainedOn TRUE if pretrained configs are enabled
 *
 * Globals:
 * - PreTrainedTemplates built-in training
 * - AdaptedTemplates adaptive templates
 * - AllProtosOn dummy proto mask
 * - AllConfigsOn dummy config mask
 *
 * @note Exceptions: none
 * @note History: Fri Mar 22 08:43:52 1991, DSJ, Created.
 */
void Classify::ShowBestMatchFor(TBLOB *Blob,
                                LINE_STATS *LineStats,
                                CLASS_ID ClassId,
                                BOOL8 AdaptiveOn,
                                BOOL8 PreTrainedOn) {
  int NumCNFeatures = 0, NumBLFeatures = 0;
  INT_FEATURE_ARRAY CNFeatures, BLFeatures;
  INT_RESULT_STRUCT CNResult, BLResult;
  inT32 BlobLength;
  uinT32 ConfigMask;
  static int next_config = -1;

  if (PreTrainedOn) next_config = -1;

  CNResult.Rating = BLResult.Rating = 2.0;

  if (!LegalClassId (ClassId)) {
    cprintf ("%d is not a legal class id!!\n", ClassId);
    return;
  }

  uinT8 *CNAdjust = new uinT8[MAX_NUM_CLASSES];
  uinT8 *BLAdjust = new uinT8[MAX_NUM_CLASSES];

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
        SetCharNormMatch();
        IntegerMatcher (ClassForClassId (PreTrainedTemplates, ClassId),
                        AllProtosOn, AllConfigsOn,
                        BlobLength, NumCNFeatures, CNFeatures,
                        CNAdjust[ClassId], &CNResult, NO_DEBUG);

        cprintf ("Best built-in template match is config %2d (%4.1f) (cn=%d)\n",
                 CNResult.Config, CNResult.Rating * 100.0, CNAdjust[ClassId]);
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
        SetBaseLineMatch();
        IntegerMatcher (ClassForClassId
                        (AdaptedTemplates->Templates, ClassId),
                        AllProtosOn, AllConfigsOn,
                        // AdaptedTemplates->Class[ClassId]->PermProtos,
                        // AdaptedTemplates->Class[ClassId]->PermConfigs,
                        BlobLength, NumBLFeatures, BLFeatures,
                        BLAdjust[ClassId], &BLResult, NO_DEBUG);

#ifndef SECURE_NAMES
        ADAPT_CLASS Class = AdaptedTemplates->Class[ClassId];
        cprintf ("Best adaptive template match is config %2d (%4.1f) %s\n",
                 BLResult.Config, BLResult.Rating * 100.0,
                 ConfigIsPermanent(Class, BLResult.Config) ? "Perm" : "Temp");
#endif
      }
    }
  }

  cprintf ("\n");
  if (BLResult.Rating < CNResult.Rating) {
    if (next_config < 0) {
      ConfigMask = 1 << BLResult.Config;
      next_config = 0;
    } else {
      ConfigMask = 1 << next_config;
      ++next_config;
    }
    classify_norm_method.set_value(baseline);

    SetBaseLineMatch();
    IntegerMatcher (ClassForClassId (AdaptedTemplates->Templates, ClassId),
                    AllProtosOn,
                    //        AdaptedTemplates->Class[ClassId]->PermProtos,
                    (BIT_VECTOR) & ConfigMask,
                    BlobLength, NumBLFeatures, BLFeatures,
                    BLAdjust[ClassId], &BLResult, matcher_debug_flags);
    cprintf ("Adaptive template match for config %2d is %4.1f\n",
             BLResult.Config, BLResult.Rating * 100.0);
  }
  else {
    ConfigMask = 1 << CNResult.Config;
    classify_norm_method.set_value(character);

    SetCharNormMatch();
    //xiaofan
    IntegerMatcher (ClassForClassId (PreTrainedTemplates, ClassId), AllProtosOn, (BIT_VECTOR) & ConfigMask,
                    BlobLength, NumCNFeatures, CNFeatures,
                    CNAdjust[ClassId], &CNResult, matcher_debug_flags);
  }

  // Clean up.
  delete[] CNAdjust;
  delete[] BLAdjust;
}                              /* ShowBestMatchFor */
}  // namespace tesseract
