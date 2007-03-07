/* -*-C-*-
 ********************************************************************************
 *
 * File:        vecfuncs.h  (Formerly vecfuncs.h)
 * Description:  Vector calculations
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed Dec 20 09:37:18 1989
 * Modified:     Tue Jul  9 17:44:37 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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
#ifndef VECFUNCS_H
#define VECFUNCS_H

#include "tessclas.h"
#include <math.h>

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * point_diff
 *
 * Return the difference from point (p1) to point (p2).  Put the value
 * into point (p).
 **********************************************************************/

#define point_diff(p,p1,p2)  \
((p).x = (p1).x - (p2).x,        \
	(p).y = (p1).y - (p2).y,        \
	(p))

/**********************************************************************
 * CROSS
 *
 * cross product
 **********************************************************************/

#define CROSS(a,b) \
((a).x * (b).y - (a).y * (b).x)

/**********************************************************************
 * SCALAR
 *
 * scalar vector product
 **********************************************************************/

#define SCALAR(a,b) \
((a).x * (b).x + (a).y * (b).y)

/**********************************************************************
 * LENGTH
 *
 * length of vector
 **********************************************************************/

#define LENGTH(a) \
((a).x * (a).x + (a).y * (a).y)

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
int direction(EDGEPT *point); 

/*
#if defined(__STDC__) || defined(__cplusplus) || MAC_OR_DOS
# define	_ARGS(s) s
#else
# define	_ARGS(s) ()
#endif*/

/* vecfuncs.c
int direction
  _ARGS((EDGEPT *point));

#undef _ARGS
*/
#endif
