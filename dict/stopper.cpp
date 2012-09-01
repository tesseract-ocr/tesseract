/******************************************************************************
 **	Filename:    stopper.c
 **	Purpose:     Stopping criteria for word classifier.
 **	Author:      Dan Johnson
 **	History:     Mon Apr 29 14:56:49 1991, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
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

#include "stopper.h"
#include "matchdefs.h"
#include "callcpp.h"
#include "permute.h"
#include "danerror.h"
#include "const.h"
#include "efio.h"
#include "scanutils.h"
#include "unichar.h"
#include "params.h"
#include "dict.h"
#include "image.h"
#include "ccutil.h"
#include "ratngs.h"
#include "ambigs.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifdef __UNIX__
#include <assert.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#pragma warning(disable:4800)  // int/bool warnings
#endif

/* these are kludges - add appropriate .h file later */
/* from adaptmatch.cpp */
#define MAX_WERD_SIZE   100

typedef struct
{
  VIABLE_CHOICE Choice;
  float ChunkCertainty[MAX_NUM_CHUNKS];
  UNICHAR_ID ChunkClass[MAX_NUM_CHUNKS];
} EXPANDED_CHOICE;

void DeleteViableChoiceStruct(void *vcs) {
  delete (static_cast<VIABLE_CHOICE_STRUCT *>(vcs));
}

#define BestCertainty(Choices) \
  (((VIABLE_CHOICE) first_node (Choices))->Certainty)

#define BestRating(Choices) (((VIABLE_CHOICE) first_node (Choices))->Rating)

#define BestFactor(Choices) \
  (((VIABLE_CHOICE) first_node (Choices))->AdjustFactor)

/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
// Returns -1 if the rating for Choice1 is less than the rating for Choice2,
// otherwise returns 1.
static int CmpChoiceRatings(void *arg1,    // VIABLE_CHOICE Choice1
                            void *arg2) {  // VIABLE_CHOICE Choice2
  float R1, R2;
  VIABLE_CHOICE Choice1 = (VIABLE_CHOICE) arg1;
  VIABLE_CHOICE Choice2 = (VIABLE_CHOICE) arg2;
  R1 = Choice1->Rating;
  R2 = Choice2->Rating;
  return (R1 < R2) ? -1 : 1;
}

// Expands Choice and places the results in ExpandedChoice. The primary
// function of expansion is to create an two arrays, one which holds the
// corresponding certainty for each chunk in Choice, and one which holds
// the class for each chunk.
static void ExpandChoice(VIABLE_CHOICE Choice,
                         EXPANDED_CHOICE *ExpandedChoice) {
  int i, j, Chunk;
  ExpandedChoice->Choice = Choice;
  for (i = 0, Chunk = 0; i < Choice->Length; i++)
  for (j = 0; j < Choice->Blob[i].NumChunks; j++, Chunk++) {
    ExpandedChoice->ChunkCertainty[Chunk] = Choice->Blob[i].Certainty;
    ExpandedChoice->ChunkClass[Chunk] = Choice->Blob[i].Class;
  }
}

VIABLE_CHOICE_STRUCT::VIABLE_CHOICE_STRUCT(int length)
    : Length(length) {
  Blob = new CHAR_CHOICE[length];
  blob_choices = NULL;
}

VIABLE_CHOICE_STRUCT::VIABLE_CHOICE_STRUCT() : Length(0) {
  Blob = NULL;
  blob_choices = NULL;
}

VIABLE_CHOICE_STRUCT::~VIABLE_CHOICE_STRUCT() {
  delete []Blob;
  if (blob_choices) {
    blob_choices->deep_clear();
    delete blob_choices;
  }
}

void VIABLE_CHOICE_STRUCT::Init(
    const WERD_CHOICE &word_choice,
    const PIECES_STATE &pieces_state,
    const float certainties[],
    FLOAT32 adjust_factor) {
  this->Rating = word_choice.rating();
  this->Certainty = word_choice.certainty();
  this->AdjustFactor = adjust_factor;
  this->ComposedFromCharFragments = false;
  ASSERT_HOST(this->Length == word_choice.length());

  for (int i = 0, bw_idx = 0; i < word_choice.length(); i++, bw_idx++) {
    int blob_width = pieces_state[bw_idx];
    CHAR_CHOICE *blob_choice = &this->Blob[i];
    blob_choice->Class = word_choice.unichar_id(i);
    blob_choice->NumChunks = blob_width;
    blob_choice->Certainty = certainties[i];
    for (int f = 1; f < word_choice.fragment_length(i); ++f) {
      blob_width = pieces_state[++bw_idx];
      assert(blob_width > 0);
      blob_choice->NumChunks += blob_width;
      this->ComposedFromCharFragments = true;
    }
  }
}

void VIABLE_CHOICE_STRUCT::SetBlobChoices(
    const BLOB_CHOICE_LIST_VECTOR &src_choices) {
  if (blob_choices != NULL) {
    blob_choices->deep_clear();
  } else {
    blob_choices = new BLOB_CHOICE_LIST_CLIST();
  }
  BLOB_CHOICE_LIST_C_IT list_it(blob_choices);

  for (int i = 0; i < src_choices.size(); ++i) {
    BLOB_CHOICE_LIST *cc_list = new BLOB_CHOICE_LIST();
    cc_list->deep_copy(src_choices[i], &BLOB_CHOICE::deep_copy);
    list_it.add_after_then_move(cc_list);
  }
}

