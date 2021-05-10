/**********************************************************************
 * File:        tordmain.h  (Formerly textordp.h)
 * Description: C++ top level textord code.
 * Author:      Ray Smith
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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

#ifndef TORDMAIN_H
#define TORDMAIN_H

#include "blobbox.h"
#include "blobs.h"
#include "ocrblock.h"
#include "params.h"

struct Pix;

namespace tesseract {

class Tesseract;

void SetBlobStrokeWidth(Image pix, BLOBNBOX *blob);
void assign_blobs_to_blocks2(Image pix, BLOCK_LIST *blocks, TO_BLOCK_LIST *port_blocks);

void tweak_row_baseline(ROW *row, double blshift_maxshift, double blshift_xfraction);

} // namespace tesseract

#endif
