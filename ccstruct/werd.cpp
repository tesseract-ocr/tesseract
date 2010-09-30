/**********************************************************************
 * File:        werd.cpp  (Formerly word.c)
 * Description: Code for the WERD class.
 * Author:		Ray Smith
 * Created:		Tue Oct 08 14:32:12 BST 1991
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

#include "mfcpch.h"
#include          "blckerr.h"
#include          "linlsq.h"
#include          "werd.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define FIRST_COLOUR    ScrollView::RED      //< first rainbow colour

/// last rainbow colour
#define LAST_COLOUR     ScrollView::AQUAMARINE
#define CHILD_COLOUR    ScrollView::BROWN    //< colour of children

const ERRCODE CANT_SCALE_EDGESTEPS =
"Attempted to scale an edgestep format word";

#define EXTERN

EXTERN BOOL_VAR (bln_numericmode, 0, "Optimize for numbers");
EXTERN INT_VAR (bln_x_height, 128, "Baseline Normalisation X-height");
EXTERN INT_VAR (bln_baseline_offset, 64, "Baseline Norm. offset of baseline");
EXTERN double_VAR (bln_blshift_maxshift, -1.0,
"Fraction of xh before shifting");
EXTERN double_VAR (bln_blshift_xfraction, 0.75,
"Size fraction of xh before shifting");

ELISTIZE_S (WERD)
/**
 * WERD::WERD
 *
 * Constructor to build a WERD from a list of C_BLOBs.
 * The C_BLOBs are not copied so the source list is emptied.
 */
WERD::WERD (                     //constructor
C_BLOB_LIST * blob_list,         //< in word order
uinT8 blank_count,               //< blanks in front
const char *text                 //< correct text
):
flags (0),
correct(text) {
  C_BLOB_IT start_it = blob_list;//iterator
  C_BLOB_IT end_it = blob_list;  //another
                                 //rejected blobs in wd
  C_BLOB_IT rej_cblob_it = &rej_cblobs;
  C_OUTLINE_IT c_outline_it;     //coutline iterator
  BOOL8 blob_inverted;
  BOOL8 reject_blob;
  inT16 inverted_vote = 0;
  inT16 non_inverted_vote = 0;

  while (!end_it.at_last ())
    end_it.forward ();           //move to last
                                 //move to our list
  cblobs.assign_to_sublist (&start_it, &end_it);
  blanks = blank_count;
  /*
    Set white on black flag for the WERD, moving any duff blobs onto the
    rej_cblobs list.
    First, walk the cblobs checking the inverse flag for each outline of each
    cblob. If a cblob has inconsistent flag settings for its different
    outlines, move the blob to the reject list. Otherwise, increment the
    appropriate w-on-b or b-on-w vote for the word.

    Now set the inversion flag for the WERD by maximum vote.

    Walk the blobs again, moving any blob whose inversion flag does not agree
    with the concencus onto the reject list.
  */
  start_it.set_to_list (&cblobs);
  if (start_it.empty ())
    return;
  for (start_it.mark_cycle_pt ();
  !start_it.cycled_list (); start_it.forward ()) {
    c_outline_it.set_to_list (start_it.data ()->out_list ());
    blob_inverted = c_outline_it.data ()->flag (COUT_INVERSE);
    reject_blob = FALSE;
    for (c_outline_it.mark_cycle_pt ();
      !c_outline_it.cycled_list () && !reject_blob;
    c_outline_it.forward ()) {
      reject_blob =
        c_outline_it.data ()->flag (COUT_INVERSE) != blob_inverted;
    }
    if (reject_blob)
      rej_cblob_it.add_after_then_move (start_it.extract ());
    else {
      if (blob_inverted)
        inverted_vote++;
      else
        non_inverted_vote++;
    }
  }

  flags.set_bit (W_INVERSE, (inverted_vote > non_inverted_vote));

  start_it.set_to_list (&cblobs);
  if (start_it.empty ())
    return;
  for (start_it.mark_cycle_pt ();
  !start_it.cycled_list (); start_it.forward ()) {
    c_outline_it.set_to_list (start_it.data ()->out_list ());
    if (c_outline_it.data ()->flag (COUT_INVERSE) != flags.bit (W_INVERSE))
      rej_cblob_it.add_after_then_move (start_it.extract ());
  }
}


