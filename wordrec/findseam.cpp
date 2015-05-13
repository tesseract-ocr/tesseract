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
#include "plotedges.h"
#include "outlines.h"
#include "freelist.h"
#include "seam.h"
#include "wordrec.h"

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
              F u n c t i o n s
----------------------------------------------------------------------*/
namespace tesseract {

/**********************************************************************
 * add_seam_to_queue
 *
 * Adds the given new_seam to the seams priority queue, unless it is full
 * and the new seam is worse than the worst.
 **********************************************************************/
void Wordrec::add_seam_to_queue(float new_priority, SEAM *new_seam,
                                SeamQueue* seams) {
  if (new_seam == NULL) return;
  if (chop_debug) {
    tprintf("Pushing new seam with priority %g :", new_priority);
    new_seam->Print("seam: ");
  }
  if (seams->size() >= MAX_NUM_SEAMS) {
    SeamPair old_pair(0, NULL);
    if (seams->PopWorst(&old_pair) && old_pair.key() <= new_priority) {
      if (chop_debug) {
        tprintf("Old seam staying with priority %g\n", old_pair.key());
      }
      delete new_seam;
      seams->Push(&old_pair);
      return;
    } else if (chop_debug) {
      tprintf("New seam with priority %g beats old worst seam with %g\n",
              new_priority, old_pair.key());
    }
  }
  SeamPair new_pair(new_priority, new_seam);
  seams->Push(&new_pair);
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
void Wordrec::choose_best_seam(SeamQueue *seam_queue, const SPLIT *split,
                               PRIORITY priority, SEAM **seam_result,
                               TBLOB *blob, SeamPile *seam_pile) {
  SEAM *seam;
  char str[80];
  float my_priority;
  /* Add seam of split */
  my_priority = priority;
  if (split != NULL) {
    TPOINT split_point = split->point1->pos;
    split_point += split->point2->pos;
    split_point /= 2;
    seam = new SEAM(my_priority, split_point, *split);
    if (chop_debug > 1) seam->Print("Partial priority    ");
    add_seam_to_queue(my_priority, seam, seam_queue);

    if (my_priority > chop_good_split)
      return;
  }

  TBOX bbox = blob->bounding_box();
  /* Queue loop */
  while (!seam_queue->empty()) {
    SeamPair seam_pair;
    seam_queue->Pop(&seam_pair);
    seam = seam_pair.extract_data();
    /* Set full priority */
    my_priority = seam->FullPriority(bbox.left(), bbox.right(),
                                     chop_overlap_knob, chop_centered_maxwidth,
                                     chop_center_knob, chop_width_change_knob);
    if (chop_debug) {
      sprintf (str, "Full my_priority %0.0f,  ", my_priority);
      seam->Print(str);
    }

    if ((*seam_result == NULL || (*seam_result)->priority() > my_priority) &&
        my_priority < chop_ok_split) {
      /* No crossing */
      if (seam->IsHealthy(*blob, chop_min_outline_points,
                          chop_min_outline_area)) {
        delete *seam_result;
        *seam_result = new SEAM(*seam);
        (*seam_result)->set_priority(my_priority);
      } else {
        delete seam;
        seam = NULL;
        my_priority = BAD_PRIORITY;
      }
    }

    if (my_priority < chop_good_split) {
      if (seam)
        delete seam;
      return;                    /* Made good answer */
    }

    if (seam) {
      /* Combine with others */
      if (seam_pile->size() < chop_seam_pile_size) {
        combine_seam(*seam_pile, seam, seam_queue);
        SeamDecPair pair(seam_pair.key(), seam);
        seam_pile->Push(&pair);
      } else if (chop_new_seam_pile &&
                 seam_pile->size() == chop_seam_pile_size &&
                 seam_pile->PeekTop().key() > seam_pair.key()) {
        combine_seam(*seam_pile, seam, seam_queue);
        SeamDecPair pair;
        seam_pile->Pop(&pair);  // pop the worst.
        // Replace the seam in pair (deleting the old one) with
        // the new seam and score, then push back into the heap.
        pair.set_key(seam_pair.key());
        pair.set_data(seam);
        seam_pile->Push(&pair);
      } else {
        delete seam;
      }
    }

    my_priority = seam_queue->empty() ? NO_FULL_PRIORITY
                                      : seam_queue->PeekTop().key();
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
void Wordrec::combine_seam(const SeamPile& seam_pile,
                           const SEAM* seam, SeamQueue* seam_queue) {
  for (int x = 0; x < seam_pile.size(); ++x) {
    const SEAM *this_one = seam_pile.get(x).data();
    if (seam->CombineableWith(*this_one, SPLIT_CLOSENESS, chop_ok_split)) {
      SEAM *new_one = new SEAM(*seam);
      new_one->CombineWith(*this_one);
      if (chop_debug > 1) new_one->Print("Combo priority       ");
      add_seam_to_queue(new_one->priority(), new_one, seam_queue);
    }
  }
}

/**********************************************************************
 * pick_good_seam
 *
 * Find and return a good seam that will split this blob into two pieces.
 * Work from the outlines provided.
 **********************************************************************/
SEAM *Wordrec::pick_good_seam(TBLOB *blob) {
  SeamPile seam_pile(chop_seam_pile_size);
  EDGEPT *points[MAX_NUM_POINTS];
  EDGEPT_CLIST new_points;
  SEAM *seam = NULL;
  TESSLINE *outline;
  inT16 num_points = 0;

#ifndef GRAPHICS_DISABLED
  if (chop_debug > 2)
    wordrec_display_splits.set_value(true);

  draw_blob_edges(blob);
#endif

  PointHeap point_heap(MAX_NUM_POINTS);
  for (outline = blob->outlines; outline; outline = outline->next)
    prioritize_points(outline, &point_heap);

  while (!point_heap.empty() && num_points < MAX_NUM_POINTS) {
    points[num_points++] = point_heap.PeekTop().data;
    point_heap.Pop(NULL);
  }

  /* Initialize queue */
  SeamQueue seam_queue(MAX_NUM_SEAMS);

  try_point_pairs(points, num_points, &seam_queue, &seam_pile, &seam, blob);
  try_vertical_splits(points, num_points, &new_points,
                      &seam_queue, &seam_pile, &seam, blob);

  if (seam == NULL) {
    choose_best_seam(&seam_queue, NULL, BAD_PRIORITY, &seam, blob, &seam_pile);
  } else if (seam->priority() > chop_good_split) {
    choose_best_seam(&seam_queue, NULL, seam->priority(), &seam, blob,
                     &seam_pile);
  }

  EDGEPT_C_IT it(&new_points);
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    EDGEPT *inserted_point = it.data();
    if (seam == NULL || !seam->UsesPoint(inserted_point)) {
      for (outline = blob->outlines; outline; outline = outline->next) {
        if (outline->loop == inserted_point) {
          outline->loop = outline->loop->next;
        }
      }
      remove_edgept(inserted_point);
    }
  }

  if (seam) {
    if (seam->priority() > chop_ok_split) {
      delete seam;
      seam = NULL;
    }
#ifndef GRAPHICS_DISABLED
    else if (wordrec_display_splits) {
      seam->Mark(edge_window);
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
 * try_point_pairs
 *
 * Try all the splits that are produced by pairing critical points
 * together.  See if any of them are suitable for use.  Use a seam
 * queue and seam pile that have already been initialized and used.
 **********************************************************************/
void Wordrec::try_point_pairs(EDGEPT * points[MAX_NUM_POINTS],
                              inT16 num_points,
                              SeamQueue* seam_queue,
                              SeamPile* seam_pile,
                              SEAM ** seam,
                              TBLOB * blob) {
  inT16 x;
  inT16 y;
  PRIORITY priority;

  for (x = 0; x < num_points; x++) {
    for (y = x + 1; y < num_points; y++) {
      if (points[y] &&
          points[x]->WeightedDistance(*points[y], chop_x_y_weight) <
              chop_split_length &&
          points[x] != points[y]->next && points[y] != points[x]->next &&
          !is_exterior_point(points[x], points[y]) &&
          !is_exterior_point(points[y], points[x])) {
        SPLIT split(points[x], points[y]);
        priority = partial_split_priority(&split);

        choose_best_seam(seam_queue, &split, priority, seam, blob, seam_pile);
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
 * Return in new_points a collection of points that were inserted into
 * the blob while examining vertical splits and which may safely be
 * removed once a seam is chosen if they are not part of the seam.
 **********************************************************************/
void Wordrec::try_vertical_splits(EDGEPT * points[MAX_NUM_POINTS],
                                  inT16 num_points,
                                  EDGEPT_CLIST *new_points,
                                  SeamQueue* seam_queue,
                                  SeamPile* seam_pile,
                                  SEAM ** seam,
                                  TBLOB * blob) {
  EDGEPT *vertical_point = NULL;
  inT16 x;
  PRIORITY priority;
  TESSLINE *outline;

  for (x = 0; x < num_points; x++) {
    vertical_point = NULL;
    for (outline = blob->outlines; outline; outline = outline->next) {
      vertical_projection_point(points[x], outline->loop,
                                &vertical_point, new_points);
    }

    if (vertical_point && points[x] != vertical_point->next &&
        vertical_point != points[x]->next &&
        points[x]->WeightedDistance(*vertical_point, chop_x_y_weight) <
            chop_split_length) {
      SPLIT split(points[x], vertical_point);
      priority = partial_split_priority(&split);
      choose_best_seam(seam_queue, &split, priority, seam, blob, seam_pile);
    }
  }
}

}
