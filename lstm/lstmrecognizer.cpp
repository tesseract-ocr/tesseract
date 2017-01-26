///////////////////////////////////////////////////////////////////////
// File:        lstmrecognizer.cpp
// Description: Top-level line recognizer class for LSTM-based networks.
// Author:      Ray Smith
// Created:     Thu May 02 10:59:06 PST 2013
//
// (C) Copyright 2013, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "lstmrecognizer.h"

#include "allheaders.h"
#include "callcpp.h"
#include "dict.h"
#include "genericheap.h"
#include "helpers.h"
#include "imagedata.h"
#include "input.h"
#include "lstm.h"
#include "normalis.h"
#include "pageres.h"
#include "ratngs.h"
#include "recodebeam.h"
#include "scrollview.h"
#include "shapetable.h"
#include "statistc.h"
#include "tprintf.h"

namespace tesseract {

// Max number of blob choices to return in any given position.
const int kMaxChoices = 4;
// Default ratio between dict and non-dict words.
const double kDictRatio = 2.25;
// Default certainty offset to give the dictionary a chance.
const double kCertOffset = -0.085;

LSTMRecognizer::LSTMRecognizer()
    : network_(NULL),
      training_flags_(0),
      training_iteration_(0),
      sample_iteration_(0),
      null_char_(UNICHAR_BROKEN),
      weight_range_(0.0f),
      learning_rate_(0.0f),
      momentum_(0.0f),
      dict_(NULL),
      search_(NULL),
      debug_win_(NULL) {}

LSTMRecognizer::~LSTMRecognizer() {
  delete network_;
  delete dict_;
  delete search_;
}

// Writes to the given file. Returns false in case of error.
bool LSTMRecognizer::Serialize(TFile* fp) const {
  if (!network_->Serialize(fp)) return false;
  if (!GetUnicharset().save_to_file(fp)) return false;
  if (!network_str_.Serialize(fp)) return false;
  if (fp->FWrite(&training_flags_, sizeof(training_flags_), 1) != 1)
    return false;
  if (fp->FWrite(&training_iteration_, sizeof(training_iteration_), 1) != 1)
    return false;
  if (fp->FWrite(&sample_iteration_, sizeof(sample_iteration_), 1) != 1)
    return false;
  if (fp->FWrite(&null_char_, sizeof(null_char_), 1) != 1) return false;
  if (fp->FWrite(&weight_range_, sizeof(weight_range_), 1) != 1) return false;
  if (fp->FWrite(&learning_rate_, sizeof(learning_rate_), 1) != 1) return false;
  if (fp->FWrite(&momentum_, sizeof(momentum_), 1) != 1) return false;
  if (IsRecoding() && !recoder_.Serialize(fp)) return false;
  return true;
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool LSTMRecognizer::DeSerialize(bool swap, TFile* fp) {
  delete network_;
  network_ = Network::CreateFromFile(swap, fp);
  if (network_ == NULL) return false;
  if (!ccutil_.unicharset.load_from_file(fp, false)) return false;
  if (!network_str_.DeSerialize(swap, fp)) return false;
  if (fp->FRead(&training_flags_, sizeof(training_flags_), 1) != 1)
    return false;
  if (fp->FRead(&training_iteration_, sizeof(training_iteration_), 1) != 1)
    return false;
  if (fp->FRead(&sample_iteration_, sizeof(sample_iteration_), 1) != 1)
    return false;
  if (fp->FRead(&null_char_, sizeof(null_char_), 1) != 1) return false;
  if (fp->FRead(&weight_range_, sizeof(weight_range_), 1) != 1) return false;
  if (fp->FRead(&learning_rate_, sizeof(learning_rate_), 1) != 1) return false;
  if (fp->FRead(&momentum_, sizeof(momentum_), 1) != 1) return false;
  if (IsRecoding()) {
    if (!recoder_.DeSerialize(swap, fp)) return false;
    RecodedCharID code;
    recoder_.EncodeUnichar(UNICHAR_SPACE, &code);
    if (code(0) != UNICHAR_SPACE) {
      tprintf("Space was garbled in recoding!!\n");
      return false;
    }
  }
  // TODO(rays) swaps!
  network_->SetRandomizer(&randomizer_);
  network_->CacheXScaleFactor(network_->XScaleFactor());
  return true;
}

// Loads the dictionary if possible from the traineddata file.
// Prints a warning message, and returns false but otherwise fails silently
// and continues to work without it if loading fails.
// Note that dictionary load is independent from DeSerialize, but dependent
// on the unicharset matching. This enables training to deserialize a model
// from checkpoint or restore without having to go back and reload the
// dictionary.
bool LSTMRecognizer::LoadDictionary(const char* data_file_name,
                                    const char* lang) {
  delete dict_;
  dict_ = new Dict(&ccutil_);
  dict_->SetupForLoad(Dict::GlobalDawgCache());
  dict_->LoadLSTM(data_file_name, lang);
  if (dict_->FinishLoad()) return true;  // Success.
  tprintf("Failed to load any lstm-specific dictionaries for lang %s!!\n",
          lang);
  delete dict_;
  dict_ = NULL;
  return false;
}

// Recognizes the line image, contained within image_data, returning the
// ratings matrix and matching box_word for each WERD_RES in the output.
void LSTMRecognizer::RecognizeLine(const ImageData& image_data, bool invert,
                                   bool debug, double worst_dict_cert,
                                   bool use_alternates,
                                   const UNICHARSET* target_unicharset,
                                   const TBOX& line_box, float score_ratio,
                                   bool one_word,
                                   PointerVector<WERD_RES>* words) {
  NetworkIO outputs;
  float label_threshold = use_alternates ? 0.75f : 0.0f;
  float scale_factor;
  NetworkIO inputs;
  if (!RecognizeLine(image_data, invert, debug, false, label_threshold,
                     &scale_factor, &inputs, &outputs))
    return;
  if (IsRecoding()) {
    if (search_ == NULL) {
      search_ =
          new RecodeBeamSearch(recoder_, null_char_, SimpleTextOutput(), dict_);
    }
    search_->Decode(outputs, kDictRatio, kCertOffset, worst_dict_cert, NULL);
    search_->ExtractBestPathAsWords(line_box, scale_factor, debug,
                                    &GetUnicharset(), words);
  } else {
    GenericVector<int> label_coords;
    GenericVector<int> labels;
    LabelsFromOutputs(outputs, label_threshold, &labels, &label_coords);
    WordsFromOutputs(outputs, labels, label_coords, line_box, debug,
                     use_alternates, one_word, score_ratio, scale_factor,
                     target_unicharset, words);
  }
}

// Builds a set of tesseract-compatible WERD_RESs aligned to line_box,
// corresponding to the network output in outputs, labels, label_coords.
// one_word generates a single word output, that may include spaces inside.
// use_alternates generates alternative BLOB_CHOICEs and segmentation paths.
// If not NULL, we attempt to translate the output to target_unicharset, but do
// not guarantee success, due to mismatches. In that case the output words are
// marked with our UNICHARSET, not the caller's.
void LSTMRecognizer::WordsFromOutputs(
    const NetworkIO& outputs, const GenericVector<int>& labels,
    const GenericVector<int> label_coords, const TBOX& line_box, bool debug,
    bool use_alternates, bool one_word, float score_ratio, float scale_factor,
    const UNICHARSET* target_unicharset, PointerVector<WERD_RES>* words) {
  // Convert labels to unichar-ids.
  int word_end = 0;
  float prev_space_cert = 0.0f;
  for (int i = 0; i < labels.size(); i = word_end) {
    word_end = i + 1;
    if (labels[i] == null_char_ || labels[i] == UNICHAR_SPACE) {
      continue;
    }
    float space_cert = 0.0f;
    if (one_word) {
      word_end = labels.size();
    } else {
      // Find the end of the word at the first null_char_ that leads to the
      // first UNICHAR_SPACE.
      while (word_end < labels.size() && labels[word_end] != UNICHAR_SPACE)
        ++word_end;
      if (word_end < labels.size()) {
        float rating;
        outputs.ScoresOverRange(label_coords[word_end],
                                label_coords[word_end] + 1, UNICHAR_SPACE,
                                null_char_, &rating, &space_cert);
      }
      while (word_end > i && labels[word_end - 1] == null_char_) --word_end;
    }
    ASSERT_HOST(word_end > i);
    // Create a WERD_RES for the output word.
    if (debug)
      tprintf("Creating word from outputs over [%d,%d)\n", i, word_end);
    WERD_RES* word =
        WordFromOutput(line_box, outputs, i, word_end, score_ratio,
                       MIN(prev_space_cert, space_cert), debug,
                       use_alternates && !SimpleTextOutput(), target_unicharset,
                       labels, label_coords, scale_factor);
    if (word == NULL && target_unicharset != NULL) {
      // Unicharset translation failed - use decoder_ instead, and disable
      // the segmentation search on output, as it won't understand the encoding.
      word = WordFromOutput(line_box, outputs, i, word_end, score_ratio,
                            MIN(prev_space_cert, space_cert), debug, false,
                            NULL, labels, label_coords, scale_factor);
    }
    prev_space_cert = space_cert;
    words->push_back(word);
  }
}

// Helper computes min and mean best results in the output.
void LSTMRecognizer::OutputStats(const NetworkIO& outputs, float* min_output,
                                 float* mean_output, float* sd) {
  const int kOutputScale = MAX_INT8;
  STATS stats(0, kOutputScale + 1);
  for (int t = 0; t < outputs.Width(); ++t) {
    int best_label = outputs.BestLabel(t, NULL);
    if (best_label != null_char_ || t == 0) {
      float best_output = outputs.f(t)[best_label];
      stats.add(static_cast<int>(kOutputScale * best_output), 1);
    }
  }
  *min_output = static_cast<float>(stats.min_bucket()) / kOutputScale;
  *mean_output = stats.mean() / kOutputScale;
  *sd = stats.sd() / kOutputScale;
}

// Recognizes the image_data, returning the labels,
// scores, and corresponding pairs of start, end x-coords in coords.
// If label_threshold is positive, uses it for making the labels, otherwise
// uses standard ctc.
bool LSTMRecognizer::RecognizeLine(const ImageData& image_data, bool invert,
                                   bool debug, bool re_invert,
                                   float label_threshold, float* scale_factor,
                                   NetworkIO* inputs, NetworkIO* outputs) {
  // Maximum width of image to train on.
  const int kMaxImageWidth = 2560;
  // This ensures consistent recognition results.
  SetRandomSeed();
  int min_width = network_->XScaleFactor();
  Pix* pix = Input::PrepareLSTMInputs(image_data, network_, min_width,
                                      &randomizer_, scale_factor);
  if (pix == NULL) {
    tprintf("Line cannot be recognized!!\n");
    return false;
  }
  if (network_->IsTraining() && pixGetWidth(pix) > kMaxImageWidth) {
    tprintf("Image too large to learn!! Size = %dx%d\n", pixGetWidth(pix),
            pixGetHeight(pix));
    pixDestroy(&pix);
    return false;
  }
  // Reduction factor from image to coords.
  *scale_factor = min_width / *scale_factor;
  inputs->set_int_mode(IsIntMode());
  SetRandomSeed();
  Input::PreparePixInput(network_->InputShape(), pix, &randomizer_, inputs);
  network_->Forward(debug, *inputs, NULL, &scratch_space_, outputs);
  // Check for auto inversion.
  float pos_min, pos_mean, pos_sd;
  OutputStats(*outputs, &pos_min, &pos_mean, &pos_sd);
  if (invert && pos_min < 0.5) {
    // Run again inverted and see if it is any better.
    NetworkIO inv_inputs, inv_outputs;
    inv_inputs.set_int_mode(IsIntMode());
    SetRandomSeed();
    pixInvert(pix, pix);
    Input::PreparePixInput(network_->InputShape(), pix, &randomizer_,
                           &inv_inputs);
    network_->Forward(debug, inv_inputs, NULL, &scratch_space_, &inv_outputs);
    float inv_min, inv_mean, inv_sd;
    OutputStats(inv_outputs, &inv_min, &inv_mean, &inv_sd);
    if (inv_min > pos_min && inv_mean > pos_mean && inv_sd < pos_sd) {
      // Inverted did better. Use inverted data.
      if (debug) {
        tprintf("Inverting image: old min=%g, mean=%g, sd=%g, inv %g,%g,%g\n",
                pos_min, pos_mean, pos_sd, inv_min, inv_mean, inv_sd);
      }
      *outputs = inv_outputs;
      *inputs = inv_inputs;
    } else if (re_invert) {
      // Inverting was not an improvement, so undo and run again, so the
      // outputs match the best forward result.
      SetRandomSeed();
      network_->Forward(debug, *inputs, NULL, &scratch_space_, outputs);
    }
  }
  pixDestroy(&pix);
  if (debug) {
    GenericVector<int> labels, coords;
    LabelsFromOutputs(*outputs, label_threshold, &labels, &coords);
    DisplayForward(*inputs, labels, coords, "LSTMForward", &debug_win_);
    DebugActivationPath(*outputs, labels, coords);
  }
  return true;
}

// Returns a tesseract-compatible WERD_RES from the line recognizer outputs.
// line_box should be the bounding box of the line image in the main image,
// outputs the output of the network,
// [word_start, word_end) the interval over which to convert,
// score_ratio for choosing alternate classifier choices,
// use_alternates to control generation of alternative segmentations,
// labels, label_coords, scale_factor from RecognizeLine above.
// If target_unicharset is not NULL, attempts to translate the internal
// unichar_ids to the target_unicharset, but falls back to untranslated ids
// if the translation should fail.
WERD_RES* LSTMRecognizer::WordFromOutput(
    const TBOX& line_box, const NetworkIO& outputs, int word_start,
    int word_end, float score_ratio, float space_certainty, bool debug,
    bool use_alternates, const UNICHARSET* target_unicharset,
    const GenericVector<int>& labels, const GenericVector<int>& label_coords,
    float scale_factor) {
  WERD_RES* word_res = InitializeWord(
      line_box, word_start, word_end, space_certainty, use_alternates,
      target_unicharset, labels, label_coords, scale_factor);
  int max_blob_run = word_res->ratings->bandwidth();
  for (int width = 1; width <= max_blob_run; ++width) {
    int col = 0;
    for (int i = word_start; i + width <= word_end; ++i) {
      if (labels[i] != null_char_) {
        // Starting at i, use width labels, but stop at the next null_char_.
        // This forms all combinations of blobs between regions of null_char_.
        int j = i + 1;
        while (j - i < width && labels[j] != null_char_) ++j;
        if (j - i == width) {
          // Make the blob choices.
          int end_coord = label_coords[j];
          if (j < word_end && labels[j] == null_char_)
            end_coord = label_coords[j + 1];
          BLOB_CHOICE_LIST* choices = GetBlobChoices(
              col, col + width - 1, debug, outputs, target_unicharset,
              label_coords[i], end_coord, score_ratio);
          if (choices == NULL) {
            delete word_res;
            return NULL;
          }
          word_res->ratings->put(col, col + width - 1, choices);
        }
        ++col;
      }
    }
  }
  if (use_alternates) {
    // Merge adjacent single results over null_char boundaries.
    int col = 0;
    for (int i = word_start; i + 2 < word_end; ++i) {
      if (labels[i] != null_char_ && labels[i + 1] == null_char_ &&
          labels[i + 2] != null_char_ &&
          (i == word_start || labels[i - 1] == null_char_) &&
          (i + 3 == word_end || labels[i + 3] == null_char_)) {
        int end_coord = label_coords[i + 3];
        if (i + 3 < word_end && labels[i + 3] == null_char_)
          end_coord = label_coords[i + 4];
        BLOB_CHOICE_LIST* choices =
            GetBlobChoices(col, col + 1, debug, outputs, target_unicharset,
                           label_coords[i], end_coord, score_ratio);
        if (choices == NULL) {
          delete word_res;
          return NULL;
        }
        word_res->ratings->put(col, col + 1, choices);
      }
      if (labels[i] != null_char_) ++col;
    }
  } else {
    word_res->FakeWordFromRatings(TOP_CHOICE_PERM);
  }
  return word_res;
}

// Sets up a word with the ratings matrix and fake blobs with boxes in the
// right places.
WERD_RES* LSTMRecognizer::InitializeWord(const TBOX& line_box, int word_start,
                                         int word_end, float space_certainty,
                                         bool use_alternates,
                                         const UNICHARSET* target_unicharset,
                                         const GenericVector<int>& labels,
                                         const GenericVector<int>& label_coords,
                                         float scale_factor) {
  // Make a fake blob for each non-zero label.
  C_BLOB_LIST blobs;
  C_BLOB_IT b_it(&blobs);
  // num_blobs is the length of the diagonal of the ratings matrix.
  int num_blobs = 0;
  // max_blob_run is the diagonal width of the ratings matrix
  int max_blob_run = 0;
  int blob_run = 0;
  for (int i = word_start; i < word_end; ++i) {
    if (IsRecoding() && !recoder_.IsValidFirstCode(labels[i])) continue;
    if (labels[i] != null_char_) {
      // Make a fake blob.
      TBOX box(label_coords[i], 0, label_coords[i + 1], line_box.height());
      box.scale(scale_factor);
      box.move(ICOORD(line_box.left(), line_box.bottom()));
      box.set_top(line_box.top());
      b_it.add_after_then_move(C_BLOB::FakeBlob(box));
      ++num_blobs;
      ++blob_run;
    }
    if (labels[i] == null_char_ || i + 1 == word_end) {
      if (blob_run > max_blob_run)
        max_blob_run = blob_run;
    }
  }
  if (!use_alternates) max_blob_run = 1;
  ASSERT_HOST(label_coords.size() >= word_end);
  // Make a fake word from the blobs.
  WERD* word = new WERD(&blobs, word_start > 1 ? 1 : 0, NULL);
  // Make a WERD_RES from the word.
  WERD_RES* word_res = new WERD_RES(word);
  word_res->uch_set =
      target_unicharset != NULL ? target_unicharset : &GetUnicharset();
  word_res->combination = true;  // Give it ownership of the word.
  word_res->space_certainty = space_certainty;
  word_res->ratings = new MATRIX(num_blobs, max_blob_run);
  return word_res;
}

// Converts an array of labels to utf-8, whether or not the labels are
// augmented with character boundaries.
STRING LSTMRecognizer::DecodeLabels(const GenericVector<int>& labels) {
  STRING result;
  int end = 1;
  for (int start = 0; start < labels.size(); start = end) {
    if (labels[start] == null_char_) {
      end = start + 1;
    } else {
      result += DecodeLabel(labels, start, &end, NULL);
    }
  }
  return result;
}

// Displays the forward results in a window with the characters and
// boundaries as determined by the labels and label_coords.
void LSTMRecognizer::DisplayForward(const NetworkIO& inputs,
                                    const GenericVector<int>& labels,
                                    const GenericVector<int>& label_coords,
                                    const char* window_name,
                                    ScrollView** window) {
#ifndef GRAPHICS_DISABLED  // do nothing if there's no graphics
  Pix* input_pix = inputs.ToPix();
  Network::ClearWindow(false, window_name, pixGetWidth(input_pix),
                       pixGetHeight(input_pix), window);
  int line_height = Network::DisplayImage(input_pix, *window);
  DisplayLSTMOutput(labels, label_coords, line_height, *window);
#endif  // GRAPHICS_DISABLED
}

// Displays the labels and cuts at the corresponding xcoords.
// Size of labels should match xcoords.
void LSTMRecognizer::DisplayLSTMOutput(const GenericVector<int>& labels,
                                       const GenericVector<int>& xcoords,
                                       int height, ScrollView* window) {
#ifndef GRAPHICS_DISABLED  // do nothing if there's no graphics
  int x_scale = network_->XScaleFactor();
  window->TextAttributes("Arial", height / 4, false, false, false);
  int end = 1;
  for (int start = 0; start < labels.size(); start = end) {
    int xpos = xcoords[start] * x_scale;
    if (labels[start] == null_char_) {
      end = start + 1;
      window->Pen(ScrollView::RED);
    } else {
      window->Pen(ScrollView::GREEN);
      const char* str = DecodeLabel(labels, start, &end, NULL);
      if (*str == '\\') str = "\\\\";
      xpos = xcoords[(start + end) / 2] * x_scale;
      window->Text(xpos, height, str);
    }
    window->Line(xpos, 0, xpos, height * 3 / 2);
  }
  window->Update();
#endif  // GRAPHICS_DISABLED
}

// Prints debug output detailing the activation path that is implied by the
// label_coords.
void LSTMRecognizer::DebugActivationPath(const NetworkIO& outputs,
                                         const GenericVector<int>& labels,
                                         const GenericVector<int>& xcoords) {
  if (xcoords[0] > 0)
    DebugActivationRange(outputs, "<null>", null_char_, 0, xcoords[0]);
  int end = 1;
  for (int start = 0; start < labels.size(); start = end) {
    if (labels[start] == null_char_) {
      end = start + 1;
      DebugActivationRange(outputs, "<null>", null_char_, xcoords[start],
                           xcoords[end]);
      continue;
    } else {
      int decoded;
      const char* label = DecodeLabel(labels, start, &end, &decoded);
      DebugActivationRange(outputs, label, labels[start], xcoords[start],
                           xcoords[start + 1]);
      for (int i = start + 1; i < end; ++i) {
        DebugActivationRange(outputs, DecodeSingleLabel(labels[i]), labels[i],
                             xcoords[i], xcoords[i + 1]);
      }
    }
  }
}

// Prints debug output detailing activations and 2nd choice over a range
// of positions.
void LSTMRecognizer::DebugActivationRange(const NetworkIO& outputs,
                                          const char* label, int best_choice,
                                          int x_start, int x_end) {
  tprintf("%s=%d On [%d, %d), scores=", label, best_choice, x_start, x_end);
  double max_score = 0.0;
  double mean_score = 0.0;
  int width = x_end - x_start;
  for (int x = x_start; x < x_end; ++x) {
    const float* line = outputs.f(x);
    double score = line[best_choice] * 100.0;
    if (score > max_score) max_score = score;
    mean_score += score / width;
    int best_c = 0;
    double best_score = 0.0;
    for (int c = 0; c < outputs.NumFeatures(); ++c) {
      if (c != best_choice && line[c] > best_score) {
        best_c = c;
        best_score = line[c];
      }
    }
    tprintf(" %.3g(%s=%d=%.3g)", score, DecodeSingleLabel(best_c), best_c,
            best_score * 100.0);
  }
  tprintf(", Mean=%g, max=%g\n", mean_score, max_score);
}

// Helper returns true if the null_char is the winner at t, and it beats the
// null_threshold, or the next choice is space, in which case we will use the
// null anyway.
static bool NullIsBest(const NetworkIO& output, float null_thr,
                       int null_char, int t) {
  if (output.f(t)[null_char] >= null_thr) return true;
  if (output.BestLabel(t, null_char, null_char, NULL) != UNICHAR_SPACE)
    return false;
  return output.f(t)[null_char] > output.f(t)[UNICHAR_SPACE];
}

// Converts the network output to a sequence of labels. Outputs labels, scores
// and start xcoords of each char, and each null_char_, with an additional
// final xcoord for the end of the output.
// The conversion method is determined by internal state.
void LSTMRecognizer::LabelsFromOutputs(const NetworkIO& outputs, float null_thr,
                                       GenericVector<int>* labels,
                                       GenericVector<int>* xcoords) {
  if (SimpleTextOutput()) {
    LabelsViaSimpleText(outputs, labels, xcoords);
  } else if (IsRecoding()) {
    LabelsViaReEncode(outputs, labels, xcoords);
  } else if (null_thr <= 0.0) {
    LabelsViaCTC(outputs, labels, xcoords);
  } else {
    LabelsViaThreshold(outputs, null_thr, labels, xcoords);
  }
}

// Converts the network output to a sequence of labels, using a threshold
// on the null_char_ to determine character boundaries. Outputs labels, scores
// and start xcoords of each char, and each null_char_, with an additional
// final xcoord for the end of the output.
// The label output is the one with the highest score in the interval between
// null_chars_.
void LSTMRecognizer::LabelsViaThreshold(const NetworkIO& output,
                                        float null_thr,
                                        GenericVector<int>* labels,
                                        GenericVector<int>* xcoords) {
  labels->truncate(0);
  xcoords->truncate(0);
  int width = output.Width();
  int t = 0;
  // Skip any initial non-char.
  while (t < width && NullIsBest(output, null_thr, null_char_, t)) {
    ++t;
  }
  while (t < width) {
    ASSERT_HOST(!std::isnan(output.f(t)[null_char_]));
    int label = output.BestLabel(t, null_char_, null_char_, NULL);
    int char_start = t++;
    while (t < width && !NullIsBest(output, null_thr, null_char_, t) &&
           label == output.BestLabel(t, null_char_, null_char_, NULL)) {
      ++t;
    }
    int char_end = t;
    labels->push_back(label);
    xcoords->push_back(char_start);
    // Find the end of the non-char, and compute its score.
    while (t < width && NullIsBest(output, null_thr, null_char_, t)) {
      ++t;
    }
    if (t > char_end) {
      labels->push_back(null_char_);
      xcoords->push_back(char_end);
    }
  }
  xcoords->push_back(width);
}

// Converts the network output to a sequence of labels, with scores and
// start x-coords of the character labels. Retains the null_char_ as the
// end x-coord, where already present, otherwise the start of the next
// character is the end.
// The number of labels, scores, and xcoords is always matched, except that
// there is always an additional xcoord for the last end position.
void LSTMRecognizer::LabelsViaCTC(const NetworkIO& output,
                                  GenericVector<int>* labels,
                                  GenericVector<int>* xcoords) {
  labels->truncate(0);
  xcoords->truncate(0);
  int width = output.Width();
  int t = 0;
  while (t < width) {
    float score = 0.0f;
    int label = output.BestLabel(t, &score);
    labels->push_back(label);
    xcoords->push_back(t);
    while (++t < width && output.BestLabel(t, NULL) == label) {
    }
  }
  xcoords->push_back(width);
}

// As LabelsViaCTC except that this function constructs the best path that
// contains only legal sequences of subcodes for CJK.
void LSTMRecognizer::LabelsViaReEncode(const NetworkIO& output,
                                       GenericVector<int>* labels,
                                       GenericVector<int>* xcoords) {
  if (search_ == NULL) {
    search_ =
        new RecodeBeamSearch(recoder_, null_char_, SimpleTextOutput(), dict_);
  }
  search_->Decode(output, 1.0, 0.0, RecodeBeamSearch::kMinCertainty, NULL);
  search_->ExtractBestPathAsLabels(labels, xcoords);
}

// Converts the network output to a sequence of labels, with scores, using
// the simple character model (each position is a char, and the null_char_ is
// mainly intended for tail padding.)
void LSTMRecognizer::LabelsViaSimpleText(const NetworkIO& output,
                                         GenericVector<int>* labels,
                                         GenericVector<int>* xcoords) {
  labels->truncate(0);
  xcoords->truncate(0);
  int width = output.Width();
  for (int t = 0; t < width; ++t) {
    float score = 0.0f;
    int label = output.BestLabel(t, &score);
    if (label != null_char_) {
      labels->push_back(label);
      xcoords->push_back(t);
    }
  }
  xcoords->push_back(width);
}

// Helper returns a BLOB_CHOICE_LIST for the choices in a given x-range.
// Handles either LSTM labels or direct unichar-ids.
// Score ratio determines the worst ratio between top choice and remainder.
// If target_unicharset is not NULL, attempts to translate to the target
// unicharset, returning NULL on failure.
BLOB_CHOICE_LIST* LSTMRecognizer::GetBlobChoices(
    int col, int row, bool debug, const NetworkIO& output,
    const UNICHARSET* target_unicharset, int x_start, int x_end,
    float score_ratio) {
  float rating = 0.0f, certainty = 0.0f;
  int label = output.BestChoiceOverRange(x_start, x_end, UNICHAR_SPACE,
                                         null_char_, &rating, &certainty);
  int unichar_id = label == null_char_ ? UNICHAR_SPACE : label;
  if (debug) {
    tprintf("Best choice over range %d,%d=unichar%d=%s r = %g, cert=%g\n",
            x_start, x_end, unichar_id, DecodeSingleLabel(label), rating,
            certainty);
  }
  BLOB_CHOICE_LIST* choices = new BLOB_CHOICE_LIST;
  BLOB_CHOICE_IT bc_it(choices);
  if (!AddBlobChoices(unichar_id, rating, certainty, col, row,
                      target_unicharset, &bc_it)) {
    delete choices;
    return NULL;
  }
  // Get the other choices.
  double best_cert = certainty;
  for (int c = 0; c < output.NumFeatures(); ++c) {
    if (c == label || c == UNICHAR_SPACE || c == null_char_) continue;
    // Compute the score over the range.
    output.ScoresOverRange(x_start, x_end, c, null_char_, &rating, &certainty);
    int unichar_id = c == null_char_ ? UNICHAR_SPACE : c;
    if (certainty >= best_cert - score_ratio &&
        !AddBlobChoices(unichar_id, rating, certainty, col, row,
                        target_unicharset, &bc_it)) {
      delete choices;
      return NULL;
    }
  }
  choices->sort(&BLOB_CHOICE::SortByRating);
  if (bc_it.length() > kMaxChoices) {
    bc_it.move_to_first();
    for (int i = 0; i < kMaxChoices; ++i)
      bc_it.forward();
    while (!bc_it.at_first()) {
      delete bc_it.extract();
      bc_it.forward();
    }
  }
  return choices;
}

// Adds to the given iterator, the blob choices for the target_unicharset
// that correspond to the given LSTM unichar_id.
// Returns false if unicharset translation failed.
bool LSTMRecognizer::AddBlobChoices(int unichar_id, float rating,
                                    float certainty, int col, int row,
                                    const UNICHARSET* target_unicharset,
                                    BLOB_CHOICE_IT* bc_it) {
  int target_id = unichar_id;
  if (target_unicharset != NULL) {
    const char* utf8 = GetUnicharset().id_to_unichar(unichar_id);
    if (target_unicharset->contains_unichar(utf8)) {
      target_id = target_unicharset->unichar_to_id(utf8);
    } else {
      return false;
    }
  }
  BLOB_CHOICE* choice = new BLOB_CHOICE(target_id, rating, certainty, -1, 1.0f,
                                        static_cast<float>(MAX_INT16), 0.0f,
                                        BCC_STATIC_CLASSIFIER);
  choice->set_matrix_cell(col, row);
  bc_it->add_after_then_move(choice);
  return true;
}

// Returns a string corresponding to the label starting at start. Sets *end
// to the next start and if non-null, *decoded to the unichar id.
const char* LSTMRecognizer::DecodeLabel(const GenericVector<int>& labels,
                                        int start, int* end, int* decoded) {
  *end = start + 1;
  if (IsRecoding()) {
    // Decode labels via recoder_.
    RecodedCharID code;
    if (labels[start] == null_char_) {
      if (decoded != NULL) {
        code.Set(0, null_char_);
        *decoded = recoder_.DecodeUnichar(code);
      }
      return "<null>";
    }
    int index = start;
    while (index < labels.size() &&
           code.length() < RecodedCharID::kMaxCodeLen) {
      code.Set(code.length(), labels[index++]);
      while (index < labels.size() && labels[index] == null_char_) ++index;
      int uni_id = recoder_.DecodeUnichar(code);
      // If the next label isn't a valid first code, then we need to continue
      // extending even if we have a valid uni_id from this prefix.
      if (uni_id != INVALID_UNICHAR_ID &&
          (index == labels.size() ||
           code.length() == RecodedCharID::kMaxCodeLen ||
           recoder_.IsValidFirstCode(labels[index]))) {
        *end = index;
        if (decoded != NULL) *decoded = uni_id;
        if (uni_id == UNICHAR_SPACE) return " ";
        return GetUnicharset().get_normed_unichar(uni_id);
      }
    }
    return "<Undecodable>";
  } else {
    if (decoded != NULL) *decoded = labels[start];
    if (labels[start] == null_char_) return "<null>";
    if (labels[start] == UNICHAR_SPACE) return " ";
    return GetUnicharset().get_normed_unichar(labels[start]);
  }
}

// Returns a string corresponding to a given single label id, falling back to
// a default of ".." for part of a multi-label unichar-id.
const char* LSTMRecognizer::DecodeSingleLabel(int label) {
  if (label == null_char_) return "<null>";
  if (IsRecoding()) {
    // Decode label via recoder_.
    RecodedCharID code;
    code.Set(0, label);
    label = recoder_.DecodeUnichar(code);
    if (label == INVALID_UNICHAR_ID) return "..";  // Part of a bigger code.
  }
  if (label == UNICHAR_SPACE) return " ";
  return GetUnicharset().get_normed_unichar(label);
}

}  // namespace tesseract.