/**
 * WERD::WERD
 *
 * Constructor to build a WERD from a list of BLOBs.
 * The BLOBs are not copied so the source list is emptied.
 */

WERD::WERD (                     //constructor
PBLOB_LIST * blob_list,          //< in word order
uinT8 blank_count,               //< blanks in front
const char *text                 //< correct text
):
flags (0),
correct(text) {
  PBLOB_IT start_it = blob_list; //iterator
  PBLOB_IT end_it = blob_list;   //another

  while (!end_it.at_last ())
    end_it.forward ();           //move to last
  ((PBLOB_LIST *) (&cblobs))->assign_to_sublist (&start_it, &end_it);
  //move to our list
                                 //it's a polygon
  flags.set_bit (W_POLYGON, TRUE);
  blanks = blank_count;
  //      fprintf(stderr,"Wrong constructor!!!!\n");
}


/**
 * WERD::WERD
 *
 * Constructor to build a WERD from a list of BLOBs.
 * The BLOBs are not copied so the source list is emptied.
 */

WERD::WERD (                     //constructor
PBLOB_LIST * blob_list,          //< in word order
WERD * clone                     //< sorce of flags
):flags (clone->flags), correct (clone->correct) {
  PBLOB_IT start_it = blob_list; //iterator
  PBLOB_IT end_it = blob_list;   //another

  while (!end_it.at_last ())
    end_it.forward ();           //move to last
  ((PBLOB_LIST *) (&cblobs))->assign_to_sublist (&start_it, &end_it);
  //move to our list
  blanks = clone->blanks;
  //      fprintf(stderr,"Wrong constructor!!!!\n");
}


/**
 * WERD::WERD
 *
 * Constructor to build a WERD from a list of C_BLOBs.
 * The C_BLOBs are not copied so the source list is emptied.
 */

WERD::WERD (                     //constructor
C_BLOB_LIST * blob_list,         //< in word order
WERD * clone                     //< source of flags
):flags (clone->flags), correct (clone->correct) {
  C_BLOB_IT start_it = blob_list;//iterator
  C_BLOB_IT end_it = blob_list;  //another

  while (!end_it.at_last ())
    end_it.forward ();           //move to last
  ((C_BLOB_LIST *) (&cblobs))->assign_to_sublist (&start_it, &end_it);
  //move to our list
  blanks = clone->blanks;
  //      fprintf(stderr,"Wrong constructor!!!!\n");
}


/**
 * WERD::poly_copy
 *
 * Make a copy of a WERD in polygon format.
 * The source WERD is untouched.
 */

WERD *WERD::poly_copy(               //make a poly copy
                      float xheight  //< row height
                     ) {
  PBLOB *blob;                   //new blob
  WERD *result = new WERD;       //output word
  C_BLOB_IT src_it = &cblobs;    //iterator
  //      LARC_BLOB_IT                            larc_it=(LARC_BLOB_LIST*)(&cblobs);
  PBLOB_IT dest_it = (PBLOB_LIST *) (&result->cblobs);
  //another

  if (flags.bit (W_POLYGON)) {
    *result = *this;             //just copy it
  }
  else {
    result->flags = flags;
    result->correct = correct;   //copy info
    result->dummy = dummy;
    if (!src_it.empty ()) {
      //                      if (flags.bit(W_LINEARC))
      //                      {
      //                              do
      //                              {
      //                                      blob=new PBLOB;
      //                                      poly_linearc_outlines(larc_it.data()->out_list(),
      //                                                                                              blob->out_list());      //convert outlines
      //                                      dest_it.add_after_then_move(blob);                      //add to dest list
      //                                      larc_it.forward();
      //                              }
      //                              while (!larc_it.at_first());
      //                      }
      //                      else
      //                      {
      do {
        blob = new PBLOB (src_it.data (), xheight);
        //convert blob
                                 //add to dest list
        dest_it.add_after_then_move (blob);
        src_it.forward ();
      }
      while (!src_it.at_first ());
      //                      }
    }
    if (!rej_cblobs.empty ()) {
      /* Polygonal approx of reject blobs */
      src_it.set_to_list (&rej_cblobs);
      dest_it = (PBLOB_LIST *) (&result->rej_cblobs);
      do {
                                 //convert blob
        blob = new PBLOB (src_it.data (), xheight);
                                 //add to dest list
        dest_it.add_after_then_move (blob);
        src_it.forward ();
      }
      while (!src_it.at_first ());
    }
                                 //polygon now
    result->flags.set_bit (W_POLYGON, TRUE);
    result->blanks = blanks;
  }
  return result;
}


