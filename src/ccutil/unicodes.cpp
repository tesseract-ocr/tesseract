/**********************************************************************
 * File:        unicodes.cpp
 * Description: Unicode related machinery
 * Author:      David Eger
 * Created:     Wed Jun 15 16:37:50 PST 2011
 *
 * (C) Copyright 2011, Google, Inc.
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

#include "unicodes.h"
#include "host.h"  // for nullptr

namespace tesseract {

const char *kUTF8LineSeparator = "\u2028";  // "\xe2\x80\xa8";
const char *kUTF8ParagraphSeparator = "\u2029";  // "\xe2\x80\xa9";
const char *kLRM = "\u200E";  // Left-to-Right Mark
const char *kRLM = "\u200F";  // Right-to-Left Mark
const char *kRLE = "\u202A";  // Right-to-Left Embedding
const char *kPDF = "\u202C";  // Pop Directional Formatting

const char *kHyphenLikeUTF8[] = {
  "-",       // ASCII hyphen-minus
  "\u05BE",  // word hyphen in hybrew
  "\u2010",  // hyphen
  "\u2011",  // non-breaking hyphen
  "\u2012",  // a hyphen the same width as digits
  "\u2013",  // en dash
  "\u2014",  // em dash
  "\u2015",  // horizontal bar
  "\u2212",  // arithmetic minus sign
  "\uFE58",  // small em dash
  "\uFE63",  // small hyphen-minus
  "\uFF0D",  // fullwidth hyphen-minus
  nullptr,      // end of our list
};

const char *kApostropheLikeUTF8[] = {
  "'",       // ASCII apostrophe
  "`",       // ASCII backtick
  "\u2018",  // opening single quote
  "\u2019",  // closing single quote
  "\u2032",  // mathematical prime mark
  nullptr,      // end of our list.
};

}  // namespace
