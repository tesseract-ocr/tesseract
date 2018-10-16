/******************************************************************************
 ** Filename:    adaptmatch.cpp
 ** Purpose:     High level adaptive matcher.
 ** Author:      Dan Johnson
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
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include <algorithm>            // for max, min
#include <cassert>              // for assert
#include <cmath>                // for fabs
#include <cstdint>              // for INT32_MAX, UINT8_MAX
#include <cstdio>               // for fflush, fclose, fopen, stdout, FILE
#include <cstdlib>              // for malloc
#include <cstring>              // for strstr, memset, strcmp
#include "adaptive.h"           // for ADAPT_CLASS, free_adapted_templates
#include "ambigs.h"             // for UnicharIdVector, UnicharAmbigs
#include "bitvec.h"             // for FreeBitVector, NewBitVector, BIT_VECTOR
#include "blobs.h"              // for TBLOB, TWERD
#include "callcpp.h"            // for cprintf, window_wait
#include "classify.h"           // for Classify, CST_FRAGMENT, CST_WHOLE
#include "dict.h"               // for Dict
#include "errcode.h"            // for ASSERT_HOST
#include "featdefs.h"           // for CharNormDesc
#include "float2int.h"          // for BASELINE_Y_SHIFT
#include "fontinfo.h"           // for ScoredFont, FontSet
#include "genericvector.h"      // for GenericVector
#include "helpers.h"            // for IntCastRounded, ClipToRange
#include "host.h"               // for FALSE, TRUE
#include "intfx.h"              // for BlobToTrainingSample, INT_FX_RESULT_S...
#include "intmatcher.h"         // for CP_RESULT_STRUCT, IntegerMatcher
#include "intproto.h"           // for INT_FEATURE_STRUCT, (anonymous), Clas...
#include "matchdefs.h"          // for CLASS_ID, FEATURE_ID, PROTO_ID, NO_PROTO
#include "mfoutline.h"          // for baseline, character, MF_SCALE_FACTOR
#include "normalis.h"           // for DENORM, kBlnBaselineOffset, kBlnXHeight
#include "normfeat.h"           // for ActualOutlineLength, CharNormLength
#include "ocrfeatures.h"        // for FEATURE_STRUCT, FreeFeatureSet, FEATURE
#include "oldlist.h"            // for push, delete_d
#include "outfeat.h"            // for OutlineFeatDir, OutlineFeatLength
#include "pageres.h"            // for WERD_RES
#include "params.h"             // for IntParam, BoolParam, DoubleParam, Str...
#include "picofeat.h"           // for PicoFeatDir, PicoFeatX, PicoFeatY
#include "protos.h"             // for PROTO_STRUCT, FillABC, PROTO
#include "ratngs.h"             // for BLOB_CHOICE_IT, BLOB_CHOICE_LIST, BLO...
#include "rect.h"               // for TBOX
#include "scrollview.h"         // for ScrollView, ScrollView::BROWN, Scroll...
#include "seam.h"               // for SEAM
#include "serialis.h"           // for TFile
#include "shapeclassifier.h"    // for ShapeClassifier
#include "shapetable.h"         // for UnicharRating, ShapeTable, Shape, Uni...
#include "strngs.h"             // for STRING
#include "tessclassifier.h"     // for TessClassifier
#include "tessdatamanager.h"    // for TessdataManager, TESSDATA_INTTEMP
#include "tprintf.h"            // for tprintf
#include "trainingsample.h"     // for TrainingSample
#include "unichar.h"            // for UNICHAR_ID, INVALID_UNICHAR_ID
#include "unicharset.h"         // for UNICHARSET, CHAR_FRAGMENT, UNICHAR_SPACE
#include "unicity_table.h"      // for UnicityTable

#define ADAPT_TEMPLATE_SUFFIX ".a"

#define MAX_MATCHES         10
#define UNLIKELY_NUM_FEAT 200
#define NO_DEBUG      0
#define MAX_ADAPTABLE_WERD_SIZE 40

#define ADAPTABLE_WERD_ADJUSTMENT    (0.05)

#define Y_DIM_OFFSET    (Y_SHIFT - BASELINE_Y_SHIFT)

#define WORST_POSSIBLE_RATING (0.0f)

using tesseract::UnicharRating;
using tesseract::ScoredFont;

struct ADAPT_RESULTS {
  int32_t BlobLength;
  bool HasNonfragment;
  UNICHAR_ID best_unichar_id;
  int best_match_index;
  float best_rating;
  GenericVector<UnicharRating> match;
  GenericVector<CP_RESULT_STRUCT> CPResults;

  /// Initializes data members to the default values. Sets the initial
  /// rating of each class to be the worst possible rating (1.0).
  inline void Initialize() {
    BlobLength = INT32_MAX;
    HasNonfragment = false;
    ComputeBest();
  }
  // Computes best_unichar_id, best_match_index and best_rating.
  void ComputeBest() {
    best_unichar_id = INVALID_UNICHAR_ID;
    best_match_index = -1;
    best_rating = WORST_POSSIBLE_RATING;
    for (int i = 0; i < match.size(); ++i) {
      if (match[i].rating > best_rating) {
        best_rating = match[i].rating;
        best_unichar_id = match[i].unichar_id;
        best_match_index = i;
      }
    }
  }
};

struct PROTO_KEY {
  ADAPT_TEMPLATES Templates;
  CLASS_ID ClassId;
  int ConfigId;
};

/*-----------------------------------------------------------------------------
          Private Macros
-----------------------------------------------------------------------------*/
inline bool MarginalMatch(float confidence, float matcher_great_threshold) {
  return (1.0f - confidence) > matcher_great_threshold;
}

/*-----------------------------------------------------------------------------
          Private Function Prototypes
-----------------------------------------------------------------------------*/
// Returns the index of the given id in results, if present, or the size of the
// vector (index it will go at) if not present.
static int FindScoredUnichar(UNICHAR_ID id, const ADAPT_RESULTS& results) {
  for (int i = 0; i < results.match.size(); i++) {
    if (results.match[i].unichar_id == id)
      return i;
  }
  return results.match.size();
}

// Returns the current rating for a unichar id if we have rated it, defaulting
// to WORST_POSSIBLE_RATING.
static float ScoredUnichar(UNICHAR_ID id, const ADAPT_RESULTS& results) {
  int index = FindScoredUnichar(id, results);
  if (index >= results.match.size()) return WORST_POSSIBLE_RATING;
  return results.match[index].rating;
}

void InitMatcherRatings(float *Rating);

int MakeTempProtoPerm(void *item1, void *item2);

void SetAdaptiveThreshold(float Threshold);


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
 * @param Blob    blob to be classified
 * @param[out] Choices    List of choices found by adaptive matcher.
 * filled on return with the choices found by the
 * class pruner and the ratings therefrom. Also
 * contains the detailed results of the integer matcher.
 *
 */
void Classify::AdaptiveClassifier(TBLOB *Blob, BLOB_CHOICE_LIST *Choices) {
  assert(Choices != nullptr);
  ADAPT_RESULTS *Results = new ADAPT_RESULTS;
  Results->Initialize();

  ASSERT_HOST(AdaptedTemplates != nullptr);

  DoAdaptiveMatch(Blob, Results);

  RemoveBadMatches(Results);
  Results->match.sort(&UnicharRating::SortDescendingRating);
  RemoveExtraPuncs(Results);
  Results->ComputeBest();
  ConvertMatchesToChoices(Blob->denorm(), Blob->bounding_box(), Results,
                          Choices);

  // TODO(rays) Move to before ConvertMatchesToChoices!
  if (LargeSpeckle(*Blob) || Choices->length() == 0)
    AddLargeSpeckleTo(Results->BlobLength, Choices);

  if (matcher_debug_level >= 1) {
    tprintf("AD Matches =  ");
    PrintAdaptiveMatchResults(*Results);
  }

#ifndef GRAPHICS_DISABLED
  if (classify_enable_adaptive_debugger)
    DebugAdaptiveClassifier(Blob, Results);
#endif

  delete Results;
}                                /* AdaptiveClassifier */

// If *win is nullptr, sets it to a new ScrollView() object with title msg.
// Clears the window and draws baselines.
void Classify::RefreshDebugWindow(ScrollView **win, const char *msg,
                                  int y_offset, const TBOX &wbox) {
  #ifndef GRAPHICS_DISABLED
  const int kSampleSpaceWidth = 500;
  if (*win == nullptr) {
    *win = new ScrollView(msg, 100, y_offset, kSampleSpaceWidth * 2, 200,
                          kSampleSpaceWidth * 2, 200, true);
  }
  (*win)->Clear();
  (*win)->Pen(64, 64, 64);
  (*win)->Line(-kSampleSpaceWidth, kBlnBaselineOffset,
               kSampleSpaceWidth, kBlnBaselineOffset);
  (*win)->Line(-kSampleSpaceWidth, kBlnXHeight + kBlnBaselineOffset,
               kSampleSpaceWidth, kBlnXHeight + kBlnBaselineOffset);
  (*win)->ZoomToRectangle(wbox.left(), wbox.top(),
                          wbox.right(), wbox.bottom());
  #endif  // GRAPHICS_DISABLED
}