/**
 * WERD::bounding_box
 *
 * Return the bounding box of the WERD.
 * This is quite a mess to compute!
 * ORIGINALLY, REJECT CBLOBS WERE EXCLUDED, however, this led to bugs when the
 * words on the row were re-sorted. The original words were built with reject
 * blobs included. The FUZZY SPACE flags were set accordingly. If ALL the
 * blobs in a word are rejected the BB for the word is NULL, causing the sort
 * to screw up, leading to the erroneous possibility of the first word in a
 * row being marked as FUZZY space.
 */

TBOX WERD::bounding_box() {  //bounding box
  TBOX box;                       //box being built
                                 //rejected blobs in wd
  C_BLOB_IT rej_cblob_it = &rej_cblobs;

  for (rej_cblob_it.mark_cycle_pt ();
  !rej_cblob_it.cycled_list (); rej_cblob_it.forward ()) {
    box += rej_cblob_it.data ()->bounding_box ();
  }

  if (flags.bit (W_POLYGON)) {
                                 //polygons
    PBLOB_IT it = (PBLOB_LIST *) (&cblobs);

    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      box += it.data ()->bounding_box ();
    }
  }
  else {
    C_BLOB_IT it = &cblobs;      //blobs of WERD

    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      box += it.data ()->bounding_box ();
    }
  }
  return box;
}


/**
 * WERD::move
 *
 * Reposition WERD by vector
 * NOTE!! REJECT CBLOBS ARE NOT MOVED
 */

void WERD::move(                  // reposition WERD
                const ICOORD vec  //< by vector
               ) {
  PBLOB_IT blob_it ((PBLOB_LIST *) & cblobs);
  // blob iterator
  //      LARC_BLOB_IT                            lblob_it((LARC_BLOB_LIST*)&cblobs);
  C_BLOB_IT cblob_it(&cblobs);  // cblob iterator

  if (flags.bit (W_POLYGON))
    for (blob_it.mark_cycle_pt ();
    !blob_it.cycled_list (); blob_it.forward ())
  blob_it.data ()->move (vec);
  //      else if (flags.bit(W_LINEARC))
  //              for( lblob_it.mark_cycle_pt();
  //                      !lblob_it.cycled_list();
  //                      lblob_it.forward() )
  //                      lblob_it.data()->move( vec );
  else
    for (cblob_it.mark_cycle_pt ();
    !cblob_it.cycled_list (); cblob_it.forward ())
  cblob_it.data ()->move (vec);
}


/**
 * WERD::scale
 *
 * Scale WERD by multiplier
 */

void WERD::scale(               // scale WERD
                 const float f  //< by multiplier
                ) {
  PBLOB_IT blob_it ((PBLOB_LIST *) & cblobs);
  // blob iterator
  //      LARC_BLOB_IT                            lblob_it((LARC_BLOB_LIST*)&cblobs);

  if (flags.bit (W_POLYGON))
    for (blob_it.mark_cycle_pt ();
    !blob_it.cycled_list (); blob_it.forward ())
  blob_it.data ()->scale (f);
  //      else if (flags.bit(W_LINEARC))
  //              for (lblob_it.mark_cycle_pt();
  //                              !lblob_it.cycled_list();
  //                              lblob_it.forward() )
  //                      lblob_it.data()->scale( f );
  else
    CANT_SCALE_EDGESTEPS.error ("WERD::scale", ABORT, NULL);
}


/**
 * WERD::join_on
 *
 * Join other word onto this one. Delete the old word.
 */

