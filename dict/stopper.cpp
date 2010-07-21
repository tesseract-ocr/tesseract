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
/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "stopper.h"
#include "emalloc.h"
#include "matchdefs.h"
#include "callcpp.h"
#include "permute.h"
#include "context.h"
#include "danerror.h"
#include "const.h"
#include "freelist.h"
#include "efio.h"
#include "scanutils.h"
#include "unichar.h"
#include "varable.h"
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

/**----------------------------------------------------------------------------
          Macros
----------------------------------------------------------------------------**/
#define BestCertainty(Choices)  (((VIABLE_CHOICE) first_node (Choices))->Certainty)
#define BestRating(Choices) (((VIABLE_CHOICE) first_node (Choices))->Rating)
#define BestFactor(Choices) (((VIABLE_CHOICE) first_node (Choices))->AdjustFactor)

#define AmbigThreshold(F1,F2)	(((F2) - (F1)) * stopper_ambiguity_threshold_gain - \
				stopper_ambiguity_threshold_offset)

/*---------------------------------------------------------------------------
          Private Function Prototoypes
----------------------------------------------------------------------------*/
void AddNewChunk(VIABLE_CHOICE Choice, int Blob);

int CmpChoiceRatings(void *arg1,   //VIABLE_CHOICE         Choice1,
                     void *arg2);  //VIABLE_CHOICE         Choice2);

void ExpandChoice(VIABLE_CHOICE Choice, EXPANDED_CHOICE *ExpandedChoice);

int FreeBadChoice(void *item1,   //VIABLE_CHOICE                 Choice,
                  void *item2);  //EXPANDED_CHOICE                       *BestChoice);

int UniformCertainties(const BLOB_CHOICE_LIST_VECTOR &Choices,
                       const WERD_CHOICE &BestChoice);

/**----------------------------------------------------------------------
                     V a r i a b l e s
----------------------------------------------------------------------**/
double_VAR(certainty_scale, 20.0, "Certainty scaling factor");

double_VAR(stopper_nondict_certainty_base, -2.50,
           "Certainty threshold for non-dict words");

double_VAR(stopper_phase2_certainty_rejection_offset, 1.0,
           "Reject certainty offset");

INT_VAR(stopper_smallword_size, 2,
        "Size of dict word to be treated as non-dict word");

double_VAR(stopper_certainty_per_char, -0.50,
           "Certainty to add for each dict char above small word size.");

double_VAR(stopper_allowable_character_badness, 3.0,
           "Max certaintly variation allowed in a word (in sigma)");

INT_VAR(stopper_debug_level, 0, "Stopper debug level");

double_VAR(stopper_ambiguity_threshold_gain, 8.0,
           "Gain factor for ambiguity threshold");

double_VAR(stopper_ambiguity_threshold_offset, 1.5,
           "Certainty offset for ambiguity threshold");

BOOL_VAR(stopper_no_acceptable_choices, false,
         "Make AcceptableChoice() always return false. Useful"
         " when there is a need to explore all segmentations");

BOOL_VAR(save_raw_choices, false, "Save all explored raw choices");

INT_VAR (tessedit_truncate_wordchoice_log, 10, "Max words to keep in list");

STRING_VAR(word_to_debug, "", "Word for which stopper debug information"
           " should be printed to stdout");

