/**********************************************************************
 * File:        tesseractfull.cc
 * Description: Test function to link tesseractfull.a.
 * Author:      Ray Smith
 * Created:     Tue Jul 17 16:23:46 PDT 2007
 *
 * (C) Copyright 2007 Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include "baseapi.h"
#ifndef NULL
#define NULL 0L
#endif

char* run_tesseract(const char* language,
                    const unsigned char* imagedata,
                    int bytes_per_pixel,
                    int bytes_per_line,
                    int width, int height) {
  TessBaseAPI::InitWithLanguage(NULL, NULL, language, NULL, false, 0, NULL);
  char* text = TessBaseAPI::TesseractRect(imagedata, bytes_per_pixel,
                                          bytes_per_line, 0, 0,
                                          width, height);
  TessBaseAPI::End();

  return text;                      //Normal exit
}
