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

#include "allheaders.h"

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
  WERD_CHOICE *werd_choice = new WERD_CHOICE(&unicharset, num_chars);
  // within a word, cube recognizes the word in reading order.
  werd_choice->set_unichars_in_script_order(true);
  ASSERT_HOST(werd_choice != NULL);
  UNICHAR_ID uch_id;
  for (int i = 0; i < num_chars; ++i) {
    uch_id = cube_char_set->UnicharID(char_samples[i]->StrLabel());
    if (uch_id != INVALID_UNICHAR_ID)
      werd_choice->append_unichar_id_space_allocated(
          uch_id, 1, 0.0, certainty);
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
                                  0.0, certainty, -1, -1, 0, 0, 0, false);
    choices_list_it.add_after_then_move(blob_choice);
    // Add list to the clist
    blob_choices_it.add_to_end(choices_list);
  }
  werd_choice->set_certainty(certainty);
  werd_choice->set_blob_choices(blob_choices);
  return werd_choice;
}

/**********************************************************************
 * init_cube_objects
 *
 * Instantiates Tesseract object's CubeRecoContext and TesseractCubeCombiner.
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
 * run_cube_combiner
 *
 * Iterates through tesseract's results and calls cube on each word,
 * combining the results with the existing tesseract result.
 **********************************************************************/
void Tesseract::run_cube_combiner(PAGE_RES *page_res) {
  if (page_res == NULL || tess_cube_combiner_ == NULL)
    return;
  PAGE_RES_IT page_res_it(page_res);
  // Iterate through the word results and call cube on each word.
  for (page_res_it.restart_page(); page_res_it.word () != NULL;
       page_res_it.forward()) {
    WERD_RES* word = page_res_it.word();
    // Skip cube entirely if tesseract's certainty is greater than threshold.
    int combiner_run_thresh = convert_prob_to_tess_certainty(
        cube_cntxt_->Params()->CombinerRunThresh());
    if (word->best_choice->certainty() >= combiner_run_thresh) {
      continue;
    }
    // Use the same language as Tesseract used for the word.
    Tesseract* lang_tess = word->tesseract;

    // Setup a trial WERD_RES in which to classify with cube.
    WERD_RES cube_word;
    cube_word.InitForRetryRecognition(*word);
    CubeObject *cube_obj = lang_tess->cube_recognize_word(
        page_res_it.block()->block, &cube_word);
    if (cube_obj != NULL)
      lang_tess->cube_combine_word(cube_obj, &cube_word, word);
    delete cube_obj;
  }
}

/**********************************************************************
 * cube_word_pass1
 *
 * Recognizes a single word using (only) cube. Compatible with
 * Tesseract's classify_word_pass1/classify_word_pass2.
 **********************************************************************/
void Tesseract::cube_word_pass1(BLOCK* block, ROW *row, WERD_RES *word) {
  CubeObject *cube_obj = cube_recognize_word(block, word);
  delete cube_obj;
}

/**********************************************************************
 * cube_recognize_word
 *
 * Cube recognizer to recognize a single word as with classify_word_pass1
 * but also returns the cube object in case the combiner is needed.
 **********************************************************************/
CubeObject* Tesseract::cube_recognize_word(BLOCK* block, WERD_RES* word) {
  if (!cube_binary_ || !cube_cntxt_) {
    if (cube_debug_level > 0 && !cube_binary_)
      tprintf("Tesseract::run_cube(): NULL binary image.\n");
    word->SetupFake(unicharset);
    return NULL;
  }
  TBOX word_box = word->word->bounding_box();
  if (block != NULL && (block->re_rotation().x() != 1.0f ||
        block->re_rotation().y() != 0.0f)) {
    // TODO(rays) We have to rotate the bounding box to get the true coords.
    // This will be achieved in the future via DENORM.
    // In the mean time, cube can't process this word.
    if (cube_debug_level > 0) {
      tprintf("Cube can't process rotated word at:");
      word_box.print();
    }
    word->SetupFake(unicharset);
    return NULL;
  }
  CubeObject* cube_obj = new tesseract::CubeObject(
      cube_cntxt_, cube_binary_, word_box.left(),
      pixGetHeight(cube_binary_) - word_box.top(),
      word_box.width(), word_box.height());
  if (!cube_recognize(cube_obj, block, word)) {
    delete cube_obj;
    return NULL;
  }
  return cube_obj;
}

