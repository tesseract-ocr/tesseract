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

#ifndef THIRD_PARTY_TESSERACT_CLASSIFY_ERRORCOUNTER_H_
#define THIRD_PARTY_TESSERACT_CLASSIFY_ERRORCOUNTER_H_

#include "genericvector.h"
#include "matrix.h"
#include "statistc.h"

struct Pix;
template <typename T> class UnicityTable;

namespace tesseract {

struct FontInfo;
class FontInfoTable;
class SampleIterator;
class ShapeClassifier;
class TrainingSample;
struct UnicharRating;

// Enumeration of the different types of error count.
// Error counts work as follows:
//
// Ground truth is a valid unichar-id / font-id pair:
//        Number of classifier answers?
//          0                       >0
//     CT_REJECT          unichar-id matches top shape?
//     __________             yes!                      no
//                   CT_UNICHAR_TOP_OK           CT_UNICHAR_TOP1_ERR
//      Top shape-id has multiple unichars?   2nd shape unichar id matches?
//            yes!              no              yes!              no
//      CT_OK_MULTI_UNICHAR     |              _____    CT_UNICHAR_TOP2_ERR
//             Font attributes match?                 Any unichar-id matches?
//              yes!              no                  yes!        no
//      CT_FONT_ATTR_OK   CT_FONT_ATTR_ERR          ______  CT_UNICHAR_TOPN_ERR
//                |       __________________                 _________________
//      Top shape-id has multiple font attrs?
//            yes!              no
//      CT_OK_MULTI_FONT
//      _____________________________
//
// Note that multiple counts may be activated for a single sample!
//
// Ground truth is for a fragment/n-gram that is NOT in the unicharset.
// This is called junk and is expected to be rejected:
//        Number of classifier answers?
//          0                       >0
//     CT_REJECTED_JUNK     CT_ACCEPTED_JUNK
//
// Also, CT_NUM_RESULTS stores the mean number of results, and CT_RANK stores
// the mean rank of the correct result, counting from 0, and with an error
// receiving the number of answers as the correct rank.
//
// Keep in sync with the ReportString function.
enum CountTypes {
  CT_UNICHAR_TOP_OK,     // Top shape contains correct unichar id.
  // The rank of the results in TOP1, TOP2, TOPN is determined by a gap of
  // kRatingEpsilon from the first result in each group. The real top choice
  // is measured using TOPTOP.
  CT_UNICHAR_TOP1_ERR,   // Top shape does not contain correct unichar id.
  CT_UNICHAR_TOP2_ERR,   // Top 2 shapes don't contain correct unichar id.
  CT_UNICHAR_TOPN_ERR,   // No output shape contains correct unichar id.
  CT_UNICHAR_TOPTOP_ERR,   // Very top choice not correct.
  CT_OK_MULTI_UNICHAR,   // Top shape id has correct unichar id, and others.
  CT_OK_JOINED,          // Top shape id is correct but marked joined.
  CT_OK_BROKEN,          // Top shape id is correct but marked broken.
  CT_REJECT,             // Classifier hates this.
  CT_FONT_ATTR_ERR,      // Top unichar OK, but font attributes incorrect.
  CT_OK_MULTI_FONT,      // CT_FONT_ATTR_OK but there are multiple font attrs.
  CT_NUM_RESULTS,        // Number of answers produced.
  CT_RANK,               // Rank of correct answer.
  CT_REJECTED_JUNK,      // Junk that was correctly rejected.
  CT_ACCEPTED_JUNK,      // Junk that was incorrectly classified otherwise.

