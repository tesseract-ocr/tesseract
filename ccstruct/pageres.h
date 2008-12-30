/**********************************************************************
 * File:        pageres.h  (Formerly page_res.h)
 * Description: Results classes used by control.c
 * Author:		Phil Cheatle
 * Created:     Tue Sep 22 08:42:49 BST 1992
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
#ifndef           PAGERES_H
#define           PAGERES_H

#include          "elst.h"
#include          "ocrblock.h"
#include          "ocrrow.h"
#include          "werd.h"
#include          "ratngs.h"
#include          "rejctmap.h"
#include          "notdll.h"
#include          "notdll.h"

/* Forward declarations */

class BLOCK_RES;

ELISTIZEH (BLOCK_RES) CLISTIZEH (BLOCK_RES)
class
ROW_RES;

ELISTIZEH (ROW_RES)
class WERD_RES;

ELISTIZEH (WERD_RES)
/*************************************************************************
 * PAGE_RES - Page results
 *************************************************************************/
class PAGE_RES                   //page result
{
  public:
    inT32 char_count;
    inT32 rej_count;
    BLOCK_RES_LIST block_res_list;
    BOOL8 rejected;

    PAGE_RES() {
    }                            //empty constructor

    PAGE_RES(                          //simple constructor
             BLOCK_LIST *block_list);  //real blocks

    ~PAGE_RES () {               //destructor
    }
};

/*************************************************************************
 * BLOCK_RES - Block results
 *************************************************************************/

class BLOCK_RES:public ELIST_LINK
                                 //page block result
{
  public:
    BLOCK * block;               //real block
    inT32 char_count;            //chars in block
    inT32 rej_count;             //rejected chars
    inT16 font_class;            //
    inT16 row_count;
    float x_height;
    BOOL8 font_assigned;         // block already
    //      processed
    BOOL8 bold;                  // all bold
    BOOL8 italic;                // all italic

    ROW_RES_LIST row_res_list;

    BLOCK_RES() {
    }                            //empty constructor

    BLOCK_RES(                    //simple constructor
              BLOCK *the_block);  //real block

    ~BLOCK_RES () {              //destructor
    }
};

/*************************************************************************
 * ROW_RES - Row results
 *************************************************************************/

class ROW_RES:public ELIST_LINK  //row result
{
  public:
    ROW * row;                   //real row
    inT32 char_count;            //chars in block
    inT32 rej_count;             //rejected chars
    inT32 whole_word_rej_count;  //rejs in total rej wds
    WERD_RES_LIST word_res_list;
    float font_class_score;
    inT16 font_class;            //
    inT32 italic;
    inT32 bold;
    inT8 font1;                  //primary font
    inT8 font1_count;            //no of voters
    inT8 font2;                  //secondary font
    inT8 font2_count;            //no of voters

    ROW_RES() {
    }                            //empty constructor

    ROW_RES(                //simple constructor
            ROW *the_row);  //real row

    ~ROW_RES () {                //destructor
    }
};

/*************************************************************************
 * WERD_RES - Word results
 *************************************************************************/
enum CRUNCH_MODE
{
  CR_NONE,
  CR_KEEP_SPACE,
  CR_LOOSE_SPACE,
  CR_DELETE
};

class WERD_RES:public ELIST_LINK //word result
{
  public:
    WERD * word;                 //non-bln real word
    WERD *outword;               //bln best choice
    //segmentation
    DENORM denorm;               //for use on outword
    WERD_CHOICE *best_choice;    //tess output
    WERD_CHOICE *raw_choice;     //top choice permuter
    WERD_CHOICE *ep_choice;      //ep text
    REJMAP reject_map;           //best_choice rejects
    BOOL8 tess_failed;
    /*
      If tess_failed is TRUE, one of the following tests failed when Tess
      returned:
      - The outword blob list was not the same length as the best_choice string;
      - The best_choice string contained ALL blanks;
      - The best_choice string was zero length
    */
    BOOL8 tess_accepted;         //Tess thinks its ok?
    BOOL8 tess_would_adapt;      //Tess would adapt?
    BOOL8 done;                  //ready for output?
    inT8 italic;
    inT8 bold;
    inT8 font1;                  //primary font
    inT8 font1_count;            //no of voters
    inT8 font2;                  //secondary font
    inT8 font2_count;            //no of voters
    CRUNCH_MODE unlv_crunch_mode;
    float x_height;              //Post match estimate
    float caps_height;           //Post match estimate
    BOOL8 guessed_x_ht;
    BOOL8 guessed_caps_ht;
    /*
      To deal with fuzzy spaces we need to be able to combine "words" to form
      combinations when we suspect that the gap is a non-space. The (new) text
      ord code generates separate words for EVERY fuzzy gap - flags in the word
      indicate whether the gap is below the threshold (fuzzy kern) and is thus
      NOT a real word break by default, or above the threshold (fuzzy space) and
      this is a real word break by default.

      The WERD_RES list contains all these words PLUS "combination" words built
      out of (copies of) the words split by fuzzy kerns. The separate parts have
      their "part_of_combo" flag set true and should be IGNORED on a default
      reading of the list.

      Combination words are FOLLOWED by the sequence of part_of_combo words
      which they combine.
    */
    BOOL8 combination;           //of two fuzzy gap wds
    BOOL8 part_of_combo;         //part of a combo
    BOOL8 reject_spaces;         //Reject spacing?

