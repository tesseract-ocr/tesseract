///////////////////////////////////////////////////////////////////////
// File:        intsindmatrixavx2.h
// Description: AVX2 implementation of 8-bit int SIMD matrix multiply.
// Author:      Ray Smith
// Created:     Wed Aug 16 10:21:42 PST 2017
//
// (C) Copyright 2017, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////
#ifndef TESSERACT_ARCH_INTSIMDMATRIXAVX2_H_
#define TESSERACT_ARCH_INTSIMDMATRIXAVX2_H_

#include "intsimdmatrix.h"

namespace tesseract {

// AVX2 implementation of IntSimdMatrix.
class IntSimdMatrixAVX2 : public IntSimdMatrix {
 public:
  IntSimdMatrixAVX2();
};

}  // namespace tesseract

#endif  // TESSERACT_ARCH_INTSIMDMATRIXAVX2_H_
