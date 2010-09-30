/**********************************************************************
 * File:        charcut.cpp  (Formerly charclip.c)
 * Description: Code for character clipping
 * Author:      Phil Cheatle
 * Created:     Wed Nov 11 08:35:15 GMT 1992
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

#include "mfcpch.h"
#include          "charcut.h"
#include          "imgs.h"
#include          "svshowim.h"
//#include          "evnts.h"
#include          "notdll.h"
#include	  "scrollview.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define LARGEST(a,b) ( (a) > (b) ? (a) : (b) )
#define SMALLEST(a,b) ( (a) > (b) ? (b) : (a) )
#define BUG_OFFSET 1
#define EXTERN

EXTERN INT_VAR (pix_word_margin, 3, "How far outside word BB to grow");

extern IMAGE page_image;

ELISTIZE (PIXROW)
/*************************************************************************
 * PIXROW::PIXROW()
 *
 * Constructor for a specified size PIXROW from a blob
 *************************************************************************/
PIXROW::PIXROW(inT16 pos, inT16 count, PBLOB *blob) {
  OUTLINE_LIST *outline_list;
  OUTLINE_IT outline_it;
  POLYPT_LIST *pts_list;
  POLYPT_IT pts_it;
  inT16 i;
  FCOORD pt;
  FCOORD vec;
  float y_coord;
  inT16 x_coord;

  row_offset = pos;
  row_count = count;
  min = (inT16 *) alloc_mem (count * sizeof (inT16));
  max = (inT16 *) alloc_mem (count * sizeof (inT16));
  outline_list = blob->out_list ();
  outline_it.set_to_list (outline_list);

  for (i = 0; i < count; i++) {
    min[i] = MAX_INT16 - 1;
    max[i] = -MAX_INT16 + 1;
    y_coord = row_offset + i + 0.5;
    for (outline_it.mark_cycle_pt ();
    !outline_it.cycled_list (); outline_it.forward ()) {
      pts_list = outline_it.data ()->polypts ();
      pts_it.set_to_list (pts_list);
      for (pts_it.mark_cycle_pt ();
      !pts_it.cycled_list (); pts_it.forward ()) {
        pt = pts_it.data ()->pos;
        vec = pts_it.data ()->vec;
        if ((vec.y () != 0) &&
          (((pt.y () <= y_coord) && (pt.y () + vec.y () >= y_coord))
          || ((pt.y () >= y_coord)
        && (pt.y () + vec.y () <= y_coord)))) {
          /* The segment crosses y_coord so find x-point and check for min/max. */
          x_coord = (inT16) floor ((y_coord -
            pt.y ()) * vec.x () / vec.y () +
            pt.x () + 0.5);
          if (x_coord < min[i])
            min[i] = x_coord;
          x_coord--;             //to get pix to left of line
          if (x_coord > max[i])
            max[i] = x_coord;
        }
      }
    }
  }
}


/*************************************************************************
 * PIXROW::plot()
 *
 * Draw the PIXROW
 *************************************************************************/

#ifndef GRAPHICS_DISABLED
void PIXROW::plot(ScrollView* fd  //where to paint
                 ) const {
  inT16 i;
  inT16 y_coord;

  for (i = 0; i < row_count; i++) {
    y_coord = row_offset + i;
    if (min[i] <= max[i]) {
      fd->Rectangle(min[i], y_coord, max[i] + 1, y_coord + 1);
    }
  }
}
#endif

/*************************************************************************
 * PIXROW::bounding_box()
 *
 * Generate bounding box for blob image
 *************************************************************************/

bool PIXROW::bad_box(  //return true if box exceeds image
                     int xsize,
                     int ysize) const {
  TBOX bbox = bounding_box ();
  if (bbox.left () < 0 || bbox.right () > xsize
  || bbox.top () > ysize || bbox.bottom () < 0) {
    tprintf("Box (%d,%d)->(%d,%d) bad compared to %d,%d\n",
            bbox.left(),bbox.bottom(), bbox.right(), bbox.top(),
            xsize, ysize);
    return true;
  }
  return false;
}


