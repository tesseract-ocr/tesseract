///////////////////////////////////////////////////////////////////////
// File:        functions.cpp
// Description: Static initialize-on-first-use non-linearity functions.
// Author:      Ray Smith
//
// (C) Copyright 2014, Google Inc.
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

#include <cmath>        // for exp, tanh
#include "functions.h"

namespace tesseract {

#if __cplusplus < 201402 || defined(__clang__) // C++11

double TanhTable[kTableSize];
double LogisticTable[kTableSize];

class TableInit {
  TableInit() {
    for (int i = 0; i < kTableSize; i++) {
      TanhTable[i] = tanh(i / kScaleFactor);
      LogisticTable[i] = 1 / (1 + exp(-i / kScaleFactor));
    }
  }
  static TableInit tableInit;
};

TableInit TableInit::tableInit;

#else // C++14 or newer

constexpr LUTTempl<kTableSize, LUTFuncTanh> TanhTable;
constexpr LUTTempl<kTableSize, LUTFuncLog>  LogisticTable;

#endif

}  // namespace tesseract.
