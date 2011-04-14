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
#include "thresholder.h"

/*!
 *  pixFisherAdaptiveThreshold()
 *
 *      Input:  pixs (8 bpp)
 *              &pixd (<required return> thresholded input for pixs)
 *              sx, sy (desired tile dimensions; actual size may vary)
 *              scorefract (fraction of the max Otsu score; typ. 0.1)
 *              fdrthresh (threshold for Fisher's Discriminant Rate; typ. 5.0)
 *      Return: 0 if OK, 1 on error
 */
l_int32 pixFisherAdaptiveThreshold(PIX *pixs, PIX **ppixd, l_int32 tile_x, l_int32 tile_y,
                                l_float32 score_fract, l_float32 thresh) {
  l_float32 fdr;
  l_int32 w, h, d, nx, ny, x, y, t;
  PIX *pixb, *pixd, *pixt;
  PIXTILING *pt;

  PROCNAME("pixFisherAdaptiveThreshold");

  if (!pixs)
    return ERROR_INT("pixs not defined", procName, 1);
  if (!ppixd)
    return ERROR_INT("&ppixd not defined", procName, 1);
  pixGetDimensions(pixs, &w, &h, &d);
  if (d != 8)
    return ERROR_INT("pixs not 8 bpp", procName, 1);
  if (tile_x < 8 || tile_y < 8)
    return ERROR_INT("sx and sy must be >= 8", procName, 1);

  /* Compute FDR & threshold for individual tiles */
  nx = L_MAX(1, w / tile_x);
  ny = L_MAX(1, h / tile_y);
  pt = pixTilingCreate(pixs, nx, ny, 0, 0, 0, 0);
  pixd = pixCreate(w, h, 1);
  for (y = 0; y < ny; y++) {
    for (x = 0; x < nx; x++) {
      pixt = pixTilingGetTile(pt, y, x);
      pixGetFisherThresh(pixt, score_fract, &fdr, &t);

      if (fdr > thresh) {
        pixb = pixThresholdToBinary(pixt, t);
        pixTilingPaintTile(pixd, y, x, pixb, pt);
        pixDestroy(&pixb);
      }

      pixDestroy(&pixt);
    }
  }

  pixTilingDestroy(&pt);

  *ppixd = pixd;

  return 0;
}

/*!
 *  pixGetFisherThresh()
 *
 *      Input:  pixs (any depth; cmapped ok)
 *              scorefract (fraction of the max score, used to determine
 *                          the range over which the histogram min is searched)
 *              &xfdr (<optional return> Fisher's Discriminate Rate value)
 *              &xthresh (<optional return> Otsu threshold value)
 *      Return: 0 if OK, 1 on error
 */
l_int32 pixGetFisherThresh(PIX *pixs, l_float32 scorefract, l_float32 *pfdr, l_int32 *pthresh) {
  l_float32 mean1, mean2, sum, sum1, sum2, fract;
  l_float32 var, between, within, fdr;
  l_int32 thresh;
  NUMA *na;

  PROCNAME("pixGetFisherThresh");

  if (!pixs)
    return ERROR_INT("pixs not defined", procName, 1);
  if (!pfdr && !pthresh)
    return ERROR_INT("neither &pfdr nor &pthresh defined", procName, 1);

  na = pixGetGrayHistogram(pixs, 1);

  /* Compute Otsu threshold for histogram */
  numaSplitDistribution(na, scorefract, &thresh, &mean1, &mean2, &sum1, &sum2, NULL);

  /* Compute Fisher's Discriminant Rate if needed */
  if (pfdr) {
    numaGetHistogramStats(na, 0.0, 1.0, NULL, NULL, NULL, &var);
    numaGetSum(na, &sum);

    /* Between-class variance = sum of weighted squared distances
     between-class and overall means */
    fract = sum1 / sum;
    between = (fract * (1 - fract)) * (mean1 - mean2) * (mean1 - mean2);

    /* Within-class variance = difference between total variance
     and between-class variance */
    within = var - between;

    /* FDR = between-class variance over within-class variance */
    if (within <= 1) {
      fdr = between;
    } else {
      fdr = between / within;
    }

    *pfdr = fdr;
  }

  if (pthresh)
    *pthresh = thresh;

  numaDestroy(&na);

  return 0;
}