/*************************************************************************
 * PIXROW::bounding_box()
 *
 * Generate bounding box for blob image
 *************************************************************************/

TBOX PIXROW::bounding_box() const {
  inT16 i;
  inT16 y_coord;
  inT16 min_x = MAX_INT16 - 1;
  inT16 min_y = MAX_INT16 - 1;
  inT16 max_x = -MAX_INT16 + 1;
  inT16 max_y = -MAX_INT16 + 1;

  for (i = 0; i < row_count; i++) {
    y_coord = row_offset + i;
    if (min[i] <= max[i]) {
      if (y_coord < min_y)
        min_y = y_coord;
      if (y_coord + 1 > max_y)
        max_y = y_coord + 1;
      if (min[i] < min_x)
        min_x = min[i];
      if (max[i] + 1 > max_x)
        max_x = max[i] + 1;
    }
  }
  if (min_x > max_x || min_y > max_y)
    return TBOX ();
  else
    return TBOX (ICOORD (min_x, min_y), ICOORD (max_x, max_y));
}


/*************************************************************************
 * PIXROW::contract()
 *
 * Reduce the mins and maxs so that they end on black pixels
 *************************************************************************/

void PIXROW::contract(                         //image array
                      IMAGELINE *imlines,
                      inT16 x_offset,          //of pixels[0]
                      inT16 foreground_colour  //0 or 1
                     ) {
  inT16 i;
  uinT8 *line_pixels;

  for (i = 0; i < row_count; i++) {
    if (min[i] > max[i])
      continue;

    line_pixels = imlines[i].pixels;
    while (line_pixels[min[i] - x_offset] != foreground_colour) {
      if (min[i] == max[i]) {
        min[i] = MAX_INT16 - 1;
        max[i] = -MAX_INT16 + 1;
        goto nextline;
      }
      else
        min[i]++;
    }
    while (line_pixels[max[i] - x_offset] != foreground_colour) {
      if (min[i] == max[i]) {
        min[i] = MAX_INT16 - 1;
        max[i] = -MAX_INT16 + 1;
        goto nextline;
      }
      else
        max[i]--;
    }
    nextline:;
    //goto label!
  }
}


/*************************************************************************
 * PIXROW::extend()
 *
 * 1 pixel extension in each direction to cover extra black area
 *************************************************************************/

