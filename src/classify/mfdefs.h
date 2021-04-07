/******************************************************************************
 ** Filename:    mfdefs.h
 ** Purpose:     Definition of micro-features
 ** Author:      Dan Johnson
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/
#ifndef MFDEFS_H
#define MFDEFS_H

#include <array>
#include <forward_list>

namespace tesseract {

enum class MicroFeatureParameter {
  MFXPosition,
  MFYPosition,
  MFLength,
  MFDirection,
  MFBulge1,
  MFBulge2,

  MFCount // For array sizes.
};

using MicroFeature = std::array<float, (int)MicroFeatureParameter::MFCount>;
using MICROFEATURES = std::forward_list<MicroFeature>;

} // namespace tesseract

#endif