STRING_VAR(word_to_debug_lengths, "", "Lengths of unichars in word_to_debug");

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
namespace tesseract {
int Dict::AcceptableChoice(BLOB_CHOICE_LIST_VECTOR *Choices,
                           WERD_CHOICE *BestChoice,
                           const WERD_CHOICE &RawChoice,
                           DANGERR *fixpt,
                           ACCEPTABLE_CHOICE_CALLER caller,
                           bool *modified_blobs) {
/*
 **	Parameters:
 **		Choices		choices for current segmentation
 **		BestChoice	best choice for current segmentation
 **		RawChoice	best raw choice for current segmentation
 **	Variables Used:
 **		stopper_nondict_certainty_base	certainty for a non-dict word
 **		stopper_smallword_size		size of word to be treated as non-word
 **		stopper_certainty_per_char	certainty to add for each dict char
 **	Operation: Return TRUE if the results from this segmentation are
 **		good enough to stop.  Otherwise return FALSE.
 **	Return: TRUE or FALSE.
 **	Exceptions: none
 **	History: Mon Apr 29 14:57:32 1991, DSJ, Created.
 */
  float CertaintyThreshold = stopper_nondict_certainty_base;
  int WordSize;

  if (stopper_no_acceptable_choices) return false;

  if (fixpt != NULL)
    fixpt->index = -1;
  if (BestChoice->length() == 0)
    return (FALSE);
  if (caller == CHOPPER_CALLER && BestChoice->fragment_mark()) {
    if (stopper_debug_level >= 1) {
      cprintf("AcceptableChoice(): a choice with fragments beats BestChoice");
    }
    return false;
  }

  bool no_dang_ambigs =
    NoDangerousAmbig(BestChoice, fixpt, true, Choices, modified_blobs);

  if (stopper_debug_level >= 1)
    tprintf("\nStopper:  %s (word=%c, case=%c)\n",
            BestChoice->debug_string(getUnicharset()).string(),
            (valid_word(*BestChoice) ? 'y' : 'n'),
            (Context::case_ok(*BestChoice, getUnicharset()) ? 'y' : 'n'));

  if (valid_word(*BestChoice) &&
      Context::case_ok(*BestChoice, getUnicharset())) {
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
    return (TRUE);
  } else {
    return (FALSE);
  }
}                                /* AcceptableChoice */


/*---------------------------------------------------------------------------*/
int Dict::AcceptableResult(const WERD_CHOICE &BestChoice,
                           const WERD_CHOICE &RawChoice) {
/*
 **	Parameters:
 **		BestChoice	best choice for current word
 **		RawChoice	best raw choice for current word
 **	Variables Used:
 **		stopper_nondict_certainty_base	certainty for a non-dict word
 **		stopper_smallword_size		size of word to be treated as non-word
 **		stopper_certainty_per_char	certainty to add for each dict char
 **		best_choices_		list of all good choices found
 **		reject_offset_		allowed offset before a word is rejected
 **	Operation: Return FALSE if the best choice for the current word
 **		is questionable and should be tried again on the second
 **		pass or should be flagged to the user.
 **	Return: TRUE or FALSE.
 **	Exceptions: none
 **	History: Thu May  9 14:05:05 1991, DSJ, Created.
 */
  float CertaintyThreshold = stopper_nondict_certainty_base - reject_offset_;
  int WordSize;

  if (stopper_debug_level >= 1) {
    tprintf("\nRejecter: %s (word=%c, case=%c, unambig=%c)\n",
            BestChoice.debug_string(getUnicharset()).string(),
            (valid_word(BestChoice) ? 'y' : 'n'),
            (Context::case_ok(BestChoice, getUnicharset()) ? 'y' : 'n'),
            ((rest (best_choices_) != NIL) ? 'n' : 'y'));
  }

  if (BestChoice.length() == 0 || CurrentWordAmbig())
    return (FALSE);
  if (BestChoice.fragment_mark()) {
    if (stopper_debug_level >= 1) {
      cprintf("AcceptableResult(): a choice with fragments beats BestChoice\n");
    }
    return false;
  }
  if (valid_word(BestChoice) &&
      Context::case_ok(BestChoice, getUnicharset())) {
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
    return (TRUE);
  }
  else {
    if (stopper_debug_level >= 1)
      cprintf("REJECTED\n");
    return (FALSE);
  }
}                                /* AcceptableResult */


/*---------------------------------------------------------------------------*/
int Dict::AlternativeChoicesWorseThan(FLOAT32 Threshold) {
/*
 **	Parameters:
 **		Threshold	minimum adjust factor for alternative choices
 **	Variables Used:
 **		best_choices_	alternative choices for current word
 **	Operation: This routine returns TRUE if there are no alternative
 **		choices for the current word OR if all alternatives have
 **		an adjust factor worse than Threshold.
 **	Return: TRUE or FALSE.
 **	Exceptions: none
 **	History: Mon Jun  3 09:36:31 1991, DSJ, Created.
 */
  LIST Alternatives;
  VIABLE_CHOICE Choice;

  Alternatives = rest (best_choices_);
  iterate(Alternatives) {
    Choice = (VIABLE_CHOICE) first_node (Alternatives);
    if (Choice->AdjustFactor <= Threshold)
      return (FALSE);
  }

  return (TRUE);

}                                /* AlternativeChoicesWorseThan */


/*---------------------------------------------------------------------------*/
int Dict::CurrentBestChoiceIs(const WERD_CHOICE &WordChoice) {
/*
 **	Parameters:
 **             Word            word that will be compared to the best choice
 **	Variables Used:
 **		best_choices_	set of best choices for current word
 **	Operation: Returns TRUE if Word is the same as the current best
 **		choice, FALSE otherwise.
 **	Return: TRUE or FALSE
 **	Exceptions: none
 **	History: Thu May 30 14:44:22 1991, DSJ, Created.
 */
  return (best_choices_ != NIL &&
          StringSameAs(WordChoice, (VIABLE_CHOICE)first_node(best_choices_)));
}                                /* CurrentBestChoiceIs */


/*---------------------------------------------------------------------------*/
FLOAT32 Dict::CurrentBestChoiceAdjustFactor() {
/*
 **	Parameters: none
 **	Variables Used:
 **		best_choices_	set of best choices for current word
 **	Operation: Return the adjustment factor for the best choice for
 **		the current word.
 **	Return: Adjust factor for current best choice.
 **	Exceptions: none
 **	History: Thu May 30 14:48:24 1991, DSJ, Created.
 */
  VIABLE_CHOICE BestChoice;

  if (best_choices_ == NIL)
    return (MAX_FLOAT32);

  BestChoice = (VIABLE_CHOICE) first_node (best_choices_);
  return (BestChoice->AdjustFactor);

}                                /* CurrentBestChoiceAdjustFactor */


/*---------------------------------------------------------------------------*/
int Dict::CurrentWordAmbig() {
/*
 **	Parameters: none
 **	Variables Used:
 **		best_choices_	set of best choices for current word
 **	Operation: This routine returns TRUE if there are multiple good
 **		choices for the current word and FALSE otherwise.
 **	Return: TRUE or FALSE
 **	Exceptions: none
 **	History: Wed May 22 15:38:38 1991, DSJ, Created.
 */
  return (rest (best_choices_) != NIL);

}                                /* CurrentWordAmbig */


/*---------------------------------------------------------------------------*/
void Dict::DebugWordChoices() {
/*
 **	Parameters: none
 **	Variables Used:
 **		best_raw_choice_
 **		best_choices_
 **	Operation: Print the current choices for this word to stdout.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 15 13:52:08 1991, DSJ, Created.
 */
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
}                                /* DebugWordChoices */

// Print all the choices in raw_choices_ list for non 1-1 ambiguities.
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

/*---------------------------------------------------------------------------*/
void Dict::FilterWordChoices() {
/*
 **	Parameters: none
 **	Variables Used:
 **		best_choices_	set of choices for current word
 **	Operation: This routine removes from best_choices_ all choices which
 **		are not within a reasonable range of the best choice.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 15 13:08:24 1991, DSJ, Created.
 */
  EXPANDED_CHOICE BestChoice;

  if (best_choices_ == NIL || second_node (best_choices_) == NIL)
    return;

  /* compute certainties and class for each chunk in best choice */
  ExpandChoice((VIABLE_CHOICE_STRUCT *)first_node(best_choices_), &BestChoice);

  set_rest (best_choices_, delete_d (rest (best_choices_),
    &BestChoice, FreeBadChoice));

}                                /* FilterWordChoices */

/*---------------------------------------------------------------------------*/
void Dict::FindClassifierErrors(FLOAT32 MinRating,
                                FLOAT32 MaxRating,
                                FLOAT32 RatingMargin,
                                FLOAT32 Thresholds[]) {
/*
 **	Parameters:
 **		MinRating		limits how tight to make a template
 **		MaxRating		limits how loose to make a template
 **		RatingMargin		amount of margin to put in template
 **		Thresholds[]		place to put error thresholds
 **	Operation: This routine compares the best choice for the current
 **		word to the best raw choice to determine which characters
 **		were classified incorrectly by the classifier.  It then
 **		places a separate threshold into Thresholds for each
 **		character in the word.  If the classifier was correct,
 **		MaxRating is placed into Thresholds.  If the
 **		classifier was incorrect, the avg. match rating (error
 **		percentage) of the classifier's incorrect choice minus
 **		some margin is
 **		placed into thresholds.  This can then be used by the
 **		caller to try to create a new template for the desired
 **		class that will classify the character with a rating better
 **		than the threshold value.  The match rating placed into
 **		Thresholds is never allowed to be below MinRating in order
 **		to prevent trying to make overly tight templates.
 **	Return: none (results are placed in Thresholds)
 **	Exceptions: none
 **	History: Fri May 31 16:02:57 1991, DSJ, Created.
 */
  EXPANDED_CHOICE BestRaw;
  VIABLE_CHOICE Choice;
  int i, j, Chunk;
  FLOAT32 AvgRating;
  int NumErrorChunks;

  assert (best_choices_ != NIL);
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
}                                /* FindClassifierErrors */


/*---------------------------------------------------------------------------*/
void Dict::InitChoiceAccum() {
/*
 **	Parameters: none
 **	Operation: This routine initializes the data structures used to
 **		keep track the good word choices found for a word.
 **	Return: none
 **	Exceptions: none
 **	History: Fri May 17 07:59:00 1991, DSJ, Created.
 */
  BLOB_WIDTH *BlobWidth, *End;

  if (best_raw_choice_)
    memfree(best_raw_choice_);
  best_raw_choice_ = NULL;

  if (best_choices_)
    destroy_nodes(best_choices_, memfree);
  best_choices_ = NIL;

  if (raw_choices_)
    destroy_nodes(raw_choices_, memfree);
  raw_choices_ = NIL;

  EnableChoiceAccum();

  for (BlobWidth = current_segmentation_,
    End = current_segmentation_ + MAX_NUM_CHUNKS;
    BlobWidth < End; *BlobWidth++ = 1);

}                                /* InitChoiceAccum */


/*---------------------------------------------------------------------------*/
void Dict::LogNewSegmentation(PIECES_STATE BlobWidth) {
/*
 **	Parameters:
 **		BlobWidth[]	number of chunks in each blob in segmentation
 **	Variables Used:
 **		current_segmentation	blob widths for current segmentation
 **	Operation: This routine updates the blob widths in current_segmentation
 **		to be the same as provided in BlobWidth.
 **	Return: none
 **	Exceptions: none
 **	History: Mon May 20 11:52:26 1991, DSJ, Created.
 */
  BLOB_WIDTH *Segmentation;

  for (Segmentation = current_segmentation_; *BlobWidth != 0;
    BlobWidth++, Segmentation++)
  *Segmentation = *BlobWidth;
  *Segmentation = 0;

}                                /* LogNewSegmentation */


/*---------------------------------------------------------------------------*/
void Dict::LogNewSplit(int Blob) {
/*
 **	Parameters:
 **		Blob	index of blob that was split
 **	Variables Used:
 **		best_raw_choice_	current best raw choice
 **		best_choices_	list of best choices found so far
 **	Operation: This routine adds 1 chunk to the specified blob for each
 **		choice in best_choices_ and for the best_raw_choice_.
 **	Return: none
 **	Exceptions: none
 **	History: Mon May 20 11:38:56 1991, DSJ, Created.
 */
  LIST Choices;

  if (best_raw_choice_) {
    AddNewChunk(best_raw_choice_, Blob);
  }

  Choices = best_choices_;
  iterate(Choices) {
    AddNewChunk ((VIABLE_CHOICE) first_node (Choices), Blob);
  }
  Choices = raw_choices_;
  iterate(Choices) {
    AddNewChunk ((VIABLE_CHOICE) first_node (Choices), Blob);
  }
}                                /* LogNewSplit */


/*---------------------------------------------------------------------------*/
void Dict::LogNewChoice(const WERD_CHOICE &WordChoice,
                        FLOAT32 AdjustFactor,
                        const float Certainties[],
                        bool raw_choice) {
/*
 **	Parameters:
 **		Choice		new choice for current word
 **		AdjustFactor	adjustment factor which was applied to choice
 **		Certainties	certainties for each char in new choice
 **		ChoicesList	list with choices seen so far
 **     Variables Used:
 **		best_raw_choice_	best raw choice so far for current word
 **	Operation: This routine adds Choice to ChoicesList if the
 **		adjusted certainty for Choice is within a reasonable range
 **		of the best choice in ChoicesList.  The ChoicesList
 **		list is kept in sorted order by rating. Duplicates are
 **		removed.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 15 09:57:19 1991, DSJ, Created.
 */
  VIABLE_CHOICE NewChoice;
  LIST ChoicesList;
  LIST Choices;
  FLOAT32 Threshold;

  if (!keep_word_choices_)
    return;

  if (raw_choice) {
    if (!best_raw_choice_)
      best_raw_choice_ = NewViableChoice(WordChoice, AdjustFactor, Certainties);
    else if (WordChoice.rating() < best_raw_choice_->Rating) {
      if (ChoiceSameAs(WordChoice, best_raw_choice_))
        FillViableChoice(WordChoice, AdjustFactor, Certainties, true,
                         best_raw_choice_);
      else {
        memfree(best_raw_choice_);
        best_raw_choice_ =
          NewViableChoice(WordChoice, AdjustFactor, Certainties);
      }
    }
    if (!save_raw_choices) return;
    ChoicesList = raw_choices_;
  } else {
    ChoicesList = best_choices_;
  }

  /* throw out obviously bad choices to save some work */
  if (ChoicesList != NIL) {
    Threshold = AmbigThreshold (BestFactor (ChoicesList), AdjustFactor);
    if (Threshold > -stopper_ambiguity_threshold_offset)
      Threshold = -stopper_ambiguity_threshold_offset;
    if (WordChoice.certainty() - BestCertainty (ChoicesList) < Threshold)
      return;
  }

  /* see if a choice with the same text string has already been found */
  NewChoice = NULL;
  Choices = ChoicesList;

  iterate(Choices) {
    if (ChoiceSameAs (WordChoice, (VIABLE_CHOICE) first_node (Choices))) {
      if (WordChoice.rating() < BestRating (Choices)) {
        NewChoice = (VIABLE_CHOICE) first_node (Choices);
      } else {
        return;
      }
    }
  }

  if (NewChoice) {
    FillViableChoice(WordChoice, AdjustFactor, Certainties, true, NewChoice);
    ChoicesList = delete_d(ChoicesList, NewChoice, is_same_node);
  }
  else {
    NewChoice = NewViableChoice (WordChoice, AdjustFactor, Certainties);
  }

  ChoicesList = s_adjoin (ChoicesList, NewChoice, CmpChoiceRatings);
  if (stopper_debug_level >= 2)
    raw_choice ? PrintViableChoice (stderr, "New Raw Choice:  ", NewChoice) :
      PrintViableChoice (stderr, "New Word Choice:  ", NewChoice);
  if (count (ChoicesList) > tessedit_truncate_wordchoice_log) {
    Choices =
      (LIST) nth_cell (ChoicesList, tessedit_truncate_wordchoice_log);
    destroy_nodes (rest (Choices), Efree);
    set_rest(Choices, NIL);
  }

  // Update raw_choices_/best_choices_ pointer.
  if (raw_choice) {
    raw_choices_ = ChoicesList;
  } else {
    best_choices_ = ChoicesList;
  }
}                                /* LogNewChoice */


/*---------------------------------------------------------------------------*/
int Dict::NoDangerousAmbig(WERD_CHOICE *best_choice,
                           DANGERR *fix_pt,
                           bool fix_replaceable,
                           BLOB_CHOICE_LIST_VECTOR *blob_choices,
                           bool *modified_blobs) {
  if (stopper_debug_level > 2) {
    tprintf("\nRunning NoDangerousAmbig() for %s\n",
            best_choice->debug_string(getUnicharset()).string());
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
  for (int pass = 0; pass < 2; ++pass) {
    bool replace = (pass == 0);
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
        lst_it.add_to_end(new BLOB_CHOICE(best_choice->unichar_id(i),
                                          0.0, 0.0, 0, -1));
        ambig_blob_choices.push_back(lst);
      }
    }
    UNICHAR_ID wrong_ngram[MAX_AMBIG_SIZE + 1];
    int wrong_ngram_index;
    int next_index;
    for (i = 0; i < best_choice->length(); ++i) {
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
                  ambig_spec->correct_fragments[tmp_index], -1.0, 0.0, 0, -1));
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
  if (modified_best_choice) best_choice->populate_unichars(getUnicharset());
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
    if (ambigs_found && stopper_debug_level >= 1) {
      tprintf ("Stopper: Possible ambiguous word = %s\n",
               alt_word->debug_string(getUnicharset()).string());
    }
    delete alt_word;
  }
  ambig_blob_choices.delete_data_pointers();
  return !ambigs_found;
}

