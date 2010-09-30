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
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

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

/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/
namespace tesseract {
/**
 * @name classify_blob
 *
 * Classify the this blob if it is not already recorded in the match
 * table. Attempt to recognize this blob as a character. The recognition
 * rating for this blob will be stored as a part of the blob. This value
 * will also be returned to the caller.
 *
 * @param pblob Previous blob
 * @param blob Current blob
 * @param nlob Next blob
 * @param row The row to process
 * @param string The string to display in ScrollView
 * @param color The colour to use when displayed with ScrollView
 */
BLOB_CHOICE_LIST *Wordrec::classify_blob(TBLOB *pblob,
                                         TBLOB *blob,
                                         TBLOB *nblob,
                                         TEXTROW *row, 
                                         const char *string,
                                         C_COL color) {
  BLOB_CHOICE_LIST *choices = NULL;
  chars_classified++;            /* Global value */
  if (tord_blob_skip)
    return (NULL);
#ifndef GRAPHICS_DISABLED
  if (wordrec_display_all_blobs)
    display_blob(blob, color);
#endif
  choices = get_match(blob);
  if (choices == NULL) {
    choices = call_matcher(pblob, blob, nblob, NULL, row);
    put_match(blob, choices);
  }
#ifndef GRAPHICS_DISABLED
  if (tord_display_ratings && string)
    print_ratings_list(string, choices, getDict().getUnicharset());

  if (wordrec_blob_pause)
    window_wait(blob_window);
#endif

  return (choices);
}

/**
 * @name update_blob_classifications
 *
 * For each blob in the given word update match_table with the
 * corresponding BLOB_CHOICES_LIST from choices.
 */
void Wordrec::update_blob_classifications(
    TWERD *word, const BLOB_CHOICE_LIST_VECTOR &choices) {
  TBLOB *tblob = word->blobs;
  int index = 0;
  for (; tblob != NULL && index < choices.length();
       tblob = tblob->next, index++) {
    add_to_match(tblob, choices.get(index));
  }
}

}  // namespace tesseract;


/**
 * @name write_text_files
 *
 * Write an answer to the output file that is the raw guess (without
 * context) directly from the classifier.
 */
void write_text_files(TWERD *word,
                      char *raw_choice,
                      int same_row,
                      int good_word,
                      int firstpass) {
  int x;
  /* Raw output */
  if (tord_write_raw_output) {
    if (same_row)
      fprintf (rawfile, "\n");
    if (raw_choice && strlen (raw_choice)) {
      fprintf (rawfile, "%s ", raw_choice);
      fflush(rawfile);
    }
  }
  /* Text file output */
  if (tord_write_output) {
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