// Learns the given word using its chopped_word, seam_array, denorm,
// box_word, best_state, and correct_text to learn both correctly and
// incorrectly segmented blobs. If fontname is not nullptr, then LearnBlob
// is called and the data will be saved in an internal buffer.
// Otherwise AdaptToBlob is called for adaption within a document.
void Classify::LearnWord(const char* fontname, WERD_RES* word) {
  int word_len = word->correct_text.size();
  if (word_len == 0) return;

  float* thresholds = nullptr;
  if (fontname == nullptr) {
    // Adaption mode.
    if (!EnableLearning || word->best_choice == nullptr)
      return;  // Can't or won't adapt.

    if (classify_learning_debug_level >= 1)
      tprintf("\n\nAdapting to word = %s\n",
              word->best_choice->debug_string().string());
    thresholds = new float[word_len];
    word->ComputeAdaptionThresholds(certainty_scale,
                                    matcher_perfect_threshold,
                                    matcher_good_threshold,
                                    matcher_rating_margin, thresholds);
  }
  int start_blob = 0;

  #ifndef GRAPHICS_DISABLED
  if (classify_debug_character_fragments) {
    if (learn_fragmented_word_debug_win_ != nullptr) {
      window_wait(learn_fragmented_word_debug_win_);
    }
    RefreshDebugWindow(&learn_fragments_debug_win_, "LearnPieces", 400,
                       word->chopped_word->bounding_box());
    RefreshDebugWindow(&learn_fragmented_word_debug_win_, "LearnWord", 200,
                       word->chopped_word->bounding_box());
    word->chopped_word->plot(learn_fragmented_word_debug_win_);
    ScrollView::Update();
  }
  #endif  // GRAPHICS_DISABLED

  for (int ch = 0; ch < word_len; ++ch) {
    if (classify_debug_character_fragments) {
      tprintf("\nLearning %s\n",  word->correct_text[ch].string());
    }
    if (word->correct_text[ch].length() > 0) {
      float threshold = thresholds != nullptr ? thresholds[ch] : 0.0f;

      LearnPieces(fontname, start_blob, word->best_state[ch], threshold,
                  CST_WHOLE, word->correct_text[ch].string(), word);

      if (word->best_state[ch] > 1 && !disable_character_fragments) {
        // Check that the character breaks into meaningful fragments
        // that each match a whole character with at least
        // classify_character_fragments_garbage_certainty_threshold
        bool garbage = false;
        int frag;
        for (frag = 0; frag < word->best_state[ch]; ++frag) {
          TBLOB* frag_blob = word->chopped_word->blobs[start_blob + frag];
          if (classify_character_fragments_garbage_certainty_threshold < 0) {
            garbage |= LooksLikeGarbage(frag_blob);
          }
        }
        // Learn the fragments.
        if (!garbage) {
          bool pieces_all_natural = word->PiecesAllNatural(start_blob,
              word->best_state[ch]);
          if (pieces_all_natural || !prioritize_division) {
            for (frag = 0; frag < word->best_state[ch]; ++frag) {
              GenericVector<STRING> tokens;
              word->correct_text[ch].split(' ', &tokens);

              tokens[0] = CHAR_FRAGMENT::to_string(
                  tokens[0].string(), frag, word->best_state[ch],
                  pieces_all_natural);

              STRING full_string;
              for (int i = 0; i < tokens.size(); i++) {
                full_string += tokens[i];
                if (i != tokens.size() - 1)
                  full_string += ' ';
              }
              LearnPieces(fontname, start_blob + frag, 1, threshold,
                          CST_FRAGMENT, full_string.string(), word);
            }
          }
        }
      }

      // TODO(rays): re-enable this part of the code when we switch to the
      // new classifier that needs to see examples of garbage.
      /*
      if (word->best_state[ch] > 1) {
        // If the next blob is good, make junk with the rightmost fragment.
        if (ch + 1 < word_len && word->correct_text[ch + 1].length() > 0) {
          LearnPieces(fontname, start_blob + word->best_state[ch] - 1,
                      word->best_state[ch + 1] + 1,
                      threshold, CST_IMPROPER, INVALID_UNICHAR, word);
        }
        // If the previous blob is good, make junk with the leftmost fragment.
        if (ch > 0 && word->correct_text[ch - 1].length() > 0) {
          LearnPieces(fontname, start_blob - word->best_state[ch - 1],
                      word->best_state[ch - 1] + 1,
                      threshold, CST_IMPROPER, INVALID_UNICHAR, word);
        }
      }
      // If the next blob is good, make a join with it.
      if (ch + 1 < word_len && word->correct_text[ch + 1].length() > 0) {
        STRING joined_text = word->correct_text[ch];
        joined_text += word->correct_text[ch + 1];
        LearnPieces(fontname, start_blob,
                    word->best_state[ch] + word->best_state[ch + 1],
                    threshold, CST_NGRAM, joined_text.string(), word);
      }
      */
    }
    start_blob += word->best_state[ch];
  }
  delete [] thresholds;
}  // LearnWord.

// Builds a blob of length fragments, from the word, starting at start,
// and then learns it, as having the given correct_text.
// If fontname is not nullptr, then LearnBlob is called and the data will be
// saved in an internal buffer for static training.
// Otherwise AdaptToBlob is called for adaption within a document.
// threshold is a magic number required by AdaptToChar and generated by
// ComputeAdaptionThresholds.
// Although it can be partly inferred from the string, segmentation is
// provided to explicitly clarify the character segmentation.
void Classify::LearnPieces(const char* fontname, int start, int length,
                           float threshold, CharSegmentationType segmentation,
                           const char* correct_text, WERD_RES* word) {
  // TODO(daria) Remove/modify this if/when we want
  // to train and/or adapt to n-grams.
  if (segmentation != CST_WHOLE &&
      (segmentation != CST_FRAGMENT || disable_character_fragments))
    return;

  if (length > 1) {
    SEAM::JoinPieces(word->seam_array, word->chopped_word->blobs, start,
                     start + length - 1);
  }
  TBLOB* blob = word->chopped_word->blobs[start];
  // Rotate the blob if needed for classification.
  TBLOB* rotated_blob = blob->ClassifyNormalizeIfNeeded();
  if (rotated_blob == nullptr)
    rotated_blob = blob;

  #ifndef GRAPHICS_DISABLED
  // Draw debug windows showing the blob that is being learned if needed.
  if (strcmp(classify_learn_debug_str.string(), correct_text) == 0) {
    RefreshDebugWindow(&learn_debug_win_, "LearnPieces", 600,
                       word->chopped_word->bounding_box());
    rotated_blob->plot(learn_debug_win_, ScrollView::GREEN, ScrollView::BROWN);
    learn_debug_win_->Update();
    window_wait(learn_debug_win_);
  }
  if (classify_debug_character_fragments && segmentation == CST_FRAGMENT) {
    ASSERT_HOST(learn_fragments_debug_win_ != nullptr);  // set up in LearnWord
    blob->plot(learn_fragments_debug_win_,
               ScrollView::BLUE, ScrollView::BROWN);
    learn_fragments_debug_win_->Update();
  }
  #endif  // GRAPHICS_DISABLED

  if (fontname != nullptr) {
    classify_norm_method.set_value(character);  // force char norm spc 30/11/93
    tess_bn_matching.set_value(false);    // turn it off
    tess_cn_matching.set_value(false);
    DENORM bl_denorm, cn_denorm;
    INT_FX_RESULT_STRUCT fx_info;
    SetupBLCNDenorms(*rotated_blob, classify_nonlinear_norm,
                     &bl_denorm, &cn_denorm, &fx_info);
    LearnBlob(fontname, rotated_blob, cn_denorm, fx_info, correct_text);
  } else if (unicharset.contains_unichar(correct_text)) {
    UNICHAR_ID class_id = unicharset.unichar_to_id(correct_text);
    int font_id = word->fontinfo != nullptr
                ? fontinfo_table_.get_id(*word->fontinfo)
                : 0;
    if (classify_learning_debug_level >= 1)
      tprintf("Adapting to char = %s, thr= %g font_id= %d\n",
              unicharset.id_to_unichar(class_id), threshold, font_id);
    // If filename is not nullptr we are doing recognition
    // (as opposed to training), so we must have already set word fonts.
    AdaptToChar(rotated_blob, class_id, font_id, threshold, AdaptedTemplates);
    if (BackupAdaptedTemplates != nullptr) {
      // Adapt the backup templates too. They will be used if the primary gets
      // too full.
      AdaptToChar(rotated_blob, class_id, font_id, threshold,
                  BackupAdaptedTemplates);
    }
  } else if (classify_debug_level >= 1) {
    tprintf("Can't adapt to %s not in unicharset\n", correct_text);
  }
  if (rotated_blob != blob) {
    delete rotated_blob;
  }

  SEAM::BreakPieces(word->seam_array, word->chopped_word->blobs, start,
                    start + length - 1);
}  // LearnPieces.

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
 */
void Classify::EndAdaptiveClassifier() {
  STRING Filename;
  FILE *File;

  if (AdaptedTemplates != nullptr &&
      classify_enable_adaptive_matcher && classify_save_adapted_templates) {
    Filename = imagefile + ADAPT_TEMPLATE_SUFFIX;
    File = fopen (Filename.string(), "wb");
    if (File == nullptr)
      cprintf ("Unable to save adapted templates to %s!\n", Filename.string());
    else {
      cprintf ("\nSaving adapted templates to %s ...", Filename.string());
      fflush(stdout);
      WriteAdaptedTemplates(File, AdaptedTemplates);
      cprintf ("\n");
      fclose(File);
    }
  }

  if (AdaptedTemplates != nullptr) {
    free_adapted_templates(AdaptedTemplates);
    AdaptedTemplates = nullptr;
  }
  if (BackupAdaptedTemplates != nullptr) {
    free_adapted_templates(BackupAdaptedTemplates);
    BackupAdaptedTemplates = nullptr;
  }

  if (PreTrainedTemplates != nullptr) {
    free_int_templates(PreTrainedTemplates);
    PreTrainedTemplates = nullptr;
  }
  getDict().EndDangerousAmbigs();
  FreeNormProtos();
  if (AllProtosOn != nullptr) {
    FreeBitVector(AllProtosOn);
    FreeBitVector(AllConfigsOn);
    FreeBitVector(AllConfigsOff);
    FreeBitVector(TempProtoMask);
    AllProtosOn = nullptr;
    AllConfigsOn = nullptr;
    AllConfigsOff = nullptr;
    TempProtoMask = nullptr;
  }
  delete shape_table_;
  shape_table_ = nullptr;
  delete static_classifier_;
  static_classifier_ = nullptr;
}                                /* EndAdaptiveClassifier */