    WERD_RES() {
    }                            //empty constructor

    WERD_RES(                   //simple constructor
             WERD *the_word) {  //real word
      word = the_word;
      outword = NULL;
      best_choice = NULL;
      raw_choice = NULL;
      ep_choice = NULL;
      tess_failed = FALSE;
      tess_accepted = FALSE;
      tess_would_adapt = FALSE;
      done = FALSE;
      unlv_crunch_mode = CR_NONE;
      italic = FALSE;
      bold = FALSE;
      font1 = -1;
      font1_count = 0;
      font2 = -1;
      font2_count = 0;
      x_height = 0.0;
      caps_height = 0.0;
      guessed_x_ht = TRUE;
      guessed_caps_ht = TRUE;
      combination = FALSE;
      part_of_combo = FALSE;
      reject_spaces = FALSE;
    }
    WERD_RES(const WERD_RES &source) {
      *this = source;            //see operator=
    }

    ~WERD_RES ();                //destructor

    WERD_RES& operator=(const WERD_RES& source);  //from this

    static WERD_RES* deep_copy(const WERD_RES* src) {
      return new WERD_RES(*src);
    }

    void copy_on(                       //copy blobs onto word
                 WERD_RES *word_res) {  //from this word
      word->set_flag (W_EOL, word_res->word->flag (W_EOL));
      word->copy_on (word_res->word);
    }
};

/*************************************************************************
 * PAGE_RES_IT - Page results iterator
 *************************************************************************/

class PAGE_RES_IT
{
  public:
    PAGE_RES * page_res;         //page being iterated

    PAGE_RES_IT() {
    }                            //empty contructor

    PAGE_RES_IT(                           //empty contructor
                PAGE_RES *the_page_res) {  //page result
      page_res = the_page_res;
      restart_page();  //ready to scan
    }

    WERD_RES *restart_page();  //get ready

    WERD_RES *internal_forward(  //get next word
                               BOOL8 new_block);

    WERD_RES *forward() {  //get next word
      return internal_forward (FALSE);
    }

    WERD_RES *forward_block();  //get first word in
    //next non-empty block
    WERD_RES *prev_word() {  //previous word
      return prev_word_res;
    }
    ROW_RES *prev_row() {  //row of prev word
      return prev_row_res;
    }
    BLOCK_RES *prev_block() {  //block of prev word
      return prev_block_res;
    }
    WERD_RES *word() {  //current word
      return word_res;
    }
    ROW_RES *row() {  //row of current word
      return row_res;
    }
    BLOCK_RES *block() {  //block of cur. word
      return block_res;
    }
    WERD_RES *next_word() {  //next word
      return next_word_res;
    }
    ROW_RES *next_row() {  //row of next word
      return next_row_res;
    }
    BLOCK_RES *next_block() {  //block of next word
      return next_block_res;
    }
    void rej_stat_word();  //for page/block/row

  private:
    WERD_RES * prev_word_res;    //previous word
    ROW_RES *prev_row_res;       //row of prev word
    BLOCK_RES *prev_block_res;   //block of prev word

    WERD_RES *word_res;          //current word
    ROW_RES *row_res;            //row of current word
    BLOCK_RES *block_res;        //block of cur. word

    WERD_RES *next_word_res;     //next word
    ROW_RES *next_row_res;       //row of next word
    BLOCK_RES *next_block_res;   //block of next word

    BLOCK_RES_IT block_res_it;   //iterators
    ROW_RES_IT row_res_it;
    WERD_RES_IT word_res_it;
};
#endif
