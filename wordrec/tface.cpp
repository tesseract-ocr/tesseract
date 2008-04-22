/**********************************************************************
 * File:        tface.c  (Formerly tface.c)
 * Description: C side of the Tess/tessedit C/C++ interface.
 * Author:		Ray Smith
 * Created:		Mon Apr 27 11:57:06 BST 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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
 **********************************************************************/
#include "tface.h"
#include "danerror.h"
#include "globals.h"
#include "tordvars.h"            /* Feature stuff */
#include "fxid.h"
#include "wordclass.h"
#include "bestfirst.h"
#include "context.h"
#include "gradechop.h"
#include "hyphen.h"
/* includes for init */
#include "msmenus.h"
#include "djmenus.h"
#include "tessinit.h"
#include "mfvars.h"
#include "variables.h"
#include "metrics.h"
#include "adaptmatch.h"
#include "matchtab.h"
#include "chopper.h"
#include "permdawg.h"
#include "permute.h"
#include "chop.h"
#include "callcpp.h"
#include "badwords.h"

#include <math.h>
#ifdef __UNIX__
#include <unistd.h>
#endif
//extern "C" int record_matcher_output;

/*----------------------------------------------------------------------
              Variables
----------------------------------------------------------------------*/
static PRIORITY pass2_ok_split;
static int pass2_seg_states;
extern int NO_BLOCK;
/*----------------------------------------------------------------------
              Function Code
----------------------------------------------------------------------*/
/**********************************************************************
 * start_recog
 *
 * Startup recog program ready to recognize words.
 **********************************************************************/
int start_recog(const char *configfile, const char *textbase) {

  program_editup(configfile);
  program_editup2(textbase);
  return (0);
}


/**********************************************************************
 * program_editup
 *
 * Initialize all the things in the program that need to be initialized.
 **********************************************************************/
void program_editup(const char *configfile) {
  init_ms_debug();
  init_dj_debug();

  program_variables();
  mfeature_variables();

  if (configfile != NULL) {
    //              cprintf ("Reading configuration from file '%s'\n", configfile);
    /* Read config file */
    read_variables(configfile);
  }
  /* Initialize subsystems */
  program_init();
  mfeature_init();
  init_permute();
  setup_cp_maps();
}


/*-------------------------------------------------------------------------*/
void program_editup2(const char *textbase) {
  if (textbase != NULL) {
    strcpy(imagefile, textbase);
    /* Read in data files */
    edit_with_ocr(textbase);
  }

  init_metrics();
  pass2_ok_split = ok_split;
  pass2_seg_states = num_seg_states;
}


/**********************************************************************
 * edit_with_ocr
 *
 * Initialize all the things in the program needed before the classifier
 * code is called.
 **********************************************************************/
void edit_with_ocr(const char *imagename) {
  char name[FILENAMESIZE];       /*base name of file */

  if (write_output) {
    strcpy(name, imagename);
    strcat (name, ".txt");
                                 //xiaofan
    textfile = open_file (name, "w");
  }
  if (write_raw_output) {
    strcpy(name, imagename);
    strcat (name, ".raw");
    rawfile = open_file (name, "w");
  }
  if (record_matcher_output) {
    strcpy(name, imagename);
    strcat (name, ".mlg");
    matcher_fp = open_file (name, "w");
    strcpy(name, imagename);
    strcat (name, ".ctx");
    correct_fp = open_file (name, "r");
  }
}


/**********************************************************************
 * end_recog
 *
 * Cleanup and exit the recog program.
 **********************************************************************/
int end_recog() {
  program_editdown (0);

  return (0);
}


/**********************************************************************
 * program_editdown
 *
 * This function holds any nessessary post processing for the Wise Owl
 * program.
 **********************************************************************/
void program_editdown(inT32 elasped_time) {
  dj_cleanup();
  if (display_text)
    cprintf ("\n");
  if (!NO_BLOCK && write_output)
    fprintf (textfile, "\n");
  if (write_raw_output)
    fprintf (rawfile, "\n");
  if (write_output) {
    #ifdef __UNIX__
    fsync (fileno (textfile));
    #endif
    fclose(textfile);
  }
  if (write_raw_output) {
    #ifdef __UNIX__
    fsync (fileno (rawfile));
    #endif
    fclose(rawfile);
  }
  close_choices();
  if (tessedit_save_stats)
    save_summary (elasped_time);
  end_match_table();
  InitChoiceAccum();
  if (global_hash != NULL) {
    free_mem(global_hash);
    global_hash = NULL;
  }
  end_metrics();
  end_permute();
  free_variables();
}


/**********************************************************************
 * set_pass1
 *
 * Get ready to do some pass 1 stuff.
 **********************************************************************/
void set_pass1() {
  blob_skip = FALSE;
  ok_split = 70.0;
  num_seg_states = 15;
  SettupPass1();
  first_pass = 1;
}


/**********************************************************************
 * set_pass2
 *
 * Get ready to do some pass 2 stuff.
 **********************************************************************/
void set_pass2() {
  blob_skip = FALSE;
  ok_split = pass2_ok_split;
  num_seg_states = pass2_seg_states;
  SettupPass2();
  first_pass = 0;
}


/**********************************************************************
 * cc_recog
 *
 * Recognize a word.
 **********************************************************************/
CHOICES_LIST cc_recog(TWERD *tessword,
                      A_CHOICE *best_choice,
                      A_CHOICE *best_raw_choice,
                      BOOL8 tester,
                      BOOL8 trainer) {
  int fx;
  CHOICES_LIST results;          /*matcher results */

  if (SetErrorTrap (NULL)) {
    cprintf ("Tess copped out!\n");
    ReleaseErrorTrap();
    class_string (best_choice) = NULL;
    return NULL;
  }
  InitChoiceAccum();
  init_match_table();
  for (fx = 0; fx < MAX_FX && (acts[OCR] & (FXSELECT << fx)) == 0; fx++);
  results =
    chop_word_main(tessword,
                   fx,
                   best_choice,
                   best_raw_choice,
                   tester,
                   trainer);
  DebugWordChoices();
  reset_hyphen_word();
  ReleaseErrorTrap();
  return results;
}


/**********************************************************************
 * dict_word()
 *
 * Test the dictionaries, returning NO_PERM (0) if not found, or one of the
 * DAWG_PERM values if found, according to the dictionary.
 **********************************************************************/
int dict_word(const char *word) {

  if (test_freq_words (word))
    return FREQ_DAWG_PERM;
  else
    return valid_word (word);
}
