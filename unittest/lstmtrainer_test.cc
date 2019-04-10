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

#include "allheaders.h"
#include "baseapi.h"
#include "lstm_test.h"

namespace tesseract {
namespace {

TEST_F(LSTMTrainerTest, EncodesEng) {
  TestEncodeDecodeBoth("eng",
                       "The quick brown 'fox' jumps over: the lazy dog!");
}

TEST_F(LSTMTrainerTest, EncodesKan) {
  TestEncodeDecodeBoth("kan", "ಫ್ರಬ್ರವರಿ ತತ್ವಾಂಶಗಳೆಂದರೆ ಮತ್ತು ಜೊತೆಗೆ ಕ್ರಮವನ್ನು");
}

TEST_F(LSTMTrainerTest, EncodesKor) {
  TestEncodeDecodeBoth("kor",
                       "이는 것으로 다시 넣을 수는 있지만 선택의 의미는");
}

TEST_F(LSTMTrainerTest, MapCoder) {
  LSTMTrainer fra_trainer;
  fra_trainer.InitCharSet(TestDataNameToPath("fra/fra.traineddata"));
  LSTMTrainer deu_trainer;
  deu_trainer.InitCharSet(TestDataNameToPath("deu/deu.traineddata"));
  // A string that uses characters common to French and German.
  std::string kTestStr = "The quick brown 'fox' jumps over: the lazy dog!";
  GenericVector<int> deu_labels;
  EXPECT_TRUE(deu_trainer.EncodeString(kTestStr.c_str(), &deu_labels));
  // The french trainer cannot decode them correctly.
  STRING badly_decoded = fra_trainer.DecodeLabels(deu_labels);
  std::string bad_str(&badly_decoded[0], badly_decoded.length());
  LOG(INFO) << "bad_str fra=" << bad_str << "\n";
  EXPECT_NE(kTestStr, bad_str);
  // Encode the string as fra.
  GenericVector<int> fra_labels;
  EXPECT_TRUE(fra_trainer.EncodeString(kTestStr.c_str(), &fra_labels));
  // Use the mapper to compute what the labels are as deu.
  std::vector<int> mapping = fra_trainer.MapRecoder(deu_trainer.GetUnicharset(),
                                                    deu_trainer.GetRecoder());
  GenericVector<int> mapped_fra_labels(fra_labels.size(), -1);
  for (int i = 0; i < fra_labels.size(); ++i) {
    mapped_fra_labels[i] = mapping[fra_labels[i]];
    EXPECT_NE(-1, mapped_fra_labels[i]) << "i=" << i << ", ch=" << kTestStr[i];
    EXPECT_EQ(mapped_fra_labels[i], deu_labels[i])
        << "i=" << i << ", ch=" << kTestStr[i]
        << " has deu label=" << deu_labels[i] << ", but mapped to "
        << mapped_fra_labels[i];
  }
  // The german trainer can now decode them correctly.
  STRING decoded = deu_trainer.DecodeLabels(mapped_fra_labels);
  std::string ok_str(&decoded[0], decoded.length());
  LOG(INFO) << "ok_str deu=" << ok_str << "\n";
  EXPECT_EQ(kTestStr, ok_str);
}

// Tests that the actual fra model can be converted to the deu character set
// and still read an eng image with 100% accuracy.
TEST_F(LSTMTrainerTest, ConvertModel) {
  // Setup a trainer with a deu charset.
  LSTMTrainer deu_trainer;
  deu_trainer.InitCharSet(TestDataNameToPath("deu/deu.traineddata"));
  // Load the fra traineddata, strip out the model, and save to a tmp file.
  TessdataManager mgr;
  std::string fra_data =
      file::JoinPath(TESSDATA_BEST_DIR, "fra.traineddata");
  CHECK(mgr.Init(fra_data.c_str()));
  LOG(INFO) << "Load " << fra_data  << "\n";
  std::string model_path = file::JoinPath(FLAGS_test_tmpdir, "fra.lstm");
  CHECK(mgr.ExtractToFile(model_path.c_str()));
  LOG(INFO) << "Extract " << model_path << "\n";
  // Load the fra model into the deu_trainer, and save the converted model.
  CHECK(deu_trainer.TryLoadingCheckpoint(model_path.c_str(), fra_data.c_str()));
  LOG(INFO) << "Checkpoint load for " << model_path << " and " << fra_data << "\n";
  std::string deu_data = file::JoinPath(FLAGS_test_tmpdir, "deu.traineddata");
  CHECK(deu_trainer.SaveTraineddata(deu_data.c_str()));
  LOG(INFO) << "Save " << deu_data << "\n";
  // Now run the saved model on phototest. (See BasicTesseractTest in
  // baseapi_test.cc).
  TessBaseAPI api;
  api.Init(FLAGS_test_tmpdir, "deu", tesseract::OEM_LSTM_ONLY);
  Pix* src_pix = pixRead(TestingNameToPath("phototest.tif").c_str());
  CHECK(src_pix);
  api.SetImage(src_pix);
  std::unique_ptr<char[]> result(api.GetUTF8Text());
  std::string truth_text;
  CHECK_OK(file::GetContents(TestingNameToPath("phototest.gold.txt"),
                             &truth_text, file::Defaults()));

  EXPECT_STREQ(truth_text.c_str(), result.get());
  pixDestroy(&src_pix);
}

}  // namespace
}  // namespace tesseract
