/**********************************************************************
 * File:        adaptions.cpp  (Formerly adaptions.c)
 * Description: Functions used to adapt to blobs already confidently
 *              identified
 * Author:      Chris Newton
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

#include <cctype>
#include <cstring>
#include "tessvars.h"
#include "reject.h"
#include "control.h"
#include "stopper.h"
#include "tesseractclass.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

namespace tesseract {
bool Tesseract::word_adaptable(  //should we adapt?
        WERD_RES* word,
        uint16_t mode) {
  if (tessedit_adaption_debug) {
    tprintf("Running word_adaptable() for %s rating %.4f certainty %.4f\n",
          word->best_choice->unichar_string().string(),
          word->best_choice->rating(), word->best_choice->certainty());
  }

  bool status = false;
  BITS16 flags(mode);

  enum MODES
  {
    ADAPTABLE_WERD,
    ACCEPTABLE_WERD,
    CHECK_DAWGS,
    CHECK_SPACES,
    CHECK_ONE_ELL_CONFLICT,
    CHECK_AMBIG_WERD
  };

  /*
  0: NO adaption
  */
  if (mode == 0) {
    if (tessedit_adaption_debug) tprintf("adaption disabled\n");
    return false;
  }

  if (flags.bit (ADAPTABLE_WERD)) {
    status |= word->tess_would_adapt;  // result of Classify::AdaptableWord()
    if (tessedit_adaption_debug && !status) {
      tprintf("tess_would_adapt bit is false\n");
    }
  }

  if (flags.bit (ACCEPTABLE_WERD)) {
    status |= word->tess_accepted;
    if (tessedit_adaption_debug && !status) {
      tprintf("tess_accepted bit is false\n");
    }
  }

  if (!status) {                  // If not set then
    return false;                // ignore other checks
  }

  if (flags.bit (CHECK_DAWGS) &&
    (word->best_choice->permuter () != SYSTEM_DAWG_PERM) &&
    (word->best_choice->permuter () != FREQ_DAWG_PERM) &&
    (word->best_choice->permuter () != USER_DAWG_PERM) &&
    (word->best_choice->permuter () != NUMBER_PERM)) {
    if (tessedit_adaption_debug) tprintf("word not in dawgs\n");
    return false;
  }

  if (flags.bit (CHECK_ONE_ELL_CONFLICT) && one_ell_conflict (word, false)) {
    if (tessedit_adaption_debug) tprintf("word has ell conflict\n");
    return false;
  }

  if (flags.bit (CHECK_SPACES) &&
    (strchr(word->best_choice->unichar_string().string(), ' ') != nullptr)) {
    if (tessedit_adaption_debug) tprintf("word contains spaces\n");
    return false;
  }

  if (flags.bit (CHECK_AMBIG_WERD) &&
      word->best_choice->dangerous_ambig_found()) {
    if (tessedit_adaption_debug) tprintf("word is ambiguous\n");
    return false;
  }

  if (tessedit_adaption_debug) {
    tprintf("returning status %d\n", status);
  }
  return status;
}

}  // namespace tesseract
