/*
 * Copyright 2011, Google Inc.
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

#include <math.h>

#include "leptonica.h"
#include "validator.h"
#include "thresholder.h"
#include "utilities.h"
#include "hydrogentextdetector.h"

l_int32 BBoxHDist(BOX *b1, BOX *b2) {
  return L_MAX(b1->x, b2->x) - L_MIN(b1->x + b1->w, b2->x + b2->w);
}

l_int32 BBoxVDist(BOX *b1, BOX *b2) {
  return L_MAX(b1->y, b2->y) - L_MIN(b1->y + b1->h, b2->y + b2->h);
}

l_float32 RelativeDiff(l_int32 v1, l_int32 v2) {
  return L_ABS(v1 - v2) / (L_MIN(v1, v2) + 1.0);
}

#define OLDPAIR_MIN_HPAIR_RATIO 0.5
#define OLDPAIR_MIN_WPAIR_RATIO 0.1
#define OLDPAIR_MAX_HDIST_RATIO 3.0
#define OLDPAIR_MAX_VDIST_RATIO 0.5

/**
 * Test whether b1 and b2 are close enough to be a character pair.
 */
bool ValidatePairOld(BOX *b1, BOX *b2) {
  l_int32 max_w = L_MAX(b1->w, b2->w);
  l_int32 centerx1 = b1->x + b1->w / 2;
  l_int32 centerx2 = b2->x + b2->w / 2;
  l_int32 h_dist = L_ABS(centerx1 - centerx2);

  /* Horizontal distance between centers is
   * less than twice the wider character */
  if (h_dist > max_w * OLDPAIR_MAX_HDIST_RATIO)
    return false;

  l_int32 max_h = L_MAX(b1->h, b2->h);
  l_int32 centery1 = b1->y + b1->h / 2;
  l_int32 centery2 = b2->y + b2->h / 2;
  l_int32 v_dist = L_ABS(centery1 - centery2);

  /* Vertical distance between centers is
   less than 50% of the taller character */
  if (v_dist > max_h * OLDPAIR_MAX_VDIST_RATIO)
    return false;

  l_int32 min_h = L_MIN(b1->h, b2->h);
  l_float32 h_ratio = min_h / (max_h + 1.0);

  /* Height ratio is between 0.5 and 2 */
  if (h_ratio < OLDPAIR_MIN_HPAIR_RATIO)
    return false;

  l_int32 min_w = L_MIN(b1->w, b2->w);
  l_float32 w_ratio = min_w / (max_w + 1.0);

  /* Width ratio is between 0.1 and 10 */
  if (w_ratio < OLDPAIR_MIN_WPAIR_RATIO)
    return false;

  return true;
}

l_float32 ComputeFDR(PIX *cc8) {
  l_float32 fdr;

  pixGetFisherThresh(cc8, 0.0, &fdr, NULL);

  return fdr;
}

l_float32 ComputeGradientEnergy(PIX *cc8, PIX *cc) {
  l_float32 energy;

  pixGradientEnergy(cc8, cc, &energy);

  return energy;
}

l_float32 ComputeCCDensity(PIX *pix) {
  l_int32 area = pix->w * pix->h;
  l_int32 pixel_count;

  pixCountPixels(pix, &pixel_count, NULL);

  return pixel_count / (l_float32) area;
}

l_float32 ComputeCCEdgeMax(PIX *pix8) {
  l_int32 max;
  l_int32 avg;

  pixEdgeMax(pix8, &max, &avg);

  return (l_float32) max;
}

l_float32 ComputeSingletonConfidence(PIX *pix, BOX *box, PIX *pix8) {
  l_float32 aspect_ratio = box->w / (l_float32) box->h;
  l_float32 density = ComputeCCDensity(pix);
  l_float32 gradient = ComputeGradientEnergy(pix8, pix);
  l_float32 edgemax = ComputeCCEdgeMax(pix8);

  /* Compute features for confidence */
  l_float32 features[7];
  features[0] = 1.0;
  features[1] = aspect_ratio;
  features[2] = aspect_ratio * aspect_ratio;
  features[3] = gradient;
  features[4] = aspect_ratio / density;
  features[5] = edgemax;

  l_float32 beta[5];
  beta[0] = -3.099;
  beta[1] = 1.244;
  beta[2] = -0.1142;
  beta[3] = 39.86;
  beta[4] = -0.4005;
  beta[5] = 0;

  l_float32 confidence = 0.0;
  for (int i = 0; i < 6; i++) {
    confidence += features[i] * beta[i];
  }

  return confidence;
}

