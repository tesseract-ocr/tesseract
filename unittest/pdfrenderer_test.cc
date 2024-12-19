// (C) Copyright 2023, Tesseract Contributors.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <allheaders.h>
#include <tesseract/renderer.h>
#include <filesystem>
#include <string>

#include "include_gunit.h"

namespace tesseract {

static std::map<std::string, std::string> userdefined_dpi_variables = {
    {"user_defined_dpi", "300"}};

class TessPDFRendererTest : public testing::Test {
protected:
  static std::string TestDataNameToPath(const std::string &name) {
    return file::JoinPath(TESTING_DIR, name);
  }
  static std::string TessdataPath() {
    return TESSDATA_DIR;
  }
  static std::string TestPDFName(const std::string &suffix) {
    return "/tmp/tesseract_pdf_renderer_test_phottest" + suffix;
  }

  static void AssertPDFSizeLT(const std::string &filename, int size) {
    std::filesystem::path p = filename + ".pdf";
    ASSERT_LT(std::filesystem::file_size(p), size);
  }

  static void AssertPDFRemove(const std::string &filename) {
    ASSERT_EQ(std::remove((filename + ".pdf").c_str()), 0);
  }

  static bool initializeAPI(
      TessBaseAPI &api, const std::map<std::string, std::string> &variables) {
    EXPECT_EQ(api.Init(TESSDATA_DIR, "eng", OEM_LSTM_ONLY), 0);
    for (const auto &[name, value] : variables) {
      api.SetVariable(name.c_str(), value.c_str());
    }
    return true;
  }

  static bool ProcessAndRenderPages(
      const std::string &input_filename, TessPDFRenderer *pdf_renderer,
      const std::map<std::string, std::string> &variables) {
    TessBaseAPI api;
    initializeAPI(api, variables);
    auto testdata_input_filename = TestDataNameToPath(input_filename);
    EXPECT_TRUE(api.ProcessPages(testdata_input_filename.c_str(), TESSDATA_DIR,
                                 1000, pdf_renderer));
    api.End();
    return pdf_renderer->happy();
  }

  static void RenderPDFAndAssertSize(
      const std::string &image_file, const std::string &pdf_suffix,
      bool text_only, int max_file_size,
      const std::map<std::string, std::string> &variables = {}) {
    auto pdf_name = TestPDFName(pdf_suffix);
    auto pdf_renderer = std::make_unique<TessPDFRenderer>(
        pdf_name.c_str(), "tessdata", text_only);
    ASSERT_TRUE(
        ProcessAndRenderPages(image_file, pdf_renderer.get(), variables));
    AssertPDFSizeLT(pdf_name, max_file_size);
    AssertPDFRemove(pdf_name);
  }
};

// Test basic pdf rendering
TEST_F(TessPDFRendererTest, TestPDFRenderBasicTest) {
  RenderPDFAndAssertSize("phototest_2.tif", "", false, 113000);
}

// Test pdf rendering with lower jpeg quality
TEST_F(TessPDFRendererTest, TestPDFRenderJPEGQualityTest) {
  static std::map<std::string, std::string> variables = {{"jpg_quality", "40"}};
  RenderPDFAndAssertSize("phototest_2.tif", "jpg_quality", false, 66000,
                         variables);
}

// Test pdf renderer text only
TEST_F(TessPDFRendererTest, TestPDFRenderTextOnlyTest) {
  RenderPDFAndAssertSize("phototest_2.tif", "text_only", true, 3500);
}

// Test that pdf renderer generates a custom image resolution in the pdf export
TEST_F(TessPDFRendererTest, TestPDFRenderLowerResolutionTest) {
  std::string pdf_name = TestPDFName("lower_resolution");
  auto pdf_renderer =
      std::make_unique<TessPDFRenderer>(pdf_name.c_str(), "tessdata", false);
  pdf_renderer->SetRenderingResolution(110);
  CHECK_OK(ProcessAndRenderPages("phototest_2.tif", pdf_renderer.get(),
                                 userdefined_dpi_variables));
  AssertPDFSizeLT(pdf_name, 35000);
  AssertPDFRemove(pdf_name);
}

// Test that pdf renderer generates a custom image resolution in the pdf export
// with variable directive
TEST_F(TessPDFRendererTest, TestPDFLowerResolutionVariableTest) {
  std::string pdf_name = TestPDFName("lower_resolution_variable");
  static std::map<std::string, std::string> variables = {
      {"rendering_dpi", "110"}};
  variables.insert(begin(userdefined_dpi_variables),
                   end(userdefined_dpi_variables));
  auto pdf_renderer =
      std::make_unique<TessPDFRenderer>(pdf_name.c_str(), "tessdata", false);
  CHECK_OK(
      ProcessAndRenderPages("phototest_2.tif", pdf_renderer.get(), variables));
  AssertPDFSizeLT(pdf_name, 35000);
  AssertPDFRemove(pdf_name);
}

// Test that pdf renderer generates an alternate image in the pdf export
TEST_F(TessPDFRendererTest, TestPDFAlternateImageTest) {
  std::string pdf_name = TestPDFName("alternate_image");
  auto pdf_renderer =
      std::make_unique<TessPDFRenderer>(pdf_name.c_str(), "tessdata", false);
  auto alternate_image = pixRead(TestDataNameToPath("phototest.tif").c_str());
  pdf_renderer->SetRenderingImage(alternate_image);
  CHECK_OK(ProcessAndRenderPages("phototest_2.tif", pdf_renderer.get(),
                                 std::map<std::string, std::string>()));
  pixDestroy(&alternate_image);
  AssertPDFSizeLT(pdf_name, 8000);
  AssertPDFRemove(pdf_name);
}

} // namespace tesseract