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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifdef __UNIX__
#include <assert.h>
#endif

/* these are kludges - add appropriate .h file later */
extern float CertaintyScale;     /* from subfeat.h */

#define MAX_WERD_SIZE   100
#define MAX_AMBIG_SIZE    3
#define DANGEROUS_AMBIGS  "DangAmbigs"

typedef LIST AMBIG_TABLE;

typedef struct
{
  UNICHAR_ID Class;
  uinT16 NumChunks;
  float Certainty;
}


CHAR_CHOICE;

typedef struct
{
  float Rating;
  float Certainty;
  FLOAT32 AdjustFactor;
  int Length;
  CHAR_CHOICE Blob[1];
} VIABLE_CHOICE_STRUCT;
typedef VIABLE_CHOICE_STRUCT *VIABLE_CHOICE;

typedef struct
{
  VIABLE_CHOICE Choice;
  float ChunkCertainty[MAX_NUM_CHUNKS];
  UNICHAR_ID ChunkClass[MAX_NUM_CHUNKS];
}


EXPANDED_CHOICE;

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

#define AmbigThreshold(F1,F2)	(((F2) - (F1)) * AmbigThresholdGain - \
				AmbigThresholdOffset)

/*---------------------------------------------------------------------------
          Private Function Prototoypes
----------------------------------------------------------------------------*/
void AddNewChunk(VIABLE_CHOICE Choice, int Blob);

int AmbigsFound(char *Word,
                char *CurrentChar,
                const char *Tail,
                const char *Tail_lengths,
                LIST Ambigs,
                DANGERR *fixpt);

int ChoiceSameAs(A_CHOICE *Choice, VIABLE_CHOICE ViableChoice);

int CmpChoiceRatings(void *arg1,   //VIABLE_CHOICE         Choice1,
                     void *arg2);  //VIABLE_CHOICE         Choice2);

void ExpandChoice(VIABLE_CHOICE Choice, EXPANDED_CHOICE *ExpandedChoice);

AMBIG_TABLE *FillAmbigTable();

int FreeBadChoice(void *item1,   //VIABLE_CHOICE                 Choice,
                  void *item2);  //EXPANDED_CHOICE                       *BestChoice);

int LengthOfShortestAlphaRun(register char *Word, const char *Word_lengths);

VIABLE_CHOICE NewViableChoice (A_CHOICE * Choice,
FLOAT32 AdjustFactor, float Certainties[]);

void PrintViableChoice(FILE *File, const char *Label, VIABLE_CHOICE Choice);

void ReplaceDuplicateChoice (VIABLE_CHOICE OldChoice,
A_CHOICE * NewChoice,
FLOAT32 AdjustFactor, float Certainties[]);

int StringSameAs(const char *String,
                 const char *String_lengths,
                 VIABLE_CHOICE ViableChoice);

int UniformCertainties(CHOICES_LIST Choices, A_CHOICE *BestChoice);

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

make_float_var (NonDictCertainty, -2.50, MakeNonDictCertainty,
17, 2, SetNonDictCertainty,
"Certainty threshold for non-dict words");

make_float_var (RejectCertaintyOffset, 1.0, MakeRejectCertaintyOffset,
17, 3, SetRejectCertaintyOffset, "Reject certainty offset");

make_int_var (SmallWordSize, 2, MakeSmallWordSize,
17, 4, SetSmallWordSize,
"Size of dict word to be treated as non-dict word");

make_float_var (CertaintyPerChar, -0.50, MakeCertaintyPerChar,
17, 5, SetCertaintyPerChar,
"Certainty to add for each dict char above SmallWordSize");

make_float_var (CertaintyVariation, 3.0, MakeCertaintyVariation,
17, 6, SetCertaintyVariation,
"Max certaintly variation allowed in a word (in sigma)");

make_int_var (StopperDebugLevel, 0, MakeStopperDebugLevel,
17, 7, SetStopperDebugLevel, "Stopper debug level");

make_float_var (AmbigThresholdGain, 8.0, MakeAmbigThresholdGain,
17, 8, SetAmbigThresholdGain,
"Gain factor for ambiguity threshold");

