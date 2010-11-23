/******************************************************************
 * File:        cube_control.cpp
 * Description: Tesseract class methods for invoking cube convolutional
 *              neural network word recognizer.
 * Author:      Raquel Romano
 * Created:     September 2009
 *
 **********************************************************************/

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifdef HAVE_LIBLEPT
// Include leptonica library only if autoconf (or makefile etc) tell us to.
#include "allheaders.h"
#endif

#include "cube_object.h"
#include "cube_reco_context.h"
#include "tesseractclass.h"
#include "tesseract_cube_combiner.h"

namespace tesseract {

/**********************************************************************
 * convert_prob_to_tess_certainty
 *
 * Normalize a probability in the range [0.0, 1.0] to a tesseract
 * certainty in the range [-20.0, 0.0]
 **********************************************************************/
static float convert_prob_to_tess_certainty(float prob) {
  return (prob - 1.0) * 20.0;
}

/**********************************************************************
 * char_box_to_tbox
 *
 * Create a TBOX from a character bounding box. If nonzero, the
 * x_offset accounts for any additional padding of the word box that
 * should be taken into account.
 *
 **********************************************************************/
TBOX char_box_to_tbox(Box* char_box, TBOX word_box, int x_offset) {
  l_int32 left;
  l_int32 top;
  l_int32 width;
  l_int32 height;
  l_int32 right;
  l_int32 bottom;

  boxGetGeometry(char_box, &left, &top, &width, &height);
  left += word_box.left() - x_offset;
  right = left + width;
  top = word_box.bottom() + word_box.height() - top;
  bottom = top - height;
  return TBOX(left, bottom, right, top);
}

/**********************************************************************
 * extract_cube_state
 *
 * Extract CharSamp objects and character bounding boxes from the
 * CubeObject's state. The caller should free both structres.
 *
**********************************************************************/
bool Tesseract::extract_cube_state(CubeObject* cube_obj,
                                   int* num_chars,
                                   Boxa** char_boxes,
                                   CharSamp*** char_samples) {
  if (!cube_obj) {
    if (cube_debug_level > 0) {
      tprintf("Cube WARNING (extract_cube_state): Invalid cube object "
              "passed to extract_cube_state\n");
    }
    return false;
  }

  // Note that the CubeObject accessors return either the deslanted or
  // regular objects search object or beam search object, whichever
  // was used in the last call to Recognize()
  CubeSearchObject* cube_search_obj = cube_obj->SrchObj();
  if (!cube_search_obj) {
    if (cube_debug_level > 0) {
      tprintf("Cube WARNING (Extract_cube_state): Could not retrieve "
              "cube's search object in extract_cube_state.\n");
    }
    return false;
  }
  BeamSearch *beam_search_obj = cube_obj->BeamObj();
  if (!beam_search_obj) {
    if (cube_debug_level > 0) {
      tprintf("Cube WARNING (Extract_cube_state): Could not retrieve "
              "cube's beam search object in extract_cube_state.\n");
    }
    return false;
  }

  // Get the character samples and bounding boxes by backtracking
  // through the beam search path
  int best_node_index = beam_search_obj->BestPresortedNodeIndex();
  *char_samples = beam_search_obj->BackTrack(
      cube_search_obj, best_node_index, num_chars, NULL, char_boxes);
  if (!*char_samples)
    return false;
  return true;
}

/**********************************************************************
 * create_cube_box_word
 *
 * Fill the given BoxWord with boxes from character bounding
 * boxes. The char_boxes have local coordinates w.r.t. the
 * word bounding box, i.e., the left-most character bbox of each word
 * has (0,0) left-top coord, but the BoxWord must be defined in page
 * coordinates.
 **********************************************************************/
bool Tesseract::create_cube_box_word(Boxa *char_boxes,
                                     int num_chars,
                                     TBOX word_box,
                                     BoxWord* box_word) {
  if (!box_word) {
    if (cube_debug_level > 0) {
      tprintf("Cube WARNING (create_cube_box_word): Invalid box_word.\n");
    }
    return false;
  }

  // Find the x-coordinate of left-most char_box, which could be
  // nonzero if the word image was padded before recognition took place.
  int x_offset = -1;
  for (int i = 0; i < num_chars; ++i) {
    Box* char_box = boxaGetBox(char_boxes, i, L_CLONE);
    if (x_offset < 0 || char_box->x < x_offset) {
      x_offset = char_box->x;
    }
    boxDestroy(&char_box);
  }

  for (int i = 0; i < num_chars; ++i) {
    Box* char_box = boxaGetBox(char_boxes, i, L_CLONE);
    TBOX tbox = char_box_to_tbox(char_box, word_box, x_offset);
    boxDestroy(&char_box);
    box_word->InsertBox(i, tbox);
  }
  return true;
}

/**********************************************************************
 * create_werd_choice
 *
 **********************************************************************/
static WERD_CHOICE *create_werd_choice(
                                       CharSamp** char_samples,
                                       int num_chars,
                                       const char* str,
                                       float certainty,
                                       const UNICHARSET &unicharset,
                                       CharSet* cube_char_set
                                       ) {
  // Insert unichar ids into WERD_CHOICE
  WERD_CHOICE *werd_choice = new WERD_CHOICE(num_chars);
  ASSERT_HOST(werd_choice != NULL);
  UNICHAR_ID uch_id;
  for (int i = 0; i < num_chars; ++i) {
    uch_id = cube_char_set->UnicharID(char_samples[i]->StrLabel());
    if (uch_id != INVALID_UNICHAR_ID)
      werd_choice->append_unichar_id_space_allocated(uch_id, 1, 0.0, certainty);
  }

  BLOB_CHOICE *blob_choice;
  BLOB_CHOICE_LIST *choices_list;
  BLOB_CHOICE_IT choices_list_it;
  BLOB_CHOICE_LIST_CLIST *blob_choices = new BLOB_CHOICE_LIST_CLIST();
  BLOB_CHOICE_LIST_C_IT blob_choices_it;
  blob_choices_it.set_to_list(blob_choices);

  for (int i = 0; i < werd_choice->length(); ++i) {
    // Create new BLOB_CHOICE_LIST for this unichar
    choices_list = new BLOB_CHOICE_LIST();
    choices_list_it.set_to_list(choices_list);
    // Add a single BLOB_CHOICE to the list
    blob_choice = new BLOB_CHOICE(werd_choice->unichar_id(i),
                                  0.0, certainty, -1, -1, 0);
    choices_list_it.add_after_then_move(blob_choice);
    // Add list to the clist
    blob_choices_it.add_to_end(choices_list);
  }
  werd_choice->populate_unichars(unicharset);
  werd_choice->set_certainty(certainty);
  werd_choice->set_blob_choices(blob_choices);
  return werd_choice;
}

/**********************************************************************
 * init_cube_objects
 *
 * Instantitates Tesseract object's CubeRecoContext and TesseractCubeCombiner.
 * Returns false if cube context could not be created or if load_combiner is
 * true, but the combiner could not be loaded.
 **********************************************************************/
bool Tesseract::init_cube_objects(bool load_combiner,
                                  TessdataManager *tessdata_manager) {
  ASSERT_HOST(cube_cntxt_ == NULL);
  ASSERT_HOST(tess_cube_combiner_ == NULL);

  // Create the cube context object
  cube_cntxt_ = CubeRecoContext::Create(this, tessdata_manager, &unicharset);
  if (cube_cntxt_ == NULL) {
    if (cube_debug_level > 0) {
      tprintf("Cube WARNING (Tesseract::init_cube_objects()): Failed to "
              "instantiate CubeRecoContext\n");
    }
    return false;
  }

  // Create the combiner object and load the combiner net for target languages.
  if (load_combiner) {
    tess_cube_combiner_ = new tesseract::TesseractCubeCombiner(cube_cntxt_);
    if (!tess_cube_combiner_ || !tess_cube_combiner_->LoadCombinerNet()) {
      delete cube_cntxt_;
      cube_cntxt_ = NULL;
      if (tess_cube_combiner_ != NULL) {
        delete tess_cube_combiner_;
        tess_cube_combiner_ = NULL;
      }
      if (cube_debug_level > 0)
        tprintf("Cube ERROR (Failed to instantiate TesseractCubeCombiner\n");
      return false;
    }
  }
  return true;
}

/**********************************************************************
 * run_cube
 *
 * Iterate through tesseract's results and call cube on each word.
 * If the combiner is present, optionally run the tesseract-cube
 * combiner on each word.
 **********************************************************************/
void Tesseract::run_cube(
                         PAGE_RES *page_res  // page structure
                         ) {
  ASSERT_HOST(cube_cntxt_ != NULL);
  if (!pix_binary_) {
    if (cube_debug_level > 0)
      tprintf("Tesseract::run_cube(): NULL binary image.\n");
    return;
  }
  if (!page_res)
    return;
  PAGE_RES_IT page_res_it(page_res);
  page_res_it.restart_page();

  // Iterate through the word results and call cube on each word.
  CubeObject *cube_obj;
  for (page_res_it.restart_page(); page_res_it.word () != NULL;
       page_res_it.forward()) {
    WERD_RES* word = page_res_it.word();
    TBOX word_box = word->word->bounding_box();
    const BLOCK* block = word->denorm.block();
    if (block != NULL && (block->re_rotation().x() != 1.0f ||
          block->re_rotation().y() != 0.0f)) {
      // TODO(rays) We have to rotate the bounding box to get the true coords.
      // This will be achieved in the future via DENORM.
      // In the mean time, cube can't process this word.
      if (cube_debug_level > 0) {
        tprintf("Cube can't process rotated word at:");
        word_box.print();
      }
      if (word->best_choice == NULL)
        page_res_it.DeleteCurrentWord();  // Nobody has an answer.
      continue;
    }
    cube_obj = new tesseract::CubeObject(cube_cntxt_, pix_binary_,
                                         word_box.left(),
                                         pix_binary_->h - word_box.top(),
                                         word_box.width(), word_box.height());
    cube_recognize(cube_obj, &page_res_it);
    delete cube_obj;
  }
}

/**********************************************************************
 * cube_recognize
 *
 * Call cube on the current word, optionally run the tess-cube combiner, and
 * modify the tesseract result if cube wins. If cube fails to run, or
 * if tesseract wins, leave the tesseract result unchanged. If the
 * combiner is not instantiated, always use cube's result.
 *
 **********************************************************************/
void Tesseract::cube_recognize(
                               CubeObject *cube_obj,
                               PAGE_RES_IT *page_res_it
                               ) {
  // Retrieve tesseract's data structure for the current word.
  WERD_RES *tess_werd_res = page_res_it->word();
  if (!tess_werd_res->best_choice && tess_cube_combiner_ != NULL) {
    if (cube_debug_level > 0)
      tprintf("Cube WARNING (Tesseract::cube_recognize): Cannot run combiner "
              "without a tess result.\n");
    return;
  }

  // Skip cube entirely if combiner is present but tesseract's
  // certainty is greater than threshold.
  int combiner_run_thresh = convert_prob_to_tess_certainty(
      cube_cntxt_->Params()->CombinerRunThresh());
  if (tess_cube_combiner_ != NULL &&
      (tess_werd_res->best_choice->certainty() >= combiner_run_thresh)) {
    return;
  }

  // Run cube
  WordAltList *cube_alt_list = cube_obj->RecognizeWord();
  if (!cube_alt_list || cube_alt_list->AltCount() <= 0) {
    if (cube_debug_level > 0) {
      tprintf("Cube returned nothing for word at:");
      tess_werd_res->word->bounding_box().print();
    }
    if (tess_werd_res->best_choice == NULL) {
      // Nobody has recognized it, so pretend it doesn't exist.
      if (cube_debug_level > 0) {
        tprintf("Deleted word not recognized by cube and/or tesseract at:");
        tess_werd_res->word->bounding_box().print();
      }
      page_res_it->DeleteCurrentWord();
    }
    return;
  }

  // At this point we *could* run the combiner and bail out if
  // Tesseract wins, but that would require instantiating a new
  // CubeObject to avoid losing the original recognition results
  // (e.g., beam search lattice) stored with the CubeObject. Instead,
  // we first extract the state we need from the current recognition
  // and then reuse the CubeObject so that the combiner does not need
  // to recompute the image's connected components, segmentation, etc.

  // Get cube's best result and its probability, mapped to tesseract's
  // certainty range
  char_32 *cube_best_32 = cube_alt_list->Alt(0);
  double cube_prob = CubeUtils::Cost2Prob(cube_alt_list->AltCost(0));
  float cube_certainty = convert_prob_to_tess_certainty(cube_prob);
  string cube_best_str;
  CubeUtils::UTF32ToUTF8(cube_best_32, &cube_best_str);

  // Retrieve Cube's character bounding boxes and CharSamples,
  // corresponding to the most recent call to RecognizeWord().
  Boxa *char_boxes = NULL;
  CharSamp **char_samples = NULL;;
  int num_chars;
  if (!extract_cube_state(cube_obj, &num_chars, &char_boxes, &char_samples)
      && cube_debug_level > 0) {
    tprintf("Cube WARNING (Tesseract::cube_recognize): Cannot extract "
            "cube state.\n");
    return;
  }

  // Convert cube's character bounding boxes to a BoxWord.
  BoxWord cube_box_word;
  TBOX tess_word_box = tess_werd_res->word->bounding_box();
  if (tess_werd_res->denorm.block() != NULL)
    tess_word_box.rotate(tess_werd_res->denorm.block()->re_rotation());
  bool box_word_success = create_cube_box_word(char_boxes, num_chars,
                                               tess_word_box,
                                               &cube_box_word);
  boxaDestroy(&char_boxes);
  if (!box_word_success) {
    if (cube_debug_level > 0) {
      tprintf("Cube WARNING (Tesseract::cube_recognize): Could not "
              "create cube BoxWord\n");
    }
    return;
  }

  // Create cube's best choice.
  WERD_CHOICE* cube_werd_choice = create_werd_choice(
      char_samples, num_chars, cube_best_str.c_str(), cube_certainty,
      unicharset, cube_cntxt_->CharacterSet());
  delete []char_samples;

  if (!cube_werd_choice) {
    if (cube_debug_level > 0) {
      tprintf("Cube WARNING (Tesseract::cube_recognize): Could not "
              "create cube WERD_CHOICE\n");
    }
    return;
  }

  // Run combiner if present, now that we're free to reuse the CubeObject.
  if (tess_cube_combiner_ != NULL) {
    float combiner_prob = tess_cube_combiner_->CombineResults(tess_werd_res,
                                                              cube_obj);
    // If combiner probability is greater than tess/cube combiner
    // classifier threshold, i.e. tesseract wins, then reset the WERD_RES
    // certainty to the combiner certainty and return. Note that when
    // tesseract and cube agree, the combiner probability is 1.0, so
    // the final WERD_RES certainty will be maximized to 0.0.
    if (combiner_prob >=
        cube_cntxt_->Params()->CombinerClassifierThresh()) {
      float combiner_certainty = convert_prob_to_tess_certainty(combiner_prob);
      tess_werd_res->best_choice->set_certainty(combiner_certainty);
      delete cube_werd_choice;
      return;
    }
    if (cube_debug_level > 5) {
      tprintf("Cube INFO: tesseract result replaced by cube: "
              "%s -> %s\n",
              tess_werd_res->best_choice->unichar_string().string(),
              cube_best_str.c_str());
    }
  }

  // Fill tesseract result's fields with cube results
  fill_werd_res(cube_box_word, cube_werd_choice, cube_best_str.c_str(),
                page_res_it);
}

/**********************************************************************
 * fill_werd_res
 *
 * Fill Tesseract's word result fields with cube's.
 *
 **********************************************************************/
void Tesseract::fill_werd_res(const BoxWord& cube_box_word,
                              WERD_CHOICE* cube_werd_choice,
                              const char* cube_best_str,
                              PAGE_RES_IT *page_res_it) {
  WERD_RES *tess_werd_res = page_res_it->word();

  // Replace tesseract results's best choice with cube's
  delete tess_werd_res->best_choice;
  tess_werd_res->best_choice = cube_werd_choice;

  delete tess_werd_res->box_word;
  tess_werd_res->box_word = new BoxWord(cube_box_word);
  tess_werd_res->box_word->ClipToOriginalWord(page_res_it->block()->block,
                                              tess_werd_res->word);
  // Fill text and remaining fields
  tess_werd_res->word->set_text(cube_best_str);
  tess_werd_res->tess_failed = FALSE;
  tess_werd_res->tess_accepted =
      tess_acceptable_word(tess_werd_res->best_choice,
                           tess_werd_res->raw_choice);
  // There is no output word, so we can' call AdaptableWord, but then I don't
  // think we need to. Fudge the result with accepted.
  tess_werd_res->tess_would_adapt = tess_werd_res->tess_accepted;

  // Initialize the reject_map and set it to done, i.e., ignore all of
  // tesseract's tests for rejection
  tess_werd_res->reject_map.initialise(cube_werd_choice->length());
  tess_werd_res->done = tess_werd_res->tess_accepted;

  // Some sanity checks
  ASSERT_HOST(tess_werd_res->best_choice->length() ==
              tess_werd_res->best_choice->blob_choices()->length());
  ASSERT_HOST(tess_werd_res->best_choice->length() ==
              tess_werd_res->reject_map.length());
}

}  // namespace tesseract
