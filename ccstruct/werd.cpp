/**********************************************************************
 * File:        werd.cpp  (Formerly word.c)
 * Description: Code for the WERD class.
 * Author:      Ray Smith
 * Created:     Tue Oct 08 14:32:12 BST 1991
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

#include "blckerr.h"
#include "helpers.h"
#include "linlsq.h"
#include "werd.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define FIRST_COLOUR    ScrollView::RED         //< first rainbow colour
#define LAST_COLOUR     ScrollView::AQUAMARINE  //< last rainbow colour
#define CHILD_COLOUR    ScrollView::BROWN       //< colour of children

const ERRCODE CANT_SCALE_EDGESTEPS =
    "Attempted to scale an edgestep format word";

ELIST2IZE(WERD)

/**
 * WERD::WERD
 *
 * Constructor to build a WERD from a list of C_BLOBs.
 *   blob_list     The C_BLOBs (in word order) are not copied;
 *                 we take its elements and put them in our lists.
 *   blank_count   blanks in front of the word
 *   text          correct text, outlives this WERD
 */
WERD::WERD(C_BLOB_LIST *blob_list, uinT8 blank_count, const char *text)
  : blanks(blank_count),
    flags(0),
    script_id_(0),
    correct(text) {
  C_BLOB_IT start_it = &cblobs;
  C_BLOB_IT rej_cblob_it = &rej_cblobs;
  C_OUTLINE_IT c_outline_it;
  inT16 inverted_vote = 0;
  inT16 non_inverted_vote = 0;

  // Move blob_list's elements into cblobs.
  start_it.add_list_after(blob_list);

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
  start_it.set_to_list(&cblobs);
  if (start_it.empty())
    return;
  for (start_it.mark_cycle_pt(); !start_it.cycled_list(); start_it.forward()) {
    BOOL8 reject_blob = FALSE;
    BOOL8 blob_inverted;

    c_outline_it.set_to_list(start_it.data()->out_list());
    blob_inverted = c_outline_it.data()->flag(COUT_INVERSE);
    for (c_outline_it.mark_cycle_pt();
         !c_outline_it.cycled_list() && !reject_blob;
         c_outline_it.forward()) {
      reject_blob = c_outline_it.data()->flag(COUT_INVERSE) != blob_inverted;
    }
    if (reject_blob) {
      rej_cblob_it.add_after_then_move(start_it.extract());
    } else {
      if (blob_inverted)
        inverted_vote++;
      else
        non_inverted_vote++;
    }
  }

  flags.set_bit(W_INVERSE, (inverted_vote > non_inverted_vote));

  start_it.set_to_list(&cblobs);
  if (start_it.empty())
    return;
  for (start_it.mark_cycle_pt(); !start_it.cycled_list(); start_it.forward()) {
    c_outline_it.set_to_list(start_it.data()->out_list());
    if (c_outline_it.data()->flag(COUT_INVERSE) != flags.bit(W_INVERSE))
      rej_cblob_it.add_after_then_move(start_it.extract());
  }
}


/**
 * WERD::WERD
 *
 * Constructor to build a WERD from a list of C_BLOBs.
 * The C_BLOBs are not copied so the source list is emptied.
 */

WERD::WERD(C_BLOB_LIST * blob_list,         //< In word order
           WERD * clone)                    //< Source of flags
  : flags(clone->flags),
    script_id_(clone->script_id_),
    correct(clone->correct) {
  C_BLOB_IT start_it = blob_list;  // iterator
  C_BLOB_IT end_it = blob_list;    // another

  while (!end_it.at_last ())
    end_it.forward ();           //move to last
  ((C_BLOB_LIST *) (&cblobs))->assign_to_sublist (&start_it, &end_it);
  //move to our list
  blanks = clone->blanks;
  //      fprintf(stderr,"Wrong constructor!!!!\n");
}