void Dict::EndDangerousAmbigs() {}

/*---------------------------------------------------------------------------*/
void Dict::SettupStopperPass1() {
/*
 **	Parameters: none
 **	Variables Used:
 **		reject_offset_	offset allowed before word is rejected
 **	Operation: This routine performs any settup of stopper variables
 **		that is needed in preparation for the first pass.
 **	Return: none
 **	Exceptions: none
 **	History: Mon Jun  3 12:32:00 1991, DSJ, Created.
 */
  reject_offset_ = 0.0;
}                                /* SettupStopperPass1 */


/*---------------------------------------------------------------------------*/
void Dict::SettupStopperPass2() {
/*
 **	Parameters: none
 **	Variables Used:
 **		reject_offset_	offset allowed before word is rejected
 **	Operation: This routine performs any settup of stopper variables
 **		that is needed in preparation for the second pass.
 **	Return: none
 **	Exceptions: none
 **	History: Mon Jun  3 12:32:00 1991, DSJ, Created.
 */
  reject_offset_ = stopper_phase2_certainty_rejection_offset;
}                                /* SettupStopperPass2 */
}  // namespace tesseract


/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void AddNewChunk(VIABLE_CHOICE Choice, int Blob) {
/*
 **	Parameters:
 **		Choice	choice to add a new chunk to
 **		Blob	index of blob being split
 **	Operation: This routine increments the chunk count of the character
 **		in Choice which corresponds to Blob.
 **	Return: none
 **	Exceptions: none
 **	History: Mon May 20 11:43:27 1991, DSJ, Created.
 */
  int i, LastChunk;

  for (i = 0, LastChunk = 0; i < Choice->Length; i++) {
    LastChunk += Choice->Blob[i].NumChunks;
    if (Blob < LastChunk) {
      (Choice->Blob[i].NumChunks)++;
      return;
    }
  }
  mem_tidy (1);
  cprintf ("AddNewChunk failed:Choice->Length=%d, LastChunk=%d, Blob=%d\n",
           Choice->Length, LastChunk, Blob);
  assert(FALSE);  /* this should never get executed */

}                                /* AddNewChunk */