bool ValidateSingleton(PIX *pix, BOX *box, PIX *pix8, l_float32 *pconf,
                       HydrogenTextDetector::TextDetectorParameters &params) {
  l_float32 aspect_ratio = box->w / (l_float32) box->h;
  l_float32 density = ComputeCCDensity(pix);

  *pconf = 0.0;

  /* Aspect ratio */
  if (aspect_ratio > params.single_max_aspect)
    return false;

  if (aspect_ratio < params.single_min_aspect)
    return false;

  /* Pixel density */
  if (density < params.single_min_density)
    return false;

  l_int32 area = box->w * box->h;

  /* Area */
  if (area < params.single_min_area)
    return false;

  *pconf = 1.0; //ComputeSingletonConfidence(pix, box, pix8);

  return true;
}

/**
 * Test whether b1 and b2 are close enough to be a character pair.
 */
bool ValidatePair(BOX *b1, BOX *b2, l_float32 *pconf,
                  HydrogenTextDetector::TextDetectorParameters &params) {
  *pconf = 0.0;

  l_int32 max_h = L_MAX(b1->h, b2->h);
  l_int32 h_dist = BBoxHDist(b1, b2);
  l_int32 v_dist = BBoxVDist(b1, b2);
  l_float32 h_ratio = RelativeDiff(b1->h, b2->h);
  l_int32 d1 = L_MAX(b1->h, b1->w);
  l_int32 d2 = L_MAX(b2->h, b2->w);
  l_float32 d_ratio = RelativeDiff(d1, d2);

  /* Horizontal spacing less than 2x taller edge */
  if (h_dist > params.pair_h_dist_ratio * max_h)
    return false;

  /* Must share at least 0.25x the larger vertical edge */
  if (v_dist > 0 || L_ABS(v_dist) < max_h * params.pair_h_shared)
    return false;

  /* Heights must be at least 2x tolerance */
  if (h_ratio > params.pair_h_ratio)
    return false;

  /* Maximum dimensions must be within 3x tolerance */
  if (d_ratio > params.pair_d_ratio)
    return false;

  // TODO(alanv): Does this need to return a confidence value?
  *pconf = 1.0;

  return true;
}

l_float32 ComputePairNormalizedOverlapArea(BOX *b1, BOX *b2) {
  BOX *overlap = boxOverlapRegion(b1, b2);

  if (!overlap || overlap->w == 0.0 || overlap->h == 0.0) return 0.0;

  l_float32 area0 = overlap->w * overlap->h;
  l_float32 area1 = b1->w * b1->h;
  l_float32 area2 = b2->w * b2->h;
  l_float32 oarea = 2.0 * area0 / (area1 + area2);

  return oarea;
}

l_float32 ComputePairNormalizedBaselineDistance(BOX *b1, BOX *b2) {
  l_float32 dy = (b1->y + b1->h) - (b2->y  + b2->h);
  l_float32 vdist = 2.0 * L_ABS(dy) / (b1->h + b2->h);

  return vdist;
}

l_float32 ComputePairNormalizedToplineDistance(BOX *b1, BOX *b2) {
  l_float32 dy = b1->y - b2->y;
  l_float32 vdist = 2.0 * L_ABS(dy) / (b1->h + b2->h);

  return vdist;
}

l_float32 ComputePairNormalizedHorizontalDistance(BOX *b1, BOX *b2) {
  l_float32 dx = (b1->x - b2->x) + (b1->w - b2->w) / 2.0;
  l_float32 hdist = 2.0 * L_ABS(dx) / (b1->w + b2->w);

  return hdist;
}

l_float32 ComputePairAreaRatio(BOX *b1, BOX *b2) {
  l_float32 area1 = b1->w * b1->h;
  l_float32 area2 = b2->w * b2->h;
  l_float32 ratio = L_MIN(area1, area2) / L_MAX(area1, area2);

  return ratio;
}

