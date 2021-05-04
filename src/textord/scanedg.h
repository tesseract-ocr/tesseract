/**********************************************************************
 * File:        scanedg.h  (Formerly scanedge.h)
 * Description: Raster scanning crack based edge extractor.
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

#ifndef SCANEDG_H
#define SCANEDG_H

#include "image.h"
#include "params.h"

struct Pix;

namespace tesseract {

class C_OUTLINE_IT;
class PDBLK;

void block_edges(Image t_image, // thresholded image
                 PDBLK *block, // block in image
                 C_OUTLINE_IT *outline_it);

} // namespace tesseract

#endif