/*---------------------------------------------------------------------------*/
namespace tesseract {
// Replaces the corresponding wrong ngram in werd_choice with the correct one.
// We indicate that this newly inserted ngram unichar is composed from several
// fragments and modify the corresponding entries in blob_choices to contain
// fragments of the correct ngram unichar instead of the original unichars.
// Ratings and certainties of entries in blob_choices and werd_choice are
// unichaged. E.g. for werd_choice mystring'' and ambiguity ''->":
// werd_choice becomes mystring", first ' in blob_choices becomes |"|0|2,
// second one is set to |"|1|2.
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
            CHAR_FRAGMENT::to_string(temp_uch, i, fraglen);
          getUnicharset().unichar_insert(frag_str.string());
          uch_id = getUnicharset().unichar_to_id(frag_str.string());
        }
        bit.set_to_list(blob_choices->get(temp_blob_index));
        STRING correct_frag_uch =
          CHAR_FRAGMENT::to_string(correct_ngram_str,
                                   temp_blob_index - begin_blob_index,
                                   num_blobs_to_replace);
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
  if (stopper_debug_level >= 1) {
    tprintf("ReplaceAmbigs() modified werd_choice: %s\n",
            werd_choice->debug_string(getUnicharset()).string());
    werd_choice->print();
    if (modified_blobs != NULL && *modified_blobs && blob_choices != NULL) {
      tprintf("Modified blob_choices: ");
      for (int i = 0; i < blob_choices->size(); ++i) {
        print_ratings_list("\n", blob_choices->get(i), getUnicharset());
      }
    }
  }
}


