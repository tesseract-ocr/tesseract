/**********************************************************************
 * File:        blread.h  (Formerly pdread.h)
 * Description: Friend function of BLOCK to read the uscan pd file.
 * Author:		Ray Smith
 * Created:		Mon Mar 18 14:39:00 GMT 1991
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

#ifndef           BLREAD_H
#define           BLREAD_H

#include          "varable.h"
#include          "ocrblock.h"

BOOL8 read_pd_file(                    //print list of sides
                   STRING name,        //basename of file
                   inT32 xsize,        //image size
                   inT32 ysize,        //image size
                   BLOCK_LIST *blocks  //output list
                  );
void get_pd_vertex(                //get new vertex
                   FILE *pdfp,     //file to read
                   inT32 xsize,    //image size
                   inT32 ysize,    //image size
                   TBOX *box,       //bounding box
                   inT32 &xcoord,  //output coords
                   inT32 &ycoord);
BOOL8 read_hpd_file(                    //print list of sides
                    STRING name,        //basename of file
                    inT32 xsize,        //image size
                    inT32 ysize,        //image size
                    BLOCK_LIST *blocks  //output list
                   );
void scan_hpd_blocks(                               //print list of sides
                     const char *name,              //block label
                     PAGE_BLOCK_LIST *page_blocks,  //head of full pag
                     inT32 &block_no,               //no of blocks
                     BLOCK_IT *block_it             //block iterator
                    );
BOOL8 read_vec_file(                    //print list of sides
                    STRING name,        //basename of file
                    inT32 xsize,        //image size
                    inT32 ysize,        //image size
                    BLOCK_LIST *blocks  //output list
                   );
BOOL8 read_unlv_file(                    //print list of sides
                     STRING name,        //basename of file
                     inT32 xsize,        //image size
                     inT32 ysize,        //image size
                     BLOCK_LIST *blocks  //output list
                    );
#endif
