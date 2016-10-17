/******************************************************************************
 ** Filename:    mfoutline.c
 ** Purpose:     Interface to outline struct used for extracting features
 ** Author:      Dan Johnson
 ** History:     Thu May 17 08:14:18 1990, DSJ, Created.
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
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
/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/
#include "clusttool.h"           //If remove you get cought in a loop somewhere
#include "emalloc.h"
#include "mfoutline.h"
#include "blobs.h"
#include "const.h"
#include "mfx.h"
#include "params.h"
#include "classify.h"

#include <math.h>
#include <stdio.h>

/*----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/** Convert a blob into a list of MFOUTLINEs (float-based microfeature format). */
LIST ConvertBlob(TBLOB *blob) {
  LIST outlines = NIL_LIST;
  return (blob == NULL)
      ? NIL_LIST
      : ConvertOutlines(blob->outlines, outlines, outer);
}


/*---------------------------------------------------------------------------*/
/** Convert a TESSLINE into the float-based MFOUTLINE micro-feature format. */
MFOUTLINE ConvertOutline(TESSLINE *outline) {
  MFEDGEPT *NewPoint;
  MFOUTLINE MFOutline = NIL_LIST;
  EDGEPT *EdgePoint;
  EDGEPT *StartPoint;
  EDGEPT *NextPoint;

  if (outline == NULL || outline->loop == NULL)
    return MFOutline;

  StartPoint = outline->loop;
  EdgePoint = StartPoint;
  do {
    NextPoint = EdgePoint->next;

    /* filter out duplicate points */
    if (EdgePoint->pos.x != NextPoint->pos.x ||
        EdgePoint->pos.y != NextPoint->pos.y) {
      NewPoint = NewEdgePoint();
      ClearMark(NewPoint);
      NewPoint->Hidden = EdgePoint->IsHidden();
      NewPoint->Point.x = EdgePoint->pos.x;
      NewPoint->Point.y = EdgePoint->pos.y;
      MFOutline = push(MFOutline, NewPoint);
    }
    EdgePoint = NextPoint;
  } while (EdgePoint != StartPoint);

  if (MFOutline != NULL)
    MakeOutlineCircular(MFOutline);
  return MFOutline;
}


/*---------------------------------------------------------------------------*/
/**
 * Convert a tree of outlines to a list of MFOUTLINEs (lists of MFEDGEPTs).
 *
 * @param outline      first outline to be converted
 * @param mf_outlines  list to add converted outlines to
 * @param outline_type  are the outlines outer or holes?
 */
LIST ConvertOutlines(TESSLINE *outline,
                     LIST mf_outlines,
                     OUTLINETYPE outline_type) {
  MFOUTLINE mf_outline;

  while (outline != NULL) {
    mf_outline = ConvertOutline(outline);
    if (mf_outline != NULL)
      mf_outlines = push(mf_outlines, mf_outline);
    outline = outline->next;
  }
  return mf_outlines;
}

/*---------------------------------------------------------------------------*/
/**
 * This routine searches through the specified outline, computes
 * a slope for each vector in the outline, and marks each
 * vector as having one of the following directions:
 *   N, S, E, W, NE, NW, SE, SW
 * This information is then stored in the outline and the
 * outline is returned.
 * @param Outline   micro-feature outline to analyze
 * @param MinSlope  controls "snapping" of segments to horizontal
 * @param MaxSlope  controls "snapping" of segments to vertical
 * @return none
 * @note Exceptions: none
 * @note History: 7/21/89, DSJ, Created.
 */
void FindDirectionChanges(MFOUTLINE Outline,
                          FLOAT32 MinSlope,
                          FLOAT32 MaxSlope) {
  MFEDGEPT *Current;
  MFEDGEPT *Last;
  MFOUTLINE EdgePoint;

  if (DegenerateOutline (Outline))
    return;

  Last = PointAt (Outline);
  Outline = NextPointAfter (Outline);
  EdgePoint = Outline;
  do {
    Current = PointAt (EdgePoint);
    ComputeDirection(Last, Current, MinSlope, MaxSlope);

    Last = Current;
    EdgePoint = NextPointAfter (EdgePoint);
  }
  while (EdgePoint != Outline);

}                                /* FindDirectionChanges */


