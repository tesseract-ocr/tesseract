///////////////////////////////////////////////////////////////////////
// File:        ctc.h
// Description: Slightly improved standard CTC to compute the targets.
// Author:      Ray Smith
// Created:     Wed Jul 13 15:17:06 PDT 2016
//
// (C) Copyright 2016, Google Inc.
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

#ifndef TESSERACT_LSTM_CTC_H_
#define TESSERACT_LSTM_CTC_H_

#include "genericvector.h"
#include "network.h"
#include "networkio.h"
#include "scrollview.h"

namespace tesseract {

// Class to encapsulate CTC and simple target generation.
class CTC {
 public:
  // Normalizes the probabilities such that no target has a prob below min_prob,
  // and, provided that the initial total is at least min_total_prob, then all
  // probs will sum to 1, otherwise to sum/min_total_prob. The maximum output
  // probability is thus 1 - (num_classes-1)*min_prob.
  static void NormalizeProbs(NetworkIO* probs) {
    NormalizeProbs(probs->mutable_float_array());
  }

  // Builds a target using CTC. Slightly improved as follows:
  // Includes normalizations and clipping for stability.
  // labels should be pre-padded with nulls wherever desired, but they don't
  // have to be between all labels. Allows for multi-label codes with no
  // nulls between.
  // labels can be longer than the time sequence, but the total number of
  // essential labels (non-null plus nulls between equal labels) must not exceed
  // the number of timesteps in outputs.
  // outputs is the output of the network, and should have already been
  // normalized with NormalizeProbs.
  // On return targets is filled with the computed targets.
  // Returns false if there is insufficient time for the labels.
  static bool ComputeCTCTargets(const GenericVector<int>& truth_labels,
                                int null_char,
                                const GENERIC_2D_ARRAY<float>& outputs,
                                NetworkIO* targets);

 private:
  // Constructor is private as the instance only holds information specific to
  // the current labels, outputs etc, and is built by the static function.
  CTC(const GenericVector<int>& labels, int null_char,
      const GENERIC_2D_ARRAY<float>& outputs);

  // Computes vectors of min and max label index for each timestep, based on
  // whether skippability of nulls makes it possible to complete a valid path.
  bool ComputeLabelLimits();
  // Computes targets based purely on the labels by spreading the labels evenly
  // over the available timesteps.
  void ComputeSimpleTargets(GENERIC_2D_ARRAY<float>* targets) const;
  // Computes mean positions and half widths of the simple targets by spreading
  // the labels even over the available timesteps.
  void ComputeWidthsAndMeans(GenericVector<float>* half_widths,
                             GenericVector<int>* means) const;
  // Calculates and returns a suitable fraction of the simple targets to add
  // to the network outputs.
  float CalculateBiasFraction();
  // Runs the forward CTC pass, filling in log_probs.
  void Forward(GENERIC_2D_ARRAY<double>* log_probs) const;
  // Runs the backward CTC pass, filling in log_probs.
  void Backward(GENERIC_2D_ARRAY<double>* log_probs) const;
  // Normalizes and brings probs out of log space with a softmax over time.
  void NormalizeSequence(GENERIC_2D_ARRAY<double>* probs) const;
  // For each timestep computes the max prob for each class over all
  // instances of the class in the labels_, and sets the targets to
  // the max observed prob.
  void LabelsToClasses(const GENERIC_2D_ARRAY<double>& probs,
                       NetworkIO* targets) const;
  // Normalizes the probabilities such that no target has a prob below min_prob,
  // and, provided that the initial total is at least min_total_prob, then all
  // probs will sum to 1, otherwise to sum/min_total_prob. The maximum output
  // probability is thus 1 - (num_classes-1)*min_prob.
  static void NormalizeProbs(GENERIC_2D_ARRAY<float>* probs);
  // Returns true if the label at index is a needed null.
  bool NeededNull(int index) const;
  // Returns exp(clipped(x)), clipping x to a reasonable range to prevent over/
  // underflow.
  static double ClippedExp(double x) {
    if (x < -kMaxExpArg_) return exp(-kMaxExpArg_);
    if (x > kMaxExpArg_) return exp(kMaxExpArg_);
    return exp(x);
  }

  // Minimum probability limit for softmax input to ctc_loss.
  static const float kMinProb_;
  // Maximum absolute argument to exp().
  static const double kMaxExpArg_;
  // Minimum probability for total prob in time normalization.
  static const double kMinTotalTimeProb_;
  // Minimum probability for total prob in final normalization.
  static const double kMinTotalFinalProb_;

  // The truth label indices that are to be matched to outputs_.
  const GenericVector<int>& labels_;
  // The network outputs.
  GENERIC_2D_ARRAY<float> outputs_;
  // The null or "blank" label.
  int null_char_;
  // Number of timesteps in outputs_.
  int num_timesteps_;
  // Number of classes in outputs_.
  int num_classes_;
  // Number of labels in labels_.
  int num_labels_;
  // Min and max valid label indices for each timestep.
  GenericVector<int> min_labels_;
  GenericVector<int> max_labels_;
};

}  // namespace tesseract

#endif  // TESSERACT_LSTM_CTC_H_
