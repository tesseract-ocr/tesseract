///////////////////////////////////////////////////////////////////////
// File:        cutil_class.h
// Description: cutil class.
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

#ifndef TESSERACT_CUTIL_CUTIL_CLASS_H_
#define TESSERACT_CUTIL_CUTIL_CLASS_H_

#include "ccutil.h"
#include "strngs.h"

namespace tesseract {

class CUtil : public CCUtil {
 public:
  CUtil() = default;
  virtual ~CUtil();
  void read_variables(const char *filename, bool global_only);
};

}  // namespace tesseract

#endif  // TESSERACT_CUTIL_CUTIL_CLASS_H_
