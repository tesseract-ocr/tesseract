///////////////////////////////////////////////////////////////////////
// File:        leptonica_pageseg_interface.h
// Description: Leptonica-based page segmenter interface.
// Author:      Thomas Kielbus
// Created:     Mon Aug 27 10:05:01 PDT 2007
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

#ifndef LEPTONICA_PAGESEG_INTERFACE_H
#define LEPTONICA_PAGESEG_INTERFACE_H

class IMAGE;

// Compute the text block mask of the page_image and put the result into the
// textblock_mask_image. Return true if no error has occured.
bool leptonica_pageseg_get_textblock_mask(IMAGE* page_image,
                                          IMAGE* textblock_mask_image);

#endif  // LEPTONICA_PAGESEG_INTERFACE_H
