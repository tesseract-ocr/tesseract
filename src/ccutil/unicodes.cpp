/**********************************************************************
 * File:        unicodes.cpp
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

#include "unicodes.h"

namespace tesseract {

const char * const kLRM = "\u200E";  // Left-to-Right Mark
const char * const kRLM = "\u200F";  // Right-to-Left Mark
const char * const kRLE = "\u202A";  // Right-to-Left Embedding
const char * const kPDF = "\u202C";  // Pop Directional Formatting

}  // namespace
