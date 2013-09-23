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
#include <assert.h>
#include <stdio.h>

#include "associate.h"
#include "render.h"
#include "callcpp.h"
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

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
 * @param blob Current blob
 * @param string The string to display in ScrollView
 * @param color The colour to use when displayed with ScrollView
 */
BLOB_CHOICE_LIST *Wordrec::classify_blob(TBLOB *blob,
                                         const char *string, C_COL color,
                                         BlamerBundle *blamer_bundle) {
#ifndef GRAPHICS_DISABLED
  if (wordrec_display_all_blobs)
    display_blob(blob, color);
#endif
  // TODO(rays) collapse with call_matcher and move all to wordrec.cpp.
  BLOB_CHOICE_LIST* choices = call_matcher(blob);
  // If a blob with the same bounding box as one of the truth character
  // bounding boxes is not classified as the corresponding truth character
  // blame character classifier for incorrect answer.
  if (blamer_bundle != NULL) {
    blamer_bundle->BlameClassifier(getDict().getUnicharset(),
                                   blob->bounding_box(),
                                   *choices,
                                   wordrec_debug_blamer);
  }
  #ifndef GRAPHICS_DISABLED
  if (classify_debug_level && string)
    print_ratings_list(string, choices, getDict().getUnicharset());

  if (wordrec_blob_pause)
    window_wait(blob_window);
#endif

  return choices;
}

}  // namespace tesseract;
