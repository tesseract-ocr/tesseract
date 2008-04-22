/**********************************************************************
 * File:        tessout.h  (Formerly tessconv.h)
 * Description: Code to convert from tesseract data to mithras data.
 * Author:		Ray Smith
 * Created:		Tue Oct 22 12:54:38 BST 1991
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

#ifndef           TESSOUT_H
#define           TESSOUT_H

#include          "ocrblock.h"
#include          "tessclas.h"
#include          "statistc.h"
#include          "notdll.h"

BOOL8 make_blocks_from_rows(                    //make thresholds
                            TEXTROW *tessrows,  //old style rows
                            const char *name,   //name of image
                            ICOORD page_tr,     //page size
                            BOOL8 copy_poly,    //true to copy poly
                            BLOCK_IT *block_it  //blocks to make
                           );
ROW *convert_row(                   //convert a row
                 TEXTROW *tessrow,  //row to convert
                 BOOL8 do_shift,    //true do do shift
                 inT16 &top,        //top of row
                 inT16 &bottom      //bottom of row
                );
void convert_words(                   //convert a row
                   TEXTROW *tessrow,  //row to convert
                   BOOL8 do_shift,    //true do do shift
                   ROW *row           //destination
                  );
PBLOB *convert_blob(                //convert a blob
                    TBLOB *tblob,   //blob to convert
                    BOOL8 do_shift  //true do do shift
                   );
void convert_outline(                     //convert a outline
                     TESSLINE *tessline,  //outline to convert
                     BOOL8 do_shift,      //true do do shift
                     BOOL8 reverse,       //reverse it
                     OUTLINE_IT *it       //output list
                    );
void accumulate_word_stats(                    //get stats
                           TWERD *word,        //word to do
                           STATS *kern_stats,  //kerning
                           ICOORD &bleft,      //corners
                           ICOORD &tright);
void blob_bounding_box(              //get bounding box
                       TBLOB *blob,  //blob to do
                       inT16 &xmin,  //bounding box
                       inT16 &ymin,
                       inT16 &xmax,  //of blob
                       inT16 &ymax);
void free_blob(             //free tess blob
               TBLOB *blob  //blob to free
              );
void free_tree(                   //free outlines
               TESSLINE *outline  //outlines to free
              );
void free_outline(                   //free one
                  TESSLINE *outline  //outline to free
                 );
#endif
