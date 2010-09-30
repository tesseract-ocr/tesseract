/**********************************************************************
 * File:        fpchop.cpp  (Formerly fp_chop.c)
 * Description: Code to chop fixed pitch text into character cells.
 * Author:		Ray Smith
 * Created:		Thu Sep 16 11:14:15 BST 1993
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

#include "mfcpch.h"
#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "stderr.h"
#include          "blobbox.h"
#include          "lmedsq.h"
#include          "statistc.h"
#include          "drawtord.h"
#include          "tovars.h"
#include          "topitch.h"
#include          "fpchop.h"
#include          "notdll.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define EXTERN

EXTERN INT_VAR (textord_fp_chop_error, 2,
"Max allowed bending of chop cells");
EXTERN double_VAR (textord_fp_chop_snap, 0.5,
"Max distance of chop pt from vertex");

ELISTIZE (OUTLINE_FRAG) ELISTIZE (C_OUTLINE_FRAG)
//#undef ASSERT_HOST
//#define ASSERT_HOST(x) if (!(x)) AfxMessageBox(#x);
/**********************************************************************
 * fixed_pitch_words
 *
 * Make a ROW from a fixed pitch TO_ROW.
 **********************************************************************/
ROW *fixed_pitch_words(                 //find lines
                       TO_ROW *row,     //row to do
                       FCOORD rotation  //for drawing
                      ) {
  BOOL8 bol;                     //start of line
  uinT8 blanks;                  //in front of word
  uinT8 new_blanks;              //blanks in empty cell
  inT16 chop_coord;              //chop boundary
  inT16 prev_chop_coord;         //start of cell
  inT16 rep_left;                //left edge of rep word
  ROW *real_row;                 //output row
  OUTLINE_LIST left_outlines;    //in current blob
  OUTLINE_LIST right_outlines;   //for next blob
  C_OUTLINE_LIST left_coutlines;
  C_OUTLINE_LIST right_coutlines;
  PBLOB_LIST blobs;              //blobs in word
  C_BLOB_LIST cblobs;
  PBLOB_IT blob_it = &blobs;     //iterator
  C_BLOB_IT cblob_it = &cblobs;
  WERD_LIST words;
  WERD_IT word_it = &words;      //new words
                                 //repeated blobs
  WERD_IT rep_it = &row->rep_words;
  WERD *word;                    //new word
  inT32 xstarts[2];              //row ends
  double coeffs[3];              //quadratic
  inT32 prev_x;                  //end of prev blob
                                 //iterator
  BLOBNBOX_IT box_it = row->blob_list ();
                                 //boundaries
  ICOORDELT_IT cell_it = &row->char_cells;

#ifndef GRAPHICS_DISABLED
  if (textord_show_page_cuts && to_win != NULL) {
    plot_row_cells (to_win, ScrollView::RED, row, 0, &row->char_cells);
  }
#endif

  prev_x = -MAX_INT16;
  bol = TRUE;
  blanks = 0;
  if (rep_it.empty ())
    rep_left = MAX_INT16;
  else
    rep_left = rep_it.data ()->bounding_box ().left ();
  if (box_it.empty ())
    return NULL;                 //empty row
  xstarts[0] = box_it.data ()->bounding_box ().left ();
  if (rep_left < xstarts[0]) {
    xstarts[0] = rep_left;
  }
  if (cell_it.empty () || row->char_cells.singleton ()) {
    tprintf ("Row without enough char cells!\n");
    tprintf ("Leftmost blob is at (%d,%d)\n",
      box_it.data ()->bounding_box ().left (),
      box_it.data ()->bounding_box ().bottom ());
    return NULL;
  }
  ASSERT_HOST (!cell_it.empty () && !row->char_cells.singleton ());
  prev_chop_coord = cell_it.data ()->x ();
  word = NULL;
  while (rep_left < cell_it.data ()->x ()) {
    word = add_repeated_word (&rep_it, rep_left, prev_chop_coord,
      blanks, row->fixed_pitch, &word_it);
  }
  cell_it.mark_cycle_pt ();
  if (prev_chop_coord >= cell_it.data ()->x ())
    cell_it.forward ();
  for (; !cell_it.cycled_list (); cell_it.forward ()) {
    chop_coord = cell_it.data ()->x ();
    while (!box_it.empty ()
    && box_it.data ()->bounding_box ().left () <= chop_coord) {
      if (box_it.data ()->bounding_box ().right () > prev_x)
        prev_x = box_it.data ()->bounding_box ().right ();
      split_to_blob (box_it.extract (), chop_coord,
        textord_fp_chop_error + 0.5f,
        &left_outlines, &left_coutlines,
        &right_outlines, &right_coutlines);
      box_it.forward ();
      while (!box_it.empty ()
        && box_it.data ()->blob () == NULL
      && box_it.data ()->cblob () == NULL) {
        delete box_it.extract ();
        box_it.forward ();
      }
    }
    if ((!right_outlines.empty () || !right_coutlines.empty ())
      && left_outlines.empty () && left_coutlines.empty ())
      split_to_blob (NULL, chop_coord,
        textord_fp_chop_error + 0.5f,
        &left_outlines, &left_coutlines,
        &right_outlines, &right_coutlines);
    if (!left_outlines.empty ())
      blob_it.add_after_then_move (new PBLOB (&left_outlines));
    else if (!left_coutlines.empty ())
      cblob_it.add_after_then_move (new C_BLOB (&left_coutlines));
    else {
      if (rep_left < chop_coord) {
        if (rep_left > prev_chop_coord)
          new_blanks = (uinT8) floor ((rep_left - prev_chop_coord)
            / row->fixed_pitch + 0.5);
        else
          new_blanks = 0;
      }
      else {
        if (chop_coord > prev_chop_coord)
          new_blanks = (uinT8) floor ((chop_coord - prev_chop_coord)
            / row->fixed_pitch + 0.5);
        else
          new_blanks = 0;
      }
      if (!blob_it.empty () || !cblob_it.empty ()) {
        if (blanks < 1 && word != NULL && !word->flag (W_REP_CHAR))
          blanks = 1;
        if (!blob_it.empty ()) {
                                 //make real word
          word = new WERD (&blobs, blanks, NULL);
          blob_it.set_to_list (&blobs);
        }
        else {
          word = new WERD (&cblobs, blanks, NULL);
          cblob_it.set_to_list (&cblobs);
        }
        word->set_flag (W_DONT_CHOP, TRUE);
        word_it.add_after_then_move (word);
        if (bol) {
          word->set_flag (W_BOL, TRUE);
          bol = FALSE;
        }
        blanks = new_blanks;
      }
      else
        blanks += new_blanks;
      while (rep_left < chop_coord) {
        word = add_repeated_word (&rep_it, rep_left, prev_chop_coord,
          blanks, row->fixed_pitch, &word_it);
      }
    }
    if (prev_chop_coord < chop_coord)
      prev_chop_coord = chop_coord;
  }
  if (!blob_it.empty () || !cblob_it.empty ()) {
    if (!blob_it.empty ())
                                 //last word on line
      word = new WERD (&blobs, blanks, NULL);
    else
      word = new WERD (&cblobs, blanks, NULL);
    word->set_flag (W_DONT_CHOP, TRUE);
    word_it.add_after_then_move (word);
    if (bol)
      word->set_flag (W_BOL, TRUE);
  }
  ASSERT_HOST (word != NULL);
  while (!rep_it.empty ()) {
    add_repeated_word (&rep_it, rep_left, prev_chop_coord,
      blanks, row->fixed_pitch, &word_it);
  }
                                 //at end of line
  word_it.data ()->set_flag (W_EOL, TRUE);
  if (prev_chop_coord > prev_x)
    prev_x = prev_chop_coord;
  xstarts[1] = prev_x + 1;
  coeffs[0] = 0;
  coeffs[1] = row->line_m ();
  coeffs[2] = row->line_c ();
  real_row = new ROW (row, (inT16) row->kern_size, (inT16) row->space_size);
  word_it.set_to_list (real_row->word_list ());
                                 //put words in row
  word_it.add_list_after (&words);
  real_row->recalc_bounding_box ();
  return real_row;
}


