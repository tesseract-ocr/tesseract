/**********************************************************************
 * File:        cube_utils.cpp
 * Description: Implementation of the Cube Utilities Class
 * Author:    Ahmad Abdulkader
 * Created:   2008
 *
 * (C) Copyright 2008, Google Inc.
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

#include <math.h>
#include <string>
#include <vector>
#include "cube_utils.h"
#include "char_set.h"
#include "unichar.h"

namespace tesseract {
CubeUtils::CubeUtils() {
}

CubeUtils::~CubeUtils() {
}

/**
 * convert a prob to a cost (-ve log prob)
 */
int CubeUtils::Prob2Cost(double prob_val) {
  if (prob_val < MIN_PROB)   {
    return MIN_PROB_COST;
  }
  return static_cast<int>(-log(prob_val) * PROB2COST_SCALE);
}

/**
 * converts a cost to probability
 */
double CubeUtils::Cost2Prob(int cost) {
  return exp(-cost / PROB2COST_SCALE);
}

/**
 * computes the length of a NULL terminated char_32 string
 */
int CubeUtils::StrLen(const char_32 *char_32_ptr) {
  if (char_32_ptr == NULL) {
    return 0;
  }
  int len = -1;
  while (char_32_ptr[++len]);
  return len;
}

/**
 * compares two char_32 strings
 */
int CubeUtils::StrCmp(const char_32 *str1, const char_32 *str2) {
  const char_32 *pch1 = str1;
  const char_32 *pch2 = str2;

  for (; (*pch1) != 0 && (*pch2) != 0; pch1++, pch2++) {
    if ((*pch1) != (*pch2)) {
      return (*pch1) - (*pch2);
    }
  }

  if ((*pch1) == 0) {
    if ((*pch2) == 0) {
      return 0;
    } else {
      return -1;
    }
  } else {
    return 1;
  }
}

/**
 * Duplicates a 32-bit char buffer
 */
char_32 *CubeUtils::StrDup(const char_32 *str32) {
  int len = StrLen(str32);
  char_32 *new_str = new char_32[len + 1];
  if (new_str == NULL) {
    return NULL;
  }
  memcpy(new_str, str32, len * sizeof(*str32));
  new_str[len] = 0;
  return new_str;
}

/**
 * creates a char samp from a specified portion of the image
 */
CharSamp *CubeUtils::CharSampleFromPix(Pix *pix, int left, int top,
                                       int wid, int hgt) {
  // get the raw img data from the image
  unsigned char *temp_buff = GetImageData(pix, left, top, wid, hgt);
  if (temp_buff == NULL) {
    return NULL;
  }

  // create a char samp from temp buffer
  CharSamp *char_samp = CharSamp::FromRawData(left, top, wid, hgt, temp_buff);

  // clean up temp buffer
  delete []temp_buff;
  return char_samp;
}

/**
 * create a B/W image from a char_sample
 */
Pix *CubeUtils::PixFromCharSample(CharSamp *char_samp) {
  // parameter check
  if (char_samp == NULL) {
    return NULL;
  }

  // get the raw data
  int stride = char_samp->Stride();
  int wid = char_samp->Width();
  int hgt = char_samp->Height();

  Pix *pix = pixCreate(wid, hgt, 1);
  if (pix == NULL) {
    return NULL;
  }

  // copy the contents
  unsigned char *line = char_samp->RawData();
  for (int y = 0; y < hgt ; y++, line += stride) {
    for (int x = 0; x < wid; x++) {
      if (line[x] != 0) {
        pixSetPixel(pix, x, y, 0);
      } else {
        pixSetPixel(pix, x, y, 255);
      }
    }
  }

  return pix;
}

/**
 * creates a raw buffer from the specified location of the pix
 */
