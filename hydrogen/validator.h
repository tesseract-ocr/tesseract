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

#ifndef HYDROGEN_VALIDATOR_H_
#define HYDROGEN_VALIDATOR_H_

#include "leptonica.h"
#include "hydrogentextdetector.h"

bool ValidatePairOld(BOX *b1, BOX *b2);

bool ValidateSingleton(PIX *pix, BOX *box, PIX *pix8, l_float32 *pconf,
                       HydrogenTextDetector::TextDetectorParameters &params);

bool ValidatePair(BOX *b1, BOX *b2, l_float32 *pconf,
                  HydrogenTextDetector::TextDetectorParameters &params);

bool ValidateClusterPair(BOX *b1, BOX *b2, bool *too_far, l_float32 *pconf,
                         HydrogenTextDetector::TextDetectorParameters &params);

bool ValidateCluster(PIX *pix8, PIXA *pixa, BOX *box, l_float32 *pconf,
                     HydrogenTextDetector::TextDetectorParameters &params);

#endif /* HYDROGEN_VALIDATOR_H_ */
