/******************************************************************************
 *
 * File:         chopper.cpp  (Formerly chopper.c)
 * Author:       Mark Seaman, OCR Technology
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
 *****************************************************************************/

/*----------------------------------------------------------------------
          I n c l u d e s
----------------------------------------------------------------------*/

#include "blamer.h"    // for BlamerBundle, IRR_CORRECT
#include "blobs.h"     // for TPOINT, TBLOB, EDGEPT, TESSLINE, divisible_blob
#include "dict.h"      // for Dict
#include "lm_pain_points.h" // for LMPainPoints
#include "lm_state.h"  // for BestChoiceBundle
#include "matrix.h"    // for MATRIX
#include "normalis.h"  // for DENORM
#include "pageres.h"   // for WERD_RES
#include "params.h"    // for IntParam, BoolParam
#include "ratngs.h"    // for BLOB_CHOICE (ptr only), BLOB_CHOICE_LIST (ptr ...
#include "rect.h"      // for TBOX
#include "render.h"    // for display_blob
#include "seam.h"      // for SEAM
#include "split.h"     // for remove_edgept
#include "stopper.h"   // for DANGERR
#include "tprintf.h"   // for tprintf
#include "wordrec.h"   // for Wordrec, SegSearchPending (ptr only)

template <typename T> class GenericVector;

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

// Even though the limit on the number of chunks may now be removed, keep
// the same limit for repeatable behavior, and it may be a speed advantage.
static const int kMaxNumChunks = 64;

/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/

/**
 * @name check_blob
 *
 * @return true if blob has a non whole outline.
 */
static int check_blob(TBLOB *blob) {
  TESSLINE *outline;
  EDGEPT *edgept;

  for (outline = blob->outlines; outline != nullptr; outline = outline->next) {
    edgept = outline->loop;
    do {
      if (edgept == nullptr)
        break;
      edgept = edgept->next;
    }
    while (edgept != outline->loop);
    if (edgept == nullptr)
      return 1;
  }
  return 0;
}

/**
 * @name any_shared_split_points
 *
 * Return true if any of the splits share a point with this one.
 */
static int any_shared_split_points(const GenericVector<SEAM*>& seams, SEAM *seam) {
  int length;
  int index;

  length = seams.size();
  for (index = 0; index < length; index++)
    if (seam->SharesPosition(*seams[index])) return true;
  return false;
}

/**
 * @name preserve_outline_tree
 *
 * Copy the list of outlines.
 */
static void preserve_outline(EDGEPT *start) {
  EDGEPT *srcpt;

  if (start == nullptr)
    return;
  srcpt = start;
  do {
    srcpt->flags[1] = 1;
    srcpt = srcpt->next;
  }
  while (srcpt != start);
  srcpt->flags[1] = 2;
}

static void preserve_outline_tree(TESSLINE *srcline) {
  TESSLINE *outline;

  for (outline = srcline; outline != nullptr; outline = outline->next) {
    preserve_outline (outline->loop);
  }
}

/**
 * @name restore_outline_tree
 *
 * Copy the list of outlines.
 */
static EDGEPT *restore_outline(EDGEPT *start) {
  EDGEPT *srcpt;
  EDGEPT *real_start;

  if (start == nullptr)
    return nullptr;
  srcpt = start;
  do {
    if (srcpt->flags[1] == 2)
      break;
    srcpt = srcpt->next;
  }
  while (srcpt != start);
  real_start = srcpt;
  do {
    srcpt = srcpt->next;
    if (srcpt->prev->flags[1] == 0) {
      remove_edgept(srcpt->prev);
    }
  }
  while (srcpt != real_start);
  return real_start;
}

static void restore_outline_tree(TESSLINE *srcline) {
  TESSLINE *outline;

  for (outline = srcline; outline != nullptr; outline = outline->next) {
    outline->loop = restore_outline (outline->loop);
    outline->start = outline->loop->pos;
  }
}

/**********************************************************************
 * total_containment
 *
 * Check to see if one of these outlines is totally contained within
 * the bounding box of the other.
 **********************************************************************/
static int16_t total_containment(TBLOB *blob1, TBLOB *blob2) {
  TBOX box1 = blob1->bounding_box();
  TBOX box2 = blob2->bounding_box();
  return box1.contains(box2) || box2.contains(box1);
}