// Construct a WERD from a single_blob and clone the flags from this.
// W_BOL and W_EOL flags are set according to the given values.
WERD* WERD::ConstructFromSingleBlob(bool bol, bool eol, C_BLOB* blob) {
  C_BLOB_LIST temp_blobs;
  C_BLOB_IT temp_it(&temp_blobs);
  temp_it.add_after_then_move(blob);
  WERD* blob_word = new WERD(&temp_blobs, this);
  blob_word->set_flag(W_BOL, bol);
  blob_word->set_flag(W_EOL, eol);
  return blob_word;
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

TBOX WERD::bounding_box() const { return restricted_bounding_box(true, true); }

// Returns the bounding box including the desired combination of upper and
// lower noise/diacritic elements.
TBOX WERD::restricted_bounding_box(bool upper_dots, bool lower_dots) const {
  TBOX box = true_bounding_box();
  int bottom = box.bottom();
  int top = box.top();
  // This is a read-only iteration of the rejected blobs.
  C_BLOB_IT it(const_cast<C_BLOB_LIST*>(&rej_cblobs));
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    TBOX dot_box = it.data()->bounding_box();
    if ((upper_dots || dot_box.bottom() <= top) &&
        (lower_dots || dot_box.top() >= bottom)) {
      box += dot_box;
    }
  }
  return box;
}

// Returns the bounding box of only the good blobs.
TBOX WERD::true_bounding_box() const {
  TBOX box;  // box being built
  // This is a read-only iteration of the good blobs.
  C_BLOB_IT it(const_cast<C_BLOB_LIST*>(&cblobs));
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    box += it.data()->bounding_box();
  }
  return box;
}

/**
 * WERD::move
 *
 * Reposition WERD by vector
 * NOTE!! REJECT CBLOBS ARE NOT MOVED
 */

void WERD::move(const ICOORD vec) {
  C_BLOB_IT cblob_it(&cblobs);  // cblob iterator

  for (cblob_it.mark_cycle_pt(); !cblob_it.cycled_list(); cblob_it.forward())
    cblob_it.data()->move(vec);
}

/**
 * WERD::join_on
 *
 * Join other word onto this one. Delete the old word.
 */

void WERD::join_on(WERD* other) {
  C_BLOB_IT blob_it(&cblobs);
  C_BLOB_IT src_it(&other->cblobs);
  C_BLOB_IT rej_cblob_it(&rej_cblobs);
  C_BLOB_IT src_rej_it(&other->rej_cblobs);

  while (!src_it.empty()) {
    blob_it.add_to_end(src_it.extract());
    src_it.forward();
  }
  while (!src_rej_it.empty()) {
    rej_cblob_it.add_to_end(src_rej_it.extract());
    src_rej_it.forward();
  }
}


/**
 * WERD::copy_on
 *
 * Copy blobs from other word onto this one.
 */

void WERD::copy_on(WERD* other) {
  bool reversed = other->bounding_box().left() < bounding_box().left();
  C_BLOB_IT c_blob_it(&cblobs);
  C_BLOB_LIST c_blobs;

  c_blobs.deep_copy(&other->cblobs, &C_BLOB::deep_copy);
  if (reversed) {
    c_blob_it.add_list_before(&c_blobs);
  } else {
    c_blob_it.move_to_last();
    c_blob_it.add_list_after(&c_blobs);
  }
  if (!other->rej_cblobs.empty()) {
    C_BLOB_IT rej_c_blob_it(&rej_cblobs);
    C_BLOB_LIST new_rej_c_blobs;

    new_rej_c_blobs.deep_copy(&other->rej_cblobs, &C_BLOB::deep_copy);
    if (reversed) {
      rej_c_blob_it.add_list_before(&new_rej_c_blobs);
    } else {
      rej_c_blob_it.move_to_last();
      rej_c_blob_it.add_list_after(&new_rej_c_blobs);
    }
  }
}

/**
 * WERD::print
 *
 * Display members
 */