/*---------------------------------------------------------------------------*/
int Dict::ChoiceSameAs(const WERD_CHOICE &WordChoice,
                       VIABLE_CHOICE ViableChoice) {
/*
 **	Parameters:
 **		Choice		choice to compare to ViableChoice
 **		ViableChoice	viable choice to compare to Choice
 **	Operation: This routine compares the corresponding strings of
 **		Choice and ViableChoice and returns TRUE if they are the
 **		same, FALSE otherwise.
 **	Return: TRUE or FALSE.
 **	Exceptions: none
 **	History: Fri May 17 08:48:04 1991, DSJ, Created.
 */
  return (StringSameAs(WordChoice, ViableChoice));

}                                /* ChoiceSameAs */
}  // namespace tesseract


/*---------------------------------------------------------------------------*/
int CmpChoiceRatings(void *arg1,    //VIABLE_CHOICE                 Choice1,
                     void *arg2) {  //VIABLE_CHOICE                 Choice2)
/*
 **	Parameters:
 **		Choice1, Choice2	choices to compare ratings for
 **	Operation: Return -1 if the rating for Choice1 is less than the
 **		rating for Choice2, otherwise return (1).
 **	Return: -1 or 1
 **	Exceptions: none
 **	History: Wed May 15 13:02:37 1991, DSJ, Created.
 */
  float R1, R2;
  VIABLE_CHOICE Choice1 = (VIABLE_CHOICE) arg1;
  VIABLE_CHOICE Choice2 = (VIABLE_CHOICE) arg2;

  R1 = Choice1->Rating;
  R2 = Choice2->Rating;

  if (R1 < R2)
    return (-1);
  else
    return (1);

}                                /* CmpChoiceRatings */


