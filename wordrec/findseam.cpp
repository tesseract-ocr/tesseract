/* -*-C-*-
 ********************************************************************************
 *
 * File:        findseam.c  (Formerly findseam.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul 30 15:44:59 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 *********************************************************************************/
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "findseam.h"
#include "gradechop.h"
#include "olutil.h"
#include "plotedges.h"
#include "outlines.h"
#include "freelist.h"
#include "seam.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
#define SPLIT_CLOSENESS        20/* Difference in x value */
                                 /* How many to keep */
#define MAX_NUM_SEAMS          150
                                 /* How many to keep */
#define MAX_OLD_SEAMS          150
#define NO_FULL_PRIORITY       -1/* Special marker for pri. */
                                 /* Evalute right away */
#define BAD_PRIORITY           9999.0

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * add_seam_to_queue
 *
 * Add this seam value to the seam queue.  If the heap is already full
 * then nothing is done.
 **********************************************************************/

#define add_seam_to_queue(seams,seam,priority)  \
if (seam)\
{\
		if (HeapFull(seams))\
			junk_worst_seam(seams,seam,priority);\
		else\
			HeapPush (seams, priority, (char*) seam);\
	}

/**********************************************************************
 * best_seam_priority
 *
 * Return the best priority value on the queue.
 **********************************************************************/

#define best_seam_priority(seam_queue)   \
(HeapEmpty (seam_queue) ?              \
	NO_FULL_PRIORITY       :              \
	((SEAM*) seam_queue_element(seam_queue, 0))->priority)

/**********************************************************************
 * create_seam_queue
 *
 * Create a new seam queue with no elements in it.
 **********************************************************************/

#define create_seam_queue(seam_queue)     \
(seam_queue = MakeHeap (MAX_NUM_SEAMS))

/**********************************************************************
 * create_seam_pile
 *
 * Create a new seam pile with no elements in it.
 **********************************************************************/

#define create_seam_pile(seam_pile)     \
(seam_pile = array_new (MAX_OLD_SEAMS))

/**********************************************************************
 * delete_seam_queue
 *
 * Delete a seam queue along with all the seam structures associated
 * with it.
 **********************************************************************/

#define delete_seam_queue(seam_queue)      \
(FreeHeapData (seam_queue, delete_seam), \
	seam_queue = NULL)                      \


/**********************************************************************
 * pop_next_seam
 *
 * Remove the next seam from the queue.  Put the seam and priority
 * values in the requested variables.  If there was nothing to pop
 * then return FALSE, else return TRUE.
 **********************************************************************/

#define pop_next_seam(seams,seam,priority)  \
(HeapPop (seams,&priority,&seam) == OK)   \


/**********************************************************************
 * seam_queue_element
 *
 * Return the element from the seam queue at the requested index.
 **********************************************************************/

#define seam_queue_element(seam_queue,index)  \
((index < SizeOfHeap (seam_queue)) ?        \
	HeapDataFor (seam_queue, index)   :        \
	NULL)                                      \


/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * junk_worst_seam
 *
 * Delete the worst seam from the queue because it is full.
 **********************************************************************/
void junk_worst_seam(SEAM_QUEUE seams, SEAM *new_seam, float new_priority) {
  SEAM *seam;
  float priority;

  HeapPopWorst(seams, &priority, &seam);
  if (priority > new_priority) {
    delete_seam(seam);  /*get rid of it */
    HeapPush (seams, new_priority, (char *) new_seam);
  }
  else {
    delete_seam(new_seam);
    HeapPush (seams, priority, (char *) seam);
  }
}


/**********************************************************************
 * choose_best_seam
 *
 * Choose the best seam that can be created by assembling this a
 * collection of splits.  A queue of all the possible seams is
 * maintained.  Each new split received is placed in that queue with
 * its partial priority value.  These values in the seam queue are
 * evaluated and combined until a good enough seam is found.  If no
 * further good seams are being found then this function returns to the
 * caller, who will send more splits.  If this function is called with
 * a split of NULL, then no further splits can be supplied by the
 * caller.
 **********************************************************************/
