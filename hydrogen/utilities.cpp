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

/* smart sharpen
 * PIX *edgemask = pixThreshedSobelEdgeFilter(pix8, 16, 2, L_ALL_EDGES);
 * PIX *enhanced = pixUnsharpMasking(pix8, UNSHARP_HALFWIDTH, UNSHARP_FRACTION);
 * PIX *blended = pixBlendWithGrayMask(normalized, enhanced, edgemask, 0, 0);
 */

#include "leptonica.h"
#include "utilities.h"

void numaJoin(NUMA *dst, NUMA *src) {
  l_int32 count = numaGetCount(src);
  l_float32 temp;

  for (int i = 0; i < count; i++) {
    numaGetFValue(src, i, &temp);
    numaAddNumber(dst, temp);
  }
}

PIX *pixaDisplayRandomCmapFiltered(PIXA *pixa, l_int32 w, l_int32 h, l_uint8 *filter) {
  l_int32 i, n, d, index, xb, yb, wb, hb;
  BOXA *boxa;
  PIX *pixs, *pixt, *pixd;
  PIXCMAP *cmap;

  PROCNAME("pixaDisplayRandomCmapFiltered");

  if (!pixa)
    return (PIX *) ERROR_PTR("pixa not defined", procName, NULL);
  n = pixaGetCount(pixa);
  if (n == 0)
    return (PIX *) ERROR_PTR("no components", procName, NULL);

  /* Use the first pix in pixa to verify depth is 1 bpp  */
  pixs = pixaGetPix(pixa, 0, L_CLONE);
  d = pixGetDepth(pixs);
  pixDestroy(&pixs);
  if (d != 1)
    return (PIX *) ERROR_PTR("components not 1 bpp", procName, NULL);

  /* If w and h not input, determine the minimum size required
   * to contain the origin and all c.c. */
  if (w == 0 || h == 0) {
    boxa = pixaGetBoxa(pixa, L_CLONE);
    boxaGetExtent(boxa, &w, &h, NULL);
    boxaDestroy(&boxa);
  }

  /* Set up an 8 bpp dest pix, with a colormap with 254 random colors */
  if ((pixd = pixCreate(w, h, 8)) == NULL)
    return (PIX *) ERROR_PTR("pixd not made", procName, NULL);
  cmap = pixcmapCreateRandom(8, 1, 1);
  pixSetColormap(pixd, cmap);

  /* Color each component and blit it in */
  for (i = 0; i < n; i++) {
    if (filter[i])
      continue;
    index = 1 + (i % 254);
    pixaGetBoxGeometry(pixa, i, &xb, &yb, &wb, &hb);
    pixs = pixaGetPix(pixa, i, L_CLONE);
    pixt = pixConvert1To8(NULL, pixs, 0, index);
    pixRasterop(pixd, xb, yb, wb, hb, PIX_PAINT, pixt, 0, 0);
    pixDestroy(&pixs);
    pixDestroy(&pixt);
  }

  return pixd;
}

/*!
 *  pixcmapCreateHeatmap()
 *
 *      Input:  d (depth of pix for this colormap; 1, 2, 4 or 8)
 *      Return: cmap, or null on error
 *
 *  Notes:
 *      (1) Colormap shows 0 as black, 1-MAX as a range from violet to red.
 */
PIXCMAP *
pixcmapCreateHeatmap(l_int32  d)
{
l_int32   nlevels, i, h, s, v;
PIXCMAP  *cmap;

    PROCNAME("pixcmapCreateHeatmap");

    if (d != 1 && d != 2 && d !=4 && d != 8)
        return (PIXCMAP *)ERROR_PTR("d not in {1, 2, 4, 8}", procName, NULL);
    nlevels = 1 << d;

    cmap = pixcmapCreate(d);

    nlevels--;
    pixcmapAddColor(cmap, 0, 0, 0);

    for (i = nlevels; i > 0; i--) {
        h = 170 * i / nlevels;
        s = 255;
        v = 255;
        pixcmapAddColor(cmap, h, s, v);
    }

    pixcmapConvertHSVToRGB(cmap);

    return cmap;
}

PIX *pixaDisplayHeatmap(PIXA *pixa, l_int32 w, l_int32 h, NUMA *confs) {
  l_int32 i, n, d, val, xb, yb, wb, hb;
  l_float32 maxconf, minconf, confrange, conf;
  BOXA *boxa;
  PIX *pixs, *pixt, *pixd;
  PIXCMAP *cmap;

  PROCNAME("pixaDisplayHeatmap");

  if (!pixa)
    return (PIX *) ERROR_PTR("pixa not defined", procName, NULL);
  n = pixaGetCount(pixa);
  if (n == 0)
    return (PIX *) ERROR_PTR("no components", procName, NULL);

  /* Use the first pix in pixa to verify depth is 1 bpp  */
  pixs = pixaGetPix(pixa, 0, L_CLONE);
  d = pixGetDepth(pixs);
  pixDestroy(&pixs);
  if (d != 1)
    return (PIX *) ERROR_PTR("components not 1 bpp", procName, NULL);

  /* If w and h not input, determine the minimum size required
   * to contain the origin and all c.c. */
  if (w == 0 || h == 0) {
    boxa = pixaGetBoxa(pixa, L_CLONE);
    boxaGetExtent(boxa, &w, &h, NULL);
    boxaDestroy(&boxa);
  }

  /* Determine the confidence range */
  numaGetMin(confs, &minconf, NULL);
  numaGetMax(confs, &maxconf, NULL);
  confrange = maxconf - minconf;

  /* Set up an 8 bpp dest pix, with a colormap with 254 random colors */
  if ((pixd = pixCreate(w, h, 8)) == NULL)
    return (PIX *) ERROR_PTR("pixd not made", procName, NULL);
  cmap = pixcmapCreateHeatmap(8);
  pixSetColormap(pixd, cmap);

  /* Color each component and blit it in */
  for (i = 0; i < n; i++) {
    numaGetFValue(confs, i, &conf);
    val = (l_int32) ((conf - minconf) / confrange * 254) + 1;
    pixaGetBoxGeometry(pixa, i, &xb, &yb, &wb, &hb);
    pixs = pixaGetPix(pixa, i, L_CLONE);
    pixt = pixConvert1To8(NULL, pixs, 0, val);
    pixRasterop(pixd, xb, yb, wb, hb, PIX_PAINT, pixt, 0, 0);
    pixDestroy(&pixs);
    pixDestroy(&pixt);
  }

  return pixd;
}
