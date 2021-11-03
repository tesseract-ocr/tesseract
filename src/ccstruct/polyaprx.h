/**********************************************************************
 * File:        polyaprx.h
 * Description: Code for polygonal approximation from old edgeprog.
 * Author:      Ray Smith
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifndef POLYAPRX_H
#define POLYAPRX_H

namespace tesseract {

class C_OUTLINE;
struct TESSLINE;

// convert a chain-coded input to the old OUTLINE approximation
TESSLINE *ApproximateOutline(bool allow_detailed_fx, C_OUTLINE *c_outline);

} // namespace tesseract

#endif