void choose_best_seam(SEAM_QUEUE seam_queue,
                      SEAM_PILE *seam_pile,
                      SPLIT *split,
                      PRIORITY priority,
                      SEAM **seam_result,
                      TBLOB *blob) {
  SEAM *seam;
  TPOINT topleft;
  TPOINT botright;
  char str[80];
  float my_priority;
  /* Add seam of split */
  my_priority = priority;
  if (split != NULL) {
    seam = new_seam (my_priority,
      (split->point1->pos.x + split->point1->pos.x) / 2,
      split, NULL, NULL);
    if (chop_debug > 1)
      print_seam ("Partial priority    ", seam);
    add_seam_to_queue (seam_queue, seam, (float) my_priority);

    if (my_priority > chop_good_split)
      return;
  }

  blob_bounding_box(blob, &topleft, &botright);
  /* Queue loop */
  while (pop_next_seam (seam_queue, seam, my_priority)) {
    /* Set full priority */
    my_priority = seam_priority (seam, topleft.x, botright.x);
    if (chop_debug) {
      sprintf (str, "Full my_priority %0.0f,  ", my_priority);
      print_seam(str, seam);
    }

    if ((*seam_result == NULL || /* Replace answer */
    (*seam_result)->priority > my_priority) && my_priority < chop_ok_split) {
      /* No crossing */
      if (constrained_split (seam->split1, blob)) {
        delete_seam(*seam_result);
        clone_seam(*seam_result, seam);
        (*seam_result)->priority = my_priority;
      }
      else {
        delete_seam(seam);
        seam = NULL;
        my_priority = BAD_PRIORITY;
      }
    }

    if (my_priority < chop_good_split) {
      if (seam)
        delete_seam(seam);
      return;                    /* Made good answer */
    }

    if (seam) {
                                 /* Combine with others */
      if (array_count (*seam_pile) < MAX_NUM_SEAMS
      /*|| tessedit_truncate_chopper==0 */ ) {
        combine_seam(seam_queue, *seam_pile, seam);
        *seam_pile = array_push (*seam_pile, seam);
      }
      else
        delete_seam(seam);
    }

    my_priority = best_seam_priority (seam_queue);
    if ((my_priority > chop_ok_split) ||
      (my_priority > chop_good_split && split))
      return;
  }
}


/**********************************************************************
 * combine_seam
 *
 * Find other seams to combine with this one.  The new seams that result
 * from this union should be added to the seam queue.  The return value
 * tells whether or not any additional seams were added to the queue.
 **********************************************************************/
void combine_seam(SEAM_QUEUE seam_queue, SEAM_PILE seam_pile, SEAM *seam) {
  register inT16 x;
  register inT16 dist;
  inT16 bottom1, top1;
  inT16 bottom2, top2;

  SEAM *new_one;
  SEAM *this_one;

  bottom1 = seam->split1->point1->pos.y;
  if (seam->split1->point2->pos.y >= bottom1)
    top1 = seam->split1->point2->pos.y;
  else {
    top1 = bottom1;
    bottom1 = seam->split1->point2->pos.y;
  }
  if (seam->split2 != NULL) {
    bottom2 = seam->split2->point1->pos.y;
    if (seam->split2->point2->pos.y >= bottom2)
      top2 = seam->split2->point2->pos.y;
    else {
      top2 = bottom2;
      bottom2 = seam->split2->point2->pos.y;
    }
  }
  else {
    bottom2 = bottom1;
    top2 = top1;
  }
  array_loop(seam_pile, x) {
    this_one = (SEAM *) array_value (seam_pile, x);
    dist = seam->location - this_one->location;
    if (-SPLIT_CLOSENESS < dist &&
      dist < SPLIT_CLOSENESS &&
    seam->priority + this_one->priority < chop_ok_split) {
      inT16 split1_point1_y = this_one->split1->point1->pos.y;
      inT16 split1_point2_y = this_one->split1->point2->pos.y;
      inT16 split2_point1_y = 0;
      inT16 split2_point2_y = 0;
      if (this_one->split2) {
        split2_point1_y = this_one->split2->point1->pos.y;
        split2_point2_y = this_one->split2->point2->pos.y;
      }
      if (
        /*!tessedit_fix_sideways_chops || */
        (
          /* this_one->split1 always exists */
          (
            ((split1_point1_y >= top1 && split1_point2_y >= top1) ||
             (split1_point1_y <= bottom1 && split1_point2_y <= bottom1))
            &&
            ((split1_point1_y >= top2 && split1_point2_y >= top2) ||
             (split1_point1_y <= bottom2 && split1_point2_y <= bottom2))
          )
        )
        &&
        (
          this_one->split2 == NULL ||
          (
            ((split2_point1_y >= top1 && split2_point2_y >= top1) ||
             (split2_point1_y <= bottom1 && split2_point2_y <= bottom1))
            &&
            ((split2_point1_y >= top2 && split2_point2_y >= top2) ||
             (split2_point1_y <= bottom2 && split2_point2_y <= bottom2))
          )
        )
      ) {
        new_one = join_two_seams (seam, this_one);
        if (chop_debug > 1)
          print_seam ("Combo priority       ", new_one);
        add_seam_to_queue (seam_queue, new_one, new_one->priority);
      }
    }
  }
}


