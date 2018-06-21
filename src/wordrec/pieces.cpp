/* -*-C-*-
 ********************************************************************************
 *
 * File:         pieces.cpp  (Formerly pieces.c)
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
#include "helpers.h"
#include "matrix.h"
#include "ratngs.h"
#include "seam.h"
#include "wordrec.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

using tesseract::ScoredFont;

/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/

/**********************************************************************
 * classify_piece
 *
 * Create a larger piece from a collection of smaller ones.  Classify
 * it and return the results.  Take the large piece apart to leave
 * the collection of small pieces un modified.
 **********************************************************************/
namespace tesseract {
BLOB_CHOICE_LIST *Wordrec::classify_piece(const GenericVector<SEAM*>& seams,
                                          int16_t start,
                                          int16_t end,
                                          const char* description,
                                          TWERD *word,
                                          BlamerBundle *blamer_bundle) {
  if (end > start) SEAM::JoinPieces(seams, word->blobs, start, end);
  BLOB_CHOICE_LIST *choices = classify_blob(word->blobs[start], description,
                                            White, blamer_bundle);
  // Set the matrix_cell_ entries in all the BLOB_CHOICES.
  BLOB_CHOICE_IT bc_it(choices);
  for (bc_it.mark_cycle_pt(); !bc_it.cycled_list(); bc_it.forward()) {
    bc_it.data()->set_matrix_cell(start, end);
  }

  if (end > start) SEAM::BreakPieces(seams, word->blobs, start, end);

  return (choices);
}

template<class BLOB_CHOICE>
int SortByUnicharID(const void *void1, const void *void2) {
  const BLOB_CHOICE *p1 = *static_cast<const BLOB_CHOICE *const *>(void1);
  const BLOB_CHOICE *p2 = *static_cast<const BLOB_CHOICE *const *>(void2);

  return p1->unichar_id() - p2->unichar_id();
}

template<class BLOB_CHOICE>
int SortByRating(const void *void1, const void *void2) {
  const BLOB_CHOICE *p1 = *static_cast<const BLOB_CHOICE *const *>(void1);
  const BLOB_CHOICE *p2 = *static_cast<const BLOB_CHOICE *const *>(void2);

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

    if (frag != nullptr && frag->get_pos() == fragment_pos &&
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
void Wordrec::merge_and_put_fragment_lists(int16_t row, int16_t column,
                                           int16_t num_frag_parts,
                                           BLOB_CHOICE_LIST *choice_lists,
                                           MATRIX *ratings) {
  BLOB_CHOICE_IT *choice_lists_it = new BLOB_CHOICE_IT[num_frag_parts];

  for (int i = 0; i < num_frag_parts; i++) {
    choice_lists_it[i].set_to_list(&choice_lists[i]);
    choice_lists_it[i].mark_cycle_pt();
  }

  BLOB_CHOICE_LIST *merged_choice = ratings->get(row, column);
  if (merged_choice == nullptr)
    merged_choice = new BLOB_CHOICE_LIST;

  bool end_of_list = false;
  BLOB_CHOICE_IT merged_choice_it(merged_choice);
  while (!end_of_list) {
    // Find the maximum unichar_id of the current entry the iterators
    // are pointing at
    UNICHAR_ID max_unichar_id = choice_lists_it[0].data()->unichar_id();
    for (int i = 0; i < num_frag_parts; i++) {
      UNICHAR_ID unichar_id = choice_lists_it[i].data()->unichar_id();
      if (max_unichar_id < unichar_id) {
        max_unichar_id = unichar_id;
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
      GenericVector<ScoredFont> merged_fonts =
          choice_lists_it[0].data()->fonts();
      float merged_min_xheight = choice_lists_it[0].data()->min_xheight();
      float merged_max_xheight = choice_lists_it[0].data()->max_xheight();
      float positive_yshift = 0, negative_yshift = 0;
      int merged_script_id = choice_lists_it[0].data()->script_id();
      BlobChoiceClassifier classifier = choice_lists_it[0].data()->classifier();

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
        float yshift = choice_lists_it[i].data()->yshift();
        if (yshift > positive_yshift) positive_yshift = yshift;
        if (yshift < negative_yshift) negative_yshift = yshift;
        // Use the min font rating over the parts.
        // TODO(rays) font lists are unsorted. Need to be faster?
        const GenericVector<ScoredFont>& frag_fonts =
            choice_lists_it[i].data()->fonts();
        for (int f = 0; f < frag_fonts.size(); ++f) {
          int merged_f = 0;
          for (merged_f = 0; merged_f < merged_fonts.size() &&
               merged_fonts[merged_f].fontinfo_id != frag_fonts[f].fontinfo_id;
               ++merged_f) {}
          if (merged_f == merged_fonts.size()) {
            merged_fonts.push_back(frag_fonts[f]);
          } else if (merged_fonts[merged_f].score > frag_fonts[f].score) {
            merged_fonts[merged_f].score = frag_fonts[f].score;
          }
        }
      }

      float merged_yshift = positive_yshift != 0
          ? (negative_yshift != 0 ? 0 : positive_yshift)
          : negative_yshift;
      BLOB_CHOICE* choice = new BLOB_CHOICE(merged_unichar_id,
                                            merged_rating,
                                            merged_certainty,
                                            merged_script_id,
                                            merged_min_xheight,
                                            merged_max_xheight,
                                            merged_yshift,
                                            classifier);
      choice->set_fonts(merged_fonts);
      merged_choice_it.add_to_end(choice);
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
 * current_frag is the position of the piece we are looking for.
 * current_row is the row in the rating matrix we are currently at.
 * start is the row we started initially, so that we can know where
 * to append the results to the matrix. num_frag_parts is the total
 * number of pieces we are looking for and num_blobs is the size of the
 * ratings matrix.
 **********************************************************************/
void Wordrec::get_fragment_lists(int16_t current_frag, int16_t current_row,
                                 int16_t start, int16_t num_frag_parts,
                                 int16_t num_blobs, MATRIX *ratings,
                                 BLOB_CHOICE_LIST *choice_lists) {
  if (current_frag == num_frag_parts) {
    merge_and_put_fragment_lists(start, current_row - 1, num_frag_parts,
                                 choice_lists, ratings);
    return;
  }

  for (int16_t x = current_row; x < num_blobs; x++) {
    BLOB_CHOICE_LIST *choices = ratings->get(current_row, x);
    if (choices == nullptr)
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
void Wordrec::merge_fragments(MATRIX *ratings, int16_t num_blobs) {
  BLOB_CHOICE_LIST choice_lists[CHAR_FRAGMENT::kMaxChunks];
  for (int16_t start = 0; start < num_blobs; start++) {
    for (int frag_parts = 2; frag_parts <= CHAR_FRAGMENT::kMaxChunks;
         frag_parts++) {
      get_fragment_lists(0, start, start, frag_parts, num_blobs,
                         ratings, choice_lists);
    }
  }

  // Delete fragments from the rating matrix
  for (int16_t x = 0; x < num_blobs; x++) {
    for (int16_t y = x; y < num_blobs; y++) {
      BLOB_CHOICE_LIST *choices = ratings->get(x, y);
      if (choices != nullptr) {
        BLOB_CHOICE_IT choices_it(choices);
        for (choices_it.mark_cycle_pt(); !choices_it.cycled_list();
             choices_it.forward()) {
          UNICHAR_ID choice_unichar_id = choices_it.data()->unichar_id();
          const CHAR_FRAGMENT *frag =
              unicharset.get_fragment(choice_unichar_id);
          if (frag != nullptr)
            delete choices_it.extract();
        }
      }
    }
  }
}


}  // namespace tesseract
