/******************************************************************************
 ** Filename:    normmatch.h
 ** Purpose:     Simple matcher based on character normalization features.
 ** Author:      Dan Johnson
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

#ifndef NORMMATCH_H
#define NORMMATCH_H

#include "matchdefs.h"
#include "ocrfeatures.h"
#include "params.h"

namespace tesseract {

/* control knobs used to control the normalization adjustment process */
extern double_VAR_H(classify_norm_adj_midpoint);
extern double_VAR_H(classify_norm_adj_curl);

} // namespace tesseract

#endif
