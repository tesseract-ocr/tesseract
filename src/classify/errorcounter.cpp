// Copyright 2011 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////
#include <algorithm>
#include <ctime>

#include "errorcounter.h"

#include "fontinfo.h"
#include "sampleiterator.h"
#include "shapeclassifier.h"
#include "shapetable.h"
#include "trainingsample.h"
#include "trainingsampleset.h"
#include "unicity_table.h"

namespace tesseract {

// Difference in result rating to be thought of as an "equal" choice.
const double kRatingEpsilon = 1.0 / 32;

// Tests a classifier, computing its error rate.
// See errorcounter.h for description of arguments.
// Iterates over the samples, calling the classifier in normal/silent mode.
// If the classifier makes a CT_UNICHAR_TOPN_ERR error, and the appropriate
// report_level is set (4 or greater), it will then call the classifier again
// with a debug flag and a keep_this argument to find out what is going on.
double ErrorCounter::ComputeErrorRate(ShapeClassifier* classifier,
    int report_level, CountTypes boosting_mode,
    const FontInfoTable& fontinfo_table,
    const GenericVector<Pix*>& page_images, SampleIterator* it,
    double* unichar_error,  double* scaled_error, STRING* fonts_report) {
  const int fontsize = it->sample_set()->NumFonts();
  ErrorCounter counter(classifier->GetUnicharset(), fontsize);
  GenericVector<UnicharRating> results;

  clock_t start = clock();
  unsigned total_samples = 0;
  double unscaled_error = 0.0;
  // Set a number of samples on which to run the classify debug mode.
  int error_samples = report_level > 3 ? report_level * report_level : 0;
  // Iterate over all the samples, accumulating errors.
  for (it->Begin(); !it->AtEnd(); it->Next()) {
    TrainingSample* mutable_sample = it->MutableSample();
    int page_index = mutable_sample->page_num();
    Pix* page_pix = 0 <= page_index && page_index < page_images.size()
                  ? page_images[page_index] : nullptr;
    // No debug, no keep this.
    classifier->UnicharClassifySample(*mutable_sample, page_pix, 0,
                                      INVALID_UNICHAR_ID, &results);
    bool debug_it = false;
    int correct_id = mutable_sample->class_id();
    if (counter.unicharset_.has_special_codes() &&
        (correct_id == UNICHAR_SPACE || correct_id == UNICHAR_JOINED ||
         correct_id == UNICHAR_BROKEN)) {
      // This is junk so use the special counter.
      debug_it = counter.AccumulateJunk(report_level > 3,
                                        results,
                                        mutable_sample);
    } else {
      debug_it = counter.AccumulateErrors(report_level > 3, boosting_mode,
                                          fontinfo_table,
                                          results, mutable_sample);
    }
    if (debug_it && error_samples > 0) {
      // Running debug, keep the correct answer, and debug the classifier.
      tprintf("Error on sample %d: %s Classifier debug output:\n",
              it->GlobalSampleIndex(),
              it->sample_set()->SampleToString(*mutable_sample).string());
      classifier->DebugDisplay(*mutable_sample, page_pix, correct_id);
      --error_samples;
    }
    ++total_samples;
  }
  const double total_time = 1.0 * (clock() - start) / CLOCKS_PER_SEC;
  // Create the appropriate error report.
  unscaled_error = counter.ReportErrors(report_level, boosting_mode,
                                        fontinfo_table,
                                        *it, unichar_error, fonts_report);
  if (scaled_error != nullptr) *scaled_error = counter.scaled_error_;
  if (report_level > 1 && total_samples > 0) {
    // It is useful to know the time in microseconds/char.
    tprintf("Errors computed in %.2fs at %.1f μs/char\n",
            total_time, 1000000.0 * total_time / total_samples);
  }
  return unscaled_error;
}

// Tests a pair of classifiers, debugging errors of the new against the old.
// See errorcounter.h for description of arguments.
// Iterates over the samples, calling the classifiers in normal/silent mode.
// If the new_classifier makes a boosting_mode error that the old_classifier
// does not, it will then call the new_classifier again with a debug flag
// and a keep_this argument to find out what is going on.
void ErrorCounter::DebugNewErrors(
    ShapeClassifier* new_classifier, ShapeClassifier* old_classifier,
    CountTypes boosting_mode,
    const FontInfoTable& fontinfo_table,
    const GenericVector<Pix*>& page_images, SampleIterator* it) {
  int fontsize = it->sample_set()->NumFonts();
  ErrorCounter old_counter(old_classifier->GetUnicharset(), fontsize);
  ErrorCounter new_counter(new_classifier->GetUnicharset(), fontsize);
  GenericVector<UnicharRating> results;

  int total_samples = 0;
  int error_samples = 25;
  int total_new_errors = 0;
  // Iterate over all the samples, accumulating errors.
  for (it->Begin(); !it->AtEnd(); it->Next()) {
    TrainingSample* mutable_sample = it->MutableSample();
    int page_index = mutable_sample->page_num();
    Pix* page_pix = 0 <= page_index && page_index < page_images.size()
                  ? page_images[page_index] : nullptr;
    // No debug, no keep this.
    old_classifier->UnicharClassifySample(*mutable_sample, page_pix, 0,
                                          INVALID_UNICHAR_ID, &results);
    int correct_id = mutable_sample->class_id();
    if (correct_id != 0 &&
        !old_counter.AccumulateErrors(true, boosting_mode, fontinfo_table,
                                      results, mutable_sample)) {
      // old classifier was correct, check the new one.
      new_classifier->UnicharClassifySample(*mutable_sample, page_pix, 0,
                                            INVALID_UNICHAR_ID, &results);
      if (correct_id != 0 &&
          new_counter.AccumulateErrors(true, boosting_mode, fontinfo_table,
                                        results, mutable_sample)) {
        tprintf("New Error on sample %d: Classifier debug output:\n",
                it->GlobalSampleIndex());
        ++total_new_errors;
        new_classifier->UnicharClassifySample(*mutable_sample, page_pix, 1,
                                              correct_id, &results);
        if (results.size() > 0 && error_samples > 0) {
          new_classifier->DebugDisplay(*mutable_sample, page_pix, correct_id);
          --error_samples;
        }
      }
    }
    ++total_samples;
  }
  tprintf("Total new errors = %d\n", total_new_errors);
}

// Constructor is private. Only anticipated use of ErrorCounter is via
// the static ComputeErrorRate.
ErrorCounter::ErrorCounter(const UNICHARSET& unicharset, int fontsize)
  : scaled_error_(0.0), rating_epsilon_(kRatingEpsilon),
    unichar_counts_(unicharset.size(), unicharset.size(), 0),
    ok_score_hist_(0, 101), bad_score_hist_(0, 101),
    unicharset_(unicharset) {
  Counts empty_counts;
  font_counts_.init_to_size(fontsize, empty_counts);
  multi_unichar_counts_.init_to_size(unicharset.size(), 0);
}

// Accumulates the errors from the classifier results on a single sample.
// Returns true if debug is true and a CT_UNICHAR_TOPN_ERR error occurred.
// boosting_mode selects the type of error to be used for boosting and the
// is_error_ member of sample is set according to whether the required type
// of error occurred. The font_table provides access to font properties
// for error counting and shape_table is used to understand the relationship
// between unichar_ids and shape_ids in the results
bool ErrorCounter::AccumulateErrors(bool debug, CountTypes boosting_mode,
                                    const FontInfoTable& font_table,
                                    const GenericVector<UnicharRating>& results,
                                    TrainingSample* sample) {
  int num_results = results.size();
  int answer_actual_rank = -1;
  int font_id = sample->font_id();
  int unichar_id = sample->class_id();
  sample->set_is_error(false);
  if (num_results == 0) {
    // Reject. We count rejects as a separate category, but still mark the
    // sample as an error in case any training module wants to use that to
    // improve the classifier.
    sample->set_is_error(true);
    ++font_counts_[font_id].n[CT_REJECT];
  } else {
    // Find rank of correct unichar answer, using rating_epsilon_ to allow
    // different answers to score as equal. (Ignoring the font.)
    int epsilon_rank = 0;
    int answer_epsilon_rank = -1;
    int num_top_answers = 0;
    double prev_rating = results[0].rating;
    bool joined = false;
    bool broken = false;
    int res_index = 0;
    while (res_index < num_results) {
      if (results[res_index].rating < prev_rating - rating_epsilon_) {
        ++epsilon_rank;
        prev_rating = results[res_index].rating;
      }
      if (results[res_index].unichar_id == unichar_id &&
          answer_epsilon_rank < 0) {
        answer_epsilon_rank = epsilon_rank;
        answer_actual_rank = res_index;
      }
      if (results[res_index].unichar_id == UNICHAR_JOINED &&
          unicharset_.has_special_codes())
        joined = true;
      else if (results[res_index].unichar_id == UNICHAR_BROKEN &&
               unicharset_.has_special_codes())
        broken = true;
      else if (epsilon_rank == 0)
        ++num_top_answers;
      ++res_index;
    }
    if (answer_actual_rank != 0) {
      // Correct result is not absolute top.
      ++font_counts_[font_id].n[CT_UNICHAR_TOPTOP_ERR];
      if (boosting_mode == CT_UNICHAR_TOPTOP_ERR) sample->set_is_error(true);
    }
    if (answer_epsilon_rank == 0) {
      ++font_counts_[font_id].n[CT_UNICHAR_TOP_OK];
      // Unichar OK, but count if multiple unichars.
      if (num_top_answers > 1) {
        ++font_counts_[font_id].n[CT_OK_MULTI_UNICHAR];
        ++multi_unichar_counts_[unichar_id];
      }
      // Check to see if any font in the top choice has attributes that match.
      // TODO(rays) It is easy to add counters for individual font attributes
      // here if we want them.
      if (font_table.SetContainsFontProperties(
          font_id, results[answer_actual_rank].fonts)) {
        // Font attributes were matched.
        // Check for multiple properties.
        if (font_table.SetContainsMultipleFontProperties(
            results[answer_actual_rank].fonts))
          ++font_counts_[font_id].n[CT_OK_MULTI_FONT];
      } else {
        // Font attributes weren't matched.
        ++font_counts_[font_id].n[CT_FONT_ATTR_ERR];
      }
    } else {
      // This is a top unichar error.
      ++font_counts_[font_id].n[CT_UNICHAR_TOP1_ERR];
      if (boosting_mode == CT_UNICHAR_TOP1_ERR) sample->set_is_error(true);
      // Count maps from unichar id to wrong unichar id.
      ++unichar_counts_(unichar_id, results[0].unichar_id);
      if (answer_epsilon_rank < 0 || answer_epsilon_rank >= 2) {
        // It is also a 2nd choice unichar error.
        ++font_counts_[font_id].n[CT_UNICHAR_TOP2_ERR];
        if (boosting_mode == CT_UNICHAR_TOP2_ERR) sample->set_is_error(true);
      }
      if (answer_epsilon_rank < 0) {
        // It is also a top-n choice unichar error.
        ++font_counts_[font_id].n[CT_UNICHAR_TOPN_ERR];
        if (boosting_mode == CT_UNICHAR_TOPN_ERR) sample->set_is_error(true);
        answer_epsilon_rank = epsilon_rank;
      }
    }
    // Compute mean number of return values and mean rank of correct answer.
    font_counts_[font_id].n[CT_NUM_RESULTS] += num_results;
    font_counts_[font_id].n[CT_RANK] += answer_epsilon_rank;
    if (joined)
      ++font_counts_[font_id].n[CT_OK_JOINED];
    if (broken)
      ++font_counts_[font_id].n[CT_OK_BROKEN];
  }
  // If it was an error for boosting then sum the weight.
  if (sample->is_error()) {
    scaled_error_ += sample->weight();
    if (debug) {
      tprintf("%d results for char %s font %d :",
              num_results, unicharset_.id_to_unichar(unichar_id),
              font_id);
      for (int i = 0; i < num_results; ++i) {
        tprintf(" %.3f : %s\n",
                results[i].rating,
                unicharset_.id_to_unichar(results[i].unichar_id));
      }
      return true;
    }
    int percent = 0;
    if (num_results > 0)
      percent = IntCastRounded(results[0].rating * 100);
    bad_score_hist_.add(percent, 1);
  } else {
    int percent = 0;
    if (answer_actual_rank >= 0)
      percent = IntCastRounded(results[answer_actual_rank].rating * 100);
    ok_score_hist_.add(percent, 1);
  }
  return false;
}

// Accumulates counts for junk. Counts only whether the junk was correctly
// rejected or not.
bool ErrorCounter::AccumulateJunk(bool debug,
                                  const GenericVector<UnicharRating>& results,
                                  TrainingSample* sample) {
  // For junk we accept no answer, or an explicit shape answer matching the
  // class id of the sample.
  const int num_results = results.size();
  const int font_id = sample->font_id();
  const int unichar_id = sample->class_id();
  int percent = 0;
  if (num_results > 0)
    percent = IntCastRounded(results[0].rating * 100);
  if (num_results > 0 && results[0].unichar_id != unichar_id) {
    // This is a junk error.
    ++font_counts_[font_id].n[CT_ACCEPTED_JUNK];
    sample->set_is_error(true);
    // It counts as an error for boosting too so sum the weight.
    scaled_error_ += sample->weight();
    bad_score_hist_.add(percent, 1);
    return debug;
  } else {
    // Correctly rejected.
    ++font_counts_[font_id].n[CT_REJECTED_JUNK];
    sample->set_is_error(false);
    ok_score_hist_.add(percent, 1);
  }
  return false;
}

// Creates a report of the error rate. The report_level controls the detail
// that is reported to stderr via tprintf:
// 0   -> no output.
// >=1 -> bottom-line error rate.
// >=3 -> font-level error rate.
// boosting_mode determines the return value. It selects which (un-weighted)
// error rate to return.
// The fontinfo_table from MasterTrainer provides the names of fonts.
// The it determines the current subset of the training samples.
// If not nullptr, the top-choice unichar error rate is saved in unichar_error.
// If not nullptr, the report string is saved in fonts_report.
// (Ignoring report_level).
double ErrorCounter::ReportErrors(int report_level, CountTypes boosting_mode,
                                  const FontInfoTable& fontinfo_table,
                                  const SampleIterator& it,
                                  double* unichar_error,
                                  STRING* fonts_report) {
  // Compute totals over all the fonts and report individual font results
  // when required.
  Counts totals;
  int fontsize = font_counts_.size();
  for (int f = 0; f < fontsize; ++f) {
    // Accumulate counts over fonts.
    totals += font_counts_[f];
    STRING font_report;
    if (ReportString(false, font_counts_[f], &font_report)) {
      if (fonts_report != nullptr) {
        *fonts_report += fontinfo_table.get(f).name;
        *fonts_report += ": ";
        *fonts_report += font_report;
        *fonts_report += "\n";
      }
      if (report_level > 2) {
        // Report individual font error rates.
        tprintf("%s: %s\n", fontinfo_table.get(f).name, font_report.string());
      }
    }
  }
  // Report the totals.
  STRING total_report;
  bool any_results = ReportString(true, totals, &total_report);
  if (fonts_report != nullptr && fonts_report->length() == 0) {
    // Make sure we return something even if there were no samples.
    *fonts_report = "NoSamplesFound: ";
    *fonts_report += total_report;
    *fonts_report += "\n";
  }
  if (report_level > 0) {
    // Report the totals.
    STRING total_report;
    if (any_results) {
      tprintf("TOTAL Scaled Err=%.4g%%, %s\n",
              scaled_error_ * 100.0, total_report.string());
    }
    // Report the worst substitution error only for now.
    if (totals.n[CT_UNICHAR_TOP1_ERR] > 0) {
      int charsetsize = unicharset_.size();
      int worst_uni_id = 0;
      int worst_result_id = 0;
      int worst_err = 0;
      for (int u = 0; u < charsetsize; ++u) {
        for (int v = 0; v < charsetsize; ++v) {
          if (unichar_counts_(u, v) > worst_err) {
            worst_err = unichar_counts_(u, v);
            worst_uni_id = u;
            worst_result_id = v;
          }
        }
      }
      if (worst_err > 0) {
        tprintf("Worst error = %d:%s -> %s with %d/%d=%.2f%% errors\n",
                worst_uni_id, unicharset_.id_to_unichar(worst_uni_id),
                unicharset_.id_to_unichar(worst_result_id),
                worst_err, totals.n[CT_UNICHAR_TOP1_ERR],
                100.0 * worst_err / totals.n[CT_UNICHAR_TOP1_ERR]);
      }
    }
    tprintf("Multi-unichar shape use:\n");
    for (int u = 0; u < multi_unichar_counts_.size(); ++u) {
      if (multi_unichar_counts_[u] > 0) {
        tprintf("%d multiple answers for unichar: %s\n",
                multi_unichar_counts_[u],
                unicharset_.id_to_unichar(u));
      }
    }
    tprintf("OK Score histogram:\n");
    ok_score_hist_.print();
    tprintf("ERROR Score histogram:\n");
    bad_score_hist_.print();
  }

  double rates[CT_SIZE];
  if (!ComputeRates(totals, rates))
    return 0.0;
  // Set output values if asked for.
  if (unichar_error != nullptr)
    *unichar_error = rates[CT_UNICHAR_TOP1_ERR];
  return rates[boosting_mode];
}

// Sets the report string to a combined human and machine-readable report
// string of the error rates.
// Returns false if there is no data, leaving report unchanged, unless
// even_if_empty is true.
bool ErrorCounter::ReportString(bool even_if_empty, const Counts& counts,
                                STRING* report) {
  // Compute the error rates.
  double rates[CT_SIZE];
  if (!ComputeRates(counts, rates) && !even_if_empty)
    return false;
  // Using %.4g%%, the length of the output string should exactly match the
  // length of the format string, but in case of overflow, allow for +eddd
  // on each number.
  const int kMaxExtraLength = 5;  // Length of +eddd.
  // Keep this format string and the snprintf in sync with the CountTypes enum.
  const char* format_str = "Unichar=%.4g%%[1], %.4g%%[2], %.4g%%[n], %.4g%%[T] "
                           "Mult=%.4g%%, Jn=%.4g%%, Brk=%.4g%%, Rej=%.4g%%, "
                           "FontAttr=%.4g%%, Multi=%.4g%%, "
                           "Answers=%.3g, Rank=%.3g, "
                           "OKjunk=%.4g%%, Badjunk=%.4g%%";
  const size_t max_str_len = strlen(format_str) + kMaxExtraLength * (CT_SIZE - 1) + 1;
  char* formatted_str = new char[max_str_len];
  snprintf(formatted_str, max_str_len, format_str,
           rates[CT_UNICHAR_TOP1_ERR] * 100.0,
           rates[CT_UNICHAR_TOP2_ERR] * 100.0,
           rates[CT_UNICHAR_TOPN_ERR] * 100.0,
           rates[CT_UNICHAR_TOPTOP_ERR] * 100.0,
           rates[CT_OK_MULTI_UNICHAR] * 100.0,
           rates[CT_OK_JOINED] * 100.0,
           rates[CT_OK_BROKEN] * 100.0,
           rates[CT_REJECT] * 100.0,
           rates[CT_FONT_ATTR_ERR] * 100.0,
           rates[CT_OK_MULTI_FONT] * 100.0,
           rates[CT_NUM_RESULTS],
           rates[CT_RANK],
           100.0 * rates[CT_REJECTED_JUNK],
           100.0 * rates[CT_ACCEPTED_JUNK]);
  *report = formatted_str;
  delete [] formatted_str;
  // Now append each field of counts with a tab in front so the result can
  // be loaded into a spreadsheet.
  for (int ct = 0; ct < CT_SIZE; ++ct)
    report->add_str_int("\t", counts.n[ct]);
  return true;
}

// Computes the error rates and returns in rates which is an array of size
// CT_SIZE. Returns false if there is no data, leaving rates unchanged.
bool ErrorCounter::ComputeRates(const Counts& counts, double rates[CT_SIZE]) {
  const int ok_samples = counts.n[CT_UNICHAR_TOP_OK] + counts.n[CT_UNICHAR_TOP1_ERR] +
      counts.n[CT_REJECT];
  const int junk_samples = counts.n[CT_REJECTED_JUNK] + counts.n[CT_ACCEPTED_JUNK];
  // Compute rates for normal chars.
  double denominator = static_cast<double>(std::max(ok_samples, 1));
  for (int ct = 0; ct <= CT_RANK; ++ct)
    rates[ct] = counts.n[ct] / denominator;
  // Compute rates for junk.
  denominator = static_cast<double>(std::max(junk_samples, 1));
  for (int ct = CT_REJECTED_JUNK; ct <= CT_ACCEPTED_JUNK; ++ct)
    rates[ct] = counts.n[ct] / denominator;
  return ok_samples != 0 || junk_samples != 0;
}

ErrorCounter::Counts::Counts() {
  memset(n, 0, sizeof(n[0]) * CT_SIZE);
}
// Adds other into this for computing totals.
void ErrorCounter::Counts::operator+=(const Counts& other) {
  for (int ct = 0; ct < CT_SIZE; ++ct)
    n[ct] += other.n[ct];
}


}  // namespace tesseract.
