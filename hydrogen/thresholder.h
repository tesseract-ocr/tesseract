/*
 * Copyright 2010, Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HYDROGEN_THRESHOLDER_H_
#define HYDROGEN_THRESHOLDER_H_

#include "leptonica.h"

l_int32 pixGetFisherThresh(PIX *pixs, l_float32 scorefract, l_float32 *pfdr, l_int32 *pthresh);

l_int32 pixFisherAdaptiveThreshold(PIX *pixs, PIX **ppixd, l_int32 tile_x, l_int32 tile_y,
                                l_float32 score_fract, l_float32 thresh);

PIX *pixThreshedSobelEdgeFilter(PIX *pixs, l_int32 threshold);

l_uint8 pixGradientEnergy(PIX *pixs, PIX *mask, l_float32 *pdensity);

l_uint8 pixEdgeMax(PIX *pixs, l_int32 *pmax, l_int32 *pavg);

l_uint8 pixEdgeAdaptiveThreshold(PIX *pixs, PIX **ppixd, l_int32 tile_x, l_int32 tile_y,
                                 l_int32 thresh, l_int32 avg_thresh);

#endif /* HYDROGEN_THRESHOLDER_H_ */