void WERD::join_on(              // join WERD
                   WERD *&other  //< other word
                  ) {
  PBLOB_IT blob_it ((PBLOB_LIST *) & cblobs);
  // blob iterator
  PBLOB_IT src_it ((PBLOB_LIST *) & other->cblobs);
  C_BLOB_IT rej_cblob_it(&rej_cblobs);
  C_BLOB_IT src_rej_it (&other->rej_cblobs);

  while (!src_it.empty ()) {
    blob_it.add_to_end (src_it.extract ());
    src_it.forward ();
  }
  while (!src_rej_it.empty ()) {
    rej_cblob_it.add_to_end (src_rej_it.extract ());
    src_rej_it.forward ();
  }
}


/**
 * WERD::copy_on
 *
 * Copy blobs from other word onto this one.
 */

void WERD::copy_on(              //copy blobs
                   WERD *&other  //< from other
                  ) {
  if (flags.bit (W_POLYGON)) {
    PBLOB_IT blob_it ((PBLOB_LIST *) & cblobs);
    // blob iterator
    PBLOB_LIST blobs;

    blobs.deep_copy(reinterpret_cast<PBLOB_LIST*>(&other->cblobs),
                    &PBLOB::deep_copy);
    blob_it.move_to_last();
    blob_it.add_list_after(&blobs);
  } else {
    C_BLOB_IT c_blob_it(&cblobs);
    C_BLOB_LIST c_blobs;

    c_blobs.deep_copy(&other->cblobs, &C_BLOB::deep_copy);
    c_blob_it.move_to_last ();
    c_blob_it.add_list_after (&c_blobs);
  }
  if (!other->rej_cblobs.empty ()) {
    C_BLOB_IT rej_c_blob_it(&rej_cblobs);
    C_BLOB_LIST new_rej_c_blobs;

    new_rej_c_blobs.deep_copy(&other->rej_cblobs, &C_BLOB::deep_copy);
    rej_c_blob_it.move_to_last ();
    rej_c_blob_it.add_list_after (&new_rej_c_blobs);
  }
}


/**
 * WERD::baseline_normalise
 *
 * Baseline Normalise the word in Tesseract style.  (I.e origin at centre of
 * word at bottom. x-height region scaled to region y =
 * (bln_baseline_offset)..(bln_baseline_offset + bln_x_height)
 * - usually 64..192)
 */

void WERD::baseline_normalise(                // Tess style BL Norm
                              ROW *row,
                              DENORM *denorm  //< antidote
                             ) {
  baseline_normalise_x (row, row->x_height (), denorm);
  //Use standard x ht
}


/**
 * WERD::baseline_normalise_x
 *
 * Baseline Normalise the word in Tesseract style.  (I.e origin at centre of
 * word at bottom. x-height region scaled to region y =
 * (bln_baseline_offset)..(bln_baseline_offset + bln_x_height)
 * - usually 64..192)
 *  USE A SPECIFIED X-HEIGHT - NOT NECESSARILY THE ONE IN row
 */

