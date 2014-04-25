/**********************************************************************
 * File:        degradeimage.cpp
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

#include "degradeimage.h"

#include <stdlib.h>
#include "allheaders.h"   // from leptonica
#include "helpers.h"  // For TRand.

namespace tesseract {

// Rotation is +/- kRotationRange radians.
const float kRotationRange = 0.02f;
// Number of grey levels to shift by for each exposure step.
const int kExposureFactor = 16;
// Salt and pepper noise is +/- kSaltnPepper.
const int kSaltnPepper = 5;
// Min sum of width + height on which to operate the ramp.
const int kMinRampSize = 1000;

// Degrade the pix as if by a print/copy/scan cycle with exposure > 0
// corresponding to darkening on the copier and <0 lighter and 0 not copied.
// Exposures in [-2,2] are most useful, with -3 and 3 being extreme.
// If rotation is NULL, rotation is skipped. If *rotation is non-zero, the pix
// is rotated by *rotation else it is randomly rotated and *rotation is
// modified.
//
// HOW IT WORKS:
// Most of the process is really dictated by the fact that the minimum
// available convolution is 3X3, which is too big really to simulate a
// good quality print/scan process. (2X2 would be better.)
// 1 pixel wide inputs are heavily smeared by the 3X3 convolution, making the
// images generally biased to being too light, so most of the work is to make
// them darker. 3 levels of thickening/darkening are achieved with 2 dilations,
// (using a greyscale erosion) one heavy (by being before convolution) and one
// light (after convolution).
// With no dilation, after covolution, the images are so light that a heavy
// constant offset is required to make the 0 image look reasonable. A simple
// constant offset multiple of exposure to undo this value is enough to achieve
// all the required lightening. This gives the advantage that exposure level 1
// with a single dilation gives a good impression of the broken-yet-too-dark
// problem that is often seen in scans.
// A small random rotation gives some varying greyscale values on the edges,
// and some random salt and pepper noise on top helps to realistically jaggy-up
// the edges.
// Finally a greyscale ramp provides a continuum of effects between exposure
// levels.
Pix* DegradeImage(Pix* input, int exposure, TRand* randomizer,
                  float* rotation) {
  Pix* pix = pixConvertTo8(input, false);
  pixDestroy(&input);
  input = pix;
  int width = pixGetWidth(input);
  int height = pixGetHeight(input);
  if (exposure >= 2) {
    // An erosion simulates the spreading darkening of a dark copy.
    // This is backwards to binary morphology,
    // see http://www.leptonica.com/grayscale-morphology.html
    pix = input;
    input = pixErodeGray(pix, 3, 3);
    pixDestroy(&pix);
  }
  // A convolution is essential to any mode as no scanner produces an
  // image as sharp as the electronic image.
  pix = pixBlockconv(input, 1, 1);
  pixDestroy(&input);
  // A small random rotation helps to make the edges jaggy in a realistic way.
  if (rotation != NULL) {
    float radians_clockwise = 0.0f;
    if (*rotation) {
      radians_clockwise = *rotation;
    } else if (randomizer != NULL) {
      radians_clockwise = randomizer->SignedRand(kRotationRange);
    }

    input = pixRotate(pix, radians_clockwise,
                      L_ROTATE_AREA_MAP, L_BRING_IN_WHITE,
                      0, 0);
    // Rotate the boxes to match.
    *rotation = radians_clockwise;
    pixDestroy(&pix);
  } else {
    input = pix;
  }

  if (exposure >= 3 || exposure == 1) {
    // Erosion after the convolution is not as heavy as before, so it is
    // good for level 1 and in addition as a level 3.
    // This is backwards to binary morphology,
    // see http://www.leptonica.com/grayscale-morphology.html
    pix = input;
    input = pixErodeGray(pix, 3, 3);
    pixDestroy(&pix);
  }
  // The convolution really needed to be 2x2 to be realistic enough, but
  // we only have 3x3, so we have to bias the image darker or lose thin
  // strokes.
  int erosion_offset = 0;
  // For light and 0 exposure, there is no dilation, so compensate for the
  // convolution with a big darkening bias which is undone for lighter
  // exposures.
  if (exposure <= 0)
    erosion_offset = -3 * kExposureFactor;
  // Add in a general offset of the greyscales for the exposure level so
  // a threshold of 128 gives a reasonable binary result.
  erosion_offset -= exposure * kExposureFactor;
  // Add a gradual fade over the page and a small amount of salt and pepper
  // noise to simulate noise in the sensor/paper fibres and varying
  // illumination.
  l_uint32* data = pixGetData(input);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int pixel = GET_DATA_BYTE(data, x);
      if (randomizer != NULL)
        pixel += randomizer->IntRand() % (kSaltnPepper*2 + 1) - kSaltnPepper;
      if (height + width > kMinRampSize)
        pixel -= (2*x + y) * 32 / (height + width);
      pixel += erosion_offset;
      if (pixel < 0)
        pixel = 0;
      if (pixel > 255)
        pixel = 255;
      SET_DATA_BYTE(data, x, pixel);
    }
    data += input->wpl;
  }
  return input;
}

}  // namespace tesseract