unsigned char *CubeUtils::GetImageData(Pix *pix, int left, int top,
                                       int wid, int hgt) {
  // skip invalid dimensions
  if (left < 0 || top < 0 || wid < 0 || hgt < 0 ||
      (left + wid) > pix->w || (top + hgt) > pix->h ||
      pix->d != 1) {
    return NULL;
  }

  // copy the char img to a temp buffer
  unsigned char *temp_buff = new unsigned char[wid * hgt];
  if (temp_buff == NULL) {
    return NULL;
  }
  l_int32 w;
  l_int32 h;
  l_int32 d;
  l_int32 wpl;
  l_uint32 *line;
  l_uint32 *data;

  pixGetDimensions(pix, &w, &h, &d);
  wpl = pixGetWpl(pix);
  data = pixGetData(pix);
  line = data + (top * wpl);

  for (int y = 0, off = 0; y < hgt ; y++) {
    for (int x = 0; x < wid; x++, off++) {
      temp_buff[off] = GET_DATA_BIT(line, x + left) ? 0 : 255;
    }
    line += wpl;
  }
  return temp_buff;
}

/**
 * read file contents to a string
 */
bool CubeUtils::ReadFileToString(const string &file_name, string *str) {
  str->clear();
  FILE *fp = fopen(file_name.c_str(), "rb");
  if (fp == NULL) {
    return false;
  }

  // get the size of the size
  fseek(fp, 0, SEEK_END);
  int file_size = ftell(fp);
  if (file_size < 1) {
    fclose(fp);
    return false;
  }
  // adjust string size
  str->reserve(file_size);
  // read the contents
  rewind(fp);
  char *buff = new char[file_size];
  if (buff == NULL) {
    fclose(fp);
    return false;
  }
  int read_bytes = fread(buff, 1, static_cast<int>(file_size), fp);
  if (read_bytes == file_size) {
    str->append(buff, file_size);
  }
  delete []buff;
  fclose(fp);
  return (read_bytes == file_size);
}

/**
 * splits a string into vectors based on specified delimiters
 */
void CubeUtils::SplitStringUsing(const string &str,
                                 const string &delims,
                                 vector<string> *str_vec) {
  // Optimize the common case where delims is a single character.
  if (delims[0] != '\0' && delims[1] == '\0') {
    char c = delims[0];
    const char* p = str.data();
    const char* end = p + str.size();
    while (p != end) {
      if (*p == c) {
        ++p;
      } else {
        const char* start = p;
        while (++p != end && *p != c);
        str_vec->push_back(string(start, p - start));
      }
    }
    return;
  }

  string::size_type begin_index, end_index;
  begin_index = str.find_first_not_of(delims);
  while (begin_index != string::npos) {
    end_index = str.find_first_of(delims, begin_index);
    if (end_index == string::npos) {
      str_vec->push_back(str.substr(begin_index));
      return;
    }
    str_vec->push_back(str.substr(begin_index, (end_index - begin_index)));
    begin_index = str.find_first_not_of(delims, end_index);
  }
}

/**
 * UTF-8 to UTF-32 conversion functions
 */
void CubeUtils::UTF8ToUTF32(const char *utf8_str, string_32 *str32) {
  str32->clear();
  int len = strlen(utf8_str);
  int step = 0;
  for (int ch = 0; ch < len; ch += step) {
    step = UNICHAR::utf8_step(utf8_str + ch);
    if (step > 0) {
      UNICHAR uni_ch(utf8_str + ch, step);
      (*str32) += uni_ch.first_uni();
    }
  }
}

/**
 * UTF-8 to UTF-32 conversion functions
 */
void CubeUtils::UTF32ToUTF8(const char_32 *utf32_str, string *str) {
  str->clear();
  for (const char_32 *ch_32 = utf32_str; (*ch_32) != 0; ch_32++)  {
    UNICHAR uni_ch((*ch_32));
    char *utf8 = uni_ch.utf8_str();
    if (utf8 != NULL) {
      (*str) += utf8;
      delete []utf8;
    }
  }
}

