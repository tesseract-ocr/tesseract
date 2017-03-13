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

#include <malloc.h>
#include "leptonica.h"
#include "clusterer.h"
#include "validator.h"

/* Type of connected components: 4 is up/down/left/right. 8 includes diagonals */
#define CONN_COMP 8

l_int32 ConnCompValidPixa(PIX *pix8, PIX *pix, PIXA **ppixa, NUMA **pconfs,
                          HydrogenTextDetector::TextDetectorParameters &params) {
  l_int32 h, iszero;
  l_int32 x, y, xstart, ystart;
  l_float32 singleton_conf;
  PIX *pixt1, *pixt2, *pixt3, *pixt4, *pixt5;
  PIXA *pixa, *pixasort;
  NUMA *confs, *confsort;
  BOX *box;
  BOXA *boxa;
  L_STACK *lstack, *auxstack;

  PROCNAME("pixConnCompValidPixa");

  if (!ppixa)
    return ERROR_INT("&pixa not defined", procName, 1);
  if (!pconfs)
    return ERROR_INT("&confs not defined", procName, 1);
  *ppixa = NULL;
  *pconfs = NULL;
  if (!pix || pixGetDepth(pix) != 1)
    return ERROR_INT("pixs undefined or not 1 bpp", procName, 1);

  pixa = pixaCreate(0);
  confs = numaCreate(0);

  pixZero(pix, &iszero);
  if (iszero) {
    *ppixa = pixa;
    return 0;
  }

  if ((pixt1 = pixCopy(NULL, pix)) == NULL)
    return ERROR_INT("pixt1 not made", procName, 1);
  if ((pixt2 = pixCopy(NULL, pix)) == NULL)
    return ERROR_INT("pixt2 not made", procName, 1);

  h = pixGetHeight(pix);
  if ((lstack = lstackCreate(h)) == NULL)
    return ERROR_INT("lstack not made", procName, 1);
  if ((auxstack = lstackCreate(0)) == NULL)
    return ERROR_INT("auxstack not made", procName, 1);
  lstack->auxstack = auxstack;
  if ((boxa = boxaCreate(0)) == NULL)
    return ERROR_INT("boxa not made", procName, 1);

  xstart = 0;
  ystart = 0;
  while (1) {
    if (!nextOnPixelInRaster(pixt1, xstart, ystart, &x, &y))
      break;

    if ((box = pixSeedfillBB(pixt1, lstack, x, y, CONN_COMP)) == NULL)
      return ERROR_INT("box not made", procName, 1);

    /* Save the c.c. and remove from pixt2 as well */
    pixt3 = pixClipRectangle(pixt1, box, NULL);
    pixt4 = pixClipRectangle(pixt2, box, NULL);
    pixt5 = pixClipRectangle(pix8, box, NULL);
    pixXor(pixt3, pixt3, pixt4);
    pixRasterop(pixt2, box->x, box->y, box->w, box->h, PIX_SRC ^ PIX_DST, pixt3, 0, 0);
    pixDestroy(&pixt4);

    if (ValidateSingleton(pixt3, box, pixt5, &singleton_conf, params)) {
      boxaAddBox(boxa, box, L_INSERT);
      pixaAddPix(pixa, pixt3, L_INSERT);
      numaAddNumber(confs, singleton_conf);
    } else {
      boxDestroy(&box);
      pixDestroy(&pixt3);
    }

    pixDestroy(&pixt5);

    xstart = x;
    ystart = y;
  }

  /* Remove old boxa of pixa and replace with a clone copy */
  boxaDestroy(&pixa->boxa);
  pixa->boxa = boxaCopy(boxa, L_CLONE);

  /* Sort pixa, then destroy old pixa */
  NUMA *naindex;
  if ((pixasort = pixaSort(pixa, L_SORT_BY_X, L_SORT_INCREASING, &naindex, L_CLONE)) == NULL)
    return ERROR_INT("pixasort not made", procName, 1);
  confsort = numaSortByIndex(confs, naindex);

  /* Cleanup, freeing the fillsegs on each stack */
  lstackDestroy(&lstack, TRUE);
  pixDestroy(&pixt1);
  pixDestroy(&pixt2);
  boxaDestroy(&boxa);
  pixaDestroy(&pixa);

  *ppixa = pixasort;
  *pconfs = confsort;

  return 0;
}