/*---------------------------------------------------------------------------*/
void ExpandChoice(VIABLE_CHOICE Choice, EXPANDED_CHOICE *ExpandedChoice) {
/*
 **	Parameters:
 **		Choice		choice to be expanded
 **		ExpandedChoice	place to put resulting expanded choice
 **	Operation: This routine expands Choice and places the results
 **		in ExpandedChoice.  The primary function of expansion
 **		is to create an two arrays, one which holds the corresponding
 **		certainty for each chunk in Choice, and one which holds
 **		the class for each chunk.
 **	Return: none (results are placed in ExpandedChoice)
 **	Exceptions: none
 **	History: Fri May 31 15:21:57 1991, DSJ, Created.
 */
  int i, j, Chunk;

  ExpandedChoice->Choice = Choice;
  for (i = 0, Chunk = 0; i < Choice->Length; i++)
  for (j = 0; j < Choice->Blob[i].NumChunks; j++, Chunk++) {
    ExpandedChoice->ChunkCertainty[Chunk] = Choice->Blob[i].Certainty;
    ExpandedChoice->ChunkClass[Chunk] = Choice->Blob[i].Class;
  }
}                                /* ExpandChoice */

/*---------------------------------------------------------------------------*/
int FreeBadChoice(void *item1,    //VIABLE_CHOICE                 Choice,
                  void *item2) {  //EXPANDED_CHOICE                       *BestChoice)
/*
 **	Parameters:
 **		Choice			choice to be tested
 **		BestChoice		best choice found
 **	Variables Used:
 **		stopper_ambiguity_threshold_gain
 **		stopper_ambiguity_threshold_offset
 **	Operation: If the certainty of any chunk in Choice is not ambiguous
 **		with the corresponding chunk in the best choice, free
 **		Choice and return TRUE.  Otherwise, return FALSE.
 **	Return: TRUE or FALSE.
 **	Exceptions: none
 **	History: Wed May 15 13:20:26 1991, DSJ, Created.
 */
  int i, j, Chunk;
  FLOAT32 Threshold;
  VIABLE_CHOICE Choice;
  EXPANDED_CHOICE *BestChoice;

  Choice = (VIABLE_CHOICE) item1;
  BestChoice = (EXPANDED_CHOICE *) item2;

  Threshold = AmbigThreshold (BestChoice->Choice->AdjustFactor,
    Choice->AdjustFactor);

  for (i = 0, Chunk = 0; i < Choice->Length; i++)
    for (j = 0; j < Choice->Blob[i].NumChunks; j++, Chunk++)
      if (Choice->Blob[i].Class != BestChoice->ChunkClass[Chunk] &&
    Choice->Blob[i].Certainty - BestChoice->ChunkCertainty[Chunk] <
      Threshold) {
        memfree(Choice);
    return (TRUE);
  }

  return (FALSE);

}                                /* FreeBadChoice */


