///////////////////////////////////////////////////////////////////////
// File:        lstmrecognizer.h
// Description: Top-level line recognizer class for LSTM-based networks.
// Author:      Ray Smith
// Created:     Thu May 02 08:57:06 PST 2013
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

#ifndef TESSERACT_LSTM_LSTMRECOGNIZER_H_
#define TESSERACT_LSTM_LSTMRECOGNIZER_H_

#include "ccutil.h"
#include "helpers.h"
#include "imagedata.h"
#include "matrix.h"
#include "network.h"
#include "networkscratch.h"
#include "recodebeam.h"
#include "series.h"
#include "strngs.h"
#include "unicharcompress.h"

class BLOB_CHOICE_IT;
struct Pix;
class ROW_RES;
class ScrollView;
class TBOX;
class WERD_RES;

namespace tesseract {

class Dict;
class ImageData;

// Enum indicating training mode control flags.
enum TrainingFlags {
  TF_INT_MODE = 1,
  TF_AUTO_HARDEN = 2,
  TF_ROUND_ROBIN_TRAINING = 16,
  TF_COMPRESS_UNICHARSET = 64,
};

// Top-level line recognizer class for LSTM-based networks.
// Note that a sub-class, LSTMTrainer is used for training.
class LSTMRecognizer {
 public:
  LSTMRecognizer();
  ~LSTMRecognizer();

  int NumOutputs() const {
    return network_->NumOutputs();
  }
  int training_iteration() const {
    return training_iteration_;
  }
  int sample_iteration() const {
    return sample_iteration_;
  }
  double learning_rate() const {
    return learning_rate_;
  }
  bool IsHardening() const {
    return (training_flags_ & TF_AUTO_HARDEN) != 0;
  }
  LossType OutputLossType() const {
    if (network_ == nullptr) return LT_NONE;
    StaticShape shape;
    shape = network_->OutputShape(shape);
    return shape.loss_type();
  }
  bool SimpleTextOutput() const { return OutputLossType() == LT_SOFTMAX; }
  bool IsIntMode() const { return (training_flags_ & TF_INT_MODE) != 0; }
  // True if recoder_ is active to re-encode text to a smaller space.
  bool IsRecoding() const {
    return (training_flags_ & TF_COMPRESS_UNICHARSET) != 0;
  }
  // Returns the cache strategy for the DocumentCache.
  CachingStrategy CacheStrategy() const {
    return training_flags_ & TF_ROUND_ROBIN_TRAINING ? CS_ROUND_ROBIN
                                                     : CS_SEQUENTIAL;
  }
  // Returns true if the network is a TensorFlow network.
  bool IsTensorFlow() const { return network_->type() == NT_TENSORFLOW; }
  // Returns a vector of layer ids that can be passed to other layer functions
  // to access a specific layer.
  GenericVector<STRING> EnumerateLayers() const {
    ASSERT_HOST(network_ != NULL && network_->type() == NT_SERIES);
    Series* series = reinterpret_cast<Series*>(network_);
    GenericVector<STRING> layers;
    series->EnumerateLayers(NULL, &layers);
    return layers;
  }
  // Returns a specific layer from its id (from EnumerateLayers).
  Network* GetLayer(const STRING& id) const {
    ASSERT_HOST(network_ != NULL && network_->type() == NT_SERIES);
    ASSERT_HOST(id.length() > 1 && id[0] == ':');
    Series* series = reinterpret_cast<Series*>(network_);
    return series->GetLayer(&id[1]);
  }
  // Returns the learning rate of the layer from its id.
  float GetLayerLearningRate(const STRING& id) const {
    ASSERT_HOST(network_ != NULL && network_->type() == NT_SERIES);
    if (network_->TestFlag(NF_LAYER_SPECIFIC_LR)) {
      ASSERT_HOST(id.length() > 1 && id[0] == ':');
      Series* series = reinterpret_cast<Series*>(network_);
      return series->LayerLearningRate(&id[1]);
    } else {
      return learning_rate_;
    }
  }
  // Multiplies the all the learning rate(s) by the given factor.
  void ScaleLearningRate(double factor) {
    ASSERT_HOST(network_ != NULL && network_->type() == NT_SERIES);
    learning_rate_ *= factor;
    if (network_->TestFlag(NF_LAYER_SPECIFIC_LR)) {
      GenericVector<STRING> layers = EnumerateLayers();
      for (int i = 0; i < layers.size(); ++i) {
        ScaleLayerLearningRate(layers[i], factor);
      }
    }
  }
  // Multiplies the learning rate of the layer with id, by the given factor.
  void ScaleLayerLearningRate(const STRING& id, double factor) {
    ASSERT_HOST(network_ != NULL && network_->type() == NT_SERIES);
    ASSERT_HOST(id.length() > 1 && id[0] == ':');
    Series* series = reinterpret_cast<Series*>(network_);
    series->ScaleLayerLearningRate(&id[1], factor);
  }

