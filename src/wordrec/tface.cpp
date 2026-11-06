/**********************************************************************
 * File:        tface.cpp  (Formerly tface.c)
 * Description: C side of the Tess/tessedit C/C++ interface.
 * Author:      Ray Smith
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

#include <cmath>

#include "wordrec.h"

#ifndef DISABLED_LEGACY_ENGINE
#  include "chop.h"
#  include "featdefs.h"
#  include "pageres.h"
#  include "params_model.h"
#  include "tesserrstream.h"  // for tesserr
#endif

namespace tesseract {

/**
 * @name program_editup
 *
 * Initialize all the things in the program that need to be initialized.
 * init_permute determines whether to initialize the permute functions
 * and Dawg models.
 */
bool Wordrec::program_editup(const std::string &textbase, TessdataManager *init_classifier,
                             TessdataManager *init_dict) {
  if (!textbase.empty()) {
    imagefile = textbase;
  }
#ifndef DISABLED_LEGACY_ENGINE
  InitFeatureDefs(&feature_defs_);
  if (!InitAdaptiveClassifier(init_classifier)) {
    return false;
  }
  if (init_dict) {
    getDict().SetupForLoad(Dict::GlobalDawgCache());
    getDict().Load(lang, init_dict);
    getDict().FinishLoad();
  }
  pass2_ok_split = chop_ok_split;
#endif // ndef DISABLED_LEGACY_ENGINE
  return true;
}

/**
 * @name end_recog
 *
 * Cleanup and exit the recog program.
 */
int Wordrec::end_recog() {
  program_editdown();

  return (0);
}

/**
 * @name program_editdown
 *
 * This function holds any necessary post processing for the Wise Owl
 * program.
 */
void Wordrec::program_editdown() {
#ifndef DISABLED_LEGACY_ENGINE
  EndAdaptiveClassifier();
#endif // ndef DISABLED_LEGACY_ENGINE
  getDict().End();
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

#ifndef DISABLED_LEGACY_ENGINE

/**
 * @name set_pass1
 *
 * Get ready to do some pass 1 stuff.
 */
void Wordrec::set_pass1() {
  chop_ok_split.set_value(70.0);
  language_model_->getParamsModel().SetPass(ParamsModel::PTRAIN_PASS1);
  SetupPass1();
}

/**
 * @name set_pass2
 *
 * Get ready to do some pass 2 stuff.
 */
void Wordrec::set_pass2() {
  chop_ok_split.set_value(pass2_ok_split);
  language_model_->getParamsModel().SetPass(ParamsModel::PTRAIN_PASS2);
  SetupPass2();
}

/**
 * @name cc_recog
 *
 * Recognize a word.
 */
void Wordrec::cc_recog(WERD_RES *word) {
  getDict().reset_hyphen_vars(word->word->flag(W_EOL));
  chop_word_main(word);
  word->DebugWordChoices(getDict().stopper_debug_level >= 1, getDict().word_to_debug.c_str());
  ASSERT_HOST(word->StatesAllValid());
}

/**
 * @name call_matcher
 *
 * Called from Tess with a blob in tess form.
 * The blob may need rotating to the correct orientation for classification.
 */
BLOB_CHOICE_LIST *Wordrec::call_matcher(TBLOB *tessblob) {
  // The per-pass blob-classification budget may already be exhausted. It is
  // set at the start of the recognition pass (see
  // Tesseract::recog_all_words()/Tesseract::ApplyBoxes(), which call
  // ResetClassificationBudget() for the primary language and all sub-languages)
  // and decremented below for every blob that actually reaches the adaptive
  // classifier. Once exhausted, stop classifying and hand back a single junk
  // (speckle) choice, which is exactly what the classifier returns for a blob
  // it cannot match (AddLargeSpeckleTo() on an empty result list in
  // AdaptiveClassifier()). This bounds the worst-case cost of the legacy
  // classifier on degenerate inputs where pageseg produces a large number of
  // blob-like pieces with no inter-word gaps (OSS-Fuzz issue 581720522).
  if (classifications_pass_remaining_ < 0) {
    // Lazy fallback for classification paths that are not covered by the pass
    // entry points (e.g. the ambigs training path via
    // recog_training_segmented()). This still gives them a bounded budget for
    // the lifetime of the Tesseract object, but the "per pass" guarantee does
    // not hold on those paths.
    ResetClassificationBudget();
  }
  auto *ratings = new BLOB_CHOICE_LIST();
  if (classifications_pass_remaining_ <= 0) {
    // Budget exhausted: assign a single junk choice and skip the (expensive)
    // class pruner + integer matcher.
    if (!classifications_pass_warning_) {
      tesserr << "WARNING: reached the blob classification budget "
             << "(chop_max_classifications_pass = "
             << (int)chop_max_classifications_pass
             << ") for this recognition pass; assigning junk choices to the "
             << "remaining blobs instead of classifying them.\n";
      classifications_pass_warning_ = true;
    }
    AddLargeSpeckleTo(0, ratings);
    return ratings;
  }
  --classifications_pass_remaining_;
  // Rotate the blob for classification if necessary.
  TBLOB *rotated_blob = tessblob->ClassifyNormalizeIfNeeded();
  if (rotated_blob == nullptr) {
    rotated_blob = tessblob;
  }
  AdaptiveClassifier(rotated_blob, ratings);
  if (rotated_blob != tessblob) {
    delete rotated_blob;
  }
  return ratings;
}

#endif // ndef DISABLED_LEGACY_ENGINE

} // namespace tesseract