/*---------------------------------------------------------------------------*/
namespace tesseract {
int Dict::LengthOfShortestAlphaRun(const WERD_CHOICE &WordChoice) {
/*
 **	Parameters:
 **		Word            word to be tested
 **	Operation: Return the length of the shortest alpha run in Word.
 **	Return:  Return the length of the shortest alpha run in Word.
 **	Exceptions: none
 **	History: Tue May 14 07:50:45 1991, DSJ, Created.
 */
  register int Shortest = MAX_INT32;
  register int Length;
  int x;
  int y;

  for (x = 0; x < WordChoice.length(); ++x) {
    if (getUnicharset().get_isalpha(WordChoice.unichar_id(x))) {
      for (y = x + 1, Length = 1;
           y < WordChoice.length() &&
           getUnicharset().get_isalpha(WordChoice.unichar_id(y));
           ++y, ++Length);
      if (Length < Shortest) {
        Shortest = Length;
      }
      if (y == WordChoice.length()) {
        break;
      }
    }
  }
  if (Shortest == MAX_INT32)
    Shortest = 0;

  return (Shortest);

}                                /* LengthOfShortestAlphaRun */


/*---------------------------------------------------------------------------*/
VIABLE_CHOICE Dict::NewViableChoice(const WERD_CHOICE &WordChoice,
                                    FLOAT32 AdjustFactor,
                                    const float Certainties[]) {
/*
 **	Parameters:
 **		Choice		choice to be converted to a viable choice
 **		AdjustFactor	factor used to adjust ratings for Choice
 **		Certainties	certainty for each character in Choice
 **	Variables Used:
 **		current_segmentation	segmentation corresponding to Choice
 **	Operation: Allocate a new viable choice data structure, copy
 **		Choice, Certainties, and current_segmentation_ into it,
 **		and return a pointer to it.
 **	Return: Ptr to new viable choice.
 **	Exceptions: none
 **	History: Thu May 16 15:28:29 1991, DSJ, Created.
 */
  int Length = WordChoice.length();
  assert (Length <= MAX_NUM_CHUNKS && Length > 0);
  VIABLE_CHOICE NewChoice = (VIABLE_CHOICE) Emalloc (
      sizeof (VIABLE_CHOICE_STRUCT) + (Length - 1) * sizeof (CHAR_CHOICE));
  FillViableChoice(WordChoice, AdjustFactor, Certainties, false, NewChoice);
  return (NewChoice);
}                                /* NewViableChoice */


/*---------------------------------------------------------------------------*/
void Dict::PrintViableChoice(FILE *File, const char *Label, VIABLE_CHOICE Choice) {
/*
 **	Parameters:
 **		File	open text file to print Choice to
 **		Label	text label to be printed with Choice
 **		Choice	choice to be printed
 **	Operation: This routine dumps a text representation of the
 **		specified Choice to File.
 **	Return: none
 **	Exceptions: none
 **	History: Mon May 20 11:16:44 1991, DSJ, Created.
 */
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
}                                /* PrintViableChoice */


/*---------------------------------------------------------------------------*/
void Dict::FillViableChoice(const WERD_CHOICE &WordChoice,
                            FLOAT32 AdjustFactor, const float Certainties[],
                            bool SameString, VIABLE_CHOICE ViableChoice) {
/*
 **	Parameters:
 **		WordChoice 	a choice with info that will be copied
 **		AdjustFactor	factor used to adjust ratings for AChoice
 **		Certainties	certainty for each character in AChoice
 **             SameString      if true the string in the viable choice
 **                             will not be changed
 **		ViableChoice	existing viable choice to fill in
 **	Variables Used:
 **		current_segmentation_	segmentation for NewChoice
 **	Operation:
 **             Fill ViableChoice with information from AChoice,
 **             AdjustFactor, and Certainties.
 **	Return: none
 **	Exceptions: none
 **	History: Fri May 17 13:35:58 1991, DSJ, Created.
 */
  CHAR_CHOICE *NewChar;
  BLOB_WIDTH *BlobWidth;
  int x;

  ViableChoice->Rating = WordChoice.rating();
  ViableChoice->Certainty = WordChoice.certainty();
  ViableChoice->AdjustFactor = AdjustFactor;
  ViableChoice->ComposedFromCharFragments = false;
  if (!SameString) {
    ViableChoice->Length = WordChoice.length();
  }
  for (x = 0,
       NewChar = &(ViableChoice->Blob[0]),
       BlobWidth = current_segmentation_;
       x < WordChoice.length();
       x++, NewChar++, Certainties++, BlobWidth++) {
    if (!SameString) {
      NewChar->Class = WordChoice.unichar_id(x);
    }
    NewChar->NumChunks = *BlobWidth;
    NewChar->Certainty = *Certainties;
    for (int i = 1; i < WordChoice.fragment_length(x); ++i) {
      BlobWidth++;
      assert(*BlobWidth > 0);
      NewChar->NumChunks += *BlobWidth;
      ViableChoice->ComposedFromCharFragments = true;
    }
  }
}                                /* FillViableChoice */


// Compares unichar ids in word_choice to those in viable_choice,
// returns true if they are the same, false otherwise.
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

/*---------------------------------------------------------------------------*/
int Dict::StringSameAs(const char *String,
                       const char *String_lengths,
                       VIABLE_CHOICE ViableChoice) {
/*
 **	Parameters:
 **		String		string to compare to ViableChoice
 **		String_lengths	lengths of unichars in String
 **		ViableChoice	viable choice to compare to String
 **	Operation: This routine compares String to ViableChoice and
 **		returns TRUE if they are the same, FALSE otherwise.
 **	Return: TRUE or FALSE.
 **	Exceptions: none
 **	History: Fri May 17 08:48:04 1991, DSJ, Created.
 */
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
    return (FALSE);
  }

