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
#include <ctime>

#include "errorcounter.h"

#include "fontinfo.h"
#include "ndminx.h"
#include "sampleiterator.h"
#include "shapeclassifier.h"
#include "shapetable.h"
#include "trainingsample.h"
#include "trainingsampleset.h"
#include "unicity_table.h"

namespace tesseract {

// Tests a classifier, computing its error rate.
// See errorcounter.h for description of arguments.
// Iterates over the samples, calling the classifier in normal/silent mode.
// If the classifier makes a CT_UNICHAR_TOPN_ERR error, and the appropriate
// report_level is set (4 or greater), it will then call the classifier again
// with a debug flag and a keep_this argument to find out what is going on.
double ErrorCounter::ComputeErrorRate(ShapeClassifier* classifier,
    int report_level, CountTypes boosting_mode,
    const UnicityTable<FontInfo>& fontinfo_table,
    const GenericVector<Pix*>& page_images, SampleIterator* it,
    double* unichar_error,  double* scaled_error, STRING* fonts_report) {
  int charsetsize = it->shape_table()->unicharset().size();
  int shapesize = it->CompactCharsetSize();
  int fontsize = it->sample_set()->NumFonts();
  ErrorCounter counter(charsetsize, shapesize, fontsize);
  GenericVector<ShapeRating> results;

  clock_t start = clock();
  int total_samples = 0;
  double unscaled_error = 0.0;
  // Set a number of samples on which to run the classify debug mode.
  int error_samples = report_level > 3 ? report_level * report_level : 0;
  // Iterate over all the samples, accumulating errors.
  for (it->Begin(); !it->AtEnd(); it->Next()) {
    TrainingSample* mutable_sample = it->MutableSample();
    int page_index = mutable_sample->page_num();
    Pix* page_pix = 0 <= page_index && page_index < page_images.size()
                  ? page_images[page_index] : NULL;
    // No debug, no keep this.
    classifier->ClassifySample(*mutable_sample, page_pix, 0, INVALID_UNICHAR_ID,
                               &results);
    if (mutable_sample->class_id() == 0) {
      // This is junk so use the special counter.
      counter.AccumulateJunk(*it->shape_table(), results, mutable_sample);
    } else if (counter.AccumulateErrors(report_level > 3, boosting_mode,
                                        fontinfo_table, *it->shape_table(),
                                        results, mutable_sample) &&
               error_samples > 0) {
      // Running debug, keep the correct answer, and debug the classifier.
      tprintf("Error on sample %d: Classifier debug output:\n",
              it->GlobalSampleIndex());
      int keep_this = it->GetSparseClassID();
      classifier->ClassifySample(*mutable_sample, page_pix, 1, keep_this,
                                 &results);
      --error_samples;
    }
    ++total_samples;
  }
  double total_time = 1.0 * (clock() - start) / CLOCKS_PER_SEC;
  // Create the appropriate error report.
  unscaled_error = counter.ReportErrors(report_level, boosting_mode,
                                        fontinfo_table,
                                        *it, unichar_error, fonts_report);
  if (scaled_error != NULL) *scaled_error = counter.scaled_error_;
  if (report_level > 1) {
    // It is useful to know the time in microseconds/char.
    tprintf("Errors computed in %.2fs at %.1f μs/char\n",
            total_time, 1000000.0 * total_time / total_samples);
  }
  return unscaled_error;
}

// Constructor is private. Only anticipated use of ErrorCounter is via
// the static ComputeErrorRate.
ErrorCounter::ErrorCounter(int charsetsize, int shapesize, int fontsize)
  : scaled_error_(0.0), unichar_counts_(charsetsize, shapesize, 0) {
  Counts empty_counts;
  font_counts_.init_to_size(fontsize, empty_counts);
}
ErrorCounter::~ErrorCounter() {
}

// Accumulates the errors from the classifier results on a single sample.
// Returns true if debug is true and a CT_UNICHAR_TOPN_ERR error occurred.
// boosting_mode selects the type of error to be used for boosting and the
// is_error_ member of sample is set according to whether the required type
// of error occurred. The font_table provides access to font properties
// for error counting and shape_table is used to understand the relationship
// between unichar_ids and shape_ids in the results
bool ErrorCounter::AccumulateErrors(bool debug, CountTypes boosting_mode,
                                    const UnicityTable<FontInfo>& font_table,
                                    const ShapeTable& shape_table,
                                    const GenericVector<ShapeRating>& results,
                                    TrainingSample* sample) {
  int num_results = results.size();
  int res_index = 0;
  bool debug_it = false;
  int font_id = sample->font_id();
  int unichar_id = sample->class_id();
  sample->set_is_error(false);
  if (num_results == 0) {
    // Reject. We count rejects as a separate category, but still mark the
    // sample as an error in case any training module wants to use that to
    // improve the classifier.
    sample->set_is_error(true);
    ++font_counts_[font_id].n[CT_REJECT];
  } else if (shape_table.GetShape(results[0].shape_id).
          ContainsUnicharAndFont(unichar_id, font_id)) {
    ++font_counts_[font_id].n[CT_SHAPE_TOP_CORRECT];
    // Unichar and font OK, but count if multiple unichars.
    if (shape_table.GetShape(results[0].shape_id).size() > 1)
      ++font_counts_[font_id].n[CT_OK_MULTI_UNICHAR];
  } else {
    // This is a top shape error.
    ++font_counts_[font_id].n[CT_SHAPE_TOP_ERR];
    // Check to see if any font in the top choice has attributes that match.
    bool attributes_match = false;
    uinT32 font_props = font_table.get(font_id).properties;
    const Shape& shape = shape_table.GetShape(results[0].shape_id);
    for (int c = 0; c < shape.size() && !attributes_match; ++c) {
      for (int f = 0; f < shape[c].font_ids.size(); ++f) {
        if (font_table.get(shape[c].font_ids[f]).properties == font_props) {
          attributes_match = true;
          break;
        }
      }
    }
    // TODO(rays) It is easy to add counters for individual font attributes
    // here if we want them.
    if (!attributes_match)
      ++font_counts_[font_id].n[CT_FONT_ATTR_ERR];
    if (boosting_mode == CT_SHAPE_TOP_ERR) sample->set_is_error(true);
    // Find rank of correct unichar answer. (Ignoring the font.)
    while (res_index < num_results &&
           !shape_table.GetShape(results[res_index].shape_id).
                ContainsUnichar(unichar_id)) {
      ++res_index;
    }
    if (res_index == 0) {
      // Unichar OK, but count if multiple unichars.
      if (shape_table.GetShape(results[res_index].shape_id).size() > 1) {
        ++font_counts_[font_id].n[CT_OK_MULTI_UNICHAR];
      }
    } else {
      // Count maps from unichar id to shape id.
      if (num_results > 0)
        ++unichar_counts_(unichar_id, results[0].shape_id);
      // This is a unichar error.
      ++font_counts_[font_id].n[CT_UNICHAR_TOP1_ERR];
      if (boosting_mode == CT_UNICHAR_TOP1_ERR) sample->set_is_error(true);
      if (res_index >= MIN(2, num_results)) {
        // It is also a 2nd choice unichar error.
        ++font_counts_[font_id].n[CT_UNICHAR_TOP2_ERR];
        if (boosting_mode == CT_UNICHAR_TOP2_ERR) sample->set_is_error(true);
      }
      if (res_index >= num_results) {
        // It is also a top-n choice unichar error.
        ++font_counts_[font_id].n[CT_UNICHAR_TOPN_ERR];
        if (boosting_mode == CT_UNICHAR_TOPN_ERR) sample->set_is_error(true);
        debug_it = debug;
      }
    }
  }
  // Compute mean number of return values and mean rank of correct answer.
  font_counts_[font_id].n[CT_NUM_RESULTS] += num_results;
  font_counts_[font_id].n[CT_RANK] += res_index;
  // If it was an error for boosting then sum the weight.
  if (sample->is_error()) {
    scaled_error_ += sample->weight();
  }
  if (debug_it) {
    tprintf("%d results for char %s font %d :",
            num_results, shape_table.unicharset().id_to_unichar(unichar_id),
            font_id);
    for (int i = 0; i < num_results; ++i) {
      tprintf(" %.3f/%.3f:%s",
              results[i].rating, results[i].font,
              shape_table.DebugStr(results[i].shape_id).string());
    }
    tprintf("\n");
    return true;
  }
  return false;
}

// Accumulates counts for junk. Counts only whether the junk was correctly
// rejected or not.
void ErrorCounter::AccumulateJunk(const ShapeTable& shape_table,
                                  const GenericVector<ShapeRating>& results,
                                  TrainingSample* sample) {
  // For junk we accept no answer, or an explicit shape answer matching the
  // class id of the sample.
  int num_results = results.size();
  int font_id = sample->font_id();
  int unichar_id = sample->class_id();
  if (num_results > 0 &&
      !shape_table.GetShape(results[0].shape_id).ContainsUnichar(unichar_id)) {
    // This is a junk error.
    ++font_counts_[font_id].n[CT_ACCEPTED_JUNK];
    sample->set_is_error(true);
    // It counts as an error for boosting too so sum the weight.
    scaled_error_ += sample->weight();
  } else {
    // Correctly rejected.
    ++font_counts_[font_id].n[CT_REJECTED_JUNK];
    sample->set_is_error(false);
  }
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
// If not NULL, the top-choice unichar error rate is saved in unichar_error.
// If not NULL, the report string is saved in fonts_report.
// (Ignoring report_level).
double ErrorCounter::ReportErrors(int report_level, CountTypes boosting_mode,
                                  const UnicityTable<FontInfo>& fontinfo_table,
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
    if (ReportString(font_counts_[f], &font_report)) {
      if (fonts_report != NULL) {
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
  if (report_level > 0) {
    // Report the totals.
    STRING total_report;
    if (ReportString(totals, &total_report)) {
      tprintf("TOTAL Scaled Err=%.4g%%, %s\n",
              scaled_error_ * 100.0, total_report.string());
    }
    // Report the worst substitution error only for now.
    if (totals.n[CT_UNICHAR_TOP1_ERR] > 0) {
      const UNICHARSET& unicharset = it.shape_table()->unicharset();
      int charsetsize = unicharset.size();
      int shapesize = it.CompactCharsetSize();
      int worst_uni_id = 0;
      int worst_shape_id = 0;
      int worst_err = 0;
      for (int u = 0; u < charsetsize; ++u) {
        for (int s = 0; s < shapesize; ++s) {
          if (unichar_counts_(u, s) > worst_err) {
            worst_err = unichar_counts_(u, s);
            worst_uni_id = u;
            worst_shape_id = s;
          }
        }
      }
      if (worst_err > 0) {
        tprintf("Worst error = %d:%s -> %s with %d/%d=%.2f%% errors\n",
                worst_uni_id, unicharset.id_to_unichar(worst_uni_id),
                it.shape_table()->DebugStr(worst_shape_id).string(),
                worst_err, totals.n[CT_UNICHAR_TOP1_ERR],
                100.0 * worst_err / totals.n[CT_UNICHAR_TOP1_ERR]);
      }
    }
  }
  double rates[CT_SIZE];
  if (!ComputeRates(totals, rates))
    return 0.0;
  // Set output values if asked for.
  if (unichar_error != NULL)
    *unichar_error = rates[CT_UNICHAR_TOP1_ERR];
  return rates[boosting_mode];
}

// Sets the report string to a combined human and machine-readable report
// string of the error rates.
// Returns false if there is no data, leaving report unchanged.
bool ErrorCounter::ReportString(const Counts& counts, STRING* report) {
  // Compute the error rates.
  double rates[CT_SIZE];
  if (!ComputeRates(counts, rates))
    return false;
  // Using %.4g%%, the length of the output string should exactly match the
  // length of the format string, but in case of overflow, allow for +eddd
  // on each number.
  const int kMaxExtraLength = 5;  // Length of +eddd.
  // Keep this format string and the snprintf in sync with the CountTypes enum.
  const char* format_str = "ShapeErr=%.4g%%, FontAttr=%.4g%%, "
                           "Unichar=%.4g%%[1], %.4g%%[2], %.4g%%[n], "
                           "Multi=%.4g%%, Rej=%.4g%%, "
                           "Answers=%.3g, Rank=%.3g, "
                           "OKjunk=%.4g%%, Badjunk=%.4g%%";
  int max_str_len = strlen(format_str) + kMaxExtraLength * (CT_SIZE - 1) + 1;
  char* formatted_str = new char[max_str_len];
  snprintf(formatted_str, max_str_len, format_str,
           rates[CT_SHAPE_TOP_ERR] * 100.0,
           rates[CT_FONT_ATTR_ERR] * 100.0,
           rates[CT_UNICHAR_TOP1_ERR] * 100.0,
           rates[CT_UNICHAR_TOP2_ERR] * 100.0,
           rates[CT_UNICHAR_TOPN_ERR] * 100.0,
           rates[CT_OK_MULTI_UNICHAR] * 100.0,
           rates[CT_REJECT] * 100.0,
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
  int ok_samples = counts.n[CT_SHAPE_TOP_CORRECT] + counts.n[CT_SHAPE_TOP_ERR] +
      counts.n[CT_REJECT];
  int junk_samples = counts.n[CT_REJECTED_JUNK] + counts.n[CT_ACCEPTED_JUNK];
  if (ok_samples == 0 && junk_samples == 0) {
    // There is no data.
    return false;
  }
  // Compute rates for normal chars.
  double denominator = static_cast<double>(MAX(ok_samples, 1));
  for (int ct = 0; ct <= CT_RANK; ++ct)
    rates[ct] = counts.n[ct] / denominator;
  // Compute rates for junk.
  denominator = static_cast<double>(MAX(junk_samples, 1));
  for (int ct = CT_REJECTED_JUNK; ct <= CT_ACCEPTED_JUNK; ++ct)
    rates[ct] = counts.n[ct] / denominator;
  return true;
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





