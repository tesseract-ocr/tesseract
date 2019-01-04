/* -*-C-*-
 ********************************************************************************
 *
 * File:        findseam.h  (Formerly findseam.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Thu May 16 17:05:17 1991 (Mark Seaman) marks@hpgrlt
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

#ifndef FINDSEAM_H
#define FINDSEAM_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "seam.h"
#include "genericheap.h"
#include "kdpair.h"
#include "chop.h"

// The SeamPair elements own their SEAMs and delete them upon destruction.
using SeamPair = tesseract::KDPtrPairInc<float, SEAM>;
using SeamQueue = tesseract::GenericHeap<SeamPair>;

using SeamDecPair = tesseract::KDPtrPairDec<float, SEAM>;
using SeamPile = tesseract::GenericHeap<SeamDecPair>;

#endif