l_float32 ComputePairWidthRatio(BOX *b1, BOX *b2) {
  l_float32 ratio = L_MIN(b1->w, b2->w) / L_MAX(b1->w, b2->w);

  return ratio;
}

l_float32 ComputePairHeightRatio(BOX *b1, BOX *b2) {
  l_float32 ratio = L_MIN(b1->h, b2->h) / L_MAX(b1->h, b2->h);

  return ratio;
}

l_float32 ComputePairContainmentCheck(BOX *b1, BOX *b2) {
  l_int32 contains1, contains2;

  boxContains(b1, b2, &contains1);
  boxContains(b2, b1, &contains2);

  l_float32 contains = (l_float32) (contains1 || contains2);

  return contains;
}

l_float32 ComputePairConfidence(BOX *b1, BOX *b2) {
  l_float32 features[9];
  features[0] = 1.0;
  features[1] = ComputePairNormalizedOverlapArea(b1, b2);
  features[2] = ComputePairNormalizedBaselineDistance(b1, b2);
  features[3] = ComputePairNormalizedToplineDistance(b1, b2);
  features[4] = ComputePairNormalizedHorizontalDistance(b1, b2);
  features[5] = ComputePairAreaRatio(b1, b2);
  features[6] = ComputePairWidthRatio(b1, b2);
  features[7] = ComputePairHeightRatio(b1, b2);
  features[8] = ComputePairContainmentCheck(b1, b2);

  l_float32 beta[9];
  beta[0] = 3.987;
  beta[1] = -9.681;
  beta[2] = -5.804;
  beta[3] = -4.857;
  beta[4] = -2.906;
  beta[5] = -1.813;
  beta[6] = 3.481;
  beta[7] = 3.983;
  beta[8] = -39.24;

  l_float32 confidence = 0.0;
  for (int i = 0; i < 5; i++) {
    confidence += features[i] * beta[i];
  }

  return confidence;
}

/**
 * Test whether b1 and b2 are close enough to be clustered. More relaxed constraints than ValidatePair().
 */
bool ValidateClusterPair(BOX *b1, BOX *b2, bool *too_far, l_float32 *pconf,
                         HydrogenTextDetector::TextDetectorParameters &params) {
  *pconf = 0.0;

  l_int32 max_d = L_MAX(b1->w, b1->h);
  l_float32 h_ratio = RelativeDiff(b1->h, b2->h);

  // If we're already too far out, quit
  if (b2->x > b1->x + b1->w + params.cluster_width_spacing * max_d) {
    *too_far = true;

    return false;
  }

  *too_far = false;

  // Must share at least 0.25x the larger vertical edge
  //l_int32 v_dist = BBoxVDist(b1, b2);
  //if (v_dist > 0 || L_ABS(v_dist) < L_MIN(min_h, max_h) * PAIR_H_SHARED)
  //  return false;

  // i and j must share at least half an edge
  if (b2->y + b2->h * params.cluster_shared_edge < b1->y)
    return false;
  if (b1->y + b1->h * params.cluster_shared_edge < b2->y)
    return false;

  // Heights must be at least 2x tolerance
  if (h_ratio > params.pair_h_ratio)
    return false;

  *pconf = 1.0; //ComputePairConfidence(b1, b2);

  return true;
}

/**
 * Test whether a finalized cluster is valid.
 */
bool ValidateCluster(PIX *pix8, PIXA *pixa, BOX *box, l_float32 *pconf,
                     HydrogenTextDetector::TextDetectorParameters &params) {
  *pconf = 0.0;

  l_float32 aspect = box->w / (l_float32) box->h;
  l_int32 count = pixaGetCount(pixa);
  l_float32 fdr = ComputeFDR(pix8);

  if (box->h < 15)
    return false;

  if (aspect < params.cluster_min_aspect)
    return false;

  if (count < params.cluster_min_blobs)
    return false;

  if (fdr < params.cluster_min_fdr)
    return false;
/*
  l_int32 edge_max, edge_avg;
  pixEdgeMax(pix8, &edge_max, &edge_avg);

  if (edge_max < params.cluster_min_edge || edge_avg < params.cluster_min_edge_avg)
    return false;
*/
  // TODO(alanv): Combine all of these into a confidence score, higher = better
  *pconf = log(fdr); //log(fdr * edge_max * edge_avg);

  return true;
}
