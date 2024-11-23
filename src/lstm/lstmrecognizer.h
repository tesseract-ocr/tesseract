///////////////////////////////////////////////////////////////////////
// File:        lstmrecognizer.h
// Description: Top-level line recognizer class for LSTM-based networks.
// Author:      Ray Smith
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
#include "matrix.h"
#include "network.h"
#include "networkscratch.h"
#include "params.h"
#include "recodebeam.h"
#include "series.h"
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
  TF_COMPRESS_UNICHARSET = 64,
};

// Top-level line recognizer class for LSTM-based networks.
// Note that a sub-class, LSTMTrainer is used for training.
class TESS_API LSTMRecognizer {
public:
  LSTMRecognizer();
  LSTMRecognizer(const std::string &language_data_path_prefix);
  ~LSTMRecognizer();

  int NumOutputs() const {
    return network_->NumOutputs();
  }

  // Return the training iterations.
  int training_iteration() const {
    return training_iteration_;
  }

  // Return the sample iterations.
  int sample_iteration() const {
    return sample_iteration_;
  }

  // Return the learning rate.
  float learning_rate() const {
    return learning_rate_;
  }

  LossType OutputLossType() const {
    if (network_ == nullptr) {
      return LT_NONE;
    }
    StaticShape shape;
    shape = network_->OutputShape(shape);
    return shape.loss_type();
  }
  bool SimpleTextOutput() const {
    return OutputLossType() == LT_SOFTMAX;
  }
  bool IsIntMode() const {
    return (training_flags_ & TF_INT_MODE) != 0;
  }
  // True if recoder_ is active to re-encode text to a smaller space.
  bool IsRecoding() const {
    return (training_flags_ & TF_COMPRESS_UNICHARSET) != 0;
  }
  // Returns true if the network is a TensorFlow network.
  bool IsTensorFlow() const {
    return network_->type() == NT_TENSORFLOW;
  }
  // Returns a vector of layer ids that can be passed to other layer functions
  // to access a specific layer.
  std::vector<std::string> EnumerateLayers() const {
    ASSERT_HOST(network_ != nullptr && network_->type() == NT_SERIES);
    auto *series = static_cast<Series *>(network_);
    std::vector<std::string> layers;
    series->EnumerateLayers(nullptr, layers);
    return layers;
  }
  // Returns a specific layer from its id (from EnumerateLayers).
  Network *GetLayer(const std::string &id) const {
    ASSERT_HOST(network_ != nullptr && network_->type() == NT_SERIES);
    ASSERT_HOST(id.length() > 1 && id[0] == ':');
    auto *series = static_cast<Series *>(network_);
    return series->GetLayer(&id[1]);
  }
  // Returns the learning rate of the layer from its id.
  float GetLayerLearningRate(const std::string &id) const {
    ASSERT_HOST(network_ != nullptr && network_->type() == NT_SERIES);
    if (network_->TestFlag(NF_LAYER_SPECIFIC_LR)) {
      ASSERT_HOST(id.length() > 1 && id[0] == ':');
      auto *series = static_cast<Series *>(network_);
      return series->LayerLearningRate(&id[1]);
    } else {
      return learning_rate_;
    }
  }

  // Return the network string.
  const char *GetNetwork() const {
    return network_str_.c_str();
  }

  // Return the adam beta.
  float GetAdamBeta() const {
    return adam_beta_;
  }

  // Return the momentum.
  float GetMomentum() const {
    return momentum_;
  }

  // Multiplies the all the learning rate(s) by the given factor.
  void ScaleLearningRate(double factor) {
    ASSERT_HOST(network_ != nullptr && network_->type() == NT_SERIES);
    learning_rate_ *= factor;
    if (network_->TestFlag(NF_LAYER_SPECIFIC_LR)) {
      std::vector<std::string> layers = EnumerateLayers();
      for (auto &layer : layers) {
        ScaleLayerLearningRate(layer, factor);
      }
    }
  }
  // Multiplies the learning rate of the layer with id, by the given factor.
  void ScaleLayerLearningRate(const std::string &id, double factor) {
    ASSERT_HOST(network_ != nullptr && network_->type() == NT_SERIES);
    ASSERT_HOST(id.length() > 1 && id[0] == ':');
    auto *series = static_cast<Series *>(network_);
    series->ScaleLayerLearningRate(&id[1], factor);
  }