void WERD::baseline_normalise_x(                 // Tess style BL Norm
                                ROW *row,
                                float x_height,  //< non standard value
                                DENORM *denorm   //< antidote
                               ) {
  BOOL8 using_row;               //as baseline
  float blob_x_centre;           //middle of blob
  float blob_offset;             //bottom miss
  float top_offset;              //top miss
  float blob_x_height;           //xh for this blob
  inT16 segments;                //no of segments
  inT16 segment;                 //current segment
  DENORM_SEG *segs;              //array of segments
  float mean_x;                  //mean xheight
  inT32 x_count;                 //no of xs
  TBOX word_box = bounding_box ();//word bounding box
  TBOX blob_box;                  //blob bounding box
  PBLOB_IT blob_it ((PBLOB_LIST *) & cblobs);
  // blob iterator
  PBLOB *blob;
  LLSQ line;                     //fitted line
  double line_m, line_c;         //fitted line
                                 //inverse norm
  DENORM antidote (word_box.left () +

    (word_box.right () - word_box.left ()) / 2.0,
    bln_x_height / x_height, row);

  if (!flags.bit (W_POLYGON)) {
    WRONG_WORD.error ("WERD::baseline_normalise", ABORT,
      "Need to poly approx");
  }

  if (flags.bit (W_NORMALIZED)) {
    WRONG_WORD.error ("WERD::baseline_normalise", ABORT,
      "Baseline unnormalised");
  }

  if (bln_numericmode) {
    segs = new DENORM_SEG[blob_it.length ()];
    segments = 0;
    float factor;  // For scaling to baseline normalised size.
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      blob->move (FCOORD (-antidote.origin (),
        -blob_box.bottom ()));
      factor = bln_x_height * 4.0f / (3 * blob_box.height ());
      // Constrain the scale factor as target numbers should be either
      // cap height already or xheight.
      if (factor < antidote.scale())
        factor = antidote.scale();
      else if (factor > antidote.scale() * 1.5f)
        factor = antidote.scale() * 1.5f;
      blob->scale (factor);
      blob->move (FCOORD (0.0, bln_baseline_offset));
      segs[segments].xstart = blob->bounding_box().left();
      segs[segments].ycoord = blob_box.bottom();
      segs[segments++].scale_factor = factor;
    }
    antidote = DENORM (antidote.origin (), antidote.scale (),
      0.0f, 0.0f, segments, segs, true, row);
    delete [] segs;

                                 //Repeat for rej blobs
    blob_it.set_to_list ((PBLOB_LIST *) & rej_cblobs);
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      blob->move (FCOORD (-antidote.origin (),
                          -blob_box.bottom ()));
      blob->scale (bln_x_height * 4.0f / (3 * blob_box.height ()));
      blob->move (FCOORD (0.0, bln_baseline_offset));
    }
  }
  else if (bln_blshift_maxshift < 0) {
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      blob_x_centre = blob_box.left () +
        (blob_box.right () - blob_box.left ()) / 2.0;
      blob->move (FCOORD (-antidote.origin (),
        -(row->base_line (blob_x_centre))));
      blob->scale (antidote.scale ());
      blob->move (FCOORD (0.0, bln_baseline_offset));
    }

                                 //Repeat for rej blobs
    blob_it.set_to_list ((PBLOB_LIST *) & rej_cblobs);
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      blob_x_centre = blob_box.left () +
        (blob_box.right () - blob_box.left ()) / 2.0;
      blob->move (FCOORD (-antidote.origin (),
        -(row->base_line (blob_x_centre))));
      blob->scale (antidote.scale ());
      blob->move (FCOORD (0.0, bln_baseline_offset));
    }

  }
  else {
    mean_x = x_height;
    x_count = 1;
    segs = new DENORM_SEG[blob_it.length ()];
    segments = 0;
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      if (blob_box.height () > bln_blshift_xfraction * x_height) {
        blob_x_centre = blob_box.left () +
          (blob_box.right () - blob_box.left ()) / 2.0;
        blob_offset =
          blob_box.bottom () - row->base_line (blob_x_centre);
        top_offset = blob_offset + blob_box.height () - x_height - 1;
        blob_x_height = top_offset + x_height;
        if (top_offset < 0)
          top_offset = -top_offset;
        if (blob_offset < 0)
          blob_offset = -blob_offset;
        if (blob_offset < bln_blshift_maxshift * x_height) {
          segs[segments].ycoord = blob_box.bottom ();
          line.add (blob_x_centre, blob_box.bottom ());
          if (top_offset < bln_blshift_maxshift * x_height) {
            segs[segments].scale_factor = blob_box.height () - 1.0f;
            x_count++;
          }
          else
            segs[segments].scale_factor = 0.0f;
          //fix it later
        }
        else {
                                 //not a goer
          segs[segments].ycoord = -MAX_INT32;
          if (top_offset < bln_blshift_maxshift * x_height) {
            segs[segments].scale_factor = blob_x_height;
            x_count++;
          }
          else
            segs[segments].scale_factor = 0.0f;
          //fix it later
        }
      }
      else {
        segs[segments].scale_factor = 0.0f;
        segs[segments].ycoord = -MAX_INT32;
      }
      segs[segments].xstart = blob_box.left ();
      segments++;
    }
    using_row = line.count () <= 1;
    if (!using_row) {
      line_m = line.m ();
      line_c = line.c (line_m);
    }
    else
      line_m = line_c = 0;
    segments = 0;
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      blob_x_centre = blob_box.left () +
        (blob_box.right () - blob_box.left ()) / 2.0;
      if (segs[segments].ycoord == -MAX_INT32
      && segs[segments].scale_factor != 0 && !using_row) {
        blob_offset = line_m * blob_x_centre + line_c;
        segs[segments].scale_factor = blob_box.top () - blob_offset;
      }
      if (segs[segments].scale_factor != 0)
        mean_x += segs[segments].scale_factor;
      segments++;
    }
    mean_x /= x_count;
    //              printf("mean x=%g, count=%d, line_m=%g, line_c=%g\n",
    //                      mean_x,x_count,line_m,line_c);
    segments = 0;
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      blob_x_centre = blob_box.left () +
        (blob_box.right () - blob_box.left ()) / 2.0;
      if (segs[segments].ycoord != -MAX_INT32)
        blob_offset = (float) segs[segments].ycoord;
      else if (using_row)
        blob_offset = row->base_line (blob_x_centre);
      else
        blob_offset = line_m * blob_x_centre + line_c;
      if (segs[segments].scale_factor == 0)
        segs[segments].scale_factor = mean_x;
      segs[segments].scale_factor =
        bln_x_height / segs[segments].scale_factor;
      //                      printf("Blob sf=%g, top=%d, bot=%d, base=%g\n",
      //                              segs[segments].scale_factor,blob_box.top(),
      //                              blob_box.bottom(),blob_offset);
      blob->move (FCOORD (-antidote.origin (), -blob_offset));
      blob->
        scale (FCOORD (antidote.scale (), segs[segments].scale_factor));
      blob->move (FCOORD (0.0, bln_baseline_offset));
      segments++;
    }

                                 //Repeat for rej blobs
    blob_it.set_to_list ((PBLOB_LIST *) & rej_cblobs);
    segment = 0;
    for (blob_it.mark_cycle_pt (); !blob_it.cycled_list ();
    blob_it.forward ()) {
      blob = blob_it.data ();
      blob_box = blob->bounding_box ();
      blob_x_centre = blob_box.left () +
        (blob_box.right () - blob_box.left ()) / 2.0;
      while (segment < segments - 1
        && segs[segment + 1].xstart <= blob_x_centre)
        segment++;
      if (segs[segment].ycoord != -MAX_INT32)
        blob_offset = (float) segs[segment].ycoord;
      else if (using_row)
        blob_offset = row->base_line (blob_x_centre);
      else
        blob_offset = line_m * blob_x_centre + line_c;
      blob->move (FCOORD (-antidote.origin (), -blob_offset));
      blob->
        scale (FCOORD (antidote.scale (), segs[segment].scale_factor));
      blob->move (FCOORD (0.0, bln_baseline_offset));
    }
    if (line.count () > 0 || x_count > 1)
      antidote = DENORM (antidote.origin (), antidote.scale (),
        line_m, line_c, segments, segs, using_row, row);
    delete[]segs;
  }
  if (denorm != NULL)
    *denorm = antidote;
                                 //it's normalised
  flags.set_bit (W_NORMALIZED, TRUE);
}