/*---------------------------------------------------------------------------*/
/**
 * This routine deallocates all of the memory consumed by
 * a micro-feature outline.
 * @param arg   micro-feature outline to be freed
 * @return none
 * @note Exceptions: none
 * @note History: 7/27/89, DSJ, Created.
 */
void FreeMFOutline(void *arg) {  //MFOUTLINE                             Outline)
  MFOUTLINE Start;
  MFOUTLINE Outline = (MFOUTLINE) arg;

  /* break the circular outline so we can use std. techniques to deallocate */
  Start = list_rest (Outline);
  set_rest(Outline, NIL_LIST);
  while (Start != NULL) {
    free_struct (first_node (Start), sizeof (MFEDGEPT), "MFEDGEPT");
    Start = pop (Start);
  }

}                                /* FreeMFOutline */


/*---------------------------------------------------------------------------*/
/**
 * Release all memory consumed by the specified list
 * of outlines.
 * @param Outlines  list of mf-outlines to be freed
 * @return none
 * @note Exceptions: none
 * @note History: Thu Dec 13 16:14:50 1990, DSJ, Created.
 */
void FreeOutlines(LIST Outlines) {
  destroy_nodes(Outlines, FreeMFOutline);
}                                /* FreeOutlines */


/*---------------------------------------------------------------------------*/
/**
 * This routine searches through the specified outline and finds
 * the points at which the outline changes direction.  These
 * points are then marked as "extremities".  This routine is
 * used as an alternative to FindExtremities().  It forces the
 * endpoints of the microfeatures to be at the direction
 * changes rather than at the midpoint between direction
 * changes.
 * @param Outline   micro-feature outline to analyze
 * @return none
 * @note Globals: none
 * @note Exceptions: none
 * @note History: 6/29/90, DSJ, Created.
 */
void MarkDirectionChanges(MFOUTLINE Outline) {
  MFOUTLINE Current;
  MFOUTLINE Last;
  MFOUTLINE First;

  if (DegenerateOutline (Outline))
    return;

  First = NextDirectionChange (Outline);
  Last = First;
  do {
    Current = NextDirectionChange (Last);
    MarkPoint (PointAt (Current));
    Last = Current;
  }
  while (Last != First);

}                                /* MarkDirectionChanges */


/*---------------------------------------------------------------------------*/
/** Return a new edge point for a micro-feature outline. */
MFEDGEPT *NewEdgePoint() {
  return ((MFEDGEPT *) alloc_struct(sizeof(MFEDGEPT), "MFEDGEPT"));
}


/*---------------------------------------------------------------------------*/
/**
 * This routine returns the next point in the micro-feature
 * outline that is an extremity.  The search starts after
 * EdgePoint.  The routine assumes that the outline being
 * searched is not a degenerate outline (i.e. it must have
 * 2 or more edge points).
 * @param EdgePoint start search from this point
 * @return Next extremity in the outline after EdgePoint.
 * @note Globals: none
 * @note Exceptions: none
 * @note History: 7/26/89, DSJ, Created.
 */
MFOUTLINE NextExtremity(MFOUTLINE EdgePoint) {
  EdgePoint = NextPointAfter(EdgePoint);
  while (!PointAt(EdgePoint)->ExtremityMark)
    EdgePoint = NextPointAfter(EdgePoint);

  return (EdgePoint);

}                                /* NextExtremity */


