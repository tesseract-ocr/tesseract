/**********************************************************************
 * File:			wordit.c
 * Description: An iterator for passing over all the words in a document.
 * Author:		Ray Smith
 * Created:		Mon Apr 27 08:51:22 BST 1992
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

#ifndef           WERDIT_H
#define           WERDIT_H

#include          "varable.h"
#include          "ocrblock.h"
#include          "notdll.h"

class WERDIT
{
  public:
    WERDIT() {
    }                            //empty contructor
    WERDIT(                          //empty contructor
           BLOCK_LIST *blocklist) {  //blocks on page
      start_page(blocklist);  //ready to scan
    }

    void start_page(                         //get ready
                    BLOCK_LIST *blocklist);  //blocks on page

    WERD *forward();  //get next word
    WERD *next_word() {  //get next word
      return word_it.data ();    //already at next
    }
    ROW *row() {  //get current row
      return word_it.cycled_list ()? NULL : row_it.data ();
    }
    ROW *next_row() {  //get next row
      return row_it.data_relative (1);
    }
    BLOCK *block() {  //get current block
      return block_it.data ();
    }

  private:
    BLOCK_IT block_it;           //iterators
    ROW_IT row_it;
    WERD_IT word_it;
};

//extern BOOL_VAR_H(wordit_linearc,FALSE,"Pass poly of linearc to Tess");
WERD *make_pseudo_word(                         //make fake word
                       BLOCK_LIST *block_list,  //blocks to check //block of selection
                       TBOX &selection_box,
                       BLOCK *&pseudo_block,
                       ROW *&pseudo_row         //row of selection
                      );
#endif
