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

#include <cstdlib>
#include "allheaders.h"   // from leptonica
#include <tesseract/genericvector.h>
#include <tesseract/helpers.h>  // For TRand.
#include "rect.h"

namespace tesseract {

// A randomized perspective distortion can be applied to synthetic input.
// The perspective distortion comes from leptonica, which uses 2 sets of 4
// corners to determine the distortion. There are random values for each of
// the x numbers x0..x3 and y0..y3, except for x2 and x3 which are instead
// defined in terms of a single shear value. This reduces the degrees of
// freedom enough to make the distortion more realistic than it would otherwise
// be if all 8 coordinates could move independently.
// One additional factor is used for the color of the pixels that don't exist
// in the source image.
// Name for each of the randomizing factors.
enum FactorNames {
  FN_INCOLOR,
  FN_Y0,
  FN_Y1,
  FN_Y2,
  FN_Y3,
  FN_X0,
  FN_X1,
  FN_SHEAR,
  // x2 = x1 - shear
  // x3 = x0 + shear
  FN_NUM_FACTORS
};

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
// If rotation is nullptr, rotation is skipped. If *rotation is non-zero, the
// pix is rotated by *rotation else it is randomly rotated and *rotation is
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
  if (rotation != nullptr) {
    float radians_clockwise = 0.0f;
    if (*rotation) {
      radians_clockwise = *rotation;
    } else if (randomizer != nullptr) {
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
      if (randomizer != nullptr)
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

// Creates and returns a Pix distorted by various means according to the bool
// flags. If boxes is not nullptr, the boxes are resized/positioned according to
// any spatial distortion and also by the integer reduction factor box_scale
// so they will match what the network will output.
// Returns nullptr on error. The returned Pix must be pixDestroyed.
Pix* PrepareDistortedPix(const Pix* pix, bool perspective, bool invert,
                         bool white_noise, bool smooth_noise, bool blur,
                         int box_reduction, TRand* randomizer,
                         GenericVector<TBOX>* boxes) {
  Pix* distorted = pixCopy(nullptr, const_cast<Pix*>(pix));
  // Things to do to synthetic training data.
  if ((white_noise || smooth_noise) && randomizer->SignedRand(1.0) > 0.0) {
    // TODO(rays) Cook noise in a more thread-safe manner than rand().
    // Attempt to make the sequences reproducible.
    srand(randomizer->IntRand());
    Pix* pixn = pixAddGaussianNoise(distorted, 8.0);
    pixDestroy(&distorted);
    if (smooth_noise) {
      distorted = pixBlockconv(pixn, 1, 1);
      pixDestroy(&pixn);
    } else {
      distorted = pixn;
    }
  }
  if (blur && randomizer->SignedRand(1.0) > 0.0) {
    Pix* blurred = pixBlockconv(distorted, 1, 1);
    pixDestroy(&distorted);
    distorted = blurred;
  }
  if (perspective)
    GeneratePerspectiveDistortion(0, 0, randomizer, &distorted, boxes);
  if (boxes != nullptr) {
    for (int b = 0; b < boxes->size(); ++b) {
      (*boxes)[b].scale(1.0f / box_reduction);
      if ((*boxes)[b].width() <= 0)
        (*boxes)[b].set_right((*boxes)[b].left() + 1);
    }
  }
  if (invert && randomizer->SignedRand(1.0) < -0)
    pixInvert(distorted, distorted);
  return distorted;
}

// Distorts anything that has a non-null pointer with the same pseudo-random
// perspective distortion. Width and height only need to be set if there
// is no pix. If there is a pix, then they will be taken from there.
void GeneratePerspectiveDistortion(int width, int height, TRand* randomizer,
                                   Pix** pix, GenericVector<TBOX>* boxes) {
  if (pix != nullptr && *pix != nullptr) {
    width = pixGetWidth(*pix);
    height = pixGetHeight(*pix);
  }
  float* im_coeffs = nullptr;
  float* box_coeffs = nullptr;
  l_int32 incolor =
      ProjectiveCoeffs(width, height, randomizer, &im_coeffs, &box_coeffs);
  if (pix != nullptr && *pix != nullptr) {
    // Transform the image.
    Pix* transformed = pixProjective(*pix, im_coeffs, incolor);
    if (transformed == nullptr) {
      tprintf("Projective transformation failed!!\n");
      return;
    }
    pixDestroy(pix);
    *pix = transformed;
  }
  if (boxes != nullptr) {
    // Transform the boxes.
    for (int b = 0; b < boxes->size(); ++b) {
      int x1, y1, x2, y2;
      const TBOX& box = (*boxes)[b];
      projectiveXformSampledPt(box_coeffs, box.left(), height - box.top(), &x1,
                               &y1);
      projectiveXformSampledPt(box_coeffs, box.right(), height - box.bottom(),
                               &x2, &y2);
      TBOX new_box1(x1, height - y2, x2, height - y1);
      projectiveXformSampledPt(box_coeffs, box.left(), height - box.bottom(),
                               &x1, &y1);
      projectiveXformSampledPt(box_coeffs, box.right(), height - box.top(), &x2,
                               &y2);
      TBOX new_box2(x1, height - y1, x2, height - y2);
      (*boxes)[b] = new_box1.bounding_union(new_box2);
    }
  }
  free(im_coeffs);
  free(box_coeffs);
}

// Computes the coefficients of a randomized projective transformation.
// The image transform requires backward transformation coefficient, and the
// box transform the forward coefficients.
// Returns the incolor arg to pixProjective.
int ProjectiveCoeffs(int width, int height, TRand* randomizer,
                     float** im_coeffs, float** box_coeffs) {
  // Setup "from" points.
  Pta* src_pts = ptaCreate(4);
  ptaAddPt(src_pts, 0.0f, 0.0f);
  ptaAddPt(src_pts, width, 0.0f);
  ptaAddPt(src_pts, width, height);
  ptaAddPt(src_pts, 0.0f, height);
  // Extract factors from pseudo-random sequence.
  float factors[FN_NUM_FACTORS];
  float shear = 0.0f;  // Shear is signed.
  for (int i = 0; i < FN_NUM_FACTORS; ++i) {
    // Everything is squared to make wild values rarer.
    if (i == FN_SHEAR) {
      // Shear is signed.
      shear = randomizer->SignedRand(0.5 / 3.0);
      shear = shear >= 0.0 ? shear * shear : -shear * shear;
      // Keep the sheared points within the original rectangle.
      if (shear < -factors[FN_X0]) shear = -factors[FN_X0];
      if (shear > factors[FN_X1]) shear = factors[FN_X1];
      factors[i] = shear;
    } else if (i != FN_INCOLOR) {
      factors[i] = fabs(randomizer->SignedRand(1.0));
      if (i <= FN_Y3)
        factors[i] *= 5.0 / 8.0;
      else
        factors[i] *= 0.5;
      factors[i] *= factors[i];
    }
  }
  // Setup "to" points.
  Pta* dest_pts = ptaCreate(4);
  ptaAddPt(dest_pts, factors[FN_X0] * width, factors[FN_Y0] * height);
  ptaAddPt(dest_pts, (1.0f - factors[FN_X1]) * width, factors[FN_Y1] * height);
  ptaAddPt(dest_pts, (1.0f - factors[FN_X1] + shear) * width,
           (1 - factors[FN_Y2]) * height);
  ptaAddPt(dest_pts, (factors[FN_X0] + shear) * width,
           (1 - factors[FN_Y3]) * height);
  getProjectiveXformCoeffs(dest_pts, src_pts, im_coeffs);
  getProjectiveXformCoeffs(src_pts, dest_pts, box_coeffs);
  ptaDestroy(&src_pts);
  ptaDestroy(&dest_pts);
  return factors[FN_INCOLOR] > 0.5f ? L_BRING_IN_WHITE : L_BRING_IN_BLACK;
}

}  // namespace tesseract