/**********************************************************************
 * add_repeated_word
 *
 * Add repeated word into the row at the given point.
 **********************************************************************/

WERD *add_repeated_word(                         //move repeated word
                        WERD_IT *rep_it,         //repeated words
                        inT16 &rep_left,         //left edge of word
                        inT16 &prev_chop_coord,  //previous word end
                        uinT8 &blanks,           //no of blanks
                        float pitch,             //char cell size
                        WERD_IT *word_it         //list of words
                       ) {
  WERD *word;                    //word to move
  inT16 new_blanks;              //extra blanks

  if (rep_left > prev_chop_coord) {
    new_blanks = (uinT8) floor ((rep_left - prev_chop_coord) / pitch + 0.5);
    blanks += new_blanks;
  }
  word = rep_it->extract ();
  prev_chop_coord = word->bounding_box ().right ();
  word_it->add_after_then_move (word);
  word->set_blanks (blanks);
  rep_it->forward ();
  if (rep_it->empty ())
    rep_left = MAX_INT16;
  else
    rep_left = rep_it->data ()->bounding_box ().left ();
  blanks = 0;
  return word;
}


/**********************************************************************
 * split_to_blob
 *
 * Split a BLOBNBOX across a vertical chop line and put the pieces
 * into a left outline list and a right outline list.
 **********************************************************************/

void split_to_blob(                                 //split the blob
                   BLOBNBOX *blob,                  //blob to split
                   inT16 chop_coord,                //place to chop
                   float pitch_error,               //allowed deviation
                   OUTLINE_LIST *left_outlines,     //left half of chop
                   C_OUTLINE_LIST *left_coutlines,  //for cblobs
                   OUTLINE_LIST *right_outlines,    //right half of chop
                   C_OUTLINE_LIST *right_coutlines) {
  PBLOB *real_blob;              //blob to chop
  C_BLOB *real_cblob;            //cblob to chop

  if (blob != NULL) {
    real_blob = blob->blob ();
    real_cblob = blob->cblob ();
  }
  else {
    real_blob = NULL;
    real_cblob = NULL;
  }
  if (!right_outlines->empty () || real_blob != NULL)
    fixed_chop_blob(real_blob,
                    chop_coord,
                    pitch_error,
                    left_outlines,
                    right_outlines);
  else if (!right_coutlines->empty () || real_cblob != NULL)
    fixed_chop_cblob(real_cblob,
                     chop_coord,
                     pitch_error,
                     left_coutlines,
                     right_coutlines);
  if (blob != NULL)
    delete blob;                 //free it
}


/**********************************************************************
 * fixed_chop_blob
 *
 * Chop the given blob (if any) and the existing right outlines to
 * produce a list of outlines left of the chop point and more to the right.
 **********************************************************************/

void fixed_chop_blob(                              //split the blob
                     PBLOB *blob,                  //blob to split
                     inT16 chop_coord,             //place to chop
                     float pitch_error,            //allowed deviation
                     OUTLINE_LIST *left_outlines,  //left half of chop
                     OUTLINE_LIST *right_outlines  //right half of chop
                    ) {
  OUTLINE *old_right;            //already there
  OUTLINE_LIST new_outlines;     //new right ones
                                 //ouput iterator
  OUTLINE_IT left_it = left_outlines;
                                 //in/out iterator
  OUTLINE_IT right_it = right_outlines;
  OUTLINE_IT new_it = &new_outlines;
  OUTLINE_IT blob_it;            //outlines in blob

  if (!right_it.empty ()) {
    while (!right_it.empty ()) {
      old_right = right_it.extract ();
      right_it.forward ();
      fixed_split_outline(old_right,
                          chop_coord,
                          pitch_error,
                          &left_it,
                          &new_it);
    }
    right_it.add_list_before (&new_outlines);
  }
  if (blob != NULL) {
    blob_it.set_to_list (blob->out_list ());
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
      blob_it.forward ())
    fixed_split_outline (blob_it.extract (), chop_coord, pitch_error,
        &left_it, &right_it);
    delete blob;
  }
}


/**********************************************************************
 * fixed_split_outline
 *
 * Chop the given outline (if necessary) placing the fragments which
 * fall either side of the chop line into the appropriate list.
 **********************************************************************/

void fixed_split_outline(                      //chop the outline
                         OUTLINE *srcline,     //source outline
                         inT16 chop_coord,     //place to chop
                         float pitch_error,    //allowed deviation
                         OUTLINE_IT *left_it,  //left half of chop
                         OUTLINE_IT *right_it  //right half of chop
                        ) {
  OUTLINE *child;                //child outline
  TBOX srcbox;                    //box of outline
  OUTLINE_LIST left_ch;          //left children
  OUTLINE_LIST right_ch;         //right children
  OUTLINE_FRAG_LIST left_frags;  //chopped fragments
  OUTLINE_FRAG_LIST right_frags;;
  OUTLINE_IT left_ch_it = &left_ch;
                                 //for whole children
  OUTLINE_IT right_ch_it = &right_ch;
                                 //for holes
  OUTLINE_IT child_it = srcline->child ();

  srcbox = srcline->bounding_box ();
                                 //left of line
  if (srcbox.left () + srcbox.right () <= chop_coord * 2
                                 //and not far over
    && srcbox.right () < chop_coord + pitch_error)
                                 //stick whole in left
    left_it->add_after_then_move (srcline);
  else if (srcbox.left () + srcbox.right () > chop_coord * 2
    && srcbox.left () > chop_coord - pitch_error)
                                 //stick whole in right
    right_it->add_before_stay_put (srcline);
  else {
                                 //needs real chopping
    if (fixed_chop_outline (srcline, chop_coord, pitch_error,
    &left_frags, &right_frags)) {
      for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
      child_it.forward ()) {
        child = child_it.extract ();
        srcbox = child->bounding_box ();
        if (srcbox.right () < chop_coord)
          left_ch_it.add_after_then_move (child);
        else if (srcbox.left () > chop_coord)
          right_ch_it.add_after_then_move (child);
        else {
          if (fixed_chop_outline (child, chop_coord, pitch_error,
            &left_frags, &right_frags))
            delete child;
          else {
            if (srcbox.left () + srcbox.right () <= chop_coord * 2)
              left_ch_it.add_after_then_move (child);
            else
              right_ch_it.add_after_then_move (child);
          }
        }
      }
      close_chopped_fragments(&left_frags, &left_ch, left_it);
      close_chopped_fragments(&right_frags, &right_ch, right_it);
      ASSERT_HOST (left_ch.empty () && right_ch.empty ());
      //no children left
      delete srcline;            //smashed up
    }
    else {
      if (srcbox.left () + srcbox.right () <= chop_coord * 2)
                                 //stick whole in left
        left_it->add_after_then_move (srcline);
      else
        right_it->add_before_stay_put (srcline);
    }
  }
}


