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
#include "debug.h"
#include "callcpp.h"
#include "permute.h"
#include "context.h"
#include "permnum.h"
#include "danerror.h"
#include "const.h"
#include "freelist.h"
#include "efio.h"
#include "globals.h"
#include "scanutils.h"
#include "unichar.h"
#include "varable.h"
#include "dict.h"
#include "image.h"
#include "ccutil.h"
#include "ratngs.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifdef __UNIX__
#include <assert.h>
#endif

/* these are kludges - add appropriate .h file later */
/* from adaptmatch.cpp */
double_VAR(certainty_scale, 20.0, "Certainty scaling factor");

#define MAX_WERD_SIZE   100
#define MAX_AMBIG_SIZE    3
#define DANGEROUS_AMBIGS  "DangAmbigs"

typedef LIST AMBIG_TABLE;

typedef struct
{
  VIABLE_CHOICE Choice;
  float ChunkCertainty[MAX_NUM_CHUNKS];
  UNICHAR_ID ChunkClass[MAX_NUM_CHUNKS];
} EXPANDED_CHOICE;

typedef struct
{
  char ambig[2 * (UNICHAR_LEN * MAX_AMBIG_SIZE) + 2];
  char lengths[2 * (MAX_AMBIG_SIZE) + 2];
} AMBIG_SPEC;

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

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
/* Name of file containing potentially dangerous ambiguities */
static const char *DangerousAmbigs = DANGEROUS_AMBIGS;

/* Word for which stopper debug information should be printed to stdout */
static char *WordToDebug = NULL;
static char *WordToDebug_lengths = NULL;

/* flag used to disable accumulation of word choices during compound word
  permutation */
BOOL8 KeepWordChoices = TRUE;

/* additional certainty padding allowed before a word is rejected */
static FLOAT32 RejectOffset = 0.0;

/* structures to keep track of viable word choices */
static VIABLE_CHOICE BestRawChoice = NULL;
static LIST BestChoices = NIL;
static PIECES_STATE CurrentSegmentation;

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

INT_VAR(stopper_debug_level, 0,
        "Stopper debug level");

double_VAR(stopper_ambiguity_threshold_gain, 8.0,
           "Gain factor for ambiguity threshold");

double_VAR(stopper_ambiguity_threshold_offset, 1.5,
           "Certainty offset for ambiguity threshold");