BOOL8 PIXROW::extend(               //image array
                     IMAGELINE *imlines,
                     TBOX &imbox,
                     PIXROW *prev,  //for prev blob
                     PIXROW *next,  //for next blob
                     inT16 foreground_colour) {
  inT16 i;
  inT16 x_offset = imbox.left ();
  inT16 limit;
  inT16 left_limit;
  inT16 right_limit;
  uinT8 *pixels = NULL;
  uinT8 *pixels_below = NULL;    //row below current
  uinT8 *pixels_above = NULL;    //row above current
  BOOL8 changed = FALSE;

  pixels_above = imlines[0].pixels;
  for (i = 0; i < row_count; i++) {
    pixels_below = pixels;
    pixels = pixels_above;
    if (i < (row_count - 1))
      pixels_above = imlines[i + 1].pixels;
    else
      pixels_above = NULL;

    /* Extend Left by one pixel*/
    if (prev == NULL || prev->max[i] < prev->min[i])
      limit = imbox.left ();
    else
      limit = prev->max[i] + 1;
    if ((min[i] <= max[i]) &&
      (min[i] > limit) &&
    (pixels[min[i] - 1 - x_offset] == foreground_colour)) {
      min[i]--;
      changed = TRUE;
    }

    /* Extend Right by one pixel*/
    if (next == NULL || next->min[i] > next->max[i])
      limit = imbox.right () - 1;//-1 to index inside pix
    else
      limit = next->min[i] - 1;
    if ((min[i] <= max[i]) &&
      (max[i] < limit) &&
    (pixels[max[i] + 1 - x_offset] == foreground_colour)) {
      max[i]++;
      changed = TRUE;
    }

    /* Extend down by one row */
    if (pixels_below != NULL) {
      if (min[i] < min[i - 1]) { //row goes left of row below
        if (prev == NULL || prev->max[i - 1] < prev->min[i - 1])
          left_limit = min[i];
        else
          left_limit = LARGEST (min[i], prev->max[i - 1] + 1);
      }
      else
        left_limit = min[i - 1];

      if (max[i] > max[i - 1]) { //row goes right of row below
        if (next == NULL || next->min[i - 1] > next->max[i - 1])
          right_limit = max[i];
        else
          right_limit = SMALLEST (max[i], next->min[i - 1] - 1);
      }
      else
        right_limit = max[i - 1];

      while ((left_limit <= right_limit) &&
        (pixels_below[left_limit - x_offset] != foreground_colour))
        left_limit++;            //find black extremity

      if ((left_limit <= right_limit) && (left_limit < min[i - 1])) {
        min[i - 1] = left_limit; //widen left if poss
        changed = TRUE;
      }

      while ((left_limit <= right_limit) &&
        (pixels_below[right_limit - x_offset] != foreground_colour))
        right_limit--;           //find black extremity

      if ((left_limit <= right_limit) && (right_limit > max[i - 1])) {
        max[i - 1] = right_limit;//widen right if poss
        changed = TRUE;
      }
    }

    /* Extend up by one row */
    if (pixels_above != NULL) {
      if (min[i] < min[i + 1]) { //row goes left of row above
        if (prev == NULL || prev->min[i + 1] > prev->max[i + 1])
          left_limit = min[i];
        else
          left_limit = LARGEST (min[i], prev->max[i + 1] + 1);
      }
      else
        left_limit = min[i + 1];

      if (max[i] > max[i + 1]) { //row goes right of row above
        if (next == NULL || next->min[i + 1] > next->max[i + 1])
          right_limit = max[i];
        else
          right_limit = SMALLEST (max[i], next->min[i + 1] - 1);
      }
      else
        right_limit = max[i + 1];

      while ((left_limit <= right_limit) &&
        (pixels_above[left_limit - x_offset] != foreground_colour))
        left_limit++;            //find black extremity

      if ((left_limit <= right_limit) && (left_limit < min[i + 1])) {
        min[i + 1] = left_limit; //widen left if poss
        changed = TRUE;
      }

      while ((left_limit <= right_limit) &&
        (pixels_above[right_limit - x_offset] != foreground_colour))
        right_limit--;           //find black extremity

      if ((left_limit <= right_limit) && (right_limit > max[i + 1])) {
        max[i + 1] = right_limit;//widen right if poss
        changed = TRUE;
      }
    }
  }
  return changed;
}


/*************************************************************************
 * PIXROW::char_clip_image()
 * Cut out a sub image for a character
 *************************************************************************/