l_int32 MergePix(PIXA *pixad, l_int32 d_idx, PIXA *pixas, l_int32 s_idx) {
  l_int32 op;
  l_int32 x, y, w, h;
  l_int32 dx, dy, dw, dh;
  PIX *pixd, *pixs, *pixmerge;
  BOX *boxd, *boxs, *boxmerge;

  PROCNAME("pixMergePix");

  if (!pixad)
    return ERROR_INT("pixad not defined", procName, 1);
  if (!pixas)
    return ERROR_INT("pixas not defined", procName, 1);

  boxd = pixaGetBox(pixad, d_idx, L_CLONE);
  boxs = pixaGetBox(pixas, s_idx, L_CLONE);
  boxmerge = boxBoundingRegion(boxd, boxs);

  boxGetGeometry(boxmerge, &x, &y, &w, &h);
  pixmerge = pixCreate(w, h, 1);

  op = PIX_SRC | PIX_DST;

  pixs = pixaGetPix(pixas, s_idx, L_CLONE);

  if (!pixs)
    return ERROR_INT("s_idx not valid", procName, 1);

  boxGetGeometry(boxs, &dx, &dy, &dw, &dh);
  pixRasterop(pixmerge, dx - x, dy - y, dw, dh, op, pixs, 0, 0);
  pixDestroy(&pixs);
  boxDestroy(&boxs);

  pixd = pixaGetPix(pixad, d_idx, L_CLONE);

  if (!pixd)
    return ERROR_INT("d_idx not valid", procName, 1);

  boxGetGeometry(boxd, &dx, &dy, &dw, &dh);
  pixRasterop(pixmerge, dx - x, dy - y, dw, dh, op, pixd, 0, 0);
  pixDestroy(&pixd);
  boxDestroy(&boxd);

  pixaReplacePix(pixad, d_idx, pixmerge, boxmerge);

  return 0;
}

l_int32 MergePairFragments(PIX *pix8, PIXA *clusters, PIXA *pixa, l_uint8 *remove) {
  l_uint8 setj;
  l_int32 i, j, real_j, contains, n, count, num_clusters, initj;
  l_int32 xi, yi, wi, hi;
  l_int32 xj, yj, wj, hj;
  BOX *boxi, *boxj;
  PIXA *pixasort;
  NUMA *numa;

  PROCNAME("pixMergePairFragments");

  if (!pixa)
    return ERROR_INT("pixa not defined", procName, -1);
  if (!remove)
    return ERROR_INT("remove not defined", procName, -1);

  n = pixaGetCount(pixa);
  num_clusters = pixaGetCount(clusters);

  if (!n) {
    L_INFO("pixa contained 0 pix", procName);
    return 0;
  }
  if (!num_clusters) {
    L_INFO("clusters contained 0 pix", procName);
    return 0;
  }
  if ((pixasort = pixaSort(pixa, L_SORT_BY_Y, L_SORT_INCREASING, &numa, L_CLONE)) == NULL)
    return ERROR_INT("failed to sort pixa", procName, -1);

  count = 0;
  initj = 0;
  setj = 0;

  for (i = 0; i < num_clusters; i++) {
    pixaGetBoxGeometry(clusters, i, &xi, &yi, &wi, &hi);
    boxi = pixaGetBox(clusters, i, L_CLONE);

    setj = 0;

    for (j = initj; j < n; j++) {
      numaGetIValue(numa, j, &real_j);

      // Only consider removed pix
      if (!remove[real_j])
        continue;

      pixaGetBoxGeometry(pixasort, j, &xj, &yj, &wj, &hj);

      // If the top of this pix is above the top of the cluster, skip
      if (yj < yi)
        continue;

      if (!setj) {
        initj = j;
        setj = 1;
      }

      // If the bottom of this pix is below the bottom of the cluster, stop
      if (yj > yi + hi)
        break;

      boxj = pixaGetBox(pixasort, j, L_CLONE);

      boxIntersects(boxi, boxj, &contains);

      if (contains) {
        MergePix(clusters, i, pixasort, j);
        //remove[real_j] = 0; // TODO eliminates duplicates
        count++;
      }

      boxDestroy(&boxj);
    }

    boxDestroy(&boxi);
  }

  pixaDestroy(&pixasort);
  numaDestroy(&numa);

  return count;
}