bool CubeUtils::IsCaseInvariant(const char_32 *str32, CharSet *char_set) {
  bool all_one_case = true;
  bool capitalized;
  bool prev_upper;
  bool prev_lower;
  bool first_upper;
  bool first_lower;
  bool cur_upper;
  bool cur_lower;

  string str8;
  if (!char_set) {
    // If cube char_set is missing, use C-locale-dependent functions
    // on UTF8 characters to determine case properties.
    first_upper = isupper(str32[0]);
    first_lower = islower(str32[0]);
    if (first_upper)
      capitalized = true;
    prev_upper = first_upper;
    prev_lower = islower(str32[0]);
    for (int c = 1; str32[c] != 0; ++c) {
      cur_upper = isupper(str32[c]);
      cur_lower = islower(str32[c]);
      if ((prev_upper && cur_lower) || (prev_lower && cur_upper))
        all_one_case = false;
      if (cur_upper)
        capitalized = false;
      prev_upper = cur_upper;
      prev_lower = cur_lower;
    }
  } else {
    UNICHARSET *unicharset = char_set->InternalUnicharset();
    // Use UNICHARSET functions to determine case properties
    first_upper = unicharset->get_isupper(char_set->ClassID(str32[0]));
    first_lower = unicharset->get_islower(char_set->ClassID(str32[0]));
    if (first_upper)
      capitalized = true;
    prev_upper = first_upper;
    prev_lower = unicharset->get_islower(char_set->ClassID(str32[0]));

    for (int c = 1; c < StrLen(str32); ++c) {
      cur_upper = unicharset->get_isupper(char_set->ClassID(str32[c]));
      cur_lower = unicharset->get_islower(char_set->ClassID(str32[c]));
      if ((prev_upper && cur_lower) || (prev_lower && cur_upper))
        all_one_case = false;
      if (cur_upper)
        capitalized = false;
      prev_upper = cur_upper;
      prev_lower = cur_lower;
    }
  }
  return all_one_case || capitalized;
}

char_32 *CubeUtils::ToLower(const char_32 *str32, CharSet *char_set) {
  if (!char_set) {
    return NULL;
  }
  UNICHARSET *unicharset = char_set->InternalUnicharset();
  int len = StrLen(str32);
  char_32 *lower = new char_32[len + 1];
  if (!lower)
    return NULL;
  for (int i = 0; i < len; ++i) {
    char_32 ch = str32[i];
    if (ch == INVALID_UNICHAR_ID) {
      delete [] lower;
      return NULL;
    }
    // convert upper-case characters to lower-case
    if (unicharset->get_isupper(char_set->ClassID(ch))) {
      UNICHAR_ID uid_lower = unicharset->get_other_case(char_set->ClassID(ch));
      const char_32 *str32_lower = char_set->ClassString(uid_lower);
      // expect lower-case version of character to be a single character
      if (!str32_lower || StrLen(str32_lower) != 1) {
        delete [] lower;
        return NULL;
      }
      lower[i] = str32_lower[0];
    } else {
      lower[i] = ch;
    }
  }
  lower[len] = 0;
  return lower;
}

char_32 *CubeUtils::ToUpper(const char_32 *str32, CharSet *char_set) {
  if (!char_set) {
    return NULL;
  }
  UNICHARSET *unicharset = char_set->InternalUnicharset();
  int len = StrLen(str32);
  char_32 *upper = new char_32[len + 1];
  if (!upper)
    return NULL;
  for (int i = 0; i < len; ++i) {
    char_32 ch = str32[i];
    if (ch == INVALID_UNICHAR_ID) {
      delete [] upper;
      return NULL;
    }
    // convert lower-case characters to upper-case
    if (unicharset->get_islower(char_set->ClassID(ch))) {
      UNICHAR_ID uid_upper = unicharset->get_other_case(char_set->ClassID(ch));
      const char_32 *str32_upper = char_set->ClassString(uid_upper);
      // expect upper-case version of character to be a single character
      if (!str32_upper || StrLen(str32_upper) != 1) {
        delete [] upper;
        return NULL;
      }
      upper[i] = str32_upper[0];
    } else {
      upper[i] = ch;
    }
  }
  upper[len] = 0;
  return upper;
}
}  // namespace tesseract