make_float_var (AmbigThresholdOffset, 1.5, MakeAmbigThresholdOffset,
17, 9, SetAmbigThresholdOffset,
"Certainty offset for ambiguity threshold");

extern int first_pass;
INT_VAR (tessedit_truncate_wordchoice_log, 10, "Max words to keep in list");

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
int AcceptableChoice(CHOICES_LIST Choices,
                     A_CHOICE *BestChoice,
                     A_CHOICE *RawChoice,
                     DANGERR *fixpt) {
/*
 **	Parameters:
 **		Choices		choices for current segmentation
 **		BestChoice	best choice for current segmentation
 **		RawChoice	best raw choice for current segmentation
 **	Globals:
 **		NonDictCertainty	certainty for a non-dict word
 **		SmallWordSize		size of word to be treated as non-word
 **		CertaintyPerChar	certainty to add for each dict char
 **	Operation: Return TRUE if the results from this segmentation are
 **		good enough to stop.  Otherwise return FALSE.
 **	Return: TRUE or FALSE.
 **	Exceptions: none
 **	History: Mon Apr 29 14:57:32 1991, DSJ, Created.
 */
  float CertaintyThreshold = NonDictCertainty;
  int WordSize;

  if (fixpt != NULL)
    fixpt->index = -1;
  if ((BestChoice == NULL) || (class_string (BestChoice) == NULL))
    return (FALSE);

  if (StopperDebugLevel >= 1)
    cprintf ("\nStopper:  %s (word=%c, case=%c, punct=%c)\n",
      class_string (BestChoice),
      (valid_word (class_string (BestChoice)) ? 'y' : 'n'),
    (case_ok (class_string (BestChoice),
              class_lengths (BestChoice)) ? 'y' : 'n'),
    ((punctuation_ok (class_string (BestChoice),
                      class_lengths (BestChoice)) !=
    -1) ? 'y' : 'n'));

  if (valid_word (class_string (BestChoice)) &&
    case_ok (class_string (BestChoice), class_lengths (BestChoice)) &&
  punctuation_ok (class_string (BestChoice),
                  class_lengths (BestChoice)) != -1) {
    WordSize = LengthOfShortestAlphaRun (class_string (BestChoice),
                                         class_lengths (BestChoice));
    WordSize -= SmallWordSize;
    if (WordSize < 0)
      WordSize = 0;
    CertaintyThreshold += WordSize * CertaintyPerChar;
  }
  else if (stopper_numbers_on && valid_number (class_string (BestChoice),
                                               class_lengths (BestChoice))) {
    CertaintyThreshold += stopper_numbers_on * CertaintyPerChar;
  }

  if (StopperDebugLevel >= 1)
    cprintf ("Stopper:  Certainty = %4.1f, Threshold = %4.1f\n",
      class_certainty (BestChoice), CertaintyThreshold);

  if (NoDangerousAmbig (class_string (BestChoice),
                        class_lengths (BestChoice), fixpt)
    && class_certainty (BestChoice) > CertaintyThreshold &&
    UniformCertainties (Choices, BestChoice))
    return (TRUE);
  else
    return (FALSE);

}                                /* AcceptableChoice */


