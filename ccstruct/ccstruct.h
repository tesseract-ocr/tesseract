///////////////////////////////////////////////////////////////////////
// File:        ccstruct.h
// Description: ccstruct class.
// Author:      Samuel Charron
//
// (C) Copyright 2006, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_CCSTRUCT_CCSTRUCT_H__
#define TESSERACT_CCSTRUCT_CCSTRUCT_H__

#include "cutil.h"
#include "image.h"

namespace tesseract {
class CCStruct : public CUtil {
 public:
  CCStruct();
  ~CCStruct();

  // Globally accessible constants.
  // APPROXIMATIONS of the fractions of the character cell taken by
  // the descenders, ascenders, and x-height.
  static const double kDescenderFraction;  // = 0.25;
  static const double kXHeightFraction;    // = 0.5;
  static const double kAscenderFraction;   // = 0.25;
  // Derived value giving the x-height as a fraction of cap-height.
  static const double kXHeightCapRatio;    // = XHeight/(XHeight + Ascender).

 protected:
  Image image_;
};

class Tesseract;
}  // namespace tesseract


#endif  // TESSERACT_CCSTRUCT_CCSTRUCT_H__