extern int first_pass;
INT_VAR (tessedit_truncate_wordchoice_log, 10, "Max words to keep in list");

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
namespace tesseract {
int Dict::AcceptableChoice(const BLOB_CHOICE_LIST_VECTOR &Choices,
                           const WERD_CHOICE &BestChoice,
                           const WERD_CHOICE &RawChoice,
                           DANGERR *fixpt,
                           ACCEPTABLE_CHOICE_CALLER caller) {
/*
 **	Parameters:
 **		Choices		choices for current segmentation
 **		BestChoice	best choice for current segmentation
 **		RawChoice	best raw choice for current segmentation
 **	Globals:
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

  if (fixpt != NULL)
    fixpt->index = -1;
  if (BestChoice.length() == 0)
    return (FALSE);
  if (caller == CHOPPER_CALLER && BestChoice.fragment_mark()) {
    if (stopper_debug_level >= 1) {
      cprintf("AcceptableChoice(): a choice with fragments beats BestChoice");
    }
    return false;
  }

  // TODO(daria): remove this conversion once dawg (valid_word)
  // is switched to use unichar ids, valid_number is deprecated
  // and DanAmbigs are fixed to work with unichar ids.
  STRING word_str = BestChoice.unichar_string();
  STRING word_lengths_str = BestChoice.unichar_lengths();

  if (stopper_debug_level >= 1)
    cprintf ("\nStopper:  %s (word=%c, case=%c, punct=%c)\n",
      word_str.string(),
      (valid_word(word_str.string()) ? 'y' : 'n'),
      (case_ok(word_str.string(), word_lengths_str.string()) ? 'y' : 'n'),
      ((punctuation_ok(word_str.string(),
                       word_lengths_str.string()) != -1) ? 'y' : 'n'));

  if (valid_word(word_str.string()) &&
      case_ok(word_str.string(), word_lengths_str.string()) &&
      punctuation_ok(word_str.string(), word_lengths_str.string()) != -1) {
    WordSize = LengthOfShortestAlphaRun(BestChoice);
    WordSize -= stopper_smallword_size;
    if (WordSize < 0)
      WordSize = 0;
    CertaintyThreshold += WordSize * stopper_certainty_per_char;
  } else if (stopper_numbers_on &&
             valid_number(word_str.string(), word_lengths_str.string())) {
    CertaintyThreshold += stopper_numbers_on * stopper_certainty_per_char;
  }

  if (stopper_debug_level >= 1)
    cprintf ("Stopper:  Certainty = %4.1f, Threshold = %4.1f\n",
             BestChoice.certainty(), CertaintyThreshold);

  if (NoDangerousAmbig(word_str.string(), word_lengths_str.string(), fixpt) &&
      BestChoice.certainty() > CertaintyThreshold &&
      UniformCertainties(Choices, BestChoice)) {
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
 **	Globals:
 **		stopper_nondict_certainty_base	certainty for a non-dict word
 **		stopper_smallword_size		size of word to be treated as non-word
 **		stopper_certainty_per_char	certainty to add for each dict char
 **		BestChoices		list of all good choices found
 **		RejectOffset		allowed offset before a word is rejected
 **	Operation: Return FALSE if the best choice for the current word
 **		is questionable and should be tried again on the second
 **		pass or should be flagged to the user.
 **	Return: TRUE or FALSE.
 **	Exceptions: none
 **	History: Thu May  9 14:05:05 1991, DSJ, Created.
 */
  float CertaintyThreshold = stopper_nondict_certainty_base - RejectOffset;
  int WordSize;

  // TODO(daria): remove this conversion once dawg (valid_word)
  // is switched to use unichar ids, valid_number is deprecated
  // and DanAmbigs are fixed to work with unichar ids.
  STRING word_str = BestChoice.unichar_string();
  STRING word_lengths_str = BestChoice.unichar_lengths();

  if (stopper_debug_level >= 1)
    cprintf ("\nRejecter: %s (word=%c, case=%c, punct=%c, unambig=%c)\n",
      word_str.string(),
      (valid_word(word_str.string()) ? 'y' : 'n'),
      (case_ok(word_str.string(), word_lengths_str.string()) ? 'y' : 'n'),
      ((punctuation_ok(word_str.string(),
                       word_lengths_str.string()) != -1) ? 'y' : 'n'),
      ((rest (BestChoices) != NIL) ? 'n' : 'y'));

  if (BestChoice.length() == 0 || CurrentWordAmbig())
    return (FALSE);
  if (BestChoice.fragment_mark()) {
    if (stopper_debug_level >= 1) {
      cprintf("AcceptableResult(): a choice with fragments beats BestChoice\n");
    }
    return false;
  }
  if (valid_word(word_str.string()) &&
      case_ok(word_str.string(), word_lengths_str.string()) &&
      (punctuation_ok(word_str.string(), word_lengths_str.string())) != -1) {
    WordSize = LengthOfShortestAlphaRun(BestChoice);
    WordSize -= stopper_smallword_size;
    if (WordSize < 0)
      WordSize = 0;
    CertaintyThreshold += WordSize * stopper_certainty_per_char;
  }

  if (stopper_debug_level >= 1)
    cprintf ("Rejecter: Certainty = %4.1f, Threshold = %4.1f   ",
      BestChoice.certainty(), CertaintyThreshold);

  if (BestChoice.certainty() > CertaintyThreshold) {
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
}  // namespace tesseract


/*---------------------------------------------------------------------------*/
int AlternativeChoicesWorseThan(FLOAT32 Threshold) {
/*
 **	Parameters:
 **		Threshold	minimum adjust factor for alternative choices
 **	Globals:
 **		BestChoices	alternative choices for current word
 **	Operation: This routine returns TRUE if there are no alternative
 **		choices for the current word OR if all alternatives have
 **		an adjust factor worse than Threshold.
 **	Return: TRUE or FALSE.
 **	Exceptions: none
 **	History: Mon Jun  3 09:36:31 1991, DSJ, Created.
 */
  LIST Alternatives;
  VIABLE_CHOICE Choice;

  Alternatives = rest (BestChoices);
  iterate(Alternatives) {
    Choice = (VIABLE_CHOICE) first_node (Alternatives);
    if (Choice->AdjustFactor <= Threshold)
      return (FALSE);
  }

  return (TRUE);

}                                /* AlternativeChoicesWorseThan */


namespace tesseract {
/*---------------------------------------------------------------------------*/
int Dict::CurrentBestChoiceIs(const WERD_CHOICE &WordChoice) {
/*
 **	Parameters:
 **             Word            word that will be compared to the best choice
 **	Globals:
 **		BestChoices	set of best choices for current word
 **	Operation: Returns TRUE if Word is the same as the current best
 **		choice, FALSE otherwise.
 **	Return: TRUE or FALSE
 **	Exceptions: none
 **	History: Thu May 30 14:44:22 1991, DSJ, Created.
 */
  return (BestChoices != NIL &&
          StringSameAs(WordChoice, (VIABLE_CHOICE)first_node(BestChoices)));
}                                /* CurrentBestChoiceIs */


/*---------------------------------------------------------------------------*/
FLOAT32 Dict::CurrentBestChoiceAdjustFactor() {
/*
 **	Parameters: none
 **	Globals:
 **		BestChoices	set of best choices for current word
 **	Operation: Return the adjustment factor for the best choice for
 **		the current word.
 **	Return: Adjust factor for current best choice.
 **	Exceptions: none
 **	History: Thu May 30 14:48:24 1991, DSJ, Created.
 */
  VIABLE_CHOICE BestChoice;

  if (BestChoices == NIL)
    return (MAX_FLOAT32);

  BestChoice = (VIABLE_CHOICE) first_node (BestChoices);
  return (BestChoice->AdjustFactor);

}                                /* CurrentBestChoiceAdjustFactor */


/*---------------------------------------------------------------------------*/
int Dict::CurrentWordAmbig() {
/*
 **	Parameters: none
 **	Globals:
 **		BestChoices	set of best choices for current word
 **	Operation: This routine returns TRUE if there are multiple good
 **		choices for the current word and FALSE otherwise.
 **	Return: TRUE or FALSE
 **	Exceptions: none
 **	History: Wed May 22 15:38:38 1991, DSJ, Created.
 */
  return (rest (BestChoices) != NIL);

}                                /* CurrentWordAmbig */


/*---------------------------------------------------------------------------*/
void Dict::DebugWordChoices() {
/*
 **	Parameters: none
 **	Globals:
 **		BestRawChoice
 **		BestChoices
 **	Operation: Print the current choices for this word to stdout.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 15 13:52:08 1991, DSJ, Created.
 */
  LIST Choices;
  int i;
  char LabelString[80];
  VIABLE_CHOICE VChoice = (VIABLE_CHOICE)first_node(BestChoices);
  bool force_debug =
    fragments_debug && VChoice != NULL && VChoice->ComposedFromCharFragments;

  if (stopper_debug_level >= 1 || force_debug ||
      (WordToDebug && BestChoices &&
       StringSameAs(WordToDebug, WordToDebug_lengths,
                    (VIABLE_CHOICE)first_node(BestChoices)))) {
    if (BestRawChoice)
      PrintViableChoice(stderr, "\nBest Raw Choice:   ", BestRawChoice);

    i = 1;
    Choices = BestChoices;
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
}  // namespace tesseract


/*---------------------------------------------------------------------------*/
void FilterWordChoices() {
/*
 **	Parameters: none
 **	Globals:
 **		BestChoices	set of choices for current word
 **	Operation: This routine removes from BestChoices all choices which
 **		are not within a reasonable range of the best choice.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 15 13:08:24 1991, DSJ, Created.
 */
  EXPANDED_CHOICE BestChoice;

  if (BestChoices == NIL || second_node (BestChoices) == NIL)
    return;

  /* compute certainties and class for each chunk in best choice */
  ExpandChoice ((VIABLE_CHOICE_STRUCT *) first_node (BestChoices), &BestChoice);

  set_rest (BestChoices, delete_d (rest (BestChoices),
    &BestChoice, FreeBadChoice));

}                                /* FilterWordChoices */


/*---------------------------------------------------------------------------*/
void FindClassifierErrors(FLOAT32 MinRating,
                          FLOAT32 MaxRating,
                          FLOAT32 RatingMargin,
                          FLOAT32 Thresholds[]) {
/*
 **	Parameters:
 **		MinRating		limits how tight to make a template
 **		MaxRating		limits how loose to make a template
 **		RatingMargin		amount of margin to put in template
 **		Thresholds[]		place to put error thresholds
 **	Globals: none
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

  assert (BestChoices != NIL);
  assert (BestRawChoice != NULL);

  ExpandChoice(BestRawChoice, &BestRaw);
  Choice = (VIABLE_CHOICE) first_node (BestChoices);

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
void InitStopperVars() {
/*
 **	Parameters: none
 **	Globals: none
 **	Operation: Initializes the control variables used by the stopper.
 **	Return: none
 **	Exceptions: none
 **	History: Thu May  9 10:06:04 1991, DSJ, Created.
 */
  VALUE dummy;

  string_variable (DangerousAmbigs, "DangerousAmbigs", DANGEROUS_AMBIGS);
  string_variable (WordToDebug, "WordToDebug", "");
  string_variable (WordToDebug_lengths, "WordToDebug_lengths", "");

}                                /* InitStopperVars */


/*---------------------------------------------------------------------------*/
void InitChoiceAccum() {
/*
 **	Parameters: none
 **	Globals: none
 **	Operation: This routine initializes the data structures used to
 **		keep track the good word choices found for a word.
 **	Return: none
 **	Exceptions: none
 **	History: Fri May 17 07:59:00 1991, DSJ, Created.
 */
  BLOB_WIDTH *BlobWidth, *End;

  if (BestRawChoice)
    memfree(BestRawChoice);

  if (BestChoices)
    destroy_nodes(BestChoices, memfree);

  BestRawChoice = NULL;
  BestChoices = NIL;
  EnableChoiceAccum();

  for (BlobWidth = CurrentSegmentation,
    End = CurrentSegmentation + MAX_NUM_CHUNKS;
    BlobWidth < End; *BlobWidth++ = 1);

}                                /* InitChoiceAccum */


/*---------------------------------------------------------------------------*/
namespace tesseract {
void Dict::LogNewRawChoice(const WERD_CHOICE &WordChoice,
                           FLOAT32 AdjustFactor,
                           const float Certainties[]) {
/*
 **	Parameters:
 **		Choice		new raw choice for current word
 **		AdjustFactor	adjustment factor which was applied to choice
 **		Certainties	certainties for each char in new choice
 **	Globals:
 **		BestRawChoice	best raw choice so far for current word
 **	Operation: This routine compares Choice to the best raw (non-dict)
 **		choice so far and replaces it if the new choice is better.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 15 09:57:19 1991, DSJ, Created.
 */
  if (!KeepWordChoices)
    return;

  if (!BestRawChoice)
    BestRawChoice = NewViableChoice (WordChoice, AdjustFactor, Certainties);
  else if (WordChoice.rating() < BestRawChoice->Rating) {
    if (ChoiceSameAs(WordChoice, BestRawChoice))
      FillViableChoice(WordChoice, AdjustFactor, Certainties, true,
                       BestRawChoice);
    else {
      memfree(BestRawChoice);
      BestRawChoice = NewViableChoice(WordChoice, AdjustFactor, Certainties);
    }
  }
}                                /* LogNewRawChoice */

}  // namespace tesseract

/*---------------------------------------------------------------------------*/
void LogNewSegmentation(PIECES_STATE BlobWidth) {
/*
 **	Parameters:
 **		BlobWidth[]	number of chunks in each blob in segmentation
 **	Globals:
 **		CurrentSegmentation	blob widths for current segmentation
 **	Operation: This routine updates the blob widths in CurrentSegmentation
 **		to be the same as provided in BlobWidth.
 **	Return: none
 **	Exceptions: none
 **	History: Mon May 20 11:52:26 1991, DSJ, Created.
 */
  BLOB_WIDTH *Segmentation;

  for (Segmentation = CurrentSegmentation; *BlobWidth != 0;
    BlobWidth++, Segmentation++)
  *Segmentation = *BlobWidth;
  *Segmentation = 0;

}                                /* LogNewSegmentation */


/*---------------------------------------------------------------------------*/
void LogNewSplit(int Blob) {
/*
 **	Parameters:
 **		Blob	index of blob that was split
 **	Globals:
 **		BestRawChoice	current best raw choice
 **		BestChoices	list of best choices found so far
 **	Operation: This routine adds 1 chunk to the specified blob for each
 **		choice in BestChoices and for the BestRawChoice.
 **	Return: none
 **	Exceptions: none
 **	History: Mon May 20 11:38:56 1991, DSJ, Created.
 */
  LIST Choices;

  if (BestRawChoice) {
    AddNewChunk(BestRawChoice, Blob);
  }

  Choices = BestChoices;
  iterate(Choices) {
    AddNewChunk ((VIABLE_CHOICE) first_node (Choices), Blob);
  }

}                                /* LogNewSplit */


/*---------------------------------------------------------------------------*/
namespace tesseract {
void Dict::LogNewWordChoice(const WERD_CHOICE &WordChoice,
                            FLOAT32 AdjustFactor,
                            const float Certainties[]) {
/*
 **	Parameters:
 **		Choice		new choice for current word
 **		AdjustFactor	adjustment factor which was applied to choice
 **		Certainties	certainties for each char in new choice
 **	Globals:
 **		BestChoices	best choices so far for current word
 **	Operation: This routine adds Choice to BestChoices if the
 **		adjusted certainty for Choice is within a reasonable range
 **		of the best choice in BestChoices.  The BestChoices
 **		list is kept in sorted order by rating. Duplicates are
 **		removed.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 15 09:57:19 1991, DSJ, Created.
 */
  VIABLE_CHOICE NewChoice;
  LIST Choices;
  FLOAT32 Threshold;

  if (!KeepWordChoices)
    return;

  /* throw out obviously bad choices to save some work */
  if (BestChoices != NIL) {
    Threshold = AmbigThreshold (BestFactor (BestChoices), AdjustFactor);
    if (Threshold > -stopper_ambiguity_threshold_offset)
      Threshold = -stopper_ambiguity_threshold_offset;
    if (WordChoice.certainty() - BestCertainty (BestChoices) < Threshold)
      return;
  }

  /* see if a choice with the same text string has already been found */
  NewChoice = NULL;
  Choices = BestChoices;

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
    BestChoices = delete_d(BestChoices, NewChoice, is_same_node);
  }
  else {
    NewChoice = NewViableChoice (WordChoice, AdjustFactor, Certainties);
  }

  BestChoices = s_adjoin (BestChoices, NewChoice, CmpChoiceRatings);
  if (stopper_debug_level >= 2)
    PrintViableChoice (stderr, "New Word Choice:  ", NewChoice);
  if (count (BestChoices) > tessedit_truncate_wordchoice_log) {
    Choices =
      (LIST) nth_cell (BestChoices, tessedit_truncate_wordchoice_log);
    destroy_nodes (rest (Choices), Efree);
    set_rest(Choices, NIL);
  }

}                                /* LogNewWordChoice */


/*---------------------------------------------------------------------------*/
static AMBIG_TABLE *AmbigFor = NULL;

int Dict::NoDangerousAmbig(const char *Word,
                           const char *Word_lengths,
                           DANGERR *fixpt) {
/*
 **	Parameters:
 **		Word	word to check for dangerous ambiguities
 **		Word_lengths	lengths of unichars in Word
 **	Globals: none
 **	Operation: This word checks each letter in word against a list
 **		of potentially ambiguous characters.  If a match is found
 **		that letter is replaced with its ambiguity and tested in
 **		the dictionary.  If the ambiguous word is found in the
 **		dictionary, FALSE is returned.  Otherwise, the search
 **		continues for other ambiguities.  If no ambiguities that
 **		match in the dictionary are found, TRUE is returned.
 **	Return: TRUE if Word contains no dangerous ambiguities.
 **	Exceptions: none
 **	History: Mon May  6 16:28:56 1991, DSJ, Created.
 */

  char NewWord[MAX_WERD_SIZE * UNICHAR_LEN + 1];
  char *NextNewChar;
  int bad_index = 0;

  if (!AmbigFor)
    AmbigFor = FillAmbigTable ();

  NextNewChar = NewWord;
  while (*Word) {
    if (AmbigsFound (NewWord, NextNewChar,
                     Word + *Word_lengths, Word_lengths + 1,
                     AmbigFor[getUnicharset().unichar_to_id(
                         Word, *Word_lengths)],
                     fixpt)) {
      if (fixpt != NULL)
        fixpt->index = bad_index;
      return (FALSE);
    } else {
      strncpy(NextNewChar, Word, *Word_lengths);
      NextNewChar += *Word_lengths;
      Word += *Word_lengths;
      Word_lengths++;
      bad_index++;
    }
  }
  return (TRUE);
}                                /* NoDangerousAmbig */

void Dict::EndDangerousAmbigs() {
  if (AmbigFor != NULL) {
    for (int i = 0; i <= MAX_CLASS_ID; ++i) {
      destroy_nodes(AmbigFor[i], Efree);
    }
    Efree(AmbigFor);
    AmbigFor = NULL;
  }
}
}  // namespace tesseract

/*---------------------------------------------------------------------------*/
void SettupStopperPass1() {
/*
 **	Parameters: none
 **	Globals:
 **		RejectOffset	offset allowed before word is rejected
 **	Operation: This routine performs any settup of stopper variables
 **		that is needed in preparation for the first pass.
 **	Return: none
 **	Exceptions: none
 **	History: Mon Jun  3 12:32:00 1991, DSJ, Created.
 */
  RejectOffset = 0.0;
}                                /* SettupStopperPass1 */


/*---------------------------------------------------------------------------*/
void SettupStopperPass2() {
/*
 **	Parameters: none
 **	Globals:
 **		RejectOffset	offset allowed before word is rejected
 **	Operation: This routine performs any settup of stopper variables
 **		that is needed in preparation for the second pass.
 **	Return: none
 **	Exceptions: none
 **	History: Mon Jun  3 12:32:00 1991, DSJ, Created.
 */
  RejectOffset = stopper_phase2_certainty_rejection_offset;
}                                /* SettupStopperPass2 */


/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void AddNewChunk(VIABLE_CHOICE Choice, int Blob) {
/*
 **	Parameters:
 **		Choice	choice to add a new chunk to
 **		Blob	index of blob being split
 **	Globals: none
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
int Dict::AmbigsFound(char *Word,
                      char *CurrentChar,
                      const char *Tail,
                      const char *Tail_lengths,
                      LIST Ambigs,
                      DANGERR *fixpt) {
/*
 **	Parameters:
 **		Word		word being tested for ambiguities
 **		CurrentChar	position in Word to put ambig replacement
 **		Tail		end of word to place after ambiguity
 **		Tail_lengths    lengths of the unichars in Tail
 **		Ambigs		list of ambiguities to test at this position
 **	Globals: none
 **	Operation: For each ambiguity in Ambigs, see if the remainder of
 **		the test string matches the start of Tail.  If it does,
 **		construct a word consisting of the contents of Word up to,
 **		but not including, CurrentChar followed by the replacement
 **		string for the ambiguity followed by the unmatched
 **		contents of Tail.  Then test this word to see if it
 **		is a dictionary word.  If it is return TRUE.  If none of
 **		the ambiguities result in a dictionary word, return FALSE.
 **	Return: TRUE if the Word is ambiguous at the specified position
 **	Exceptions: none
 **	History: Thu May  9 10:10:28 1991, DSJ, Created.
 */
  AMBIG_SPEC *AmbigSpec;
  char *ambig;
  char *ambig_lengths;
  const char *UnmatchedTail;
  const char *UnmatchedTail_lengths;
  int Matches;
  int bad_length;

  iterate(Ambigs) {
    AmbigSpec = (AMBIG_SPEC *) first_node (Ambigs);
    ambig = AmbigSpec->ambig;
    ambig_lengths = AmbigSpec->lengths;
    bad_length = 1;
    UnmatchedTail = Tail;
    UnmatchedTail_lengths = Tail_lengths;
    Matches = TRUE;

    while (*ambig != ' ' && Matches)
      if (*UnmatchedTail_lengths == *ambig_lengths &&
          strncmp(ambig, UnmatchedTail, *ambig_lengths) == 0) {
        ambig += *(ambig_lengths++);
        UnmatchedTail += *(UnmatchedTail_lengths++);
        bad_length++;
      }
      else
        Matches = FALSE;

    if (Matches) {
      ambig += *(ambig_lengths++); /* skip over the space */
                                   /* insert replacement string */
      strcpy(CurrentChar, ambig);
                                   /* add tail */
      strcat(Word, UnmatchedTail);
      if (valid_word (Word)) {
        if (stopper_debug_level >= 1)
          cprintf ("Stopper:  Possible ambiguous word = %s\n", Word);
        if (fixpt != NULL) {
          fixpt->good_length = strlen (ambig_lengths);
          fixpt->bad_length = bad_length;
        }
        return (TRUE);
      }
    }
  }
  return (FALSE);

}                                /* AmbigsFound */


/*---------------------------------------------------------------------------*/
int Dict::ChoiceSameAs(const WERD_CHOICE &WordChoice,
                       VIABLE_CHOICE ViableChoice) {
/*
 **	Parameters:
 **		Choice		choice to compare to ViableChoice
 **		ViableChoice	viable choice to compare to Choice
 **	Globals: none
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
 **	Globals: none
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
 **	Globals: none
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
namespace tesseract {
AMBIG_TABLE *Dict::FillAmbigTable() {
/*
 **	Parameters: none
 **	Globals:
 **		DangerousAmbigs		filename of dangerous ambig info
 **	Operation: This routine allocates a new ambiguity table and fills
 **		it in from the file specified by DangerousAmbigs.  An
 **		ambiguity table is an array of lists.  The array is indexed
 **		by a class id.  Therefore, each entry in the table provides
 **		a list of potential ambiguities which can start with the
 **		corresponding character.  Each potential ambiguity is
 **		described by a string which contains the remainder of the
 **		test string followed by a space followed by the replacement
 **		string.  For example the ambiguity "rn -> m", would be
 **		located in the table at index 'r'.  The string corresponding
 **		to this ambiguity would be "n m".
 **	Return: Pointer to new ambiguity table.
 **	Exceptions: none
 **	History: Thu May  9 09:20:57 1991, DSJ, Created.
 */
  FILE *AmbigFile;
  AMBIG_TABLE *NewTable;
  int i;
  int AmbigPartSize;
  char buffer[256 * UNICHAR_LEN];
  char TestString[256 * UNICHAR_LEN];
  char TestString_lengths[256];
  char ReplacementString[256 * UNICHAR_LEN];
  char ReplacementString_lengths[256];
  STRING name;
  char lengths[2];
  AMBIG_SPEC *AmbigSpec;
  UNICHAR_ID unichar_id;

  lengths[1] = 0;

  name = getImage()->getCCUtil()->language_data_path_prefix;
  name += DangerousAmbigs;
  AmbigFile = Efopen (name.string(), "r");
  NewTable = (AMBIG_TABLE *) Emalloc (sizeof (LIST) * (MAX_CLASS_ID + 1));

  for (i = 0; i <= MAX_CLASS_ID; i++)
    NewTable[i] = NIL;

  while (fscanf (AmbigFile, "%d", &AmbigPartSize) == 1) {
    TestString[0] = '\0';
    TestString_lengths[0] = 0;
    ReplacementString[0] = '\0';
    ReplacementString_lengths[0] = 0;
    bool illegal_char = false;
    for (i = 0; i < AmbigPartSize; ++i) {
      fscanf (AmbigFile, "%s", buffer);
      strcat(TestString, buffer);
      lengths[0] = strlen(buffer);
      strcat(TestString_lengths, lengths);
      if (!getUnicharset().contains_unichar(buffer))
        illegal_char = true;
    }
    fscanf (AmbigFile, "%d", &AmbigPartSize);
    for (i = 0; i < AmbigPartSize; ++i) {
      fscanf (AmbigFile, "%s", buffer);
      strcat(ReplacementString, buffer);
      lengths[0] = strlen(buffer);
      strcat(ReplacementString_lengths, lengths);
      if (!getUnicharset().contains_unichar(buffer))
        illegal_char = true;
    }

    if (strlen (TestString_lengths) > MAX_AMBIG_SIZE ||
        strlen (ReplacementString_lengths) > MAX_AMBIG_SIZE)
      DoError (0, "Illegal ambiguity specification!");
    if (illegal_char) {
      continue;
    }

    AmbigSpec = (AMBIG_SPEC *) Emalloc (sizeof (AMBIG_SPEC));

    strcpy(AmbigSpec->ambig, TestString + TestString_lengths[0]);
    strcat(AmbigSpec->ambig, " ");
    strcat(AmbigSpec->ambig, ReplacementString);

    strcpy(AmbigSpec->lengths, TestString_lengths + 1);
    lengths[0] = 1;
    strcat(AmbigSpec->lengths, lengths);
    strcat(AmbigSpec->lengths, ReplacementString_lengths);
    unichar_id = getUnicharset().unichar_to_id(TestString,
                                               TestString_lengths[0]);
    NewTable[unichar_id] = push_last (NewTable[unichar_id], AmbigSpec);
  }

  fclose(AmbigFile);
  return (NewTable);

}                                /* FillAmbigTable */
}  // namespace tesseract

/*---------------------------------------------------------------------------*/
int FreeBadChoice(void *item1,    //VIABLE_CHOICE                 Choice,
                  void *item2) {  //EXPANDED_CHOICE                       *BestChoice)
/*
 **	Parameters:
 **		Choice			choice to be tested
 **		BestChoice		best choice found
 **	Globals:
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
 **	Globals: none
 **	Operation: Return the length of the shortest alpha run in Word.
 **	Return:  Return the length of the shortest alpha run in Word.
 **	Exceptions: none
 **	History: Tue May 14 07:50:45 1991, DSJ, Created.
 */
  register int Shortest = MAXINT;
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
  if (Shortest == MAXINT)
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
 **	Globals:
 **		CurrentSegmentation	segmentation corresponding to Choice
 **	Operation: Allocate a new viable choice data structure, copy
 **		Choice, Certainties, and CurrentSegmentation into it,
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
 **	Globals: none
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
 **	Globals:
 **		CurrentSegmentation	segmentation for NewChoice
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
       BlobWidth = CurrentSegmentation;
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
 **	Globals: none
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
 **	Globals:
 **		stopper_allowable_character_badness	max allowed certainty variation
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
