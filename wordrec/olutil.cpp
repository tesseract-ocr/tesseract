/* -*-C-*-
 ********************************************************************************
 *
 * File:        olutil.c  (Formerly olutil.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri May 17 13:11:24 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
              I n c l u d e s
----------------------------------------------------------------------*/
#include "olutil.h"
#include "structures.h"
#include "blobs.h"
#include "const.h"

#ifdef __UNIX__
#include <assert.h>
#endif

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * correct_blob_order
 *
 * Check to see if the blobs are in the correct order.  If they are not
 * then swap which outlines are attached to which blobs.
 **********************************************************************/
void correct_blob_order(TBLOB *blob1, TBLOB *blob2) { 
  TPOINT origin1;
  TPOINT origin2;
  TESSLINE *temp;

  blob_origin(blob1, &origin1); 
  blob_origin(blob2, &origin2); 

  if (origin1.x > origin2.x) {
    temp = blob2->outlines;
    blob2->outlines = blob1->outlines;
    blob1->outlines = temp;
  }
}


/**********************************************************************
 * eliminate_duplicate_outlines
 *
 * Find and delete any duplicate outline records in this blob.
 **********************************************************************/
void eliminate_duplicate_outlines(TBLOB *blob) { 
  TESSLINE *outline;
  TESSLINE *other_outline;
  TESSLINE *last_outline;

  for (outline = blob->outlines; outline; outline = outline->next) {

    for (last_outline = outline, other_outline = outline->next;
      other_outline;
    last_outline = other_outline, other_outline = other_outline->next) {

      if (same_outline_bounds (outline, other_outline)) {
        last_outline->next = other_outline->next;
        // This doesn't leak - the outlines share the EDGEPTs.
        other_outline->loop = NULL;
        delete other_outline;
        other_outline = last_outline;
        // If it is part of a cut, then it can't be a hole any more.
        outline->is_hole = false;
      }
    }
  }
}

/**********************************************************************
 * setup_blob_outlines
 *
 * Set up each of the outlines in this blob.
 **********************************************************************/
void setup_blob_outlines(TBLOB *blob) { 
  TESSLINE *outline;

  for (outline = blob->outlines; outline; outline = outline->next) {
    outline->ComputeBoundingBox();
  }
}
