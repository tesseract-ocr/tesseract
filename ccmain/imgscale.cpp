/**********************************************************************
 * File:        imgscale.cpp  (Formerly dyn_prog.c)
 * Description: Dynamic programming for smart scaling of images.
 * Author:      Phil Cheatle
 * Created:     Wed Nov 18 16:12:03 GMT 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

/*************************************************************************
 * This is really Sheelagh's code that I've hacked into a more usable form.
 * It is used by scaleim.c  All I did to it was to change "factor" from int to
 * float.
 *************************************************************************/
/************************************************************************
 *  This version uses the result of the previous row to influence the
 *  current row's calculation.
 ************************************************************************/

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "mfcpch.h"
#include <stdio.h>
#include <stdlib.h>
#include "errcode.h"

#define f(xc, yc) ((xc - factor*yc)*(xc - factor*yc))

#define g(oldyc, yc, oldxc, xc) (factor*factor*(oldyc - yc)*(oldyc - yc)/(abs(oldxc - xc) + 1))

void
dyn_exit (const char s[]) {
  fprintf (stderr, "%s", s);
  err_exit(); 
}


void dyn_prog(  //The clever bit
              int n,
              int *x,
              int *y,
              int ymax,
              int *oldx,
              int *oldy,
              int oldn,
              float factor) {
  int i, z, j, matchflag;
  int **ymin;
  float **F, fz;

  /* F[i][z] gives minimum over y <= z */

  F = (float **) calloc (n, sizeof (float *));
  ymin = (int **) calloc (n, sizeof (int *));
  if ((F == NULL) || (ymin == NULL))
    dyn_exit ("Error in calloc\n");

  for (i = 0; i < n; i++) {
    F[i] = (float *) calloc (ymax - n + i + 1, sizeof (float));
    ymin[i] = (int *) calloc (ymax - n + i + 1, sizeof (int));
    if ((F[i] == NULL) || (ymin[i] == NULL))
      dyn_exit ("Error in calloc\n");
  }

  F[0][0] = f (x[0], 0);
  /* find nearest transition of same sign (white to black) */
  j = 0;
  while ((j < oldn) && (oldx[j] < x[0]))
    j += 2;
  if (j >= oldn)
    j -= 2;
  else if ((j - 2 >= 0) && ((x[0] - oldx[j - 2]) < (oldx[j] - x[0])))
    j -= 2;
  if (abs (oldx[j] - x[0]) < factor) {
    matchflag = 1;
    F[0][0] += g (oldy[j], 0, oldx[j], x[0]);
  }
  else
    matchflag = 0;
  ymin[0][0] = 0;

  for (z = 1; z < ymax - n + 1; z++) {
    fz = f (x[0], z);
    /* add penalty for deviating from previous row if necessary */

    if (matchflag)
      fz += g (oldy[j], z, oldx[j], x[0]);
    if (fz < F[0][z - 1]) {
      F[0][z] = fz;
      ymin[0][z] = z;
    }
    else {
      F[0][z] = F[0][z - 1];
      ymin[0][z] = ymin[0][z - 1];
    }
  }

  for (i = 1; i < n; i++) {
    F[i][i] = f (x[i], i) + F[i - 1][i - 1];
    /* add penalty for deviating from previous row if necessary */
    if (j > 0)
      j--;
    else
      j++;
    while ((j < oldn) && (oldx[j] < x[i]))
      j += 2;
    if (j >= oldn)
      j -= 2;
    else if ((j - 2 >= 0) && ((x[i] - oldx[j - 2]) < (oldx[j] - x[i])))
      j -= 2;
    if (abs (oldx[j] - x[i]) < factor) {
      matchflag = 1;
      F[i][i] += g (oldy[j], i, oldx[j], x[i]);
    }
    else
      matchflag = 0;
    ymin[i][i] = i;
    for (z = i + 1; z < ymax - n + i + 1; z++) {
      fz = f (x[i], z) + F[i - 1][z - 1];
      /* add penalty for deviating from previous row if necessary */
      if (matchflag)
        fz += g (oldy[j], z, oldx[j], x[i]);
      if (fz < F[i][z - 1]) {
        F[i][z] = fz;
        ymin[i][z] = z;
      }
      else {
        F[i][z] = F[i][z - 1];
        ymin[i][z] = ymin[i][z - 1];
      }
    }
  }

  y[n - 1] = ymin[n - 1][ymax - 1];
  for (i = n - 2; i >= 0; i--)
    y[i] = ymin[i][y[i + 1] - 1];

  for (i = 0; i < n; i++) {
    free (F[i]);
    free (ymin[i]);
  }
  free(F); 
  free(ymin); 

  return;
}
