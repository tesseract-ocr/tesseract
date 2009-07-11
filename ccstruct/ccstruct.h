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

class PBLOB;
class DENORM;
class WERD;
class BLOB_CHOICE_LIST;

namespace tesseract {
class CCStruct : public CUtil {
 public:
  CCStruct();
  ~CCStruct();

 protected:
  Image image_;
};

class Tesseract;
}  // namespace tesseract

typedef void (tesseract::Tesseract::*POLY_MATCHER)
  (PBLOB *, PBLOB *, PBLOB *, WERD *,
   DENORM *, BLOB_CHOICE_LIST *, const char*);
/*
  typedef void (tesseract::Tesseract::*POLY_TESTER)
  (const STRING&, PBLOB *, DENORM *, BOOL8, char *,
  inT32, BLOB_CHOICE_LIST *);
*/

#endif  // TESSERACT_CCSTRUCT_CCSTRUCT_H__