/*---------------------------------------------------------------------------*/
/**
 * This routine reads in the training
 * information needed by the adaptive classifier
 * and saves it into global variables.
 *  Parameters:
 *      load_pre_trained_templates  Indicates whether the pre-trained
 *                     templates (inttemp, normproto and pffmtable components)
 *                     should be loaded. Should only be set to true if the
 *                     necessary classifier components are present in the
 *                     [lang].traineddata file.
 *  Globals:
 *      BuiltInTemplatesFile  file to get built-in temps from
 *      BuiltInCutoffsFile    file to get avg. feat per class from
 *      classify_use_pre_adapted_templates
 *                            enables use of pre-adapted templates
 */
void Classify::InitAdaptiveClassifier(TessdataManager* mgr) {
  if (!classify_enable_adaptive_matcher)
    return;
  if (AllProtosOn != nullptr)
    EndAdaptiveClassifier();  // Don't leak with multiple inits.

  // If there is no language_data_path_prefix, the classifier will be
  // adaptive only.
  if (language_data_path_prefix.length() > 0 && mgr != nullptr) {
    TFile fp;
    ASSERT_HOST(mgr->GetComponent(TESSDATA_INTTEMP, &fp));
    PreTrainedTemplates = ReadIntTemplates(&fp);

    if (mgr->GetComponent(TESSDATA_SHAPE_TABLE, &fp)) {
      shape_table_ = new ShapeTable(unicharset);
      if (!shape_table_->DeSerialize(&fp)) {
        tprintf("Error loading shape table!\n");
        delete shape_table_;
        shape_table_ = nullptr;
      }
    }

    ASSERT_HOST(mgr->GetComponent(TESSDATA_PFFMTABLE, &fp));
    ReadNewCutoffs(&fp, CharNormCutoffs);

    ASSERT_HOST(mgr->GetComponent(TESSDATA_NORMPROTO, &fp));
    NormProtos = ReadNormProtos(&fp);
    static_classifier_ = new TessClassifier(false, this);
  }

  InitIntegerFX();

  AllProtosOn = NewBitVector(MAX_NUM_PROTOS);
  AllConfigsOn = NewBitVector(MAX_NUM_CONFIGS);
  AllConfigsOff = NewBitVector(MAX_NUM_CONFIGS);
  TempProtoMask = NewBitVector(MAX_NUM_PROTOS);
  set_all_bits(AllProtosOn, WordsInVectorOfSize(MAX_NUM_PROTOS));
  set_all_bits(AllConfigsOn, WordsInVectorOfSize(MAX_NUM_CONFIGS));
  zero_all_bits(AllConfigsOff, WordsInVectorOfSize(MAX_NUM_CONFIGS));

  for (int i = 0; i < MAX_NUM_CLASSES; i++) {
     BaselineCutoffs[i] = 0;
  }

  if (classify_use_pre_adapted_templates) {
    TFile fp;
    STRING Filename;

    Filename = imagefile;
    Filename += ADAPT_TEMPLATE_SUFFIX;
    if (!fp.Open(Filename.string(), nullptr)) {
      AdaptedTemplates = NewAdaptedTemplates(true);
    } else {
      cprintf("\nReading pre-adapted templates from %s ...\n",
              Filename.string());
      fflush(stdout);
      AdaptedTemplates = ReadAdaptedTemplates(&fp);
      cprintf("\n");
      PrintAdaptedTemplates(stdout, AdaptedTemplates);

      for (int i = 0; i < AdaptedTemplates->Templates->NumClasses; i++) {
        BaselineCutoffs[i] = CharNormCutoffs[i];
      }
    }
  } else {
    if (AdaptedTemplates != nullptr)
      free_adapted_templates(AdaptedTemplates);
    AdaptedTemplates = NewAdaptedTemplates(true);
  }
}                                /* InitAdaptiveClassifier */

void Classify::ResetAdaptiveClassifierInternal() {
  if (classify_learning_debug_level > 0) {
    tprintf("Resetting adaptive classifier (NumAdaptationsFailed=%d)\n",
            NumAdaptationsFailed);
  }
  free_adapted_templates(AdaptedTemplates);
  AdaptedTemplates = NewAdaptedTemplates(true);
  if (BackupAdaptedTemplates != nullptr)
    free_adapted_templates(BackupAdaptedTemplates);
  BackupAdaptedTemplates = nullptr;
  NumAdaptationsFailed = 0;
}

// If there are backup adapted templates, switches to those, otherwise resets
// the main adaptive classifier (because it is full.)
void Classify::SwitchAdaptiveClassifier() {
  if (BackupAdaptedTemplates == nullptr) {
    ResetAdaptiveClassifierInternal();
    return;
  }
  if (classify_learning_debug_level > 0) {
    tprintf("Switch to backup adaptive classifier (NumAdaptationsFailed=%d)\n",
            NumAdaptationsFailed);
  }
  free_adapted_templates(AdaptedTemplates);
  AdaptedTemplates = BackupAdaptedTemplates;
  BackupAdaptedTemplates = nullptr;
  NumAdaptationsFailed = 0;
}

// Resets the backup adaptive classifier to empty.
void Classify::StartBackupAdaptiveClassifier() {
  if (BackupAdaptedTemplates != nullptr)
    free_adapted_templates(BackupAdaptedTemplates);
  BackupAdaptedTemplates = NewAdaptedTemplates(true);
}

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
 * @param ClassId id of the class to be initialized
 * @param FontinfoId font information inferred from pre-trained templates
 * @param Class adapted class to be initialized
 * @param Templates adapted templates to add new class to
 *
 * Globals:
 * - #AllProtosOn dummy mask with all 1's
 * - BaselineCutoffs kludge needed to get cutoffs
 * - #PreTrainedTemplates kludge needed to get cutoffs
 */
void Classify::InitAdaptedClass(TBLOB *Blob,
                                CLASS_ID ClassId,
                                int FontinfoId,
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
  Features = ExtractOutlineFeatures(Blob);
  NumFeatures = Features->NumFeatures;
  if (NumFeatures > UNLIKELY_NUM_FEAT || NumFeatures <= 0) {
    FreeFeatureSet(Features);
    return;
  }

  Config = NewTempConfig(NumFeatures - 1, FontinfoId);
  TempConfigFor(Class, 0) = Config;

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
    AddProtoToProtoPruner(Proto, Pid, IClass,
                          classify_learning_debug_level >= 2);

    Class->TempProtos = push (Class->TempProtos, TempProto);
  }
  FreeFeatureSet(Features);

  AddIntConfig(IClass);
  ConvertConfig (AllProtosOn, 0, IClass);

  if (classify_learning_debug_level >= 1) {
    tprintf("Added new class '%s' with class id %d and %d protos.\n",
            unicharset.id_to_unichar(ClassId), ClassId, NumFeatures);
    if (classify_learning_debug_level > 1)
      DisplayAdaptedChar(Blob, IClass);
  }

  if (IsEmptyAdaptedClass(Class))
    (Templates->NumNonEmptyClasses)++;
}                                /* InitAdaptedClass */


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
 * @param[out] IntFeatures array to fill with integer features
 * @param[out] FloatFeatures place to return actual floating-pt features
 *
 * @return Number of pico-features returned (0 if
 * an error occurred)
 */
int Classify::GetAdaptiveFeatures(TBLOB *Blob,
                                  INT_FEATURE_ARRAY IntFeatures,
                                  FEATURE_SET *FloatFeatures) {
  FEATURE_SET Features;
  int NumFeatures;

  classify_norm_method.set_value(baseline);
  Features = ExtractPicoFeatures(Blob);

  NumFeatures = Features->NumFeatures;
  if (NumFeatures == 0 || NumFeatures > UNLIKELY_NUM_FEAT) {
    FreeFeatureSet(Features);
    return 0;
  }

  ComputeIntFeatures(Features, IntFeatures);
  *FloatFeatures = Features;

  return NumFeatures;
}                                /* GetAdaptiveFeatures */


/*-----------------------------------------------------------------------------
              Private Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * Return TRUE if the specified word is
 * acceptable for adaptation.
 *
 * Globals: none
 *
 * @param word current word
 *
 * @return true or false
 */
bool Classify::AdaptableWord(WERD_RES* word) {
  if (word->best_choice == nullptr) return false;
  int BestChoiceLength = word->best_choice->length();
  float adaptable_score =
    getDict().segment_penalty_dict_case_ok + ADAPTABLE_WERD_ADJUSTMENT;
  return   // rules that apply in general - simplest to compute first
      BestChoiceLength > 0 &&
      BestChoiceLength == word->rebuild_word->NumBlobs() &&
      BestChoiceLength <= MAX_ADAPTABLE_WERD_SIZE &&
      // This basically ensures that the word is at least a dictionary match
      // (freq word, user word, system dawg word, etc).
      // Since all the other adjustments will make adjust factor higher
      // than higher than adaptable_score=1.1+0.05=1.15
      // Since these are other flags that ensure that the word is dict word,
      // this check could be at times redundant.
      word->best_choice->adjust_factor() <= adaptable_score &&
      // Make sure that alternative choices are not dictionary words.
      word->AlternativeChoiceAdjustmentsWorseThan(adaptable_score);
}

/*---------------------------------------------------------------------------*/
/**
 * @param Blob blob to add to templates for ClassId
 * @param ClassId class to add blob to
 * @param FontinfoId font information from pre-trained templates
 * @param Threshold minimum match rating to existing template
 * @param adaptive_templates current set of adapted templates
 *
 * Globals:
 * - AllProtosOn dummy mask to match against all protos
 * - AllConfigsOn dummy mask to match against all configs
 *
 * @return none
 */
