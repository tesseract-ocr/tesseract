///////////////////////////////////////////////////////////////////////
// File:        image.h
// Description: image class.
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

#ifndef THIRD_PARTY_TESSERACT_IMAGE_IMAGE_H_
#define THIRD_PARTY_TESSERACT_IMAGE_IMAGE_H_

namespace tesseract {

class CCUtil;

class Image {
 public:
  Image(CCUtil* ccutil_ptr);
  CCUtil* getCCUtil() {
    return ccutil_ptr_;
  }

 private:
  CCUtil* ccutil_ptr_;
};
}  // namespace tesseract

#endif  // THIRD_PARTY_TESSERACT_IMAGE_IMAGE_H_