/**********************************************************************
 * constrained_split
 *
 * Constrain this split to obey certain rules.  It must not cross any
 * inner outline.  It must not cut off a small chunk of the outline.
 **********************************************************************/
inT16 constrained_split(SPLIT *split, TBLOB *blob) {
  TESSLINE *outline;

  if (is_little_chunk (split->point1, split->point2))
    return (FALSE);

  for (outline = blob->outlines; outline; outline = outline->next) {
    if (split_bounds_overlap (split, outline) &&
    crosses_outline (split->point1, split->point2, outline->loop)) {
      return (FALSE);
    }
  }
  return (TRUE);
}


/**********************************************************************
 * delete_seam_pile
 *
 * Delete the seams that are held in the seam pile.  Destroy the splits
 * that are referenced by these seams.
 **********************************************************************/
void delete_seam_pile(SEAM_PILE seam_pile) {
  inT16 x;

  array_loop(seam_pile, x) {
    delete_seam ((SEAM *) array_value (seam_pile, x));
  }
  array_free(seam_pile);
}


/**********************************************************************
 * pick_good_seam
 *
 * Find and return a good seam that will split this blob into two pieces.
 * Work from the outlines provided.
 **********************************************************************/
SEAM *pick_good_seam(TBLOB *blob) {
  SEAM_QUEUE seam_queue;
  SEAM_PILE seam_pile;
  POINT_GROUP point_heap;
  PRIORITY priority;
  EDGEPT *edge;
  EDGEPT *points[MAX_NUM_POINTS];
  SEAM *seam = NULL;
  TESSLINE *outline;
  inT16 num_points = 0;

#ifndef GRAPHICS_DISABLED
  if (chop_debug > 2)
    wordrec_display_splits.set_value(true);

  draw_blob_edges(blob);
#endif

  point_heap = MakeHeap (MAX_NUM_POINTS);
  for (outline = blob->outlines; outline; outline = outline->next)
    prioritize_points(outline, point_heap);

  while (HeapPop (point_heap, &priority, &edge) == OK) {
    if (num_points < MAX_NUM_POINTS)
      points[num_points++] = (EDGEPT *) edge;
  }
  FreeHeap(point_heap);

  /* Initialize queue & pile */
  create_seam_pile(seam_pile);
  create_seam_queue(seam_queue);

  try_point_pairs(points, num_points, seam_queue, &seam_pile, &seam, blob);

  try_vertical_splits(points, num_points, seam_queue, &seam_pile, &seam, blob);

  if (seam == NULL) {
    choose_best_seam(seam_queue, &seam_pile, NULL, BAD_PRIORITY, &seam, blob);
  }
  else if (seam->priority > chop_good_split) {
    choose_best_seam (seam_queue, &seam_pile, NULL, seam->priority,
      &seam, blob);
  }
  delete_seam_queue(seam_queue);
  delete_seam_pile(seam_pile);

  if (seam) {
    if (seam->priority > chop_ok_split) {
      delete_seam(seam);
      seam = NULL;
    }
#ifndef GRAPHICS_DISABLED
    else if (wordrec_display_splits) {
      if (seam->split1)
        mark_split (seam->split1);
      if (seam->split2)
        mark_split (seam->split2);
      if (seam->split3)
        mark_split (seam->split3);
      if (chop_debug > 2) {
        update_edge_window();
        edge_window_wait();
      }
    }
#endif
  }

  if (chop_debug)
    wordrec_display_splits.set_value(false);

  return (seam);
}