PIX *pixThreshedSobelEdgeFilter(PIX *pixs, l_int32 threshold) {
  l_uint8 bval, bidx;
  l_int32 w, h, d, i, j, wplt, wpld, gx, gy, vald;
  l_int32 val1, val2, val3, val4, val5, val6, val7, val8, val9;
  l_uint32 *datat, *linet, *datad, *lined;
  PIX *pixd;

  PROCNAME("pixThreshedSobelEdgeFilter");

  if (!pixs)
    return (PIX *) ERROR_PTR("pixs not defined", procName, NULL);
  pixGetDimensions(pixs, &w, &h, &d);
  if (d != 8)
    return (PIX *) ERROR_PTR("pixs not 8 bpp", procName, NULL);

  /* Compute filter output at each location. */
  pixd = pixCreateNoInit(w, h, 1);
  datat = pixGetData(pixs);
  wplt = pixGetWpl(pixs);
  datad = pixGetData(pixd);
  wpld = pixGetWpl(pixd);
  val1 = val2 = val3 = val4 = val5 = 0;
  val6 = val7 = val8 = val9 = 0;
  bval = bidx = 0;
  for (i = 0; i < h - 1; i++) {
    linet = datat + i * wplt;
    lined = datad + i * wpld;
    for (j = 0; j < w - 1; j++) {
      if (j == 0) { /* start a new row */
        val1 = GET_DATA_BYTE(linet, j);
        val2 = GET_DATA_BYTE(linet + wplt, j);
        val3 = GET_DATA_BYTE(linet + (wplt << 1), j);
        val4 = GET_DATA_BYTE(linet, j + 1);
        val5 = GET_DATA_BYTE(linet + wplt, j + 1);
        val6 = GET_DATA_BYTE(linet + (wplt << 1), j + 1);
        val7 = GET_DATA_BYTE(linet, j + 2);
        val8 = GET_DATA_BYTE(linet + wplt, j + 2);
        val9 = GET_DATA_BYTE(linet + (wplt << 1), j + 2);
        bval = 0;
        bidx = 0x80;
      } else { /* shift right by 1 pixel; update incrementally */
        val1 = val4;
        val2 = val5;
        val3 = val6;
        val4 = val7;
        val5 = val8;
        val6 = val9;
        val7 = GET_DATA_BYTE(linet, j + 2);
        val8 = GET_DATA_BYTE(linet + wplt, j + 2);
        val9 = GET_DATA_BYTE(linet + (wplt << 1), j + 2);
      }

      bval <<= 1;
      bidx >>= 1;

      gx = val1 + (val2 << 1) + val3 - val7 - (val8 << 1) - val9;
      gy = val1 + (val4 << 1) + val7 - val3 - (val6 << 1) - val9;
      vald = L_MIN(255, L_ABS(gx) + L_ABS(gy));

      /* Flip high bit if value exceeds threshold */
      if (vald >= threshold) {
        bval |= 1;
      }

      if (bidx == 0) {
        SET_DATA_BYTE(lined, j / 8, bval);

        bval = 0;
        bidx = 0x80;
      }
    }
  }

  return pixd;
}

l_uint8 pixGradientEnergy(PIX *pixs, PIX *mask, l_float32 *penergy) {
  l_int32 w, h, d;
  l_uint8 val1, val2;
  l_uint32 mask1, mask2;
  l_int32 wpls, wplm;
  l_uint32 *datas, *lines;
  l_uint32 *datam, *linem;
  l_int32 total, count;

  PROCNAME("pixGradientEnergy");

  if (!pixs)
    return ERROR_INT("pixs not defined", procName, -1);
  pixGetDimensions(pixs, &w, &h, &d);
  if (d != 8)
    return ERROR_INT("pixs not 8 bpp", procName, -1);

  /* Compute filter output at each location. */
  datas = pixGetData(pixs);
  wpls = pixGetWpl(pixs);
  datam = pixGetData(mask);
  wplm = pixGetWpl(mask);
  total = 0;
  count = 1;
  mask1 = mask2 = 0;
  val1 = val2 = 0;
  for (int y = 0; y < h; y++) {
    lines = datas + y * wpls;
    linem = datam + y * wplm;
    for (int x = 0; x < w - 1; x++) {
      if (x == 0) { /* start a new row */
        mask1 = GET_DATA_BIT(linem, x);
        mask2 = GET_DATA_BIT(linem, x + 1);
        val1 = GET_DATA_BYTE(lines, x);
        val2 = GET_DATA_BYTE(lines, x + 1);
      } else { /* shift right by 1 pixel; update incrementally */
        val1 = val2;
        val2 = GET_DATA_BYTE(lines, x + 1);
        mask1 = mask2;
        mask2 = GET_DATA_BIT(linem, x + 1);
      }

      /* If we're on an edge, add the gradient value and increment */
      if (mask1 != mask2) {
        total += L_ABS(val1 - val2);
        count += 1;
      }
    }
  }

  *penergy = total / (l_float32) count;

  return 0;
}