/**
 * WERD::baseline_denormalise
 *
 * Baseline DeNormalise the word in Tesseract style.  (I.e origin at centre of
 * word at bottom. x-height region scaled to region y =
 * (bln_baseline_offset)..(bln_baseline_offset + bln_x_height)
 * - usually 64..192)
 */

void WERD::baseline_denormalise(                      // Tess style BL Norm
                                const DENORM *denorm  //< antidote
                               ) {
  PBLOB_IT blob_it ((PBLOB_LIST *) & cblobs);
  // blob iterator
  PBLOB *blob;

  if (!flags.bit (W_NORMALIZED)) {
    WRONG_WORD.error ("WERD::baseline_denormalise", ABORT,
      "Baseline normalised");
  }

  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
    blob = blob_it.data ();
                                 //denormalise it
    blob->baseline_denormalise (denorm);
  }

                                 //Repeat for rej blobs
  blob_it.set_to_list ((PBLOB_LIST *) & rej_cblobs);
  for (blob_it.mark_cycle_pt (); !blob_it.cycled_list (); blob_it.forward ()) {
    blob = blob_it.data ();
                                 //denormalise it
    blob->baseline_denormalise (denorm);
  }

                                 //it's not normalised
  flags.set_bit (W_NORMALIZED, FALSE);
}