/**********************************************************************
 * fixed_chop_outline
 *
 * Chop the given outline (if necessary) placing the fragments which
 * fall either side of the chop line into the appropriate list.
 * If the outline lies too heavily to one side to chop, FALSE is returned.
 **********************************************************************/

BOOL8 fixed_chop_outline(                                //chop the outline
                         OUTLINE *srcline,               //source outline
                         inT16 chop_coord,               //place to chop
                         float pitch_error,              //allowed deviation
                         OUTLINE_FRAG_LIST *left_frags,  //left half of chop
                         OUTLINE_FRAG_LIST *right_frags  //right half of chop
                        ) {
  BOOL8 not_first;               //fragment
  BOOL8 test_valid;              //test pt valid
  float left_edge;               //of outline
  FCOORD chop_pos;               //coords of chop
  float chop_starty;             //test chop pt
  POLYPT *startpt;               //in first fragment
                                 //general iterator
  POLYPT_IT poly_it = srcline->polypts ();
  POLYPT_IT head_it;             //head of fragment
  POLYPT_IT tail_it;             //tail of fragment
  POLYPT_IT test_tail;           //possible chop pt

  left_edge = poly_it.data ()->pos.x ();
  tail_it = poly_it;
  for (poly_it.mark_cycle_pt (); !poly_it.cycled_list (); poly_it.forward ()) {
    if (poly_it.data ()->pos.x () < left_edge) {
      left_edge = poly_it.data ()->pos.x ();
      tail_it = poly_it;         //find leftmost pt
    }
  }
  if (left_edge >= chop_coord - pitch_error)
    return FALSE;                //not worth it

  startpt = tail_it.data ();
  not_first = FALSE;
  head_it = tail_it;
  chop_starty = tail_it.data ()->pos.y ();
  do {
    test_valid = FALSE;
    do {
      tail_it.forward ();
      if (test_valid
        && tail_it.data ()->pos.x () >= chop_coord
        && tail_it.data ()->pos.x () + tail_it.data ()->vec.x () <=
      chop_coord) {
        chop_pos = find_chop_coords (&tail_it, chop_coord);
        if (chop_pos.y () >= chop_starty)
          test_valid = FALSE;
        else {
          tail_it = test_tail;
          break;                 //must chop there
        }
      }
      if (tail_it.data ()->pos.x () <= chop_coord
        && tail_it.data ()->pos.x () + tail_it.data ()->vec.x () >=
      chop_coord) {
        chop_pos = find_chop_coords (&tail_it, chop_coord);
        chop_starty = chop_pos.y ();
        test_tail = tail_it;     //save possible chop pt
        test_valid = TRUE;
        if (tail_it.data ()->vec.x () == 0
          && tail_it.data ()->vec.y () < 0)
          break;                 //must chop here
      }
    }
    while (tail_it.data () != startpt
      && tail_it.data ()->pos.x () < chop_coord + pitch_error);
                                 //back to start
    if (tail_it.data () == startpt) {
      if (not_first)
        break;
      else
        return FALSE;            //doesn't cross line
    }
    while (tail_it.data ()->pos.x () > chop_coord)
      tail_it.backward ();
    if (head_it.data () == tail_it.data ())
      insert_extra_pt(&tail_it);
    insert_chop_pt(&tail_it, chop_coord);
    if (not_first) {
      save_chop_fragment(&head_it, &tail_it, left_frags);
    }
    else {
      tail_it.forward ();
      head_it = tail_it;
    }
    test_valid = FALSE;
    do {
      tail_it.forward ();
      if (test_valid
        && tail_it.data ()->pos.x () <= chop_coord
        && tail_it.data ()->pos.x () + tail_it.data ()->vec.x () >=
      chop_coord) {
        chop_pos = find_chop_coords (&tail_it, chop_coord);
        if (chop_pos.y () <= chop_starty)
          test_valid = FALSE;
        else {
          tail_it = test_tail;
          break;                 //must chop there
        }
      }
      if (tail_it.data ()->pos.x () >= chop_coord
        && tail_it.data ()->pos.x () + tail_it.data ()->vec.x () <=
      chop_coord) {
        chop_pos = find_chop_coords (&tail_it, chop_coord);
        chop_starty = chop_pos.y ();
        test_tail = tail_it;
        test_valid = TRUE;       //save possible chop pt
        if (tail_it.data ()->vec.x () == 0
          && tail_it.data ()->vec.y () > 0)
          break;                 //must chop here
      }
    }
    while (tail_it.data () != startpt
      && tail_it.data ()->pos.x () > chop_coord - pitch_error);
    while (tail_it.data ()->pos.x () < chop_coord)
      tail_it.backward ();
    if (head_it.data () == tail_it.data ())
      insert_extra_pt(&tail_it);
    insert_chop_pt(&tail_it, chop_coord);
    save_chop_fragment(&head_it, &tail_it, right_frags);
    not_first = TRUE;
  }
  while (tail_it.data () != startpt);
  startpt = head_it.data_relative (-1);
  while (tail_it.data () != startpt)
    tail_it.forward ();
  save_chop_fragment(&head_it, &tail_it, left_frags);
  return TRUE;                   //did some chopping
}


/**********************************************************************
 * save_chop_fragment
 *
 * Store the given fragment in the given fragment list.
 **********************************************************************/

void save_chop_fragment(                          //chop the outline
                        POLYPT_IT *head_it,       //head of fragment
                        POLYPT_IT *tail_it,       //tail of fragment
                        OUTLINE_FRAG_LIST *frags  //fragment list
                       ) {
  OUTLINE_FRAG *head;            //head of fragment
  OUTLINE_FRAG *tail;            //tail of fragment
  float tail_y;                  //ycoord of tail

  tail_y = tail_it->data ()->pos.y ();
  head = new OUTLINE_FRAG (head_it, tail_it);
  tail = new OUTLINE_FRAG (head, tail_y);
  head->other_end = tail;
  add_frag_to_list(head, frags);
  add_frag_to_list(tail, frags);
  head_it->forward ();
  tail_it->forward ();
}


/**********************************************************************
 * OUTLINE_FRAG::OUTLINE_FRAG
 *
 * Constructors for OUTLINE_FRAG.
 **********************************************************************/