/**********************************************************************
 * seam_priority
 *
 * Assign a full priority value to the seam.
 **********************************************************************/
PRIORITY seam_priority(SEAM *seam, inT16 xmin, inT16 xmax) {
  PRIORITY priority;

  if (seam->split1 == NULL)
    priority = 0;

  else if (seam->split2 == NULL) {
    priority = (seam->priority +
      full_split_priority (seam->split1, xmin, xmax));
  }

  else if (seam->split3 == NULL) {
    split_outline (seam->split2->point1, seam->split2->point2);
    priority = (seam->priority +
      full_split_priority (seam->split1, xmin, xmax));
    unsplit_outlines (seam->split2->point1, seam->split2->point2);
  }

  else {
    split_outline (seam->split2->point1, seam->split2->point2);
    split_outline (seam->split3->point1, seam->split3->point2);
    priority = (seam->priority +
      full_split_priority (seam->split1, xmin, xmax));
    unsplit_outlines (seam->split3->point1, seam->split3->point2);
    unsplit_outlines (seam->split2->point1, seam->split2->point2);
  }

  return (priority);
}


/**********************************************************************
 * try_point_pairs
 *
 * Try all the splits that are produced by pairing critical points
 * together.  See if any of them are suitable for use.  Use a seam
 * queue and seam pile that have already been initialized and used.
 **********************************************************************/
void
try_point_pairs (EDGEPT * points[MAX_NUM_POINTS],
inT16 num_points,
SEAM_QUEUE seam_queue,
SEAM_PILE * seam_pile, SEAM ** seam, TBLOB * blob) {
  inT16 x;
  inT16 y;
  SPLIT *split;
  PRIORITY priority;

  for (x = 0; x < num_points; x++) {
    for (y = x + 1; y < num_points; y++) {

      if (points[y] &&
          weighted_edgept_dist(points[x], points[y],
                               chop_x_y_weight) < chop_split_length &&
          points[x] != points[y]->next &&
          points[y] != points[x]->next &&
          !is_exterior_point(points[x], points[y]) &&
          !is_exterior_point(points[y], points[x])) {
        split = new_split (points[x], points[y]);
        priority = partial_split_priority (split);

        choose_best_seam(seam_queue, seam_pile, split, priority, seam, blob);

        if (*seam && (*seam)->priority < chop_good_split)
          return;
      }
    }
  }

}


/**********************************************************************
 * try_vertical_splits
 *
 * Try all the splits that are produced by vertical projection to see
 * if any of them are suitable for use.  Use a seam queue and seam pile
 * that have already been initialized and used.
 **********************************************************************/
void
try_vertical_splits (EDGEPT * points[MAX_NUM_POINTS],
inT16 num_points,
SEAM_QUEUE seam_queue,
SEAM_PILE * seam_pile, SEAM ** seam, TBLOB * blob) {
  EDGEPT *vertical_point = NULL;
  SPLIT *split;
  inT16 x;
  PRIORITY priority;
  TESSLINE *outline;

  for (x = 0; x < num_points; x++) {

    if (*seam != NULL && (*seam)->priority < chop_good_split)
      return;

    vertical_point = NULL;
    for (outline = blob->outlines; outline; outline = outline->next) {
      vertical_projection_point (points[x],
        outline->loop, &vertical_point);
    }

    if (vertical_point &&
      points[x] != vertical_point->next &&
      vertical_point != points[x]->next &&
      weighted_edgept_dist(points[x], vertical_point,
                           chop_x_y_weight) < chop_split_length) {

      split = new_split (points[x], vertical_point);
      priority = partial_split_priority (split);

      choose_best_seam(seam_queue, seam_pile, split, priority, seam, blob);
    }
  }
}
