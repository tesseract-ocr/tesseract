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

#define MIN_INERTIA (0.00001)

/*----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
// Convert a blob into a list of MFOUTLINEs (float-based microfeature format).
LIST ConvertBlob(TBLOB *blob) {
  LIST outlines = NIL_LIST;
  return (blob == NULL)
      ? NIL_LIST
      : ConvertOutlines(blob->outlines, outlines, outer);
}


/*---------------------------------------------------------------------------*/
// Convert a TESSLINE into the float-based MFOUTLINE micro-feature format.
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
// Convert a tree of outlines to a list of MFOUTLINEs (lists of MFEDGEPTs).
//
// Parameters:
//   outline      first outline to be converted
//   mf_outlines  list to add converted outlines to
//   outline_type  are the outlines outer or holes?
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
void ComputeOutlineStats(LIST Outlines, OUTLINE_STATS *OutlineStats) {
/*
 ** Parameters:
 **   Outlines  list of outlines to compute stats for
 **   OutlineStats  place to put results
 ** Globals: none
 ** Operation: This routine computes several statistics about the outlines
 **   in Outlines.  These statistics are usually used to perform
 **   anistropic normalization of all of the outlines.  The
 **   statistics generated are:
 **     first moments about x and y axes
 **     total length of all outlines
 **     center of mass of all outlines
 **     second moments about center of mass axes
 **     radius of gyration about center of mass axes
 ** Return: none (results are returned in OutlineStats)
 ** Exceptions: none
 ** History: Fri Dec 14 08:32:03 1990, DSJ, Created.
 */
  MFOUTLINE Outline;
  MFOUTLINE EdgePoint;
  MFEDGEPT *Current;
  MFEDGEPT *Last;

  InitOutlineStats(OutlineStats);
  iterate(Outlines) {
    Outline = (MFOUTLINE) first_node (Outlines);

    Last = PointAt (Outline);
    Outline = NextPointAfter (Outline);
    EdgePoint = Outline;
    do {
      Current = PointAt (EdgePoint);

      UpdateOutlineStats (OutlineStats,
        Last->Point.x, Last->Point.y,
        Current->Point.x, Current->Point.y);

      Last = Current;
      EdgePoint = NextPointAfter (EdgePoint);
    }
    while (EdgePoint != Outline);
  }
  FinishOutlineStats(OutlineStats);

}                                /* ComputeOutlineStats */


