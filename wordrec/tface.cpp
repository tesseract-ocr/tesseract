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
/* includes for init */
#include "tessinit.h"
#include "mfvars.h"
#include "metrics.h"
#include "adaptmatch.h"
#include "matchtab.h"
#include "chopper.h"
#include "permdawg.h"
#include "permute.h"
#include "chop.h"
#include "callcpp.h"
#include "badwords.h"
#include "wordrec.h"

#include <math.h>
#ifdef __UNIX__
#include <unistd.h>
#endif

const int kReallyBadCertainty = -20;

namespace tesseract {
  class Tesseract;
}

//extern "C" int record_matcher_output;

/*----------------------------------------------------------------------
              Variables
----------------------------------------------------------------------*/
static PRIORITY pass2_ok_split;
static int pass2_seg_states;

BOOL_VAR(wordrec_no_block, false, "Don't output block information");

/*----------------------------------------------------------------------
              Function Code
----------------------------------------------------------------------*/
namespace tesseract {
/**
 * @name start_recog
 *
 * Startup recog program ready to recognize words.
 */
int Wordrec::start_recog(const char *textbase) {

  program_editup(textbase, true);
  return (0);
}


/**
 * @name program_editup
 *
 * Initialize all the things in the program that need to be initialized.
 * init_permute determines whether to initialize the permute functions
 * and Dawg models.
 */
void Wordrec::program_editup(const char *textbase, bool init_permute) {
  if (textbase != NULL) {
    imagefile = textbase;
    /* Read in data files */
    edit_with_ocr(textbase);
  }

  /* Initialize subsystems */
  program_init();
  mfeature_init();  // assumes that imagefile is initialized
  if (init_permute)
    getDict().init_permute();
  setup_cp_maps();

  init_metrics();
  pass2_ok_split = chop_ok_split;
  pass2_seg_states = wordrec_num_seg_states;
}
}  // namespace tesseract


/**
 * @name edit_with_ocr
 *
 * Initialize all the things in the program needed before the classifier
 * code is called.
 */
