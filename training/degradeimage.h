/**********************************************************************
 * File:        degradeimage.h
 * Description: Function to degrade an image (usually of text) as if it
 *              has been printed and then scanned.
 * Authors:     Ray Smith
 * Created:     Tue Nov 19 2013
 *
 * (C) Copyright 2013, Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **********************************************************************/
#ifndef TESSERACT_TRAINING_DEGRADEIMAGE_H_
#define TESSERACT_TRAINING_DEGRADEIMAGE_H_

#include "allheaders.h"
#include "genericvector.h"
#include "helpers.h"  // For TRand.
#include "rect.h"

namespace tesseract {

// Degrade the pix as if by a print/copy/scan cycle with exposure > 0
// corresponding to darkening on the copier and <0 lighter and 0 not copied.
// If rotation is not NULL, the clockwise rotation in radians is saved there.
// The input pix must be 8 bit grey. (Binary with values 0 and 255 is OK.)
// The input image is destroyed and a different image returned.
struct Pix* DegradeImage(struct Pix* input, int exposure, TRand* randomizer,
                         float* rotation);

// Creates and returns a Pix distorted by various means according to the bool
// flags. If boxes is not NULL, the boxes are resized/positioned according to
// any spatial distortion and also by the integer reduction factor box_scale
// so they will match what the network will output.
// Returns NULL on error. The returned Pix must be pixDestroyed.
Pix* PrepareDistortedPix(const Pix* pix, bool perspective, bool invert,
                         bool white_noise, bool smooth_noise, bool blur,
                         int box_reduction, TRand* randomizer,
                         GenericVector<TBOX>* boxes);
// Distorts anything that has a non-null pointer with the same pseudo-random
// perspective distortion. Width and height only need to be set if there
// is no pix. If there is a pix, then they will be taken from there.
void GeneratePerspectiveDistortion(int width, int height, TRand* randomizer,
                                   Pix** pix, GenericVector<TBOX>* boxes);
// Computes the coefficients of a randomized projective transformation.
// The image transform requires backward transformation coefficient, and the
// box transform the forward coefficients.
// Returns the incolor arg to pixProjective.
int ProjectiveCoeffs(int width, int height, TRand* randomizer,
                     float** im_coeffs, float** box_coeffs);

}  // namespace tesseract

#endif  // TESSERACT_TRAINING_DEGRADEIMAGE_H_
