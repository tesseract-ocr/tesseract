/**********************************************************************
 * File:        cube_utils.h
 * Description: Declaration of the Cube Utilities Class
 * Author:    Ahmad Abdulkader
 * Created:   2008
 *
 *(C) Copyright 2008, Google Inc.
 ** Licensed under the Apache License, Version 2.0(the "License");
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

// The CubeUtils class provides miscellaneous utility and helper functions
// to the rest of the Cube Engine

#ifndef CUBE_UTILS_H
#define CUBE_UTILS_H

#include <vector>
#include <string>

#include "allheaders.h"
#include "const.h"
#include "char_set.h"
#include "char_samp.h"
#include "img.h"

namespace tesseract {
class CubeUtils {
 public:
  CubeUtils();
  ~CubeUtils();

  // Converts a probability value to a cost by getting the -log() of the
  // probability value to a known base
  static int Prob2Cost(double prob_val);
  // Converts a cost to probability by getting the exp(-normalized cost)
  static double Cost2Prob(int cost);
  // Computes the length of a 32-bit char buffer
  static int StrLen(const char_32 *str);
  // Compares two 32-bit char buffers
  static int StrCmp(const char_32 *str1, const char_32 *str2);
  // Duplicates a 32-bit char buffer
  static char_32 *StrDup(const char_32 *str);
  // Creates a CharSamp from an IMAGE and a bounding box
  static CharSamp *CharSampleFromImg(IMAGE *img,
                                     int left, int top, int wid, int hgt);
  // Creates a CharSamp from an Pix and a bounding box
  static CharSamp *CharSampleFromPix(Pix *pix,
                                     int left, int top, int wid, int hgt);
  // Creates an IMAGE from a CharSamp
  static IMAGE *ImageFromCharSample(CharSamp *char_samp);
  // Creates a Pix from a CharSamp
  static Pix *PixFromCharSample(CharSamp *char_samp);
  // read the contents of a file to a string
  static bool ReadFileToString(const string &file_name, string *str);
  // split a string into vectors using any of the specified delimiters
  static void SplitStringUsing(const string &str, const string &delims,
                               vector<string> *str_vec);
  // UTF-8 to UTF-32 convesion functions
  static void UTF8ToUTF32(const char *utf8_str, string_32 *str32);
  static void UTF32ToUTF8(const char_32 *utf32_str, string *str);
  // Returns true if input word has either 1) all-one-case, or 2)
  // first character upper-case, and remaining characters lower-case.
  // If char_set is not NULL, uses tesseract's unicharset functions
  // to determine case properties. Otherwise, uses C-locale-dependent
  // functions, which may be unreliable on non-ASCII characters.
  static bool IsCaseInvariant(const char_32 *str32, CharSet *char_set);
  // Returns char_32 pointer to the lower-case-transformed version of
  // the input string or NULL on error. If char_set is NULL returns NULL.
  // Return array must be freed by caller.
  static char_32 *ToLower(const char_32 *str32, CharSet *char_set);
  // Returns char_32 pointer to the upper-case-transformed version of
  // the input string or NULL on error. If char_set is NULL returns NULL.
  // Return array must be freed by caller.
  static char_32 *ToUpper(const char_32 *str32, CharSet *char_set);
 private:
  static unsigned char *GetImageData(IMAGE *img,
                                     int left, int top, int wid, int hgt);
  static unsigned char *GetImageData(Pix *pix,
                                     int left, int top, int wid, int hgt);
};
}  // namespace tesseract
#endif  // CUBE_UTILS_H