l_int32 RemoveInvalidPairs(PIX *pix8, PIXA *pixa, NUMA *confs, l_uint8 *remove,
                           HydrogenTextDetector::TextDetectorParameters &params) {
  l_int32 i, j, n, count;
  l_float32 pair_conf;
  l_uint8 *has_partner;
  BOX *b1, *b2;

  PROCNAME("pixRemoveInvalidPairs");

  if (!pixa)
    return ERROR_INT("pixa not defined", procName, -1);
  if (!remove)
    return ERROR_INT("remove not defined", procName, -1);

  n = pixaGetCount(pixa);

  if (!n) {
    L_INFO("pixa contained 0 pix", procName);
    return 0;
  }

  has_partner = (l_uint8 *) calloc(n, sizeof(l_uint8));
  count = 0;

  for (i = 0; i < n; i++) {
    if (remove[i])
      continue;

    b1 = pixaGetBox(pixa, i, L_CLONE);

    /* Search right for a partner for i */
    for (j = i + 1; j < n; j++) {
      if (remove[j])
        continue;

      b2 = pixaGetBox(pixa, j, L_CLONE);

      /* Check whether this is a valid pair */
      if (!ValidatePair(b1, b2, &pair_conf, params)) {
        boxDestroy(&b2);
        continue;
      }

      // We don't need to adjust confidence values here, since we'll
      // generate cluster pairs and use those later.

      boxDestroy(&b2);

      has_partner[i] = 1;
      has_partner[j] = 1;
      break;
    }

    boxDestroy(&b1);
  }

  for (i = 0; i < n; i++) {
    if (!has_partner[i]) {
      remove[i] = 1;

      count++;
    }
  }

  free(has_partner);

  return count;
}

// Clustering pass

l_int32 GenerateClusterPartners(PIX *pix8, PIXA *pixa, NUMA *confs, l_uint8 *remove, l_int32 **pleft,
                                l_int32 **pright, HydrogenTextDetector::TextDetectorParameters &params) {
  l_int32 n, i, j;
  l_int32 xi, yi, wi, hi, maxd;
  l_int32 xj, yj, wj, hj;
  l_int32 dx, dy, d, mind, minj;
  l_int32 *left, *right;
  l_float32 clusterpair_conf, minconf;
  BOX *b1, *b2;
  bool too_far;

  PROCNAME("GenerateClusterPartners");

  if (!pixa)
    return ERROR_INT("pixa not defined", procName, -1);
  if (!pright)
    return ERROR_INT("&right not defined", procName, -1);
  if (!pleft)
    return ERROR_INT("&left not defined", procName, -1);

  n = pixaGetCount(pixa);

  if (!n) {
    L_INFO("pixa contained 0 pix", procName);
    return 0;
  }

  /* If n == 0, remove may be NULL. Since we have already checked for that,
   * any NULL arrays signal an error condition.
   */
  if (!remove)
    return ERROR_INT("remove not defined", procName, -1);

  left = (l_int32 *) malloc(n * sizeof(l_int32));
  right = (l_int32 *) malloc(n * sizeof(l_int32));

  /* Initialize left and right arrays */
  for (i = 0; i < n; i++) {
    left[i] = -2;
    right[i] = -2;
  }

  /* For each component, check all possible neighbors to find the most likely
   * right neighbor. If that right neighbor already has a left neighbor, insert
   * the component to the right of the existing neighbor and the left of the
   * right neighbor.
   */
  for (i = 0; i < n; i++) {
    if (remove[i])
      continue;

    pixaGetBoxGeometry(pixa, i, &xi, &yi, &wi, &hi);
    b1 = pixaGetBox(pixa, i, L_CLONE);
    mind = -1;
    minj = -1;
    maxd = L_MAX(wi, hi);
    minconf = 0.0;

    /* Search for closest right neighbor */
    for (j = i + 1; j < n; j++) {
      if (remove[j])
        continue;

      pixaGetBoxGeometry(pixa, j, &xj, &yj, &wj, &hj);
      b2 = pixaGetBox(pixa, j, L_CLONE);

      if (!ValidateClusterPair(b1, b2, &too_far, &clusterpair_conf, params)) {
        if (too_far)
          break;
        else
          continue;
      }

      /* calculate spacing between i and j */
      dx = xj - (xi + wi);
      dy = (yj + hj) - (yi + hi);
      d = dx * dx + dy * dy;

      /* If we haven't found a neighbor OR we're the closest neighbor, update
       * i's record for most likely neighbor.
       */
      if (mind < 0 || d < mind) {
        mind = d;
        minj = j;
        minconf = clusterpair_conf;
      }
    }

    /* If we found a valid neighbor, go ahead and use it. */
    if (mind >= 0) {
      j = left[minj];

      /* If minj already had a left neighbor, replace it with i */
      // TODO(alanv): Insertion fudges the partner confidence value
      if (j >= 0) {
        left[i] = j;
        right[j] = i;
      }

      left[minj] = i;
      right[i] = minj;

      // Adjust confidence to reflect partner confidence
      l_float32 conf;
      numaGetFValue(confs, i, &conf);
      conf *= minconf;
      numaReplaceNumber(confs, i, conf);
    }
  }

  *pleft = left;
  *pright = right;

  return 0;
}

