/**********************************************************************
 * File:        edgloop.h  (Formerly edgeloop.h)
 * Description: Functions to clean up an outline before approximation.
 * Author:      Ray Smith
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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
 **********************************************************************/

#ifndef EDGLOOP_H
#define EDGLOOP_H

#include "coutln.h"
#include "crakedge.h"
#include "params.h"
#include "pdblock.h"
#include "scrollview.h"

namespace tesseract {

#define BUCKETSIZE 16

void complete_edge(CRACKEDGE *start, // start of loop
                   C_OUTLINE_IT *outline_it);
ScrollView::Color check_path_legal( // certify outline
    CRACKEDGE *start                // start of loop
);
int16_t loop_bounding_box( // get bounding box
    CRACKEDGE *&start,     // edge loop
    ICOORD &botleft,       // bounding box
    ICOORD &topright);

} // namespace tesseract

#endif
