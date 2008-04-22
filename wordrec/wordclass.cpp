/* -*-C-*-
 ********************************************************************************
 *
 * File:        wordclass.c  (Formerly wordclass.c)
 * Description:  Word classifier
 * Author:       Mark Seaman, OCR Technology
 * Created:      Tue Jan 30 14:03:25 1990
 * Modified:     Fri Jul 12 16:03:06 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 *********************************************************************************/
/*----------------------------------------------------------------------
          I N C L U D E S
----------------------------------------------------------------------*/
#include <stdio.h>
#ifdef __UNIX__
#include <assert.h>
#endif

#include "wordclass.h"
#include "fxid.h"
#include "tordvars.h"
#include "associate.h"
#include "render.h"
#include "metrics.h"
#include "matchtab.h"
//#include "tfacepp.h"
#include "permute.h"
#include "context.h"
#include "badwords.h"
#include "callcpp.h"
#include <assert.h>

extern TBLOB *newblob();

/*----------------------------------------------------------------------
            Variables
----------------------------------------------------------------------*/
inT16 first_pass;

/*----------------------------------------------------------------------
          C o n s t a n t s
----------------------------------------------------------------------*/

#define BOLD_ON              "&dB(s3B"
#define BOLD_OFF             "&d@(s0B"
#define UNDERLINE_ON         "&dD"
#define UNDERLINE_OFF        "&d@"
LIST call_matcher(                  //call a matcher
                  TBLOB *ptblob,    //previous
                  TBLOB *tessblob,  //blob to match
                  TBLOB *ntblob,    //next
                  void *,           //unused parameter
                  TEXTROW *         //always null anyway
                 );

/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * classify_blob
 *
 * Classify the this blob if it is not already recorded in the match
 * table. Attempt to recognize this blob as a character.  The recognition
 * rating (probability) for this blob will be stored as a part of the
 * blob.  This value will also be returned to the caller.
 **********************************************************************/
CHOICES classify_blob(TBLOB *pblob,
                      TBLOB *blob,
                      TBLOB *nblob,
                      TEXTROW *row,
                      int fx,
                      const char *string,
                      C_COL color,
                      STATE *this_state,
                      STATE *best_state,
                      inT32 pass,
                      inT32 blob_index) {
  CHOICES rating;
  inT32 old_index;

  chars_classified++;            /* Global value */
  if (blob_skip)
    return (NIL);

#ifndef GRAPHICS_DISABLED
  if (display_all_blobs)
    display_blob(blob, color);
#endif
  rating = get_match (blob);
  if (rating == NIL) {
    if (pass) {
      old_index = blob_index;
                                 //?cast to int*
      blob_type = compare_states (best_state, this_state, (int *) &blob_index);
      // TODO(tkielbus) Remove this assert and reactivate code.
      // Manage the blob_answer, word_answer mechanism.
      // (convert or remove because it doesnt seem to be used anymore)
      assert(0);
#if 0
      blob_answer = word_answer[blob_index];
      if (blob_answer < '!')
        fprintf (matcher_fp,
          "Bad compare states: best state=0x%x%x, this=0x%x%x, bits="
          INT32FORMAT ", index=" INT32FORMAT ", outdex="
          INT32FORMAT ", word=%s\n", best_state->part1,
          best_state->part2, this_state->part1, this_state->part2,
          bits_in_states, old_index, blob_index, word_answer);
#endif
    }
    else
      blob_type = 0;
    rating = /*(*blob_matchers [fx]) */ (CHOICES) call_matcher (pblob, blob,
      nblob, NULL,
      row);
    put_match(blob, rating);
  }

#ifndef GRAPHICS_DISABLED
  if (display_ratings && string)
    print_choices(string, rating);

  if (blob_pause)
    window_wait(blob_window);
#endif

  return (rating);
}


/**********************************************************************
 * write_text_files
 *
 * Write an answer to the output file that is the raw guess (without
 * context) directly from the classifier.
 **********************************************************************/
void write_text_files(TWERD *word,
                      char *raw_choice,
                      int same_row,
                      int good_word,
                      int firstpass) {
  int x;
  /* Raw output */
  if (write_raw_output) {
    if (same_row)
      fprintf (rawfile, "\n");
    if (raw_choice && strlen (raw_choice)) {
      fprintf (rawfile, "%s ", raw_choice);
      fflush(rawfile);
    }
  }
  /* Text file output */
  if (write_output) {
    if (same_row)
      fprintf (textfile, "\n");
    if (word->guess && strlen (word->guess)) {
      for (x = 0; x < word->blanks; x++)
        fprintf (textfile, " ");
      if (!firstpass)
        fprintf(textfile, BOLD_ON);
      if (!good_word)
        fprintf(textfile, UNDERLINE_ON);
      fprintf (textfile, "%s", word->guess);
      if (!good_word)
        fprintf(textfile, UNDERLINE_OFF);
      if (!firstpass)
        fprintf(textfile, BOLD_OFF);
      fflush(textfile);
    }
  }
  /* Global counters */
  character_count += (word->guess ? strlen (word->guess) : 0);
  word_count++;
}


/**********************************************************************
 * save_answer
 *
 * Write an answer to the output file that is the raw guess (without
 * context) directly from the classifier.
 **********************************************************************/
void save_answer(TWERD *word,
                 TEXTROW *row,
                 A_CHOICE *best_choice,
                 A_CHOICE *raw_choice,
                 int firstpass) {
  static TEXTROW *last_row;
  char raw_answer[CHARS_PER_LINE];
  int answer_already;
  int good_answer;
  char *string = NULL;

  if (best_choice) {
    good_answer = AcceptableResult (best_choice, raw_choice);
    string = class_string (best_choice);
  }
  else {
    good_answer = FALSE;
  }

  if (firstpass) {
                                 /* First pass */
    if (string) {
                                 /* Got answer */
      add_document_word(best_choice);

      word->guess = string;
      fix_quotes (word->guess);
      strcpy (raw_answer, word->guess);

      record_certainty (class_certainty (best_choice), 1);

      if (good_answer) {
        record_certainty (class_certainty (best_choice), 2);
        strcat (raw_answer, " ");
        strcat (raw_answer, class_string (raw_choice));
        word->guess = strsave (raw_answer);
        word->guess[strlen (string)] = 0;
        if (string) {
          strfree(string);
          class_string (best_choice) = NULL;
        }
      }
      else {
                                 /* Not good enough */
        if (word->guess)
          strfree (word->guess);
        word->guess = NULL;
      }
    }
    else {
      word->guess = NULL;
      raw_answer[0] = '\0';
    }
  }
  else {
                                 /* Second pass */
    answer_already = (word->guess != NULL);
    if (answer_already) {
      write_text_files (word,
        &word->guess[strlen (word->guess) + 1],
        (row != last_row), TRUE, TRUE);
    }
    else {
                                 /* Required second pass */
      if (string) {
        if (!good_answer && tessedit_save_stats) {
          SaveBadWord (string, class_certainty (best_choice));
        }
        record_certainty (class_certainty (best_choice), 2);
        word->guess = class_string (best_choice);
        fix_quotes (word->guess);
        write_text_files (word, class_string (raw_choice),
          (row != last_row), good_answer, FALSE);
      }
    }
  }
  /* Word Display */
  if (display_text) {
    if (row != last_row)
      cprintf ("\n");
    if (word->guess && strlen (word->guess))
      cprintf ("%s ", word->guess);
    else
      cprintf ("%s ", raw_answer);
    fflush(stdout);
  }

  last_row = row;
}
