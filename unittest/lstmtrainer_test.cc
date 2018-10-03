#include "leptonica/include/allheaders.h"
#include "tesseract/api/baseapi.h"
#include "tesseract/unittest/lstm_test.h"

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
  fra_trainer.InitCharSet(TestDataNameToPath("fra.traineddata"));
  LSTMTrainer deu_trainer;
  deu_trainer.InitCharSet(TestDataNameToPath("deu.traineddata"));
  // A string that uses characters common to French and German.
  string kTestStr = "The quick brown 'fox' jumps over: the lazy dog!";
  GenericVector<int> deu_labels;
  EXPECT_TRUE(deu_trainer.EncodeString(kTestStr.c_str(), &deu_labels));
  // The french trainer cannot decode them correctly.
  STRING badly_decoded = fra_trainer.DecodeLabels(deu_labels);
  string bad_str(&badly_decoded[0], badly_decoded.length());
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
  string ok_str(&decoded[0], decoded.length());
  EXPECT_EQ(kTestStr, ok_str);
}

// Tests that the actual fra model can be converted to the deu character set
// and still read an eng image with 100% accuracy.
TEST_F(LSTMTrainerTest, ConvertModel) {
  // Setup a trainer with a deu charset.
  LSTMTrainer deu_trainer;
  deu_trainer.InitCharSet(TestDataNameToPath("deu.traineddata"));
  // Load the fra traineddata, strip out the model, and save to a tmp file.
  TessdataManager mgr;
  string fra_data =
      file::JoinPath(FLAGS_test_srcdir, "tessdata_best", "fra.traineddata");
  CHECK(mgr.Init(fra_data.c_str())) << "Failed to load " << fra_data;
  string model_path = file::JoinPath(FLAGS_test_tmpdir, "fra.lstm");
  CHECK(mgr.ExtractToFile(model_path.c_str()));
  // Load the fra model into the deu_trainer, and save the converted model.
  CHECK(deu_trainer.TryLoadingCheckpoint(model_path.c_str(), fra_data.c_str()))
      << "Failed checkpoint load for " << model_path << " and " << fra_data;
  string deu_data = file::JoinPath(FLAGS_test_tmpdir, "deu.traineddata");
  CHECK(deu_trainer.SaveTraineddata(deu_data.c_str()));
  // Now run the saved model on phototest. (See BasicTesseractTest in
  // baseapi_test.cc).
  TessBaseAPI api;
  api.Init(FLAGS_test_tmpdir.c_str(), "deu", tesseract::OEM_LSTM_ONLY);
  Pix* src_pix = pixRead(TestDataNameToPath("phototest.tif").c_str());
  CHECK(src_pix);
  api.SetImage(src_pix);
  std::unique_ptr<char[]> result(api.GetUTF8Text());
  string truth_text;
  CHECK_OK(file::GetContents(TestDataNameToPath("phototest.gold.txt"),
                             &truth_text, file::Defaults()));

  EXPECT_STREQ(truth_text.c_str(), result.get());
  pixDestroy(&src_pix);
}

}  // namespace
}  // namespace tesseract