/*---------------------------------------------------------------------------*/
void FindDirectionChanges(MFOUTLINE Outline,
                          FLOAT32 MinSlope,
                          FLOAT32 MaxSlope) {
/*
 ** Parameters:
 **   Outline   micro-feature outline to analyze
 **   MinSlope  controls "snapping" of segments to horizontal
 **   MaxSlope  controls "snapping" of segments to vertical
 ** Globals: none
 ** Operation:
 **   This routine searches thru the specified outline, computes
 **   a slope for each vector in the outline, and marks each
 **   vector as having one of the following directions:
 **     N, S, E, W, NE, NW, SE, SW
 **   This information is then stored in the outline and the
 **   outline is returned.
 ** Return: none
 ** Exceptions: none
 ** History: 7/21/89, DSJ, Created.
 */
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
void FreeMFOutline(void *arg) {  //MFOUTLINE                             Outline)
/*
 ** Parameters:
 **   Outline   micro-feature outline to be freed
 ** Globals: none
 ** Operation:
 **   This routine deallocates all of the memory consumed by
 **   a micro-feature outline.
 ** Return: none
 ** Exceptions: none
 ** History: 7/27/89, DSJ, Created.
 */
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
void FreeOutlines(LIST Outlines) {
/*
 ** Parameters:
 **   Outlines  list of mf-outlines to be freed
 ** Globals: none
 ** Operation: Release all memory consumed by the specified list
 **   of outlines.
 ** Return: none
 ** Exceptions: none
 ** History: Thu Dec 13 16:14:50 1990, DSJ, Created.
 */
  destroy_nodes(Outlines, FreeMFOutline);
}                                /* FreeOutlines */


/*---------------------------------------------------------------------------*/
void MarkDirectionChanges(MFOUTLINE Outline) {
/*
 ** Parameters:
 **   Outline   micro-feature outline to analyze
 ** Globals: none
 ** Operation:
 **   This routine searches thru the specified outline and finds
 **   the points at which the outline changes direction.  These
 **   points are then marked as "extremities".  This routine is
 **   used as an alternative to FindExtremities().  It forces the
 **   endpoints of the microfeatures to be at the direction
 **   changes rather than at the midpoint between direction
 **   changes.
 ** Return: none
 ** Exceptions: none
 ** History: 6/29/90, DSJ, Created.
 */
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
// Return a new edge point for a micro-feature outline.
MFEDGEPT *NewEdgePoint() {
  return ((MFEDGEPT *) alloc_struct(sizeof(MFEDGEPT), "MFEDGEPT"));
}


/*---------------------------------------------------------------------------*/
MFOUTLINE NextExtremity(MFOUTLINE EdgePoint) {
/*
 ** Parameters:
 **   EdgePoint start search from this point
 ** Globals: none
 ** Operation:
 **   This routine returns the next point in the micro-feature
 **   outline that is an extremity.  The search starts after
 **   EdgePoint.  The routine assumes that the outline being
 **   searched is not a degenerate outline (i.e. it must have
 **   2 or more edge points).
 ** Return: Next extremity in the outline after EdgePoint.
 ** Exceptions: none
 ** History: 7/26/89, DSJ, Created.
 */
  EdgePoint = NextPointAfter(EdgePoint);
  while (!PointAt(EdgePoint)->ExtremityMark)
    EdgePoint = NextPointAfter(EdgePoint);

  return (EdgePoint);

}                                /* NextExtremity */


/*---------------------------------------------------------------------------*/
void NormalizeOutline(MFOUTLINE Outline,
                      FLOAT32 XOrigin) {
/*
 ** Parameters:
 **   Outline   outline to be normalized
 **   XOrigin   x-origin of text
 ** Globals: none
 ** Operation:
 **   This routine normalizes the coordinates of the specified
 **   outline so that the outline is deskewed down to the
 **   baseline, translated so that x=0 is at XOrigin, and scaled
 **   so that the height of a character cell from descender to
 **   ascender is 1.  Of this height, 0.25 is for the descender,
 **   0.25 for the ascender, and 0.5 for the x-height.  The
 **   y coordinate of the baseline is 0.
 ** Return: none
 ** Exceptions: none
 ** History: 8/2/89, DSJ, Created.
 */
  if (Outline == NIL_LIST)
    return;

  MFOUTLINE EdgePoint = Outline;
  do {
    MFEDGEPT *Current = PointAt(EdgePoint);
    Current->Point.y = MF_SCALE_FACTOR * (Current->Point.y - BASELINE_OFFSET);
    Current->Point.x = MF_SCALE_FACTOR * (Current->Point.x - XOrigin);
    EdgePoint = NextPointAfter(EdgePoint);
  } while (EdgePoint != Outline);
}                                /* NormalizeOutline */


/*---------------------------------------------------------------------------*/
namespace tesseract {
void Classify::NormalizeOutlines(LIST Outlines,
                                 FLOAT32 *XScale,
                                 FLOAT32 *YScale) {
/*
 ** Parameters:
 **   Outlines  list of outlines to be normalized
 **   XScale    x-direction scale factor used by routine
 **   YScale    y-direction scale factor used by routine
 ** Globals:
 **   classify_norm_method  method being used for normalization
 **   classify_char_norm_range map radius of gyration to this value
 ** Operation: This routine normalizes every outline in Outlines
 **   according to the currently selected normalization method.
 **   It also returns the scale factors that it used to do this
 **   scaling.  The scale factors returned represent the x and
 **   y sizes in the normalized coordinate system that correspond
 **   to 1 pixel in the original coordinate system.
 ** Return: none (Outlines are changed and XScale and YScale are updated)
 ** Exceptions: none
 ** History: Fri Dec 14 08:14:55 1990, DSJ, Created.
 */
  MFOUTLINE Outline;
  OUTLINE_STATS OutlineStats;
  FLOAT32 BaselineScale;

  switch (classify_norm_method) {
    case character:
      ComputeOutlineStats(Outlines, &OutlineStats);

      /* limit scale factor to avoid overscaling small blobs (.,`'),
         thin blobs (l1ift), and merged blobs */
      *XScale = *YScale = BaselineScale = MF_SCALE_FACTOR;
      *XScale *= OutlineStats.Ry;
      *YScale *= OutlineStats.Rx;
      if (*XScale < classify_min_norm_scale_x)
        *XScale = classify_min_norm_scale_x;
      if (*YScale < classify_min_norm_scale_y)
        *YScale = classify_min_norm_scale_y;
      if (*XScale > classify_max_norm_scale_x &&
          *YScale <= classify_max_norm_scale_y)
        *XScale = classify_max_norm_scale_x;
      *XScale = classify_char_norm_range * BaselineScale / *XScale;
      *YScale = classify_char_norm_range * BaselineScale / *YScale;

      iterate(Outlines) {
        Outline = (MFOUTLINE) first_node (Outlines);
        CharNormalizeOutline (Outline,
          OutlineStats.x, OutlineStats.y,
          *XScale, *YScale);
      }
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

/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void ChangeDirection(MFOUTLINE Start, MFOUTLINE End, DIRECTION Direction) {
/*
 ** Parameters:
 **   Start, End  defines segment of outline to be modified
 **   Direction new direction to assign to segment
 ** Globals: none
 ** Operation: Change the direction of every vector in the specified
 **   outline segment to Direction.  The segment to be changed
 **   starts at Start and ends at End.  Note that the previous
 **   direction of End must also be changed to reflect the
 **   change in direction of the point before it.
 ** Return: none
 ** Exceptions: none
 ** History: Fri May  4 10:42:04 1990, DSJ, Created.
 */
  MFOUTLINE Current;

  for (Current = Start; Current != End; Current = NextPointAfter (Current))
    PointAt (Current)->Direction = Direction;

  PointAt (End)->PreviousDirection = Direction;

}                                /* ChangeDirection */


/*---------------------------------------------------------------------------*/
void CharNormalizeOutline(MFOUTLINE Outline,
                          FLOAT32 XCenter,
                          FLOAT32 YCenter,
                          FLOAT32 XScale,
                          FLOAT32 YScale) {
/*
 ** Parameters:
 **   Outline     outline to be character normalized
 **   XCenter, YCenter  center point for normalization
 **   XScale, YScale    scale factors for normalization
 ** Globals: none
 ** Operation: This routine normalizes each point in Outline by
 **   translating it to the specified center and scaling it
 **   anisotropically according to the given scale factors.
 ** Return: none
 ** Exceptions: none
 ** History: Fri Dec 14 10:27:11 1990, DSJ, Created.
 */
  MFOUTLINE First, Current;
  MFEDGEPT *CurrentPoint;

  if (Outline == NIL_LIST)
    return;

  First = Outline;
  Current = First;
  do {
    CurrentPoint = PointAt (Current);
    CurrentPoint->Point.x =
      (CurrentPoint->Point.x - XCenter) * XScale;
    CurrentPoint->Point.y =
      (CurrentPoint->Point.y - YCenter) * YScale;

    Current = NextPointAfter (Current);
  }
  while (Current != First);

}                                /* CharNormalizeOutline */


/*---------------------------------------------------------------------------*/
void ComputeDirection(MFEDGEPT *Start,
                      MFEDGEPT *Finish,
                      FLOAT32 MinSlope,
                      FLOAT32 MaxSlope) {
/*
 ** Parameters:
 **   Start   starting point to compute direction from
 **   Finish    finishing point to compute direction to
 **   MinSlope  slope below which lines are horizontal
 **   MaxSlope  slope above which lines are vertical
 ** Globals: none
 ** Operation:
 **   This routine computes the slope from Start to Finish and
 **   and then computes the approximate direction of the line
 **   segment from Start to Finish.  The direction is quantized
 **   into 8 buckets:
 **     N, S, E, W, NE, NW, SE, SW
 **   Both the slope and the direction are then stored into
 **   the appropriate fields of the Start edge point.  The
 **   direction is also stored into the PreviousDirection field
 **   of the Finish edge point.
 ** Return: none
 ** Exceptions: none
 ** History: 7/25/89, DSJ, Created.
 */
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
}                                /* ComputeDirection */


/*---------------------------------------------------------------------------*/
void FinishOutlineStats(register OUTLINE_STATS *OutlineStats) {
/*
 ** Parameters:
 **   OutlineStats  statistics about a set of outlines
 ** Globals: none
 ** Operation: Use the preliminary statistics accumulated in OutlineStats
 **   to compute the final statistics.
 **   (see Dan Johnson's Tesseract lab
 **   notebook #2, pgs. 74-78).
 ** Return: none
 ** Exceptions: none
 ** History: Fri Dec 14 10:13:36 1990, DSJ, Created.
 */
  OutlineStats->x = 0.5 * OutlineStats->My / OutlineStats->L;
  OutlineStats->y = 0.5 * OutlineStats->Mx / OutlineStats->L;

  OutlineStats->Ix = (OutlineStats->Ix / 3.0 -
    OutlineStats->y * OutlineStats->Mx +
    OutlineStats->y * OutlineStats->y * OutlineStats->L);

  OutlineStats->Iy = (OutlineStats->Iy / 3.0 -
    OutlineStats->x * OutlineStats->My +
    OutlineStats->x * OutlineStats->x * OutlineStats->L);

  /* Ix and/or Iy could possibly be negative due to roundoff error */
  if (OutlineStats->Ix < 0.0)
    OutlineStats->Ix = MIN_INERTIA;
  if (OutlineStats->Iy < 0.0)
    OutlineStats->Iy = MIN_INERTIA;

  OutlineStats->Rx = sqrt (OutlineStats->Ix / OutlineStats->L);
  OutlineStats->Ry = sqrt (OutlineStats->Iy / OutlineStats->L);

  OutlineStats->Mx *= 0.5;
  OutlineStats->My *= 0.5;

}                                /* FinishOutlineStats */


/*---------------------------------------------------------------------------*/
void InitOutlineStats(OUTLINE_STATS *OutlineStats) {
/*
 ** Parameters:
 **   OutlineStats  stats data structure to be initialized
 ** Globals: none
 ** Operation: Initialize the outline statistics data structure so
 **   that it is ready to start accumulating statistics.
 ** Return: none
 ** Exceptions: none
 ** History: Fri Dec 14 08:55:22 1990, DSJ, Created.
 */
  OutlineStats->Mx = 0.0;
  OutlineStats->My = 0.0;
  OutlineStats->L = 0.0;
  OutlineStats->x = 0.0;
  OutlineStats->y = 0.0;
  OutlineStats->Ix = 0.0;
  OutlineStats->Iy = 0.0;
  OutlineStats->Rx = 0.0;
  OutlineStats->Ry = 0.0;
}                                /* InitOutlineStats */


/*---------------------------------------------------------------------------*/
MFOUTLINE NextDirectionChange(MFOUTLINE EdgePoint) {
/*
 ** Parameters:
 **   EdgePoint start search from this point
 ** Globals: none
 ** Operation:
 **   This routine returns the next point in the micro-feature
 **   outline that has a direction different than EdgePoint.  The
 **   routine assumes that the outline being searched is not a
 **   degenerate outline (i.e. it must have 2 or more edge points).
 ** Return: Point of next direction change in micro-feature outline.
 ** Exceptions: none
 ** History: 7/25/89, DSJ, Created.
 */
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
}                                /* NextDirectionChange */


/*---------------------------------------------------------------------------*/
void UpdateOutlineStats(register OUTLINE_STATS *OutlineStats,
                        register FLOAT32 x1,
                        register FLOAT32 x2,
                        register FLOAT32 y1,
                        register FLOAT32 y2) {
/*
 ** Parameters:
 **   OutlineStats  statistics to add this segment to
 **   x1, y1, x2, y2  segment to be added to statistics
 ** Globals: none
 ** Operation: This routine adds the statistics for the specified
 **   line segment to OutlineStats.  The statistics that are
 **   kept are:
 **     sum of length of all segments
 **     sum of 2*Mx for all segments
 **     sum of 2*My for all segments
 **     sum of 2*Mx*(y1+y2) - L*y1*y2 for all segments
 **     sum of 2*My*(x1+x2) - L*x1*x2 for all segments
 **   These numbers, once collected can later be used to easily
 **   compute the center of mass, first and second moments,
 **   and radii of gyration.  (see Dan Johnson's Tesseract lab
 **   notebook #2, pgs. 74-78).
 ** Return: none
 ** Exceptions: none
 ** History: Fri Dec 14 08:59:17 1990, DSJ, Created.
 */
  register FLOAT64 L;
  register FLOAT64 Mx2;
  register FLOAT64 My2;

  /* compute length of segment */
  L = sqrt ((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
  OutlineStats->L += L;

  /* compute 2Mx and 2My components */
  Mx2 = L * (y1 + y2);
  My2 = L * (x1 + x2);
  OutlineStats->Mx += Mx2;
  OutlineStats->My += My2;

  /* compute second moment component */
  OutlineStats->Ix += Mx2 * (y1 + y2) - L * y1 * y2;
  OutlineStats->Iy += My2 * (x1 + x2) - L * x1 * x2;

}                                /* UpdateOutlineStats */