void WERD::print() {
  tprintf("Blanks= %d\n", blanks);
  bounding_box().print();
  tprintf("Flags = %d = 0%o\n", flags.val, flags.val);
  tprintf("   W_SEGMENTED = %s\n", flags.bit(W_SEGMENTED) ? "TRUE" : "FALSE ");
  tprintf("   W_ITALIC = %s\n", flags.bit(W_ITALIC) ? "TRUE" : "FALSE ");
  tprintf("   W_BOL = %s\n", flags.bit(W_BOL) ? "TRUE" : "FALSE ");
  tprintf("   W_EOL = %s\n", flags.bit(W_EOL) ? "TRUE" : "FALSE ");
  tprintf("   W_NORMALIZED = %s\n",
          flags.bit(W_NORMALIZED) ? "TRUE" : "FALSE ");
  tprintf("   W_SCRIPT_HAS_XHEIGHT = %s\n",
          flags.bit(W_SCRIPT_HAS_XHEIGHT) ? "TRUE" : "FALSE ");
  tprintf("   W_SCRIPT_IS_LATIN = %s\n",
          flags.bit(W_SCRIPT_IS_LATIN) ? "TRUE" : "FALSE ");
  tprintf("   W_DONT_CHOP = %s\n", flags.bit(W_DONT_CHOP) ? "TRUE" : "FALSE ");
  tprintf("   W_REP_CHAR = %s\n", flags.bit(W_REP_CHAR) ? "TRUE" : "FALSE ");
  tprintf("   W_FUZZY_SP = %s\n", flags.bit(W_FUZZY_SP) ? "TRUE" : "FALSE ");
  tprintf("   W_FUZZY_NON = %s\n", flags.bit(W_FUZZY_NON) ? "TRUE" : "FALSE ");
  tprintf("Correct= %s\n", correct.string());
  tprintf("Rejected cblob count = %d\n", rej_cblobs.length());
  tprintf("Script = %d\n", script_id_);
}


/**
 * WERD::plot
 *
 * Draw the WERD in the given colour.
 */

#ifndef GRAPHICS_DISABLED
void WERD::plot(ScrollView *window, ScrollView::Color colour) {
  C_BLOB_IT it = &cblobs;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->plot(window, colour, colour);
  }
  plot_rej_blobs(window);
}

// Get the next color in the (looping) rainbow.
ScrollView::Color WERD::NextColor(ScrollView::Color colour) {
  ScrollView::Color next = static_cast<ScrollView::Color>(colour + 1);
  if (next >= LAST_COLOUR || next < FIRST_COLOUR)
    next = FIRST_COLOUR;
  return next;
}

/**
 * WERD::plot
 *
 * Draw the WERD in rainbow colours in window.
 */

void WERD::plot(ScrollView* window) {
  ScrollView::Color colour = FIRST_COLOUR;
  C_BLOB_IT it = &cblobs;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->plot(window, colour, CHILD_COLOUR);
    colour = NextColor(colour);
  }
  plot_rej_blobs(window);
}


/**
 * WERD::plot_rej_blobs
 *
 * Draw the WERD rejected blobs in window - ALWAYS GREY
 */


void WERD::plot_rej_blobs(ScrollView *window) {
  C_BLOB_IT it = &rej_cblobs;
  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
    it.data()->plot(window, ScrollView::GREY, ScrollView::GREY);
  }
}
#endif  // GRAPHICS_DISABLED


/**
 * WERD::shallow_copy()
 *
 * Make a shallow copy of a word
 */

