///////////////////////////////////////////////////////////////////////
// File:        pageseg.h
// Description: Page Segmenter
// Author:      Thomas Kielbus
// Created:     Wed Jul 18 10:05:01 PDT 2007
//
// (C) Copyright 2007, Google Inc.
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

#ifndef PAGESEG_H
#define PAGESEG_H

class BLOCK_LIST;

// Segment the global page in text blocks and append the corresponding BLOCKs to
// the given list. Currently, they are not sorted by reading order.
void segment_page(BLOCK_LIST *blocks);

#endif  // PAGESEG_H