OUTLINE_FRAG::OUTLINE_FRAG(                     //record fragment
                           POLYPT_IT *head_it,  //head of fragment
                           POLYPT_IT *tail_it   //tail of fragment
                          ) {
  ycoord = head_it->data ()->pos.y ();
  other_end = NULL;
  polypts.assign_to_sublist (head_it, tail_it);
}


OUTLINE_FRAG::OUTLINE_FRAG(                     //record fragment
                           OUTLINE_FRAG *head,  //other end
                           float tail_y) {
  ycoord = tail_y;
  other_end = head;
}


/**********************************************************************
 * add_frag_to_list
 *
 * Insert the fragment in the list at the appropriate place to keep
 * them in ascending ycoord order.
 **********************************************************************/

void add_frag_to_list(                          //ordered add
                      OUTLINE_FRAG *frag,       //fragment to add
                      OUTLINE_FRAG_LIST *frags  //fragment list
                     ) {
                                 //output list
  OUTLINE_FRAG_IT frag_it = frags;

  if (!frags->empty ()) {
    for (frag_it.mark_cycle_pt (); !frag_it.cycled_list ();
    frag_it.forward ()) {
      if (frag_it.data ()->ycoord >= frag->ycoord) {
        frag_it.add_before_then_move (frag);
        return;
      }
    }
  }
  frag_it.add_to_end (frag);
}


/**********************************************************************
 * insert_chop_pt
 *
 * Decide whether or not to use the actual point as chop coord.
 * Insert either a duplicate of the current point or 2 copies
 * of the new chop point. Position the iterator at the first.
 **********************************************************************/

void insert_chop_pt(                  //make chop
                    POLYPT_IT *it,    //iterator
                    inT16 chop_coord  //required chop pt
                   ) {
  POLYPT *prev_pt;               //point befor chop
  POLYPT *chop_pt;               //new vertex
  FCOORD chop_pos;               //coords of chop
  FCOORD chop_vec;               //vector to next

  prev_pt = it->data ();
  if (prev_pt->pos.x () + textord_fp_chop_snap >= chop_coord
  && prev_pt->pos.x () - textord_fp_chop_snap <= chop_coord) {
    chop_pt = new POLYPT (prev_pt->pos, prev_pt->vec);
  }
  else {
    chop_pos = FCOORD (chop_coord, prev_pt->pos.y ()
      + prev_pt->vec.y () * (chop_coord -
      prev_pt->pos.x ()) /
      prev_pt->vec.x ());
    chop_vec = it->data_relative (1)->pos - chop_pos;
    chop_pt = new POLYPT (chop_pos, chop_vec);
    it->add_after_then_move (chop_pt);
    chop_pt = new POLYPT (chop_pos, chop_vec);
  }
  it->add_after_stay_put (chop_pt);
}


/**********************************************************************
 * find_chop_coords
 *
 * Decide whether or not to use the actual point as chop coord.
 * Return the coords of the chop point.
 **********************************************************************/

FCOORD find_chop_coords(                  //make chop
                        POLYPT_IT *it,    //iterator
                        inT16 chop_coord  //required chop pt
                       ) {
  POLYPT *prev_pt;               //point befor chop
  FCOORD chop_pos;               //coords of chop

  prev_pt = it->data ();
  if (prev_pt->pos.x () + textord_fp_chop_snap >= chop_coord
  && prev_pt->pos.x () - textord_fp_chop_snap <= chop_coord) {
    chop_pos = prev_pt->pos;
  }
  else {
    chop_pos = FCOORD (chop_coord, prev_pt->pos.y ()
      + prev_pt->vec.y () * (chop_coord -
      prev_pt->pos.x ()) /
      prev_pt->vec.x ());
  }
  return chop_pos;
}


/**********************************************************************
 * insert_extra_pt
 *
 * Add an extra pt to prevent single point fragments being made.
 **********************************************************************/

void insert_extra_pt(               //make extra
                     POLYPT_IT *it  //iterator
                    ) {
  POLYPT *prev_pt;               //point befor chop
  POLYPT *chop_pt;               //new vertex
  FCOORD chop_pos;               //coords of chop
  FCOORD chop_vec;               //vector to next

  prev_pt = it->data ();
  if (it->data_relative (1)->pos.y () > it->data_relative (-1)->pos.y ()) {
    chop_pos = prev_pt->pos + FCOORD (0.0f,
                                      static_cast<float>(textord_fp_chop_snap));
  }
  else {
    chop_pos = prev_pt->pos - FCOORD (0.0f,
                                      static_cast<float>(textord_fp_chop_snap));
  }
  chop_vec = it->data_relative (1)->pos - chop_pos;
  prev_pt->vec = chop_pos - prev_pt->pos;
  chop_pt = new POLYPT (chop_pos, chop_vec);
  it->add_after_then_move (chop_pt);
}


/**********************************************************************
 * close_chopped_fragments
 *
 * Clear the given list of fragments joining them up into outlines.
 * Each outline made soaks up any of the child outlines which it encloses.
 **********************************************************************/

void close_chopped_fragments(                           //chop the outline
                             OUTLINE_FRAG_LIST *frags,  //list to clear
                             OUTLINE_LIST *children,    //potential children
                             OUTLINE_IT *dest_it        //output list
                            ) {
                                 //iterator
  OUTLINE_FRAG_IT frag_it = frags;
  OUTLINE_FRAG *bottom_frag;     //bottom of cut
  OUTLINE_FRAG *top_frag;        //top of cut
  OUTLINE *outline;              //new outline
  OUTLINE *child;                //current child
  OUTLINE_IT child_it = children;
  OUTLINE_IT olchild_it;         //children of outline
  POLYPT_IT poly_it;             //iterator for constr

  while (!frag_it.empty ()) {
    frag_it.move_to_first ();
                                 //get bottom one
    bottom_frag = frag_it.extract ();
    frag_it.forward ();
                                 //and one above it
    top_frag = frag_it.extract ();
    while (top_frag->other_end != bottom_frag) {
      do {
        frag_it.forward ();
      }
                                 //find other end
      while (frag_it.data () != top_frag->other_end);
      join_chopped_fragments(bottom_frag, top_frag);
      delete top_frag;
      delete frag_it.extract (); //remove middle section
      frag_it.forward ();
      top_frag = frag_it.extract ();
    }
    join_chopped_fragments(bottom_frag, top_frag);
    if (bottom_frag->polypts.empty ())
      poly_it.set_to_list (&top_frag->polypts);
    else
      poly_it.set_to_list (&bottom_frag->polypts);
    outline = new OUTLINE (&poly_it);
    olchild_it.set_to_list (outline->child ());
    for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
    child_it.forward ()) {
      child = child_it.data ();
      if (*child < *outline)
        olchild_it.add_to_end (child_it.extract ());
    }
    dest_it->add_after_then_move (outline);
  }
  while (!child_it.empty ()) {
    dest_it->add_after_then_move (child_it.extract ());
    child_it.forward ();
  }
}