/**********************************************************************
 * cube_combine_word
 *
 * Combines the cube and tesseract results for a single word, leaving the
 * result in tess_word.
 **********************************************************************/
void Tesseract::cube_combine_word(CubeObject* cube_obj, WERD_RES* cube_word,
                                  WERD_RES* tess_word) {
  float combiner_prob = tess_cube_combiner_->CombineResults(tess_word,
                                                            cube_obj);
  // If combiner probability is greater than tess/cube combiner
  // classifier threshold, i.e. tesseract wins, then just return the
  // tesseract result unchanged, as the combiner knows nothing about how
  // correct the answer is. If cube and tesseract agree, then improve the
  // scores before returning.
  WERD_CHOICE* tess_best = tess_word->best_choice;
  WERD_CHOICE* cube_best = cube_word->best_choice;
  if (cube_debug_level || classify_debug_level) {
    tprintf("Combiner prob = %g vs threshold %g\n",
            combiner_prob, cube_cntxt_->Params()->CombinerClassifierThresh());
  }
  if (combiner_prob >=
      cube_cntxt_->Params()->CombinerClassifierThresh()) {
    if (tess_best->unichar_string() == cube_best->unichar_string()) {
      // Cube and tess agree, so improve the scores.
      tess_best->set_rating(tess_best->rating() / 2);
      tess_best->set_certainty(tess_best->certainty() / 2);
    }
    return;
  }
  // Cube wins.
  // It is better for the language combiner to have all tesseract scores,
  // so put them in the cube result.
  cube_best->set_rating(tess_best->rating());
  cube_best->set_certainty(tess_best->certainty());
  if (cube_debug_level || classify_debug_level) {
    tprintf("Cube INFO: tesseract result replaced by cube: %s -> %s\n",
            tess_best->unichar_string().string(),
            cube_best->unichar_string().string());
  }
  tess_word->ConsumeWordResults(cube_word);
}

/**********************************************************************
 * cube_recognize
 *
 * Call cube on the current word, and write the result to word.
 * Sets up a fake result and returns false if something goes wrong.
 **********************************************************************/
bool Tesseract::cube_recognize(CubeObject *cube_obj, BLOCK* block,
                               WERD_RES *word) {
  if (!word->SetupForCubeRecognition(unicharset, this, block)) {
    return false;  // Graphics block.
  }

  // Run cube
  WordAltList *cube_alt_list = cube_obj->RecognizeWord();
  if (!cube_alt_list || cube_alt_list->AltCount() <= 0) {
    if (cube_debug_level > 0) {
      tprintf("Cube returned nothing for word at:");
      word->word->bounding_box().print();
    }
    word->SetupFake(unicharset);
    return false;
  }

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
    word->SetupFake(unicharset);
    return false;
  }

  // Convert cube's character bounding boxes to a BoxWord.
  BoxWord cube_box_word;
  TBOX tess_word_box = word->word->bounding_box();
  if (word->denorm.block() != NULL)
    tess_word_box.rotate(word->denorm.block()->re_rotation());
  bool box_word_success = create_cube_box_word(char_boxes, num_chars,
                                               tess_word_box,
                                               &cube_box_word);
  boxaDestroy(&char_boxes);
  if (!box_word_success) {
    if (cube_debug_level > 0) {
      tprintf("Cube WARNING (Tesseract::cube_recognize): Could not "
              "create cube BoxWord\n");
    }
    word->SetupFake(unicharset);
    return false;
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
    word->SetupFake(unicharset);
    return false;
  }
  if (cube_debug_level || classify_debug_level) {
    tprintf("Cube result: %s r=%g, c=%g\n",
            cube_werd_choice->unichar_string().string(),
            cube_werd_choice->rating(),
            cube_werd_choice->certainty());
  }

  // Fill tesseract result's fields with cube results
  fill_werd_res(cube_box_word, cube_werd_choice, cube_best_str.c_str(), word);
  return true;
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
                              WERD_RES* tess_werd_res) {
  // Replace tesseract results's best choice with cube's
  tess_werd_res->best_choice = cube_werd_choice;
  tess_werd_res->raw_choice = new WERD_CHOICE(*cube_werd_choice);

  delete tess_werd_res->box_word;
  tess_werd_res->box_word = new BoxWord(cube_box_word);
  tess_werd_res->box_word->ClipToOriginalWord(tess_werd_res->denorm.block(),
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
