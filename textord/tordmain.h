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
#include          "params.h"
#include          "ocrblock.h"
#include          "blobs.h"
#include          "blobbox.h"

struct Pix;
namespace tesseract {
class Tesseract;
}

void make_blocks_from_blobs(                       //convert & textord
                            TBLOB *tessblobs,      //tess style input
                            const char *filename,  //blob file
                            ICOORD page_tr,        //top right
                            BOOL8 do_shift,        //shift tess coords
                            BLOCK_LIST *blocks     //block list
                           );
void SetBlobStrokeWidth(Pix* pix, BLOBNBOX* blob);
void assign_blobs_to_blocks2(Pix* pix, BLOCK_LIST *blocks,
                             TO_BLOCK_LIST *port_blocks);
void textord_page(                             //make rows & words
                  ICOORD page_tr,              //top right
                  BLOCK_LIST *blocks,          //block list
                  TO_BLOCK_LIST *land_blocks,  //rotated for landscape
                  TO_BLOCK_LIST *port_blocks,  //output list
                  tesseract::Tesseract*
                 );
void tweak_row_baseline(ROW *row,
                        double blshift_maxshift,
                        double blshift_xfraction);
inT32 blob_y_order(              //sort function
                   void *item1,  //items to compare
                   void *item2);
#endif