  // Set the all the learning rate(s) to the given value.
  void SetLearningRate(float learning_rate)
  {
    ASSERT_HOST(network_ != nullptr && network_->type() == NT_SERIES);
    learning_rate_ = learning_rate;
    if (network_->TestFlag(NF_LAYER_SPECIFIC_LR)) {
      for (auto &id : EnumerateLayers()) {
        SetLayerLearningRate(id, learning_rate);
      }
    }
  }
  // Set the learning rate of the layer with id, by the given value.
  void SetLayerLearningRate(const std::string &id, float learning_rate)
  {
    ASSERT_HOST(network_ != nullptr && network_->type() == NT_SERIES);
    ASSERT_HOST(id.length() > 1 && id[0] == ':');
    auto *series = static_cast<Series *>(network_);
    series->SetLayerLearningRate(&id[1], learning_rate);
  }

  // Converts the network to int if not already.
  void ConvertToInt() {
    if ((training_flags_ & TF_INT_MODE) == 0) {
      network_->ConvertToInt();
      training_flags_ |= TF_INT_MODE;
    }
  }

  // Provides access to the UNICHARSET that this classifier works with.
  const UNICHARSET &GetUnicharset() const {
    return ccutil_.unicharset;
  }
  UNICHARSET &GetUnicharset() {
    return ccutil_.unicharset;
  }
  // Provides access to the UnicharCompress that this classifier works with.
  const UnicharCompress &GetRecoder() const {
    return recoder_;
  }
  // Provides access to the Dict that this classifier works with.
  const Dict *GetDict() const {
    return dict_;
  }
  Dict *GetDict() {
    return dict_;
  }
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

  // Return the null char index.
  int null_char() const {
    return null_char_;
  }

  // Loads a model from mgr, including the dictionary only if lang is not null.
  bool Load(const ParamsVectors *params, const std::string &lang, TessdataManager *mgr);

  // Writes to the given file. Returns false in case of error.
  // If mgr contains a unicharset and recoder, then they are not encoded to fp.
  bool Serialize(const TessdataManager *mgr, TFile *fp) const;
  // Reads from the given file. Returns false in case of error.
  // If mgr contains a unicharset and recoder, then they are taken from there,
  // otherwise, they are part of the serialization in fp.
  bool DeSerialize(const TessdataManager *mgr, TFile *fp);
  // Loads the charsets from mgr.
  bool LoadCharsets(const TessdataManager *mgr);
  // Loads the Recoder.
  bool LoadRecoder(TFile *fp);
  // Loads the dictionary if possible from the traineddata file.
  // Prints a warning message, and returns false but otherwise fails silently
  // and continues to work without it if loading fails.
  // Note that dictionary load is independent from DeSerialize, but dependent
  // on the unicharset matching. This enables training to deserialize a model
  // from checkpoint or restore without having to go back and reload the
  // dictionary.
  bool LoadDictionary(const ParamsVectors *params, const std::string &lang, TessdataManager *mgr);

  // Recognizes the line image, contained within image_data, returning the
  // recognized tesseract WERD_RES for the words.
  // If invert_threshold > 0, tries inverted as well if the normal
  // interpretation doesn't produce a result which at least reaches
  // that threshold. The line_box is used for computing the
  // box_word in the output words. worst_dict_cert is the worst certainty that
  // will be used in a dictionary word.
  void RecognizeLine(const ImageData &image_data, float invert_threshold, bool debug, double worst_dict_cert,
                     const TBOX &line_box, PointerVector<WERD_RES> *words, int lstm_choice_mode = 0,
                     int lstm_choice_amount = 5);

  // Helper computes min and mean best results in the output.
  void OutputStats(const NetworkIO &outputs, float *min_output, float *mean_output, float *sd);
  // Recognizes the image_data, returning the labels,
  // scores, and corresponding pairs of start, end x-coords in coords.
  // Returned in scale_factor is the reduction factor
  // between the image and the output coords, for computing bounding boxes.
  // If re_invert is true, the input is inverted back to its original
  // photometric interpretation if inversion is attempted but fails to
  // improve the results. This ensures that outputs contains the correct
  // forward outputs for the best photometric interpretation.
  // inputs is filled with the used inputs to the network.
  bool RecognizeLine(const ImageData &image_data, float invert_threshold, bool debug, bool re_invert,
                     bool upside_down, float *scale_factor, NetworkIO *inputs, NetworkIO *outputs);

