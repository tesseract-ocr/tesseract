/**********************************************************************
 * File:        charcut.h  (Formerly charclip.h)
 * Description: Code for character clipping
 * Author:      Phil Cheatle
 * Created:     Wed Nov 11 08:35:15 GMT 1992
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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
/**
 * @file     charcut.h  
 * @note     Formerly charclip.h
 * @brief    Code for character clipping
 * @author   Phil Cheatle
 * @date     Created Wed Nov 11 08:35:15 GMT 1992
 *
 */

#ifndef           CHARCUT_H
#define           CHARCUT_H

#include          "pgedit.h"
#include          "notdll.h"
#include          "notdll.h"
class ScrollView;

/**
 * @class PIXROW
 *
 * This class describes the pixels occupied by a blob. It uses two arrays, (min
 * and max), each with one element per row, to identify the min and max x
 * coordinates of the black pixels in the character on that row of the image.
 * The number of rows used to describe the blob is held in row_count - note that
 * some rows may be unoccupied - signified by max < min. The page coordinate of
 * the row defined by min[0] and max[0] is held in row_offset.
 */

class PIXROW:public ELIST_LINK
{
  public:
    inT16 row_offset;            ///< y coord of min[0]
    inT16 row_count;             ///< length of arrays
    inT16 *min;                  ///< array of min x
    inT16 *max;                  ///< array of max x
    /** empty constructor */
    PIXROW() {
      row_offset = 0;
      row_count = 0;
      min = NULL;
      max = NULL;
    }
    /** specified size */
    PIXROW(
           inT16 pos,
           inT16 count,
           PBLOB *blob);
    /** destructor */
    ~PIXROW () {
      if (min != NULL)
        free_mem(min);
      if (max != NULL)
        free_mem(max);
      max = NULL;
    }

    /** 
     * use current settings 
     * @param fd where to paint
     */
    void plot(ScrollView* fd) const;

    /** 
     * return bounding box
     * @return true if box exceeds image
     */
    TBOX bounding_box() const;

    bool bad_box(int xsize, int ysize) const;

    /**
     * force end on black
     * @param imlines image array
     * @param x_offset of pixels[0]
     * @param foreground_colour 0 or 1
     */
    void contract(
                  IMAGELINE *imlines,
                  inT16 x_offset,
                  inT16 foreground_colour);

    /**
     * @param imlines image array
     * @param imbox image box
     * @param prev for prev blob
     * @param next for next blob
     * @param foreground_colour 0 or 1
     */
    BOOL8 extend(IMAGELINE *imlines,
                 TBOX &imbox,
                 PIXROW *prev,
                 PIXROW *next,
                 inT16 foreground_colour);

    /**
     * @param imlines box of imlines extnt
     * @param imbox image box
	 * @param row row containing word
     * @param clip_image unscaled char image
     * @param baseline_pos baseline ht in image
     */
    void char_clip_image(IMAGELINE *imlines,
                         TBOX &im_box,
                         ROW *row,
                         IMAGE &clip_image,
                         float &baseline_pos);

};

ELISTIZEH (PIXROW)
extern INT_VAR_H (pix_word_margin, 3, "How far outside word BB to grow");
extern BOOL_VAR_H (show_char_clipping, TRUE, "Show clip image window?");
extern INT_VAR_H (net_image_width, 40, "NN input image width");
extern INT_VAR_H (net_image_height, 36, "NN input image height");
extern INT_VAR_H (net_image_x_height, 22, "NN input image x_height");
void char_clip_word(
                    WERD *word,                 ///< word to be processed
                    IMAGE &bin_image,           ///< whole image
                    PIXROW_LIST *&pixrow_list,  ///< pixrows built
                    IMAGELINE *&imlines,        ///< lines cut from image
                    TBOX &pix_box               ///< box defining imlines
                   );
/** get some imagelines */
IMAGELINE *generate_imlines(
                            IMAGE &bin_image,  ///< from here
                            TBOX &pix_box);
                                 
ScrollView* display_clip_image(WERD *word,           ///< word to be processed
                          IMAGE &bin_image,          ///< whole image
                          PIXROW_LIST *pixrow_list,  ///< pixrows built
                          TBOX &pix_box              ///< box of subimage
                         );
void display_images(IMAGE &clip_image, IMAGE &scaled_image);

/** plot for all blobs */
void plot_pixrows(
                  PIXROW_LIST *pixrow_list,
                  ScrollView* win);
#endif