/**
 * WERD::print
 *
 * Display members
 */

void WERD::print(        //print
                 FILE *  //< file to print on
                ) {
  tprintf ("Blanks= %d\n", blanks);
  bounding_box ().print ();
  tprintf ("Flags = %d = 0%o\n", flags.val, flags.val);
  tprintf ("   W_SEGMENTED = %s\n",
    flags.bit (W_SEGMENTED) ? "TRUE" : "FALSE ");
  tprintf ("   W_ITALIC = %s\n", flags.bit (W_ITALIC) ? "TRUE" : "FALSE ");
  tprintf ("   W_BOL = %s\n", flags.bit (W_BOL) ? "TRUE" : "FALSE ");
  tprintf ("   W_EOL = %s\n", flags.bit (W_EOL) ? "TRUE" : "FALSE ");
  tprintf ("   W_NORMALIZED = %s\n",
    flags.bit (W_NORMALIZED) ? "TRUE" : "FALSE ");
  tprintf ("   W_POLYGON = %s\n", flags.bit (W_POLYGON) ? "TRUE" : "FALSE ");
  tprintf ("   W_LINEARC = %s\n", flags.bit (W_LINEARC) ? "TRUE" : "FALSE ");
  tprintf ("   W_DONT_CHOP = %s\n",
    flags.bit (W_DONT_CHOP) ? "TRUE" : "FALSE ");
  tprintf ("   W_REP_CHAR = %s\n",
    flags.bit (W_REP_CHAR) ? "TRUE" : "FALSE ");
  tprintf ("   W_FUZZY_SP = %s\n",
    flags.bit (W_FUZZY_SP) ? "TRUE" : "FALSE ");
  tprintf ("   W_FUZZY_NON = %s\n",
    flags.bit (W_FUZZY_NON) ? "TRUE" : "FALSE ");
  tprintf ("Correct= %s\n", correct.string ());
  tprintf ("Rejected cblob count = %d\n", rej_cblobs.length ());
}


/**
 * WERD::plot
 *
 * Draw the WERD in the given colour.
 */

#ifndef GRAPHICS_DISABLED
void WERD::plot(                //draw it
                ScrollView* window,  //window to draw in
                ScrollView::Color colour,  //colour to draw in
                BOOL8 solid     //draw larcs solid
               ) {
  if (flags.bit (W_POLYGON)) {
                                 //polygons
    PBLOB_IT it = (PBLOB_LIST *) (&cblobs);

    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      it.data ()->plot (window, colour, colour);
    }
  }
  //      else if (flags.bit(W_LINEARC))
  //      {
  //              LARC_BLOB_IT                    it=(LARC_BLOB_LIST*)(&cblobs);

  //              for ( it.mark_cycle_pt(); !it.cycled_list(); it.forward() )
  //              {
  //                      it.data()->plot(window,solid,colour,solid ? BLACK : colour);
  //              }
  //      }
  else {
    C_BLOB_IT it = &cblobs;      //blobs of WERD

    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      it.data ()->plot (window, colour, colour);
    }
  }
  plot_rej_blobs(window, solid);
}
#endif


/**
 * WERD::plot
 *
 * Draw the WERD in rainbow colours.
 */

