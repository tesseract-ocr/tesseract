/**********************************************************************
 * File:        unicodes.h
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

#ifndef TESSERACT_CCUTIL_UNICODES_H__
#define TESSERACT_CCUTIL_UNICODES_H__

namespace tesseract {

extern const char *kUTF8LineSeparator;
extern const char *kUTF8ParagraphSeparator;
extern const char *kLRM;  //< Left-to-Right Mark
extern const char *kRLM;  //< Right-to-Left Mark
extern const char *kRLE;  //< Right-to-Left Embedding
extern const char *kPDF;  //< Pop Directional Formatting

/// The following are confusable internal word punctuation symbols
/// which we normalize to the first variant when matching in dawgs.
extern const char *kHyphenLikeUTF8[];
extern const char *kApostropheLikeUTF8[];

}  // namespace

#endif  // TESSERACT_CCUTIL_UNICODES_H__