  // True if the network is using adagrad to train.
  bool IsUsingAdaGrad() const { return network_->TestFlag(NF_ADA_GRAD); }
  // Provides access to the UNICHARSET that this classifier works with.
  const UNICHARSET& GetUnicharset() const { return ccutil_.unicharset; }
  // Provides access to the Dict that this classifier works with.
  const Dict* GetDict() const { return dict_; }
  // Sets the sample iteration to the given value. The sample_iteration_
  // determines the seed for the random number generator. The training
  // iteration is incremented only by a successful training iteration.
  void SetIteration(int iteration) {
    sample_iteration_ = iteration;
  }
  // Accessors for textline image normalization.
  int NumInputs() const {
    return network_->NumInputs();
  }
  int null_char() const { return null_char_; }

  // Writes to the given file. Returns false in case of error.
  bool Serialize(TFile* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, TFile* fp);
  // Loads the dictionary if possible from the traineddata file.
  // Prints a warning message, and returns false but otherwise fails silently
  // and continues to work without it if loading fails.
  // Note that dictionary load is independent from DeSerialize, but dependent
  // on the unicharset matching. This enables training to deserialize a model
  // from checkpoint or restore without having to go back and reload the
  // dictionary.
  bool LoadDictionary(const char* data_file_name, const char* lang);

  // Recognizes the line image, contained within image_data, returning the
  // ratings matrix and matching box_word for each WERD_RES in the output.
  // If invert, tries inverted as well if the normal interpretation doesn't
  // produce a good enough result. If use_alternates, the ratings matrix is
  // filled with segmentation and classifier alternatives that may be searched
  // using the standard beam search, otherwise, just a diagonal and prebuilt
  // best_choice. The line_box is used for computing the box_word in the
  // output words. Score_ratio is used to determine the classifier alternates.
  // If one_word, then a single WERD_RES is formed, regardless of the spaces
  // found during recognition.
  // If not NULL, we attempt to translate the output to target_unicharset, but
  // do not guarantee success, due to mismatches. In that case the output words
  // are marked with our UNICHARSET, not the caller's.
  void RecognizeLine(const ImageData& image_data, bool invert, bool debug,
                     double worst_dict_cert, bool use_alternates,
                     const UNICHARSET* target_unicharset, const TBOX& line_box,
                     float score_ratio, bool one_word,
                     PointerVector<WERD_RES>* words);
  // Builds a set of tesseract-compatible WERD_RESs aligned to line_box,
  // corresponding to the network output in outputs, labels, label_coords.
  // one_word generates a single word output, that may include spaces inside.
  // use_alternates generates alternative BLOB_CHOICEs and segmentation paths,
  // with cut-offs determined by scale_factor.
  // If not NULL, we attempt to translate the output to target_unicharset, but
  // do not guarantee success, due to mismatches. In that case the output words
  // are marked with our UNICHARSET, not the caller's.
  void WordsFromOutputs(const NetworkIO& outputs,
                        const GenericVector<int>& labels,
                        const GenericVector<int> label_coords,
                        const TBOX& line_box, bool debug, bool use_alternates,
                        bool one_word, float score_ratio, float scale_factor,
                        const UNICHARSET* target_unicharset,
                        PointerVector<WERD_RES>* words);

