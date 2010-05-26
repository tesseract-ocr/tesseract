/**********************************************************************
 * File:        scaleimg.cpp  (Formerly scaleim.c)
 * Description: Smart scaling of images.
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
 * You simply call scale_image() passing in source and target images. The target
 * image should be empty, but created - in order to define the destination
 * size.
 *************************************************************************/

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

#include "mfcpch.h"
#include          <stdlib.h>
#include          <string.h>
#include          "fileerr.h"
#include          "tprintf.h"
//#include          "grphics.h"
#include          "img.h"
//#include                                      "basefile.h"
#include          "imgscale.h"
#include          "scaleimg.h"

void scale_image(                     //scale an image
                 IMAGE &image,        //source image
                 IMAGE &target_image  //target image
                ) {
  inT32 xsize, ysize, new_xsize, new_ysize;
  IMAGELINE line, new_line;
  int *hires, *lores, *oldhires, *oldlores;
  int i, j, n, oldn, row, col;
  int offset = 0;                //not used here
  float factor;
  uinT8 curr_colour, new_colour;
  int dummy = -1;
  IMAGE image2;                  //horiz scaled image

  xsize = image.get_xsize ();
  ysize = image.get_ysize ();
  new_xsize = target_image.get_xsize ();
  new_ysize = target_image.get_ysize ();
  if (new_ysize > new_xsize)
    new_line.init (new_ysize);
  else
    new_line.init (new_xsize);

  factor = (float) xsize / (float) new_xsize;

  hires = (int *) calloc (xsize, sizeof (int));
  lores = (int *) calloc (new_xsize, sizeof (int));
  oldhires = (int *) calloc (xsize, sizeof (int));
  oldlores = (int *) calloc (new_xsize, sizeof (int));
  if ((hires == NULL) || (lores == NULL) || (oldhires == NULL)
  || (oldlores == NULL)) {
    fprintf (stderr, "Calloc error in scale_image\n");
    err_exit();
  }

  image2.create (new_xsize, ysize, image.get_bpp ());

  oldn = 0;
  /* do first row separately because hires[col-1] doesn't make sense here */
  image.fast_get_line (0, 0, xsize, &line);
  /* each line nominally begins with white */
  curr_colour = 1;
  n = 0;
  for (i = 0; i < xsize; i++) {
    new_colour = *(line.pixels + i);
    if (new_colour != curr_colour) {
      hires[n] = i;
      n++;
      curr_colour = new_colour;
    }
  }
  if (offset != 0)
    for (i = 0; i < n; i++)
      hires[i] += offset;

  if (n > new_xsize) {
    tprintf ("Too many transitions (%d) on line 0\n", n);
    scale_image_cop_out(image,
                        target_image,
                        factor,
                        hires,
                        lores,
                        oldhires,
                        oldlores);
    return;
  }
  else if (n > 0)
    dyn_prog (n, hires, lores, new_xsize, &dummy, &dummy, 0, factor);
  else
    lores[0] = new_xsize;

  curr_colour = 1;
  j = 0;
  for (i = 0; i < new_xsize; i++) {
    if (lores[j] == i) {
      curr_colour = 1 - curr_colour;
      j++;
    }
    *(new_line.pixels + i) = curr_colour;
  }
  image2.put_line (0, 0, new_xsize, &new_line, 0);

  for (i = 0; i < n; i++) {
    oldhires[i] = hires[i];
    oldlores[i] = lores[i];
  }

  for (i = n; i < oldn; i++) {
    oldhires[i] = 0;
    oldlores[i] = 0;
  }
  oldn = n;

  for (row = 1; row < ysize; row++) {
    image.fast_get_line (0, row, xsize, &line);
    /* each line nominally begins with white */
    curr_colour = 1;
    n = 0;
    for (i = 0; i < xsize; i++) {
      new_colour = *(line.pixels + i);
      if (new_colour != curr_colour) {
        hires[n] = i;
        n++;
        curr_colour = new_colour;
      }
    }
    for (i = n; i < oldn; i++) {
      hires[i] = 0;
      lores[i] = 0;
    }
    if (offset != 0)
      for (i = 0; i < n; i++)
        hires[i] += offset;

    if (n > new_xsize) {
      tprintf ("Too many transitions (%d) on line %d\n", n, row);
      scale_image_cop_out(image,
                          target_image,
                          factor,
                          hires,
                          lores,
                          oldhires,
                          oldlores);
      return;
    }
    else if (n > 0)
      dyn_prog(n, hires, lores, new_xsize, oldhires, oldlores, oldn, factor);
    else
      lores[0] = new_xsize;

    curr_colour = 1;
    j = 0;
    for (i = 0; i < new_xsize; i++) {
      if (lores[j] == i) {
        curr_colour = 1 - curr_colour;
        j++;
      }
      *(new_line.pixels + i) = curr_colour;
    }
    image2.put_line (0, row, new_xsize, &new_line, 0);

    for (i = 0; i < n; i++) {
      oldhires[i] = hires[i];
      oldlores[i] = lores[i];
    }
    for (i = n; i < oldn; i++) {
      oldhires[i] = 0;
      oldlores[i] = 0;
    }
    oldn = n;
  }

  free(hires);
  free(lores);
  free(oldhires);
  free(oldlores);

  /* NOW DO THE VERTICAL SCALING from image2 to target_image*/

  xsize = new_xsize;
  factor = (float) ysize / (float) new_ysize;
  offset = 0;

  hires = (int *) calloc (ysize, sizeof (int));
  lores = (int *) calloc (new_ysize, sizeof (int));
  oldhires = (int *) calloc (ysize, sizeof (int));
  oldlores = (int *) calloc (new_ysize, sizeof (int));
  if ((hires == NULL) || (lores == NULL) || (oldhires == NULL)
  || (oldlores == NULL)) {
    fprintf (stderr, "Calloc error in scale_image (vert)\n");
    err_exit();
  }

  oldn = 0;
  /* do first col separately because hires[col-1] doesn't make sense here */
  image2.get_column (0, 0, ysize, &line, 0);
  /* each line nominally begins with white */
  curr_colour = 1;
  n = 0;
  for (i = 0; i < ysize; i++) {
    new_colour = *(line.pixels + i);
    if (new_colour != curr_colour) {
      hires[n] = i;
      n++;
      curr_colour = new_colour;
    }
  }

  if (offset != 0)
    for (i = 0; i < n; i++)
      hires[i] += offset;

  if (n > new_ysize) {
    tprintf ("Too many transitions (%d) on column 0\n", n);
    scale_image_cop_out(image,
                        target_image,
                        factor,
                        hires,
                        lores,
                        oldhires,
                        oldlores);
    return;
  }
  else if (n > 0)
    dyn_prog (n, hires, lores, new_ysize, &dummy, &dummy, 0, factor);
  else
    lores[0] = new_ysize;

  curr_colour = 1;
  j = 0;
  for (i = 0; i < new_ysize; i++) {
    if (lores[j] == i) {
      curr_colour = 1 - curr_colour;
      j++;
    }
    *(new_line.pixels + i) = curr_colour;
  }
  target_image.put_column (0, 0, new_ysize, &new_line, 0);

  for (i = 0; i < n; i++) {
    oldhires[i] = hires[i];
    oldlores[i] = lores[i];
  }
  for (i = n; i < oldn; i++) {
    oldhires[i] = 0;
    oldlores[i] = 0;
  }
  oldn = n;

  for (col = 1; col < xsize; col++) {
    image2.get_column (col, 0, ysize, &line, 0);
    /* each line nominally begins with white */
    curr_colour = 1;
    n = 0;
    for (i = 0; i < ysize; i++) {
      new_colour = *(line.pixels + i);
      if (new_colour != curr_colour) {
        hires[n] = i;
        n++;
        curr_colour = new_colour;
      }
    }
    for (i = n; i < oldn; i++) {
      hires[i] = 0;
      lores[i] = 0;
    }

    if (offset != 0)
      for (i = 0; i < n; i++)
        hires[i] += offset;

    if (n > new_ysize) {
      tprintf ("Too many transitions (%d) on column %d\n", n, col);
      scale_image_cop_out(image,
                          target_image,
                          factor,
                          hires,
                          lores,
                          oldhires,
                          oldlores);
      return;
    }
    else if (n > 0)
      dyn_prog(n, hires, lores, new_ysize, oldhires, oldlores, oldn, factor);
    else
      lores[0] = new_ysize;

    curr_colour = 1;
    j = 0;
    for (i = 0; i < new_ysize; i++) {
      if (lores[j] == i) {
        curr_colour = 1 - curr_colour;
        j++;
      }
      *(new_line.pixels + i) = curr_colour;
    }
    target_image.put_column (col, 0, new_ysize, &new_line, 0);

    for (i = 0; i < n; i++) {
      oldhires[i] = hires[i];
      oldlores[i] = lores[i];
    }
    for (i = n; i < oldn; i++) {
      oldhires[i] = 0;
      oldlores[i] = 0;
    }
    oldn = n;
  }
  free(hires);
  free(lores);
  free(oldhires);
  free(oldlores);
}


/**********************************************************************
 * scale_image_cop_out
 *
 * Cop-out of scale_image by doing it the easy way and free the data.
 **********************************************************************/

void scale_image_cop_out(                      //scale an image
                         IMAGE &image,         //source image
                         IMAGE &target_image,  //target image
                         float factor,         //scale factor
                         int *hires,
                         int *lores,
                         int *oldhires,
                         int *oldlores) {
  inT32 xsize, ysize, new_xsize, new_ysize;

  xsize = image.get_xsize ();
  ysize = image.get_ysize ();
  new_xsize = target_image.get_xsize ();
  new_ysize = target_image.get_ysize ();

  if (factor <= 0.5)
    reduce_sub_image (&image, 0, 0, xsize, ysize,
      &target_image, 0, 0, (inT32) (1.0 / factor), FALSE);
  else if (factor >= 2)
    enlarge_sub_image (&image, 0, 0, &target_image,
        0, 0, new_xsize, new_ysize, (inT32) factor, FALSE);
  else
    copy_sub_image (&image, 0, 0, xsize, ysize, &target_image, 0, 0, FALSE);
  free(hires);
  free(lores);
  free(oldhires);
  free(oldlores);
}
