// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "debugging.h"
#include "mask.h"
#include "perspectivedetection.h"
#include "tesseract.h"

// C includes
#include <stdlib.h>
#include <math.h>

using namespace helium;

const char* kMaskFile = "tmp_mask.tif";
const char* kFlipFile = "tmp_flip.tif";

bool PerspectiveDetection::DetectSurface(const Mask& mask, Surface& surface) {
  int base_offset, top_offset;
  float base_slope, top_slope;
  const float max_slope_diff = 1.0;  // maximum top-base slope difference
  const float max_slope_avg = 0.5;   // maximum average top-base slope

  // Detect baseline
  if (!Tesseract::DetectBaseline(mask, top_offset, top_slope, false))
    return false;

  if (!Tesseract::DetectBaseline(mask, base_offset, base_slope, true))
    return false;

  // Limit the maximum slope differences would prevent extreme cases
  // of fast divergent or convergent baselines, which creates enormous
  // resulting image.
  if (fabs(top_slope - base_slope) > max_slope_diff)
    return false;

  // Limit the maximum average slope to stop attempting to correct
  // nearly vertical texts into horizontal.
  if (fabs((top_slope + base_slope)/2) > max_slope_avg)
    return false;

  base_offset = mask.height() - base_offset;
  base_slope = -base_slope;

  int v_margin = static_cast<int>((base_offset - top_offset) * 0.1);

  surface.set_top_left(Point(0, top_offset - v_margin));
  surface.set_bottom_left(Point(0, base_offset + v_margin));

  float width = mask.width();

  Point top_right(
    mask.width() - 1,
    static_cast<int>(top_offset + top_slope * width) - v_margin);

  Point bottom_right(
    mask.width() - 1,
    static_cast<int>(base_offset + base_slope * width) + v_margin);

  surface.set_top_right(top_right);
  surface.set_bottom_right(bottom_right);

  return true;
}
