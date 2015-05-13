/**********************************************************************
 * File:        tordmain.h  (Formerly textordp.h)
 * Description: C++ top level textord code.
 * Author:		Ray Smith
 * Created:		Tue Jul 28 17:12:33 BST 1992
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

#ifndef           TORDMAIN_H
#define           TORDMAIN_H

#include          <time.h>
#include          "params.h"
#include          "ocrblock.h"
#include          "blobs.h"
#include          "blobbox.h"

struct Pix;
namespace tesseract {
class Tesseract;

void SetBlobStrokeWidth(Pix* pix, BLOBNBOX* blob);
void assign_blobs_to_blocks2(Pix* pix, BLOCK_LIST *blocks,
                             TO_BLOCK_LIST *port_blocks);
}  // namespace tesseract

void tweak_row_baseline(ROW *row,
                        double blshift_maxshift,
                        double blshift_xfraction);

#endif
