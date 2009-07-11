/**********************************************************************
 * File:        tordmain.h  (Formerly textordp.h)
 * Description: C++ top level textord code.
 * Author:		Ray Smith
 * Created:		Tue Jul 28 17:12:33 BST 1992
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

#ifndef           TORDMAIN_H
#define           TORDMAIN_H

#include          <time.h>
#include          "varable.h"
#include          "ocrblock.h"
#include          "tessclas.h"
#include          "blobbox.h"
#include          "notdll.h"

namespace tesseract {
class Tesseract;
}

extern BOOL_VAR_H (textord_show_blobs, FALSE, "Display unsorted blobs");
extern BOOL_VAR_H (textord_new_initial_xheight, TRUE,
"Use test xheight mechanism");
extern BOOL_VAR_H (textord_exit_after, FALSE,
"Exit after completing textord");
extern INT_VAR_H (textord_max_noise_size, 7, "Pixel size of noise");
extern double_VAR_H (textord_blob_size_bigile, 95,
"Percentile for large blobs");
extern double_VAR_H (textord_noise_area_ratio, 0.7,
"Fraction of bounding box for noise");
extern double_VAR_H (textord_blob_size_smallile, 20,
"Percentile for small blobs");
extern double_VAR_H (textord_initialx_ile, 0.75,
"Ile of sizes for xheight guess");
extern double_VAR_H (textord_initialasc_ile, 0.90,
"Ile of sizes for xheight guess");
extern INT_VAR_H (textord_noise_sizefraction, 10,
"Fraction of size for maxima");
extern double_VAR_H (textord_noise_sizelimit, 0.5,
"Fraction of x for big t count");
extern INT_VAR_H (textord_noise_translimit, 16,
"Transitions for normal blob");
extern double_VAR_H (textord_noise_normratio, 2.0,
"Dot to norm ratio for deletion");
extern BOOL_VAR_H (textord_noise_rejwords, TRUE, "Reject noise-like words");
extern BOOL_VAR_H (textord_noise_rejrows, TRUE, "Reject noise-like rows");
extern double_VAR_H (textord_noise_syfract, 0.2,
"xh fract error for norm blobs");
extern double_VAR_H (textord_noise_sxfract, 0.4,
"xh fract width error for norm blobs");
extern INT_VAR_H (textord_noise_sncount, 1, "super norm blobs to save row");
extern double_VAR_H (textord_noise_rowratio, 6.0,
"Dot to norm ratio for deletion");
extern BOOL_VAR_H (textord_noise_debug, FALSE, "Debug row garbage detector");
extern double_VAR_H (textord_blshift_maxshift, 0.00, "Max baseline shift");
extern double_VAR_H (textord_blshift_xfraction, 9.99,
"Min size of baseline shift");
                                 //xiaofan
extern STRING_EVAR_H (tessedit_image_ext, ".tif", "Externsion for image file");
extern clock_t previous_cpu;
void make_blocks_from_blobs(                       //convert & textord
                            TBLOB *tessblobs,      //tess style input
                            const char *filename,  //blob file
                            ICOORD page_tr,        //top right
                            BOOL8 do_shift,        //shift tess coords
                            BLOCK_LIST *blocks     //block list
                           );
void find_components(  // find components in blocks
                       BLOCK_LIST *blocks,
                       TO_BLOCK_LIST *land_blocks,
                       TO_BLOCK_LIST *port_blocks,
                       TBOX *page_box);
void SetBlobStrokeWidth(bool debug, BLOBNBOX* blob);
void assign_blobs_to_blocks2(                             //split into groups
                             BLOCK_LIST *blocks,          //blocks to process
                             TO_BLOCK_LIST *land_blocks,  //rotated for landscape
                             TO_BLOCK_LIST *port_blocks   //output list
                            );
void filter_blobs(                        //split into groups
                  ICOORD page_tr,         //top right
                  TO_BLOCK_LIST *blocks,  //output list
                  BOOL8 testing_on        //for plotting
                 );
float filter_noise_blobs(                            //separate noise
                         BLOBNBOX_LIST *src_list,    //origonal list
                         BLOBNBOX_LIST *noise_list,  //noise list
                         BLOBNBOX_LIST *small_list,  //small blobs
                         BLOBNBOX_LIST *large_list   //large blobs
                        );
float filter_noise_blobs2(                            //separate noise
                          BLOBNBOX_LIST *src_list,    //origonal list
                          BLOBNBOX_LIST *noise_list,  //noise list
                          BLOBNBOX_LIST *small_list,  //small blobs
                          BLOBNBOX_LIST *large_list   //large blobs
                         );
void textord_page(                             //make rows & words
                  ICOORD page_tr,              //top right
                  BLOCK_LIST *blocks,          //block list
                  TO_BLOCK_LIST *land_blocks,  //rotated for landscape
                  TO_BLOCK_LIST *port_blocks,  //output list
                  tesseract::Tesseract*
                 );
void cleanup_blocks(                    //remove empties
                    BLOCK_LIST *blocks  //list
                   );
BOOL8 clean_noise_from_row(          //remove empties
                           ROW *row  //row to clean
                          );
void clean_noise_from_words(          //remove empties
                            ROW *row  //row to clean
                           );
// Remove outlines that are a tiny fraction in either width or height
// of the word height.
void clean_small_noise_from_words(ROW *row);
void tweak_row_baseline(          //remove empties
                        ROW *row  //row to clean
                       );
inT32 blob_y_order(              //sort function
                   void *item1,  //items to compare
                   void *item2);
#endif