void Classify::AdaptToChar(TBLOB* Blob, CLASS_ID ClassId, int FontinfoId,
                           float Threshold,
                           ADAPT_TEMPLATES adaptive_templates) {
  int NumFeatures;
  INT_FEATURE_ARRAY IntFeatures;
  UnicharRating int_result;
  INT_CLASS IClass;
  ADAPT_CLASS Class;
  TEMP_CONFIG TempConfig;
  FEATURE_SET FloatFeatures;
  int NewTempConfigId;

  if (!LegalClassId (ClassId))
    return;

  int_result.unichar_id = ClassId;
  Class = adaptive_templates->Class[ClassId];
  assert(Class != nullptr);
  if (IsEmptyAdaptedClass(Class)) {
    InitAdaptedClass(Blob, ClassId, FontinfoId, Class, adaptive_templates);
  } else {
    IClass = ClassForClassId(adaptive_templates->Templates, ClassId);

    NumFeatures = GetAdaptiveFeatures(Blob, IntFeatures, &FloatFeatures);
    if (NumFeatures <= 0) {
      return;  // Features already freed by GetAdaptiveFeatures.
    }

    // Only match configs with the matching font.
    BIT_VECTOR MatchingFontConfigs = NewBitVector(MAX_NUM_PROTOS);
    for (int cfg = 0; cfg < IClass->NumConfigs; ++cfg) {
      if (GetFontinfoId(Class, cfg) == FontinfoId) {
        SET_BIT(MatchingFontConfigs, cfg);
      } else {
        reset_bit(MatchingFontConfigs, cfg);
      }
    }
    im_.Match(IClass, AllProtosOn, MatchingFontConfigs,
              NumFeatures, IntFeatures,
              &int_result, classify_adapt_feature_threshold,
              NO_DEBUG, matcher_debug_separate_windows);
    FreeBitVector(MatchingFontConfigs);

    SetAdaptiveThreshold(Threshold);

    if (1.0f - int_result.rating <= Threshold) {
      if (ConfigIsPermanent(Class, int_result.config)) {
        if (classify_learning_debug_level >= 1)
          tprintf("Found good match to perm config %d = %4.1f%%.\n",
                  int_result.config, int_result.rating * 100.0);
        FreeFeatureSet(FloatFeatures);
        return;
      }

      TempConfig = TempConfigFor(Class, int_result.config);
      IncreaseConfidence(TempConfig);
      if (TempConfig->NumTimesSeen > Class->MaxNumTimesSeen) {
        Class->MaxNumTimesSeen = TempConfig->NumTimesSeen;
      }
      if (classify_learning_debug_level >= 1)
        tprintf("Increasing reliability of temp config %d to %d.\n",
                int_result.config, TempConfig->NumTimesSeen);

      if (TempConfigReliable(ClassId, TempConfig)) {
        MakePermanent(adaptive_templates, ClassId, int_result.config, Blob);
        UpdateAmbigsGroup(ClassId, Blob);
      }
    } else {
      if (classify_learning_debug_level >= 1) {
        tprintf("Found poor match to temp config %d = %4.1f%%.\n",
                int_result.config, int_result.rating * 100.0);
        if (classify_learning_debug_level > 2)
          DisplayAdaptedChar(Blob, IClass);
      }
      NewTempConfigId =
          MakeNewTemporaryConfig(adaptive_templates, ClassId, FontinfoId,
                                 NumFeatures, IntFeatures, FloatFeatures);
      if (NewTempConfigId >= 0 &&
          TempConfigReliable(ClassId, TempConfigFor(Class, NewTempConfigId))) {
        MakePermanent(adaptive_templates, ClassId, NewTempConfigId, Blob);
        UpdateAmbigsGroup(ClassId, Blob);
      }

#ifndef GRAPHICS_DISABLED
      if (classify_learning_debug_level > 1) {
        DisplayAdaptedChar(Blob, IClass);
      }
#endif
    }
    FreeFeatureSet(FloatFeatures);
  }
}                                /* AdaptToChar */

void Classify::DisplayAdaptedChar(TBLOB* blob, INT_CLASS_STRUCT* int_class) {
#ifndef GRAPHICS_DISABLED
  INT_FX_RESULT_STRUCT fx_info;
  GenericVector<INT_FEATURE_STRUCT> bl_features;
  TrainingSample* sample =
      BlobToTrainingSample(*blob, classify_nonlinear_norm, &fx_info,
                           &bl_features);
  if (sample == nullptr) return;

  UnicharRating int_result;
  im_.Match(int_class, AllProtosOn, AllConfigsOn,
            bl_features.size(), &bl_features[0],
            &int_result, classify_adapt_feature_threshold,
            NO_DEBUG, matcher_debug_separate_windows);
  tprintf("Best match to temp config %d = %4.1f%%.\n",
          int_result.config, int_result.rating * 100.0);
  if (classify_learning_debug_level >= 2) {
    uint32_t ConfigMask;
    ConfigMask = 1 << int_result.config;
    ShowMatchDisplay();
    im_.Match(int_class, AllProtosOn, (BIT_VECTOR)&ConfigMask,
              bl_features.size(), &bl_features[0],
              &int_result, classify_adapt_feature_threshold,
              6 | 0x19, matcher_debug_separate_windows);
    UpdateMatchDisplay();
  }

  delete sample;
#endif
}

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
 * @param new_result new result to add
 * @param[out] results results to add new result to
 */