/*---------------------------------------------------------------------------*/
/**
 * This routine normalizes the coordinates of the specified
 * outline so that the outline is deskewed down to the
 * baseline, translated so that x=0 is at XOrigin, and scaled
 * so that the height of a character cell from descender to
 * ascender is 1.  Of this height, 0.25 is for the descender,
 * 0.25 for the ascender, and 0.5 for the x-height.  The
 * y coordinate of the baseline is 0.
 * @param Outline   outline to be normalized
 * @param XOrigin   x-origin of text
 * @return none
 * @note Globals: none
 * @note Exceptions: none
 * @note History: 8/2/89, DSJ, Created.
 */
void NormalizeOutline(MFOUTLINE Outline,
                      FLOAT32 XOrigin) {
  if (Outline == NIL_LIST)
    return;

  MFOUTLINE EdgePoint = Outline;
  do {
    MFEDGEPT *Current = PointAt(EdgePoint);
    Current->Point.y = MF_SCALE_FACTOR *
        (Current->Point.y - kBlnBaselineOffset);
    Current->Point.x = MF_SCALE_FACTOR * (Current->Point.x - XOrigin);
    EdgePoint = NextPointAfter(EdgePoint);
  } while (EdgePoint != Outline);
}                                /* NormalizeOutline */


/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 * This routine normalizes every outline in Outlines
 * according to the currently selected normalization method.
 * It also returns the scale factors that it used to do this
 * scaling.  The scale factors returned represent the x and
 * y sizes in the normalized coordinate system that correspond
 * to 1 pixel in the original coordinate system.
 *
 * Globals:
 * - classify_norm_method  method being used for normalization
 * - classify_char_norm_range map radius of gyration to this value
 * @param Outlines  list of outlines to be normalized
 * @param XScale    x-direction scale factor used by routine
 * @param YScale    y-direction scale factor used by routine
 * @return none (Outlines are changed and XScale and YScale are updated)
 * @note Exceptions: none
 * @note History: Fri Dec 14 08:14:55 1990, DSJ, Created.
 */
void Classify::NormalizeOutlines(LIST Outlines,
                                 FLOAT32 *XScale,
                                 FLOAT32 *YScale) {
  MFOUTLINE Outline;

  switch (classify_norm_method) {
    case character:
      ASSERT_HOST(!"How did NormalizeOutlines get called in character mode?");
      break;

    case baseline:
      iterate(Outlines) {
        Outline = (MFOUTLINE) first_node(Outlines);
        NormalizeOutline(Outline, 0.0);
      }
      *XScale = *YScale = MF_SCALE_FACTOR;
      break;
  }
}                                /* NormalizeOutlines */
}  // namespace tesseract

/*----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------*/
/**
 * Change the direction of every vector in the specified
 * outline segment to Direction.  The segment to be changed
 * starts at Start and ends at End.  Note that the previous
 * direction of End must also be changed to reflect the
 * change in direction of the point before it.
 * @param Start, End  defines segment of outline to be modified
 * @param Direction new direction to assign to segment
 * @return none
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Fri May  4 10:42:04 1990, DSJ, Created.
 */
void ChangeDirection(MFOUTLINE Start, MFOUTLINE End, DIRECTION Direction) {
  MFOUTLINE Current;

  for (Current = Start; Current != End; Current = NextPointAfter (Current))
    PointAt (Current)->Direction = Direction;

  PointAt (End)->PreviousDirection = Direction;

}                                /* ChangeDirection */


/**
 * This routine normalizes each point in Outline by
 * translating it to the specified center and scaling it
 * anisotropically according to the given scale factors.
 * @param Outline     outline to be character normalized
 * @param cn_denorm
 * @return none
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Fri Dec 14 10:27:11 1990, DSJ, Created.
 */