  if (*String == 0)
    return (TRUE);
  else
    return (FALSE);

}                                /* StringSameAs */
}  // namespace tesseract

/*---------------------------------------------------------------------------*/
int UniformCertainties(const BLOB_CHOICE_LIST_VECTOR &Choices,
                       const WERD_CHOICE &BestChoice) {
/*
 **	Parameters:
 **		Choices		choices for current segmentation
 **		BestChoice	best choice for current segmentation
 **	Variables Used:
 **		stopper_allowable_character_badness
 **             max allowed certainty variation
 **	Operation: This routine returns TRUE if the certainty of the
 **		BestChoice word is within a reasonable range of the average
 **		certainties for the best choices for each character in
 **		the segmentation.  This test is used to catch words in which
 **		one character is much worse than the other characters in
 **		the word (i.e. FALSE will be returned in that case).
 **		The algorithm computes the mean and std deviation of the
 **		certainties in the word with the worst certainty thrown out.
 **	Return: TRUE or FALSE.
 **	Exceptions: none
 **	History: Tue May 14 08:23:21 1991, DSJ, Created.
 */
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
    return (TRUE);

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

  /* subtract off worst certainty from statistics */
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
    return (FALSE);
  } else {
    return (TRUE);
  }
}                                /* UniformCertainties */