// Helper runs all the checks on a seam to make sure it is valid.
// Returns the seam if OK, otherwise deletes the seam and returns nullptr.
static SEAM* CheckSeam(int debug_level, int32_t blob_number, TWERD* word,
                       TBLOB* blob, TBLOB* other_blob,
                       const GenericVector<SEAM*>& seams, SEAM* seam) {
  if (seam == nullptr || blob->outlines == nullptr || other_blob->outlines == nullptr ||
      total_containment(blob, other_blob) || check_blob(other_blob) ||
      !seam->ContainedByBlob(*blob) || !seam->ContainedByBlob(*other_blob) ||
      any_shared_split_points(seams, seam) ||
      !seam->PrepareToInsertSeam(seams, word->blobs, blob_number, false)) {
    word->blobs.remove(blob_number + 1);
    if (seam) {
      seam->UndoSeam(blob, other_blob);
      delete seam;
      seam = nullptr;
#ifndef GRAPHICS_DISABLED
      if (debug_level) {
        if (debug_level >2)
          display_blob(blob, ScrollView::RED);
        tprintf("\n** seam being removed ** \n");
      }
#endif
    } else {
      delete other_blob;
    }
    return nullptr;
  }
  return seam;
}

namespace tesseract {

/**
 * @name attempt_blob_chop
 *
 * Try to split the this blob after this one.  Check to make sure that
 * it was successful.
 */
SEAM *Wordrec::attempt_blob_chop(TWERD *word, TBLOB *blob, int32_t blob_number,
                                 bool italic_blob,
                                 const GenericVector<SEAM*>& seams) {
  if (repair_unchopped_blobs)
    preserve_outline_tree (blob->outlines);
  TBLOB *other_blob = TBLOB::ShallowCopy(*blob);       /* Make new blob */
  // Insert it into the word.
  word->blobs.insert(other_blob, blob_number + 1);

  SEAM *seam = nullptr;
  if (prioritize_division) {
    TPOINT location;
    if (divisible_blob(blob, italic_blob, &location)) {
      seam = new SEAM(0.0f, location);
    }
  }
  if (seam == nullptr)
    seam = pick_good_seam(blob);
  if (chop_debug) {
    if (seam != nullptr)
      seam->Print("Good seam picked=");
    else
      tprintf("\n** no seam picked *** \n");
  }
  if (seam) {
    seam->ApplySeam(italic_blob, blob, other_blob);
  }

  seam = CheckSeam(chop_debug, blob_number, word, blob, other_blob,
                   seams, seam);
  if (seam == nullptr) {
    if (repair_unchopped_blobs)
      restore_outline_tree(blob->outlines);
    if (allow_blob_division && !prioritize_division) {
      // If the blob can simply be divided into outlines, then do that.
      TPOINT location;
      if (divisible_blob(blob, italic_blob, &location)) {
        other_blob = TBLOB::ShallowCopy(*blob);       /* Make new blob */
        word->blobs.insert(other_blob, blob_number + 1);
        seam = new SEAM(0.0f, location);
        seam->ApplySeam(italic_blob, blob, other_blob);
        seam = CheckSeam(chop_debug, blob_number, word, blob, other_blob,
                         seams, seam);
      }
    }
  }
  if (seam != nullptr) {
    // Make sure this seam doesn't get chopped again.
    seam->Finalize();
  }
  return seam;
}


SEAM *Wordrec::chop_numbered_blob(TWERD *word, int32_t blob_number,
                                  bool italic_blob,
                                  const GenericVector<SEAM*>& seams) {
  return attempt_blob_chop(word, word->blobs[blob_number], blob_number,
                           italic_blob, seams);
}


SEAM *Wordrec::chop_overlapping_blob(const GenericVector<TBOX>& boxes,
                                     bool italic_blob, WERD_RES *word_res,
                                     int *blob_number) {
  TWERD *word = word_res->chopped_word;
  for (*blob_number = 0; *blob_number < word->NumBlobs(); ++*blob_number) {
    TBLOB *blob = word->blobs[*blob_number];
    TPOINT topleft, botright;
    topleft.x = blob->bounding_box().left();
    topleft.y = blob->bounding_box().top();
    botright.x = blob->bounding_box().right();
    botright.y = blob->bounding_box().bottom();

    TPOINT original_topleft, original_botright;
    word_res->denorm.DenormTransform(nullptr, topleft, &original_topleft);
    word_res->denorm.DenormTransform(nullptr, botright, &original_botright);

    TBOX original_box = TBOX(original_topleft.x, original_botright.y,
                             original_botright.x, original_topleft.y);

    bool almost_equal_box = false;
    int num_overlap = 0;
    for (int i = 0; i < boxes.size(); i++) {
      if (original_box.overlap_fraction(boxes[i]) > 0.125)
        num_overlap++;
      if (original_box.almost_equal(boxes[i], 3))
        almost_equal_box = true;
    }

    TPOINT location;
    if (divisible_blob(blob, italic_blob, &location) ||
        (!almost_equal_box && num_overlap > 1)) {
      SEAM *seam = attempt_blob_chop(word, blob, *blob_number,
                                     italic_blob, word_res->seam_array);
      if (seam != nullptr)
        return seam;
    }
  }

  *blob_number = -1;
  return nullptr;
}

/**
 * @name improve_one_blob
 *
 * Finds the best place to chop, based on the worst blob, fixpt, or next to
 * a fragment, according to the input. Returns the SEAM corresponding to the
 * chop point, if any is found, and the index in the ratings_matrix of the
 * chopped blob. Note that blob_choices is just a copy of the pointers in the
 * leading diagonal of the ratings MATRIX.
 * Although the blob is chopped, the returned SEAM is yet to be inserted into
 * word->seam_array and the resulting blobs are unclassified, so this function
 * can be used by ApplyBox as well as during recognition.
 */
SEAM* Wordrec::improve_one_blob(const GenericVector<BLOB_CHOICE*>& blob_choices,
                                DANGERR *fixpt,
                                bool split_next_to_fragment,
                                bool italic_blob,
                                WERD_RES* word,
                                int* blob_number) {
  float rating_ceiling = FLT_MAX;
  SEAM *seam = nullptr;
  do {
    *blob_number = select_blob_to_split_from_fixpt(fixpt);
    if (chop_debug) tprintf("blob_number from fixpt = %d\n", *blob_number);
    bool split_point_from_dict = (*blob_number != -1);
    if (split_point_from_dict) {
      fixpt->clear();
    } else {
      *blob_number = select_blob_to_split(blob_choices, rating_ceiling,
                                          split_next_to_fragment);
    }
    if (chop_debug) tprintf("blob_number = %d\n", *blob_number);
    if (*blob_number == -1)
      return nullptr;

    // TODO(rays) it may eventually help to allow italic_blob to be true,
    seam = chop_numbered_blob(word->chopped_word, *blob_number, italic_blob,
                              word->seam_array);
    if (seam != nullptr)
      return seam;  // Success!
    if (blob_choices[*blob_number] == nullptr)
      return nullptr;
    if (!split_point_from_dict) {
      // We chopped the worst rated blob, try something else next time.
      rating_ceiling = blob_choices[*blob_number]->rating();
    }
  } while (true);
  return seam;
}

/**
 * @name chop_one_blob
 *
 * Start with the current one-blob word and its classification.  Find
 * the worst blobs and try to divide it up to improve the ratings.
 * Used for testing chopper.
 */
SEAM* Wordrec::chop_one_blob(const GenericVector<TBOX>& boxes,
                             const GenericVector<BLOB_CHOICE*>& blob_choices,
                             WERD_RES* word_res,
                             int* blob_number) {
  if (prioritize_division) {
    return chop_overlapping_blob(boxes, true, word_res, blob_number);
  } else {
    return improve_one_blob(blob_choices, nullptr, false, true, word_res,
                            blob_number);
  }
}

/**
 * @name chop_word_main
 *
 * Classify the blobs in this word and permute the results.  Find the
 * worst blob in the word and chop it up.  Continue this process until
 * a good answer has been found or all the blobs have been chopped up
 * enough.  The results are returned in the WERD_RES.
 */
void Wordrec::chop_word_main(WERD_RES *word) {
  int num_blobs = word->chopped_word->NumBlobs();
  if (word->ratings == nullptr) {
    word->ratings = new MATRIX(num_blobs, wordrec_max_join_chunks);
  }
  if (word->ratings->get(0, 0) == nullptr) {
    // Run initial classification.
    for (int b = 0; b < num_blobs; ++b) {
      BLOB_CHOICE_LIST* choices = classify_piece(word->seam_array, b, b,
                                                 "Initial:", word->chopped_word,
                                                 word->blamer_bundle);
      word->ratings->put(b, b, choices);
    }
  } else {
    // Blobs have been pre-classified. Set matrix cell for all blob choices
    for (int col = 0; col < word->ratings->dimension(); ++col) {
      for (int row = col; row < word->ratings->dimension() &&
           row < col + word->ratings->bandwidth(); ++row) {
        BLOB_CHOICE_LIST* choices = word->ratings->get(col, row);
        if (choices != nullptr) {
          BLOB_CHOICE_IT bc_it(choices);
          for (bc_it.mark_cycle_pt(); !bc_it.cycled_list(); bc_it.forward()) {
            bc_it.data()->set_matrix_cell(col, row);
          }
        }
      }
    }
  }

  // Run Segmentation Search.
  BestChoiceBundle best_choice_bundle(word->ratings->dimension());
  SegSearch(word, &best_choice_bundle, word->blamer_bundle);

  if (word->best_choice == nullptr) {
    // SegSearch found no valid paths, so just use the leading diagonal.
    word->FakeWordFromRatings(TOP_CHOICE_PERM);
  }
  word->RebuildBestState();
  // If we finished without a hyphen at the end of the word, let the next word
  // be found in the dictionary.
  if (word->word->flag(W_EOL) &&
      !getDict().has_hyphen_end(*word->best_choice)) {
    getDict().reset_hyphen_vars(true);
  }

  if (word->blamer_bundle != nullptr && this->fill_lattice_ != nullptr) {
    CallFillLattice(*word->ratings, word->best_choices,
                    *word->uch_set, word->blamer_bundle);
  }
  if (wordrec_debug_level > 0) {
    tprintf("Final Ratings Matrix:\n");
    word->ratings->print(getDict().getUnicharset());
  }
  word->FilterWordChoices(getDict().stopper_debug_level);
}

/**
 * @name improve_by_chopping
 *
 * Repeatedly chops the worst blob, classifying the new blobs fixing up all
 * the data, and incrementally runs the segmentation search until a good word
 * is found, or no more chops can be found.
 */
void Wordrec::improve_by_chopping(float rating_cert_scale,
                                  WERD_RES* word,
                                  BestChoiceBundle* best_choice_bundle,
                                  BlamerBundle* blamer_bundle,
                                  LMPainPoints* pain_points,
                                  GenericVector<SegSearchPending>* pending) {
  int blob_number;
  do {  // improvement loop.
    // Make a simple vector of BLOB_CHOICEs to make it easy to pick which
    // one to chop.
    GenericVector<BLOB_CHOICE*> blob_choices;
    int num_blobs = word->ratings->dimension();
    for (int i = 0; i < num_blobs; ++i) {
      BLOB_CHOICE_LIST* choices = word->ratings->get(i, i);
      if (choices == nullptr || choices->empty()) {
        blob_choices.push_back(nullptr);
      } else {
        BLOB_CHOICE_IT bc_it(choices);
        blob_choices.push_back(bc_it.data());
      }
    }
    SEAM* seam = improve_one_blob(blob_choices, &best_choice_bundle->fixpt,
                                  false, false, word, &blob_number);
    if (seam == nullptr) break;
    // A chop has been made. We have to correct all the data structures to
    // take into account the extra bottom-level blob.
    // Put the seam into the seam_array and correct everything else on the
    // word: ratings matrix (including matrix location in the BLOB_CHOICES),
    // states in WERD_CHOICEs, and blob widths.
    word->InsertSeam(blob_number, seam);
    // Insert a new entry in the beam array.
    best_choice_bundle->beam.insert(new LanguageModelState, blob_number);
    // Fixpts are outdated, but will get recalculated.
    best_choice_bundle->fixpt.clear();
    // Remap existing pain points.
    pain_points->RemapForSplit(blob_number);
    // Insert a new pending at the chop point.
    pending->insert(SegSearchPending(), blob_number);

    // Classify the two newly created blobs using ProcessSegSearchPainPoint,
    // as that updates the pending correctly and adds new pain points.
    MATRIX_COORD pain_point(blob_number, blob_number);
    ProcessSegSearchPainPoint(0.0f, pain_point, "Chop1", pending, word,
                              pain_points, blamer_bundle);
    pain_point.col = blob_number + 1;
    pain_point.row = blob_number + 1;
    ProcessSegSearchPainPoint(0.0f, pain_point, "Chop2", pending, word,
                              pain_points, blamer_bundle);
    if (language_model_->language_model_ngram_on) {
      // N-gram evaluation depends on the number of blobs in a chunk, so we
      // have to re-evaluate everything in the word.
      ResetNGramSearch(word, best_choice_bundle, pending);
      blob_number = 0;
    }
    // Run language model incrementally. (Except with the n-gram model on.)
    UpdateSegSearchNodes(rating_cert_scale, blob_number, pending,
                         word, pain_points, best_choice_bundle, blamer_bundle);
  } while (!language_model_->AcceptableChoiceFound() &&
           word->ratings->dimension() < kMaxNumChunks);

  // If after running only the chopper best_choice is incorrect and no blame
  // has been yet set, blame the classifier if best_choice is classifier's
  // top choice and is a dictionary word (i.e. language model could not have
  // helped). Otherwise blame the tradeoff between the classifier and
  // the old language model (permuters).
  if (word->blamer_bundle != nullptr &&
      word->blamer_bundle->incorrect_result_reason() == IRR_CORRECT &&
      !word->blamer_bundle->ChoiceIsCorrect(word->best_choice)) {
    bool valid_permuter = word->best_choice != nullptr &&
        Dict::valid_word_permuter(word->best_choice->permuter(), false);
    word->blamer_bundle->BlameClassifierOrLangModel(word,
                                                    getDict().getUnicharset(),
                                                    valid_permuter,
                                                    wordrec_debug_blamer);
  }
}


/**********************************************************************
 * select_blob_to_split
 *
 * These are the results of the last classification.  Find a likely
 * place to apply splits.  If none, return -1.
 **********************************************************************/
int Wordrec::select_blob_to_split(
    const GenericVector<BLOB_CHOICE*>& blob_choices,
    float rating_ceiling, bool split_next_to_fragment) {
  BLOB_CHOICE *blob_choice;
  int x;
  float worst = -FLT_MAX;
  int worst_index = -1;
  float worst_near_fragment = -FLT_MAX;
  int worst_index_near_fragment = -1;
  const CHAR_FRAGMENT **fragments = nullptr;

  if (chop_debug) {
    if (rating_ceiling < FLT_MAX)
      tprintf("rating_ceiling = %8.4f\n", rating_ceiling);
    else
      tprintf("rating_ceiling = No Limit\n");
  }

  if (split_next_to_fragment && blob_choices.size() > 0) {
    fragments = new const CHAR_FRAGMENT *[blob_choices.size()];
    if (blob_choices[0] != nullptr) {
      fragments[0] = getDict().getUnicharset().get_fragment(
          blob_choices[0]->unichar_id());
    } else {
      fragments[0] = nullptr;
    }
  }

  for (x = 0; x < blob_choices.size(); ++x) {
    if (blob_choices[x] == nullptr) {
      delete[] fragments;
      return x;
    } else {
      blob_choice = blob_choices[x];
      // Populate fragments for the following position.
      if (split_next_to_fragment && x+1 < blob_choices.size()) {
        if (blob_choices[x + 1] != nullptr) {
          fragments[x + 1] = getDict().getUnicharset().get_fragment(
              blob_choices[x + 1]->unichar_id());
        } else {
          fragments[x + 1] = nullptr;
        }
      }
      if (blob_choice->rating() < rating_ceiling &&
          blob_choice->certainty() < tessedit_certainty_threshold) {
        // Update worst and worst_index.
        if (blob_choice->rating() > worst) {
          worst_index = x;
          worst = blob_choice->rating();
        }
        if (split_next_to_fragment) {
          // Update worst_near_fragment and worst_index_near_fragment.
          bool expand_following_fragment =
            (x + 1 < blob_choices.size() &&
             fragments[x+1] != nullptr && !fragments[x+1]->is_beginning());
          bool expand_preceding_fragment =
            (x > 0 && fragments[x-1] != nullptr && !fragments[x-1]->is_ending());
          if ((expand_following_fragment || expand_preceding_fragment) &&
              blob_choice->rating() > worst_near_fragment) {
            worst_index_near_fragment = x;
            worst_near_fragment = blob_choice->rating();
            if (chop_debug) {
              tprintf("worst_index_near_fragment=%d"
                      " expand_following_fragment=%d"
                      " expand_preceding_fragment=%d\n",
                      worst_index_near_fragment,
                      expand_following_fragment,
                      expand_preceding_fragment);
            }
          }
        }
      }
    }
  }
  delete[] fragments;
  // TODO(daria): maybe a threshold of badness for
  // worst_near_fragment would be useful.
  return worst_index_near_fragment != -1 ?
    worst_index_near_fragment : worst_index;
}

/**********************************************************************
 * select_blob_to_split_from_fixpt
 *
 * Given the fix point from a dictionary search, if there is a single
 * dangerous blob that maps to multiple characters, return that blob
 * index as a place we need to split.  If none, return -1.
 **********************************************************************/
int Wordrec::select_blob_to_split_from_fixpt(DANGERR *fixpt) {
  if (!fixpt)
    return -1;
  for (int i = 0; i < fixpt->size(); i++) {
    if ((*fixpt)[i].begin + 1 == (*fixpt)[i].end &&
        (*fixpt)[i].dangerous &&
        (*fixpt)[i].correct_is_ngram) {
      return (*fixpt)[i].begin;
    }
  }
  return -1;
}

}  // namespace tesseract
