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

#ifndef           CHARCUT_H
#define           CHARCUT_H

#include          "pgedit.h"
#include          "notdll.h"
#include          "notdll.h"
class ScrollView;

/*************************************************************************
 * CLASS PIXROW
 *
 * This class describes the pixels occupied by a blob. It uses two arrays, (min
 * and max), each with one element per row, to identify the min and max x
 * coordinates of the black pixels in the character on that row of the image.
 * The number of rows used to describe the blob is held in row_count - note that
 * some rows may be unoccupied - signified by max < min. The page coordinate of
 * the row defined by min[0] and max[0] is held in row_offset.
 *************************************************************************/

class PIXROW:public ELIST_LINK
{
  public:
    inT16 row_offset;            //y coord of min[0]
    inT16 row_count;             //length of arrays
    inT16 *min;                  //array of min x
    inT16 *max;                  //array of max x

    PIXROW() {  //empty constructor
      row_offset = 0;
      row_count = 0;
      min = NULL;
      max = NULL;
    }
    PIXROW(  //specified size
           inT16 pos,
           inT16 count,
           PBLOB *blob);

    ~PIXROW () {                 //destructor
      if (min != NULL)
        free_mem(min);
      if (max != NULL)
        free_mem(max);
      max = NULL;
    }

    void plot(                   //use current settings
              ScrollView* fd) const;  //where to paint

    TBOX bounding_box() const;  //return bounding box
                                 //return true if box exceeds image
    bool bad_box(int xsize, int ysize) const;

    void contract(                           //force end on black
                  IMAGELINE *imlines,        //image array
                  inT16 x_offset,            //of pixels[0]
                  inT16 foreground_colour);  //0 or 1

                                 //image array
    BOOL8 extend(IMAGELINE *imlines,
                 TBOX &imbox,
                 PIXROW *prev,              //for prev blob
                 PIXROW *next,              //for next blob
                 inT16 foreground_colour);  //0 or 1

                                 //box of imlines extnt
    void char_clip_image(IMAGELINE *imlines,
                         TBOX &im_box,
                         ROW *row,              //row containing word
                         IMAGE &clip_image,     //unscaled char image
                         float &baseline_pos);  //baseline ht in image

};

ELISTIZEH (PIXROW)
extern INT_VAR_H (pix_word_margin, 3, "How far outside word BB to grow");
extern BOOL_VAR_H (show_char_clipping, TRUE, "Show clip image window?");
extern INT_VAR_H (net_image_width, 40, "NN input image width");
extern INT_VAR_H (net_image_height, 36, "NN input image height");
extern INT_VAR_H (net_image_x_height, 22, "NN input image x_height");
void char_clip_word(                            //
                    WERD *word,                 //word to be processed
                    IMAGE &bin_image,           //whole image
                    PIXROW_LIST *&pixrow_list,  //pixrows built
                    IMAGELINE *&imlines,        //lines cut from image
                    TBOX &pix_box                //box defining imlines
                   );
IMAGELINE *generate_imlines(                   //get some imagelines
                            IMAGE &bin_image,  //from here
                            TBOX &pix_box);
                                 //word to be processed
ScrollView* display_clip_image(WERD *word,
                          IMAGE &bin_image,          //whole image
                          PIXROW_LIST *pixrow_list,  //pixrows built
                          TBOX &pix_box               //box of subimage
                         );
void display_images(IMAGE &clip_image, IMAGE &scaled_image);
void plot_pixrows(  //plot for all blobs
                  PIXROW_LIST *pixrow_list,
                  ScrollView* win);
#endif