  CT_SIZE                // Number of types for array sizing.
};

// Class to encapsulate all the functionality and sub-structures required
// to count errors for an isolated character classifier (ShapeClassifier).
class ErrorCounter {
 public:
  // Computes and returns the unweighted boosting_mode error rate of the given
  // classifier. Can be used for testing, or inside an iterative training
  // system, including one that uses boosting.
  // report_levels:
  // 0 = no output.
  // 1 = bottom-line error rate.
  // 2 = bottom-line error rate + time.
  // 3 = font-level error rate + time.
  // 4 = list of all errors + short classifier debug output on 16 errors.
  // 5 = list of all errors + short classifier debug output on 25 errors.
  // * The boosting_mode determines which error type is used for computing the
  //   scaled_error output, and setting the is_error flag in the samples.
  // * The fontinfo_table is used to get string font names for the debug
  //   output, and also to count font attributes errors.
  // * The page_images vector may contain a Pix* (which may be nullptr) for each
  //   page index assigned to the samples.
  // * The it provides encapsulated iteration over some sample set.
  // * The outputs unichar_error, scaled_error and totals_report are all
  //   optional.
  // * If not nullptr, unichar error gets the top1 unichar error rate.
  // * Scaled_error gets the error chosen by boosting_mode weighted by the
  //   weights on the samples.
  // * Fonts_report gets a string summarizing the error rates for each font in
  //   both human-readable form and as a tab-separated list of error counts.
  //   The human-readable form is all before the first tab.
  // * The return value is the un-weighted version of the scaled_error.
  static double ComputeErrorRate(ShapeClassifier* classifier,
                                 int report_level, CountTypes boosting_mode,
                                 const FontInfoTable& fontinfo_table,
                                 const GenericVector<Pix*>& page_images,
                                 SampleIterator* it,
                                 double* unichar_error,
                                 double* scaled_error,
                                 STRING* fonts_report);
  // Tests a pair of classifiers, debugging errors of the new against the old.
  // See errorcounter.h for description of arguments.
  // Iterates over the samples, calling the classifiers in normal/silent mode.
  // If the new_classifier makes a boosting_mode error that the old_classifier
  // does not, and the appropriate, it will then call the new_classifier again
  // with a debug flag and a keep_this argument to find out what is going on.
  static void DebugNewErrors(ShapeClassifier* new_classifier,
                             ShapeClassifier* old_classifier,
                             CountTypes boosting_mode,
                             const FontInfoTable& fontinfo_table,
                             const GenericVector<Pix*>& page_images,
                             SampleIterator* it);

 private:
  // Simple struct to hold an array of counts.
  struct Counts {
    Counts();
    // Adds other into this for computing totals.
    void operator+=(const Counts& other);

    int n[CT_SIZE];
  };

  // Constructor is private. Only anticipated use of ErrorCounter is via
  // the static ComputeErrorRate.
  ErrorCounter(const UNICHARSET& unicharset, int fontsize);
  ~ErrorCounter() = default;

  // Accumulates the errors from the classifier results on a single sample.
  // Returns true if debug is true and a CT_UNICHAR_TOPN_ERR error occurred.
  // boosting_mode selects the type of error to be used for boosting and the
  // is_error_ member of sample is set according to whether the required type
  // of error occurred. The font_table provides access to font properties
  // for error counting and shape_table is used to understand the relationship
  // between unichar_ids and shape_ids in the results
  bool AccumulateErrors(bool debug, CountTypes boosting_mode,
                        const FontInfoTable& font_table,
                        const GenericVector<UnicharRating>& results,
                        TrainingSample* sample);

  // Accumulates counts for junk. Counts only whether the junk was correctly
  // rejected or not.
  bool AccumulateJunk(bool debug, const GenericVector<UnicharRating>& results,
                      TrainingSample* sample);

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
  double ReportErrors(int report_level, CountTypes boosting_mode,
                      const FontInfoTable& fontinfo_table,
                      const SampleIterator& it,
                      double* unichar_error,
                      STRING* fonts_report);

  // Sets the report string to a combined human and machine-readable report
  // string of the error rates.
  // Returns false if there is no data, leaving report unchanged, unless
  // even_if_empty is true.
  static bool ReportString(bool even_if_empty, const Counts& counts,
                           STRING* report);

  // Computes the error rates and returns in rates which is an array of size
  // CT_SIZE. Returns false if there is no data, leaving rates unchanged.
  static bool ComputeRates(const Counts& counts, double rates[CT_SIZE]);


  // Total scaled error used by boosting algorithms.
  double scaled_error_;
  // Difference in result rating to be thought of as an "equal" choice.
  double rating_epsilon_;
  // Vector indexed by font_id from the samples of error accumulators.
  GenericVector<Counts> font_counts_;
  // Counts of the results that map each unichar_id (from samples) to an
  // incorrect shape_id.
  GENERIC_2D_ARRAY<int> unichar_counts_;
  // Count of the number of times each shape_id occurs, is correct, and multi-
  // unichar.
  GenericVector<int> multi_unichar_counts_;
  // Histogram of scores (as percent) for correct answers.
  STATS ok_score_hist_;
  // Histogram of scores (as percent) for incorrect answers.
  STATS bad_score_hist_;
  // Unicharset for printing character ids in results.
  const UNICHARSET& unicharset_;
};

}  // namespace tesseract.

#endif /* THIRD_PARTY_TESSERACT_CLASSIFY_ERRORCOUNTER_H_ */
