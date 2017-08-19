///////////////////////////////////////////////////////////////////////
// File:        apiexample.cpp
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
#include "tesseract/baseapi.h"
#include "leptonica/allheaders.h"
#include <iostream>
#include <string>
#include <fstream>
#include <locale>

TEST(TesseractTest, ApiExample) 
{
    const char* imagefile = "../testing/phototest.tif";
    const char* groundtruth = "testfiles/phototest.txt";
    char *outText;
    std::locale loc("en_US.UTF-8"); 
    std::ifstream file(groundtruth);
    file.imbue(loc); 
    std::string gtText((std::istreambuf_iterator<char>(file)),
             std::istreambuf_iterator<char>());
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    if (api->Init(NULL, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }
    Pix *image = pixRead(imagefile);
    api->SetImage(image);
    api->SetPageSegMode(tesseract::PSM_AUTO_OSD);
    outText = api->GetUTF8Text();
    ASSERT_EQ(gtText,outText) << "Phototest.tif with default values OCR does not match ground truth";
    api->End();
    delete [] outText;
    pixDestroy(&image);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}