void PIXROW::char_clip_image(                     //box of imlines extnt
                             IMAGELINE *imlines,
                             TBOX &im_box,
                             ROW *row,            //row containing word
                             IMAGE &clip_image,   //unscaled sq subimage
                             float &baseline_pos  //baseline ht in image
                            ) {
  inT16 clip_image_xsize;        //sub image x size
  inT16 clip_image_ysize;        //sub image y size
  inT16 x_shift;                 //from pixrow to subim
  inT16 y_shift;                 //from pixrow to subim
  TBOX char_pix_box;              //bbox of char pixels
  inT16 y_dest;
  inT16 x_min;
  inT16 x_max;
  inT16 x_min_dest;
  inT16 x_max_dest;
  inT16 x_width;
  inT16 y;

  clip_image_xsize = clip_image.get_xsize ();
  clip_image_ysize = clip_image.get_ysize ();

  char_pix_box = bounding_box ();
  /*
    The y shift is calculated by first finding the coord of the bottom of the
    image relative to the image lines. Then reducing this so by the amount
    relative to the clip image size, necessary to vertically position the
    character.
  */
  y_shift = char_pix_box.bottom () - row_offset -
    (inT16) floor ((clip_image_ysize - char_pix_box.height () + 0.5) / 2);

  /*
    The x_shift is the shift to be applied to the page coord in the pixrow to
    generate a centred char in the clip image.  Thus the left hand edge of the
    char is shifted to the margin width of the centred character.
  */
  x_shift = char_pix_box.left () -
    (inT16) floor ((clip_image_xsize - char_pix_box.width () + 0.5) / 2);

  for (y = 0; y < row_count; y++) {
    /*
      Check that there is something in this row of the source that will fit in the
      sub image.  If there is, reduce x range if necessary, then copy it
    */
    y_dest = y - y_shift;
    if ((min[y] <= max[y]) && (y_dest >= 0) && (y_dest < clip_image_ysize)) {
      x_min = min[y];
      x_min_dest = x_min - x_shift;
      if (x_min_dest < 0) {
        x_min = x_min - x_min_dest;
        x_min_dest = 0;
      }
      x_max = max[y];
      x_max_dest = x_max - x_shift;
      if (x_max_dest > clip_image_xsize - 1) {
        x_max = x_max - (x_max_dest - (clip_image_xsize - 1));
        x_max_dest = clip_image_xsize - 1;
      }
      x_width = x_max - x_min + 1;
      if (x_width > 0) {
        x_min -= im_box.left ();
                                 //offset pixel ptr
        imlines[y].pixels += x_min;
        clip_image.put_line (x_min_dest, y_dest, x_width, imlines + y,
          0);
        imlines[y].init ();      //reset pixel ptr
      }
    }
  }
  /*
    Baseline position relative to clip image: First find the baseline relative
    to the page origin at the x coord of the centre of the character. Then
    make this relative to the character bottom. Finally shift by the margin
    between the bottom of the character and the bottom of the clip image.
  */
  if (row == NULL)
    baseline_pos = 0;            //Not needed
  else
    baseline_pos = row->base_line ((char_pix_box.left () +
      char_pix_box.right ()) / 2.0)
      - char_pix_box.bottom ()
      + ((clip_image_ysize - char_pix_box.height ()) / 2);
}


/*************************************************************************
 * char_clip_word()
 *
 * Generate a PIXROW_LIST with one element for each blob in the word, together
 * with the image lines for the whole word.
 *************************************************************************/

