///////////////////////////////////////////////////////////////////////
// File:        version.h
// Description: Version information
//
// (C) Copyright 2018, Google Inc.
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

#ifndef TESSERACT_API_VERSION_H_
#define TESSERACT_API_VERSION_H_

#define TESSERACT_MAJOR_VERSION 4
#define TESSERACT_MINOR_VERSION 0
#define TESSERACT_MICRO_VERSION 0
#define TESSERACT_VERSION \
  (TESSERACT_MAJOR_VERSION << 16 | \
   TESSERACT_MINOR_VERSION << 8 | \
   TESSERACT_MICRO_VERSION)
#define TESSERACT_VERSION_STR "4.0.0-beta.1-207-g984a"

#endif  // TESSERACT_API_VERSION_H_