  // Helper computes min and mean best results in the output.
  void OutputStats(const NetworkIO& outputs,
                   float* min_output, float* mean_output, float* sd);
  // Recognizes the image_data, returning the labels,
  // scores, and corresponding pairs of start, end x-coords in coords.
  // If label_threshold is positive, uses it for making the labels, otherwise
  // uses standard ctc. Returned in scale_factor is the reduction factor
  // between the image and the output coords, for computing bounding boxes.
  // If re_invert is true, the input is inverted back to its original
  // photometric interpretation if inversion is attempted but fails to
  // improve the results. This ensures that outputs contains the correct
  // forward outputs for the best photometric interpretation.
  // inputs is filled with the used inputs to the network, and if not null,
  // target boxes is filled with scaled truth boxes if present in image_data.
  bool RecognizeLine(const ImageData& image_data, bool invert, bool debug,
                     bool re_invert, float label_threshold, float* scale_factor,
                     NetworkIO* inputs, NetworkIO* outputs);
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
  WERD_RES* WordFromOutput(const TBOX& line_box, const NetworkIO& outputs,
                           int word_start, int word_end, float score_ratio,
                           float space_certainty, bool debug,
                           bool use_alternates,
                           const UNICHARSET* target_unicharset,
                           const GenericVector<int>& labels,
                           const GenericVector<int>& label_coords,
                           float scale_factor);
  // Sets up a word with the ratings matrix and fake blobs with boxes in the
  // right places.
  WERD_RES* InitializeWord(const TBOX& line_box, int word_start, int word_end,
                           float space_certainty, bool use_alternates,
                           const UNICHARSET* target_unicharset,
                           const GenericVector<int>& labels,
                           const GenericVector<int>& label_coords,
                           float scale_factor);

  // Converts an array of labels to utf-8, whether or not the labels are
  // augmented with character boundaries.
  STRING DecodeLabels(const GenericVector<int>& labels);

  // Displays the forward results in a window with the characters and
  // boundaries as determined by the labels and label_coords.
  void DisplayForward(const NetworkIO& inputs,
                      const GenericVector<int>& labels,
                      const GenericVector<int>& label_coords,
                      const char* window_name,
                      ScrollView** window);

 protected:
  // Sets the random seed from the sample_iteration_;
  void SetRandomSeed() {
    inT64 seed = static_cast<inT64>(sample_iteration_) * 0x10000001;
    randomizer_.set_seed(seed);
    randomizer_.IntRand();
  }

  // Displays the labels and cuts at the corresponding xcoords.
  // Size of labels should match xcoords.
  void DisplayLSTMOutput(const GenericVector<int>& labels,
                         const GenericVector<int>& xcoords,
                         int height, ScrollView* window);

  // Prints debug output detailing the activation path that is implied by the
  // xcoords.
  void DebugActivationPath(const NetworkIO& outputs,
                           const GenericVector<int>& labels,
                           const GenericVector<int>& xcoords);

  // Prints debug output detailing activations and 2nd choice over a range
  // of positions.
  void DebugActivationRange(const NetworkIO& outputs, const char* label,
                            int best_choice, int x_start, int x_end);