void char_clip_word(                            //
                    WERD *word,                 //word to be processed
                    IMAGE &bin_image,           //whole image
                    PIXROW_LIST *&pixrow_list,  //pixrows built
                    IMAGELINE *&imlines,        //lines cut from image
                    TBOX &pix_box                //box defining imlines
                   ) {
  TBOX word_box = word->bounding_box ();
  PBLOB_LIST *blob_list;
  PBLOB_IT blob_it;
  PIXROW_IT pixrow_it;
  inT16 pix_offset;              //Y pos of pixrow[0]
  inT16 row_height;              //No of pix rows
  inT16 imlines_x_offset;
  PIXROW *prev;
  PIXROW *next;
  PIXROW *current;
  BOOL8 changed;                 //still improving
  BOOL8 just_changed;            //still improving
  inT16 iteration_count = 0;
  inT16 foreground_colour;

  if (word->flag (W_INVERSE))
    foreground_colour = 1;
  else
    foreground_colour = 0;

  /* Define region for max pixrow expansion */
  pix_box = word_box;
  pix_box.move_bottom_edge (-pix_word_margin);
  pix_box.move_top_edge (pix_word_margin);
  pix_box.move_left_edge (-pix_word_margin);
  pix_box.move_right_edge (pix_word_margin);
  pix_box -= TBOX (ICOORD (0, 0 + BUG_OFFSET),
    ICOORD (bin_image.get_xsize (),
    bin_image.get_ysize () - BUG_OFFSET));

  /* Generate pixrows list */

  pix_offset = pix_box.bottom ();
  row_height = pix_box.height ();
  blob_list = word->blob_list ();
  blob_it.set_to_list (blob_list);

  pixrow_list = new PIXROW_LIST;
  pixrow_it.set_to_list (pixrow_list);

  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
    PIXROW *row = new PIXROW (pix_offset, row_height, blob_it.data ());
    ASSERT_HOST (!row->
      bad_box (bin_image.get_xsize (), bin_image.get_ysize ()));
    pixrow_it.add_after_then_move (row);
  }

  imlines = generate_imlines (bin_image, pix_box);

  /* Contract pixrows - shrink min and max back to black pixels */

  imlines_x_offset = pix_box.left ();

  pixrow_it.move_to_first ();
  for (pixrow_it.mark_cycle_pt ();
  !pixrow_it.cycled_list (); pixrow_it.forward ()) {
    ASSERT_HOST (!pixrow_it.data ()->
      bad_box (bin_image.get_xsize (), bin_image.get_ysize ()));
    pixrow_it.data ()->contract (imlines, imlines_x_offset,
      foreground_colour);
    ASSERT_HOST (!pixrow_it.data ()->
      bad_box (bin_image.get_xsize (), bin_image.get_ysize ()));
  }

  /* Expand pixrows iteratively 1 pixel at a time */
  do {
    changed = FALSE;
    pixrow_it.move_to_first ();
    prev = NULL;
    current = NULL;
    next = pixrow_it.data ();
    for (pixrow_it.mark_cycle_pt ();
    !pixrow_it.cycled_list (); pixrow_it.forward ()) {
      prev = current;
      current = next;
      if (pixrow_it.at_last ())
        next = NULL;
      else
        next = pixrow_it.data_relative (1);
      just_changed = current->extend (imlines, pix_box, prev, next,
        foreground_colour);
      ASSERT_HOST (!current->
        bad_box (bin_image.get_xsize (),
        bin_image.get_ysize ()));
      changed = changed || just_changed;
    }
    iteration_count++;
  }
  while (changed);
}


/*************************************************************************
 * generate_imlines()
 * Get an array of IMAGELINES  holding a portion of an image
 *************************************************************************/

IMAGELINE *generate_imlines(                   //get some imagelines
                            IMAGE &bin_image,  //from here
                            TBOX &pix_box) {
  IMAGELINE *imlines;            //array of lines
  int i;

  imlines = new IMAGELINE[pix_box.height ()];
  for (i = 0; i < pix_box.height (); i++) {
    imlines[i].init (pix_box.width ());
                                 //coord to start at
    bin_image.fast_get_line (pix_box.left (),
      pix_box.bottom () + i + BUG_OFFSET,
    //line to get
      pix_box.width (),          //width to get
      imlines + i);              //dest imline
  }
  return imlines;
}


/*************************************************************************
 * display_clip_image()
 * All the boring user interface bits to let you see what's going on
 *************************************************************************/