/**********************************************************************
 * join_chopped_fragments
 *
 * Join the two lists of POLYPTs such that the first OUTLINE_FRAG
 * operand keeps responsibility for the fragment.
 **********************************************************************/

void join_chopped_fragments(                       //join pieces
                            OUTLINE_FRAG *bottom,  //bottom of cut
                            OUTLINE_FRAG *top      //top of cut
                           ) {
  POLYPT_IT master_it;           //dest list
  POLYPT_IT slave_it;            //src list
  POLYPT *cutpt;                 //vectors to change
  POLYPT *nextpt;                //other end of cut

  if (bottom->polypts.empty ()) {
    master_it.set_to_list (&bottom->other_end->polypts);
    cutpt = master_it.data_relative (-1);
    ASSERT_HOST (!top->polypts.empty ());
    slave_it.set_to_list (&top->polypts);
    nextpt = slave_it.data ();
    if (bottom->other_end != top) {
      master_it.move_to_last ();
      master_it.add_list_after (&top->polypts);
    }
  }
  else {
    master_it.set_to_list (&bottom->polypts);
    ASSERT_HOST (top->polypts.empty ());
    slave_it.set_to_list (&top->other_end->polypts);
    cutpt = slave_it.data_relative (-1);
    nextpt = master_it.data ();
    if (bottom->other_end != top)
      master_it.add_list_before (&top->other_end->polypts);
  }
  cutpt->vec = nextpt->pos - cutpt->pos;
}


/**********************************************************************
 * fixed_chop_cblob
 *
 * Chop the given cblob (if any) and the existing right outlines to
 * produce a list of outlines left of the chop point and more to the right.
 **********************************************************************/

void fixed_chop_cblob(                                //split the blob
                      C_BLOB *blob,                   //blob to split
                      inT16 chop_coord,               //place to chop
                      float pitch_error,              //allowed deviation
                      C_OUTLINE_LIST *left_outlines,  //left half of chop
                      C_OUTLINE_LIST *right_outlines  //right half of chop
                     ) {
  C_OUTLINE *old_right;          //already there
  C_OUTLINE_LIST new_outlines;   //new right ones
                                 //ouput iterator
  C_OUTLINE_IT left_it = left_outlines;
                                 //in/out iterator
  C_OUTLINE_IT right_it = right_outlines;
  C_OUTLINE_IT new_it = &new_outlines;
  C_OUTLINE_IT blob_it;          //outlines in blob

  if (!right_it.empty ()) {
    while (!right_it.empty ()) {
      old_right = right_it.extract ();
      right_it.forward ();
      fixed_split_coutline(old_right,
                           chop_coord,
                           pitch_error,
                           &left_it,
                           &new_it);
    }
    right_it.add_list_before (&new_outlines);
  }
  if (blob != NULL) {
    blob_it.set_to_list (blob->out_list ());
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
      blob_it.forward ())
    fixed_split_coutline (blob_it.extract (), chop_coord, pitch_error,
        &left_it, &right_it);
    delete blob;
  }
}


/**********************************************************************
 * fixed_split_outline
 *
 * Chop the given outline (if necessary) placing the fragments which
 * fall either side of the chop line into the appropriate list.
 **********************************************************************/

void fixed_split_coutline(                        //chop the outline
                          C_OUTLINE *srcline,     //source outline
                          inT16 chop_coord,       //place to chop
                          float pitch_error,      //allowed deviation
                          C_OUTLINE_IT *left_it,  //left half of chop
                          C_OUTLINE_IT *right_it  //right half of chop
                         ) {
  C_OUTLINE *child;              //child outline
  TBOX srcbox;                    //box of outline
  C_OUTLINE_LIST left_ch;        //left children
  C_OUTLINE_LIST right_ch;       //right children
  C_OUTLINE_FRAG_LIST left_frags;//chopped fragments
  C_OUTLINE_FRAG_LIST right_frags;;
  C_OUTLINE_IT left_ch_it = &left_ch;
                                 //for whole children
  C_OUTLINE_IT right_ch_it = &right_ch;
                                 //for holes
  C_OUTLINE_IT child_it = srcline->child ();

  srcbox = srcline->bounding_box ();
                                 //left of line
  if (srcbox.left () + srcbox.right () <= chop_coord * 2
                                 //and not far over
    && srcbox.right () < chop_coord + pitch_error)
                                 //stick whole in left
    left_it->add_after_then_move (srcline);
  else if (srcbox.left () + srcbox.right () > chop_coord * 2
    && srcbox.left () > chop_coord - pitch_error)
                                 //stick whole in right
    right_it->add_before_stay_put (srcline);
  else {
                                 //needs real chopping
    if (fixed_chop_coutline (srcline, chop_coord, pitch_error,
    &left_frags, &right_frags)) {
      for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
      child_it.forward ()) {
        child = child_it.extract ();
        srcbox = child->bounding_box ();
        if (srcbox.right () < chop_coord)
          left_ch_it.add_after_then_move (child);
        else if (srcbox.left () > chop_coord)
          right_ch_it.add_after_then_move (child);
        else {
          if (fixed_chop_coutline (child, chop_coord, pitch_error,
            &left_frags, &right_frags))
            delete child;
          else {
            if (srcbox.left () + srcbox.right () <= chop_coord * 2)
              left_ch_it.add_after_then_move (child);
            else
              right_ch_it.add_after_then_move (child);
          }
        }
      }
      close_chopped_cfragments(&left_frags, &left_ch, pitch_error, left_it);
      close_chopped_cfragments(&right_frags, &right_ch, pitch_error, right_it);
      ASSERT_HOST (left_ch.empty () && right_ch.empty ());
      //no children left
      delete srcline;            //smashed up
    }
    else {
      if (srcbox.left () + srcbox.right () <= chop_coord * 2)
                                 //stick whole in left
        left_it->add_after_then_move (srcline);
      else
        right_it->add_before_stay_put (srcline);
    }
  }
}


/**********************************************************************
 * fixed_chop_coutline
 *
 * Chop the given coutline (if necessary) placing the fragments which
 * fall either side of the chop line into the appropriate list.
 * If the coutline lies too heavily to one side to chop, FALSE is returned.
 **********************************************************************/

