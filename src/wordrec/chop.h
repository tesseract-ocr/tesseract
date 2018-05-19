/* -*-C-*-
 ********************************************************************************
 *
 * File:        chop.h  (Formerly chop.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Jul 10 14:47:37 1991 (Mark Seaman) marks@hpgrlt
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
 *******************************************************************************/

#ifndef CHOP_H
#define CHOP_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "genericheap.h"
#include "kdpair.h"
#include "seam.h"

/*----------------------------------------------------------------------
              T y p e s
---------------------------------------------------------------------*/
#define MAX_NUM_POINTS 50
// The PointPair elements do NOT own the EDGEPTs.
using PointPair = tesseract::KDPairInc<float, EDGEPT*>;
using PointHeap = tesseract::GenericHeap<PointPair>;

#endif
