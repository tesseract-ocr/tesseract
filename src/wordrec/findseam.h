/******************************************************************************
 *
 * File:        findseam.h
 * Author:      Mark Seaman, SW Productivity
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
 *****************************************************************************/

#ifndef FINDSEAM_H
#define FINDSEAM_H

#include "chop.h"
#include "genericheap.h"
#include "kdpair.h"
#include "seam.h"

namespace tesseract {

// The SeamPair elements own their SEAMs and delete them upon destruction.
using SeamPair = KDPtrPairInc<float, SEAM>;
using SeamQueue = GenericHeap<SeamPair>;

using SeamDecPair = KDPtrPairDec<float, SEAM>;
using SeamPile = GenericHeap<SeamDecPair>;

} // namespace tesseract

#endif