#ifndef GRAPHICS_DISABLED
ScrollView* display_clip_image(WERD *word,                //word to be processed
                          IMAGE &bin_image,          //whole image
                          PIXROW_LIST *pixrow_list,  //pixrows built
                          TBOX &pix_box               //box of subimage
                         ) {
  ScrollView* clip_window;            //window for debug
  TBOX word_box = word->bounding_box ();
  int border = word_box.height () / 2;
  TBOX display_box = word_box;

  display_box.move_bottom_edge (-border);
  display_box.move_top_edge (border);
  display_box.move_left_edge (-border);
  display_box.move_right_edge (border);
  display_box -= TBOX (ICOORD (0, 0 - BUG_OFFSET),
    ICOORD (bin_image.get_xsize (),
    bin_image.get_ysize () - BUG_OFFSET));

  pgeditor_msg ("Creating Clip window...");
  clip_window = new ScrollView("Clipped Blobs",
    editor_word_xpos, editor_word_ypos,
    3 * (word_box.width () + 2 * border),
    3 * (word_box.height () + 2 * border),
    display_box.left () + display_box.right (),
    display_box.bottom () - BUG_OFFSET +
    display_box.top () - BUG_OFFSET,
    true);
  // ymin, ymax
  pgeditor_msg ("Creating Clip window...Done");

  clip_window->Clear();
  sv_show_sub_image (&bin_image,
    display_box.left (),
    display_box.bottom (),
    display_box.width (),
    display_box.height (),
    clip_window,
    display_box.left (), display_box.bottom () - BUG_OFFSET);

  word->plot (clip_window, ScrollView::RED);
  word_box.plot (clip_window, ScrollView::BLUE, ScrollView::BLUE);
  pix_box.plot (clip_window, ScrollView::BLUE, ScrollView::BLUE);
  plot_pixrows(pixrow_list, clip_window);
  return clip_window;
}


/*************************************************************************
 * display_images()
 * Show a pair of clip and scaled character images and wait for key before
 * continuing.
 *************************************************************************/

void display_images(IMAGE &clip_image, IMAGE &scaled_image) {
  ScrollView* clip_im_window;         //window for debug
  ScrollView* scale_im_window;        //window for debug
  inT16 i;

                                 // xmin xmax ymin ymax
  clip_im_window = new ScrollView ("Clipped Blob", editor_word_xpos - 20,
      editor_word_ypos - 100, 5 * clip_image.get_xsize (),
      5 * clip_image.get_ysize (), clip_image.get_xsize (),
      clip_image.get_ysize (), true);

  sv_show_sub_image (&clip_image,
    0, 0,
    clip_image.get_xsize (), clip_image.get_ysize (),
    clip_im_window, 0, 0);

  clip_im_window->Pen(255,0,0);
  for (i = 1; i < clip_image.get_xsize (); i++) {
    clip_im_window->SetCursor(i,0);
    clip_im_window->DrawTo(i, clip_image.get_xsize ());
  }
  for (i = 1; i < clip_image.get_ysize (); i++) {
    clip_im_window->SetCursor(0,i);
    clip_im_window->DrawTo(clip_image.get_xsize (),i);

  }

                                 // xmin xmax ymin ymax
  scale_im_window = new ScrollView ("Scaled Blob", editor_word_xpos + 300,
      editor_word_ypos - 100, 5 * scaled_image.get_xsize (),
      5 * scaled_image.get_ysize (), scaled_image.get_xsize (),
      scaled_image.get_ysize (), true);

  sv_show_sub_image (&scaled_image,
    0, 0,
    scaled_image.get_xsize (), scaled_image.get_ysize (),
    scale_im_window, 0, 0);

  scale_im_window->Pen(255,0,0);
  for (i = 1; i < scaled_image.get_xsize (); i++) {
    scale_im_window->SetCursor(i,0);
    scale_im_window->DrawTo(i, scaled_image.get_xsize ());
  }
  for (i = 1; i < scaled_image.get_ysize (); i++) {
    scale_im_window->SetCursor(0,i);
    scale_im_window->DrawTo(scaled_image.get_xsize (),i);
  }

  ScrollView::Update();
}


/*************************************************************************
 * plot_pixrows()
 * Display a list of pixrows
 *************************************************************************/

void plot_pixrows(  //plot for all blobs
                  PIXROW_LIST *pixrow_list,
                  ScrollView* win) {
  PIXROW_IT pixrow_it(pixrow_list);
  inT16 colour = ScrollView::RED;

  for (pixrow_it.mark_cycle_pt ();
  !pixrow_it.cycled_list (); pixrow_it.forward ()) {
    if (colour > ScrollView::RED + 7)
      colour = ScrollView::RED;

   win->Pen((ScrollView::Color) colour);
    pixrow_it.data ()->plot (win);
    colour++;
  }
}
#endif
