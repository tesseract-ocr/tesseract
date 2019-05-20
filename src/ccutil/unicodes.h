/**********************************************************************
 * File:        unicodes.h
 * Description: Unicode related machinery
 * Author:      David Eger
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

#ifndef TESSERACT_CCUTIL_UNICODES_H_
#define TESSERACT_CCUTIL_UNICODES_H_

namespace tesseract {

extern const char* const kLRM;  ///< Left-to-Right Mark
extern const char* const kRLM;  ///< Right-to-Left Mark
extern const char* const kRLE;  ///< Right-to-Left Embedding
extern const char* const kPDF;  ///< Pop Directional Formatting

}  // namespace

#endif  // TESSERACT_CCUTIL_UNICODES_H_