BOOL8 fixed_chop_coutline(                                  //chop the outline
                          C_OUTLINE *srcline,               //source outline
                          inT16 chop_coord,                 //place to chop
                          float pitch_error,                //allowed deviation
                          C_OUTLINE_FRAG_LIST *left_frags,  //left half of chop
                          C_OUTLINE_FRAG_LIST *right_frags  //right half of chop
                         ) {
  BOOL8 first_frag;              //fragment
  BOOL8 anticlock;               //direction of loop
  inT16 left_edge;               //of outline
  inT16 startindex;              //in first fragment
  inT32 length;                  //of outline
  inT16 stepindex;               //into outline
  inT16 head_index;              //start of fragment
  ICOORD head_pos;               //start of fragment
  inT16 tail_index;              //end of fragment
  ICOORD tail_pos;               //end of fragment
  ICOORD pos;                    //current point
  inT16 first_index = 0;         //first tail
  ICOORD first_pos;              //first tail

  length = srcline->pathlength ();
  pos = srcline->start_pos ();
  anticlock = srcline->turn_direction () > 0;
  left_edge = pos.x ();
  tail_index = 0;
  tail_pos = pos;
  for (stepindex = 0; stepindex < length; stepindex++) {
    if (pos.x () < left_edge) {
      left_edge = pos.x ();
      tail_index = stepindex;
      tail_pos = pos;
    }
    pos += srcline->step (stepindex);
  }
  if (left_edge >= chop_coord - pitch_error)
    return FALSE;                //not worth it

  startindex = tail_index;
  first_frag = TRUE;
  head_index = tail_index;
  head_pos = tail_pos;
  do {
    do {
      tail_pos += srcline->step (tail_index);
      tail_index++;
      if (tail_index == length)
        tail_index = 0;
    }
    while (tail_pos.x () != chop_coord && tail_index != startindex);
    if (tail_index == startindex) {
      if (first_frag)
        return FALSE;            //doesn't cross line
      else
        break;
    }
    //#ifdef __UNIX__
    ASSERT_HOST (head_index != tail_index);
    //#endif
    if (!first_frag) {
      save_chop_cfragment(head_index,
                          head_pos,
                          tail_index,
                          tail_pos,
                          srcline,
                          left_frags);
    }
    else {
      first_index = tail_index;
      first_pos = tail_pos;
      first_frag = FALSE;
    }
    while (srcline->step (tail_index).x () == 0) {
      tail_pos += srcline->step (tail_index);
      tail_index++;
      if (tail_index == length)
        tail_index = 0;
    }
    head_index = tail_index;
    head_pos = tail_pos;
    while (srcline->step (tail_index).x () > 0) {
      do {
        tail_pos += srcline->step (tail_index);
        tail_index++;
        if (tail_index == length)
          tail_index = 0;
      }
      while (tail_pos.x () != chop_coord);
      //#ifdef __UNIX__
      ASSERT_HOST (head_index != tail_index);
      //#endif
      save_chop_cfragment(head_index,
                          head_pos,
                          tail_index,
                          tail_pos,
                          srcline,
                          right_frags);
      while (srcline->step (tail_index).x () == 0) {
        tail_pos += srcline->step (tail_index);
        tail_index++;
        if (tail_index == length)
          tail_index = 0;
      }
      head_index = tail_index;
      head_pos = tail_pos;
    }
  }
  while (tail_index != startindex);
  save_chop_cfragment(head_index,
                      head_pos,
                      first_index,
                      first_pos,
                      srcline,
                      left_frags);
  return TRUE;                   //did some chopping
}


/**********************************************************************
 * next_anti_left_seg
 *
 * Search the outline for a suitable point at which it crosses the
 * chop_coord from left to right.
 **********************************************************************/

inT16 next_anti_left_seg(                     //chop the outline
                         C_OUTLINE *srcline,  //source outline
                         inT16 tail_index,    //of tailpos
                         inT16 startindex,    //end of search
                         inT32 length,        //of outline
                         inT16 chop_coord,    //place to chop
                         float pitch_error,   //allowed deviation
                         ICOORD *tail_pos     //current position
                        ) {
  BOOL8 test_valid;              //test pt valid
  inT16 chop_starty;             //test chop pt
  inT16 test_index;              //possible chop pt
  ICOORD test_pos;               //possible chop pt
  ICOORD prev_step;              //in x to tail pos

  test_valid = FALSE;
  chop_starty = -MAX_INT16;
  test_index = tail_index;       //stop warnings
  do {
    *tail_pos += srcline->step (tail_index);
    prev_step = srcline->step (tail_index);
    tail_index++;
    if (tail_index >= length)
      tail_index = 0;
    if (test_valid && tail_pos->x () == chop_coord && prev_step.x () < 0) {
      if (tail_pos->y () >= chop_starty) {
        chop_starty = -MAX_INT16;
        test_valid = FALSE;
      }
      else {
        *tail_pos = test_pos;
        tail_index = test_index;
        break;                   //must chop there
      }
    }
    if (tail_pos->x () == chop_coord
      && srcline->step (tail_index).x () > 0
    && tail_pos->y () > chop_starty) {
      chop_starty = tail_pos->y ();
      test_index = tail_index;
      test_pos = *tail_pos;
      test_valid = TRUE;
    }
    else if (tail_pos->x () == chop_coord
      && srcline->step (tail_index).y () < 0
      && prev_step.x () > 0 && tail_pos->y () > chop_starty)
      break;                     //must chop here
  }
  while (tail_index != startindex
    && tail_pos->x () < chop_coord + pitch_error);
  return tail_index;
}


/**********************************************************************
 * next_anti_right_seg
 *
 * Search the outline for a suitable point at which it crosses the
 * chop_coord from right to left.
 **********************************************************************/

inT16 next_anti_right_seg(                     //chop the outline
                          C_OUTLINE *srcline,  //source outline
                          inT16 tail_index,    //of tailpos
                          inT16 startindex,    //end of search
                          inT32 length,        //of outline
                          inT16 chop_coord,    //place to chop
                          float pitch_error,   //allowed deviation
                          ICOORD *tail_pos     //current position
                         ) {
  BOOL8 test_valid;              //test pt valid
  inT16 chop_starty;             //test chop pt
  inT16 test_index;              //possible chop pt
  ICOORD test_pos;               //possible chop pt
  ICOORD prev_step;              //in x to tail pos

  test_valid = FALSE;
  chop_starty = MAX_INT16;
  test_index = tail_index;       //stop warnings
  do {
                                 //move forward
    *tail_pos += srcline->step (tail_index);
    prev_step = srcline->step (tail_index);
    tail_index++;
    if (tail_index >= length)
      tail_index = 0;
    if (test_valid && tail_pos->x () == chop_coord && prev_step.x () > 0) {
      if (tail_pos->y () <= chop_starty) {
        chop_starty = MAX_INT16;
        test_valid = FALSE;
      }
      else {
        *tail_pos = test_pos;
        tail_index = test_index;
        break;                   //must chop there
      }
    }
    if (tail_pos->x () == chop_coord
      && srcline->step (tail_index).x () < 0
    && tail_pos->y () < chop_starty) {
      chop_starty = tail_pos->y ();
      test_index = tail_index;
      test_pos = *tail_pos;
      test_valid = TRUE;         //save possible chop pt
    }
    else if (tail_pos->x () == chop_coord
      && srcline->step (tail_index).y () > 0
      && prev_step.x () < 0 && tail_pos->y () < chop_starty)
      break;                     //must chop here
  }
  while (tail_index != startindex
    && tail_pos->x () > chop_coord - pitch_error);
  return tail_index;
}


/**********************************************************************
 * next_clock_left_seg
 *
 * Search the outline for a suitable point at which it crosses the
 * chop_coord from left to right.
 **********************************************************************/