void CharNormalizeOutline(MFOUTLINE Outline, const DENORM& cn_denorm) {
  MFOUTLINE First, Current;
  MFEDGEPT *CurrentPoint;

  if (Outline == NIL_LIST)
    return;

  First = Outline;
  Current = First;
  do {
    CurrentPoint = PointAt(Current);
    FCOORD pos(CurrentPoint->Point.x, CurrentPoint->Point.y);
    cn_denorm.LocalNormTransform(pos, &pos);
    CurrentPoint->Point.x = (pos.x() - MAX_UINT8 / 2) * MF_SCALE_FACTOR;
    CurrentPoint->Point.y = (pos.y() - MAX_UINT8 / 2) * MF_SCALE_FACTOR;

    Current = NextPointAfter(Current);
  }
  while (Current != First);

}                                /* CharNormalizeOutline */


/**
 * This routine computes the slope from Start to Finish and
 * and then computes the approximate direction of the line
 * segment from Start to Finish.  The direction is quantized
 * into 8 buckets:
 *  N, S, E, W, NE, NW, SE, SW
 * Both the slope and the direction are then stored into
 * the appropriate fields of the Start edge point.  The
 * direction is also stored into the PreviousDirection field
 * of the Finish edge point.
 * @param Start   starting point to compute direction from
 * @param Finish    finishing point to compute direction to
 * @param MinSlope  slope below which lines are horizontal
 * @param MaxSlope  slope above which lines are vertical
 * @return none
 * @note Globals: none
 * @note Exceptions: none
 * @note History: 7/25/89, DSJ, Created.
 */
void ComputeDirection(MFEDGEPT *Start,
                      MFEDGEPT *Finish,
                      FLOAT32 MinSlope,
                      FLOAT32 MaxSlope) {
  FVECTOR Delta;

  Delta.x = Finish->Point.x - Start->Point.x;
  Delta.y = Finish->Point.y - Start->Point.y;
  if (Delta.x == 0)
  if (Delta.y < 0) {
    Start->Slope = -MAX_FLOAT32;
    Start->Direction = south;
  }
  else {
    Start->Slope = MAX_FLOAT32;
    Start->Direction = north;
  }
  else {
    Start->Slope = Delta.y / Delta.x;
    if (Delta.x > 0)
      if (Delta.y > 0)
        if (Start->Slope > MinSlope)
          if (Start->Slope < MaxSlope)
            Start->Direction = northeast;
    else
      Start->Direction = north;
    else
      Start->Direction = east;
    else if (Start->Slope < -MinSlope)
    if (Start->Slope > -MaxSlope)
      Start->Direction = southeast;
    else
      Start->Direction = south;
    else
      Start->Direction = east;
    else if (Delta.y > 0)
    if (Start->Slope < -MinSlope)
      if (Start->Slope > -MaxSlope)
        Start->Direction = northwest;
    else
      Start->Direction = north;
    else
      Start->Direction = west;
    else if (Start->Slope > MinSlope)
    if (Start->Slope < MaxSlope)
      Start->Direction = southwest;
    else
      Start->Direction = south;
    else
      Start->Direction = west;
  }
  Finish->PreviousDirection = Start->Direction;
}

/**
 * This routine returns the next point in the micro-feature
 * outline that has a direction different than EdgePoint.  The
 * routine assumes that the outline being searched is not a
 * degenerate outline (i.e. it must have 2 or more edge points).
 * @param EdgePoint start search from this point
 * @return Point of next direction change in micro-feature outline.
 * @note Globals: none
 * @note Exceptions: none
 * @note History: 7/25/89, DSJ, Created.
 */
MFOUTLINE NextDirectionChange(MFOUTLINE EdgePoint) {
  DIRECTION InitialDirection;

  InitialDirection = PointAt (EdgePoint)->Direction;

  MFOUTLINE next_pt = NULL;
  do {
    EdgePoint = NextPointAfter(EdgePoint);
    next_pt = NextPointAfter(EdgePoint);
  } while (PointAt(EdgePoint)->Direction == InitialDirection &&
           !PointAt(EdgePoint)->Hidden &&
           next_pt != NULL && !PointAt(next_pt)->Hidden);

  return (EdgePoint);
}