/*---------------------------------------------------------------------------*/
int AcceptableResult(A_CHOICE *BestChoice, A_CHOICE *RawChoice) {
/*
 **	Parameters:
 **		BestChoice	best choice for current word
 **		RawChoice	best raw choice for current word
 **	Globals:
 **		NonDictCertainty	certainty for a non-dict word
 **		SmallWordSize		size of word to be treated as non-word
 **		CertaintyPerChar	certainty to add for each dict char
 **		BestChoices		list of all good choices found
 **		RejectOffset		allowed offset before a word is rejected
 **	Operation: Return FALSE if the best choice for the current word
 **		is questionable and should be tried again on the second
 **		pass or should be flagged to the user.
 **	Return: TRUE or FALSE.
 **	Exceptions: none
 **	History: Thu May  9 14:05:05 1991, DSJ, Created.
 */
  float CertaintyThreshold = NonDictCertainty - RejectOffset;
  int WordSize;

  if (StopperDebugLevel >= 1)
    cprintf ("\nRejecter: %s (word=%c, case=%c, punct=%c, unambig=%c)\n",
      class_string (BestChoice),
      (valid_word (class_string (BestChoice)) ? 'y' : 'n'),
    (case_ok (class_string (BestChoice),
              class_lengths (BestChoice)) ? 'y' : 'n'),
    ((punctuation_ok (class_string (BestChoice),
                      class_lengths (BestChoice)) != -1) ? 'y' : 'n'),
    ((rest (BestChoices) != NIL) ? 'n' : 'y'));

  if ((BestChoice == NULL) ||
    (class_string (BestChoice) == NULL) || CurrentWordAmbig ())
    return (FALSE);

  if (valid_word (class_string (BestChoice)) &&
    case_ok (class_string (BestChoice), class_lengths (BestChoice)) &&
  punctuation_ok (class_string (BestChoice),
                  class_lengths (BestChoice)) != -1) {
    WordSize = LengthOfShortestAlphaRun (class_string (BestChoice),
                                         class_lengths (BestChoice));
    WordSize -= SmallWordSize;
    if (WordSize < 0)
      WordSize = 0;
    CertaintyThreshold += WordSize * CertaintyPerChar;
  }

  if (StopperDebugLevel >= 1)
    cprintf ("Rejecter: Certainty = %4.1f, Threshold = %4.1f   ",
      class_certainty (BestChoice), CertaintyThreshold);

  if (class_certainty (BestChoice) > CertaintyThreshold) {
    if (StopperDebugLevel >= 1)
      cprintf ("ACCEPTED\n");
    return (TRUE);
  }
  else {
    if (StopperDebugLevel >= 1)
      cprintf ("REJECTED\n");
    return (FALSE);
  }
}                                /* AcceptableResult */


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


/*---------------------------------------------------------------------------*/
int CurrentBestChoiceIs(const char *Word, const char *Word_lengths) {
/*
 **	Parameters:
 **		Word            string to compare to current best choice
 **		Word_lengths	lengths of unichars in Word
 **	Globals:
 **		BestChoices	set of best choices for current word
 **	Operation: Returns TRUE if Word is the same as the current best
 **		choice, FALSE otherwise.
 **	Return: TRUE or FALSE
 **	Exceptions: none
 **	History: Thu May 30 14:44:22 1991, DSJ, Created.
 */
  return (BestChoices != NIL &&
    StringSameAs (Word, Word_lengths,
                  (VIABLE_CHOICE) first_node (BestChoices)));

}                                /* CurrentBestChoiceIs */


