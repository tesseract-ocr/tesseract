/**********************************************************************
 * File:        fpchop.h  (Formerly fp_chop.h)
 * Description: Code to chop fixed pitch text into character cells.
 * Author:      Ray Smith
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifndef           FPCHOP_H
#define           FPCHOP_H

#include          "params.h"
#include          "blobbox.h"

class C_OUTLINE_FRAG:public ELIST_LINK
{
  public:
    C_OUTLINE_FRAG() {  //empty constructor
      steps = nullptr;
      stepcount = 0;
    }
    ~C_OUTLINE_FRAG () {
      delete [] steps;
    }
                                 //start coord
    C_OUTLINE_FRAG(ICOORD start_pt,
                   ICOORD end_pt,       //end coord
                   C_OUTLINE *outline,  //source of steps
                   int16_t start_index,
                   int16_t end_index);
                                 //other end
    C_OUTLINE_FRAG(C_OUTLINE_FRAG *head, int16_t tail_y);
    C_OUTLINE *close();  //copy to outline
    C_OUTLINE_FRAG & operator= ( //assign
      const C_OUTLINE_FRAG & src);

    ICOORD start;                //start coord
    ICOORD end;                  //end coord
    DIR128 *steps;                //step array
    int32_t stepcount;             //no of steps
    C_OUTLINE_FRAG *other_end;   //head if a tail
    int16_t ycoord;                //coord of cut pt

  private:
    // Copy constructor (currently unused, therefore private).
    C_OUTLINE_FRAG(const C_OUTLINE_FRAG& other);
};

ELISTIZEH(C_OUTLINE_FRAG)

extern
INT_VAR_H (textord_fp_chop_error, 2,
"Max allowed bending of chop cells");
extern
double_VAR_H (textord_fp_chop_snap, 0.5,
"Max distance of chop pt from vertex");
ROW *fixed_pitch_words(                 //find lines
                       TO_ROW *row,     //row to do
                       FCOORD rotation  //for drawing
                      );
WERD *add_repeated_word(                         //move repeated word
                        WERD_IT *rep_it,         //repeated words
                        int16_t &rep_left,         //left edge of word
                        int16_t &prev_chop_coord,  //previous word end
                        uint8_t &blanks,           //no of blanks
                        float pitch,             //char cell size
                        WERD_IT *word_it         //list of words
                       );
void split_to_blob(                                 //split the blob
                   BLOBNBOX *blob,                  //blob to split
                   int16_t chop_coord,                //place to chop
                   float pitch_error,               //allowed deviation
                   C_OUTLINE_LIST *left_coutlines,  //for cblobs
                   C_OUTLINE_LIST *right_coutlines);
void fixed_chop_cblob(                                //split the blob
                      C_BLOB *blob,                   //blob to split
                      int16_t chop_coord,               //place to chop
                      float pitch_error,              //allowed deviation
                      C_OUTLINE_LIST *left_outlines,  //left half of chop
                      C_OUTLINE_LIST *right_outlines  //right half of chop
                     );
void fixed_split_coutline(                        //chop the outline
                          C_OUTLINE *srcline,     //source outline
                          int16_t chop_coord,       //place to chop
                          float pitch_error,      //allowed deviation
                          C_OUTLINE_IT *left_it,  //left half of chop
                          C_OUTLINE_IT *right_it  //right half of chop
                         );
bool fixed_chop_coutline(                                  //chop the outline
        C_OUTLINE* srcline,               //source outline
        int16_t chop_coord,                 //place to chop
        float pitch_error,                //allowed deviation
        C_OUTLINE_FRAG_LIST* left_frags,  //left half of chop
        C_OUTLINE_FRAG_LIST* right_frags  //right half of chop
);
void save_chop_cfragment(                            //chop the outline
                         int16_t head_index,           //head of fragment
                         ICOORD head_pos,            //head of fragment
                         int16_t tail_index,           //tail of fragment
                         ICOORD tail_pos,            //tail of fragment
                         C_OUTLINE *srcline,         //source of edgesteps
                         C_OUTLINE_FRAG_LIST *frags  //fragment list
                        );
void add_frag_to_list(                            //ordered add
                      C_OUTLINE_FRAG *frag,       //fragment to add
                      C_OUTLINE_FRAG_LIST *frags  //fragment list
                     );
void close_chopped_cfragments(                             //chop the outline
                              C_OUTLINE_FRAG_LIST *frags,  //list to clear
                              C_OUTLINE_LIST *children,    //potential children
                              float pitch_error,           //allowed shrinkage
                              C_OUTLINE_IT *dest_it        //output list
                             );
C_OUTLINE *join_chopped_fragments(                         //join pieces
                                  C_OUTLINE_FRAG *bottom,  //bottom of cut
                                  C_OUTLINE_FRAG *top      //top of cut
                                 );
void join_segments(                         //join pieces
                   C_OUTLINE_FRAG *bottom,  //bottom of cut
                   C_OUTLINE_FRAG *top      //top of cut
                  );
#endif