namespace tesseract {

// If the certainty of any chunk in Choice (item1) is not ambiguous with the
// corresponding chunk in the best choice (item2), frees Choice and
// returns true.
int Dict::FreeBadChoice(
    void *item1,    // VIABLE_CHOICE Choice,
    void *item2) {  // EXPANDED_CHOICE *BestChoice
  int i, j, Chunk;
  FLOAT32 Threshold;
  VIABLE_CHOICE Choice = reinterpret_cast<VIABLE_CHOICE>(item1);
  EXPANDED_CHOICE *BestChoice = reinterpret_cast<EXPANDED_CHOICE *>(item2);
  Threshold = StopperAmbigThreshold(BestChoice->Choice->AdjustFactor,
                                    Choice->AdjustFactor);
  for (i = 0, Chunk = 0; i < Choice->Length; i++) {
    for (j = 0; j < Choice->Blob[i].NumChunks; j++, Chunk++) {
      if (Choice->Blob[i].Class != BestChoice->ChunkClass[Chunk] &&
          Choice->Blob[i].Certainty - BestChoice->ChunkCertainty[Chunk] <
          Threshold) {
        if (stopper_debug_level >= 2)
          PrintViableChoice(stderr, "\nDiscarding bad choice:  ", Choice);
        delete Choice;
        return true;
      }
    }
  }
  return false;
}

bool Dict::AcceptableChoice(BLOB_CHOICE_LIST_VECTOR *Choices,
                            WERD_CHOICE *BestChoice,
                            DANGERR *fixpt,
                            ACCEPTABLE_CHOICE_CALLER caller,
                            bool *modified_blobs) {
  float CertaintyThreshold = stopper_nondict_certainty_base;
  int WordSize;
  if (modified_blobs != NULL) *modified_blobs = false;

  if (stopper_no_acceptable_choices) return false;

  if (fixpt != NULL) fixpt->clear();
  if (BestChoice->length() == 0)
    return false;
  if (caller == CHOPPER_CALLER && BestChoice->fragment_mark()) {
    if (stopper_debug_level >= 1) {
      cprintf("AcceptableChoice(): a choice with fragments beats BestChoice");
    }
    return false;
  }

  bool no_dang_ambigs = (GetMaxFixedLengthDawgIndex() >= 0 ||
                         NoDangerousAmbig(BestChoice, fixpt, true,
                                          Choices, modified_blobs));
  bool is_valid_word = valid_word_permuter(BestChoice->permuter(), false);
  bool is_case_ok = case_ok(*BestChoice, getUnicharset());

  if (stopper_debug_level >= 1)
    tprintf("\nStopper:  %s (word=%c, case=%c)\n",
            BestChoice->debug_string().string(),
            (is_valid_word ? 'y' : 'n'),
            (is_case_ok ? 'y' : 'n'));

  // Do not accept invalid words in PASS1.
  if (reject_offset_ <= 0.0f && !is_valid_word) return false;
  if (is_valid_word && is_case_ok) {
    WordSize = LengthOfShortestAlphaRun(*BestChoice);
    WordSize -= stopper_smallword_size;
    if (WordSize < 0)
      WordSize = 0;
    CertaintyThreshold += WordSize * stopper_certainty_per_char;
  }

  if (stopper_debug_level >= 1)
    tprintf("Stopper:  Certainty = %4.1f, Threshold = %4.1f\n",
            BestChoice->certainty(), CertaintyThreshold);

  if (no_dang_ambigs &&
      BestChoice->certainty() > CertaintyThreshold &&
      UniformCertainties(*Choices, *BestChoice)) {
    return true;
  } else {
    if (stopper_debug_level >= 2) {
      tprintf("AcceptableChoice() returned false"
              " (no_dang_ambig:%d cert:%g thresh:%g uniform:%d)\n",
              no_dang_ambigs, BestChoice->certainty(),
              CertaintyThreshold,
              UniformCertainties(*Choices, *BestChoice));
    }
    return false;
  }
}

bool Dict::AcceptableResult(const WERD_CHOICE &BestChoice) {
  float CertaintyThreshold = stopper_nondict_certainty_base - reject_offset_;
  int WordSize;

  if (stopper_debug_level >= 1) {
    tprintf("\nRejecter: %s (word=%c, case=%c, unambig=%c)\n",
            BestChoice.debug_string().string(),
            (valid_word(BestChoice) ? 'y' : 'n'),
            (case_ok(BestChoice, getUnicharset()) ? 'y' : 'n'),
            ((list_rest (best_choices_) != NIL_LIST) ? 'n' : 'y'));
  }

  if (BestChoice.length() == 0 || CurrentWordAmbig())
    return false;
  if (BestChoice.fragment_mark()) {
    if (stopper_debug_level >= 1) {
      cprintf("AcceptableResult(): a choice with fragments beats BestChoice\n");
    }
    return false;
  }
  if (valid_word(BestChoice) && case_ok(BestChoice, getUnicharset())) {
    WordSize = LengthOfShortestAlphaRun(BestChoice);
    WordSize -= stopper_smallword_size;
    if (WordSize < 0)
      WordSize = 0;
    CertaintyThreshold += WordSize * stopper_certainty_per_char;
  }

  if (stopper_debug_level >= 1)
    cprintf ("Rejecter: Certainty = %4.1f, Threshold = %4.1f   ",
      BestChoice.certainty(), CertaintyThreshold);

  if (BestChoice.certainty() > CertaintyThreshold &&
      !stopper_no_acceptable_choices) {
    if (stopper_debug_level >= 1)
      cprintf("ACCEPTED\n");
    return true;
  }
  else {
    if (stopper_debug_level >= 1)
      cprintf("REJECTED\n");
    return false;
  }
}

bool Dict::AlternativeChoicesWorseThan(FLOAT32 Threshold) {
  LIST Alternatives;
  VIABLE_CHOICE Choice;
  Alternatives = list_rest (best_choices_);
  iterate(Alternatives) {
    Choice = (VIABLE_CHOICE) first_node (Alternatives);
    if (Choice->AdjustFactor <= Threshold)
      return false;
  }
  return true;
}

bool Dict::CurrentBestChoiceIs(const WERD_CHOICE &WordChoice) {
  return (best_choices_ != NIL_LIST &&
          StringSameAs(WordChoice, (VIABLE_CHOICE)first_node(best_choices_)));
}

FLOAT32 Dict::CurrentBestChoiceAdjustFactor() {
  VIABLE_CHOICE BestChoice;
  if (best_choices_ == NIL_LIST)
    return (MAX_FLOAT32);
  BestChoice = (VIABLE_CHOICE) first_node (best_choices_);
  return (BestChoice->AdjustFactor);
}


bool Dict::CurrentWordAmbig() {
  return (list_rest (best_choices_) != NIL_LIST);
}


void Dict::DebugWordChoices() {
  LIST Choices;
  int i;
  char LabelString[80];
  VIABLE_CHOICE VChoice = (VIABLE_CHOICE)first_node(best_choices_);
  bool force_debug =
    fragments_debug && VChoice != NULL && VChoice->ComposedFromCharFragments;

  if (stopper_debug_level >= 1 || force_debug ||
  (((STRING)word_to_debug).length() > 0 && best_choices_ &&
       StringSameAs(word_to_debug.string(), word_to_debug_lengths.string(),
                    (VIABLE_CHOICE)first_node(best_choices_)))) {
    if (best_raw_choice_)
      PrintViableChoice(stderr, "\nBest Raw Choice:   ", best_raw_choice_);

    i = 1;
    Choices = best_choices_;
    if (Choices)
      cprintf("\nBest Cooked Choices:\n");
    iterate(Choices) {
      sprintf(LabelString, "Cooked Choice #%d:  ", i);
      PrintViableChoice(stderr, LabelString,
                        (VIABLE_CHOICE)first_node(Choices));
      i++;
    }
  }
}

void Dict::PrintAmbigAlternatives(FILE *file, const char *label,
                                  int label_num_unichars) {
  iterate(raw_choices_) {
    VIABLE_CHOICE Choice = (VIABLE_CHOICE)first_node(raw_choices_);
    if (Choice->Length > 0 &&
        (label_num_unichars > 1 || Choice->Length > 1)) {
      for (int i = 0; i < Choice->Length; i++) {
        fprintf(file, "%s",
                getUnicharset().id_to_unichar(Choice->Blob[i].Class));
      }
      fflush(file);
      fprintf(file, "\t%s\t%.4f\t%.4f\n", label,
              Choice->Rating, Choice->Certainty);
    }
  }
}

void Dict::FilterWordChoices() {
  EXPANDED_CHOICE BestChoice;

  if (best_choices_ == NIL_LIST || second_node (best_choices_) == NIL_LIST)
    return;

  // Compute certainties and class for each chunk in best choice.
  VIABLE_CHOICE_STRUCT *best_choice =
      (VIABLE_CHOICE_STRUCT *)first_node(best_choices_);
  ExpandChoice(best_choice, &BestChoice);
  if (stopper_debug_level >= 2)
    PrintViableChoice(stderr, "\nFiltering against best choice: ", best_choice);
  TessResultCallback2<int, void*, void*>* is_bad =
      NewPermanentTessCallback(this, &Dict::FreeBadChoice);
  set_rest(best_choices_, delete_d(list_rest(best_choices_),
                                   &BestChoice, is_bad));
  delete is_bad;
}

void Dict::FindClassifierErrors(FLOAT32 MinRating,
                                FLOAT32 MaxRating,
                                FLOAT32 RatingMargin,
                                FLOAT32 Thresholds[]) {
  EXPANDED_CHOICE BestRaw;
  VIABLE_CHOICE Choice;
  int i, j, Chunk;
  FLOAT32 AvgRating;
  int NumErrorChunks;

  assert (best_choices_ != NIL_LIST);
  assert (best_raw_choice_ != NULL);

  ExpandChoice(best_raw_choice_, &BestRaw);
  Choice = (VIABLE_CHOICE) first_node (best_choices_);

  for (i = 0, Chunk = 0; i < Choice->Length; i++, Thresholds++) {
    AvgRating = 0.0;
    NumErrorChunks = 0;

    for (j = 0; j < Choice->Blob[i].NumChunks; j++, Chunk++) {
      if (Choice->Blob[i].Class != BestRaw.ChunkClass[Chunk]) {
        AvgRating += BestRaw.ChunkCertainty[Chunk];
        NumErrorChunks++;
      }
    }

    if (NumErrorChunks > 0) {
      AvgRating /= NumErrorChunks;
      *Thresholds = (AvgRating / -certainty_scale) * (1.0 - RatingMargin);
    }
    else
      *Thresholds = MaxRating;

    if (*Thresholds > MaxRating)
      *Thresholds = MaxRating;
    if (*Thresholds < MinRating)
      *Thresholds = MinRating;
  }
}

void Dict::InitChoiceAccum() {
  BLOB_WIDTH *BlobWidth, *End;

  if (best_raw_choice_)
    delete best_raw_choice_;
  best_raw_choice_ = NULL;

  if (best_choices_)
    destroy_nodes(best_choices_, DeleteViableChoiceStruct);
  best_choices_ = NIL_LIST;

  if (raw_choices_)
    destroy_nodes(raw_choices_, DeleteViableChoiceStruct);
  raw_choices_ = NIL_LIST;

  EnableChoiceAccum();

  for (BlobWidth = current_segmentation_,
    End = current_segmentation_ + MAX_NUM_CHUNKS;
    BlobWidth < End; *BlobWidth++ = 1);

}

void Dict::ClearBestChoiceAccum() {
  if (best_choices_) destroy_nodes(best_choices_, DeleteViableChoiceStruct);
  best_choices_ = NIL_LIST;
}

void Dict::LogNewSegmentation(PIECES_STATE BlobWidth) {
  BLOB_WIDTH *Segmentation;
  for (Segmentation = current_segmentation_; *BlobWidth != 0;
    BlobWidth++, Segmentation++)
  *Segmentation = *BlobWidth;
  *Segmentation = 0;
}

void Dict::LogNewSplit(int Blob) {
  LIST Choices;
  if (best_raw_choice_) AddNewChunk(best_raw_choice_, Blob);
  Choices = best_choices_;
  iterate(Choices) {
    AddNewChunk ((VIABLE_CHOICE) first_node (Choices), Blob);
  }
  Choices = raw_choices_;
  iterate(Choices) {
    AddNewChunk ((VIABLE_CHOICE) first_node (Choices), Blob);
  }
}

void Dict::LogNewChoice(FLOAT32 AdjustFactor,
                        const float Certainties[],
                        bool raw_choice,
                        WERD_CHOICE *WordChoice,
                        const BLOB_CHOICE_LIST_VECTOR &blob_choices) {
  LIST ChoicesList;
  LIST Choices;
  FLOAT32 Threshold;

  if (!keep_word_choices_)
    return;

  if (raw_choice) {
    if (!best_raw_choice_) {
      best_raw_choice_ =
          NewViableChoice(*WordChoice, AdjustFactor, Certainties);
    } else if (WordChoice->rating() < best_raw_choice_->Rating) {
      if (ChoiceSameAs(*WordChoice, best_raw_choice_)) {
        FillViableChoice(*WordChoice, AdjustFactor, Certainties,
                         best_raw_choice_);
      } else {
        delete best_raw_choice_;
        best_raw_choice_ =
          NewViableChoice(*WordChoice, AdjustFactor, Certainties);
      }
    }
    if (!save_raw_choices) return;
    ChoicesList = raw_choices_;
  } else {
    ChoicesList = best_choices_;
  }

  // Throw out obviously bad choices to save some work.
  if (ChoicesList != NIL_LIST) {
    Threshold = StopperAmbigThreshold(BestFactor(ChoicesList), AdjustFactor);
    if (Threshold > -stopper_ambiguity_threshold_offset)
      Threshold = -stopper_ambiguity_threshold_offset;
    if (WordChoice->certainty() - BestCertainty (ChoicesList) < Threshold) {
      // Set the rating of the word to be terrible, so that it does not
      // get chosen as the best choice.
      if (stopper_debug_level >= 2) {
        STRING bad_string;
        WordChoice->string_and_lengths(&bad_string, NULL);
        tprintf("Discarding choice \"%s\" with an overly low certainty"
                " %.4f vs best choice certainty %.4f (Threshold: %.4f)\n",
                bad_string.string(), WordChoice->certainty(),
                BestCertainty(ChoicesList),
                Threshold + BestCertainty(ChoicesList));
      }
      WordChoice->set_rating(WERD_CHOICE::kBadRating);
      return;
    }
  }

  // See if a choice with the same text string has already been found.
  VIABLE_CHOICE NewChoice = NULL;
  Choices = ChoicesList;

  iterate(Choices) {
    if (ChoiceSameAs (*WordChoice, (VIABLE_CHOICE) first_node (Choices))) {
      if (WordChoice->rating() < BestRating (Choices)) {
        NewChoice = (VIABLE_CHOICE) first_node (Choices);
      } else {
        return;
      }
    }
  }

  if (NewChoice) {
    FillViableChoice(*WordChoice, AdjustFactor, Certainties, NewChoice);
    ChoicesList = delete_d(ChoicesList, NewChoice, is_same_node);
  } else {
    NewChoice = NewViableChoice(*WordChoice, AdjustFactor, Certainties);
  }

  // Now we know we're gonna save it, so add the expensive copy.
  NewChoice->SetBlobChoices(blob_choices);

  ChoicesList = s_adjoin (ChoicesList, NewChoice, CmpChoiceRatings);
  if (stopper_debug_level >= 2)
    raw_choice ? PrintViableChoice (stderr, "New Raw Choice:  ", NewChoice) :
      PrintViableChoice (stderr, "New Word Choice:  ", NewChoice);
  if (count (ChoicesList) > tessedit_truncate_wordchoice_log) {
    Choices =
      (LIST) nth_cell (ChoicesList, tessedit_truncate_wordchoice_log);
    destroy_nodes(list_rest (Choices), DeleteViableChoiceStruct);
    set_rest(Choices, NIL_LIST);
  }

  // Update raw_choices_/best_choices_ pointer.
  if (raw_choice) {
    raw_choices_ = ChoicesList;
  } else {
    best_choices_ = ChoicesList;
  }
}

bool Dict::NoDangerousAmbig(WERD_CHOICE *best_choice,
                            DANGERR *fixpt,
                            bool fix_replaceable,
                            BLOB_CHOICE_LIST_VECTOR *blob_choices,
                            bool *modified_blobs) {
  if (stopper_debug_level > 2) {
    tprintf("\nRunning NoDangerousAmbig() for %s\n",
            best_choice->debug_string().string());
  }

  // Construct BLOB_CHOICE_LIST_VECTOR with ambiguities
  // for each unichar id in BestChoice.
  BLOB_CHOICE_LIST_VECTOR ambig_blob_choices;
  int i;
  bool modified_best_choice = false;
  bool ambigs_found = false;
  // For each position in best_choice:
  // -- choose AMBIG_SPEC_LIST that corresponds to unichar_id at best_choice[i]
  // -- initialize wrong_ngram with a single unichar_id at best_choice[i]
  // -- look for ambiguities corresponding to wrong_ngram in the list while
  //    adding the following unichar_ids from best_choice to wrong_ngram
  //
  // Repeat the above procedure twice: first time look through
  // ambigs to be replaced and replace all the ambiguities found;
  // second time look through dangerous ambiguities and construct
  // ambig_blob_choices with fake a blob choice for each ambiguity
  // and pass them to dawg_permute_and_select() to search for
  // ambiguous words in the dictionaries.
  //
  // Note that during the execution of the for loop (on the first pass)
  // if replacements are made the length of best_choice might change.
  for (int pass = 0; pass < (fix_replaceable ? 2 : 1); ++pass) {
    bool replace = (fix_replaceable && pass == 0);
    const UnicharAmbigsVector &table = replace ?
      getUnicharAmbigs().replace_ambigs() : getUnicharAmbigs().dang_ambigs();
    if (!replace) {
      // Initialize ambig_blob_choices with lists containing a single
      // unichar id for the correspoding position in best_choice.
      // best_choice consisting from only the original letters will
      // have a rating of 0.0.
      for (i = 0; i < best_choice->length(); ++i) {
        BLOB_CHOICE_LIST *lst = new BLOB_CHOICE_LIST();
        BLOB_CHOICE_IT lst_it(lst);
        // TODO(rays/antonova) Should these BLOB_CHOICEs use real xheights
        // or are these fake ones good enough?
        lst_it.add_to_end(new BLOB_CHOICE(best_choice->unichar_id(i),
                                          0.0, 0.0, -1, -1, -1, 0, 1, false));
        ambig_blob_choices.push_back(lst);
      }
    }
    UNICHAR_ID wrong_ngram[MAX_AMBIG_SIZE + 1];
    int wrong_ngram_index;
    int next_index;
    int blob_index = 0;
    for (i = 0; i < best_choice->length(); ++i) {
      if (i > 0) blob_index += best_choice->fragment_length(i-1);
      UNICHAR_ID curr_unichar_id = best_choice->unichar_id(i);
      if (stopper_debug_level > 2) {
        tprintf("Looking for %s ngrams starting with %s:\n",
                replace ? "replaceable" : "ambiguous",
                getUnicharset().debug_str(curr_unichar_id).string());
      }
      wrong_ngram_index = 0;
      wrong_ngram[wrong_ngram_index] = curr_unichar_id;
      if (curr_unichar_id == INVALID_UNICHAR_ID ||
          curr_unichar_id >= table.size() ||
          table[curr_unichar_id] == NULL) {
        continue;  // there is no ambig spec for this unichar id
      }
      AmbigSpec_IT spec_it(table[curr_unichar_id]);
      for (spec_it.mark_cycle_pt(); !spec_it.cycled_list();) {
        const AmbigSpec *ambig_spec = spec_it.data();
        wrong_ngram[wrong_ngram_index+1] = INVALID_UNICHAR_ID;
        int compare = UnicharIdArrayUtils::compare(wrong_ngram,
                                                   ambig_spec->wrong_ngram);
        if (stopper_debug_level > 2) {
          tprintf("candidate ngram: ");
          UnicharIdArrayUtils::print(wrong_ngram, getUnicharset());
          tprintf("current ngram from spec: ");
          UnicharIdArrayUtils::print(ambig_spec->wrong_ngram, getUnicharset());
          tprintf("comparison result: %d\n", compare);
        }
        if (compare == 0) {
          // Record the place where we found an ambiguity.
          if (fixpt != NULL) {
            fixpt->push_back(DANGERR_INFO(
                blob_index, blob_index+wrong_ngram_index, replace,
                getUnicharset().get_isngram(ambig_spec->correct_ngram_id)));
            if (stopper_debug_level > 1) {
              tprintf("fixpt+=(%d %d %d %d)\n", blob_index,
                      blob_index+wrong_ngram_index, false,
                      getUnicharset().get_isngram(
                          ambig_spec->correct_ngram_id));
            }
          }

          if (replace) {
            if (stopper_debug_level > 2) {
              tprintf("replace ambiguity with: ");
              UnicharIdArrayUtils::print(
                  ambig_spec->correct_fragments, getUnicharset());
            }
            ReplaceAmbig(i, ambig_spec->wrong_ngram_size,
                         ambig_spec->correct_ngram_id,
                         best_choice, blob_choices, modified_blobs);
            modified_best_choice = true;
          } else if (i > 0 || ambig_spec->type != CASE_AMBIG) {
            // We found dang ambig - update ambig_blob_choices.
            if (stopper_debug_level > 2) {
              tprintf("found ambiguity: ");
              UnicharIdArrayUtils::print(
                  ambig_spec->correct_fragments, getUnicharset());
            }
            ambigs_found = true;
            for (int tmp_index = 0; tmp_index <= wrong_ngram_index;
                 ++tmp_index) {
              // Add a blob choice for the corresponding fragment of the
              // ambiguity. These fake blob choices are initialized with
              // negative ratings (which are not possible for real blob
              // choices), so that dawg_permute_and_select() considers any
              // word not consisting of only the original letters a better
              // choice and stops searching for alternatives once such a
              // choice is found.
              BLOB_CHOICE_IT bc_it(ambig_blob_choices[i+tmp_index]);
              bc_it.add_to_end(new BLOB_CHOICE(
                  ambig_spec->correct_fragments[tmp_index], -1.0, 0.0,
                  -1, -1, -1, 0, 1, false));
            }
          }
          spec_it.forward();
        } else if (compare == -1) {
          if (wrong_ngram_index+1 < ambig_spec->wrong_ngram_size &&
              ((next_index = wrong_ngram_index+1+i) < best_choice->length())) {
            // Add the next unichar id to wrong_ngram and keep looking for
            // more ambigs starting with curr_unichar_id in AMBIG_SPEC_LIST.
            wrong_ngram[++wrong_ngram_index] =
              best_choice->unichar_id(next_index);
          } else {
            break;  // no more matching ambigs in this AMBIG_SPEC_LIST
          }
        } else {
          spec_it.forward();
        }
      }  // end searching AmbigSpec_LIST
    }  // end searching best_choice
  }  // end searching replace and dangerous ambigs

  // If any ambiguities were found permute the constructed ambig_blob_choices
  // to see if an alternative dictionary word can be found.
  if (ambigs_found) {
    if (stopper_debug_level > 2) {
      tprintf("\nResulting ambig_blob_choices:\n");
      for (i = 0; i < ambig_blob_choices.length(); ++i) {
        print_ratings_list("", ambig_blob_choices.get(i), getUnicharset());
        tprintf("\n");
      }
    }
    WERD_CHOICE *alt_word = dawg_permute_and_select(ambig_blob_choices, 0.0);
    ambigs_found = (alt_word->rating() < 0.0);
    if (ambigs_found) {
      if (stopper_debug_level >= 1) {
        tprintf ("Stopper: Possible ambiguous word = %s\n",
                 alt_word->debug_string().string());
      }
      if (fixpt != NULL) {
        // Note: Currently character choices combined from fragments can only
        // be generated by NoDangrousAmbigs(). This code should be updated if
        // the capability to produce classifications combined from character
        // fragments is added to other functions.
        int orig_i = 0;
        for (i = 0; i < alt_word->length(); ++i) {
          bool replacement_is_ngram =
              getUnicharset().get_isngram(alt_word->unichar_id(i));
          int end_i = orig_i + alt_word->fragment_length(i) - 1;
          if (alt_word->fragment_length(i) > 1 ||
              (orig_i == end_i && replacement_is_ngram)) {
            fixpt->push_back(DANGERR_INFO(orig_i, end_i, true,
                                          replacement_is_ngram));
             if (stopper_debug_level > 1) {
               tprintf("fixpt->dangerous+=(%d %d %d %d)\n", orig_i, end_i,
                       true, replacement_is_ngram);
             }
          }
          orig_i += alt_word->fragment_length(i);
        }
      }
    }
    delete alt_word;
  }
  if (output_ambig_words_file_ != NULL) {
    fprintf(output_ambig_words_file_, "\n");
  }

  ambig_blob_choices.delete_data_pointers();
  return !ambigs_found;
}

void Dict::EndDangerousAmbigs() {}

void Dict::SettupStopperPass1() {
  reject_offset_ = 0.0;
}

void Dict::SettupStopperPass2() {
  reject_offset_ = stopper_phase2_certainty_rejection_offset;
}

void Dict::AddNewChunk(VIABLE_CHOICE Choice, int Blob) {
  int i, LastChunk;
  for (i = 0, LastChunk = 0; i < Choice->Length; i++) {
    LastChunk += Choice->Blob[i].NumChunks;
    if (Blob < LastChunk) {
      (Choice->Blob[i].NumChunks)++;
      return;
    }
  }
  cprintf ("AddNewChunk failed:Choice->Length=%d, LastChunk=%d, Blob=%d\n",
           Choice->Length, LastChunk, Blob);
  assert(false);  // this should never get executed
}

void Dict::ReplaceAmbig(int wrong_ngram_begin_index, int wrong_ngram_size,
                        UNICHAR_ID correct_ngram_id, WERD_CHOICE *werd_choice,
                        BLOB_CHOICE_LIST_VECTOR *blob_choices,
                        bool *modified_blobs) {
  int num_blobs_to_replace = 0;
  int begin_blob_index = 0;
  int i;
  for (i = 0; i < wrong_ngram_begin_index + wrong_ngram_size; ++i) {
    if (i >= wrong_ngram_begin_index) {
      num_blobs_to_replace +=  werd_choice->fragment_length(i);
    } else {
      begin_blob_index += werd_choice->fragment_length(i);
    }
  }
  BLOB_CHOICE_IT bit;
  int temp_blob_index = begin_blob_index;
  const char *temp_uch = NULL;
  const char *correct_ngram_str =
    getUnicharset().id_to_unichar(correct_ngram_id);
  for (int replaced_count = 0; replaced_count < wrong_ngram_size;
       ++replaced_count) {
    if (blob_choices != NULL) {
      UNICHAR_ID uch_id = werd_choice->unichar_id(wrong_ngram_begin_index);
      int fraglen = werd_choice->fragment_length(wrong_ngram_begin_index);
      if (fraglen > 1) temp_uch = getUnicharset().id_to_unichar(uch_id);
      for (i = 0; i < fraglen; ++i) {
        if (fraglen > 1) {
          STRING frag_str =
            CHAR_FRAGMENT::to_string(temp_uch, i, fraglen, false);
          getUnicharset().unichar_insert(frag_str.string());
          uch_id = getUnicharset().unichar_to_id(frag_str.string());
        }
        bit.set_to_list(blob_choices->get(temp_blob_index));
        STRING correct_frag_uch =
          CHAR_FRAGMENT::to_string(correct_ngram_str,
                                   temp_blob_index - begin_blob_index,
                                   num_blobs_to_replace, false);
        getUnicharset().unichar_insert(correct_frag_uch.string());
        UNICHAR_ID correct_frag_uch_id =
          getUnicharset().unichar_to_id(correct_frag_uch.string());
        // Find the WERD_CHOICE corresponding to the original unichar in
        // the list of blob choices, add the derived character fragment
        // before it with the same rating and certainty.
        for (bit.mark_cycle_pt(); !bit.cycled_list(); bit.forward()) {
          if (bit.data()->unichar_id() == correct_frag_uch_id) {
            break;  // the unichar we want to insert is already there
          }
          if (bit.data()->unichar_id() == uch_id) {
            bit.add_before_then_move(new BLOB_CHOICE(*(bit.data())));
            bit.data()->set_unichar_id(correct_frag_uch_id);
            if (modified_blobs != NULL) *modified_blobs = true;
            break;
          }
        }
        temp_blob_index++;
      }
    }
    // Remove current unichar from werd_choice. On the last iteration
    // set the correct replacement unichar instead of removing a unichar.
    if (replaced_count + 1 == wrong_ngram_size) {
      werd_choice->set_unichar_id(correct_ngram_id,
          num_blobs_to_replace, 0.0, 0.0, wrong_ngram_begin_index);
    } else {
      werd_choice->remove_unichar_id(wrong_ngram_begin_index);
    }
  }
  if (stopper_debug_level >= 1 && modified_blobs != NULL &&
      *modified_blobs && blob_choices != NULL) {
      werd_choice->print("ReplaceAmbig() ");
      tprintf("Modified blob_choices: ");
      for (int i = 0; i < blob_choices->size(); ++i) {
        print_ratings_list("\n", blob_choices->get(i), getUnicharset());
    }
  }
}

int Dict::ChoiceSameAs(const WERD_CHOICE &WordChoice,
                       VIABLE_CHOICE ViableChoice) {
  return (StringSameAs(WordChoice, ViableChoice));
}

int Dict::LengthOfShortestAlphaRun(const WERD_CHOICE &WordChoice) {
  int shortest = MAX_INT32;
  int curr_len = 0;
  for (int w = 0; w < WordChoice.length(); ++w) {
    if (getUnicharset().get_isalpha(WordChoice.unichar_id(w))) {
      curr_len++;
    } else if (curr_len > 0) {
      if (curr_len < shortest) shortest = curr_len;
      curr_len = 0;
    }
  }
  if (curr_len > 0 && curr_len < shortest) {
    shortest = curr_len;
  } else if (shortest == MAX_INT32) {
    shortest = 0;
  }
  return shortest;
}

VIABLE_CHOICE Dict::NewViableChoice(const WERD_CHOICE &WordChoice,
                                    FLOAT32 AdjustFactor,
                                    const float Certainties[]) {
  int Length = WordChoice.length();
  assert (Length <= MAX_NUM_CHUNKS && Length > 0);
  VIABLE_CHOICE NewChoice = new VIABLE_CHOICE_STRUCT(Length);
  FillViableChoice(WordChoice, AdjustFactor, Certainties, NewChoice);
  return NewChoice;
}

void Dict::PrintViableChoice(FILE *File, const char *Label, VIABLE_CHOICE Choice) {
  int i, j;
  fprintf (File, "%s", Label);
  fprintf(File, "(R=%5.1f, C=%4.1f, F=%4.2f, Frag=%d)  ",
    Choice->Rating, Choice->Certainty,
    Choice->AdjustFactor, Choice->ComposedFromCharFragments);

  for (i = 0; i < Choice->Length; i++)
    fprintf(File, "%s", getUnicharset().id_to_unichar(Choice->Blob[i].Class));
  fprintf(File, "\n");

  for (i = 0; i < Choice->Length; i++) {
    fprintf(File, "  %s", getUnicharset().id_to_unichar(Choice->Blob[i].Class));
    for (j = 0; j < Choice->Blob[i].NumChunks - 1; j++)
      fprintf(File, "    ");
  }
  fprintf(File, "\n");

  for (i = 0; i < Choice->Length; i++) {
    for (j = 0; j < Choice->Blob[i].NumChunks; j++)
      fprintf(File, "%3d ", (int) (Choice->Blob[i].Certainty * -10.0));
  }
  fprintf(File, "\n");

  for (i = 0; i < Choice->Length; i++) {
    for (j = 0; j < Choice->Blob[i].NumChunks; j++)
      fprintf(File, "%3d ", Choice->Blob[i].NumChunks);
  }
  fprintf(File, "\n");
}

void Dict::FillViableChoice(const WERD_CHOICE &WordChoice,
                            FLOAT32 AdjustFactor, const float Certainties[],
                            VIABLE_CHOICE ViableChoice) {
  ViableChoice->Init(WordChoice, current_segmentation_, Certainties,
                     AdjustFactor);

}

bool Dict::StringSameAs(const WERD_CHOICE &WordChoice,
                        VIABLE_CHOICE ViableChoice) {
  if (WordChoice.length() != ViableChoice->Length) {
    return false;
  }
  int i;
  CHAR_CHOICE *CharChoice;
  for (i = 0, CharChoice = &(ViableChoice->Blob[0]);
       i < ViableChoice->Length; CharChoice++, i++) {
    if (CharChoice->Class != WordChoice.unichar_id(i)) {
      return false;
    }
  }
  return true;
}

bool Dict::StringSameAs(const char *String,
                        const char *String_lengths,
                        VIABLE_CHOICE ViableChoice) {
  CHAR_CHOICE *Char;
  int i;
  int current_unichar_length;

  for (Char = &(ViableChoice->Blob[0]), i = 0;
    i < ViableChoice->Length;
       String += *(String_lengths++), Char++, i++) {
    current_unichar_length = strlen(getUnicharset().id_to_unichar(Char->Class));
  if (current_unichar_length != *String_lengths ||
      strncmp(String, getUnicharset().id_to_unichar(Char->Class),
              current_unichar_length) != 0)
    return false;
  }
  return (*String == 0) ? true : false;
}

int Dict::UniformCertainties(const BLOB_CHOICE_LIST_VECTOR &Choices,
                             const WERD_CHOICE &BestChoice) {
  float Certainty;
  float WorstCertainty = MAX_FLOAT32;
  float CertaintyThreshold;
  FLOAT64 TotalCertainty;
  FLOAT64 TotalCertaintySquared;
  FLOAT64 Variance;
  FLOAT32 Mean, StdDev;
  int WordLength;

  WordLength = Choices.length();
  if (WordLength < 3)
    return true;

  TotalCertainty = TotalCertaintySquared = 0.0;
  BLOB_CHOICE_IT BlobChoiceIt;
  for (int i = 0; i < Choices.length(); ++i) {
    BlobChoiceIt.set_to_list(Choices.get(i));
    Certainty = BlobChoiceIt.data()->certainty();
    TotalCertainty += Certainty;
    TotalCertaintySquared += Certainty * Certainty;
    if (Certainty < WorstCertainty)
      WorstCertainty = Certainty;
  }

  // Subtract off worst certainty from statistics.
  WordLength--;
  TotalCertainty -= WorstCertainty;
  TotalCertaintySquared -= WorstCertainty * WorstCertainty;

  Mean = TotalCertainty / WordLength;
  Variance = ((WordLength * TotalCertaintySquared -
    TotalCertainty * TotalCertainty) /
    (WordLength * (WordLength - 1)));
  if (Variance < 0.0)
    Variance = 0.0;
  StdDev = sqrt (Variance);

  CertaintyThreshold = Mean - stopper_allowable_character_badness * StdDev;
  if (CertaintyThreshold > stopper_nondict_certainty_base)
    CertaintyThreshold = stopper_nondict_certainty_base;

  if (BestChoice.certainty() < CertaintyThreshold) {
    if (stopper_debug_level >= 1)
      cprintf("Stopper: Non-uniform certainty = %4.1f"
              " (m=%4.1f, s=%4.1f, t=%4.1f)\n",
              BestChoice.certainty(), Mean, StdDev, CertaintyThreshold);
    return false;
  } else {
    return true;
  }
}

} // namespace tesseract
