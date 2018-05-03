///////////////////////////////////////////////////////////////////////
// File:        osd_example.cc
// Description: OSD example for Tesseract 4.0.0.
// Author:      https://gist.github.com/amitdo/7c7a522004dd79b398340c9595b377e1
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

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

int main() {
  const char* inputfile = TESTING_DIR "/eurotext.tif";
  Pix* image = pixRead(inputfile);
  if (!image) {
      fprintf(stderr, "Cannot open input file: %s\n", inputfile);
      return EXIT_FAILURE;
  }
  
  auto api = new tesseract::TessBaseAPI();
  const char* datapath = nullptr;
  const char* lang = "osd";
  api->Init(datapath, lang);
  api->SetImage(image);
      
  int orient_deg;
  float orient_conf;
  const char* script_name;
  float script_conf;
  
  bool detected = api->DetectOrientationScript(&orient_deg, &orient_conf,
                                               &script_name, &script_conf);
  if (!detected)
    return EXIT_FAILURE;
  
  printf("Orientation in degrees: %d\n"
         "Orientation confidence: %.2f\n"
         "Script: %s\n"
         "Script confidence: %.2f\n",
         orient_deg, orient_conf,
         script_name, script_conf);
  
  return EXIT_SUCCESS;
}