void edit_with_ocr(const char *imagename) {
  char name[FILENAMESIZE];       /*base name of file */

  if (tord_write_output) {
    strcpy(name, imagename);
    strcat (name, ".txt");
                                 //xiaofan
    textfile = open_file (name, "w");
  }
  if (tord_write_raw_output) {
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


/**
 * @name end_recog
 *
 * Cleanup and exit the recog program.
 */
namespace tesseract {
int Wordrec::end_recog() {
  program_editdown (0);

  return (0);
}


/**
 * @name program_editdown
 *
 * This function holds any nessessary post processing for the Wise Owl
 * program.
 */
void Wordrec::program_editdown(inT32 elasped_time) {
  dj_cleanup();
  if (tord_display_text)
    cprintf ("\n");
  if (!wordrec_no_block && tord_write_output)
    fprintf (textfile, "\n");
  if (tord_write_raw_output)
    fprintf (rawfile, "\n");
  if (tord_write_output) {
    #ifdef __UNIX__
    fsync (fileno (textfile));
    #endif
    fclose(textfile);
  }
  if (tord_write_raw_output) {
    #ifdef __UNIX__
    fsync (fileno (rawfile));
    #endif
    fclose(rawfile);
  }
  close_choices();
  if (tessedit_save_stats)
    save_summary (elasped_time);
  end_match_table();
  getDict().InitChoiceAccum();
  if (global_hash != NULL) {
    free_mem(global_hash);
    global_hash = NULL;
  }
  end_metrics();
  getDict().end_permute();
}


/**
 * @name set_pass1
 *
 * Get ready to do some pass 1 stuff.
 */
void Wordrec::set_pass1() {
  tord_blob_skip.set_value(false);
  chop_ok_split.set_value(70.0);
  wordrec_num_seg_states.set_value(15);
  SettupPass1();
  first_pass = 1;
}


/**
 * @name set_pass2
 *
 * Get ready to do some pass 2 stuff.
 */
void Wordrec::set_pass2() {
  tord_blob_skip.set_value(false);
  chop_ok_split.set_value(pass2_ok_split);
  wordrec_num_seg_states.set_value(pass2_seg_states);
  SettupPass2();
  first_pass = 0;
}


/**
 * @name cc_recog
 *
 * Recognize a word.
 */
BLOB_CHOICE_LIST_VECTOR *Wordrec::cc_recog(TWERD *tessword,
                                           WERD_CHOICE *best_choice,
                                           WERD_CHOICE *best_raw_choice,
                                           BOOL8 tester,
                                           BOOL8 trainer,
                                           bool last_word_on_line) {
  int fx;
  BLOB_CHOICE_LIST_VECTOR *results;          /*matcher results */

  if (SetErrorTrap (NULL)) {
    cprintf ("Tess copped out!\n");
    ReleaseErrorTrap();
    class_string (best_choice) = NULL;
    return NULL;
  }
  getDict().InitChoiceAccum();
  getDict().reset_hyphen_vars(last_word_on_line);
  init_match_table();
  for (fx = 0; fx < MAX_FX && (acts[OCR] & (FXSELECT << fx)) == 0; fx++);
  results =
    chop_word_main(tessword,
                   fx,
                   best_choice,
                   best_raw_choice,
                   tester,
                   trainer);
  getDict().DebugWordChoices();
  ReleaseErrorTrap();
  return results;
}


/**
 * @name dict_word()
 *
 * Test the dictionaries, returning NO_PERM (0) if not found, or one
 * of the PermuterType values if found, according to the dictionary.
 */
int Wordrec::dict_word(const WERD_CHOICE &word) {
  return getDict().valid_word(word);
}

/**
 * @name call_matcher
 *
 * Called from Tess with a blob in tess form.
 * Convert the blob to editor form.
 * Call the matcher setup by the segmenter in tess_matcher.
 * Convert the output choices back to tess form.
 */
BLOB_CHOICE_LIST *Wordrec::call_matcher(TBLOB *ptblob,    //< previous blob
                                        TBLOB *tessblob,  //< blob to match
                                        TBLOB *ntblob,    //< next blob
                                        void *,           //< unused parameter
                                        TEXTROW *         //< always null anyway
                                        ) {
  PBLOB *pblob;                  //converted blob
  PBLOB *blob;                   //converted blob
  PBLOB *nblob;                  //converted blob
  BLOB_CHOICE_LIST *ratings = new BLOB_CHOICE_LIST();  // matcher result

  blob = make_ed_blob (tessblob);//convert blob
  if (blob == NULL) {
    // Since it is actually possible to get a NULL blob here, due to invalid
    // segmentations, fake a really bad classification.
    BLOB_CHOICE *choice =
      new BLOB_CHOICE(0, static_cast<float>(MAX_NUM_INT_FEATURES),
                      static_cast<float>(-MAX_FLOAT32), 0, NULL);
    BLOB_CHOICE_IT temp_it;
    temp_it.set_to_list(ratings);
    temp_it.add_after_stay_put(choice);
    return ratings;
  }
  pblob = ptblob != NULL ? make_ed_blob (ptblob) : NULL;
  nblob = ntblob != NULL ? make_ed_blob (ntblob) : NULL;
  // Because of the typedef for tess_matcher, the object on which it is called
  // must be of type Tesseract*. With a Wordrec type it seems it doesn't work.
  (reinterpret_cast<Tesseract* const>(this)->*tess_matcher)
      (pblob, blob, nblob, tess_word, tess_denorm, ratings, NULL);

  //match it
  delete blob;                   //don't need that now
  if (pblob != NULL)
    delete pblob;
  if (nblob != NULL)
    delete nblob;
  return ratings;
}

/**
 * @name make_ed_blob
 *
 * Make an editor format blob from the tess style blob.
 */

PBLOB *make_ed_blob(                 //construct blob
                    TBLOB *tessblob  //< blob to convert
                   ) {
  TESSLINE *tessol;              //tess outline
  FRAGMENT_LIST fragments;       //list of fragments
  OUTLINE *outline;              //current outline
  OUTLINE_LIST out_list;         //list of outlines
  OUTLINE_IT out_it = &out_list; //iterator

  for (tessol = tessblob->outlines; tessol != NULL; tessol = tessol->next) {
                                 //stick in list
    register_outline(tessol, &fragments);
  }
  while (!fragments.empty ()) {
    outline = make_ed_outline (&fragments);
    if (outline != NULL) {
      out_it.add_after_then_move (outline);
    }
  }
  if (out_it.empty())
    return NULL;                 //couldn't do it
  return new PBLOB (&out_list);  //turn to blob
}
/**
 * @name make_ed_outline
 *
 * Make an editor format outline from the list of fragments.
 */

OUTLINE *make_ed_outline(                     //constructoutline
                         FRAGMENT_LIST *list  //< list of fragments
                        ) {
  FRAGMENT *fragment;            //current fragment
  EDGEPT *edgept;                //current point
  ICOORD headpos;                //coords of head
  ICOORD tailpos;                //coords of tail
  FCOORD pos;                    //coords of edgept
  FCOORD vec;                    //empty
  POLYPT *polypt;                //current point
  POLYPT_LIST poly_list;         //list of point
  POLYPT_IT poly_it = &poly_list;//iterator
  FRAGMENT_IT fragment_it = list;//fragment

  headpos = fragment_it.data ()->head;
  do {
    fragment = fragment_it.data ();
    edgept = fragment->headpt;   //start of segment
    do {
      pos = FCOORD (edgept->pos.x, edgept->pos.y);
      vec = FCOORD (edgept->vec.x, edgept->vec.y);
      polypt = new POLYPT (pos, vec);
                                 //add to list
      poly_it.add_after_then_move (polypt);
      edgept = edgept->next;
    }
    while (edgept != fragment->tailpt);
    tailpos = ICOORD (edgept->pos.x, edgept->pos.y);
                                 //get rid of it
    delete fragment_it.extract ();
    if (tailpos != headpos) {
      if (fragment_it.empty ()) {
        return NULL;
      }
      fragment_it.forward ();
                                 //find next segment
      for (fragment_it.mark_cycle_pt (); !fragment_it.cycled_list () &&
               fragment_it.data ()->head != tailpos;
        fragment_it.forward ());
      if (fragment_it.data ()->head != tailpos) {
        // It is legitimate for the heads to not all match to tails,
        // since not all combinations of seams always make sense.
        for (fragment_it.mark_cycle_pt ();
        !fragment_it.cycled_list (); fragment_it.forward ()) {
          fragment = fragment_it.extract ();
          delete fragment;
        }
        return NULL;             //can't do it
      }
    }
  }
  while (tailpos != headpos);
  return new OUTLINE (&poly_it); //turn to outline
}
/**
 * @name register_outline
 *
 * Add the fragments in the given outline to the list
 */

void register_outline(                     //add fragments
                      TESSLINE *outline,   //< tess format
                      FRAGMENT_LIST *list  //< list to add to
                     ) {
  EDGEPT *startpt;               //start of outline
  EDGEPT *headpt;                //start of fragment
  EDGEPT *tailpt;                //end of fragment
  FRAGMENT *fragment;            //new fragment
  FRAGMENT_IT it = list;         //iterator

  startpt = outline->loop;
  do {
    startpt = startpt->next;
    if (startpt == NULL)
      return;                    //illegal!
  }
  while (startpt->flags[0] == 0 && startpt != outline->loop);
  headpt = startpt;
  do
  startpt = startpt->next;
  while (startpt->flags[0] != 0 && startpt != headpt);
  if (startpt->flags[0] != 0)
    return;                      //all hidden!

  headpt = startpt;
  do {
    tailpt = headpt;
    do
    tailpt = tailpt->next;
    while (tailpt->flags[0] == 0 && tailpt != startpt);
    fragment = new FRAGMENT (headpt, tailpt);
    it.add_after_then_move (fragment);
    while (tailpt->flags[0] != 0)
      tailpt = tailpt->next;
    headpt = tailpt;
  }
  while (tailpt != startpt);
}

ELISTIZE (FRAGMENT)

/**
 * @name FRAGMENT::FRAGMENT
 *
 * Constructor for fragments.
 */
FRAGMENT::FRAGMENT (             //constructor
EDGEPT * head_pt,                //< start point
EDGEPT * tail_pt                 //< end point
):head (head_pt->pos.x, head_pt->pos.y), tail (tail_pt->pos.x,
tail_pt->pos.y) {
  headpt = head_pt;              // save ptrs
  tailpt = tail_pt;
}

}  // namespace tesseract
