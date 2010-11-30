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

#include "bestfirst.h"
#include "callcpp.h"
#include "chop.h"
#include "chopper.h"
#include "danerror.h"
#include "fxdefs.h"
#include "globals.h"
#include "gradechop.h"
#include "matchtab.h"
#include "pageres.h"
#include "permute.h"
#include "wordclass.h"
#include "wordrec.h"
#include "featdefs.h"

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
  if (init_dict) getDict().Load();
  pass2_ok_split = chop_ok_split;
  pass2_seg_states = wordrec_num_seg_states;
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
  blob_match_table.end_match_table();
  getDict().InitChoiceAccum();
  getDict().End();
}


/**
 * @name set_pass1
 *
 * Get ready to do some pass 1 stuff.
 */
void Wordrec::set_pass1() {
  chop_ok_split.set_value(70.0);
  wordrec_num_seg_states.set_value(15);
  SettupPass1();
}


/**
 * @name set_pass2
 *
 * Get ready to do some pass 2 stuff.
 */
void Wordrec::set_pass2() {
  chop_ok_split.set_value(pass2_ok_split);
  wordrec_num_seg_states.set_value(pass2_seg_states);
  SettupPass2();
}


/**
 * @name cc_recog
 *
 * Recognize a word.
 */
BLOB_CHOICE_LIST_VECTOR *Wordrec::cc_recog(WERD_RES *word) {
  getDict().InitChoiceAccum();
  getDict().reset_hyphen_vars(word->word->flag(W_EOL));
  blob_match_table.init_match_table();
  BLOB_CHOICE_LIST_VECTOR *results = chop_word_main(word);
  getDict().DebugWordChoices();
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
 * The blob may need rotating to the correct orientation for classification.
 */
BLOB_CHOICE_LIST *Wordrec::call_matcher(TBLOB *tessblob) {
  TBLOB* rotated_blob = NULL;
  // If necessary, copy the blob and rotate it.
  if (denorm_.block() != NULL &&
      denorm_.block()->classify_rotation().y() != 0.0) {
    TBOX box = tessblob->bounding_box();
    int src_width = box.width();
    int src_height = box.height();
    src_width = static_cast<int>(src_width / denorm_.y_scale() + 0.5);
    src_height = static_cast<int>(src_height / denorm_.y_scale() + 0.5);
    int x_middle = (box.left() + box.right()) / 2;
    int y_middle = (box.top() + box.bottom()) / 2;
    rotated_blob = new TBLOB(*tessblob);
    rotated_blob->Move(ICOORD(-x_middle, -y_middle));
    rotated_blob->Rotate(denorm_.block()->classify_rotation());
    tessblob = rotated_blob;
    ICOORD median_size = denorm_.block()->median_size();
    int tolerance = median_size.x() / 8;
    // TODO(dsl/rays) find a better normalization solution. In the mean time
    // make it work for CJK by normalizing for Cap height in the same way
    // as is applied in compute_block_xheight when the row is presumed to
    // be ALLCAPS, i.e. the x-height is the fixed fraction
    // blob height * CCStruct::kXHeightFraction /
    // (CCStruct::kXHeightFraction + CCStruct::kXAscenderFraction)
    if (NearlyEqual(src_width, static_cast<int>(median_size.x()), tolerance) &&
        NearlyEqual(src_height, static_cast<int>(median_size.y()), tolerance)) {
      float target_height = kBlnXHeight *
          (CCStruct::kXHeightFraction + CCStruct::kAscenderFraction) /
          CCStruct::kXHeightFraction;
      rotated_blob->Scale(target_height / box.width());
      rotated_blob->Move(ICOORD(0,
                                kBlnBaselineOffset -
                                rotated_blob->bounding_box().bottom()));
    }
  }
  BLOB_CHOICE_LIST *ratings = new BLOB_CHOICE_LIST();  // matcher result
  AdaptiveClassifier(tessblob, ratings, NULL);
  if (rotated_blob != NULL)
    delete rotated_blob;
  return ratings;
}


}  // namespace tesseract