l_int32 MergeClusterPartners(PIX *pix8, PIXA *pixa, NUMA *confs, l_uint8 *remove, l_int32 *left, l_int32 *right,
                             PIXA **ppixad, NUMA **pclusterconfs, HydrogenTextDetector::TextDetectorParameters &params) {
  l_int32 n, count, i, j, temp;
  l_uint32 x, y, w, h;
  l_int32 xi, yi, wi, hi;
  l_int32 xj, yj, wj, hj;
  PIXA *pixad, *pixa_cluster;
  NUMA *confd, *numa_cluster;
  PIX *pix, *pixd, *pix_cluster;
  BOX *box, *boxd;

  PROCNAME("ClusterValidComponents");

  if (!ppixad)
    return ERROR_INT("&pixad not defined", procName, -1);
  if (!pclusterconfs)
    return ERROR_INT("&clusterconfs not defined", procName, -1);

  n = pixaGetCount(pixa);

  pixad = pixaCreate(0);
  confd = numaCreate(0);
  *ppixad = pixad;
  *pclusterconfs = confd;

  if (!n) {
    L_INFO("pixa contained 0 pix", procName);
    return 0;
  }

  /* If n == 0, then left, right, and remove may be NULL. Since we have
   * already checked for that, any NULL arrays signal an error condition.
   */
  if (!left)
    return ERROR_INT("left not defined", procName, -1);
  if (!right)
    return ERROR_INT("right not defined", procName, -1);
  if (!remove)
    return ERROR_INT("remove not defined", procName, -1);

  count = 0;

  /* Starting from the first component, generate a cluster by traveling
   * left and right as far as possible. Ignore components that have no
   * neighbors.
   */
  for (i = 0; i < n; i++) {
    if (remove[i])
      continue;
    if (left[i] < -1 && right[i] < -1)
      remove[i] = 1;
    if (left[i] < 0 && right[i] < 0)
      continue;

    pixa_cluster = pixaCreate(1);
    numa_cluster = numaCreate(1);

    /* We don't need to destroy this pix and box since pixa_cluster
     * takes ownership with L_INSERT.
     */
    pix = pixaGetPix(pixa, i, L_CLONE);
    box = pixaGetBox(pixa, i, L_CLONE);
    pixaAddPix(pixa_cluster, pix, L_INSERT);
    pixaAddBox(pixa_cluster, box, L_INSERT);
    numaAddNumber(numa_cluster, i);

    boxGetGeometry(box, &xi, &yi, &wi, &hi);
    x = xi;
    y = yi;
    w = xi + wi;
    h = yi + hi;

    /* Move along left neighbors */
    j = left[i];
    left[i] = -1;
    while (j >= 0) {
      pix = pixaGetPix(pixa, j, L_CLONE);
      box = pixaGetBox(pixa, j, L_CLONE);
      pixaAddPix(pixa_cluster, pix, L_INSERT);
      pixaAddBox(pixa_cluster, box, L_INSERT);
      numaAddNumber(numa_cluster, j);

      boxGetGeometry(box, &xj, &yj, &wj, &hj);
      x = L_MIN(x, (l_uint32) xj);
      y = L_MIN(y, (l_uint32) yj);
      w = L_MAX(w, (l_uint32) (xj + wj));
      h = L_MAX(h, (l_uint32) (yj + hj));

      right[j] = -1;
      temp = left[j];
      left[j] = -1;
      j = temp;
    }

    /* Move along right neighbors */
    j = right[i];
    right[i] = -1;
    while (j >= 0) {
      pix = pixaGetPix(pixa, j, L_CLONE);
      box = pixaGetBox(pixa, j, L_CLONE);
      pixaAddPix(pixa_cluster, pix, L_INSERT);
      pixaAddBox(pixa_cluster, box, L_INSERT);
      numaAddNumber(numa_cluster, j);

      boxGetGeometry(box, &xj, &yj, &wj, &hj);
      x = L_MIN(x, (l_uint32) xj);
      y = L_MIN(y, (l_uint32) yj);
      w = L_MAX(w, (l_uint32) xj + wj);
      h = L_MAX(h, (l_uint32) (yj + hj));

      left[j] = -1;
      temp = right[j];
      right[j] = -1;
      j = temp;
    }

    w = w - x;
    h = h - y;

    boxd = boxCreate(x, y, w, h);
    pix_cluster = pixClipRectangle(pix8, boxd, NULL);

    l_float32 temp_conf;
    l_float32 cluster_conf;

    /* If pixa seems valid, collapse its components to a single pix */
    if (ValidateCluster(pix_cluster, pixa_cluster, boxd, &cluster_conf, params)) {
      l_int32 num_comps = pixaGetCount(pixa_cluster);
      l_float32 avg_conf = 0.0;

      pixd = pixCreate(w, h, 1);

      for (int i = 0; i < num_comps; i++) {
        pix = pixaGetPix(pixa_cluster, i, L_CLONE);
        pixaGetBoxGeometry(pixa_cluster, i, &xj, &yj, &wj, &hj);
        pixRasterop(pixd, xj - x, yj - y, wj, hj, PIX_PAINT, pix, 0, 0);
        pixDestroy(&pix);

        numaGetFValue(confs, i, &temp_conf);
        avg_conf += temp_conf;
      }

      // Adjust average confidence to reflect overall cluster confidence
      avg_conf /= num_comps;
      avg_conf *= cluster_conf;

      pixaAddPix(pixad, pixd, L_INSERT);
      pixaAddBox(pixad, boxd, L_INSERT);
      numaAddNumber(confd, avg_conf);

      count++;
    } else {
      l_int32 num_nums = numaGetCount(numa_cluster);

      // Otherwise, mark its components as removed
      for (int i = 0; i < num_nums; i++) {
        if (!numaGetIValue(numa_cluster, i, &temp)) {
          remove[temp] = 1;
        }
      }

      boxDestroy(&boxd);
    }

    pixDestroy(&pix_cluster);
    pixaDestroy(&pixa_cluster);
    numaDestroy(&numa_cluster);
  }

  free(left);
  free(right);

  PIXA *pixasort;
  NUMA *confsort;

  /* Sort pixa, then destroy old pixa */
  NUMA *naindex;
  if ((pixasort = pixaSort(pixad, L_SORT_BY_Y, L_SORT_INCREASING, &naindex, L_CLONE)) == NULL)
    return ERROR_INT("pixasort not made", procName, 1);
  confsort = numaSortByIndex(confd, naindex);

  *ppixad = pixasort;
  *pclusterconfs = confsort;

  pixaDestroy(&pixad);
  numaDestroy(&confd);

  return count;
}

l_int32 ClusterValidComponents(PIX *pix8, PIXA *pixa, NUMA *confs, l_uint8 *remove, PIXA **ppixad,
                               NUMA **pclusterconfs, HydrogenTextDetector::TextDetectorParameters &params) {
  l_int32 *left, *right;
  PIXA *pixad;
  NUMA *clusterconfs;

  if (GenerateClusterPartners(pix8, pixa, confs, remove, &left, &right, params))
    return -1;

  int count = MergeClusterPartners(pix8, pixa, confs, remove, left, right, &pixad, &clusterconfs, params);

  *ppixad = pixad;
  *pclusterconfs = clusterconfs;

  return count;
}