  // Converts an array of labels to utf-8, whether or not the labels are
  // augmented with character boundaries.
  std::string DecodeLabels(const std::vector<int> &labels);

  // Displays the forward results in a window with the characters and
  // boundaries as determined by the labels and label_coords.
  void DisplayForward(const NetworkIO &inputs, const std::vector<int> &labels,
                      const std::vector<int> &label_coords, const char *window_name,
                      ScrollView **window);
  // Converts the network output to a sequence of labels. Outputs labels, scores
  // and start xcoords of each char, and each null_char_, with an additional
  // final xcoord for the end of the output.
  // The conversion method is determined by internal state.
  void LabelsFromOutputs(const NetworkIO &outputs, std::vector<int> *labels,
                         std::vector<int> *xcoords);

protected:
  // Sets the random seed from the sample_iteration_;
  void SetRandomSeed() {
    int64_t seed = sample_iteration_ * 0x10000001LL;
    randomizer_.set_seed(seed);
    randomizer_.IntRand();
  }

  // Displays the labels and cuts at the corresponding xcoords.
  // Size of labels should match xcoords.
  void DisplayLSTMOutput(const std::vector<int> &labels, const std::vector<int> &xcoords,
                         int height, ScrollView *window);

  // Prints debug output detailing the activation path that is implied by the
  // xcoords.
  void DebugActivationPath(const NetworkIO &outputs, const std::vector<int> &labels,
                           const std::vector<int> &xcoords);

  // Prints debug output detailing activations and 2nd choice over a range
  // of positions.
  void DebugActivationRange(const NetworkIO &outputs, const char *label, int best_choice,
                            int x_start, int x_end);

  // As LabelsViaCTC except that this function constructs the best path that
  // contains only legal sequences of subcodes for recoder_.
  void LabelsViaReEncode(const NetworkIO &output, std::vector<int> *labels,
                         std::vector<int> *xcoords);
  // Converts the network output to a sequence of labels, with scores, using
  // the simple character model (each position is a char, and the null_char_ is
  // mainly intended for tail padding.)
  void LabelsViaSimpleText(const NetworkIO &output, std::vector<int> *labels,
                           std::vector<int> *xcoords);

  // Returns a string corresponding to the label starting at start. Sets *end
  // to the next start and if non-null, *decoded to the unichar id.
  const char *DecodeLabel(const std::vector<int> &labels, unsigned start, unsigned *end, int *decoded);

  // Returns a string corresponding to a given single label id, falling back to
  // a default of ".." for part of a multi-label unichar-id.
  const char *DecodeSingleLabel(int label);

protected:
  // The network hierarchy.
  Network *network_;
  // The unicharset. Only the unicharset element is serialized.
  // Has to be a CCUtil, so Dict can point to it.
  CCUtil ccutil_;
  // For backward compatibility, recoder_ is serialized iff
  // training_flags_ & TF_COMPRESS_UNICHARSET.
  // Further encode/decode ccutil_.unicharset's ids to simplify the unicharset.
  UnicharCompress recoder_;

  // ==Training parameters that are serialized to provide a record of them.==
  std::string network_str_;
  // Flags used to determine the training method of the network.
  // See enum TrainingFlags above.
  int32_t training_flags_;
  // Number of actual backward training steps used.
  int32_t training_iteration_;
  // Index into training sample set. sample_iteration >= training_iteration_.
  int32_t sample_iteration_;
  // Index in softmax of null character. May take the value UNICHAR_BROKEN or
  // ccutil_.unicharset.size().
  int32_t null_char_;
  // Learning rate and momentum multipliers of deltas in backprop.
  float learning_rate_;
  float momentum_;
  // Smoothing factor for 2nd moment of gradients.
  float adam_beta_;

  // === NOT SERIALIZED.
  TRand randomizer_;
  NetworkScratch scratch_space_;
  // Language model (optional) to use with the beam search.
  Dict *dict_;
  // Beam search held between uses to optimize memory allocation/use.
  RecodeBeamSearch *search_;

  // == Debugging parameters.==
  // Recognition debug display window.
  ScrollView *debug_win_;
};

} // namespace tesseract.

#endif // TESSERACT_LSTM_LSTMRECOGNIZER_H_
