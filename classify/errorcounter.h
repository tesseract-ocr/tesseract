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

struct Pix;
template <typename T> class UnicityTable;

namespace tesseract {

struct FontInfo;
class SampleIterator;
class ShapeClassifier;
class ShapeRating;
class ShapeTable;
class TrainingSample;

// Enumeration of the different types of error count.
// Error counts work as follows:
//
// Ground truth is a valid unichar-id / font-id pair:
//        Number of classifier answers?
//          0                       >0
//     CT_REJECT     BOTH unichar-id and font-id match top shape?
//     __________             yes!              no
//                   CT_SHAPE_TOP_CORRECT  CT_SHAPE_TOP_ERR
//                           |            Font attributes match?
//                           |               yes!        no
//                           |                 |     CT_FONT_ATTR_ERROR
//                           |         Top unichar-id matches?
//                           |         yes!          no
//       Top shape-id has multiple unichars?    CT_UNICHAR_TOP1_ERR
//               yes!            no           2nd shape unichar id matches?
//        CT_OK_MULTI_UNICHAR   ________        yes!              no
//        ___________________                  _____  CT_UNICHAR_TOP2_ERR
//                                                    Any unichar-id matches?
//                                                    yes!        no
//                                                   ______ CT_UNICHAR_TOPN_ERR
//                                                           _________________
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
  CT_SHAPE_TOP_CORRECT,  // Top shape id is actually correct.
  CT_SHAPE_TOP_ERR,      // Top shape id is not correct.
  CT_FONT_ATTR_ERR,      // Font attributes incorrect, ignoring unichar.
  CT_UNICHAR_TOP1_ERR,   // Top shape does not contain correct unichar id.
  CT_UNICHAR_TOP2_ERR,   // Top 2 shapes don't contain correct unichar id.
  CT_UNICHAR_TOPN_ERR,   // No output shape contains correct unichar id.
  CT_OK_MULTI_UNICHAR,   // Top shape id has correct unichar id, and others.
  CT_REJECT,             // Classifier hates this.
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
  // * The page_images vector may contain a Pix* (which may be NULL) for each
  //   page index assigned to the samples.
  // * The it provides encapsulated iteration over some sample set.
  // * The outputs unichar_error, scaled_error and totals_report are all
  //   optional.
  // * If not NULL, unichar error gets the top1 unichar error rate.
  // * Scaled_error gets the error chosen by boosting_mode weighted by the
  //   weights on the samples.
  // * Fonts_report gets a string summarizing the error rates for each font in
  //   both human-readable form and as a tab-separated list of error counts.
  //   The human-readable form is all before the first tab.
  // * The return value is the un-weighted version of the scaled_error.
  static double ComputeErrorRate(ShapeClassifier* classifier,
                                 int report_level, CountTypes boosting_mode,
                                 const UnicityTable<FontInfo>& fontinfo_table,
                                 const GenericVector<Pix*>& page_images,
                                 SampleIterator* it,
                                 double* unichar_error,
                                 double* scaled_error,
                                 STRING* fonts_report);

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
  ErrorCounter(int charsetsize, int shapesize, int fontsize);
  ~ErrorCounter();

  // Accumulates the errors from the classifier results on a single sample.
  // Returns true if debug is true and a CT_UNICHAR_TOPN_ERR error occurred.
  // boosting_mode selects the type of error to be used for boosting and the
  // is_error_ member of sample is set according to whether the required type
  // of error occurred. The font_table provides access to font properties
  // for error counting and shape_table is used to understand the relationship
  // between unichar_ids and shape_ids in the results
  bool AccumulateErrors(bool debug, CountTypes boosting_mode,
                        const UnicityTable<FontInfo>& font_table,
                        const ShapeTable& shape_table,
                        const GenericVector<ShapeRating>& results,
                        TrainingSample* sample);

  // Accumulates counts for junk. Counts only whether the junk was correctly
  // rejected or not.
  void AccumulateJunk(const ShapeTable& shape_table,
                      const GenericVector<ShapeRating>& results,
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
  // If not NULL, the top-choice unichar error rate is saved in unichar_error.
  // If not NULL, the report string is saved in fonts_report.
  // (Ignoring report_level).
  double ReportErrors(int report_level, CountTypes boosting_mode,
                      const UnicityTable<FontInfo>& fontinfo_table,
                      const SampleIterator& it,
                      double* unichar_error,
                      STRING* fonts_report);

  // Sets the report string to a combined human and machine-readable report
  // string of the error rates.
  // Returns false if there is no data, leaving report unchanged.
  static bool ReportString(const Counts& counts, STRING* report);

  // Computes the error rates and returns in rates which is an array of size
  // CT_SIZE. Returns false if there is no data, leaving rates unchanged.
  static bool ComputeRates(const Counts& counts, double rates[CT_SIZE]);


  // Total scaled error used by boosting algorithms.
  double scaled_error_;
  // Vector indexed by font_id from the samples of error accumulators.
  GenericVector<Counts> font_counts_;
  // Counts of the results that map each unichar_id (from samples) to an
  // incorrect shape_id.
  GENERIC_2D_ARRAY<int> unichar_counts_;
};

}  // namespace tesseract.

#endif /* THIRD_PARTY_TESSERACT_CLASSIFY_ERRORCOUNTER_H_ */