#ifndef GRAPHICS_DISABLED
void WERD::plot(                //draw it
                ScrollView* window,  //< window to draw in
                BOOL8 solid     //< draw larcs solid
               ) {
  ScrollView::Color colour = FIRST_COLOUR;  //current colour
  if (flags.bit (W_POLYGON)) {
                                 //polygons
    PBLOB_IT it = (PBLOB_LIST *) (&cblobs);

    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      it.data ()->plot (window, colour, CHILD_COLOUR);
      colour = (ScrollView::Color) (colour + 1);
      if (colour == LAST_COLOUR)
        colour = FIRST_COLOUR;   //cycle round
    }
  }
  //      else if (flags.bit(W_LINEARC))
  //      {
  //              LARC_BLOB_IT                    it=(LARC_BLOB_LIST*)(&cblobs);

  //              for ( it.mark_cycle_pt(); !it.cycled_list(); it.forward() )
  //              {
  //                      it.data()->plot(window,solid,colour,solid ? BLACK : CHILD_COLOUR);
  //                      colour=(COLOUR)(colour+1);
  //                      if (colour==LAST_COLOUR)
  //                              colour=FIRST_COLOUR;
  //              }
  //      }
  else {
    C_BLOB_IT it = &cblobs;      //blobs of WERD

    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      it.data ()->plot (window, colour, CHILD_COLOUR);
      colour = (ScrollView::Color) (colour + 1);
      if (colour == LAST_COLOUR)
        colour = FIRST_COLOUR;   //cycle round
    }
  }
  plot_rej_blobs(window, solid);
}
#endif


/**
 * WERD::plot_rej_blobs
 *
 * Draw the WERD rejected blobs - ALWAYS GREY
 */

#ifndef GRAPHICS_DISABLED
void WERD::plot_rej_blobs(                //draw it
                          ScrollView* window,  //< window to draw in
                          BOOL8 solid     //< draw larcs solid
                         ) {
  if (flags.bit (W_POLYGON)) {
    PBLOB_IT it = (PBLOB_LIST *) (&rej_cblobs);
    //polygons

    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      it.data ()->plot (window, ScrollView::GREY, ScrollView::GREY);
    }
  } else {
    C_BLOB_IT it = &rej_cblobs;  //blobs of WERD

    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      it.data ()->plot (window, ScrollView::GREY, ScrollView::GREY);
    }
  }
}
#endif


/**
 * WERD::shallow_copy()
 *
 * Make a shallow copy of a word
 */

WERD *WERD::shallow_copy() {  //shallow copy
  WERD *new_word = new WERD;

  new_word->blanks = blanks;
  new_word->flags = flags;
  new_word->dummy = dummy;
  new_word->correct = correct;
  return new_word;
}


/**
 * WERD::operator=
 *
 * Assign a word, DEEP copying the blob list
 */

WERD & WERD::operator= (         //assign words
const WERD & source              //from this
) {
  this->ELIST_LINK::operator= (source);
  blanks = source.blanks;
  flags = source.flags;
  dummy = source.dummy;
  correct = source.correct;
  if (flags.bit (W_POLYGON)) {
    if (!cblobs.empty())
      reinterpret_cast<PBLOB_LIST*>(&cblobs)->clear();
    reinterpret_cast<PBLOB_LIST*>(&cblobs)->deep_copy(
      reinterpret_cast<const PBLOB_LIST*>(&source.cblobs), &PBLOB::deep_copy);

    if (!rej_cblobs.empty())
      reinterpret_cast<PBLOB_LIST*>(&rej_cblobs)->clear();
    reinterpret_cast<PBLOB_LIST*>(&rej_cblobs)->deep_copy(
      reinterpret_cast<const PBLOB_LIST*>(&source.rej_cblobs),
      &PBLOB::deep_copy);
  } else {
    if (!cblobs.empty ())
      cblobs.clear ();
    cblobs.deep_copy(&source.cblobs, &C_BLOB::deep_copy);

    if (!rej_cblobs.empty ())
      rej_cblobs.clear ();
    rej_cblobs.deep_copy(&source.rej_cblobs, &C_BLOB::deep_copy);
  }
  return *this;
}


/**
 *  word_comparator()
 *
 *  word comparator used to sort a word list so that words are in increasing
 *  order of left edge.
 */

int word_comparator(                     //sort blobs
                    const void *word1p,  //< ptr to ptr to word1
                    const void *word2p   //< ptr to ptr to word2
                   ) {
  WERD *
    word1 = *(WERD **) word1p;
  WERD *
    word2 = *(WERD **) word2p;

  return word1->bounding_box ().left () - word2->bounding_box ().left ();
}