void Classify::AddNewResult(const UnicharRating& new_result,
                            ADAPT_RESULTS *results) {
  int old_match = FindScoredUnichar(new_result.unichar_id, *results);

  if (new_result.rating + matcher_bad_match_pad < results->best_rating ||
      (old_match < results->match.size() &&
       new_result.rating <= results->match[old_match].rating))
    return;  // New one not good enough.

  if (!unicharset.get_fragment(new_result.unichar_id))
    results->HasNonfragment = true;

  if (old_match < results->match.size()) {
    results->match[old_match].rating = new_result.rating;
  } else {
    results->match.push_back(new_result);
  }

  if (new_result.rating > results->best_rating &&
      // Ensure that fragments do not affect best rating, class and config.
      // This is needed so that at least one non-fragmented character is
      // always present in the results.
      // TODO(daria): verify that this helps accuracy and does not
      // hurt performance.
      !unicharset.get_fragment(new_result.unichar_id)) {
    results->best_match_index = old_match;
    results->best_rating = new_result.rating;
    results->best_unichar_id = new_result.unichar_id;
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
 * @param blob blob to be classified
 * @param templates built-in templates to classify against
 * @param classes adapted class templates
 * @param ambiguities array of unichar id's to match against
 * @param[out] results place to put match results
 * @param int_features
 * @param fx_info
 */
void Classify::AmbigClassifier(
    const GenericVector<INT_FEATURE_STRUCT>& int_features,
    const INT_FX_RESULT_STRUCT& fx_info,
    const TBLOB *blob,
    INT_TEMPLATES templates,
    ADAPT_CLASS *classes,
    UNICHAR_ID *ambiguities,
    ADAPT_RESULTS *results) {
  if (int_features.empty()) return;
  uint8_t* CharNormArray = new uint8_t[unicharset.size()];
  UnicharRating int_result;

  results->BlobLength = GetCharNormFeature(fx_info, templates, nullptr,
                                           CharNormArray);
  bool debug = matcher_debug_level >= 2 || classify_debug_level > 1;
  if (debug)
    tprintf("AM Matches =  ");

  int top = blob->bounding_box().top();
  int bottom = blob->bounding_box().bottom();
  while (*ambiguities >= 0) {
    CLASS_ID class_id = *ambiguities;

    int_result.unichar_id = class_id;
    im_.Match(ClassForClassId(templates, class_id),
              AllProtosOn, AllConfigsOn,
              int_features.size(), &int_features[0],
              &int_result,
              classify_adapt_feature_threshold, NO_DEBUG,
              matcher_debug_separate_windows);

    ExpandShapesAndApplyCorrections(nullptr, debug, class_id, bottom, top, 0,
                                    results->BlobLength,
                                    classify_integer_matcher_multiplier,
                                    CharNormArray, &int_result, results);
    ambiguities++;
  }
  delete [] CharNormArray;
}                                /* AmbigClassifier */

/*---------------------------------------------------------------------------*/
/// Factored-out calls to IntegerMatcher based on class pruner results.
/// Returns integer matcher results inside CLASS_PRUNER_RESULTS structure.
void Classify::MasterMatcher(INT_TEMPLATES templates,
                             int16_t num_features,
                             const INT_FEATURE_STRUCT* features,
                             const uint8_t* norm_factors,
                             ADAPT_CLASS* classes,
                             int debug,
                             int matcher_multiplier,
                             const TBOX& blob_box,
                             const GenericVector<CP_RESULT_STRUCT>& results,
                             ADAPT_RESULTS* final_results) {
  int top = blob_box.top();
  int bottom = blob_box.bottom();
  UnicharRating int_result;
  for (int c = 0; c < results.size(); c++) {
    CLASS_ID class_id = results[c].Class;
    BIT_VECTOR protos = classes != nullptr ? classes[class_id]->PermProtos
                                        : AllProtosOn;
    BIT_VECTOR configs = classes != nullptr ? classes[class_id]->PermConfigs
                                         : AllConfigsOn;

    int_result.unichar_id = class_id;
    im_.Match(ClassForClassId(templates, class_id),
              protos, configs,
              num_features, features,
              &int_result, classify_adapt_feature_threshold, debug,
              matcher_debug_separate_windows);
    bool is_debug = matcher_debug_level >= 2 || classify_debug_level > 1;
    ExpandShapesAndApplyCorrections(classes, is_debug, class_id, bottom, top,
                                    results[c].Rating,
                                    final_results->BlobLength,
                                    matcher_multiplier, norm_factors,
                                    &int_result, final_results);
  }
}

// Converts configs to fonts, and if the result is not adapted, and a
// shape_table_ is present, the shape is expanded to include all
// unichar_ids represented, before applying a set of corrections to the
// distance rating in int_result, (see ComputeCorrectedRating.)
// The results are added to the final_results output.
void Classify::ExpandShapesAndApplyCorrections(
    ADAPT_CLASS* classes, bool debug, int class_id, int bottom, int top,
    float cp_rating, int blob_length, int matcher_multiplier,
    const uint8_t* cn_factors,
    UnicharRating* int_result, ADAPT_RESULTS* final_results) {
  if (classes != nullptr) {
    // Adapted result. Convert configs to fontinfo_ids.
    int_result->adapted = true;
    for (int f = 0; f < int_result->fonts.size(); ++f) {
      int_result->fonts[f].fontinfo_id =
          GetFontinfoId(classes[class_id], int_result->fonts[f].fontinfo_id);
    }
  } else {
    // Pre-trained result. Map fonts using font_sets_.
    int_result->adapted = false;
    for (int f = 0; f < int_result->fonts.size(); ++f) {
      int_result->fonts[f].fontinfo_id =
          ClassAndConfigIDToFontOrShapeID(class_id,
                                          int_result->fonts[f].fontinfo_id);
    }
    if (shape_table_ != nullptr) {
      // Two possible cases:
      // 1. Flat shapetable. All unichar-ids of the shapes referenced by
      // int_result->fonts are the same. In this case build a new vector of
      // mapped fonts and replace the fonts in int_result.
      // 2. Multi-unichar shapetable. Variable unichars in the shapes referenced
      // by int_result. In this case, build a vector of UnicharRating to
      // gather together different font-ids for each unichar. Also covers case1.
      GenericVector<UnicharRating> mapped_results;
      for (int f = 0; f < int_result->fonts.size(); ++f) {
        int shape_id = int_result->fonts[f].fontinfo_id;
        const Shape& shape = shape_table_->GetShape(shape_id);
        for (int c = 0; c < shape.size(); ++c) {
          int unichar_id = shape[c].unichar_id;
          if (!unicharset.get_enabled(unichar_id)) continue;
          // Find the mapped_result for unichar_id.
          int r = 0;
          for (r = 0; r < mapped_results.size() &&
               mapped_results[r].unichar_id != unichar_id; ++r) {}
          if (r == mapped_results.size()) {
            mapped_results.push_back(*int_result);
            mapped_results[r].unichar_id = unichar_id;
            mapped_results[r].fonts.truncate(0);
          }
          for (int i = 0; i < shape[c].font_ids.size(); ++i) {
            mapped_results[r].fonts.push_back(
                ScoredFont(shape[c].font_ids[i], int_result->fonts[f].score));
          }
        }
      }
      for (int m = 0; m < mapped_results.size(); ++m) {
        mapped_results[m].rating =
            ComputeCorrectedRating(debug, mapped_results[m].unichar_id,
                                   cp_rating, int_result->rating,
                                   int_result->feature_misses, bottom, top,
                                   blob_length, matcher_multiplier, cn_factors);
        AddNewResult(mapped_results[m], final_results);
      }
      return;
    }
  }
  if (unicharset.get_enabled(class_id)) {
    int_result->rating = ComputeCorrectedRating(debug, class_id, cp_rating,
                                                int_result->rating,
                                                int_result->feature_misses,
                                                bottom, top, blob_length,
                                                matcher_multiplier, cn_factors);
    AddNewResult(*int_result, final_results);
  }
}

// Applies a set of corrections to the confidence im_rating,
// including the cn_correction, miss penalty and additional penalty
// for non-alnums being vertical misfits. Returns the corrected confidence.
double Classify::ComputeCorrectedRating(bool debug, int unichar_id,
                                        double cp_rating, double im_rating,
                                        int feature_misses,
                                        int bottom, int top,
                                        int blob_length, int matcher_multiplier,
                                        const uint8_t* cn_factors) {
  // Compute class feature corrections.
  double cn_corrected = im_.ApplyCNCorrection(1.0 - im_rating, blob_length,
                                              cn_factors[unichar_id],
                                              matcher_multiplier);
  double miss_penalty = tessedit_class_miss_scale * feature_misses;
  double vertical_penalty = 0.0;
  // Penalize non-alnums for being vertical misfits.
  if (!unicharset.get_isalpha(unichar_id) &&
      !unicharset.get_isdigit(unichar_id) &&
      cn_factors[unichar_id] != 0 && classify_misfit_junk_penalty > 0.0) {
    int min_bottom, max_bottom, min_top, max_top;
    unicharset.get_top_bottom(unichar_id, &min_bottom, &max_bottom,
                              &min_top, &max_top);
    if (debug) {
      tprintf("top=%d, vs [%d, %d], bottom=%d, vs [%d, %d]\n",
              top, min_top, max_top, bottom, min_bottom, max_bottom);
    }
    if (top < min_top || top > max_top ||
        bottom < min_bottom || bottom > max_bottom) {
      vertical_penalty = classify_misfit_junk_penalty;
    }
  }
  double result = 1.0 - (cn_corrected + miss_penalty + vertical_penalty);
  if (result < WORST_POSSIBLE_RATING)
    result = WORST_POSSIBLE_RATING;
  if (debug) {
    tprintf("%s: %2.1f%%(CP%2.1f, IM%2.1f + CN%.2f(%d) + MP%2.1f + VP%2.1f)\n",
            unicharset.id_to_unichar(unichar_id),
            result * 100.0,
            cp_rating * 100.0,
            (1.0 - im_rating) * 100.0,
            (cn_corrected - (1.0 - im_rating)) * 100.0,
            cn_factors[unichar_id],
            miss_penalty * 100.0,
            vertical_penalty * 100.0);
  }
  return result;
}

/*---------------------------------------------------------------------------*/
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
 * @param Templates current set of adapted templates
 * @param Results place to put match results
 * @param int_features
 * @param fx_info
 *
 * @return Array of possible ambiguous chars that should be checked.
 */
UNICHAR_ID *Classify::BaselineClassifier(
    TBLOB *Blob, const GenericVector<INT_FEATURE_STRUCT>& int_features,
    const INT_FX_RESULT_STRUCT& fx_info,
    ADAPT_TEMPLATES Templates, ADAPT_RESULTS *Results) {
  if (int_features.empty()) return nullptr;
  uint8_t* CharNormArray = new uint8_t[unicharset.size()];
  ClearCharNormArray(CharNormArray);

  Results->BlobLength = IntCastRounded(fx_info.Length / kStandardFeatureLength);
  PruneClasses(Templates->Templates, int_features.size(), -1, &int_features[0],
               CharNormArray, BaselineCutoffs, &Results->CPResults);

  if (matcher_debug_level >= 2 || classify_debug_level > 1)
    tprintf("BL Matches =  ");

  MasterMatcher(Templates->Templates, int_features.size(), &int_features[0],
                CharNormArray,
                Templates->Class, matcher_debug_flags, 0,
                Blob->bounding_box(), Results->CPResults, Results);

  delete [] CharNormArray;
  CLASS_ID ClassId = Results->best_unichar_id;
  if (ClassId == INVALID_UNICHAR_ID || Results->best_match_index < 0)
    return nullptr;

  return Templates->Class[ClassId]->
      Config[Results->match[Results->best_match_index].config].Perm->Ambigs;
}                                /* BaselineClassifier */


/*---------------------------------------------------------------------------*/
/**
 * This routine extracts character normalized features
 * from the unknown character and matches them against the
 * specified set of templates.  The classes which match
 * are added to Results.
 *
 * @param blob blob to be classified
 * @param sample templates to classify unknown against
 * @param adapt_results place to put match results
 *
 * Globals:
 * - CharNormCutoffs expected num features for each class
 * - AllProtosOn mask that enables all protos
 * - AllConfigsOn mask that enables all configs
 */
int Classify::CharNormClassifier(TBLOB *blob,
                                 const TrainingSample& sample,
                                 ADAPT_RESULTS *adapt_results) {
  // This is the length that is used for scaling ratings vs certainty.
  adapt_results->BlobLength =
      IntCastRounded(sample.outline_length() / kStandardFeatureLength);
  GenericVector<UnicharRating> unichar_results;
  static_classifier_->UnicharClassifySample(sample, blob->denorm().pix(), 0,
                                            -1, &unichar_results);
  // Convert results to the format used internally by AdaptiveClassifier.
  for (int r = 0; r < unichar_results.size(); ++r) {
    AddNewResult(unichar_results[r], adapt_results);
  }
  return sample.num_features();
}                                /* CharNormClassifier */

// As CharNormClassifier, but operates on a TrainingSample and outputs to
// a GenericVector of ShapeRating without conversion to classes.
int Classify::CharNormTrainingSample(bool pruner_only,
                                     int keep_this,
                                     const TrainingSample& sample,
                                     GenericVector<UnicharRating>* results) {
  results->clear();
  ADAPT_RESULTS* adapt_results = new ADAPT_RESULTS();
  adapt_results->Initialize();
  // Compute the bounding box of the features.
  uint32_t num_features = sample.num_features();
  // Only the top and bottom of the blob_box are used by MasterMatcher, so
  // fabricate right and left using top and bottom.
  TBOX blob_box(sample.geo_feature(GeoBottom), sample.geo_feature(GeoBottom),
                sample.geo_feature(GeoTop), sample.geo_feature(GeoTop));
  // Compute the char_norm_array from the saved cn_feature.
  FEATURE norm_feature = sample.GetCNFeature();
  uint8_t* char_norm_array = new uint8_t[unicharset.size()];
  int num_pruner_classes = std::max(unicharset.size(),
                               PreTrainedTemplates->NumClasses);
  uint8_t* pruner_norm_array = new uint8_t[num_pruner_classes];
  adapt_results->BlobLength =
      static_cast<int>(ActualOutlineLength(norm_feature) * 20 + 0.5);
  ComputeCharNormArrays(norm_feature, PreTrainedTemplates, char_norm_array,
                        pruner_norm_array);

  PruneClasses(PreTrainedTemplates, num_features, keep_this, sample.features(),
               pruner_norm_array,
               shape_table_ != nullptr ? &shapetable_cutoffs_[0] : CharNormCutoffs,
               &adapt_results->CPResults);
  delete [] pruner_norm_array;
  if (keep_this >= 0) {
    adapt_results->CPResults[0].Class = keep_this;
    adapt_results->CPResults.truncate(1);
  }
  if (pruner_only) {
    // Convert pruner results to output format.
    for (int i = 0; i < adapt_results->CPResults.size(); ++i) {
      int class_id = adapt_results->CPResults[i].Class;
      results->push_back(
          UnicharRating(class_id, 1.0f - adapt_results->CPResults[i].Rating));
    }
  } else {
    MasterMatcher(PreTrainedTemplates, num_features, sample.features(),
                  char_norm_array,
                  nullptr, matcher_debug_flags,
                  classify_integer_matcher_multiplier,
                  blob_box, adapt_results->CPResults, adapt_results);
    // Convert master matcher results to output format.
    for (int i = 0; i < adapt_results->match.size(); i++) {
      results->push_back(adapt_results->match[i]);
    }
    results->sort(&UnicharRating::SortDescendingRating);
  }
  delete [] char_norm_array;
  delete adapt_results;
  return num_features;
}                                /* CharNormTrainingSample */


/*---------------------------------------------------------------------------*/
/**
 * This routine computes a rating which reflects the
 * likelihood that the blob being classified is a noise
 * blob.  NOTE: assumes that the blob length has already been
 * computed and placed into Results.
 *
 * @param results results to add noise classification to
 *
 * Globals:
 * - matcher_avg_noise_size avg. length of a noise blob
 */
void Classify::ClassifyAsNoise(ADAPT_RESULTS *results) {
  float rating = results->BlobLength / matcher_avg_noise_size;
  rating *= rating;
  rating /= 1.0 + rating;

  AddNewResult(UnicharRating(UNICHAR_SPACE, 1.0f - rating), results);
}                                /* ClassifyAsNoise */

/// The function converts the given match ratings to the list of blob
/// choices with ratings and certainties (used by the context checkers).
/// If character fragments are present in the results, this function also makes
/// sure that there is at least one non-fragmented classification included.
/// For each classification result check the unicharset for "definite"
/// ambiguities and modify the resulting Choices accordingly.
void Classify::ConvertMatchesToChoices(const DENORM& denorm, const TBOX& box,
                                       ADAPT_RESULTS *Results,
                                       BLOB_CHOICE_LIST *Choices) {
  assert(Choices != nullptr);
  float Rating;
  float Certainty;
  BLOB_CHOICE_IT temp_it;
  bool contains_nonfrag = false;
  temp_it.set_to_list(Choices);
  int choices_length = 0;
  // With no shape_table_ maintain the previous MAX_MATCHES as the maximum
  // number of returned results, but with a shape_table_ we want to have room
  // for at least the biggest shape (which might contain hundreds of Indic
  // grapheme fragments) and more, so use double the size of the biggest shape
  // if that is more than the default.
  int max_matches = MAX_MATCHES;
  if (shape_table_ != nullptr) {
    max_matches = shape_table_->MaxNumUnichars() * 2;
    if (max_matches < MAX_MATCHES)
      max_matches = MAX_MATCHES;
  }

  float best_certainty = -FLT_MAX;
  for (int i = 0; i < Results->match.size(); i++) {
    const UnicharRating& result = Results->match[i];
    bool adapted = result.adapted;
    bool current_is_frag = (unicharset.get_fragment(result.unichar_id) != nullptr);
    if (temp_it.length()+1 == max_matches &&
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
      Rating = Certainty = (1.0f - result.rating);
      Rating *= rating_scale * Results->BlobLength;
      Certainty *= -(getDict().certainty_scale);
    }
    // Adapted results, by their very nature, should have good certainty.
    // Those that don't are at best misleading, and often lead to errors,
    // so don't accept adapted results that are too far behind the best result,
    // whether adapted or static.
    // TODO(rays) find some way of automatically tuning these constants.
    if (Certainty > best_certainty) {
      best_certainty = std::min(Certainty, static_cast<float>(classify_adapted_pruning_threshold));
    } else if (adapted &&
               Certainty / classify_adapted_pruning_factor < best_certainty) {
      continue;  // Don't accept bad adapted results.
    }

    float min_xheight, max_xheight, yshift;
    denorm.XHeightRange(result.unichar_id, unicharset, box,
                        &min_xheight, &max_xheight, &yshift);
    BLOB_CHOICE* choice =
        new BLOB_CHOICE(result.unichar_id, Rating, Certainty,
                        unicharset.get_script(result.unichar_id),
                        min_xheight, max_xheight, yshift,
                        adapted ? BCC_ADAPTED_CLASSIFIER
                                : BCC_STATIC_CLASSIFIER);
    choice->set_fonts(result.fonts);
    temp_it.add_to_end(choice);
    contains_nonfrag |= !current_is_frag;  // update contains_nonfrag
    choices_length++;
    if (choices_length >= max_matches) break;
  }
  Results->match.truncate(choices_length);
}  // ConvertMatchesToChoices


/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
/**
 *
 * @param blob blob whose classification is being debugged
 * @param Results results of match being debugged
 *
 * Globals: none
 */
void Classify::DebugAdaptiveClassifier(TBLOB *blob,
                                       ADAPT_RESULTS *Results) {
  if (static_classifier_ == nullptr) return;
  INT_FX_RESULT_STRUCT fx_info;
  GenericVector<INT_FEATURE_STRUCT> bl_features;
  TrainingSample* sample =
      BlobToTrainingSample(*blob, false, &fx_info, &bl_features);
  if (sample == nullptr) return;
  static_classifier_->DebugDisplay(*sample, blob->denorm().pix(),
                                   Results->best_unichar_id);
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
 * @param Results place to put match results
 *
 * Globals:
 * - PreTrainedTemplates built-in training templates
 * - AdaptedTemplates templates adapted for this page
 * - matcher_reliable_adaptive_result rating limit for a great match
 */
void Classify::DoAdaptiveMatch(TBLOB *Blob, ADAPT_RESULTS *Results) {
  UNICHAR_ID *Ambiguities;

  INT_FX_RESULT_STRUCT fx_info;
  GenericVector<INT_FEATURE_STRUCT> bl_features;
  TrainingSample* sample =
      BlobToTrainingSample(*Blob, classify_nonlinear_norm, &fx_info,
                           &bl_features);
  if (sample == nullptr) return;

  // TODO: With LSTM, static_classifier_ is nullptr.
  // Return to avoid crash in CharNormClassifier.
  if (static_classifier_ == nullptr) {
    delete sample;
    return;
  }

  if (AdaptedTemplates->NumPermClasses < matcher_permanent_classes_min ||
      tess_cn_matching) {
    CharNormClassifier(Blob, *sample, Results);
  } else {
    Ambiguities = BaselineClassifier(Blob, bl_features, fx_info,
                                     AdaptedTemplates, Results);
    if ((!Results->match.empty() &&
         MarginalMatch(Results->best_rating,
                       matcher_reliable_adaptive_result) &&
         !tess_bn_matching) ||
        Results->match.empty()) {
      CharNormClassifier(Blob, *sample, Results);
    } else if (Ambiguities && *Ambiguities >= 0 && !tess_bn_matching) {
      AmbigClassifier(bl_features, fx_info, Blob,
                      PreTrainedTemplates,
                      AdaptedTemplates->Class,
                      Ambiguities,
                      Results);
    }
  }

  // Force the blob to be classified as noise
  // if the results contain only fragments.
  // TODO(daria): verify that this is better than
  // just adding a nullptr classification.
  if (!Results->HasNonfragment || Results->match.empty())
    ClassifyAsNoise(Results);
  delete sample;
}   /* DoAdaptiveMatch */

/*---------------------------------------------------------------------------*/
/**
 * This routine matches blob to the built-in templates
 * to find out if there are any classes other than the correct
 * class which are potential ambiguities.
 *
 * @param Blob blob to get classification ambiguities for
 * @param CorrectClass correct class for Blob
 *
 * Globals:
 * - CurrentRatings used by qsort compare routine
 * - PreTrainedTemplates built-in templates
 *
 * @return String containing all possible ambiguous classes.
 */
UNICHAR_ID *Classify::GetAmbiguities(TBLOB *Blob,
                                     CLASS_ID CorrectClass) {
  ADAPT_RESULTS *Results = new ADAPT_RESULTS();
  UNICHAR_ID *Ambiguities;
  int i;

  Results->Initialize();
  INT_FX_RESULT_STRUCT fx_info;
  GenericVector<INT_FEATURE_STRUCT> bl_features;
  TrainingSample* sample =
      BlobToTrainingSample(*Blob, classify_nonlinear_norm, &fx_info,
                           &bl_features);
  if (sample == nullptr) {
    delete Results;
    return nullptr;
  }

  CharNormClassifier(Blob, *sample, Results);
  delete sample;
  RemoveBadMatches(Results);
  Results->match.sort(&UnicharRating::SortDescendingRating);

  /* copy the class id's into an string of ambiguities - don't copy if
     the correct class is the only class id matched */
  Ambiguities = new UNICHAR_ID[Results->match.size() + 1];
  if (Results->match.size() > 1 ||
      (Results->match.size() == 1 &&
          Results->match[0].unichar_id != CorrectClass)) {
    for (i = 0; i < Results->match.size(); i++)
      Ambiguities[i] = Results->match[i].unichar_id;
    Ambiguities[i] = -1;
  } else {
    Ambiguities[0] = -1;
  }

  delete Results;
  return Ambiguities;
}                              /* GetAmbiguities */

// Returns true if the given blob looks too dissimilar to any character
// present in the classifier templates.
bool Classify::LooksLikeGarbage(TBLOB *blob) {
  BLOB_CHOICE_LIST *ratings = new BLOB_CHOICE_LIST();
  AdaptiveClassifier(blob, ratings);
  BLOB_CHOICE_IT ratings_it(ratings);
  const UNICHARSET &unicharset = getDict().getUnicharset();
  if (classify_debug_character_fragments) {
    print_ratings_list("======================\nLooksLikeGarbage() got ",
                       ratings, unicharset);
  }
  for (ratings_it.mark_cycle_pt(); !ratings_it.cycled_list();
       ratings_it.forward()) {
    if (unicharset.get_fragment(ratings_it.data()->unichar_id()) != nullptr) {
      continue;
    }
    float certainty = ratings_it.data()->certainty();
    delete ratings;
    return certainty <
            classify_character_fragments_garbage_certainty_threshold;
  }
  delete ratings;
  return true;  // no whole characters in ratings
}

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
 * @param templates used to compute char norm adjustments
 * @param pruner_norm_array Array of factors from blob normalization
 *        process
 * @param char_norm_array array to fill with dummy char norm adjustments
 * @param fx_info
 *
 * Globals:
 *
 * @return Number of features extracted or 0 if an error occurred.
 */
int Classify::GetCharNormFeature(const INT_FX_RESULT_STRUCT& fx_info,
                                 INT_TEMPLATES templates,
                                 uint8_t* pruner_norm_array,
                                 uint8_t* char_norm_array) {
  FEATURE norm_feature = NewFeature(&CharNormDesc);
  float baseline = kBlnBaselineOffset;
  float scale = MF_SCALE_FACTOR;
  norm_feature->Params[CharNormY] = (fx_info.Ymean - baseline) * scale;
  norm_feature->Params[CharNormLength] =
      fx_info.Length * scale / LENGTH_COMPRESSION;
  norm_feature->Params[CharNormRx] = fx_info.Rx * scale;
  norm_feature->Params[CharNormRy] = fx_info.Ry * scale;
  // Deletes norm_feature.
  ComputeCharNormArrays(norm_feature, templates, char_norm_array,
                        pruner_norm_array);
  return IntCastRounded(fx_info.Length / kStandardFeatureLength);
}                              /* GetCharNormFeature */

// Computes the char_norm_array for the unicharset and, if not nullptr, the
// pruner_array as appropriate according to the existence of the shape_table.
void Classify::ComputeCharNormArrays(FEATURE_STRUCT* norm_feature,
                                     INT_TEMPLATES_STRUCT* templates,
                                     uint8_t* char_norm_array,
                                     uint8_t* pruner_array) {
  ComputeIntCharNormArray(*norm_feature, char_norm_array);
  if (pruner_array != nullptr) {
    if (shape_table_ == nullptr) {
      ComputeIntCharNormArray(*norm_feature, pruner_array);
    } else {
      memset(pruner_array, UINT8_MAX,
             templates->NumClasses * sizeof(pruner_array[0]));
      // Each entry in the pruner norm array is the MIN of all the entries of
      // the corresponding unichars in the CharNormArray.
      for (int id = 0; id < templates->NumClasses; ++id) {
        int font_set_id = templates->Class[id]->font_set_id;
        const FontSet &fs = fontset_table_.get(font_set_id);
        for (int config = 0; config < fs.size; ++config) {
          const Shape& shape = shape_table_->GetShape(fs.configs[config]);
          for (int c = 0; c < shape.size(); ++c) {
            if (char_norm_array[shape[c].unichar_id] < pruner_array[id])
              pruner_array[id] = char_norm_array[shape[c].unichar_id];
          }
        }
      }
    }
  }
  FreeFeature(norm_feature);
}

/*---------------------------------------------------------------------------*/
/**
 *
 * @param Templates adapted templates to add new config to
 * @param ClassId class id to associate with new config
 * @param FontinfoId font information inferred from pre-trained templates
 * @param NumFeatures number of features in IntFeatures
 * @param Features features describing model for new config
 * @param FloatFeatures floating-pt representation of features
 *
 * @return The id of the new config created, a negative integer in
 * case of error.
 */
int Classify::MakeNewTemporaryConfig(ADAPT_TEMPLATES Templates,
                           CLASS_ID ClassId,
                           int FontinfoId,
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

  NumOldProtos = im_.FindGoodProtos(IClass, AllProtosOn, AllConfigsOff,
                                    BlobLength, NumFeatures, Features,
                                    OldProtos, classify_adapt_proto_threshold,
                                    debug_level);

  MaskSize = WordsInVectorOfSize(MAX_NUM_PROTOS);
  zero_all_bits(TempProtoMask, MaskSize);
  for (i = 0; i < NumOldProtos; i++)
    SET_BIT(TempProtoMask, OldProtos[i]);

  NumBadFeatures = im_.FindBadFeatures(IClass, TempProtoMask, AllConfigsOn,
                                       BlobLength, NumFeatures, Features,
                                       BadFeatures,
                                       classify_adapt_feature_threshold,
                                       debug_level);

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
  Config = NewTempConfig(MaxProtoId, FontinfoId);
  TempConfigFor(Class, ConfigId) = Config;
  copy_all_bits(TempProtoMask, Config->Protos, Config->ProtoVectorSize);

  if (classify_learning_debug_level >= 1)
    cprintf("Making new temp config %d fontinfo id %d"
            " using %d old and %d new protos.\n",
            ConfigId, Config->FontinfoId,
            NumOldProtos, MaxProtoId - OldMaxProtoId);

  return ConfigId;
}                              /* MakeNewTemporaryConfig */

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
 */
PROTO_ID Classify::MakeNewTempProtos(FEATURE_SET Features,
                                     int NumBadFeat,
                                     FEATURE_ID BadFeat[],
                                     INT_CLASS IClass,
                                     ADAPT_CLASS Class,
                                     BIT_VECTOR TempProtoMask) {
  FEATURE_ID *ProtoStart;
  FEATURE_ID *ProtoEnd;
  FEATURE_ID *LastBad;
  TEMP_PROTO TempProto;
  PROTO Proto;
  FEATURE F1, F2;
  float X1, X2, Y1, Y2;
  float A1, A2, AngleDelta;
  float SegmentLength;
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
    AddProtoToProtoPruner(Proto, Pid, IClass,
                          classify_learning_debug_level >= 2);

    Class->TempProtos = push(Class->TempProtos, TempProto);
  }
  return IClass->NumProtos - 1;
}                              /* MakeNewTempProtos */

/*---------------------------------------------------------------------------*/
/**
 *
 * @param Templates current set of adaptive templates
 * @param ClassId class containing config to be made permanent
 * @param ConfigId config to be made permanent
 * @param Blob current blob being adapted to
 *
 * Globals: none
 */
void Classify::MakePermanent(ADAPT_TEMPLATES Templates,
                             CLASS_ID ClassId,
                             int ConfigId,
                             TBLOB *Blob) {
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

  // Initialize permanent config.
  Ambigs = GetAmbiguities(Blob, ClassId);
  PERM_CONFIG Perm = (PERM_CONFIG)malloc(sizeof(PERM_CONFIG_STRUCT));
  Perm->Ambigs = Ambigs;
  Perm->FontinfoId = Config->FontinfoId;

  // Free memory associated with temporary config (since ADAPTED_CONFIG
  // is a union we need to clean up before we record permanent config).
  ProtoKey.Templates = Templates;
  ProtoKey.ClassId = ClassId;
  ProtoKey.ConfigId = ConfigId;
  Class->TempProtos = delete_d(Class->TempProtos, &ProtoKey, MakeTempProtoPerm);
  FreeTempConfig(Config);

  // Record permanent config.
  PermConfigFor(Class, ConfigId) = Perm;

  if (classify_learning_debug_level >= 1) {
    tprintf("Making config %d for %s (ClassId %d) permanent:"
            " fontinfo id %d, ambiguities '",
            ConfigId, getDict().getUnicharset().debug_str(ClassId).string(),
            ClassId, PermConfigFor(Class, ConfigId)->FontinfoId);
    for (UNICHAR_ID *AmbigsPointer = Ambigs;
        *AmbigsPointer >= 0; ++AmbigsPointer)
      tprintf("%s", unicharset.id_to_unichar(*AmbigsPointer));
    tprintf("'.\n");
  }
}                              /* MakePermanent */
}  // namespace tesseract

/*---------------------------------------------------------------------------*/
/**
 * This routine converts TempProto to be permanent if
 * its proto id is used by the configuration specified in
 * ProtoKey.
 *
 * @param item1 (TEMP_PROTO) temporary proto to compare to key
 * @param item2 (PROTO_KEY) defines which protos to make permanent
 *
 * Globals: none
 *
 * @return TRUE if TempProto is converted, FALSE otherwise
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
namespace tesseract {
/**
 * This routine writes the matches in Results to File.
 *
 * @param results match results to write to File
 *
 * Globals: none
 */
void Classify::PrintAdaptiveMatchResults(const ADAPT_RESULTS& results) {
  for (int i = 0; i < results.match.size(); ++i) {
    tprintf("%s  ", unicharset.debug_str(results.match[i].unichar_id).string());
    results.match[i].Print();
  }
}                              /* PrintAdaptiveMatchResults */

/*---------------------------------------------------------------------------*/
/**
 * This routine steps through each matching class in Results
 * and removes it from the match list if its rating
 * is worse than the BestRating plus a pad.  In other words,
 * all good matches get moved to the front of the classes
 * array.
 *
 * @param Results contains matches to be filtered
 *
 * Globals:
 * - matcher_bad_match_pad defines a "bad match"
 */
void Classify::RemoveBadMatches(ADAPT_RESULTS *Results) {
  int Next, NextGood;
  float BadMatchThreshold;
  static const char* romans = "i v x I V X";
  BadMatchThreshold = Results->best_rating - matcher_bad_match_pad;

  if (classify_bln_numeric_mode) {
    UNICHAR_ID unichar_id_one = unicharset.contains_unichar("1") ?
        unicharset.unichar_to_id("1") : -1;
    UNICHAR_ID unichar_id_zero = unicharset.contains_unichar("0") ?
        unicharset.unichar_to_id("0") : -1;
    float scored_one = ScoredUnichar(unichar_id_one, *Results);
    float scored_zero = ScoredUnichar(unichar_id_zero, *Results);

    for (Next = NextGood = 0; Next < Results->match.size(); Next++) {
      const UnicharRating& match = Results->match[Next];
      if (match.rating >= BadMatchThreshold) {
        if (!unicharset.get_isalpha(match.unichar_id) ||
            strstr(romans,
                   unicharset.id_to_unichar(match.unichar_id)) != nullptr) {
        } else if (unicharset.eq(match.unichar_id, "l") &&
                   scored_one < BadMatchThreshold) {
          Results->match[Next].unichar_id = unichar_id_one;
        } else if (unicharset.eq(match.unichar_id, "O") &&
                   scored_zero < BadMatchThreshold) {
          Results->match[Next].unichar_id = unichar_id_zero;
        } else {
          Results->match[Next].unichar_id = INVALID_UNICHAR_ID;  // Don't copy.
        }
        if (Results->match[Next].unichar_id != INVALID_UNICHAR_ID) {
          if (NextGood == Next) {
            ++NextGood;
          } else {
            Results->match[NextGood++] = Results->match[Next];
          }
        }
      }
    }
  } else {
    for (Next = NextGood = 0; Next < Results->match.size(); Next++) {
      if (Results->match[Next].rating >= BadMatchThreshold) {
        if (NextGood == Next) {
          ++NextGood;
        } else {
          Results->match[NextGood++] = Results->match[Next];
        }
      }
    }
  }
  Results->match.truncate(NextGood);
}                              /* RemoveBadMatches */

/*----------------------------------------------------------------------------*/
/**
 * This routine discards extra digits or punctuation from the results.
 * We keep only the top 2 punctuation answers and the top 1 digit answer if
 * present.
 *
 * @param Results contains matches to be filtered
 */
void Classify::RemoveExtraPuncs(ADAPT_RESULTS *Results) {
  int Next, NextGood;
  int punc_count;              /*no of garbage characters */
  int digit_count;
  /*garbage characters */
  static char punc_chars[] = ". , ; : / ` ~ ' - = \\ | \" ! _ ^";
  static char digit_chars[] = "0 1 2 3 4 5 6 7 8 9";

  punc_count = 0;
  digit_count = 0;
  for (Next = NextGood = 0; Next < Results->match.size(); Next++) {
    const UnicharRating& match = Results->match[Next];
    bool keep = true;
    if (strstr(punc_chars,
               unicharset.id_to_unichar(match.unichar_id)) != nullptr) {
      if (punc_count >= 2)
        keep = false;
      punc_count++;
    } else {
      if (strstr(digit_chars,
                 unicharset.id_to_unichar(match.unichar_id)) != nullptr) {
        if (digit_count >= 1)
          keep = false;
        digit_count++;
      }
    }
    if (keep) {
      if (NextGood == Next) {
        ++NextGood;
      } else {
        Results->match[NextGood++] = match;
      }
    }
  }
  Results->match.truncate(NextGood);
}                              /* RemoveExtraPuncs */

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
 */
void Classify::SetAdaptiveThreshold(float Threshold) {
  Threshold = (Threshold == matcher_good_threshold) ? 0.9: (1.0 - Threshold);
  classify_adapt_proto_threshold.set_value(
      ClipToRange<int>(255 * Threshold, 0, 255));
  classify_adapt_feature_threshold.set_value(
      ClipToRange<int>(255 * Threshold, 0, 255));
}                              /* SetAdaptiveThreshold */

/*---------------------------------------------------------------------------*/
/**
 * This routine displays debug information for the best config
 * of the given shape_id for the given set of features.
 *
 * @param shape_id classifier id to work with
 * @param features features of the unknown character
 * @param num_features Number of features in the features array.
 */

void Classify::ShowBestMatchFor(int shape_id,
                                const INT_FEATURE_STRUCT* features,
                                int num_features) {
#ifndef GRAPHICS_DISABLED
  uint32_t config_mask;
  if (UnusedClassIdIn(PreTrainedTemplates, shape_id)) {
    tprintf("No built-in templates for class/shape %d\n", shape_id);
    return;
  }
  if (num_features <= 0) {
    tprintf("Illegal blob (char norm features)!\n");
    return;
  }
  UnicharRating cn_result;
  classify_norm_method.set_value(character);
  im_.Match(ClassForClassId(PreTrainedTemplates, shape_id),
            AllProtosOn, AllConfigsOn,
            num_features, features, &cn_result,
            classify_adapt_feature_threshold, NO_DEBUG,
            matcher_debug_separate_windows);
  tprintf("\n");
  config_mask = 1 << cn_result.config;

  tprintf("Static Shape ID: %d\n", shape_id);
  ShowMatchDisplay();
  im_.Match(ClassForClassId(PreTrainedTemplates, shape_id), AllProtosOn,
            &config_mask, num_features, features, &cn_result,
            classify_adapt_feature_threshold, matcher_debug_flags,
            matcher_debug_separate_windows);
  UpdateMatchDisplay();
#endif  // GRAPHICS_DISABLED
}                              /* ShowBestMatchFor */

// Returns a string for the classifier class_id: either the corresponding
// unicharset debug_str or the shape_table_ debug str.
STRING Classify::ClassIDToDebugStr(const INT_TEMPLATES_STRUCT* templates,
                                   int class_id, int config_id) const {
  STRING class_string;
  if (templates == PreTrainedTemplates && shape_table_ != nullptr) {
    int shape_id = ClassAndConfigIDToFontOrShapeID(class_id, config_id);
    class_string = shape_table_->DebugStr(shape_id);
  } else {
    class_string = unicharset.debug_str(class_id);
  }
  return class_string;
}

// Converts a classifier class_id index to a shape_table_ index
int Classify::ClassAndConfigIDToFontOrShapeID(int class_id,
                                              int int_result_config) const {
  int font_set_id = PreTrainedTemplates->Class[class_id]->font_set_id;
  // Older inttemps have no font_ids.
  if (font_set_id < 0)
    return kBlankFontinfoId;
  const FontSet &fs = fontset_table_.get(font_set_id);
  ASSERT_HOST(int_result_config >= 0 && int_result_config < fs.size);
  return fs.configs[int_result_config];
}

// Converts a shape_table_ index to a classifier class_id index (not a
// unichar-id!). Uses a search, so not fast.
int Classify::ShapeIDToClassID(int shape_id) const {
  for (int id = 0; id < PreTrainedTemplates->NumClasses; ++id) {
    int font_set_id = PreTrainedTemplates->Class[id]->font_set_id;
    ASSERT_HOST(font_set_id >= 0);
    const FontSet &fs = fontset_table_.get(font_set_id);
    for (int config = 0; config < fs.size; ++config) {
      if (fs.configs[config] == shape_id)
        return id;
    }
  }
  tprintf("Shape %d not found\n", shape_id);
  return -1;
}

// Returns true if the given TEMP_CONFIG is good enough to make it
// a permanent config.
bool Classify::TempConfigReliable(CLASS_ID class_id,
                                  const TEMP_CONFIG &config) {
  if (classify_learning_debug_level >= 1) {
    tprintf("NumTimesSeen for config of %s is %d\n",
            getDict().getUnicharset().debug_str(class_id).string(),
            config->NumTimesSeen);
  }
  if (config->NumTimesSeen >= matcher_sufficient_examples_for_prototyping) {
    return true;
  } else if (config->NumTimesSeen < matcher_min_examples_for_prototyping) {
    return false;
  } else if (use_ambigs_for_adaption) {
    // Go through the ambigs vector and see whether we have already seen
    // enough times all the characters represented by the ambigs vector.
    const UnicharIdVector *ambigs =
      getDict().getUnicharAmbigs().AmbigsForAdaption(class_id);
    int ambigs_size = (ambigs == nullptr) ? 0 : ambigs->size();
    for (int ambig = 0; ambig < ambigs_size; ++ambig) {
      ADAPT_CLASS ambig_class = AdaptedTemplates->Class[(*ambigs)[ambig]];
      assert(ambig_class != nullptr);
      if (ambig_class->NumPermConfigs == 0 &&
          ambig_class->MaxNumTimesSeen <
          matcher_min_examples_for_prototyping) {
        if (classify_learning_debug_level >= 1) {
          tprintf("Ambig %s has not been seen enough times,"
                  " not making config for %s permanent\n",
                  getDict().getUnicharset().debug_str(
                      (*ambigs)[ambig]).string(),
                  getDict().getUnicharset().debug_str(class_id).string());
        }
        return false;
      }
    }
  }
  return true;
}

void Classify::UpdateAmbigsGroup(CLASS_ID class_id, TBLOB *Blob) {
  const UnicharIdVector *ambigs =
    getDict().getUnicharAmbigs().ReverseAmbigsForAdaption(class_id);
  int ambigs_size = (ambigs == nullptr) ? 0 : ambigs->size();
  if (classify_learning_debug_level >= 1) {
    tprintf("Running UpdateAmbigsGroup for %s class_id=%d\n",
            getDict().getUnicharset().debug_str(class_id).string(), class_id);
  }
  for (int ambig = 0; ambig < ambigs_size; ++ambig) {
    CLASS_ID ambig_class_id = (*ambigs)[ambig];
    const ADAPT_CLASS ambigs_class = AdaptedTemplates->Class[ambig_class_id];
    for (int cfg = 0; cfg < MAX_NUM_CONFIGS; ++cfg) {
      if (ConfigIsPermanent(ambigs_class, cfg)) continue;
      const TEMP_CONFIG config =
        TempConfigFor(AdaptedTemplates->Class[ambig_class_id], cfg);
      if (config != nullptr && TempConfigReliable(ambig_class_id, config)) {
        if (classify_learning_debug_level >= 1) {
          tprintf("Making config %d of %s permanent\n", cfg,
                  getDict().getUnicharset().debug_str(
                      ambig_class_id).string());
        }
        MakePermanent(AdaptedTemplates, ambig_class_id, cfg, Blob);
      }
    }
  }
}

}  // namespace tesseract
