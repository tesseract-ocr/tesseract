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

#include "callcpp.h"
#include "chop.h"
#include "chopper.h"
#include "danerror.h"
#include "fxdefs.h"
#include "globals.h"
#include "gradechop.h"
#include "pageres.h"
#include "wordrec.h"
#include "featdefs.h"
#include "params_model.h"

#include <math.h>
#ifdef __UNIX__
#include <unistd.h>
#endif


namespace tesseract {

/**
 * @name program_editup
 *
 * Initialize all the things in the program that need to be initialized.
 * init_permute determines whether to initialize the permute functions
 * and Dawg models.
 */
void Wordrec::program_editup(const char *textbase,
                             bool init_classifier,
                             bool init_dict) {
  if (textbase != NULL) imagefile = textbase;
  InitFeatureDefs(&feature_defs_);
  SetupExtractors(&feature_defs_);
  InitAdaptiveClassifier(init_classifier);
  if (init_dict) getDict().Load(Dict::GlobalDawgCache());
  pass2_ok_split = chop_ok_split;
}

/**
 * @name end_recog
 *
 * Cleanup and exit the recog program.
 */
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
  EndAdaptiveClassifier();
  getDict().End();
}


/**
 * @name set_pass1
 *
 * Get ready to do some pass 1 stuff.
 */
void Wordrec::set_pass1() {
  chop_ok_split.set_value(70.0);
  language_model_->getParamsModel().SetPass(ParamsModel::PTRAIN_PASS1);
  SettupPass1();
}


/**
 * @name set_pass2
 *
 * Get ready to do some pass 2 stuff.
 */
void Wordrec::set_pass2() {
  chop_ok_split.set_value(pass2_ok_split);
  language_model_->getParamsModel().SetPass(ParamsModel::PTRAIN_PASS2);
  SettupPass2();
}


/**
 * @name cc_recog
 *
 * Recognize a word.
 */
void Wordrec::cc_recog(WERD_RES *word) {
  getDict().reset_hyphen_vars(word->word->flag(W_EOL));
  chop_word_main(word);
  word->DebugWordChoices(getDict().stopper_debug_level >= 1,
                         getDict().word_to_debug.string());
  ASSERT_HOST(word->StatesAllValid());
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
 * The blob may need rotating to the correct orientation for classification.
 */
BLOB_CHOICE_LIST *Wordrec::call_matcher(TBLOB *tessblob) {
  // Rotate the blob for classification if necessary.
  TBLOB* rotated_blob = tessblob->ClassifyNormalizeIfNeeded();
  if (rotated_blob == NULL) {
    rotated_blob = tessblob;
  }
  BLOB_CHOICE_LIST *ratings = new BLOB_CHOICE_LIST();  // matcher result
  AdaptiveClassifier(rotated_blob, ratings);
  if (rotated_blob != tessblob) {
    delete rotated_blob;
  }
  return ratings;
}


}  // namespace tesseract
