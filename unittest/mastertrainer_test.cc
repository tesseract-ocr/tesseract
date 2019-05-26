// (C) Copyright 2017, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Although this is a trivial-looking test, it exercises a lot of code:
// SampleIterator has to correctly iterate over the correct characters, or
// it will fail.
// The canonical and cloud features computed by TrainingSampleSet need to
// be correct, along with the distance caches, organizing samples by font
// and class, indexing of features, distance calculations.
// IntFeatureDist has to work, or the canonical samples won't work.
// Mastertrainer has ability to read tr files and set itself up tested.
// Finally the serialize/deserialize test ensures that MasterTrainer,
// TrainingSampleSet, TrainingSample can all serialize/deserialize correctly
// enough to reproduce the same results.

#include <string>
#include <utility>
#include <vector>

#include "absl/strings/numbers.h"       // for safe_strto32
#include "absl/strings/str_split.h"     // for absl::StrSplit

#include "include_gunit.h"

#include "genericvector.h"
#include "log.h"                        // for LOG
#include "unicharset.h"
#include "errorcounter.h"
#include "mastertrainer.h"
#include "shapeclassifier.h"
#include "shapetable.h"
#include "trainingsample.h"
#include "commontraining.h"
#include "tessopt.h"                    // tessoptind

// Specs of the MockClassifier.
static const int kNumTopNErrs = 10;
static const int kNumTop2Errs = kNumTopNErrs + 20;
static const int kNumTop1Errs = kNumTop2Errs + 30;
static const int kNumTopTopErrs = kNumTop1Errs + 25;
static const int kNumNonReject = 1000;
static const int kNumCorrect = kNumNonReject - kNumTop1Errs;
// The total number of answers is given by the number of non-rejects plus
// all the multiple answers.
static const int kNumAnswers = kNumNonReject + 2 * (kNumTop2Errs - kNumTopNErrs) +
                        (kNumTop1Errs - kNumTop2Errs) +
                        (kNumTopTopErrs - kNumTop1Errs);

#ifndef DISABLED_LEGACY_ENGINE
static bool safe_strto32(const std::string& str, int* pResult)
{
  long n = strtol(str.c_str(), nullptr, 0);
  *pResult = n;
  return true;
}
#endif

namespace tesseract {

// Mock ShapeClassifier that cheats by looking at the correct answer, and
// creates a specific pattern of errors that can be tested.
class MockClassifier : public ShapeClassifier {
 public:
  explicit MockClassifier(ShapeTable* shape_table)
      : shape_table_(shape_table), num_done_(0), done_bad_font_(false) {
    // Add a false font answer to the shape table. We pick a random unichar_id,
    // add a new shape for it with a false font. Font must actually exist in
    // the font table, but not match anything in the first 1000 samples.
    false_unichar_id_ = 67;
    false_shape_ = shape_table_->AddShape(false_unichar_id_, 25);
  }
  virtual ~MockClassifier() {}

  // Classifies the given [training] sample, writing to results.
  // If debug is non-zero, then various degrees of classifier dependent debug
  // information is provided.
  // If keep_this (a shape index) is >= 0, then the results should always
  // contain keep_this, and (if possible) anything of intermediate confidence.
  // The return value is the number of classes saved in results.
  virtual int ClassifySample(const TrainingSample& sample, Pix* page_pix,
                             int debug, UNICHAR_ID keep_this,
                             GenericVector<ShapeRating>* results) {
    results->clear();
    // Everything except the first kNumNonReject is a reject.
    if (++num_done_ > kNumNonReject) return 0;

    int class_id = sample.class_id();
    int font_id = sample.font_id();
    int shape_id = shape_table_->FindShape(class_id, font_id);
    // Get ids of some wrong answers.
    int wrong_id1 = shape_id > 10 ? shape_id - 1 : shape_id + 1;
    int wrong_id2 = shape_id > 10 ? shape_id - 2 : shape_id + 2;
    if (num_done_ <= kNumTopNErrs) {
      // The first kNumTopNErrs are top-n errors.
      results->push_back(ShapeRating(wrong_id1, 1.0f));
    } else if (num_done_ <= kNumTop2Errs) {
      // The next kNumTop2Errs - kNumTopNErrs are top-2 errors.
      results->push_back(ShapeRating(wrong_id1, 1.0f));
      results->push_back(ShapeRating(wrong_id2, 0.875f));
      results->push_back(ShapeRating(shape_id, 0.75f));
    } else if (num_done_ <= kNumTop1Errs) {
      // The next kNumTop1Errs - kNumTop2Errs are top-1 errors.
      results->push_back(ShapeRating(wrong_id1, 1.0f));
      results->push_back(ShapeRating(shape_id, 0.8f));
    } else if (num_done_ <= kNumTopTopErrs) {
      // The next kNumTopTopErrs - kNumTop1Errs are cases where the actual top
      // is not correct, but do not count as a top-1 error because the rating
      // is close enough to the top answer.
      results->push_back(ShapeRating(wrong_id1, 1.0f));
      results->push_back(ShapeRating(shape_id, 0.99f));
    } else if (!done_bad_font_ && class_id == false_unichar_id_) {
      // There is a single character with a bad font.
      results->push_back(ShapeRating(false_shape_, 1.0f));
      done_bad_font_ = true;
    } else {
      // Everything else is correct.
      results->push_back(ShapeRating(shape_id, 1.0f));
    }
    return results->size();
  }
  // Provides access to the ShapeTable that this classifier works with.
  virtual const ShapeTable* GetShapeTable() const { return shape_table_; }