  // Converts the network output to a sequence of labels. Outputs labels, scores
  // and start xcoords of each char, and each null_char_, with an additional
  // final xcoord for the end of the output.
  // The conversion method is determined by internal state.
  void LabelsFromOutputs(const NetworkIO& outputs, float null_thr,
                         GenericVector<int>* labels,
                         GenericVector<int>* xcoords);
  // Converts the network output to a sequence of labels, using a threshold
  // on the null_char_ to determine character boundaries. Outputs labels, scores
  // and start xcoords of each char, and each null_char_, with an additional
  // final xcoord for the end of the output.
  // The label output is the one with the highest score in the interval between
  // null_chars_.
  void LabelsViaThreshold(const NetworkIO& output,
                          float null_threshold,
                          GenericVector<int>* labels,
                          GenericVector<int>* xcoords);
  // Converts the network output to a sequence of labels, with scores and
  // start x-coords of the character labels. Retains the null_char_ character as
  // the end x-coord, where already present, otherwise the start of the next
  // character is the end.
  // The number of labels, scores, and xcoords is always matched, except that
  // there is always an additional xcoord for the last end position.
  void LabelsViaCTC(const NetworkIO& output,
                    GenericVector<int>* labels,
                    GenericVector<int>* xcoords);
  // As LabelsViaCTC except that this function constructs the best path that
  // contains only legal sequences of subcodes for recoder_.
  void LabelsViaReEncode(const NetworkIO& output, GenericVector<int>* labels,
                         GenericVector<int>* xcoords);
  // Converts the network output to a sequence of labels, with scores, using
  // the simple character model (each position is a char, and the null_char_ is
  // mainly intended for tail padding.)
  void LabelsViaSimpleText(const NetworkIO& output,
                           GenericVector<int>* labels,
                           GenericVector<int>* xcoords);

  // Helper returns a BLOB_CHOICE_LIST for the choices in a given x-range.
  // Handles either LSTM labels or direct unichar-ids.
  // Score ratio determines the worst ratio between top choice and remainder.
  // If target_unicharset is not NULL, attempts to translate to the target
  // unicharset, returning NULL on failure.
  BLOB_CHOICE_LIST* GetBlobChoices(int col, int row, bool debug,
                                   const NetworkIO& output,
                                   const UNICHARSET* target_unicharset,
                                   int x_start, int x_end, float score_ratio);

  // Adds to the given iterator, the blob choices for the target_unicharset
  // that correspond to the given LSTM unichar_id.
  // Returns false if unicharset translation failed.
  bool AddBlobChoices(int unichar_id, float rating, float certainty, int col,
                      int row, const UNICHARSET* target_unicharset,
                      BLOB_CHOICE_IT* bc_it);

  // Returns a string corresponding to the label starting at start. Sets *end
  // to the next start and if non-null, *decoded to the unichar id.
  const char* DecodeLabel(const GenericVector<int>& labels, int start, int* end,
                          int* decoded);

  // Returns a string corresponding to a given single label id, falling back to
  // a default of ".." for part of a multi-label unichar-id.
  const char* DecodeSingleLabel(int label);

 protected:
  // The network hierarchy.
  Network* network_;
  // The unicharset. Only the unicharset element is serialized.
  // Has to be a CCUtil, so Dict can point to it.
  CCUtil ccutil_;
  // For backward compatibility, recoder_ is serialized iff
  // training_flags_ & TF_COMPRESS_UNICHARSET.
  // Further encode/decode ccutil_.unicharset's ids to simplify the unicharset.
  UnicharCompress recoder_;

  // ==Training parameters that are serialized to provide a record of them.==
  STRING network_str_;
  // Flags used to determine the training method of the network.
  // See enum TrainingFlags above.
  inT32 training_flags_;
  // Number of actual backward training steps used.
  inT32 training_iteration_;
  // Index into training sample set. sample_iteration >= training_iteration_.
  inT32 sample_iteration_;
  // Index in softmax of null character. May take the value UNICHAR_BROKEN or
  // ccutil_.unicharset.size().
  inT32 null_char_;
  // Range used for the initial random numbers in the weights.
  float weight_range_;
  // Learning rate and momentum multipliers of deltas in backprop.
  float learning_rate_;
  float momentum_;

  // === NOT SERIALIZED.
  TRand randomizer_;
  NetworkScratch scratch_space_;
  // Language model (optional) to use with the beam search.
  Dict* dict_;
  // Beam search held between uses to optimize memory allocation/use.
  RecodeBeamSearch* search_;

  // == Debugging parameters.==
  // Recognition debug display window.
  ScrollView* debug_win_;
};

}  // namespace tesseract.

#endif  // TESSERACT_LSTM_LSTMRECOGNIZER_H_
