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

#ifndef HYDROGEN_CLUSTERER_H_
#define HYDROGEN_CLUSTERER_H_

#include "leptonica.h"
#include "hydrogentextdetector.h"

l_int32 ConnCompValidPixa(PIX *pix8, PIX *pix, PIXA **ppixa, NUMA **pconfs,
                          HydrogenTextDetector::TextDetectorParameters &params);

l_int32 MergePix(PIXA *pixad, l_int32 i, PIXA *pixas, l_int32 j);

l_int32 RemoveInvalidPairs(PIX *pix8, PIXA *pixa, NUMA *confs, l_uint8 *remove,
                           HydrogenTextDetector::TextDetectorParameters &params);

l_int32 ClusterValidComponents(PIX *pix8, PIXA *pixa, NUMA *confs, l_uint8 *remove, PIXA **ppixad, NUMA **pconfs,
                               HydrogenTextDetector::TextDetectorParameters &params);

l_int32 MergePairFragments(PIX *pix8, PIXA *clusters, PIXA *pixa, l_uint8 *remove);

#endif /* HYDROGEN_CLUSTERER_H_ */