 private:
  // Borrowed pointer to the ShapeTable.
  ShapeTable* shape_table_;
  // Unichar_id of a random character that occurs after the first 60 samples.
  int false_unichar_id_;
  // Shape index of prepared false answer for false_unichar_id.
  int false_shape_;
  // The number of classifications we have processed.
  int num_done_;
  // True after the false font has been emitted.
  bool done_bad_font_;
};

}  // namespace tesseract

namespace {

using tesseract::MasterTrainer;
using tesseract::Shape;
using tesseract::ShapeTable;
using tesseract::UnicharAndFonts;

const double kMin1lDistance = 0.25;

// The fixture for testing Tesseract.
class MasterTrainerTest : public testing::Test {
#ifndef DISABLED_LEGACY_ENGINE
 protected:
  void SetUp() {
    std::locale::global(std::locale(""));
  }

  std::string TestDataNameToPath(const std::string& name) {
    return file::JoinPath(TESTING_DIR, name);
  }
  std::string TmpNameToPath(const std::string& name) {
    return file::JoinPath(FLAGS_test_tmpdir, name);
  }

  MasterTrainerTest() {
    shape_table_ = nullptr;
    master_trainer_ = nullptr;
  }
  ~MasterTrainerTest() {
    delete master_trainer_;
    delete shape_table_;
  }

  // Initializes the master_trainer_ and shape_table_.
  // if load_from_tmp, then reloads a master trainer that was saved by a
  // previous call in which it was false.
  void LoadMasterTrainer() {
    FLAGS_output_trainer = TmpNameToPath("tmp_trainer").c_str();
    FLAGS_F = file::JoinPath(LANGDATA_DIR, "font_properties").c_str();
    FLAGS_X = TestDataNameToPath("eng.xheights").c_str();
    FLAGS_U = TestDataNameToPath("eng.unicharset").c_str();
    std::string tr_file_name(TestDataNameToPath("eng.Arial.exp0.tr"));
    const char* argv[] = {tr_file_name.c_str()};
    int argc = 1;
    STRING file_prefix;
    delete master_trainer_;
    delete shape_table_;
    shape_table_ = nullptr;
    tessoptind = 0;
    master_trainer_ =
        LoadTrainingData(argc, argv, false, &shape_table_, &file_prefix);
    EXPECT_TRUE(master_trainer_ != nullptr);
    EXPECT_TRUE(shape_table_ != nullptr);
  }

  // EXPECTs that the distance between I and l in Arial is 0 and that the
  // distance to 1 is significantly not 0.
  void VerifyIl1() {
    // Find the font id for Arial.
    int font_id = master_trainer_->GetFontInfoId("Arial");
    EXPECT_GE(font_id, 0);
    // Track down the characters we are interested in.
    int unichar_I = master_trainer_->unicharset().unichar_to_id("I");
    EXPECT_GT(unichar_I, 0);
    int unichar_l = master_trainer_->unicharset().unichar_to_id("l");
    EXPECT_GT(unichar_l, 0);
    int unichar_1 = master_trainer_->unicharset().unichar_to_id("1");
    EXPECT_GT(unichar_1, 0);
    // Now get the shape ids.
    int shape_I = shape_table_->FindShape(unichar_I, font_id);
    EXPECT_GE(shape_I, 0);
    int shape_l = shape_table_->FindShape(unichar_l, font_id);
    EXPECT_GE(shape_l, 0);
    int shape_1 = shape_table_->FindShape(unichar_1, font_id);
    EXPECT_GE(shape_1, 0);

    float dist_I_l =
        master_trainer_->ShapeDistance(*shape_table_, shape_I, shape_l);
    // No tolerance here. We expect that I and l should match exactly.
    EXPECT_EQ(0.0f, dist_I_l);
    float dist_l_I =
        master_trainer_->ShapeDistance(*shape_table_, shape_l, shape_I);
    // BOTH ways.
    EXPECT_EQ(0.0f, dist_l_I);

    // l/1 on the other hand should be distinct.
    float dist_l_1 =
        master_trainer_->ShapeDistance(*shape_table_, shape_l, shape_1);
    EXPECT_GT(dist_l_1, kMin1lDistance);
    float dist_1_l =
        master_trainer_->ShapeDistance(*shape_table_, shape_1, shape_l);
    EXPECT_GT(dist_1_l, kMin1lDistance);

    // So should I/1.
    float dist_I_1 =
        master_trainer_->ShapeDistance(*shape_table_, shape_I, shape_1);
    EXPECT_GT(dist_I_1, kMin1lDistance);
    float dist_1_I =
        master_trainer_->ShapeDistance(*shape_table_, shape_1, shape_I);
    EXPECT_GT(dist_1_I, kMin1lDistance);
  }