inT16 next_clock_left_seg(                     //chop the outline
                          C_OUTLINE *srcline,  //source outline
                          inT16 tail_index,    //of tailpos
                          inT16 startindex,    //end of search
                          inT32 length,        //of outline
                          inT16 chop_coord,    //place to chop
                          float pitch_error,   //allowed deviation
                          ICOORD *tail_pos     //current position
                         ) {
  BOOL8 test_valid;              //test pt valid
  inT16 chop_starty;             //test chop pt
  inT16 test_index;              //possible chop pt
  ICOORD test_pos;               //possible chop pt
  ICOORD prev_step;              //in x to tail pos

  test_valid = FALSE;
  chop_starty = MAX_INT16;
  test_index = tail_index;       //stop warnings
  do {
    *tail_pos += srcline->step (tail_index);
    prev_step = srcline->step (tail_index);
    tail_index++;
    if (tail_index >= length)
      tail_index = 0;
    if (test_valid && tail_pos->x () == chop_coord && prev_step.x () < 0) {
      if (tail_pos->y () <= chop_starty) {
        chop_starty = MAX_INT16;
        test_valid = FALSE;
      }
      else {
        *tail_pos = test_pos;
        tail_index = test_index;
        break;                   //must chop there
      }
    }
    if (tail_pos->x () == chop_coord
      && srcline->step (tail_index).x () > 0
    && tail_pos->y () < chop_starty) {
      chop_starty = tail_pos->y ();
      test_index = tail_index;
      test_pos = *tail_pos;
      test_valid = TRUE;
    }
    else if (tail_pos->x () == chop_coord
      && srcline->step (tail_index).y () > 0
      && prev_step.x () > 0 && tail_pos->y () < chop_starty)
      break;                     //must chop here
  }
  while (tail_index != startindex
    && tail_pos->x () < chop_coord + pitch_error);
  return tail_index;
}


/**********************************************************************
 * next_clock_right_seg
 *
 * Search the outline for a suitable point at which it crosses the
 * chop_coord from right to left.
 **********************************************************************/

inT16 next_clock_right_seg(                     //chop the outline
                           C_OUTLINE *srcline,  //source outline
                           inT16 tail_index,    //of tailpos
                           inT16 startindex,    //end of search
                           inT32 length,        //of outline
                           inT16 chop_coord,    //place to chop
                           float pitch_error,   //allowed deviation
                           ICOORD *tail_pos     //current position
                          ) {
  BOOL8 test_valid;              //test pt valid
  inT16 chop_starty;             //test chop pt
  inT16 test_index;              //possible chop pt
  ICOORD test_pos;               //possible chop pt
  ICOORD prev_step;              //in x to tail pos

  test_valid = FALSE;
  chop_starty = MAX_INT16;
  test_index = tail_index;       //stop warnings
  do {
                                 //move forward
    *tail_pos += srcline->step (tail_index);
    prev_step = srcline->step (tail_index);
    tail_index++;
    if (tail_index >= length)
      tail_index = 0;
    if (test_valid && tail_pos->x () == chop_coord && prev_step.x () > 0) {
      if (tail_pos->y () >= chop_starty) {
        chop_starty = MAX_INT16;
        test_valid = FALSE;
      }
      else {
        *tail_pos = test_pos;
        tail_index = test_index;
        break;                   //must chop there
      }
    }
    if (tail_pos->x () == chop_coord
      && srcline->step (tail_index).x () < 0
    && tail_pos->y () > chop_starty) {
      chop_starty = tail_pos->y ();
      test_index = tail_index;
      test_pos = *tail_pos;
      test_valid = TRUE;         //save possible chop pt
    }
    else if (tail_pos->x () == chop_coord
      && srcline->step (tail_index).y () < 0
      && prev_step.x () < 0 && tail_pos->y () > chop_starty)
      break;                     //must chop here
  }
  while (tail_index != startindex
    && tail_pos->x () > chop_coord - pitch_error);
  return tail_index;
}


/**********************************************************************
 * save_chop_cfragment
 *
 * Store the given fragment in the given fragment list.
 **********************************************************************/

void save_chop_cfragment(                            //chop the outline
                         inT16 head_index,           //head of fragment
                         ICOORD head_pos,            //head of fragment
                         inT16 tail_index,           //tail of fragment
                         ICOORD tail_pos,            //tail of fragment
                         C_OUTLINE *srcline,         //source of edgesteps
                         C_OUTLINE_FRAG_LIST *frags  //fragment list
                        ) {
  inT16 jump;                    //gap across end
  inT16 stepcount;               //total steps
  C_OUTLINE_FRAG *head;          //head of fragment
  C_OUTLINE_FRAG *tail;          //tail of fragment
  inT16 tail_y;                  //ycoord of tail

  ASSERT_HOST (tail_pos.x () == head_pos.x ());
  ASSERT_HOST (tail_index != head_index);
  stepcount = tail_index - head_index;
  if (stepcount < 0)
    stepcount += srcline->pathlength ();
  jump = tail_pos.y () - head_pos.y ();
  if (jump < 0)
    jump = -jump;
  if (jump == stepcount)
    return;                      //its a nop
  tail_y = tail_pos.y ();
  head = new C_OUTLINE_FRAG (head_pos, tail_pos, srcline,
    head_index, tail_index);
  tail = new C_OUTLINE_FRAG (head, tail_y);
  head->other_end = tail;
  add_frag_to_list(head, frags);
  add_frag_to_list(tail, frags);
}


/**********************************************************************
 * C_OUTLINE_FRAG::C_OUTLINE_FRAG
 *
 * Constructors for C_OUTLINE_FRAG.
 **********************************************************************/

C_OUTLINE_FRAG::C_OUTLINE_FRAG(                     //record fragment
                               ICOORD start_pt,     //start coord
                               ICOORD end_pt,       //end coord
                               C_OUTLINE *outline,  //source of steps
                               inT16 start_index,
                               inT16 end_index) {
  start = start_pt;
  end = end_pt;
  ycoord = start_pt.y ();
  stepcount = end_index - start_index;
  if (stepcount < 0)
    stepcount += outline->pathlength ();
  ASSERT_HOST (stepcount > 0);
  steps = new DIR128[stepcount];
  if (end_index > start_index) {
    for (int i = start_index; i < end_index; ++i)
      steps[i - start_index] = outline->step_dir(i);
  }
  else {
    int len = outline->pathlength();
    int i = start_index;
    for (; i < len; ++i)
      steps[i - start_index] = outline->step_dir(i);
    if (end_index > 0)
      for (; i < end_index + len; ++i)
        steps[i - start_index] = outline->step_dir(i - len);
  }
  other_end = NULL;
  delete close();
}


C_OUTLINE_FRAG::C_OUTLINE_FRAG(                       //record fragment
                               C_OUTLINE_FRAG *head,  //other end
                               inT16 tail_y) {
  ycoord = tail_y;
  other_end = head;
  start = head->start;
  end = head->end;
  steps = NULL;
  stepcount = 0;
}


/**********************************************************************
 * add_frag_to_list
 *
 * Insert the fragment in the list at the appropriate place to keep
 * them in ascending ycoord order.
 **********************************************************************/

