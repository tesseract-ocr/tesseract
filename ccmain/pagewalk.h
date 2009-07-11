/**********************************************************************
 * File:        pagewalk.h  (Formerly walkers.h)
 * Description: Structure processors
 * Author:      Phil Cheatle
 * Created:     Thu Oct 10 16:25:24 BST 1991
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

#ifndef           PAGEWALK_H
#define           PAGEWALK_H

#include          "ocrblock.h"
#include          "ocrrow.h"
#include          "werd.h"
#include          "polyblob.h"
#include          "stepblob.h"
#include          "rect.h"
#include          "varable.h"
#include          "notdll.h"
#include          "tesseractclass.h"

#define BLOCK_SPACING   20

extern BOOL_VAR_H (current_word_quit, FALSE, "Stop processing this word");
extern DLLSYM BOOL_VAR_H (selection_quit, FALSE,
"Stop processing this selection");
TBOX block_list_bounding_box(                        //find bounding box
                            BLOCK_LIST *block_list  //of this block list
                           );
const TBOX block_list_compress(  //shuffle up blocks
                              BLOCK_LIST *block_list);
void block_list_move(                         //move
                     BLOCK_LIST *block_list,  //this list
                     ICOORD vec               //by this vector
                    );
int block_name_order(                      //sort blocks
                     const void *block1p,  //ptr to ptr to block1
                     const void *block2p   //ptr to ptr to block2
                    );
void process_all_blobs (         //process blobs
BLOCK_LIST * block_list,         //blocks to check
BOOL8 blob_processor (           //function to call
                                 //function to call
BLOCK *, ROW *, WERD *, PBLOB *), BOOL8 c_blob_processor (
BLOCK
*,
ROW
*,
WERD
*,
C_BLOB
*));
void process_selected_blobs (    //process blobs
BLOCK_LIST * block_list,         //blocks to check
                                 //function to call
TBOX & selection_box, BOOL8 blob_processor (
                                 //function to call
BLOCK *, ROW *, WERD *, PBLOB *), BOOL8 c_blob_processor (
BLOCK
*,
ROW
*,
WERD
*,
C_BLOB
*));
void process_all_words (         //process words
BLOCK_LIST * block_list,         //blocks to check
BOOL8 word_processor (           //function to call
BLOCK *, ROW *, WERD *));
void process_selected_words (    //process words
BLOCK_LIST * block_list,         //blocks to check
                                 //function to call
TBOX & selection_box, BOOL8 word_processor (
BLOCK
*,
ROW
*,
WERD
*));

void process_all_words_it (      //process words
BLOCK_LIST * block_list,         //blocks to check
BOOL8 word_processor (           //function to call
BLOCK *,
ROW *,
WERD *,
BLOCK_IT &,
ROW_IT &, WERD_IT &));
void process_selected_words_it ( //process words
BLOCK_LIST * block_list,         //blocks to check
                                 //function to call
TBOX & selection_box, BOOL8 word_processor (
BLOCK
*,
ROW
*,
WERD
*,
BLOCK_IT
&,
ROW_IT
&,
WERD_IT
&));
void process_all_blocks (        //process blocks
BLOCK_LIST * block_list,         //blocks to check
BOOL8 block_processor (          //function to call
BLOCK *));
void process_selected_blocks (   //process blocks
BLOCK_LIST * block_list,         //blocks to check
                                 //function to call
TBOX & selection_box, BOOL8 block_processor (
BLOCK
*));
void process_all_rows (          //process words
BLOCK_LIST * block_list,         //blocks to check
BOOL8 row_processor (            //function to call
BLOCK *, ROW *));
void process_selected_rows (     //process rows
BLOCK_LIST * block_list,         //blocks to check
                                 //function to call
TBOX & selection_box, BOOL8 row_processor (
BLOCK
*,
ROW
*));
void process_all_rows_it (       //process words
BLOCK_LIST * block_list,         //blocks to check
BOOL8 row_processor (            //function to call
BLOCK *,
ROW *,
BLOCK_IT &, ROW_IT &));
void process_selected_rows_it (  //process rows
BLOCK_LIST * block_list,         //blocks to check
                                 //function to call
TBOX & selection_box, BOOL8 row_processor (
BLOCK
*,
ROW
*,
BLOCK_IT
&,
ROW_IT
&));
#endif
