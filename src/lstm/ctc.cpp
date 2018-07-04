///////////////////////////////////////////////////////////////////////
// File:        ctc.cpp
// Description: Slightly improved standard CTC to compute the targets.
// Author:      Ray Smith
// Created:     Wed Jul 13 15:50:06 PDT 2016
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
#include "ctc.h"

#include <algorithm>
#include <cfloat>      // for FLT_MAX
#include <memory>

#include "genericvector.h"
#include "host.h"
#include "matrix.h"
#include "networkio.h"

#include "network.h"
#include "scrollview.h"

namespace tesseract {

// Magic constants that keep CTC stable.
// Minimum probability limit for softmax input to ctc_loss.
const float CTC::kMinProb_ = 1e-12;
// Maximum absolute argument to exp().
const double CTC::kMaxExpArg_ = 80.0;
// Minimum probability for total prob in time normalization.
const double CTC::kMinTotalTimeProb_ = 1e-8;
// Minimum probability for total prob in final normalization.
const double CTC::kMinTotalFinalProb_ = 1e-6;

// Builds a target using CTC. Slightly improved as follows:
// Includes normalizations and clipping for stability.
// labels should be pre-padded with nulls everywhere.
// labels can be longer than the time sequence, but the total number of
// essential labels (non-null plus nulls between equal labels) must not exceed
// the number of timesteps in outputs.
// outputs is the output of the network, and should have already been
// normalized with NormalizeProbs.
// On return targets is filled with the computed targets.
// Returns false if there is insufficient time for the labels.
/* static */
bool CTC::ComputeCTCTargets(const GenericVector<int>& labels, int null_char,
                            const GENERIC_2D_ARRAY<float>& outputs,
                            NetworkIO* targets) {
  std::unique_ptr<CTC> ctc(new CTC(labels, null_char, outputs));
  if (!ctc->ComputeLabelLimits()) {
    return false;  // Not enough time.
  }
  // Generate simple targets purely from the truth labels by spreading them
  // evenly over time.
  GENERIC_2D_ARRAY<float> simple_targets;
  ctc->ComputeSimpleTargets(&simple_targets);
  // Add the simple targets as a starter bias to the network outputs.
  float bias_fraction = ctc->CalculateBiasFraction();
  simple_targets *= bias_fraction;
  ctc->outputs_ += simple_targets;
  NormalizeProbs(&ctc->outputs_);
  // Run regular CTC on the biased outputs.
  // Run forward and backward
  GENERIC_2D_ARRAY<double> log_alphas, log_betas;
  ctc->Forward(&log_alphas);
  ctc->Backward(&log_betas);
  // Normalize and come out of log space with a clipped softmax over time.
  log_alphas += log_betas;
  ctc->NormalizeSequence(&log_alphas);
  ctc->LabelsToClasses(log_alphas, targets);
  NormalizeProbs(targets);
  return true;
}

CTC::CTC(const GenericVector<int>& labels, int null_char,
         const GENERIC_2D_ARRAY<float>& outputs)
    : labels_(labels), outputs_(outputs), null_char_(null_char) {
  num_timesteps_ = outputs.dim1();
  num_classes_ = outputs.dim2();
  num_labels_ = labels_.size();
}

// Computes vectors of min and max label index for each timestep, based on
// whether skippability of nulls makes it possible to complete a valid path.
bool CTC::ComputeLabelLimits() {
  min_labels_.init_to_size(num_timesteps_, 0);
  max_labels_.init_to_size(num_timesteps_, 0);
  int min_u = num_labels_ - 1;
  if (labels_[min_u] == null_char_) --min_u;
  for (int t = num_timesteps_ - 1; t >= 0; --t) {
    min_labels_[t] = min_u;
    if (min_u > 0) {
      --min_u;
      if (labels_[min_u] == null_char_ && min_u > 0 &&
          labels_[min_u + 1] != labels_[min_u - 1]) {
        --min_u;
      }
    }
  }
  int max_u = labels_[0] == null_char_;
  for (int t = 0; t < num_timesteps_; ++t) {
    max_labels_[t] = max_u;
    if (max_labels_[t] < min_labels_[t]) return false;  // Not enough room.
    if (max_u + 1 < num_labels_) {
      ++max_u;
      if (labels_[max_u] == null_char_ && max_u + 1 < num_labels_ &&
          labels_[max_u + 1] != labels_[max_u - 1]) {
        ++max_u;
      }
    }
  }
  return true;
}

// Computes targets based purely on the labels by spreading the labels evenly
// over the available timesteps.
void CTC::ComputeSimpleTargets(GENERIC_2D_ARRAY<float>* targets) const {
  // Initialize all targets to zero.
  targets->Resize(num_timesteps_, num_classes_, 0.0f);
  GenericVector<float> half_widths;
  GenericVector<int> means;
  ComputeWidthsAndMeans(&half_widths, &means);
  for (int l = 0; l < num_labels_; ++l) {
    int label = labels_[l];
    float left_half_width = half_widths[l];
    float right_half_width = left_half_width;
    int mean = means[l];
    if (label == null_char_) {
      if (!NeededNull(l)) {
        if ((l > 0 && mean == means[l - 1]) ||
            (l + 1 < num_labels_ && mean == means[l + 1])) {
          continue;  // Drop overlapping null.
        }
      }
      // Make sure that no space is left unoccupied and that non-nulls always
      // peak at 1 by stretching nulls to meet their neighbors.
      if (l > 0) left_half_width = mean - means[l - 1];
      if (l + 1 < num_labels_) right_half_width = means[l + 1] - mean;
    }
    if (mean >= 0 && mean < num_timesteps_) targets->put(mean, label, 1.0f);
    for (int offset = 1; offset < left_half_width && mean >= offset; ++offset) {
      float prob = 1.0f - offset / left_half_width;
      if (mean - offset < num_timesteps_ &&
          prob > targets->get(mean - offset, label)) {
        targets->put(mean - offset, label, prob);
      }
    }
    for (int offset = 1;
         offset < right_half_width && mean + offset < num_timesteps_;
         ++offset) {
      float prob = 1.0f - offset / right_half_width;
      if (mean + offset >= 0 && prob > targets->get(mean + offset, label)) {
        targets->put(mean + offset, label, prob);
      }
    }
  }
}

// Computes mean positions and half widths of the simple targets by spreading
// the labels evenly over the available timesteps.
void CTC::ComputeWidthsAndMeans(GenericVector<float>* half_widths,
                                GenericVector<int>* means) const {
  // Count the number of labels of each type, in regexp terms, counts plus
  // (non-null or necessary null, which must occur at least once) and star
  // (optional null).
  int num_plus = 0, num_star = 0;
  for (int i = 0; i < num_labels_; ++i) {
    if (labels_[i] != null_char_ || NeededNull(i))
      ++num_plus;
    else
      ++num_star;
  }
  // Compute the size for each type. If there is enough space for everything
  // to have size>=1, then all are equal, otherwise plus_size=1 and star gets
  // whatever is left-over.
  float plus_size = 1.0f, star_size = 0.0f;
  float total_floating = num_plus + num_star;
  if (total_floating <= num_timesteps_) {
    plus_size = star_size = num_timesteps_ / total_floating;
  } else if (num_star > 0) {
    star_size = static_cast<float>(num_timesteps_ - num_plus) / num_star;
  }
  // Set the width and compute the mean of each.
  float mean_pos = 0.0f;
  for (int i = 0; i < num_labels_; ++i) {
    float half_width;
    if (labels_[i] != null_char_ || NeededNull(i)) {
      half_width = plus_size / 2.0f;
    } else {
      half_width = star_size / 2.0f;
    }
    mean_pos += half_width;
    means->push_back(static_cast<int>(mean_pos));
    mean_pos += half_width;
    half_widths->push_back(half_width);
  }
}

// Helper returns the index of the highest probability label at timestep t.
static int BestLabel(const GENERIC_2D_ARRAY<float>& outputs, int t) {
  int result = 0;
  int num_classes = outputs.dim2();
  const float* outputs_t = outputs[t];
  for (int c = 1; c < num_classes; ++c) {
    if (outputs_t[c] > outputs_t[result]) result = c;
  }
  return result;
}

// Calculates and returns a suitable fraction of the simple targets to add
// to the network outputs.
float CTC::CalculateBiasFraction() {
  // Compute output labels via basic decoding.
  GenericVector<int> output_labels;
  for (int t = 0; t < num_timesteps_; ++t) {
    int label = BestLabel(outputs_, t);
    while (t + 1 < num_timesteps_ && BestLabel(outputs_, t + 1) == label) ++t;
    if (label != null_char_) output_labels.push_back(label);
  }
  // Simple bag of labels error calculation.
  GenericVector<int> truth_counts(num_classes_, 0);
  GenericVector<int> output_counts(num_classes_, 0);
  for (int l = 0; l < num_labels_; ++l) {
    ++truth_counts[labels_[l]];
  }
  for (int l = 0; l < output_labels.size(); ++l) {
    ++output_counts[output_labels[l]];
  }
  // Count the number of true and false positive non-nulls and truth labels.
  int true_pos = 0, false_pos = 0, total_labels = 0;
  for (int c = 0; c < num_classes_; ++c) {
    if (c == null_char_) continue;
    int truth_count = truth_counts[c];
    int ocr_count = output_counts[c];
    if (truth_count > 0) {
      total_labels += truth_count;
      if (ocr_count > truth_count) {
        true_pos += truth_count;
        false_pos += ocr_count - truth_count;
      } else {
        true_pos += ocr_count;
      }
    }
    // We don't need to count classes that don't exist in the truth as
    // false positives, because they don't affect CTC at all.
  }
  if (total_labels == 0) return 0.0f;
  return exp(std::max(true_pos - false_pos, 1) * log(kMinProb_) / total_labels);
}

// Given ln(x) and ln(y), returns ln(x + y), using:
// ln(x + y) = ln(y) + ln(1 + exp(ln(y) - ln(x)), ensuring that ln(x) is the
// bigger number to maximize precision.
static double LogSumExp(double ln_x, double ln_y) {
  if (ln_x >= ln_y) {
    return ln_x + log1p(exp(ln_y - ln_x));
  } else {
    return ln_y + log1p(exp(ln_x - ln_y));
  }
}

// Runs the forward CTC pass, filling in log_probs.
void CTC::Forward(GENERIC_2D_ARRAY<double>* log_probs) const {
  log_probs->Resize(num_timesteps_, num_labels_, -FLT_MAX);
  log_probs->put(0, 0, log(outputs_(0, labels_[0])));
  if (labels_[0] == null_char_)
    log_probs->put(0, 1, log(outputs_(0, labels_[1])));
  for (int t = 1; t < num_timesteps_; ++t) {
    const float* outputs_t = outputs_[t];
    for (int u = min_labels_[t]; u <= max_labels_[t]; ++u) {
      // Continuing the same label.
      double log_sum = log_probs->get(t - 1, u);
      // Change from previous label.
      if (u > 0) {
        log_sum = LogSumExp(log_sum, log_probs->get(t - 1, u - 1));
      }
      // Skip the null if allowed.
      if (u >= 2 && labels_[u - 1] == null_char_ &&
          labels_[u] != labels_[u - 2]) {
        log_sum = LogSumExp(log_sum, log_probs->get(t - 1, u - 2));
      }
      // Add in the log prob of the current label.
      double label_prob = outputs_t[labels_[u]];
      log_sum += log(label_prob);
      log_probs->put(t, u, log_sum);
    }
  }
}

// Runs the backward CTC pass, filling in log_probs.
void CTC::Backward(GENERIC_2D_ARRAY<double>* log_probs) const {
  log_probs->Resize(num_timesteps_, num_labels_, -FLT_MAX);
  log_probs->put(num_timesteps_ - 1, num_labels_ - 1, 0.0);
  if (labels_[num_labels_ - 1] == null_char_)
    log_probs->put(num_timesteps_ - 1, num_labels_ - 2, 0.0);
  for (int t = num_timesteps_ - 2; t >= 0; --t) {
    const float* outputs_tp1 = outputs_[t + 1];
    for (int u = min_labels_[t]; u <= max_labels_[t]; ++u) {
      // Continuing the same label.
      double log_sum = log_probs->get(t + 1, u) + log(outputs_tp1[labels_[u]]);
      // Change from previous label.
      if (u + 1 < num_labels_) {
        double prev_prob = outputs_tp1[labels_[u + 1]];
        log_sum =
            LogSumExp(log_sum, log_probs->get(t + 1, u + 1) + log(prev_prob));
      }
      // Skip the null if allowed.
      if (u + 2 < num_labels_ && labels_[u + 1] == null_char_ &&
          labels_[u] != labels_[u + 2]) {
        double skip_prob = outputs_tp1[labels_[u + 2]];
        log_sum =
            LogSumExp(log_sum, log_probs->get(t + 1, u + 2) + log(skip_prob));
      }
      log_probs->put(t, u, log_sum);
    }
  }
}

// Normalizes and brings probs out of log space with a softmax over time.
void CTC::NormalizeSequence(GENERIC_2D_ARRAY<double>* probs) const {
  double max_logprob = probs->Max();
  for (int u = 0; u < num_labels_; ++u) {
    double total = 0.0;
    for (int t = 0; t < num_timesteps_; ++t) {
      // Separate impossible path from unlikely probs.
      double prob = probs->get(t, u);
      if (prob > -FLT_MAX)
        prob = ClippedExp(prob - max_logprob);
      else
        prob = 0.0;
      total += prob;
      probs->put(t, u, prob);
    }
    // Note that although this is a probability distribution over time and
    // therefore should sum to 1, it is important to allow some labels to be
    // all zero, (or at least tiny) as it is necessary to skip some blanks.
    if (total < kMinTotalTimeProb_) total = kMinTotalTimeProb_;
    for (int t = 0; t < num_timesteps_; ++t)
      probs->put(t, u, probs->get(t, u) / total);
  }
}

// For each timestep computes the max prob for each class over all
// instances of the class in the labels_, and sets the targets to
// the max observed prob.
void CTC::LabelsToClasses(const GENERIC_2D_ARRAY<double>& probs,
                          NetworkIO* targets) const {
  // For each timestep compute the max prob for each class over all
  // instances of the class in the labels_.
  GenericVector<double> class_probs;
  for (int t = 0; t < num_timesteps_; ++t) {
    float* targets_t = targets->f(t);
    class_probs.init_to_size(num_classes_, 0.0);
    for (int u = 0; u < num_labels_; ++u) {
      double prob = probs(t, u);
      // Note that although Graves specifies sum over all labels of the same
      // class, we need to allow skipped blanks to go to zero, so they don't
      // interfere with the non-blanks, so max is better than sum.
      if (prob > class_probs[labels_[u]]) class_probs[labels_[u]] = prob;
      //         class_probs[labels_[u]] += prob;
    }
    int best_class = 0;
    for (int c = 0; c < num_classes_; ++c) {
      targets_t[c] = class_probs[c];
      if (class_probs[c] > class_probs[best_class]) best_class = c;
    }
  }
}

// Normalizes the probabilities such that no target has a prob below min_prob,
// and, provided that the initial total is at least min_total_prob, then all
// probs will sum to 1, otherwise to sum/min_total_prob. The maximum output
// probability is thus 1 - (num_classes-1)*min_prob.
/* static */
void CTC::NormalizeProbs(GENERIC_2D_ARRAY<float>* probs) {
  int num_timesteps = probs->dim1();
  int num_classes = probs->dim2();
  for (int t = 0; t < num_timesteps; ++t) {
    float* probs_t = (*probs)[t];
    // Compute the total and clip that to prevent amplification of noise.
    double total = 0.0;
    for (int c = 0; c < num_classes; ++c) total += probs_t[c];
    if (total < kMinTotalFinalProb_) total = kMinTotalFinalProb_;
    // Compute the increased total as a result of clipping.
    double increment = 0.0;
    for (int c = 0; c < num_classes; ++c) {
      double prob = probs_t[c] / total;
      if (prob < kMinProb_) increment += kMinProb_ - prob;
    }
    // Now normalize with clipping. Any additional clipping is negligible.
    total += increment;
    for (int c = 0; c < num_classes; ++c) {
      float prob = probs_t[c] / total;
      probs_t[c] = std::max(prob, kMinProb_);
    }
  }
}

// Returns true if the label at index is a needed null.
bool CTC::NeededNull(int index) const {
  return labels_[index] == null_char_ && index > 0 && index + 1 < num_labels_ &&
         labels_[index + 1] == labels_[index - 1];
}

}  // namespace tesseract