void add_frag_to_list(                            //ordered add
                      C_OUTLINE_FRAG *frag,       //fragment to add
                      C_OUTLINE_FRAG_LIST *frags  //fragment list
                     ) {
                                 //output list
  C_OUTLINE_FRAG_IT frag_it = frags;

  if (!frags->empty ()) {
    for (frag_it.mark_cycle_pt (); !frag_it.cycled_list ();
    frag_it.forward ()) {
      if (frag_it.data ()->ycoord > frag->ycoord
        || (frag_it.data ()->ycoord == frag->ycoord
         && frag->other_end->ycoord < frag->ycoord)) {
        frag_it.add_before_then_move (frag);
        return;
      }
    }
  }
  frag_it.add_to_end (frag);
}


/**********************************************************************
 * close_chopped_cfragments
 *
 * Clear the given list of fragments joining them up into outlines.
 * Each outline made soaks up any of the child outlines which it encloses.
 **********************************************************************/

void close_chopped_cfragments(                             //chop the outline
                              C_OUTLINE_FRAG_LIST *frags,  //list to clear
                              C_OUTLINE_LIST *children,    //potential children
                              float pitch_error,           //allowed shrinkage
                              C_OUTLINE_IT *dest_it        //output list
                             ) {
                                 //iterator
  C_OUTLINE_FRAG_IT frag_it = frags;
  C_OUTLINE_FRAG *bottom_frag;   //bottom of cut
  C_OUTLINE_FRAG *top_frag;      //top of cut
  C_OUTLINE *outline;            //new outline
  C_OUTLINE *child;              //current child
  C_OUTLINE_IT child_it = children;
  C_OUTLINE_IT olchild_it;       //children of outline

  while (!frag_it.empty ()) {
    frag_it.move_to_first ();
                                 //get bottom one
    bottom_frag = frag_it.extract ();
    frag_it.forward ();
    top_frag = frag_it.data ();  //look at next
    if ((bottom_frag->steps == 0 && top_frag->steps == 0)
    || (bottom_frag->steps != 0 && top_frag->steps != 0)) {
      if (frag_it.data_relative (1)->ycoord == top_frag->ycoord)
        frag_it.forward ();
    }
    top_frag = frag_it.extract ();
    if (top_frag->other_end != bottom_frag) {
      outline = join_chopped_fragments (bottom_frag, top_frag);
      ASSERT_HOST (outline == NULL);
    }
    else {
      outline = join_chopped_fragments (bottom_frag, top_frag);
      ASSERT_HOST (outline != NULL);
      olchild_it.set_to_list (outline->child ());
      for (child_it.mark_cycle_pt (); !child_it.cycled_list ();
      child_it.forward ()) {
        child = child_it.data ();
        if (*child < *outline)
          olchild_it.add_to_end (child_it.extract ());
      }
      if (outline->bounding_box ().width () > pitch_error)
        dest_it->add_after_then_move (outline);
      else
        delete outline;          //make it disappear
    }
  }
  while (!child_it.empty ()) {
    dest_it->add_after_then_move (child_it.extract ());
    child_it.forward ();
  }
}


/**********************************************************************
 * join_chopped_fragments
 *
 * Join the two lists of POLYPTs such that neither OUTLINE_FRAG
 * operand keeps responsibility for the fragment.
 **********************************************************************/

C_OUTLINE *join_chopped_fragments(                         //join pieces
                                  C_OUTLINE_FRAG *bottom,  //bottom of cut
                                  C_OUTLINE_FRAG *top      //top of cut
                                 ) {
  C_OUTLINE *outline;            //closed loop

  if (bottom->other_end == top) {
    if (bottom->steps == 0)
      outline = top->close ();   //turn to outline
    else
      outline = bottom->close ();
    delete top;
    delete bottom;
    return outline;
  }
  if (bottom->steps == 0) {
    ASSERT_HOST (top->steps != 0);
    join_segments (bottom->other_end, top);
  }
  else {
    ASSERT_HOST (top->steps == 0);
    join_segments (top->other_end, bottom);
  }
  top->other_end->other_end = bottom->other_end;
  bottom->other_end->other_end = top->other_end;
  delete bottom;
  delete top;
  return NULL;
}


/**********************************************************************
 * join_segments
 *
 * Join the two edgestep fragments such that the second comes after
 * the first and the gap beween them is closed.
 **********************************************************************/

void join_segments(                         //join pieces
                   C_OUTLINE_FRAG *bottom,  //bottom of cut
                   C_OUTLINE_FRAG *top      //top of cut
                  ) {
  DIR128 *steps;                  //new steps
  inT32 stepcount;               //no of steps
  inT16 fake_count;              //fake steps
  DIR128 fake_step;               //step entry

  ASSERT_HOST (bottom->end.x () == top->start.x ());
  fake_count = top->start.y () - bottom->end.y ();
  if (fake_count < 0) {
    fake_count = -fake_count;
    fake_step = 32;
  }
  else
    fake_step = 96;

  stepcount = bottom->stepcount + fake_count + top->stepcount;
  steps = new DIR128[stepcount];
  memmove (steps, bottom->steps, bottom->stepcount);
  memset (steps + bottom->stepcount, fake_step.get_dir(), fake_count);
  memmove (steps + bottom->stepcount + fake_count, top->steps,
    top->stepcount);
  delete [] bottom->steps;
  bottom->steps = steps;
  bottom->stepcount = stepcount;
  bottom->end = top->end;
  bottom->other_end->end = top->end;
}


/**********************************************************************
 * C_OUTLINE_FRAG::close
 *
 * Join the ends of this fragment and turn it into an outline.
 **********************************************************************/

C_OUTLINE *C_OUTLINE_FRAG::close() {  //join pieces
  DIR128 *new_steps;              //new steps
  inT32 new_stepcount;           //no of steps
  inT16 fake_count;              //fake steps
  DIR128 fake_step;               //step entry

  ASSERT_HOST (start.x () == end.x ());
  fake_count = start.y () - end.y ();
  if (fake_count < 0) {
    fake_count = -fake_count;
    fake_step = 32;
  }
  else
    fake_step = 96;

  new_stepcount = stepcount + fake_count;
  new_steps = new DIR128[new_stepcount];
  memmove(new_steps, steps, stepcount);
  memset (new_steps + stepcount, fake_step.get_dir(), fake_count);
  C_OUTLINE* result = new C_OUTLINE (start, new_steps, new_stepcount);
  delete [] new_steps;
  return result;
}


/**********************************************************************
 * C_OUTLINE_FRAG::operator=
 *
 * Copy this fragment.
 **********************************************************************/

                                 //join pieces
C_OUTLINE_FRAG & C_OUTLINE_FRAG::operator= (
const C_OUTLINE_FRAG & src       //fragment to copy
) {
  if (steps != NULL)
    delete [] steps;

  stepcount = src.stepcount;
  steps = new DIR128[stepcount];
  memmove (steps, src.steps, stepcount);
  start = src.start;
  end = src.end;
  ycoord = src.ycoord;
  return *this;
}