l_uint8 pixEdgeMax(PIX *pixs, l_int32 *pmax, l_int32 *pavg) {
  l_int32 w, h, d, wplt, vald;
  l_uint8 val1, val2, val3, val4, val5;
  l_uint32 *datat, *linet;
  l_int32 max, total;

  PROCNAME("pixEdgeMax");

  if (!pixs)
    return ERROR_INT("pixs not defined", procName, -1);
  pixGetDimensions(pixs, &w, &h, &d);
  if (d != 8)
    return ERROR_INT("pixs not 8 bpp", procName, -1);

  /* Compute filter output at each location. */
  datat = pixGetData(pixs);
  wplt = pixGetWpl(pixs);
  max = 0;
  total = 0;
  val1 = val2 = val3 = val4 = val5 = 0;
  for (int y = 0; y < h; y++) {
    linet = datat + y * wplt;
    for (int x = 0; x < w - 5; x++) {
      if (x == 0) { /* start a new row */
        val1 = GET_DATA_BYTE(linet, x);
        val2 = GET_DATA_BYTE(linet, x + 1);
        val3 = GET_DATA_BYTE(linet, x + 2);
        val4 = GET_DATA_BYTE(linet, x + 3);
        val5 = GET_DATA_BYTE(linet, x + 4);
      } else { /* shift right by 1 pixel; update incrementally */
        val1 = val2;
        val2 = val3;
        val3 = val4;
        val4 = val5;
        val5 = GET_DATA_BYTE(linet, x + 4);
      }

      //maxd = L_MAX(val5, L_MAX(val4, L_MAX(val3, L_MAX(val2, val1))));
      //mind = L_MIN(val5, L_MIN(val4, L_MIN(val3, L_MIN(val2, val1))));
      vald = L_ABS(val1 - val5); //maxd - mind;

      if (vald > max) {
        max = vald;
      }

      total += vald;
    }
  }

  *pmax = max;
  *pavg = total / (w * h);

  return 0;
}

/*!
 *  pixEdgeAdaptiveThreshold()
 *
 *      Input:  pixs (8 bpp)
 *              &pixd (<required return> thresholded input for pixs)
 *              tile_x, tile_y (desired tile dimensions; actual size may vary)
 *              thresh
 *              avg_thresh
 *      Return: 0 if OK, 1 on error
 */
l_uint8 pixEdgeAdaptiveThreshold(PIX *pixs, PIX **ppixd, l_int32 tile_x, l_int32 tile_y,
                                  l_int32 thresh, l_int32 avg_thresh) {
  l_int32 w, h, d, nx, ny, x, y, t, max, avg;
  PIX *pixb, *pixd, *pixt;
  PIXTILING *pt;

  PROCNAME("pixEdgeAdaptiveThreshold");

  if (!pixs)
    return ERROR_INT("pixs not defined", procName, 1);
  if (!ppixd)
    return ERROR_INT("&ppixd not defined", procName, 1);
  pixGetDimensions(pixs, &w, &h, &d);
  if (d != 8)
    return ERROR_INT("pixs not 8 bpp", procName, 1);
  if (tile_x < 8 || tile_y < 8)
    return ERROR_INT("sx and sy must be >= 8", procName, 1);

  /* Compute FDR & threshold for individual tiles */
  nx = L_MAX(1, w / tile_x);
  ny = L_MAX(1, h / tile_y);
  pt = pixTilingCreate(pixs, nx, ny, 0, 0, 0, 0);
  pixd = pixCreate(w, h, 1);
  for (y = 0; y < ny; y++) {
    for (x = 0; x < nx; x++) {
      pixt = pixTilingGetTile(pt, y, x);
      pixEdgeMax(pixt, &max, &avg);

      if (max > thresh && avg > avg_thresh) {
        pixSplitDistributionFgBg(pixt, 0.0, 1, &t, NULL, NULL, 0);
        pixb = pixThresholdToBinary(pixt, t);
        pixTilingPaintTile(pixd, y, x, pixb, pt);
        pixDestroy(&pixb);
      }

      pixDestroy(&pixt);
    }
  }

  pixTilingDestroy(&pt);

  *ppixd = pixd;

  return 0;
}