WERD *WERD::shallow_copy() {
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

WERD & WERD::operator= (const WERD & source) {
  this->ELIST2_LINK::operator= (source);
  blanks = source.blanks;
  flags = source.flags;
  script_id_ = source.script_id_;
  dummy = source.dummy;
  correct = source.correct;
  if (!cblobs.empty())
    cblobs.clear();
  cblobs.deep_copy(&source.cblobs, &C_BLOB::deep_copy);

  if (!rej_cblobs.empty())
    rej_cblobs.clear();
  rej_cblobs.deep_copy(&source.rej_cblobs, &C_BLOB::deep_copy);
  return *this;
}


/**
 *  word_comparator()
 *
 *  word comparator used to sort a word list so that words are in increasing
 *  order of left edge.
 */

int word_comparator(const void *word1p, const void *word2p) {
  WERD *word1 = *(WERD **)word1p;
  WERD *word2 = *(WERD **)word2p;
  return word1->bounding_box().left() - word2->bounding_box().left();
}

/**
 *  WERD::ConstructWerdWithNewBlobs()
 *
 * This method returns a new werd constructed using the blobs in the input
 * all_blobs list, which correspond to the blobs in this werd object. The
 * blobs used to construct the new word are consumed and removed from the
 * input all_blobs list.
 * Returns NULL if the word couldn't be constructed.
 * Returns original blobs for which no matches were found in the output list
 * orphan_blobs (appends).
 */

WERD* WERD::ConstructWerdWithNewBlobs(C_BLOB_LIST* all_blobs,
                                      C_BLOB_LIST* orphan_blobs) {
  C_BLOB_LIST current_blob_list;
  C_BLOB_IT werd_blobs_it(&current_blob_list);
  // Add the word's c_blobs.
  werd_blobs_it.add_list_after(cblob_list());

  // New blob list. These contain the blobs which will form the new word.
  C_BLOB_LIST new_werd_blobs;
  C_BLOB_IT new_blobs_it(&new_werd_blobs);

  // not_found_blobs contains the list of current word's blobs for which a
  // corresponding blob wasn't found in the input all_blobs list.
  C_BLOB_LIST not_found_blobs;
  C_BLOB_IT not_found_it(&not_found_blobs);
  not_found_it.move_to_last();

  werd_blobs_it.move_to_first();
  for (werd_blobs_it.mark_cycle_pt(); !werd_blobs_it.cycled_list();
       werd_blobs_it.forward()) {
    C_BLOB* werd_blob = werd_blobs_it.extract();
    TBOX werd_blob_box = werd_blob->bounding_box();
    bool found = false;
    // Now find the corresponding blob for this blob in the all_blobs
    // list. For now, follow the inefficient method of pairwise
    // comparisons. Ideally, one can pre-bucket the blobs by row.
    C_BLOB_IT all_blobs_it(all_blobs);
    for (all_blobs_it.mark_cycle_pt(); !all_blobs_it.cycled_list();
         all_blobs_it.forward()) {
      C_BLOB* a_blob = all_blobs_it.data();
      // Compute the overlap of the two blobs. If major, a_blob should
      // be added to the new blobs list.
      TBOX a_blob_box = a_blob->bounding_box();
      if (a_blob_box.null_box()) {
        tprintf("Bounding box couldn't be ascertained\n");
      }
      if (werd_blob_box.contains(a_blob_box) ||
          werd_blob_box.major_overlap(a_blob_box)) {
        // Old blobs are from minimal splits, therefore are expected to be
        // bigger. The new small blobs should cover a significant portion.
        // This is it.
        all_blobs_it.extract();
        new_blobs_it.add_after_then_move(a_blob);
        found = true;
      }
    }
    if (!found) {
      not_found_it.add_after_then_move(werd_blob);
    } else {
      delete werd_blob;
    }
  }
  // Iterate over all not found blobs. Some of them may be due to
  // under-segmentation (which is OK, since the corresponding blob is already
  // in the list in that case.
  not_found_it.move_to_first();
  for (not_found_it.mark_cycle_pt(); !not_found_it.cycled_list();
       not_found_it.forward()) {
    C_BLOB* not_found = not_found_it.data();
    TBOX not_found_box = not_found->bounding_box();
    C_BLOB_IT existing_blobs_it(new_blobs_it);
    for (existing_blobs_it.mark_cycle_pt(); !existing_blobs_it.cycled_list();
         existing_blobs_it.forward()) {
      C_BLOB* a_blob = existing_blobs_it.data();
      TBOX a_blob_box = a_blob->bounding_box();
      if ((not_found_box.major_overlap(a_blob_box) ||
           a_blob_box.major_overlap(not_found_box)) &&
           not_found_box.y_overlap_fraction(a_blob_box) > 0.8) {
        // Already taken care of.
        delete not_found_it.extract();
        break;
      }
    }
  }
  if (orphan_blobs) {
    C_BLOB_IT orphan_blobs_it(orphan_blobs);
    orphan_blobs_it.move_to_last();
    orphan_blobs_it.add_list_after(&not_found_blobs);
  }

  // New blobs are ready. Create a new werd object with these.
  WERD* new_werd = NULL;
  if (!new_werd_blobs.empty()) {
    new_werd = new WERD(&new_werd_blobs, this);
  } else {
    // Add the blobs back to this word so that it can be reused.
    C_BLOB_IT this_list_it(cblob_list());
    this_list_it.add_list_after(&not_found_blobs);
  }
  return new_werd;
}

// Removes noise from the word by moving small outlines to the rej_cblobs
// list, based on the size_threshold.
void WERD::CleanNoise(float size_threshold) {
  C_BLOB_IT blob_it(&cblobs);
  C_BLOB_IT rej_it(&rej_cblobs);
  for (blob_it.mark_cycle_pt(); !blob_it.cycled_list(); blob_it.forward()) {
    C_BLOB* blob = blob_it.data();
    C_OUTLINE_IT ol_it(blob->out_list());
    for (ol_it.mark_cycle_pt(); !ol_it.cycled_list(); ol_it.forward()) {
      C_OUTLINE* outline = ol_it.data();
      TBOX ol_box = outline->bounding_box();
      int ol_size =
          ol_box.width() > ol_box.height() ? ol_box.width() : ol_box.height();
      if (ol_size < size_threshold) {
        // This outline is too small. Move it to a separate blob in the
        // reject blobs list.
        C_BLOB* rej_blob = new C_BLOB(ol_it.extract());
        rej_it.add_after_then_move(rej_blob);
      }
    }
    if (blob->out_list()->empty()) delete blob_it.extract();
  }
}

// Extracts all the noise outlines and stuffs the pointers into the given
// vector of outlines. Afterwards, the outlines vector owns the pointers.
void WERD::GetNoiseOutlines(GenericVector<C_OUTLINE*>* outlines) {
  C_BLOB_IT rej_it(&rej_cblobs);
  for (rej_it.mark_cycle_pt(); !rej_it.empty(); rej_it.forward()) {
    C_BLOB* blob = rej_it.extract();
    C_OUTLINE_IT ol_it(blob->out_list());
    outlines->push_back(ol_it.extract());
    delete blob;
  }
}

// Adds the selected outlines to the indcated real blobs, and puts the rest
// back in rej_cblobs where they came from. Where the target_blobs entry is
// NULL, a run of wanted outlines is put into a single new blob.
// Ownership of the outlines is transferred back to the word. (Hence
// GenericVector and not PointerVector.)
// Returns true if any new blob was added to the start of the word, which
// suggests that it might need joining to the word before it, and likewise
// sets make_next_word_fuzzy true if any new blob was added to the end.
bool WERD::AddSelectedOutlines(const GenericVector<bool>& wanted,
                               const GenericVector<C_BLOB*>& target_blobs,
                               const GenericVector<C_OUTLINE*>& outlines,
                               bool* make_next_word_fuzzy) {
  bool outline_added_to_start = false;
  if (make_next_word_fuzzy != NULL) *make_next_word_fuzzy = false;
  C_BLOB_IT rej_it(&rej_cblobs);
  for (int i = 0; i < outlines.size(); ++i) {
    C_OUTLINE* outline = outlines[i];
    if (outline == NULL) continue;  // Already used it.
    if (wanted[i]) {
      C_BLOB* target_blob = target_blobs[i];
      TBOX noise_box = outline->bounding_box();
      if (target_blob == NULL) {
        target_blob = new C_BLOB(outline);
        // Need to find the insertion point.
        C_BLOB_IT blob_it(&cblobs);
        for (blob_it.mark_cycle_pt(); !blob_it.cycled_list();
             blob_it.forward()) {
          C_BLOB* blob = blob_it.data();
          TBOX blob_box = blob->bounding_box();
          if (blob_box.left() > noise_box.left()) {
            if (blob_it.at_first() && !flag(W_FUZZY_SP) && !flag(W_FUZZY_NON)) {
              // We might want to join this word to its predecessor.
              outline_added_to_start = true;
            }
            blob_it.add_before_stay_put(target_blob);
            break;
          }
        }
        if (blob_it.cycled_list()) {
          blob_it.add_to_end(target_blob);
          if (make_next_word_fuzzy != NULL) *make_next_word_fuzzy = true;
        }
        // Add all consecutive wanted, but null-blob outlines to same blob.
        C_OUTLINE_IT ol_it(target_blob->out_list());
        while (i + 1 < outlines.size() && wanted[i + 1] &&
               target_blobs[i + 1] == NULL) {
          ++i;
          ol_it.add_to_end(outlines[i]);
        }
      } else {
        // Insert outline into this blob.
        C_OUTLINE_IT ol_it(target_blob->out_list());
        ol_it.add_to_end(outline);
      }
    } else {
      // Put back on noise list.
      rej_it.add_to_end(new C_BLOB(outline));
    }
  }
  return outline_added_to_start;
}