/*---------------------------------------------------------------------------*/
FLOAT32 CurrentBestChoiceAdjustFactor() {
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
int CurrentWordAmbig() {
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
void DebugWordChoices() {
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

  if (StopperDebugLevel >= 1 ||
    (WordToDebug && BestChoices &&
  StringSameAs (WordToDebug, WordToDebug_lengths,
                (VIABLE_CHOICE) first_node (BestChoices)))) {
    if (BestRawChoice)
      PrintViableChoice (stderr, "\nBest Raw Choice:   ", BestRawChoice);

    i = 1;
    Choices = BestChoices;
    if (Choices)
      cprintf ("\nBest Cooked Choices:\n");
    iterate(Choices) {
      sprintf (LabelString, "Cooked Choice #%d:  ", i);
      PrintViableChoice (stderr, LabelString,
        (VIABLE_CHOICE) first_node (Choices));
      i++;
    }
  }
}                                /* DebugWordChoices */


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
void
FindClassifierErrors (FLOAT32 MinRating,
FLOAT32 MaxRating,
FLOAT32 RatingMargin, FLOAT32 Thresholds[]) {
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

    for (j = 0; j < Choice->Blob[i].NumChunks; j++, Chunk++)
    if (Choice->Blob[i].Class != BestRaw.ChunkClass[Chunk]) {
      AvgRating += BestRaw.ChunkCertainty[Chunk];
      NumErrorChunks++;
    }

    if (NumErrorChunks > 0) {
      AvgRating /= NumErrorChunks;
      *Thresholds = (AvgRating / -CertaintyScale) * (1.0 - RatingMargin);
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

  MakeNonDictCertainty();
  MakeRejectCertaintyOffset();
  MakeSmallWordSize();
  MakeCertaintyPerChar();
  MakeCertaintyVariation();
  MakeStopperDebugLevel();
  MakeAmbigThresholdGain();
  MakeAmbigThresholdOffset();
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
void
LogNewRawChoice (A_CHOICE * Choice, FLOAT32 AdjustFactor, float Certainties[]) {
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
    BestRawChoice = NewViableChoice (Choice, AdjustFactor, Certainties);
  else if (class_probability (Choice) < BestRawChoice->Rating) {
    if (ChoiceSameAs (Choice, BestRawChoice))
      ReplaceDuplicateChoice(BestRawChoice, Choice, AdjustFactor, Certainties);
    else {
      memfree(BestRawChoice);
      BestRawChoice = NewViableChoice (Choice, AdjustFactor, Certainties);
    }
  }
}                                /* LogNewRawChoice */


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
void
LogNewWordChoice (A_CHOICE * Choice,
FLOAT32 AdjustFactor, float Certainties[]) {
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
    if (Threshold > -AmbigThresholdOffset)
      Threshold = -AmbigThresholdOffset;
    if (class_certainty (Choice) - BestCertainty (BestChoices) < Threshold)
      return;
  }

  /* see if a choice with the same text string has already been found */
  NewChoice = NULL;
  Choices = BestChoices;
  iterate(Choices) {
    if (ChoiceSameAs (Choice, (VIABLE_CHOICE) first_node (Choices))) {
      if (class_probability (Choice) < BestRating (Choices))
        NewChoice = (VIABLE_CHOICE) first_node (Choices);
      else
        return;
    }
  }

  if (NewChoice) {
    ReplaceDuplicateChoice(NewChoice, Choice, AdjustFactor, Certainties);
    BestChoices = delete_d (BestChoices, NewChoice, is_same_node);
  }
  else {
    NewChoice = NewViableChoice (Choice, AdjustFactor, Certainties);
  }

  BestChoices = s_adjoin (BestChoices, NewChoice, CmpChoiceRatings);
  if (StopperDebugLevel >= 2)
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

int NoDangerousAmbig(const char *Word,
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
  while (*Word)
  if (AmbigsFound (NewWord, NextNewChar,
                   Word + *Word_lengths, Word_lengths + 1,
                   AmbigFor[unicharset.unichar_to_id(Word, *Word_lengths)],
                   fixpt)) {
    if (fixpt != NULL)
      fixpt->index = bad_index;
    return (FALSE);
  }
  else {
    strncpy(NextNewChar, Word, *Word_lengths);
    NextNewChar += *Word_lengths;
    Word += *Word_lengths;
    Word_lengths++;
    bad_index++;
  }

  return (TRUE);

}                                /* NoDangerousAmbig */

void EndDangerousAmbigs() {
  if (AmbigFor != NULL) {
    for (int i = 0; i <= MAX_CLASS_ID; ++i) {
      destroy_nodes(AmbigFor[i], Efree);
    }
    Efree(AmbigFor);
    AmbigFor = NULL;
  }
}

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
  RejectOffset = RejectCertaintyOffset;
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
int AmbigsFound(char *Word,
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
        if (StopperDebugLevel >= 1)
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
int ChoiceSameAs(A_CHOICE *Choice, VIABLE_CHOICE ViableChoice) {
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
  return (StringSameAs (class_string (Choice), class_lengths (Choice),
                        ViableChoice));

}                                /* ChoiceSameAs */


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
AMBIG_TABLE *FillAmbigTable() {
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

  name = language_data_path_prefix;
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
      if (!unicharset.contains_unichar(buffer))
        illegal_char = true;
    }
    fscanf (AmbigFile, "%d", &AmbigPartSize);
    for (i = 0; i < AmbigPartSize; ++i) {
      fscanf (AmbigFile, "%s", buffer);
      strcat(ReplacementString, buffer);
      lengths[0] = strlen(buffer);
      strcat(ReplacementString_lengths, lengths);
      if (!unicharset.contains_unichar(buffer))
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
    unichar_id = unicharset.unichar_to_id(TestString, TestString_lengths[0]);
    NewTable[unichar_id] = push_last (NewTable[unichar_id], AmbigSpec);
  }

  fclose(AmbigFile);
  return (NewTable);

}                                /* FillAmbigTable */


/*---------------------------------------------------------------------------*/
int FreeBadChoice(void *item1,    //VIABLE_CHOICE                 Choice,
                  void *item2) {  //EXPANDED_CHOICE                       *BestChoice)
/*
 **	Parameters:
 **		Choice			choice to be tested
 **		BestChoice		best choice found
 **	Globals:
 **		AmbigThresholdGain
 **		AmbigThresholdOffset
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
int LengthOfShortestAlphaRun(register char *Word, const char *Word_lengths) {
/*
 **	Parameters:
 **		Word            word to be tested
 **		Word_lengths    lengths of the unichars in Word
 **	Globals: none
 **	Operation: Return the length of the shortest alpha run in Word.
 **	Return:  Return the length of the shortest alpha run in Word.
 **	Exceptions: none
 **	History: Tue May 14 07:50:45 1991, DSJ, Created.
 */
  register int Shortest = MAX_INT32;
  register int Length;

  for (; *Word; Word += *(Word_lengths++))
  if (unicharset.get_isalpha(Word, *Word_lengths)) {
    for (Length = 1, Word += *(Word_lengths++);
         *Word && unicharset.get_isalpha(Word, *Word_lengths);
         Word += *(Word_lengths++), Length++);
    if (Length < Shortest)
      Shortest = Length;

    if (*Word == 0)
      break;
  }
  if (Shortest == MAX_INT32)
    Shortest = 0;

  return (Shortest);

}                                /* LengthOfShortestAlphaRun */


/*---------------------------------------------------------------------------*/
VIABLE_CHOICE
NewViableChoice (A_CHOICE * Choice, FLOAT32 AdjustFactor, float Certainties[]) {
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
  VIABLE_CHOICE NewChoice;
  int Length;
  char *Word;
  char *Word_lengths;
  CHAR_CHOICE *NewChar;
  BLOB_WIDTH *BlobWidth;

  Length = strlen (class_lengths (Choice));
  assert (Length <= MAX_NUM_CHUNKS && Length > 0);

  NewChoice = (VIABLE_CHOICE) Emalloc (sizeof (VIABLE_CHOICE_STRUCT) +
    (Length - 1) * sizeof (CHAR_CHOICE));

  NewChoice->Rating = class_probability (Choice);
  NewChoice->Certainty = class_certainty (Choice);
  NewChoice->AdjustFactor = AdjustFactor;
  NewChoice->Length = Length;
  for (Word = class_string (Choice),
           Word_lengths = class_lengths (Choice),
           NewChar = &(NewChoice->Blob[0]),
           BlobWidth = CurrentSegmentation;
       *Word;
       Word += *(Word_lengths++), NewChar++, Certainties++, BlobWidth++) {
    NewChar->Class = unicharset.unichar_to_id(Word, *Word_lengths);
    NewChar->NumChunks = *BlobWidth;
    NewChar->Certainty = *Certainties;
  }

  return (NewChoice);

}                                /* NewViableChoice */


/*---------------------------------------------------------------------------*/
void PrintViableChoice(FILE *File, const char *Label, VIABLE_CHOICE Choice) {
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

  fprintf (File, "(R=%5.1f, C=%4.1f, F=%4.2f)  ",
    Choice->Rating, Choice->Certainty, Choice->AdjustFactor);

  for (i = 0; i < Choice->Length; i++)
    fprintf (File, "%s", unicharset.id_to_unichar(Choice->Blob[i].Class));
  fprintf (File, "\n");

  for (i = 0; i < Choice->Length; i++) {
    fprintf (File, "  %s", unicharset.id_to_unichar(Choice->Blob[i].Class));
    for (j = 0; j < Choice->Blob[i].NumChunks - 1; j++)
      fprintf (File, "   ");
  }
  fprintf (File, "\n");

  for (i = 0; i < Choice->Length; i++) {
    for (j = 0; j < Choice->Blob[i].NumChunks; j++)
      fprintf (File, "%3d", (int) (Choice->Blob[i].Certainty * -10.0));
  }
  fprintf (File, "\n");

}                                /* PrintViableChoice */


/*---------------------------------------------------------------------------*/
void
ReplaceDuplicateChoice (VIABLE_CHOICE OldChoice,
A_CHOICE * NewChoice,
FLOAT32 AdjustFactor, float Certainties[]) {
/*
 **	Parameters:
 **		OldChoice	existing viable choice to be replaced
 **		NewChoice	choice to replace OldChoice with
 **		AdjustFactor	factor used to adjust ratings for OldChoice
 **		Certainties	certainty for each character in OldChoice
 **	Globals:
 **		CurrentSegmentation	segmentation for NewChoice
 **	Operation: This routine is used whenever a better segmentation (or
 **		contextual interpretation) is found for a word which already
 **		exists.  The OldChoice is updated with the relevant
 **		information from the new choice.  The text string itself
 **		does not need to be copied since, by definition, has not
 **		changed.
 **	Return: none
 **	Exceptions: none
 **	History: Fri May 17 13:35:58 1991, DSJ, Created.
 */
  char *Word;
  char *Word_lengths;
  CHAR_CHOICE *NewChar;
  BLOB_WIDTH *BlobWidth;

  OldChoice->Rating = class_probability (NewChoice);
  OldChoice->Certainty = class_certainty (NewChoice);
  OldChoice->AdjustFactor = AdjustFactor;

  for (Word = class_string (NewChoice),
           Word_lengths = class_lengths (NewChoice),
           NewChar = &(OldChoice->Blob[0]),
           BlobWidth = CurrentSegmentation;
       *Word;
       Word += *(Word_lengths++), NewChar++, Certainties++, BlobWidth++) {
    NewChar->NumChunks = *BlobWidth;
    NewChar->Certainty = *Certainties;
  }
}                                /* ReplaceDuplicateChoice */


/*---------------------------------------------------------------------------*/
int StringSameAs(const char *String,
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
    current_unichar_length = strlen(unicharset.id_to_unichar(Char->Class));
  if (current_unichar_length != *String_lengths ||
      strncmp(String, unicharset.id_to_unichar(Char->Class),
              current_unichar_length) != 0)
    return (FALSE);
  }

  if (*String == 0)
    return (TRUE);
  else
    return (FALSE);

}                                /* StringSameAs */


/*---------------------------------------------------------------------------*/
int UniformCertainties(CHOICES_LIST Choices, A_CHOICE *BestChoice) {
/*
 **	Parameters:
 **		Choices		choices for current segmentation
 **		BestChoice	best choice for current segmentation
 **	Globals:
 **		CertaintyVariation	max allowed certainty variation
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
  int i;
  CHOICES CharChoices;
  float Certainty;
  float WorstCertainty = MAX_FLOAT32;
  float CertaintyThreshold;
  FLOAT64 TotalCertainty;
  FLOAT64 TotalCertaintySquared;
  FLOAT64 Variance;
  FLOAT32 Mean, StdDev;
  int WordLength;

  WordLength = array_count (Choices);
  if (WordLength < 3)
    return (TRUE);

  TotalCertainty = TotalCertaintySquared = 0.0;
  for_each_choice(Choices, i) {
    CharChoices = (CHOICES) array_index (Choices, i);
    Certainty = best_certainty (CharChoices);
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

  CertaintyThreshold = Mean - CertaintyVariation * StdDev;
  if (CertaintyThreshold > NonDictCertainty)
    CertaintyThreshold = NonDictCertainty;

  if (class_certainty (BestChoice) < CertaintyThreshold) {
    if (StopperDebugLevel >= 1)
      cprintf
        ("Stopper:  Non-uniform certainty = %4.1f (m=%4.1f, s=%4.1f, t=%4.1f)\n",
        class_certainty (BestChoice), Mean, StdDev, CertaintyThreshold);
    return (FALSE);
  }
  else
    return (TRUE);

}                                /* UniformCertainties */
