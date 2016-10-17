/* -*-C-*-
 ********************************************************************************
 *
 * File:        vecfuncs.c  (Formerly vecfuncs.c)
 * Description:  Blob definition
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 27 15:39:52 1989
 * Modified:     Tue Jul  9 17:44:12 1991 (Mark Seaman) marks@hpgrlt
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
 ********************************************************************************
 * Revision 5.1  89/07/27  11:47:50  11:47:50  ray ()
 * Added ratings access methods.
 * This version ready for independent development.
 */
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "vecfuncs.h"
#include "blobs.h"

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 *  direction
 *
 *  Show if the line is going in the positive or negative X direction.
 **********************************************************************/
int direction(EDGEPT *point) {
  int dir;                       /** direction to return **/
  EDGEPT *prev;                  /** prev point **/
  EDGEPT *next;                  /** next point **/

  dir = 0;
  prev = point->prev;
  next = point->next;

  if (((prev->pos.x <= point->pos.x) &&
    (point->pos.x < next->pos.x)) ||
    ((prev->pos.x < point->pos.x) && (point->pos.x <= next->pos.x)))
    dir = 1;

  if (((prev->pos.x >= point->pos.x) &&
    (point->pos.x > next->pos.x)) ||
    ((prev->pos.x > point->pos.x) && (point->pos.x >= next->pos.x)))
    dir = -1;

  return dir;
}