  // Objects declared here can be used by all tests in the test case for Foo.
  ShapeTable* shape_table_;
  MasterTrainer* master_trainer_;
#endif
};

// Tests that the MasterTrainer correctly loads its data and reaches the correct
// conclusion over the distance between Arial I l and 1.
TEST_F(MasterTrainerTest, Il1Test) {
#ifdef DISABLED_LEGACY_ENGINE
  // Skip test because LoadTrainingData is missing.
  GTEST_SKIP();
#else
  // Initialize the master_trainer_ and load the Arial tr file.
  LoadMasterTrainer();
  VerifyIl1();
#endif
}

// Tests the ErrorCounter using a MockClassifier to check that it counts
// error categories correctly.
TEST_F(MasterTrainerTest, ErrorCounterTest) {
#ifdef DISABLED_LEGACY_ENGINE
  // Skip test because LoadTrainingData is missing.
  GTEST_SKIP();
#else
  // Initialize the master_trainer_ from the saved tmp file.
  LoadMasterTrainer();
  // Add the space character to the shape_table_ if not already present to
  // count junk.
  if (shape_table_->FindShape(0, -1) < 0) shape_table_->AddShape(0, 0);
  // Make a mock classifier.
  tesseract::ShapeClassifier* shape_classifier =
      new tesseract::MockClassifier(shape_table_);
  // Get the accuracy report.
  STRING accuracy_report;
  master_trainer_->TestClassifierOnSamples(tesseract::CT_UNICHAR_TOP1_ERR, 0,
                                           false, shape_classifier,
                                           &accuracy_report);
  LOG(INFO) << accuracy_report.string();
  std::string result_string = accuracy_report.string();
  std::vector<std::string> results =
      absl::StrSplit(result_string, '\t', absl::SkipEmpty());
  EXPECT_EQ(tesseract::CT_SIZE + 1, results.size());
  int result_values[tesseract::CT_SIZE];
  for (int i = 0; i < tesseract::CT_SIZE; ++i) {
    EXPECT_TRUE(safe_strto32(results[i + 1], &result_values[i]));
  }
  // These tests are more-or-less immune to additions to the number of
  // categories or changes in the training data.
  int num_samples = master_trainer_->GetSamples()->num_raw_samples();
  EXPECT_EQ(kNumCorrect, result_values[tesseract::CT_UNICHAR_TOP_OK]);
  EXPECT_EQ(1, result_values[tesseract::CT_FONT_ATTR_ERR]);
  EXPECT_EQ(kNumTopTopErrs, result_values[tesseract::CT_UNICHAR_TOPTOP_ERR]);
  EXPECT_EQ(kNumTop1Errs, result_values[tesseract::CT_UNICHAR_TOP1_ERR]);
  EXPECT_EQ(kNumTop2Errs, result_values[tesseract::CT_UNICHAR_TOP2_ERR]);
  EXPECT_EQ(kNumTopNErrs, result_values[tesseract::CT_UNICHAR_TOPN_ERR]);
  // Each of the TOPTOP errs also counts as a multi-unichar.
  EXPECT_EQ(kNumTopTopErrs - kNumTop1Errs,
            result_values[tesseract::CT_OK_MULTI_UNICHAR]);
  EXPECT_EQ(num_samples - kNumNonReject, result_values[tesseract::CT_REJECT]);
  EXPECT_EQ(kNumAnswers, result_values[tesseract::CT_NUM_RESULTS]);

  delete shape_classifier;
#endif
}

}  // namespace.
