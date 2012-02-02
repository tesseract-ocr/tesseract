/* -*-C-*-
 ********************************************************************************
 *
 * File:        pieces.c  (Formerly pieces.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon May 20 12:12:35 1991 (Mark Seaman) marks@hpgrlt
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

#include "blobs.h"
#include "freelist.h"
#include "helpers.h"
#include "matchtab.h"
#include "matrix.h"
#include "ndminx.h"
#include "plotseg.h"
#include "ratngs.h"
#include "seam.h"
#include "states.h"
#include "wordclass.h"
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/


/**********************************************************************
 * bounds_of_piece
 *
 * Find the bounds of the piece that will be created by joining the
 * requested collection of pieces together.
 **********************************************************************/
TBOX bounds_of_piece(TBOX *bounds, inT16 start, inT16 end) {
  TBOX all_together = bounds[start];
  for (int x = start + 1; x <= end; x++) {
    all_together += bounds[x];
  }
  return all_together;
}


/**********************************************************************
 * classify_piece
 *
 * Create a larger piece from a collection of smaller ones.  Classify
 * it and return the results.  Take the large piece apart to leave
 * the collection of small pieces un modified.
 **********************************************************************/
namespace tesseract {
BLOB_CHOICE_LIST *Wordrec::classify_piece(TBLOB *pieces,
                                          const DENORM& denorm,
                                          SEAMS seams,
                                          inT16 start,
                                          inT16 end,
                                          BlamerBundle *blamer_bundle) {
  BLOB_CHOICE_LIST *choices;
  TBLOB *blob;
  inT16 x;

  join_pieces(pieces, seams, start, end);
  for (blob = pieces, x = 0; x < start; x++) {
    blob = blob->next;
  }
  choices = classify_blob(blob, denorm, "pieces:", White, blamer_bundle);

  break_pieces(blob, seams, start, end);
#ifndef GRAPHICS_DISABLED
  if (wordrec_display_segmentations > 2) {
    STATE current_state;
    SEARCH_STATE chunk_groups;
    set_n_ones (&current_state, array_count(seams));
    chunk_groups = bin_to_chunks(&current_state, array_count(seams));
    display_segmentation(pieces, chunk_groups);
    window_wait(segm_window);
    memfree(chunk_groups);
  }
#endif

  return (choices);
}

template<class BLOB_CHOICE>
int SortByUnicharID(const void *void1, const void *void2) {
  const BLOB_CHOICE *p1 = *reinterpret_cast<const BLOB_CHOICE * const *>(void1);
  const BLOB_CHOICE *p2 = *reinterpret_cast<const BLOB_CHOICE * const *>(void2);

  return p1->unichar_id() - p2->unichar_id();
}

template<class BLOB_CHOICE>
int SortByRating(const void *void1, const void *void2) {
  const BLOB_CHOICE *p1 = *reinterpret_cast<const BLOB_CHOICE * const *>(void1);
  const BLOB_CHOICE *p2 = *reinterpret_cast<const BLOB_CHOICE * const *>(void2);

  if (p1->rating() < p2->rating())
    return 1;
  return -1;
}


/**********************************************************************
 * fill_filtered_fragment_list
 *
 * Filter the fragment list so that the filtered_choices only contain
 * fragments that are in the correct position. choices is the list
 * that we are going to filter. fragment_pos is the position in the
 * fragment that we are looking for and num_frag_parts is the the
 * total number of pieces. The result will be appended to
 * filtered_choices.
 **********************************************************************/
void Wordrec::fill_filtered_fragment_list(BLOB_CHOICE_LIST *choices,
                                          int fragment_pos,
                                          int num_frag_parts,
                                          BLOB_CHOICE_LIST *filtered_choices) {
  BLOB_CHOICE_IT filtered_choices_it(filtered_choices);
  BLOB_CHOICE_IT choices_it(choices);

  for (choices_it.mark_cycle_pt(); !choices_it.cycled_list();
       choices_it.forward()) {
    UNICHAR_ID choice_unichar_id = choices_it.data()->unichar_id();
    const CHAR_FRAGMENT *frag = unicharset.get_fragment(choice_unichar_id);

    if (frag != NULL && frag->get_pos() == fragment_pos &&
        frag->get_total() == num_frag_parts) {
      // Recover the unichar_id of the unichar that this fragment is
      // a part of
      BLOB_CHOICE *b = new BLOB_CHOICE(*choices_it.data());
      int original_unichar = unicharset.unichar_to_id(frag->get_unichar());
      b->set_unichar_id(original_unichar);
      filtered_choices_it.add_to_end(b);
    }
  }

  filtered_choices->sort(SortByUnicharID<BLOB_CHOICE>);
}


/**********************************************************************
 * merge_and_put_fragment_lists
 *
 * Merge the fragment lists in choice_lists and append it to the
 * ratings matrix.
 **********************************************************************/
void Wordrec::merge_and_put_fragment_lists(inT16 row, inT16 column,
                                           inT16 num_frag_parts,
                                           BLOB_CHOICE_LIST *choice_lists,
                                           MATRIX *ratings) {
  BLOB_CHOICE_IT *choice_lists_it = new BLOB_CHOICE_IT[num_frag_parts];

  for (int i = 0; i < num_frag_parts; i++) {
    choice_lists_it[i].set_to_list(&choice_lists[i]);
    choice_lists_it[i].mark_cycle_pt();
  }

  BLOB_CHOICE_LIST *merged_choice = ratings->get(row, column);
  if (merged_choice == NULL)
    merged_choice = new BLOB_CHOICE_LIST;

  bool end_of_list = false;
  BLOB_CHOICE_IT merged_choice_it(merged_choice);
  while (!end_of_list) {
    // Find the maximum unichar_id of the current entry the iterators
    // are pointing at
    UNICHAR_ID max_unichar_id = choice_lists_it[0].data()->unichar_id();
    int max_list = 0;
    for (int i = 0; i < num_frag_parts; i++) {
      UNICHAR_ID unichar_id = choice_lists_it[i].data()->unichar_id();
      if (max_unichar_id < unichar_id) {
        max_unichar_id = unichar_id;
        max_list = i;
      }
    }

    // Move the each iterators until it gets to an entry that has a
    // value greater than or equal to max_unichar_id
    for (int i = 0; i < num_frag_parts; i++) {
      UNICHAR_ID unichar_id = choice_lists_it[i].data()->unichar_id();
      while (!choice_lists_it[i].cycled_list() &&
             unichar_id < max_unichar_id) {
        choice_lists_it[i].forward();
        unichar_id = choice_lists_it[i].data()->unichar_id();
      }
      if (choice_lists_it[i].cycled_list()) {
        end_of_list = true;
        break;
      }
    }

    if (end_of_list)
      break;

    // Checks if the fragments are parts of the same character
    UNICHAR_ID first_unichar_id = choice_lists_it[0].data()->unichar_id();
    bool same_unichar = true;
    for (int i = 1; i < num_frag_parts; i++) {
      UNICHAR_ID unichar_id = choice_lists_it[i].data()->unichar_id();
      if (unichar_id != first_unichar_id) {
        same_unichar = false;
        break;
      }
    }

    if (same_unichar) {
      // Add the merged character to the result
      UNICHAR_ID merged_unichar_id = first_unichar_id;
      inT16 merged_fontinfo_id = choice_lists_it[0].data()->fontinfo_id();
      inT16 merged_fontinfo_id2 = choice_lists_it[0].data()->fontinfo_id2();
      inT16 merged_min_xheight = choice_lists_it[0].data()->min_xheight();
      inT16 merged_max_xheight = choice_lists_it[0].data()->max_xheight();
      int merged_script_id = choice_lists_it[0].data()->script_id();
      bool merged_adapted = choice_lists_it[0].data()->adapted();

      float merged_rating = 0, merged_certainty = 0;
      for (int i = 0; i < num_frag_parts; i++) {
        float rating = choice_lists_it[i].data()->rating();
        float certainty = choice_lists_it[i].data()->certainty();

        if (i == 0 || certainty < merged_certainty)
          merged_certainty = certainty;
        merged_rating += rating;

        choice_lists_it[i].forward();
        if (choice_lists_it[i].cycled_list())
          end_of_list = true;
        IntersectRange(choice_lists_it[i].data()->min_xheight(),
                       choice_lists_it[i].data()->max_xheight(),
                       &merged_min_xheight, &merged_max_xheight);
      }

      merged_choice_it.add_to_end(new BLOB_CHOICE(merged_unichar_id,
                                                  merged_rating,
                                                  merged_certainty,
                                                  merged_fontinfo_id,
                                                  merged_fontinfo_id2,
                                                  merged_script_id,
                                                  merged_min_xheight,
                                                  merged_max_xheight,
                                                  merged_adapted));
    }
  }

  if (classify_debug_level)
    print_ratings_list("Merged Fragments", merged_choice,
                       unicharset);

  if (merged_choice->empty())
    delete merged_choice;
  else
    ratings->put(row, column, merged_choice);

  delete [] choice_lists_it;
}


/**********************************************************************
 * get_fragment_lists
 *
 * Recursively go through the ratings matrix to find lists of fragments
 * to be merged in the function merge_and_put_fragment_lists.
 * current_frag is the postion of the piece we are looking for.
 * current_row is the row in the rating matrix we are currently at.
 * start is the row we started initially, so that we can know where
 * to append the results to the matrix. num_frag_parts is the total
 * number of pieces we are looking for and num_blobs is the size of the
 * ratings matrix.
 **********************************************************************/
void Wordrec::get_fragment_lists(inT16 current_frag, inT16 current_row,
                                 inT16 start, inT16 num_frag_parts,
                                 inT16 num_blobs, MATRIX *ratings,
                                 BLOB_CHOICE_LIST *choice_lists) {
  if (current_frag == num_frag_parts) {
    merge_and_put_fragment_lists(start, current_row - 1, num_frag_parts,
                                 choice_lists, ratings);
    return;
  }

  for (inT16 x = current_row; x < num_blobs; x++) {
    BLOB_CHOICE_LIST *choices = ratings->get(current_row, x);
    if (choices == NULL)
      continue;

    fill_filtered_fragment_list(choices, current_frag, num_frag_parts,
                                &choice_lists[current_frag]);
    if (!choice_lists[current_frag].empty()) {
      get_fragment_lists(current_frag + 1, x + 1, start, num_frag_parts,
                         num_blobs, ratings, choice_lists);
      choice_lists[current_frag].clear();
    }
  }
}


/**********************************************************************
 * merge_fragments
 *
 * Try to merge fragments in the ratings matrix and put the result in
 * the corresponding row and column
 **********************************************************************/
void Wordrec::merge_fragments(MATRIX *ratings, inT16 num_blobs) {
  BLOB_CHOICE_LIST choice_lists[CHAR_FRAGMENT::kMaxChunks];
  for (inT16 start = 0; start < num_blobs; start++) {
    for (int frag_parts = 2; frag_parts <= CHAR_FRAGMENT::kMaxChunks;
         frag_parts++) {
      get_fragment_lists(0, start, start, frag_parts, num_blobs,
                         ratings, choice_lists);
    }
  }

  // Delete fragments from the rating matrix
  for (inT16 x = 0; x < num_blobs; x++) {
    for (inT16 y = x; y < num_blobs; y++) {
      BLOB_CHOICE_LIST *choices = ratings->get(x, y);
      if (choices != NULL) {
        BLOB_CHOICE_IT choices_it(choices);
        for (choices_it.mark_cycle_pt(); !choices_it.cycled_list();
             choices_it.forward()) {
          UNICHAR_ID choice_unichar_id = choices_it.data()->unichar_id();
          const CHAR_FRAGMENT *frag =
              unicharset.get_fragment(choice_unichar_id);
          if (frag != NULL)
            delete choices_it.extract();
        }
      }
    }
  }
}


/**********************************************************************
 * get_piece_rating
 *
 * Check to see if this piece has already been classified.  If it has
 * return that rating.  Otherwise build the piece from the smaller
 * pieces, classify it, store the rating for later, and take the piece
 * apart again.
 **********************************************************************/
BLOB_CHOICE_LIST *Wordrec::get_piece_rating(MATRIX *ratings,
                                            TBLOB *blobs,
                                            const DENORM& denorm,
                                            SEAMS seams,
                                            inT16 start,
                                            inT16 end,
                                            BlamerBundle *blamer_bundle) {
  BLOB_CHOICE_LIST *choices = ratings->get(start, end);
  if (choices == NOT_CLASSIFIED) {
    choices = classify_piece(blobs,
                             denorm,
                             seams,
                             start,
                             end,
                             blamer_bundle);
    ratings->put(start, end, choices);
    if (wordrec_debug_level > 1) {
      tprintf("get_piece_rating(): updated ratings matrix\n");
      ratings->print(getDict().getUnicharset());
    }
  }
  return (choices);
}


/**********************************************************************
 * record_blob_bounds
 *
 * Set up and initialize an array that holds the bounds of a set of
 * blobs.  Caller should delete[] the array.
 **********************************************************************/
TBOX *Wordrec::record_blob_bounds(TBLOB *blobs) {
  int nblobs = count_blobs(blobs);
  TBOX *bboxes = new TBOX[nblobs];

  inT16 x = 0;
  for (TBLOB* blob = blobs; blob != NULL; blob = blob->next) {
    bboxes[x] = blob->bounding_box();
    x++;
  }
  return bboxes;
}


/**********************************************************************
 * record_piece_ratings
 *
 * Save the choices for all the pieces that have been classified into
 * a matrix that can be used to look them up later.  A two dimensional
 * matrix is created.  The indices correspond to the starting and
 * ending initial piece number.
 **********************************************************************/
MATRIX *Wordrec::record_piece_ratings(TBLOB *blobs) {
  inT16 num_blobs = count_blobs(blobs);
  TBOX *bounds = record_blob_bounds(blobs);
  MATRIX *ratings = new MATRIX(num_blobs);

  for (int x = 0; x < num_blobs; x++) {
    for (int y = x; y < num_blobs; y++) {
      TBOX piecebox = bounds_of_piece(bounds, x, y);
      BLOB_CHOICE_LIST *choices = blob_match_table.get_match_by_box(piecebox);
      if (choices != NULL) {
        ratings->put(x, y, choices);
      }
    }
  }

  if (merge_fragments_in_matrix)
    merge_fragments(ratings, num_blobs);

  delete []bounds;
  return ratings;
}

}  // namespace tesseract
