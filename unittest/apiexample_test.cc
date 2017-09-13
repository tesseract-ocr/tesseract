///////////////////////////////////////////////////////////////////////
// File:        apiexample_test.cc
// Description: Api Example for Tesseract.
// Author:      ShreeDevi Kumar
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
///////////////////////////////////////////////////////////////////////
#include "gtest/gtest.h"
#include "baseapi.h"
#include "leptonica/allheaders.h"
#include <iostream>
#include <string>
#include <fstream>
#include <locale>

TEST(TesseractTest, ApiExample) 
{
    char *outText;
    std::locale loc("C"); // You can also use "" for the default system locale
    std::ifstream file("../testing/phototest.txt");
    file.imbue(loc); // Use it for file input
    std::string gtText((std::istreambuf_iterator<char>(file)),
             std::istreambuf_iterator<char>());
	
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    ASSERT_FALSE(api->Init(nullptr, "eng")) << "Could not initialize tesseract.";

    // Open input image with leptonica library
    Pix *image = pixRead("../testing/phototest.tif");
    ASSERT_TRUE(image != nullptr) << "Failed to read test image.";
    api->SetImage(image);
    // Get OCR result
    outText = api->GetUTF8Text();

    ASSERT_EQ(gtText,outText) << "Phototest.tif with default values OCR does not match ground truth";

    // Destroy used object and release memory
    api->End();
    delete [] outText;
    pixDestroy(&image);

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
