///////////////////////////////////////////////////////////////////////
// File:        functions.cpp
// Description: Static initialize-on-first-use non-linearity functions.
// Author:      Ray Smith
// Created:     Tue Jul 17 14:02:59 PST 2014
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

#include "functions.h"

namespace tesseract {

double TanhTable[kTableSize];
double LogisticTable[kTableSize];

}  // namespace tesseract.
