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
/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "speckle.h"
#include "debug.h"
#include "blobs.h"

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
/* define control knobs for adjusting definition of speckle*/
make_float_var (MaxLargeSpeckleSize, 0.30, MakeMaxLargeSpeckleSize,
16, 2, SetMaxLargeSpeckleSize, "Max Large Speckle Size ...");

make_float_var (SmallSpecklePenalty, 10.0, MakeSmallSpecklePenalty,
16, 3, SetSmallSpecklePenalty, "Small Speckle Penalty ...");

make_float_var (LargeSpecklePenalty, 10.0, MakeLargeSpecklePenalty,
16, 4, SetLargeSpecklePenalty, "Large Speckle Penalty ...");

make_float_var (SmallSpeckleCertainty, -1.0, MakeSmallSpeckleCertainty,
16, 5, SetSmallSpeckleCertainty,
"Small Speckle Certainty ...");

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
LIST AddLargeSpeckleTo(LIST Choices) {
/*
 **	Parameters:
 **		Choices		choices to add a speckle choice to
 **	Globals:
 **		SmallSpecklePenalty	rating for a small speckle
 **		LargeSpecklePenalty	rating penalty for a large speckle
 **		SmallSpeckleCertainty	certainty for a small speckle
 **	Operation: This routine adds a null choice to Choices with a
 **		rating equal to the worst rating in Choices plus a pad.
 **		The certainty of the new choice is the same as the
 **		certainty of the worst choice in Choices.  The new choice
 **		is added to the end of Choices.
 **	Return: New Choices list with null choice added to end.
 **	Exceptions: none
 **	History: Mon Mar 11 11:08:11 1991, DSJ, Created.
 */
  LIST WorstChoice;
  char empty_lengths[] = {0};

  /* if there are no other choices, use the small speckle penalty plus
     the large speckle penalty */
  if (Choices == NIL)
    return (append_choice (NIL, "", empty_lengths, SmallSpecklePenalty + LargeSpecklePenalty,
      SmallSpeckleCertainty, -1));

  /* if there are other choices,  add a null choice that is slightly worse
     than the worst choice so far */
  WorstChoice = last (Choices);
  return (append_choice (Choices, "", empty_lengths,
    best_probability (WorstChoice) + LargeSpecklePenalty,
    best_certainty (WorstChoice), -1));

}                                /* AddLargeSpeckleTo */


/*---------------------------------------------------------------------------*/
void InitSpeckleVars() {
/*
 **	Parameters: none
 **	Globals: none
 **	Operation: Install the control variables needed for the speckle
 **		filters.
 **	Return: none
 **	Exceptions: none
 **	History: Mon Mar 11 12:04:59 1991, DSJ, Created.
 */
  MakeMaxLargeSpeckleSize();
  MakeSmallSpecklePenalty();
  MakeLargeSpecklePenalty();
  MakeSmallSpeckleCertainty();
}                                /* InitSpeckleVars */


/*---------------------------------------------------------------------------*/
BOOL8 LargeSpeckle(TBLOB *Blob, TEXTROW *Row) {
/*
 **	Parameters:
 **		Blob		blob to test against speckle criteria
 **		Row		text row that blob is in
 **	Globals:
 **		MaxLargeSpeckleSize	largest allowed speckle
 **	Operation: This routine returns TRUE if both the width of height
 **		of Blob are less than the MaxLargeSpeckleSize.
 **	Return: TRUE if Blob is speckle, FALSE otherwise.
 **	Exceptions: none
 **	History: Mon Mar 11 10:06:49 1991, DSJ, Created.
 */
  FLOAT32 SpeckleSize;
  TPOINT TopLeft;
  TPOINT BottomRight;

  SpeckleSize = RowHeight (Row) * MaxLargeSpeckleSize;
  blob_bounding_box(Blob, &TopLeft, &BottomRight);

  if (TopLeft.y - BottomRight.y < SpeckleSize &&
    BottomRight.x - TopLeft.x < SpeckleSize)
    return (TRUE);
  else
    return (FALSE);

}                                /* LargeSpeckle */
