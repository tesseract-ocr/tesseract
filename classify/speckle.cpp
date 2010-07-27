/******************************************************************************
 **	Filename:    speckle.c
 **	Purpose:     Routines used by classifier to filter out speckle.
 **	Author:      Dan Johnson
 **	History:     Mon Mar 11 10:06:14 1991, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/
/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/
#include "speckle.h"

#include "blobs.h"
#include "ratngs.h"
#include "varable.h"

/*-----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/
/** define control knobs for adjusting definition of speckle*/
double_VAR(speckle_large_max_size, 0.30, "Max large speckle size");

double_VAR(speckle_small_penalty, 10.0, "Small speckle penalty");

double_VAR(speckle_large_penalty, 10.0, "Large speckle penalty");

double_VAR(speckle_small_certainty, -1.0, "Small speckle certainty");

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * This routine adds a null choice to Choices with a
 * rating equal to the worst rating in Choices plus a pad.
 * The certainty of the new choice is the same as the
 * certainty of the worst choice in Choices.  The new choice
 * is added to the end of Choices.
 *
 * Globals:
 * - #speckle_small_penalty rating for a small speckle
 * - #speckle_large_penalty rating penalty for a large speckle
 * - #speckle_small_certainty certainty for a small speckle
 *
 * @param Choices choices to add a speckle choice to
 *
 * @return New Choices list with null choice added to end.
 *
 * Exceptions: none
 * History: Mon Mar 11 11:08:11 1991, DSJ, Created.
 */
void AddLargeSpeckleTo(BLOB_CHOICE_LIST *Choices) {
  assert(Choices != NULL);
  BLOB_CHOICE *blob_choice;
  BLOB_CHOICE_IT temp_it;
  temp_it.set_to_list(Choices);

  // If there are no other choices, use the small speckle penalty plus
  // the large speckle penalty.
  if (Choices->length() == 0) {
    blob_choice =
      new BLOB_CHOICE(0, speckle_small_certainty + speckle_large_penalty,
                      speckle_small_certainty, -1, NULL);
    temp_it.add_to_end(blob_choice);
    return;
  }

  // If there are other choices,  add a null choice that is slightly worse
  // than the worst choice so far.
  temp_it.move_to_last();
  blob_choice = temp_it.data();  // pick the worst choice
  temp_it.add_to_end(
      new BLOB_CHOICE(0, blob_choice->rating() + speckle_large_penalty,
                      blob_choice->certainty(), -1, NULL));
}                                /* AddLargeSpeckleTo */


/*---------------------------------------------------------------------------*/
/**
 * This routine returns TRUE if both the width of height
 * of Blob are less than the MaxLargeSpeckleSize.
 *
 * Globals:
 * - #speckle_large_max_size largest allowed speckle
 *
 * Exceptions: none
 * History: Mon Mar 11 10:06:49 1991, DSJ, Created.
 *
 * @param Blob blob to test against speckle criteria
 * @param Row text row that blob is in
 *
 * @return TRUE if Blob is speckle, FALSE otherwise.
 */
BOOL8 LargeSpeckle(TBLOB *Blob, TEXTROW *Row) {
  double speckle_size;
  TPOINT TopLeft;
  TPOINT BottomRight;

  speckle_size = RowHeight (Row) * speckle_large_max_size;
  blob_bounding_box(Blob, &TopLeft, &BottomRight);

  if (TopLeft.y - BottomRight.y < speckle_size &&
    BottomRight.x - TopLeft.x < speckle_size)
    return (TRUE);
  else
    return (FALSE);

}                                /* LargeSpeckle */
