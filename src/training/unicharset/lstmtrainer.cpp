///////////////////////////////////////////////////////////////////////
// File:        lstmtrainer.cpp
// Description: Top-level line trainer class for LSTM-based networks.
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

#define _USE_MATH_DEFINES // needed to get definition of M_SQRT1_2

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include <cmath>
#include <iomanip>             // for std::setprecision
#include <locale>              // for std::locale::classic
#include <string>
#include "lstmtrainer.h"

#include <allheaders.h>
#include "boxread.h"
#include "ctc.h"
#include "imagedata.h"
#include "input.h"
#include "networkbuilder.h"
#include "ratngs.h"
#include "recodebeam.h"
#ifdef INCLUDE_TENSORFLOW
#  include "tfnetwork.h"
#endif
#include "tprintf.h"

namespace tesseract {

// Min actual error rate increase to constitute divergence.
const double kMinDivergenceRate = 50.0;
// Min iterations since last best before acting on a stall.
const int kMinStallIterations = 10000;
// Fraction of current char error rate that sub_trainer_ has to be ahead
// before we declare the sub_trainer_ a success and switch to it.
const double kSubTrainerMarginFraction = 3.0 / 128;
// Factor to reduce learning rate on divergence.
const double kLearningRateDecay = M_SQRT1_2;
// LR adjustment iterations.
const int kNumAdjustmentIterations = 100;
// How often to add data to the error_graph_.
const int kErrorGraphInterval = 1000;
// Number of training images to train between calls to MaintainCheckpoints.
const int kNumPagesPerBatch = 100;
// Min percent error rate to consider start-up phase over.
const int kMinStartedErrorRate = 75;
// Error rate at which to transition to stage 1.
const double kStageTransitionThreshold = 10.0;
// Confidence beyond which the truth is more likely wrong than the recognizer.
const double kHighConfidence = 0.9375; // 15/16.
// Fraction of weight sign-changing total to constitute a definite improvement.
const double kImprovementFraction = 15.0 / 16.0;
// Fraction of last written best to make it worth writing another.
const double kBestCheckpointFraction = 31.0 / 32.0;
#ifndef GRAPHICS_DISABLED
// Scale factor for display of target activations of CTC.
const int kTargetXScale = 5;
const int kTargetYScale = 100;
#endif // !GRAPHICS_DISABLED

LSTMTrainer::LSTMTrainer()
    : randomly_rotate_(false), training_data_(0), sub_trainer_(nullptr) {
  EmptyConstructor();
  debug_interval_ = 0;
}

LSTMTrainer::LSTMTrainer(const char *model_base, const char *checkpoint_name,
                         int debug_interval, int64_t max_memory)
    : randomly_rotate_(false),
      training_data_(max_memory),
      sub_trainer_(nullptr) {
  EmptyConstructor();
  debug_interval_ = debug_interval;
  model_base_ = model_base;
  checkpoint_name_ = checkpoint_name;
}

LSTMTrainer::~LSTMTrainer() {
#ifndef GRAPHICS_DISABLED
  delete align_win_;
  delete target_win_;
  delete ctc_win_;
  delete recon_win_;
#endif
}

// Tries to deserialize a trainer from the given file and silently returns
// false in case of failure.
bool LSTMTrainer::TryLoadingCheckpoint(const char *filename,
                                       const char *old_traineddata) {
  std::vector<char> data;
  if (!LoadDataFromFile(filename, &data)) {
    return false;
  }
  tprintf("Loaded file %s, unpacking...\n", filename);
  if (!ReadTrainingDump(data, *this)) {
    return false;
  }
  if (IsIntMode()) {
    tprintf("Error, %s is an integer (fast) model, cannot continue training\n",
            filename);
    return false;
  }
  if (((old_traineddata == nullptr || *old_traineddata == '\0') &&
       network_->NumOutputs() == recoder_.code_range()) ||
      filename == old_traineddata) {
    return true; // Normal checkpoint load complete.
  }
  tprintf("Code range changed from %d to %d!\n", network_->NumOutputs(),
          recoder_.code_range());
  if (old_traineddata == nullptr || *old_traineddata == '\0') {
    tprintf("Must supply the old traineddata for code conversion!\n");
    return false;
  }
  TessdataManager old_mgr;
  ASSERT_HOST(old_mgr.Init(old_traineddata));
  TFile fp;
  if (!old_mgr.GetComponent(TESSDATA_LSTM_UNICHARSET, &fp)) {
    return false;
  }
  UNICHARSET old_chset;
  if (!old_chset.load_from_file(&fp, false)) {
    return false;
  }
  if (!old_mgr.GetComponent(TESSDATA_LSTM_RECODER, &fp)) {
    return false;
  }
  UnicharCompress old_recoder;
  if (!old_recoder.DeSerialize(&fp)) {
    return false;
  }
  std::vector<int> code_map = MapRecoder(old_chset, old_recoder);
  // Set the null_char_ to the new value.
  int old_null_char = null_char_;
  SetNullChar();
  // Map the softmax(s) in the network.
  network_->RemapOutputs(old_recoder.code_range(), code_map);
  tprintf("Previous null char=%d mapped to %d\n", old_null_char, null_char_);
  return true;
}

// Initializes the trainer with a network_spec in the network description
// net_flags control network behavior according to the NetworkFlags enum.
// There isn't really much difference between them - only where the effects
// are implemented.
// For other args see NetworkBuilder::InitNetwork.
// Note: Be sure to call InitCharSet before InitNetwork!
bool LSTMTrainer::InitNetwork(const char *network_spec, int append_index,
                              int net_flags, float weight_range,
                              float learning_rate, float momentum,
                              float adam_beta) {
  mgr_.SetVersionString(mgr_.VersionString() + ":" + network_spec);
  adam_beta_ = adam_beta;
  learning_rate_ = learning_rate;
  momentum_ = momentum;
  SetNullChar();
  if (!NetworkBuilder::InitNetwork(recoder_.code_range(), network_spec,
                                   append_index, net_flags, weight_range,
                                   &randomizer_, &network_)) {
    return false;
  }
  network_str_ += network_spec;
  tprintf("Built network:%s from request %s\n", network_->spec().c_str(),
          network_spec);
  tprintf(
      "Training parameters:\n  Debug interval = %d,"
      " weights = %g, learning rate = %g, momentum=%g\n",
      debug_interval_, weight_range, learning_rate_, momentum_);
  tprintf("null char=%d\n", null_char_);
  return true;
}

// Initializes a trainer from a serialized TFNetworkModel proto.
// Returns the global step of TensorFlow graph or 0 if failed.
#ifdef INCLUDE_TENSORFLOW
int LSTMTrainer::InitTensorFlowNetwork(const std::string &tf_proto) {
  delete network_;
  TFNetwork *tf_net = new TFNetwork("TensorFlow");
  training_iteration_ = tf_net->InitFromProtoStr(tf_proto);
  if (training_iteration_ == 0) {
    tprintf("InitFromProtoStr failed!!\n");
    return 0;
  }
  network_ = tf_net;
  ASSERT_HOST(recoder_.code_range() == tf_net->num_classes());
  return training_iteration_;
}
#endif

// Resets all the iteration counters for fine tuning or traininng a head,
// where we want the error reporting to reset.
void LSTMTrainer::InitIterations() {
  sample_iteration_ = 0;
  training_iteration_ = 0;
  learning_iteration_ = 0;
  prev_sample_iteration_ = 0;
  best_error_rate_ = 100.0;
  best_iteration_ = 0;
  worst_error_rate_ = 0.0;
  worst_iteration_ = 0;
  stall_iteration_ = kMinStallIterations;
  best_error_history_.clear();
  best_error_iterations_.clear();
  improvement_steps_ = kMinStallIterations;
  perfect_delay_ = 0;
  last_perfect_training_iteration_ = 0;
  for (int i = 0; i < ET_COUNT; ++i) {
    best_error_rates_[i] = 100.0;
    worst_error_rates_[i] = 0.0;
    error_buffers_[i].clear();
    error_buffers_[i].resize(kRollingBufferSize_);
    error_rates_[i] = 100.0;
  }
  error_rate_of_last_saved_best_ = kMinStartedErrorRate;
}

// If the training sample is usable, grid searches for the optimal
// dict_ratio/cert_offset, and returns the results in a string of space-
// separated triplets of ratio,offset=worderr.
Trainability LSTMTrainer::GridSearchDictParams(
    const ImageData *trainingdata, int iteration, double min_dict_ratio,
    double dict_ratio_step, double max_dict_ratio, double min_cert_offset,
    double cert_offset_step, double max_cert_offset, std::string &results) {
  sample_iteration_ = iteration;
  NetworkIO fwd_outputs, targets;
  Trainability result =
      PrepareForBackward(trainingdata, &fwd_outputs, &targets);
  if (result == UNENCODABLE || result == HI_PRECISION_ERR || dict_ == nullptr) {
    return result;
  }

  // Encode/decode the truth to get the normalization.
  std::vector<int> truth_labels, ocr_labels, xcoords;
  ASSERT_HOST(EncodeString(trainingdata->transcription(), &truth_labels));
  // NO-dict error.
  RecodeBeamSearch base_search(recoder_, null_char_, SimpleTextOutput(),
                               nullptr);
  base_search.Decode(fwd_outputs, 1.0, 0.0, RecodeBeamSearch::kMinCertainty,
                     nullptr);
  base_search.ExtractBestPathAsLabels(&ocr_labels, &xcoords);
  std::string truth_text = DecodeLabels(truth_labels);
  std::string ocr_text = DecodeLabels(ocr_labels);
  double baseline_error = ComputeWordError(&truth_text, &ocr_text);
  results += "0,0=" + std::to_string(baseline_error);

  RecodeBeamSearch search(recoder_, null_char_, SimpleTextOutput(), dict_);
  for (double r = min_dict_ratio; r < max_dict_ratio; r += dict_ratio_step) {
    for (double c = min_cert_offset; c < max_cert_offset;
         c += cert_offset_step) {
      search.Decode(fwd_outputs, r, c, RecodeBeamSearch::kMinCertainty,
                    nullptr);
      search.ExtractBestPathAsLabels(&ocr_labels, &xcoords);
      truth_text = DecodeLabels(truth_labels);
      ocr_text = DecodeLabels(ocr_labels);
      // This is destructive on both strings.
      double word_error = ComputeWordError(&truth_text, &ocr_text);
      if ((r == min_dict_ratio && c == min_cert_offset) ||
          !std::isfinite(word_error)) {
        std::string t = DecodeLabels(truth_labels);
        std::string o = DecodeLabels(ocr_labels);
        tprintf("r=%g, c=%g, truth=%s, ocr=%s, wderr=%g, truth[0]=%d\n", r, c,
                t.c_str(), o.c_str(), word_error, truth_labels[0]);
      }
      results += " " + std::to_string(r);
      results += "," + std::to_string(c);
      results += "=" + std::to_string(word_error);
    }
  }
  return result;
}

// Provides output on the distribution of weight values.
void LSTMTrainer::DebugNetwork() {
  network_->DebugWeights();
}

// Loads a set of lstmf files that were created using the lstm.train config to
// tesseract into memory ready for training. Returns false if nothing was
// loaded.
bool LSTMTrainer::LoadAllTrainingData(const std::vector<std::string> &filenames,
                                      CachingStrategy cache_strategy,
                                      bool randomly_rotate) {
  randomly_rotate_ = randomly_rotate;
  training_data_.Clear();
  return training_data_.LoadDocuments(filenames, cache_strategy,
                                      LoadDataFromFile);
}

// Keeps track of best and locally worst char error_rate and launches tests
// using tester, when a new min or max is reached.
// Writes checkpoints at appropriate times and builds and returns a log message
// to indicate progress. Returns false if nothing interesting happened.
bool LSTMTrainer::MaintainCheckpoints(const TestCallback &tester,
                                      std::stringstream &log_msg) {
  PrepareLogMsg(log_msg);
  double error_rate = CharError();
  int iteration = learning_iteration();
  if (iteration >= stall_iteration_ &&
      error_rate > best_error_rate_ * (1.0 + kSubTrainerMarginFraction) &&
      best_error_rate_ < kMinStartedErrorRate && !best_trainer_.empty()) {
    // It hasn't got any better in a long while, and is a margin worse than the
    // best, so go back to the best model and try a different learning rate.
    StartSubtrainer(log_msg);
  }
  SubTrainerResult sub_trainer_result = STR_NONE;
  if (sub_trainer_ != nullptr) {
    sub_trainer_result = UpdateSubtrainer(log_msg);
    if (sub_trainer_result == STR_REPLACED) {
      // Reset the inputs, as we have overwritten *this.
      error_rate = CharError();
      iteration = learning_iteration();
      PrepareLogMsg(log_msg);
    }
  }
  bool result = true; // Something interesting happened.
  std::vector<char> rec_model_data;
  if (error_rate < best_error_rate_) {
    SaveRecognitionDump(&rec_model_data);
    log_msg << " New best BCER = " << error_rate;
    log_msg << UpdateErrorGraph(iteration, error_rate, rec_model_data, tester);
    // If sub_trainer_ is not nullptr, either *this beat it to a new best, or it
    // just overwrote *this. In either case, we have finished with it.
    sub_trainer_.reset();
    stall_iteration_ = learning_iteration() + kMinStallIterations;
    if (TransitionTrainingStage(kStageTransitionThreshold)) {
      log_msg << " Transitioned to stage " << CurrentTrainingStage();
    }
    SaveTrainingDump(NO_BEST_TRAINER, *this, &best_trainer_);
    if (error_rate < error_rate_of_last_saved_best_ * kBestCheckpointFraction) {
      std::string best_model_name = DumpFilename();
      if (!SaveDataToFile(best_trainer_, best_model_name.c_str())) {
        log_msg << " failed to write best model:";
      } else {
        log_msg << " wrote best model:";
        error_rate_of_last_saved_best_ = best_error_rate_;
      }
      log_msg << best_model_name;
    }
  } else if (error_rate > worst_error_rate_) {
    SaveRecognitionDump(&rec_model_data);
    log_msg << " New worst BCER = " << error_rate;
    log_msg << UpdateErrorGraph(iteration, error_rate, rec_model_data, tester);
    if (worst_error_rate_ > best_error_rate_ + kMinDivergenceRate &&
        best_error_rate_ < kMinStartedErrorRate && !best_trainer_.empty()) {
      // Error rate has ballooned. Go back to the best model.
      log_msg << "\nDivergence! ";
      // Copy best_trainer_ before reading it, as it will get overwritten.
      std::vector<char> revert_data(best_trainer_);
      if (ReadTrainingDump(revert_data, *this)) {
        LogIterations("Reverted to", log_msg);
        ReduceLearningRates(this, log_msg);
      } else {
        LogIterations("Failed to Revert at", log_msg);
      }
      // If it fails again, we will wait twice as long before reverting again.
      stall_iteration_ = iteration + 2 * (iteration - learning_iteration());
      // Re-save the best trainer with the new learning rates and stall
      // iteration.
      SaveTrainingDump(NO_BEST_TRAINER, *this, &best_trainer_);
    }
  } else {
    // Something interesting happened only if the sub_trainer_ was trained.
    result = sub_trainer_result != STR_NONE;
  }
  if (checkpoint_name_.length() > 0) {
    // Write a current checkpoint.
    std::vector<char> checkpoint;
    if (!SaveTrainingDump(FULL, *this, &checkpoint) ||
        !SaveDataToFile(checkpoint, checkpoint_name_.c_str())) {
      log_msg << " failed to write checkpoint.";
    } else {
      log_msg << " wrote checkpoint.";
    }
  }
  return result;
}

// Builds a string containing a progress message with current error rates.
void LSTMTrainer::PrepareLogMsg(std::stringstream &log_msg) const {
  LogIterations("At", log_msg);
  log_msg << std::fixed << std::setprecision(3)
          << ", mean rms=" << error_rates_[ET_RMS]
          << "%, delta=" << error_rates_[ET_DELTA]
          << "%, BCER train=" << error_rates_[ET_CHAR_ERROR]
          << "%, BWER train=" << error_rates_[ET_WORD_RECERR]
          << "%, skip ratio=" << error_rates_[ET_SKIP_RATIO] << "%,";
}

// Appends <intro_str> iteration learning_iteration()/training_iteration()/
// sample_iteration() to the log_msg.
void LSTMTrainer::LogIterations(const char *intro_str,
                                std::stringstream &log_msg) const {
  log_msg << intro_str
          << " iteration " << learning_iteration()
          << "/" << training_iteration()
          << "/" << sample_iteration();
}

// Returns true and increments the training_stage_ if the error rate has just
// passed through the given threshold for the first time.
bool LSTMTrainer::TransitionTrainingStage(float error_threshold) {
  if (best_error_rate_ < error_threshold &&
      training_stage_ + 1 < num_training_stages_) {
    ++training_stage_;
    return true;
  }
  return false;
}

// Writes to the given file. Returns false in case of error.
bool LSTMTrainer::Serialize(SerializeAmount serialize_amount,
                            const TessdataManager *mgr, TFile *fp) const {
  if (!LSTMRecognizer::Serialize(mgr, fp)) {
    return false;
  }
  if (!fp->Serialize(&learning_iteration_)) {
    return false;
  }
  if (!fp->Serialize(&prev_sample_iteration_)) {
    return false;
  }
  if (!fp->Serialize(&perfect_delay_)) {
    return false;
  }
  if (!fp->Serialize(&last_perfect_training_iteration_)) {
    return false;
  }
  for (const auto &error_buffer : error_buffers_) {
    if (!fp->Serialize(error_buffer)) {
      return false;
    }
  }
  if (!fp->Serialize(&error_rates_[0], countof(error_rates_))) {
    return false;
  }
  if (!fp->Serialize(&training_stage_)) {
    return false;
  }
  uint8_t amount = serialize_amount;
  if (!fp->Serialize(&amount)) {
    return false;
  }
  if (serialize_amount == LIGHT) {
    return true; // We are done.
  }
  if (!fp->Serialize(&best_error_rate_)) {
    return false;
  }
  if (!fp->Serialize(&best_error_rates_[0], countof(best_error_rates_))) {
    return false;
  }
  if (!fp->Serialize(&best_iteration_)) {
    return false;
  }
  if (!fp->Serialize(&worst_error_rate_)) {
    return false;
  }
  if (!fp->Serialize(&worst_error_rates_[0], countof(worst_error_rates_))) {
    return false;
  }
  if (!fp->Serialize(&worst_iteration_)) {
    return false;
  }
  if (!fp->Serialize(&stall_iteration_)) {
    return false;
  }
  if (!fp->Serialize(best_model_data_)) {
    return false;
  }
  if (!fp->Serialize(worst_model_data_)) {
    return false;
  }
  if (serialize_amount != NO_BEST_TRAINER && !fp->Serialize(best_trainer_)) {
    return false;
  }
  std::vector<char> sub_data;
  if (sub_trainer_ != nullptr &&
      !SaveTrainingDump(LIGHT, *sub_trainer_, &sub_data)) {
    return false;
  }
  if (!fp->Serialize(sub_data)) {
    return false;
  }
  if (!fp->Serialize(best_error_history_)) {
    return false;
  }
  if (!fp->Serialize(best_error_iterations_)) {
    return false;
  }
  return fp->Serialize(&improvement_steps_);
}

// Reads from the given file. Returns false in case of error.
// NOTE: It is assumed that the trainer is never read cross-endian.
bool LSTMTrainer::DeSerialize(const TessdataManager *mgr, TFile *fp) {
  if (!LSTMRecognizer::DeSerialize(mgr, fp)) {
    return false;
  }
  if (!fp->DeSerialize(&learning_iteration_)) {
    // Special case. If we successfully decoded the recognizer, but fail here
    // then it means we were just given a recognizer, so issue a warning and
    // allow it.
    tprintf("Warning: LSTMTrainer deserialized an LSTMRecognizer!\n");
    learning_iteration_ = 0;
    network_->SetEnableTraining(TS_ENABLED);
    return true;
  }
  if (!fp->DeSerialize(&prev_sample_iteration_)) {
    return false;
  }
  if (!fp->DeSerialize(&perfect_delay_)) {
    return false;
  }
  if (!fp->DeSerialize(&last_perfect_training_iteration_)) {
    return false;
  }
  for (auto &error_buffer : error_buffers_) {
    if (!fp->DeSerialize(error_buffer)) {
      return false;
    }
  }
  if (!fp->DeSerialize(&error_rates_[0], countof(error_rates_))) {
    return false;
  }
  if (!fp->DeSerialize(&training_stage_)) {
    return false;
  }
  uint8_t amount;
  if (!fp->DeSerialize(&amount)) {
    return false;
  }
  if (amount == LIGHT) {
    return true; // Don't read the rest.
  }
  if (!fp->DeSerialize(&best_error_rate_)) {
    return false;
  }
  if (!fp->DeSerialize(&best_error_rates_[0], countof(best_error_rates_))) {
    return false;
  }
  if (!fp->DeSerialize(&best_iteration_)) {
    return false;
  }
  if (!fp->DeSerialize(&worst_error_rate_)) {
    return false;
  }
  if (!fp->DeSerialize(&worst_error_rates_[0], countof(worst_error_rates_))) {
    return false;
  }
  if (!fp->DeSerialize(&worst_iteration_)) {
    return false;
  }
  if (!fp->DeSerialize(&stall_iteration_)) {
    return false;
  }
  if (!fp->DeSerialize(best_model_data_)) {
    return false;
  }
  if (!fp->DeSerialize(worst_model_data_)) {
    return false;
  }
  if (amount != NO_BEST_TRAINER && !fp->DeSerialize(best_trainer_)) {
    return false;
  }
  std::vector<char> sub_data;
  if (!fp->DeSerialize(sub_data)) {
    return false;
  }
  if (sub_data.empty()) {
    sub_trainer_ = nullptr;
  } else {
    sub_trainer_ = std::make_unique<LSTMTrainer>();
    if (!ReadTrainingDump(sub_data, *sub_trainer_)) {
      return false;
    }
  }
  if (!fp->DeSerialize(best_error_history_)) {
    return false;
  }
  if (!fp->DeSerialize(best_error_iterations_)) {
    return false;
  }
  return fp->DeSerialize(&improvement_steps_);
}

// De-serializes the saved best_trainer_ into sub_trainer_, and adjusts the
// learning rates (by scaling reduction, or layer specific, according to
// NF_LAYER_SPECIFIC_LR).
void LSTMTrainer::StartSubtrainer(std::stringstream &log_msg) {
  sub_trainer_ = std::make_unique<LSTMTrainer>();
  if (!ReadTrainingDump(best_trainer_, *sub_trainer_)) {
    log_msg << " Failed to revert to previous best for trial!";
    sub_trainer_.reset();
  } else {
    log_msg << " Trial sub_trainer_ from iteration "
            << sub_trainer_->training_iteration();
    // Reduce learning rate so it doesn't diverge this time.
    sub_trainer_->ReduceLearningRates(this, log_msg);
    // If it fails again, we will wait twice as long before reverting again.
    int stall_offset =
        learning_iteration() - sub_trainer_->learning_iteration();
    stall_iteration_ = learning_iteration() + 2 * stall_offset;
    sub_trainer_->stall_iteration_ = stall_iteration_;
    // Re-save the best trainer with the new learning rates and stall iteration.
    SaveTrainingDump(NO_BEST_TRAINER, *sub_trainer_, &best_trainer_);
  }
}

// While the sub_trainer_ is behind the current training iteration and its
// training error is at least kSubTrainerMarginFraction better than the
// current training error, trains the sub_trainer_, and returns STR_UPDATED if
// it did anything. If it catches up, and has a better error rate than the
// current best, as well as a margin over the current error rate, then the
// trainer in *this is replaced with sub_trainer_, and STR_REPLACED is
// returned. STR_NONE is returned if the subtrainer wasn't good enough to
// receive any training iterations.
SubTrainerResult LSTMTrainer::UpdateSubtrainer(std::stringstream &log_msg) {
  double training_error = CharError();
  double sub_error = sub_trainer_->CharError();
  double sub_margin = (training_error - sub_error) / sub_error;
  if (sub_margin >= kSubTrainerMarginFraction) {
    log_msg << " sub_trainer=" << sub_error
            << " margin=" << 100.0 * sub_margin << "\n";
    // Catch up to current iteration.
    int end_iteration = training_iteration();
    while (sub_trainer_->training_iteration() < end_iteration &&
           sub_margin >= kSubTrainerMarginFraction) {
      int target_iteration =
          sub_trainer_->training_iteration() + kNumPagesPerBatch;
      while (sub_trainer_->training_iteration() < target_iteration) {
        sub_trainer_->TrainOnLine(this, false);
      }
      std::stringstream batch_log("Sub:");
      batch_log.imbue(std::locale::classic());
      sub_trainer_->PrepareLogMsg(batch_log);
      batch_log << "\n";
      tprintf("UpdateSubtrainer:%s", batch_log.str().c_str());
      log_msg << batch_log.str();
      sub_error = sub_trainer_->CharError();
      sub_margin = (training_error - sub_error) / sub_error;
    }
    if (sub_error < best_error_rate_ &&
        sub_margin >= kSubTrainerMarginFraction) {
      // The sub_trainer_ has won the race to a new best. Switch to it.
      std::vector<char> updated_trainer;
      SaveTrainingDump(LIGHT, *sub_trainer_, &updated_trainer);
      ReadTrainingDump(updated_trainer, *this);
      log_msg << " Sub trainer wins at iteration "
              << training_iteration() << "\n";
      return STR_REPLACED;
    }
    return STR_UPDATED;
  }
  return STR_NONE;
}

// Reduces network learning rates, either for everything, or for layers
// independently, according to NF_LAYER_SPECIFIC_LR.
void LSTMTrainer::ReduceLearningRates(LSTMTrainer *samples_trainer,
                                      std::stringstream &log_msg) {
  if (network_->TestFlag(NF_LAYER_SPECIFIC_LR)) {
    int num_reduced = ReduceLayerLearningRates(
        kLearningRateDecay, kNumAdjustmentIterations, samples_trainer);
    log_msg << "\nReduced learning rate on layers: " << num_reduced;
  } else {
    ScaleLearningRate(kLearningRateDecay);
    log_msg << "\nReduced learning rate to :" << learning_rate_;
  }
  log_msg << "\n";
}

// Considers reducing the learning rate independently for each layer down by
// factor(<1), or leaving it the same, by double-training the given number of
// samples and minimizing the amount of changing of sign of weight updates.
// Even if it looks like all weights should remain the same, an adjustment
// will be made to guarantee a different result when reverting to an old best.
// Returns the number of layer learning rates that were reduced.
int LSTMTrainer::ReduceLayerLearningRates(TFloat factor, int num_samples,
                                          LSTMTrainer *samples_trainer) {
  enum WhichWay {
    LR_DOWN, // Learning rate will go down by factor.
    LR_SAME, // Learning rate will stay the same.
    LR_COUNT // Size of arrays.
  };
  std::vector<std::string> layers = EnumerateLayers();
  int num_layers = layers.size();
  std::vector<int> num_weights(num_layers);
  std::vector<TFloat> bad_sums[LR_COUNT];
  std::vector<TFloat> ok_sums[LR_COUNT];
  for (int i = 0; i < LR_COUNT; ++i) {
    bad_sums[i].resize(num_layers, 0.0);
    ok_sums[i].resize(num_layers, 0.0);
  }
  auto momentum_factor = 1 / (1 - momentum_);
  std::vector<char> orig_trainer;
  samples_trainer->SaveTrainingDump(LIGHT, *this, &orig_trainer);
  for (int i = 0; i < num_layers; ++i) {
    Network *layer = GetLayer(layers[i]);
    num_weights[i] = layer->IsTraining() ? layer->num_weights() : 0;
  }
  int iteration = sample_iteration();
  for (int s = 0; s < num_samples; ++s) {
    // Which way will we modify the learning rate?
    for (int ww = 0; ww < LR_COUNT; ++ww) {
      // Transfer momentum to learning rate and adjust by the ww factor.
      auto ww_factor = momentum_factor;
      if (ww == LR_DOWN) {
        ww_factor *= factor;
      }
      // Make a copy of *this, so we can mess about without damaging anything.
      LSTMTrainer copy_trainer;
      samples_trainer->ReadTrainingDump(orig_trainer, copy_trainer);
      // Clear the updates, doing nothing else.
      copy_trainer.network_->Update(0.0, 0.0, 0.0, 0);
      // Adjust the learning rate in each layer.
      for (int i = 0; i < num_layers; ++i) {
        if (num_weights[i] == 0) {
          continue;
        }
        copy_trainer.ScaleLayerLearningRate(layers[i], ww_factor);
      }
      copy_trainer.SetIteration(iteration);
      // Train on the sample, but keep the update in updates_ instead of
      // applying to the weights.
      const ImageData *trainingdata =
          copy_trainer.TrainOnLine(samples_trainer, true);
      if (trainingdata == nullptr) {
        continue;
      }
      // We'll now use this trainer again for each layer.
      std::vector<char> updated_trainer;
      samples_trainer->SaveTrainingDump(LIGHT, copy_trainer, &updated_trainer);
      for (int i = 0; i < num_layers; ++i) {
        if (num_weights[i] == 0) {
          continue;
        }
        LSTMTrainer layer_trainer;
        samples_trainer->ReadTrainingDump(updated_trainer, layer_trainer);
        Network *layer = layer_trainer.GetLayer(layers[i]);
        // Update the weights in just the layer, using Adam if enabled.
        layer->Update(0.0, momentum_, adam_beta_,
                      layer_trainer.training_iteration_ + 1);
        // Zero the updates matrix again.
        layer->Update(0.0, 0.0, 0.0, 0);
        // Train again on the same sample, again holding back the updates.
        layer_trainer.TrainOnLine(trainingdata, true);
        // Count the sign changes in the updates in layer vs in copy_trainer.
        float before_bad = bad_sums[ww][i];
        float before_ok = ok_sums[ww][i];
        layer->CountAlternators(*copy_trainer.GetLayer(layers[i]),
                                &ok_sums[ww][i], &bad_sums[ww][i]);
        float bad_frac =
            bad_sums[ww][i] + ok_sums[ww][i] - before_bad - before_ok;
        if (bad_frac > 0.0f) {
          bad_frac = (bad_sums[ww][i] - before_bad) / bad_frac;
        }
      }
    }
    ++iteration;
  }
  int num_lowered = 0;
  for (int i = 0; i < num_layers; ++i) {
    if (num_weights[i] == 0) {
      continue;
    }
    Network *layer = GetLayer(layers[i]);
    float lr = GetLayerLearningRate(layers[i]);
    TFloat total_down = bad_sums[LR_DOWN][i] + ok_sums[LR_DOWN][i];
    TFloat total_same = bad_sums[LR_SAME][i] + ok_sums[LR_SAME][i];
    TFloat frac_down = bad_sums[LR_DOWN][i] / total_down;
    TFloat frac_same = bad_sums[LR_SAME][i] / total_same;
    tprintf("Layer %d=%s: lr %g->%g%%, lr %g->%g%%", i, layer->name().c_str(),
            lr * factor, 100.0 * frac_down, lr, 100.0 * frac_same);
    if (frac_down < frac_same * kImprovementFraction) {
      tprintf(" REDUCED\n");
      ScaleLayerLearningRate(layers[i], factor);
      ++num_lowered;
    } else {
      tprintf(" SAME\n");
    }
  }
  if (num_lowered == 0) {
    // Just lower everything to make sure.
    for (int i = 0; i < num_layers; ++i) {
      if (num_weights[i] > 0) {
        ScaleLayerLearningRate(layers[i], factor);
        ++num_lowered;
      }
    }
  }
  return num_lowered;
}

// Converts the string to integer class labels, with appropriate null_char_s
// in between if not in SimpleTextOutput mode. Returns false on failure.
/* static */
bool LSTMTrainer::EncodeString(const std::string &str,
                               const UNICHARSET &unicharset,
                               const UnicharCompress *recoder, bool simple_text,
                               int null_char, std::vector<int> *labels) {
  if (str.c_str() == nullptr || str.length() <= 0) {
    tprintf("Empty truth string!\n");
    return false;
  }
  unsigned err_index;
  std::vector<int> internal_labels;
  labels->clear();
  if (!simple_text) {
    labels->push_back(null_char);
  }
  std::string cleaned = unicharset.CleanupString(str.c_str());
  if (unicharset.encode_string(cleaned.c_str(), true, &internal_labels, nullptr,
                               &err_index)) {
    bool success = true;
    for (auto internal_label : internal_labels) {
      if (recoder != nullptr) {
        // Re-encode labels via recoder.
        RecodedCharID code;
        int len = recoder->EncodeUnichar(internal_label, &code);
        if (len > 0) {
          for (int j = 0; j < len; ++j) {
            labels->push_back(code(j));
            if (!simple_text) {
              labels->push_back(null_char);
            }
          }
        } else {
          success = false;
          err_index = 0;
          break;
        }
      } else {
        labels->push_back(internal_label);
        if (!simple_text) {
          labels->push_back(null_char);
        }
      }
    }
    if (success) {
      return true;
    }
  }
  tprintf("Encoding of string failed! Failure bytes:");
  while (err_index < cleaned.size()) {
    tprintf(" %x", cleaned[err_index++] & 0xff);
  }
  tprintf("\n");
  return false;
}

// Performs forward-backward on the given trainingdata.
// Returns a Trainability enum to indicate the suitability of the sample.
Trainability LSTMTrainer::TrainOnLine(const ImageData *trainingdata,
                                      bool batch) {
  NetworkIO fwd_outputs, targets;
  Trainability trainable =
      PrepareForBackward(trainingdata, &fwd_outputs, &targets);
  ++sample_iteration_;
  if (trainable == UNENCODABLE || trainable == NOT_BOXED) {
    return trainable; // Sample was unusable.
  }
  bool debug =
      debug_interval_ > 0 && training_iteration() % debug_interval_ == 0;
  // Run backprop on the output.
  NetworkIO bp_deltas;
  if (network_->IsTraining() &&
      (trainable != PERFECT ||
       training_iteration() >
           last_perfect_training_iteration_ + perfect_delay_)) {
    network_->Backward(debug, targets, &scratch_space_, &bp_deltas);
    network_->Update(learning_rate_, batch ? -1.0f : momentum_, adam_beta_,
                     training_iteration_ + 1);
  }
#ifndef GRAPHICS_DISABLED
  if (debug_interval_ == 1 && debug_win_ != nullptr) {
    debug_win_->AwaitEvent(SVET_CLICK);
  }
#endif // !GRAPHICS_DISABLED
  // Roll the memory of past means.
  RollErrorBuffers();
  return trainable;
}

// Prepares the ground truth, runs forward, and prepares the targets.
// Returns a Trainability enum to indicate the suitability of the sample.
Trainability LSTMTrainer::PrepareForBackward(const ImageData *trainingdata,
                                             NetworkIO *fwd_outputs,
                                             NetworkIO *targets) {
  if (trainingdata == nullptr) {
    tprintf("Null trainingdata.\n");
    return UNENCODABLE;
  }
  // Ensure repeatability of random elements even across checkpoints.
  bool debug =
      debug_interval_ > 0 && training_iteration() % debug_interval_ == 0;
  std::vector<int> truth_labels;
  if (!EncodeString(trainingdata->transcription(), &truth_labels)) {
    tprintf("Can't encode transcription: '%s' in language '%s'\n",
            trainingdata->transcription().c_str(),
            trainingdata->language().c_str());
    return UNENCODABLE;
  }
  bool upside_down = false;
  if (randomly_rotate_) {
    // This ensures consistent training results.
    SetRandomSeed();
    upside_down = randomizer_.SignedRand(1.0) > 0.0;
    if (upside_down) {
      // Modify the truth labels to match the rotation:
      // Apart from space and null, increment the label. This changes the
      // script-id to the same script-id but upside-down.
      // The labels need to be reversed in order, as the first is now the last.
      for (auto truth_label : truth_labels) {
        if (truth_label != UNICHAR_SPACE && truth_label != null_char_) {
          ++truth_label;
        }
      }
      std::reverse(truth_labels.begin(), truth_labels.end());
    }
  }
  unsigned w = 0;
  while (w < truth_labels.size() &&
         (truth_labels[w] == UNICHAR_SPACE || truth_labels[w] == null_char_)) {
    ++w;
  }
  if (w == truth_labels.size()) {
    tprintf("Blank transcription: %s\n", trainingdata->transcription().c_str());
    return UNENCODABLE;
  }
  float image_scale;
  NetworkIO inputs;
  bool invert = trainingdata->boxes().empty();
  if (!RecognizeLine(*trainingdata, invert ? 0.5f : 0.0f, debug, invert, upside_down,
                     &image_scale, &inputs, fwd_outputs)) {
    tprintf("Image %s not trainable\n", trainingdata->imagefilename().c_str());
    return UNENCODABLE;
  }
  targets->Resize(*fwd_outputs, network_->NumOutputs());
  LossType loss_type = OutputLossType();
  if (loss_type == LT_SOFTMAX) {
    if (!ComputeTextTargets(*fwd_outputs, truth_labels, targets)) {
      tprintf("Compute simple targets failed for %s!\n",
              trainingdata->imagefilename().c_str());
      return UNENCODABLE;
    }
  } else if (loss_type == LT_CTC) {
    if (!ComputeCTCTargets(truth_labels, fwd_outputs, targets)) {
      tprintf("Compute CTC targets failed for %s!\n",
              trainingdata->imagefilename().c_str());
      return UNENCODABLE;
    }
  } else {
    tprintf("Logistic outputs not implemented yet!\n");
    return UNENCODABLE;
  }
  std::vector<int> ocr_labels;
  std::vector<int> xcoords;
  LabelsFromOutputs(*fwd_outputs, &ocr_labels, &xcoords);
  // CTC does not produce correct target labels to begin with.
  if (loss_type != LT_CTC) {
    LabelsFromOutputs(*targets, &truth_labels, &xcoords);
  }
  if (!DebugLSTMTraining(inputs, *trainingdata, *fwd_outputs, truth_labels,
                         *targets)) {
    tprintf("Input width was %d\n", inputs.Width());
    return UNENCODABLE;
  }
  std::string ocr_text = DecodeLabels(ocr_labels);
  std::string truth_text = DecodeLabels(truth_labels);
  targets->SubtractAllFromFloat(*fwd_outputs);
  if (debug_interval_ != 0) {
    if (truth_text != ocr_text) {
      tprintf("Iteration %d: BEST OCR TEXT : %s\n", training_iteration(),
              ocr_text.c_str());
    }
  }
  double char_error = ComputeCharError(truth_labels, ocr_labels);
  double word_error = ComputeWordError(&truth_text, &ocr_text);
  double delta_error = ComputeErrorRates(*targets, char_error, word_error);
  if (debug_interval_ != 0) {
    tprintf("File %s line %d %s:\n", trainingdata->imagefilename().c_str(),
            trainingdata->page_number(), delta_error == 0.0 ? "(Perfect)" : "");
  }
  if (delta_error == 0.0) {
    return PERFECT;
  }
  if (targets->AnySuspiciousTruth(kHighConfidence)) {
    return HI_PRECISION_ERR;
  }
  return TRAINABLE;
}

// Writes the trainer to memory, so that the current training state can be
// restored.  *this must always be the master trainer that retains the only
// copy of the training data and language model. trainer is the model that is
// actually serialized.
bool LSTMTrainer::SaveTrainingDump(SerializeAmount serialize_amount,
                                   const LSTMTrainer &trainer,
                                   std::vector<char> *data) const {
  TFile fp;
  fp.OpenWrite(data);
  return trainer.Serialize(serialize_amount, &mgr_, &fp);
}

// Restores the model to *this.
bool LSTMTrainer::ReadLocalTrainingDump(const TessdataManager *mgr,
                                        const char *data, int size) {
  if (size == 0) {
    tprintf("Warning: data size is 0 in LSTMTrainer::ReadLocalTrainingDump\n");
    return false;
  }
  TFile fp;
  fp.Open(data, size);
  return DeSerialize(mgr, &fp);
}

// Writes the full recognition traineddata to the given filename.
bool LSTMTrainer::SaveTraineddata(const char *filename) {
  std::vector<char> recognizer_data;
  SaveRecognitionDump(&recognizer_data);
  mgr_.OverwriteEntry(TESSDATA_LSTM, &recognizer_data[0],
                      recognizer_data.size());
  return mgr_.SaveFile(filename, SaveDataToFile);
}

// Writes the recognizer to memory, so that it can be used for testing later.
void LSTMTrainer::SaveRecognitionDump(std::vector<char> *data) const {
  TFile fp;
  fp.OpenWrite(data);
  network_->SetEnableTraining(TS_TEMP_DISABLE);
  ASSERT_HOST(LSTMRecognizer::Serialize(&mgr_, &fp));
  network_->SetEnableTraining(TS_RE_ENABLE);
}

// Returns a suitable filename for a training dump, based on the model_base_,
// best_error_rate_, best_iteration_ and training_iteration_.
std::string LSTMTrainer::DumpFilename() const {
  std::stringstream filename;
  filename.imbue(std::locale::classic());
  filename << model_base_ << std::fixed << std::setprecision(3)
           << "_" << best_error_rate_
           << "_" << best_iteration_
           << "_" << training_iteration_
           << ".checkpoint";
  return filename.str();
}

// Fills the whole error buffer of the given type with the given value.
void LSTMTrainer::FillErrorBuffer(double new_error, ErrorTypes type) {
  for (int i = 0; i < kRollingBufferSize_; ++i) {
    error_buffers_[type][i] = new_error;
  }
  error_rates_[type] = 100.0 * new_error;
}

// Helper generates a map from each current recoder_ code (ie softmax index)
// to the corresponding old_recoder code, or -1 if there isn't one.
std::vector<int> LSTMTrainer::MapRecoder(
    const UNICHARSET &old_chset, const UnicharCompress &old_recoder) const {
  int num_new_codes = recoder_.code_range();
  int num_new_unichars = GetUnicharset().size();
  std::vector<int> code_map(num_new_codes, -1);
  for (int c = 0; c < num_new_codes; ++c) {
    int old_code = -1;
    // Find all new unichar_ids that recode to something that includes c.
    // The <= is to include the null char, which may be beyond the unicharset.
    for (int uid = 0; uid <= num_new_unichars; ++uid) {
      RecodedCharID codes;
      int length = recoder_.EncodeUnichar(uid, &codes);
      int code_index = 0;
      while (code_index < length && codes(code_index) != c) {
        ++code_index;
      }
      if (code_index == length) {
        continue;
      }
      // The old unicharset must have the same unichar.
      int old_uid =
          uid < num_new_unichars
              ? old_chset.unichar_to_id(GetUnicharset().id_to_unichar(uid))
              : old_chset.size() - 1;
      if (old_uid == INVALID_UNICHAR_ID) {
        continue;
      }
      // The encoding of old_uid at the same code_index is the old code.
      RecodedCharID old_codes;
      if (code_index < old_recoder.EncodeUnichar(old_uid, &old_codes)) {
        old_code = old_codes(code_index);
        break;
      }
    }
    code_map[c] = old_code;
  }
  return code_map;
}

// Private version of InitCharSet above finishes the job after initializing
// the mgr_ data member.
void LSTMTrainer::InitCharSet() {
  EmptyConstructor();
  training_flags_ = TF_COMPRESS_UNICHARSET;
  // Initialize the unicharset and recoder.
  if (!LoadCharsets(&mgr_)) {
    ASSERT_HOST(
        "Must provide a traineddata containing lstm_unicharset and"
        " lstm_recoder!\n" != nullptr);
  }
  SetNullChar();
}

// Helper computes and sets the null_char_.
void LSTMTrainer::SetNullChar() {
  null_char_ = GetUnicharset().has_special_codes() ? UNICHAR_BROKEN
                                                   : GetUnicharset().size();
  RecodedCharID code;
  recoder_.EncodeUnichar(null_char_, &code);
  null_char_ = code(0);
}

// Factored sub-constructor sets up reasonable default values.
void LSTMTrainer::EmptyConstructor() {
#ifndef GRAPHICS_DISABLED
  align_win_ = nullptr;
  target_win_ = nullptr;
  ctc_win_ = nullptr;
  recon_win_ = nullptr;
#endif
  checkpoint_iteration_ = 0;
  training_stage_ = 0;
  num_training_stages_ = 2;
  InitIterations();
}

// Outputs the string and periodically displays the given network inputs
// as an image in the given window, and the corresponding labels at the
// corresponding x_starts.
// Returns false if the truth string is empty.
bool LSTMTrainer::DebugLSTMTraining(const NetworkIO &inputs,
                                    const ImageData &trainingdata,
                                    const NetworkIO &fwd_outputs,
                                    const std::vector<int> &truth_labels,
                                    const NetworkIO &outputs) {
  const std::string &truth_text = DecodeLabels(truth_labels);
  if (truth_text.c_str() == nullptr || truth_text.length() <= 0) {
    tprintf("Empty truth string at decode time!\n");
    return false;
  }
  if (debug_interval_ != 0) {
    // Get class labels, xcoords and string.
    std::vector<int> labels;
    std::vector<int> xcoords;
    LabelsFromOutputs(outputs, &labels, &xcoords);
    std::string text = DecodeLabels(labels);
    tprintf("Iteration %d: GROUND  TRUTH : %s\n", training_iteration(),
            truth_text.c_str());
    if (truth_text != text) {
      tprintf("Iteration %d: ALIGNED TRUTH : %s\n", training_iteration(),
              text.c_str());
    }
    if (debug_interval_ > 0 && training_iteration() % debug_interval_ == 0) {
      tprintf("TRAINING activation path for truth string %s\n",
              truth_text.c_str());
      DebugActivationPath(outputs, labels, xcoords);
#ifndef GRAPHICS_DISABLED
      DisplayForward(inputs, labels, xcoords, "LSTMTraining", &align_win_);
      if (OutputLossType() == LT_CTC) {
        DisplayTargets(fwd_outputs, "CTC Outputs", &ctc_win_);
        DisplayTargets(outputs, "CTC Targets", &target_win_);
      }
#endif
    }
  }
  return true;
}

#ifndef GRAPHICS_DISABLED

// Displays the network targets as line a line graph.
void LSTMTrainer::DisplayTargets(const NetworkIO &targets,
                                 const char *window_name, ScrollView **window) {
  int width = targets.Width();
  int num_features = targets.NumFeatures();
  Network::ClearWindow(true, window_name, width * kTargetXScale, kTargetYScale,
                       window);
  for (int c = 0; c < num_features; ++c) {
    int color = c % (ScrollView::GREEN_YELLOW - 1) + 2;
    (*window)->Pen(static_cast<ScrollView::Color>(color));
    int start_t = -1;
    for (int t = 0; t < width; ++t) {
      double target = targets.f(t)[c];
      target *= kTargetYScale;
      if (target >= 1) {
        if (start_t < 0) {
          (*window)->SetCursor(t - 1, 0);
          start_t = t;
        }
        (*window)->DrawTo(t, target);
      } else if (start_t >= 0) {
        (*window)->DrawTo(t, 0);
        (*window)->DrawTo(start_t - 1, 0);
        start_t = -1;
      }
    }
    if (start_t >= 0) {
      (*window)->DrawTo(width, 0);
      (*window)->DrawTo(start_t - 1, 0);
    }
  }
  (*window)->Update();
}

#endif // !GRAPHICS_DISABLED

// Builds a no-compromises target where the first positions should be the
// truth labels and the rest is padded with the null_char_.
bool LSTMTrainer::ComputeTextTargets(const NetworkIO &outputs,
                                     const std::vector<int> &truth_labels,
                                     NetworkIO *targets) {
  if (truth_labels.size() > targets->Width()) {
    tprintf("Error: transcription %s too long to fit into target of width %d\n",
            DecodeLabels(truth_labels).c_str(), targets->Width());
    return false;
  }
  int i = 0;
  for (auto truth_label : truth_labels) {
    targets->SetActivations(i, truth_label, 1.0);
    ++i;
  }
  for (i = truth_labels.size(); i < targets->Width(); ++i) {
    targets->SetActivations(i, null_char_, 1.0);
  }
  return true;
}

// Builds a target using standard CTC. truth_labels should be pre-padded with
// nulls wherever desired. They don't have to be between all labels.
// outputs is input-output, as it gets clipped to minimum probability.
bool LSTMTrainer::ComputeCTCTargets(const std::vector<int> &truth_labels,
                                    NetworkIO *outputs, NetworkIO *targets) {
  // Bottom-clip outputs to a minimum probability.
  CTC::NormalizeProbs(outputs);
  return CTC::ComputeCTCTargets(truth_labels, null_char_,
                                outputs->float_array(), targets);
}

// Computes network errors, and stores the results in the rolling buffers,
// along with the supplied text_error.
// Returns the delta error of the current sample (not running average.)
double LSTMTrainer::ComputeErrorRates(const NetworkIO &deltas,
                                      double char_error, double word_error) {
  UpdateErrorBuffer(ComputeRMSError(deltas), ET_RMS);
  // Delta error is the fraction of timesteps with >0.5 error in the top choice
  // score. If zero, then the top choice characters are guaranteed correct,
  // even when there is residue in the RMS error.
  double delta_error = ComputeWinnerError(deltas);
  UpdateErrorBuffer(delta_error, ET_DELTA);
  UpdateErrorBuffer(word_error, ET_WORD_RECERR);
  UpdateErrorBuffer(char_error, ET_CHAR_ERROR);
  // Skip ratio measures the difference between sample_iteration_ and
  // training_iteration_, which reflects the number of unusable samples,
  // usually due to unencodable truth text, or the text not fitting in the
  // space for the output.
  double skip_count = sample_iteration_ - prev_sample_iteration_;
  UpdateErrorBuffer(skip_count, ET_SKIP_RATIO);
  return delta_error;
}

// Computes the network activation RMS error rate.
double LSTMTrainer::ComputeRMSError(const NetworkIO &deltas) {
  double total_error = 0.0;
  int width = deltas.Width();
  int num_classes = deltas.NumFeatures();
  for (int t = 0; t < width; ++t) {
    const float *class_errs = deltas.f(t);
    for (int c = 0; c < num_classes; ++c) {
      double error = class_errs[c];
      total_error += error * error;
    }
  }
  return sqrt(total_error / (width * num_classes));
}

// Computes network activation winner error rate. (Number of values that are
// in error by >= 0.5 divided by number of time-steps.) More closely related
// to final character error than RMS, but still directly calculable from
// just the deltas. Because of the binary nature of the targets, zero winner
// error is a sufficient but not necessary condition for zero char error.
double LSTMTrainer::ComputeWinnerError(const NetworkIO &deltas) {
  int num_errors = 0;
  int width = deltas.Width();
  int num_classes = deltas.NumFeatures();
  for (int t = 0; t < width; ++t) {
    const float *class_errs = deltas.f(t);
    for (int c = 0; c < num_classes; ++c) {
      float abs_delta = std::fabs(class_errs[c]);
      // TODO(rays) Filtering cases where the delta is very large to cut out
      // GT errors doesn't work. Find a better way or get better truth.
      if (0.5 <= abs_delta) {
        ++num_errors;
      }
    }
  }
  return static_cast<double>(num_errors) / width;
}

// Computes a very simple bag of chars char error rate.
double LSTMTrainer::ComputeCharError(const std::vector<int> &truth_str,
                                     const std::vector<int> &ocr_str) {
  std::vector<int> label_counts(NumOutputs());
  unsigned truth_size = 0;
  for (auto ch : truth_str) {
    if (ch != null_char_) {
      ++label_counts[ch];
      ++truth_size;
    }
  }
  for (auto ch : ocr_str) {
    if (ch != null_char_) {
      --label_counts[ch];
    }
  }
  unsigned char_errors = 0;
  for (auto label_count : label_counts) {
    char_errors += abs(label_count);
  }
  // Limit BCER to interval [0,1] and avoid division by zero.
  if (truth_size <= char_errors) {
    return (char_errors == 0) ? 0.0 : 1.0;
  }
  return static_cast<double>(char_errors) / truth_size;
}

// Computes word recall error rate using a very simple bag of words algorithm.
// NOTE that this is destructive on both input strings.
double LSTMTrainer::ComputeWordError(std::string *truth_str,
                                     std::string *ocr_str) {
  using StrMap = std::unordered_map<std::string, int, std::hash<std::string>>;
  std::vector<std::string> truth_words = split(*truth_str, ' ');
  if (truth_words.empty()) {
    return 0.0;
  }
  std::vector<std::string> ocr_words = split(*ocr_str, ' ');
  StrMap word_counts;
  for (const auto &truth_word : truth_words) {
    std::string truth_word_string(truth_word.c_str());
    auto it = word_counts.find(truth_word_string);
    if (it == word_counts.end()) {
      word_counts.insert(std::make_pair(truth_word_string, 1));
    } else {
      ++it->second;
    }
  }
  for (const auto &ocr_word : ocr_words) {
    std::string ocr_word_string(ocr_word.c_str());
    auto it = word_counts.find(ocr_word_string);
    if (it == word_counts.end()) {
      word_counts.insert(std::make_pair(ocr_word_string, -1));
    } else {
      --it->second;
    }
  }
  int word_recall_errs = 0;
  for (const auto &word_count : word_counts) {
    if (word_count.second > 0) {
      word_recall_errs += word_count.second;
    }
  }
  return static_cast<double>(word_recall_errs) / truth_words.size();
}

// Updates the error buffer and corresponding mean of the given type with
// the new_error.
void LSTMTrainer::UpdateErrorBuffer(double new_error, ErrorTypes type) {
  int index = training_iteration_ % kRollingBufferSize_;
  error_buffers_[type][index] = new_error;
  // Compute the mean error.
  int mean_count =
      std::min<int>(training_iteration_ + 1, error_buffers_[type].size());
  double buffer_sum = 0.0;
  for (int i = 0; i < mean_count; ++i) {
    buffer_sum += error_buffers_[type][i];
  }
  double mean = buffer_sum / mean_count;
  // Trim precision to 1/1000 of 1%.
  error_rates_[type] = IntCastRounded(100000.0 * mean) / 1000.0;
}

// Rolls error buffers and reports the current means.
void LSTMTrainer::RollErrorBuffers() {
  prev_sample_iteration_ = sample_iteration_;
  if (NewSingleError(ET_DELTA) > 0.0) {
    ++learning_iteration_;
  } else {
    last_perfect_training_iteration_ = training_iteration_;
  }
  ++training_iteration_;
  if (debug_interval_ != 0) {
    tprintf("Mean rms=%g%%, delta=%g%%, train=%g%%(%g%%), skip ratio=%g%%\n",
            error_rates_[ET_RMS], error_rates_[ET_DELTA],
            error_rates_[ET_CHAR_ERROR], error_rates_[ET_WORD_RECERR],
            error_rates_[ET_SKIP_RATIO]);
  }
}

// Given that error_rate is either a new min or max, updates the best/worst
// error rates, and record of progress.
// Tester is an externally supplied callback function that tests on some
// data set with a given model and records the error rates in a graph.
std::string LSTMTrainer::UpdateErrorGraph(int iteration, double error_rate,
                                          const std::vector<char> &model_data,
                                          const TestCallback &tester) {
  if (error_rate > best_error_rate_ &&
      iteration < best_iteration_ + kErrorGraphInterval) {
    // Too soon to record a new point.
    if (tester != nullptr && !worst_model_data_.empty()) {
      mgr_.OverwriteEntry(TESSDATA_LSTM, &worst_model_data_[0],
                          worst_model_data_.size());
      return tester(worst_iteration_, nullptr, mgr_, CurrentTrainingStage());
    } else {
      return "";
    }
  }
  std::string result;
  // NOTE: there are 2 asymmetries here:
  // 1. We are computing the global minimum, but the local maximum in between.
  // 2. If the tester returns an empty string, indicating that it is busy,
  //    call it repeatedly on new local maxima to test the previous min, but
  //    not the other way around, as there is little point testing the maxima
  //    between very frequent minima.
  if (error_rate < best_error_rate_) {
    // This is a new (global) minimum.
    if (tester != nullptr && !worst_model_data_.empty()) {
      mgr_.OverwriteEntry(TESSDATA_LSTM, &worst_model_data_[0],
                          worst_model_data_.size());
      result = tester(worst_iteration_, worst_error_rates_, mgr_,
                      CurrentTrainingStage());
      worst_model_data_.clear();
      best_model_data_ = model_data;
    }
    best_error_rate_ = error_rate;
    memcpy(best_error_rates_, error_rates_, sizeof(error_rates_));
    best_iteration_ = iteration;
    best_error_history_.push_back(error_rate);
    best_error_iterations_.push_back(iteration);
    // Compute 2% decay time.
    double two_percent_more = error_rate + 2.0;
    int i;
    for (i = best_error_history_.size() - 1;
         i >= 0 && best_error_history_[i] < two_percent_more; --i) {
    }
    int old_iteration = i >= 0 ? best_error_iterations_[i] : 0;
    improvement_steps_ = iteration - old_iteration;
    tprintf("2 Percent improvement time=%d, best error was %g @ %d\n",
            improvement_steps_, i >= 0 ? best_error_history_[i] : 100.0,
            old_iteration);
  } else if (error_rate > best_error_rate_) {
    // This is a new (local) maximum.
    if (tester != nullptr) {
      if (!best_model_data_.empty()) {
        mgr_.OverwriteEntry(TESSDATA_LSTM, &best_model_data_[0],
                            best_model_data_.size());
        result = tester(best_iteration_, best_error_rates_, mgr_,
                        CurrentTrainingStage());
      } else if (!worst_model_data_.empty()) {
        // Allow for multiple data points with "worst" error rate.
        mgr_.OverwriteEntry(TESSDATA_LSTM, &worst_model_data_[0],
                            worst_model_data_.size());
        result = tester(worst_iteration_, worst_error_rates_, mgr_,
                        CurrentTrainingStage());
      }
      if (result.length() > 0) {
        best_model_data_.clear();
      }
      worst_model_data_ = model_data;
    }
  }
  worst_error_rate_ = error_rate;
  memcpy(worst_error_rates_, error_rates_, sizeof(error_rates_));
  worst_iteration_ = iteration;
  return result;
}

} // namespace tesseract.
